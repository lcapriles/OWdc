// 01rc01.cpp 
//
//Creado:		Luis Capriles,		Fecha:	22/01/2004 
//Modificado:	Luis Capriles,		Fecha:	01/10/2008 - Conversion a UniCode
//Modificado:	Williams Ovalles,	Fecha:	18/06/2012 - Uso de curses para manejo de salida.
//Modificado:	Luis Capriles,		Fecha:	08/08/2012 - Varios BSN.
//													   - MCU es de la OC, no del operador...
//Modificado:	Luis Capriles,		Fecha:	10/09/2012 - Múltiples OC.
//Modificado:	Luis Capriles,		Fecha:	13/10/2014 - Manejo Estado del Lote...
//
//El programa rc01 es una versión reducida del programa P4312-Recepción Compras.
//1)	Solicita Números de Ordenes de Compra y carga todas las líneas en un caché en memoria.
//1.1)		Si no se introduce Orden de Compra, la ejecución termina.
//1.2)		Si se especifican varias Ordenes de Compra, se cargan las varias órdenes en el caché.
//2)	Solicita un código de producto, se valida si es un código de barras llamando a la función GetBC 
//		y trata de localizarlo en el caché.
//2.1)		Si no se introduce un código de producto se retorna al punto 1).
//2.2)	Una vez localizado el código del producto en el caché, se solicita cantidad, etc.
//2.3)	Si el código introducido existe más de una vez en el chaché, se van procesando el caché en orden; recepcionando sucesivamente...
//2.4)	Si todo está OK, se confirma la línea de la recepción y retorna al punto 2).
//2.5)  Si no se introduce código de producto,se supone que se quiere termknar la captura:
//2.5.1) Se solicita al operador confirmar la confirmación.  Si se acepta, se crea una confirmación para cada orden de compra procesada.
//
#include "rc01.h"
#include <stdio.h>
#include "jde.h"

#include "xt4312z1.h"	// F4312BeginDoc, F4312EditLine, F4312EndDoc,F4312ClearWorkFile

#include "b9800100.h"	// GetAuditInfo
#include "B4000370.h"	// F40095GetDefaultBranch
#include "B0000130.h"	// RetrieveCompanyFromBusUnit
#include "B4000150.h"	// GetBranchConstants
#include "xf41021.h"	// VerifyAndGetItemLocation
#include "b4000310.h"	// FormatLocation
#include "b4001050.h"	// GetCrossReferenceFields
#include "n0000563.h"	// F0010RetrieveCompanyConstant
#include "B0900049.h"	// F0911FSGetNextDocNumber
#include "B0000065.h"	// X0010GetNextNumber
#include "X0903.H"		// ReturnFYPN
#include "xt4311z1.h"	// F4311ClearWorkFiles
#include "b4000610.h"	// GetLotMasterByLotNumber
#include "b0000042.h"	// FSCloseBatch
#include "F43121.h"		// PO Receiver File
#include "B1100007.h"	// DecimalsTriggerGetbyCOCRCD
#include "B4000520.h"	// GetItemUoMConversionFactor
#include "x4101.h"		// GetItemMasterByShortItem, GetItemMasterBy3rdItem, GetItemMasterBy2ndItem
#include "B4300130.h"	// GetPurchaseHeaderColumns
#include "B4000920.h"	// GetItemBranchMfgData

void OWDCmp02 (int * primeraVez, char * pantallaTitulo, int camposOffset, int camposCantidad, 
				int statusOffset, int inicioEtiquetas, int inicioCampos, int ultimaLinea, int CReqTAB,
				int camposPosiciones[], char * camposEtiquetas[], char camposContenido[][128], int camposErrores[],
				char * pantallaStatusLine, int iDbgFlg, FILE * dlg); //Manejo de la pantalla de entrada datos...

void OWDCmp90 (int iDbgFlg, FILE * dlg); //Terminar Manejo de la pantalla curses...

int OWDCgbc(HENV hEnv,HUSER hUser,JCHAR LszUsrEntry[128],JCHAR LszLineItemBuf[26],MATH_NUMERIC * mnItemShortIDBuf,
			JCHAR LszUOMdefaultBuf[3],JCHAR LszUOMstdConv[3],JCHAR LszCrossRefTypeCodeBuf[3],int iUPClenBuf,int iSCClenBuf,
			JCHAR szItemDescription[31],int iGetItemMasterBy, JCHAR c3rdItemNoSymbol, int iDbgFlg,FILE * dlg); //Valida código producto...

int ProcesaCB(char * szCodigoBarra, char * szCBprod,char * szCBlote,char * szCBfecha,int iUPClenBuf,int iSCClenBuf,int iDbgFlg, FILE * dlg);

int FormateaFecha(char * szFechaI, JCHAR * LszFechaO);

__declspec(dllexport) int OWDCrc01(HENV hEnv,HUSER hUser)
{

	HREQUEST			hRequestF4311	= (HREQUEST)NULL;;
	LPJDEERROR_RECORD	ErrorRec	= NULL;
	ID					idResult;
	LPCG_BHVR			lpVoid		= NULL;
	LPCG_BHVR			lpVoid1		= NULL;
	LPBHVRCOM			lpBhvrCom	= NULL;
	LPBHVRCOM			lpBhvrCom1	= NULL;
	ERROR_EVENT_KEY		EventKeyLocal;

	LPF4101				lpdsF4101 = (LPF4101) NULL;

	DSDF4312Z1A			dsF4312BeginDoc;
	DSDF4312Z1B			dsF4312EditLine;
	DSDF4312Z1C			dsF4312EndDoc;
	DSD4000150			dsGetBranchConstants;
	DSD4000232			dsF40095GetDefaultBranch;
	DSD0000130			dsRetrieveCompanyFromBusUnit;
	DSDXF41021C			dsVerifyAndGetItemLocation;
	DSD4001050			dsGetCrossReferenceFields;
	DSD4000310A			dsFormatLocation;
	DSD9800100			dsGetAuditInfo;
	DSD0000563			dsF0010RetrieveCompanyConstant;
	DSD0900049J			dsF0911FSGetNextDocNumber;
	DSD0000065A			dsX0010GetNextNumber;
	DSDXX00023			dsReturnFYPN;
	DSDF4312Z1D			dsF4312ClearWorkFile;
	DSDF4311Z1E			dsF4311ClearWorkFiles;
	LPFORMDSUDC			lpValidateUDC;
	FORMDSUDC			dsValidateUDC;
	DSD4000610			dsGetLotMasterByLotNumber;
	DSD0000042B			dsFSCloseBatch;
	DSD1100007			dsDecimalsTriggerGetbyCOCRCD;
	DSD4000520			dsGetItemUoMConversionFactor  = {0};
	DSDX4101B			dsGetItemMasterByShortItem;
	DSD4300130B			dsGetPurchaseHeaderColumns;
	DSD4000920			dsGetItemBranchMfgData;
	
	typedef struct {	//Implementa el cache: una linea por cada item de las OC...
		JCHAR			pdkcoo[6];	//Order Company 
		MATH_NUMERIC	pddoco;		//Document(OrderNo, Invoice, etc)
		JCHAR			pddcto[3];	//Order Type
		JCHAR			pdsfxo[4];	//Order Suffix
		MATH_NUMERIC	pdlnid;		//Line Number
		JCHAR			pdmcu[13];	//Business Unit
		MATH_NUMERIC	pditm;		//IdentifierShortID
		JCHAR			pdaitm[26];	//Identifier3rdItem
		JCHAR			pdlocn[21];	//Location
		JCHAR			pdlotn[31]; //Lot
		JCHAR			pddsc1[31];	//Description 1
		JCHAR			pdlnty[3];	//Line Type
		JCHAR			pdnxtr[4];	//Incoming Status Next
		JCHAR			pduom[3];	//UOM as Input
		MATH_NUMERIC	pduorg;		//Units Transaction Quantity
		MATH_NUMERIC	pduopn;		//Units Open Quantity
		MATH_NUMERIC	pdprrc;		//Purchasing Unit Price
		JCHAR			pdcrcd[4];  //Purchasing Currency From
		MATH_NUMERIC	pdfrrc;		//Purchasing Foreign Unit Price
		JCHAR			pduom1[3];	//UoM Primary
		MATH_NUMERIC	pdpqor;		//Units Primary UoM
		MATH_NUMERIC	CantAcc;	//Cantidad Recibida UoM Primary
		MATH_NUMERIC	NumLinRC;	//Numero Linea Recepcion
		JCHAR			F43121ZWritten;
		JCHAR			F4111Written;
		MATH_NUMERIC	JElnid;		
		int				iStatusRegistro;		//0=no procesado, 1=visualizado, 2=procesado
	} stF4311rc01;

	typedef struct { 
		MATH_NUMERIC	pddoco;		//Document(OrderNo, Invoice, etc)
		int				iEditLineLines;	//Flag para indicar que la OC tiene lineas para EndDoc..
		int				iRecordsAvailCache;//Cantidad de líneas de la OC
		MATH_NUMERIC	mnF4111JobNumber;
		MATH_NUMERIC	mnF0911JobNumber;
		JCHAR			szBatchType[3];
		MATH_NUMERIC	mnBatchNumber;
		JCHAR           szTransactionCurrency[4];
	} stVarBuf;

	typedef struct { 
		MATH_NUMERIC	pddoco;		//Document(OrderNo, Invoice, etc)
		JCHAR			pddcto[3];	//Order Type
	} stVarBuf2;

	#define CacheSize 1536
	#define DocQty	96

	stF4311rc01			dsF4311Arr[CacheSize];		//El chache: Arreglo de líneas...
	stVarBuf			dsVarBuf[DocQty];			//Arreglo de variables...
	stVarBuf2			dsVarBuf2[DocQty];			//Arreglo de variables para procesar por segunda vez...
	DSDF4312Z1A			dsF4312BeginDocArr[DocQty];	//Arreglo de BeginDocs...
	DSDF4312Z1C			dsF4312EndDocArr[DocQty];	//Arreglo de EndDocs...


	NID					szF4311ColumnsArray[21] = {NID_KCOO,NID_DOCO,NID_DCTO,NID_SFXO,NID_LNID,NID_MCU,NID_ITM,NID_AITM,
													NID_LOCN,NID_LOTN,NID_DSC1,NID_LNTY,NID_NXTR,NID_UOM,NID_UORG,
													NID_UOPN,NID_PRRC,NID_CRCD,NID_FRRC,NID_UOM1,NID_PQOR};
	KEY1_F4311			dsF4311Key1;
	//SELECTSTRUCT		lpSelect[6];	//Amazing Global Vzla - WOVALLES - 26/07/2012
	SELECTSTRUCT		lpSelect[5];	// Se Modifica porque en BSN se Obtiene la MCU de la Orden de Compra
	MATH_NUMERIC		mnTempBuf,mnTemp0Buf,mnItemShortIDBuf,mnCantRecepcionarTemp,
						mnCantIntroducidaBuf,
						mnF4111JobNumberBuf,mnF0911JobNumberBuf;

	FILE * ini;
	FILE * dlg;

	JCHAR LszLinea[80],LszLin1[64],LszLin2[64],LszUsrEntry[128],

		 LszDocTransDateBuf[16],LszDocGLDateBuf[16],LszDocBranchPlantBuf[16],LszLocationBuf[21],LszLineLocationBuf[21],
		 LszLineLotBuf[31],LszLineItemBuf[26],LszLineUMBuf[3],LszUOMdefaultBuf[3],LszUOMstdConv[3],LszLotExpirationDate[16],

		 LszPOOrderTypeBuf[3],LszReceiptDocumentTypeBuf[3],LszP4312VersionBuf[16],LszIncomingStatus1Buf[4],
		 LszIncomingStatus2Buf[4],LszIncomingStatus3Buf[4],LszOutgoingStatusPartialBuf[4],LszOutgoingStatusClosingBuf[4],
		 LszOutgoingStatusCancelingBuf[4],LszP0900049JEVersionBuf[16],LszCrossRefTypeCodeBuf[3],LszDocumentCompanyBuf[6],
		 LszDocumentTypeBuf[3],LszItemDescriptionBuf[31],LszUbicacionDfltBuf[21], 

		 LszString01[64],LszString02[64],LszString03[64],LszString04[64],LszTempBuf[128],

		 LcDecimalCharBuf,LcP4312LineOptionBuf,LcSummarizedF0911Buf,LcLotProcess,LcValidarUbicacion,LcValidarLote,LcCurrencyFlag,
		 LcYesNoBuf,LcTempBuf,LcTemp1Buf; 

	JDEDATE	jFechaTemp, jFechaTemp1;

	int	 iDocumentNumberBuf,iRecordsF4311Read,iLineaEscogida,iUPClenBuf,iSCClenBuf,iCantLotesProc,iNumLotes,iGetItemMasterBy,iLineQtyBuf,
		 iLotExpirationDate,
		 iErrorCode,iErrorCode1,iDbgFlg,i,j,iSalir,iError1,iError2,
		 iPrimeraVez,iCamposOffset,iCamposCantidad,iStatusOffset,iInicioEtiquetas,iInicioCampos,iUltimaLinea,iCReqTAB,iUnicaVez,
		 iCamposPosiciones[64], iCamposErrores[64],
		 iItemFoundInCache,iRecordsAvailCache,iEditLineLines,iOCindex,iOCindex2,iOCprocesada;

	int	 iIdxCo = 0, iIdxLoc = 0, iIdxDT = 0, iIdxDoc = 0,
		 iIdxProd = 0, iIdxDescr = 0, iIdxCant = 0, iIdxLot = 0, iIdxFVcto = 0;

	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128],
		 * szDummy1,szTempBuf[128],*szTempBuf1,*szTempBuf2;

	long nDateDifference;

	double fLineQtyBuf,fLineQty1Buf,fLinePrcBuf;

	#define INIwidth 80

	//*************************************************************************
	//***Procesamiento Archivo INI                                          ***
	//*************************************************************************

	//Contruye nombre archivo como nombre_Daammdd_Thhmmss.log
	char timebuf[9],datebuf[9];
	JCHAR Ltimebuf[9],Ldatebuf[9],szFileNameBuf[128];
	_strtime(timebuf);
	_strdate(datebuf);
	jdeToUnicode(Ltimebuf,timebuf,DIM(Ltimebuf),UTF8);
	jdeToUnicode(Ldatebuf,datebuf,DIM(Ldatebuf),UTF8);
	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCrc01_D"));
	Ldatebuf[2] = _J('\0');
	Ldatebuf[5] = _J('\0'); 
	jdeStrcpy(LszTempBuf,Ldatebuf + 6);
	jdeStrcat(szFileNameBuf,LszTempBuf);
	jdeStrcpy(LszTempBuf,Ldatebuf);
	jdeStrcat(szFileNameBuf,LszTempBuf);
	jdeStrcpy(LszTempBuf,Ldatebuf + 3);
	jdeStrcat(szFileNameBuf,LszTempBuf);
	Ltimebuf[2] = _J('\0');
	Ltimebuf[5] = _J('\0');
	jdeStrcat(szFileNameBuf,_J("_T"));
	jdeStrcpy(LszTempBuf,Ltimebuf);
	jdeStrcat(szFileNameBuf,LszTempBuf);
	jdeStrcpy(LszTempBuf,Ltimebuf + 3);
	jdeStrcat(szFileNameBuf,LszTempBuf);
	jdeStrcpy(LszTempBuf,Ltimebuf + 6);
	jdeStrcat(szFileNameBuf,LszTempBuf);
	jdeStrcat(szFileNameBuf,_J(".log"));
	dlg = jdeFopen(szFileNameBuf,_J("w"));

	iDbgFlg = 0;

	ini = jdeFopen(_J("OWDCrc01.ini"),_J("r"));
	if (!ini){
		jdeFprintf(stderr,_J("***Error abriendo INI (OWDCrc01.ini)...\n"));
        return 200;
	}

	while (jdeFgets(LszLinea,INIwidth,ini) != NULL){ //***Mientras haya registros en ini file
		for(i=0; (LszLinea[i] != _J('=') && i < INIwidth); i++);
		if (LszLinea[i] == _J('=')){//Localiza el signo "="
			jdeStrncpy(LszLin1, LszLinea, i);
			LszLin1[i] = _J('\0');//Divide el string en una porción antes "=" y otra porción despues "="
			i++;
			jdeStrncpy(LszLin2,LszLinea + i,jdeStrlen(LszLinea) - i + 1 );
			LszLin2[jdeStrlen(LszLin2) - 1] = _J('\0');
		}
//		if (iDbgFlg > 0) fprintf(dlg,"%s\n",LszLin1);
//		if (iDbgFlg > 0) fprintf(dlg,"%s\n",LszLin2);

		if(jdeStrcmp(LszLin1,_J("Debug")) == 0){
			iDbgFlg = jdeAtoi(LszLin2); // 1-> SI debug, 0->NO debug
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Debug (%d)...\n"),iDbgFlg);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("DecimalChar")) == 0){
			if (jdeStrcmp(LszLin2,_J(",")) == 0) LcDecimalCharBuf = _J(','); else  LcDecimalCharBuf = _J('.');
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: DecimalChar (%lc)...\n"),LcDecimalCharBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("camposOffset")) == 0){
			iCamposOffset = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: camposOffset (%d)...\n"),iCamposOffset); 
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("statusOffset")) == 0){
			iStatusOffset = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: statusOffset (%d)...\n"),iStatusOffset); 
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("inicioEtiquetas")) == 0){
			iInicioEtiquetas = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: inicioEtiquetas (%d)...\n"),iInicioEtiquetas); 
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("inicioCampos")) == 0){
			iInicioCampos = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: inicioCampos (%d)...\n"),iInicioCampos); 
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("ultimaLinea")) == 0){
			iUltimaLinea = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ultimaLinea (%d)...\n"),iUltimaLinea); 
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("CReqTAB")) == 0){
			iCReqTAB = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: CReqTAB (%d)...\n"),iCReqTAB); 
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("Yes")) == 0){
			LcYesNoBuf = LszLin2[0];
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Yes (%lc)...\n"),LcYesNoBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("FechaGL")) == 0){
			jdeStrcpy(LszDocGLDateBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: FechaGL (%ls)...\n"),LszDocGLDateBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("FechaTran")) == 0){
			jdeStrcpy(LszDocTransDateBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: FechaTran (%ls)...\n"),LszDocTransDateBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("POOrderType")) == 0){
			jdeStrcpy(LszPOOrderTypeBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: POOrderType (%ls)...\n"),
										LszPOOrderTypeBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("ReceiptDocumentType")) == 0){
			jdeStrcpy(LszReceiptDocumentTypeBuf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ReceiptDocumentType (%ls)...\n"),  
										LszReceiptDocumentTypeBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P4312Version")) == 0){
			jdeStrcpy(LszP4312VersionBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P4312Version (%ls)...\n"),LszP4312VersionBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P4312LineOption")) == 0){
			LcP4312LineOptionBuf = LszLin2[0];
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P4312LineOption (%lc)...\n"),LcP4312LineOptionBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("IncomingStatus1")) == 0){
			jdeStrcpy(LszIncomingStatus1Buf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: IncomingStatus1 (%ls)...\n"),
										LszIncomingStatus1Buf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("IncomingStatus2")) == 0){
			jdeStrcpy(LszIncomingStatus2Buf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: IncomingStatus2 (%ls)...\n"),
										LszIncomingStatus2Buf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("IncomingStatus3")) == 0){
			jdeStrcpy(LszIncomingStatus3Buf, LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: IncomingStatus3 (%ls)...\n"),
										LszIncomingStatus3Buf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("OutgoingStatusPartial")) == 0){
			jdeStrcpy(LszOutgoingStatusPartialBuf, LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: OutgoingStatusPartial (%ls)...\n"),
										LszOutgoingStatusPartialBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("OutgoingStatusClosing")) == 0){
			jdeStrcpy(LszOutgoingStatusClosingBuf, LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: OutgoingStatusClosing (%ls)...\n"),
										LszOutgoingStatusClosingBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("OutgoingStatusCanceling")) == 0){
			jdeStrcpy(LszOutgoingStatusCancelingBuf, LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: OutgoingStatusCanceling (%ls)...\n"),
										LszOutgoingStatusCancelingBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("SummarizedF0911")) == 0){
			if (jdeStrcmp(LszLin2,_J("1")) == 0) LcSummarizedF0911Buf = _J('1'); 
			else  LcSummarizedF0911Buf = _J(' ');			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: SummarizedF0911 (%lc)...\n"),LcSummarizedF0911Buf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P0900049JEVersion")) == 0){
			jdeStrcpy(LszP0900049JEVersionBuf, LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P0900049JEVersion (%ls)...\n"),LszP0900049JEVersionBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("CurrencyFlag")) == 0){
			LcCurrencyFlag = LszLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: CurrencyFlag (%lc)...\n"),
										LcCurrencyFlag);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("UPClen")) == 0){
			iUPClenBuf = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: UPClen (%d)...\n"),
										iUPClenBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("SCClen")) == 0){
			iSCClenBuf = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: SCClen (%d)...\n"),
										iSCClenBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("CrossRefTypeCode")) == 0){
			jdeStrcpy(LszCrossRefTypeCodeBuf, LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: CrossRefTypeCode (%ls)...\n"),
										LszCrossRefTypeCodeBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("LotProcess")) == 0){
			LcLotProcess = LszLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LotProcess (%lc)...\n"),
										LcLotProcess);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("ValidarUbicacion")) == 0){
			LcValidarUbicacion = LszLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ValidarUbicacion (%lc)...\n"),
										LcValidarUbicacion);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("ValidarLote")) == 0){
			LcValidarLote = LszLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ValidarLote (%lc)...\n"),
										LcValidarLote);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("LotExpirationDate")) == 0){
			iLotExpirationDate = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LotExpirationDate (%d)...\n"),iLotExpirationDate);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("UbicacionDflt")) == 0){
			jdeStrcpy(LszUbicacionDfltBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: UbicacionDflt (%ls)...\n"),LszUbicacionDfltBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("GetItemMasterBy")) == 0){
			iGetItemMasterBy = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: GetItemMasterBy (%d)...\n"),
										iGetItemMasterBy);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("LineQty")) == 0){
			iLineQtyBuf = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LineQty (%d)...\n"),
										iLineQtyBuf);
			continue;
		}
	}
	jdeFclose(ini);
	jdeFflush(dlg);

	iErrorCode = 0; //No hay Errores!!!

	//*************************************************************************
	// Set up the lpBhvrCom amd lpVoid objects                              ***
	//*************************************************************************
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Seteo de lpBhvrCom, lpVoid y estructuras...\n"));
	//F4312BeginDoc, F4312EditLine, F4312EndDoc
	jdeCreateBusinessFunctionParms(hUser, &lpBhvrCom1,(LPVOID*) &lpVoid1);
	lpVoid1->lpHdr = jdeErrorInitializeEx();
	lpVoid1->lpErrorEventKey = (LPERROR_EVENT_KEY) jdeAlloc(COMMON_POOL, 
										sizeof(ERROR_EVENT_KEY), MEM_ZEROINIT | MEM_FIXED);
	lpVoid1->lpHdr->nCurDisplayed = -1;
	lpBhvrCom1->lpObj->lpFormHdr = lpVoid1->lpHdr;
	EventKeyLocal.hwndCtrl = NULL;
	EventKeyLocal.iGridCol = 0;
	EventKeyLocal.iGridRow = 0;
	EventKeyLocal.wEvent = 1;
	lpBhvrCom1->lpEventKey = (LPVOID)&EventKeyLocal;

	//Otros
	jdeCreateBusinessFunctionParms(hUser, &lpBhvrCom,(LPVOID*) &lpVoid);
	lpVoid->lpHdr = jdeErrorInitializeEx();
	lpVoid->lpErrorEventKey = (LPERROR_EVENT_KEY) jdeAlloc(COMMON_POOL, 
										sizeof(ERROR_EVENT_KEY), MEM_ZEROINIT | MEM_FIXED);
	lpVoid->lpHdr->nCurDisplayed = -1;
	lpBhvrCom->lpObj->lpFormHdr = lpVoid->lpHdr;
	EventKeyLocal.hwndCtrl = NULL;
	EventKeyLocal.iGridCol = 0;
	EventKeyLocal.iGridRow = 0;
	EventKeyLocal.wEvent = 1;
	lpBhvrCom->lpEventKey = (LPVOID)&EventKeyLocal;

	memset((void *) &dsGetAuditInfo,(int) _J('\0'),sizeof(DSD9800100));
	memset((void *) &dsF40095GetDefaultBranch,(int) _J('\0'),sizeof(DSD4000232));
	memset((void *) &dsRetrieveCompanyFromBusUnit, (int) _J('\0'), sizeof(DSD0000130));
	memset((void *) &dsGetBranchConstants, (int) _J('\0'), sizeof(DSD4000150));
	memset((void *) &dsRetrieveCompanyFromBusUnit,(int) _J('\0'),sizeof(DSD0000130));
	memset((void *) &dsVerifyAndGetItemLocation,(int) _J('\0'),sizeof(DSDXF41021C));
	memset((void *) &dsGetCrossReferenceFields,(int) _J('\0'),sizeof(DSD4001050));
	memset((void *) &dsF0010RetrieveCompanyConstant,(int) _J('\0'),sizeof(DSD0000563));
	memset((void *) &dsReturnFYPN,(int)(_J('\0')),sizeof(DSDXX00023));
	memset((void *) &dsX0010GetNextNumber,(int)(_J('\0')),sizeof(DSD0000065A));
	memset((void *) &dsGetLotMasterByLotNumber,(int)(_J('\0')),sizeof(DSD4000610));
	memset((void *) &dsFSCloseBatch,(int)(_J('\0')),sizeof(DSD0000042B));

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Determina Almacen, Compania, Maquina, Fecha y ProcOpt...\n"));

	//***Obtiene Usuario, máquina, fecha
	idResult = jdeCallObject(_J("GetAuditInfo"),NULL,lpBhvrCom,lpVoid,&dsGetAuditInfo,
					(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
	if (idResult == ER_ERROR){
		iErrorCode = 201;
		jdeFprintf(dlg,_J("***Error(%d): GetAuditInfo:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 

	//***Busca Almacén por default del usuario
	idResult = jdeCallObject(_J("GetDefaultBranch"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsF40095GetDefaultBranch, 
						(CALLMAP *)NULL, (int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 202;
		jdeFprintf(dlg,_J("***Error(%d): GetDefaultBranch:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	jdeStrcpy(LszDocBranchPlantBuf,dsF40095GetDefaultBranch.szBranch);
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetDefaultBranch (%ls)...\n"),LszDocBranchPlantBuf);

	//***Busca Compañía de Centro de Costo del Usuario	
	jdeStrcpy(dsRetrieveCompanyFromBusUnit.szCostCenter,LszDocBranchPlantBuf);
	idResult = jdeCallObject(_J("RetrieveCompanyFromBusUnit"),NULL,lpBhvrCom,lpVoid,
						(LPVOID)&dsRetrieveCompanyFromBusUnit,(CALLMAP *)NULL,(int)0, 
						(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 203;
		jdeFprintf(dlg,_J("***Error(%d): RetrieveCompanyFromBusUnit:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***RetrieveCompanyFromBusUnit (%ls)...\n"),dsRetrieveCompanyFromBusUnit.szCompany);

/*	
	//Obtiene Moneda y Conversión de la compañia
	jdeStrcpy(dsF0010RetrieveCompanyConstant.szCompany,_J("00000"));
	idResult = jdeCallObject(_J("F0010RetrieveCompanyConstant"),NULL,lpBhvrCom,lpVoid,
							(LPVOID)&dsF0010RetrieveCompanyConstant,(CALLMAP *)NULL,(int)0, 
							(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 204;
		jdeFprintf(dlg,_J("***Error(%d): F0010RetrieveCompanyConstant:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	if (iDbgFlg > 0){
		jdeFprintf(dlg,_J("***F0010RetrieveCompanyConstant (%lc/%ls)...\n"),
							dsF0010RetrieveCompanyConstant.cCurrencyConverYNAR,
							dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);
		jdeFflush(dlg);
	}

*/
	// Determina Símbolo para identificar 3rd Inv Number...
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Determina dsGetCrossReferenceFields...\n"));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szSystemCode,(const JCHAR *)_J(" "));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szBranchPlant,(const JCHAR *)LszDocBranchPlantBuf);
	dsGetCrossReferenceFields.cSuppressErrorMsg = _J('1');
	idResult = jdeCallObject(_J("GetCrossReferenceFields"), NULL,lpBhvrCom,lpVoid,
	                      (LPVOID)&dsGetCrossReferenceFields,(LPCALLMAP)NULL,(int)0,
						  (JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if((idResult == ER_ERROR) || (jdeStrcmp(dsGetCrossReferenceFields.szErrorMsgID,_J(" ")) != 0)){
		iErrorCode = 205;
		jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
		while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){			
			jdeFprintf(dlg,_J("***Error(%d): GetCrossReferenceFields: (%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc);
			jdeFflush(dlg);
		} 
		goto lbFIN;
	}
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***dsGetCrossReferenceFields (%lc)...\n"),dsGetCrossReferenceFields.c3rdItemNoSymbol);


	//*************************************************************************
	//***Solicita Datos de Entrada                                          ***
	//*************************************************************************

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Inicializando panttalla datos entrada...\n"));

	memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
	memset(szCamposContenido,'\0',sizeof(szCamposContenido));
	memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
	memset(iCamposErrores,'\0',sizeof(iCamposErrores));
	memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
	memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

	iPrimeraVez = 0;

	strcpy (szPantallaTitulo, "Recepcion Ordenes Compra          ");

	szCamposEtiquetas[0] = "F. Contable    ";
	szCamposEtiquetas[1] = "F. Transaccion "; 
	
	iCamposCantidad = 2;//Dos campos a desplegar...

	//***Fecha Contable
	if (jdeStrcmp(LszDocGLDateBuf,_J("0")) == 0) FormatDate(LszDocGLDateBuf,&dsGetAuditInfo.jdDate,(JCHAR*) NULL);
	jdeFromUnicode(szCamposContenido[0],LszDocGLDateBuf,10,UTF8);	
	iCamposPosiciones[0] = strlen(szCamposContenido[0]);

	//*** Fecha Transaccion
	if (jdeStrcmp(LszDocTransDateBuf,_J("0")) == 0) FormatDate(LszDocTransDateBuf,&dsGetAuditInfo.jdDate,(JCHAR*) NULL);
	jdeFromUnicode(szCamposContenido[1],LszDocTransDateBuf,10,UTF8);
	iCamposPosiciones[1] = strlen(szCamposContenido[1]);
	
	do {  //Loop para Validar Fechas de Documento...
		iErrorCode = 0;

		memset(LszDocGLDateBuf,'\0',sizeof(LszDocGLDateBuf));
		memset(LszDocTransDateBuf,'\0',sizeof(LszDocTransDateBuf));
		
		OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
					iInicioCampos, iUltimaLinea, iCReqTAB,
					iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
					iDbgFlg, dlg);

		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,' ',sizeof(szPantallaStatusLine));
		szPantallaStatusLine[sizeof(szPantallaStatusLine) - 1] = '\0';
 
		//***Fecha Contable
		iErrorCode = FormateaFecha(szCamposContenido[0],LszDocGLDateBuf);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszDocGLDateBuf(%ls) ...\n"),LszDocGLDateBuf);
		if (iErrorCode != 0) {// Indica que la fecha es nula...
			iCamposErrores[0] = 1; // Seteamos el error...
			jdeFprintf(dlg,_J("***Error(%d): Fecha Contable Invalida (%s)...\n"),iErrorCode,LszDocGLDateBuf);
		}

		//*** Fecha Transaccion	
		iErrorCode = FormateaFecha(szCamposContenido[1],LszDocTransDateBuf);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszDocTransDateBuf(%ls) ...\n"),LszDocTransDateBuf);
		if (iErrorCode != 0) {// Indica que la fecha es nula...
			iCamposErrores[1] = 1; // Seteamos el error...
			jdeFprintf(dlg,_J("***Error(%d): Fecha Transaccion Invalida (%s)...\n"),iErrorCode,LszDocTransDateBuf);
		}

		jdeFflush(dlg);

	} while (iErrorCode != 0);

	//*************************************************************************
	//***Carga en Cache Registros de Orden de Compra                        ***
	//*************************************************************************
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Carga en Cache Registros de Orden Compra...\n"));
	//Abre la tabla F4311
	memset((void *)(&dsF4311Key1),(int)(_J('\0')),sizeof(dsF4311Key1));

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Abriendo Tabla F4311...\n"));
	idResult = JDB_OpenTable (hUser,NID_F4311,ID_F4311_DOC_NO__ORDER_TYPE____1,szF4311ColumnsArray,
	                               (ushort)(21),(JCHAR *)NULL,&hRequestF4311);
	if (idResult == JDEDB_FAILED){
		iErrorCode = 209;
		jdeFprintf(dlg,_J("***Error(%d): JDB_OpenTable (F4311) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN0;
	}

	//Construye Where del select...
	JDB_ClearSelection(hRequestF4311);

	//***Se seleccionan registros que cumplan con:
	ZeroMathNumeric(&mnTemp0Buf);  
	jdeNIDcpy(lpSelect[3].Item1.szDict, NID_UORG);//Units Transaction Quantity > 0
	jdeNIDcpy(lpSelect[3].Item1.szTable, NID_F4311);
	lpSelect[3].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelect[3].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelect[3].Item2.szTable, _J(""));
	lpSelect[3].Item2.idInstance = (ID)0;
	lpSelect[3].lpValue = (void *)&mnTemp0Buf;
	lpSelect[3].nValues = (short)1;
	lpSelect[3].nAndOr = JDEDB_ANDOR_AND;
	lpSelect[3].nCmp = JDEDB_CMP_GT;

	jdeNIDcpy(lpSelect[4].Item1.szDict, NID_UOPN);//Units Open Quantity > 0
	jdeNIDcpy(lpSelect[4].Item1.szTable, NID_F4311);
	lpSelect[4].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelect[4].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelect[4].Item2.szTable, _J(""));
	lpSelect[4].Item2.idInstance = (ID)0;
	lpSelect[4].lpValue = (void *)&mnTemp0Buf;
	lpSelect[4].nValues = (short)1;
	lpSelect[4].nAndOr = JDEDB_ANDOR_AND;
	lpSelect[4].nCmp = JDEDB_CMP_GT;

	//Amazing Global Vzla - WOVALLES - 26/07/2012 - Inicio
	//... Se Quita la Busqueda por MCU ya que en BSN se Obtiene de la Orden de Compra.
	//jdeNIDcpy(lpSelect[5].Item1.szDict, NID_MCU);//BranchPlant == LszDocBranchPlantBuf
	//jdeNIDcpy(lpSelect[5].Item1.szTable, NID_F4311);
	//lpSelect[5].Item1.idInstance = (ID)0;
	//jdeNIDcpy(lpSelect[5].Item2.szDict, _J(""));
	//jdeNIDcpy(lpSelect[5].Item2.szTable, _J(""));
	//lpSelect[5].Item2.idInstance = (ID)0;
	//lpSelect[5].lpValue = LszDocBranchPlantBuf;
	//lpSelect[5].nValues = (short)1;
	//lpSelect[5].nAndOr = JDEDB_ANDOR_AND;
	//lpSelect[5].nCmp = JDEDB_CMP_EQ;
	//Amazing Global Vzla - WOVALLES - 26/07/2012 - Fin

	//***
	iUnicaVez = 0;// Preparamos la presentación repetida de OCs...
	iOCindex2 = 0;// Contador de OCs para la segunda vez...

	do{ //while (TRUE);

		//iUnicaVez = 0;// Preparamos la presentación repetida de OCs...

		OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 1ra pantalla para mostrar la 2da...

		if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Inicializando 2da pantalla datos de Encabezado...\n"));
			jdeFflush(dlg);
		}

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
		memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

		jdeErrorClearEx(lpBhvrCom,lpVoid); 
		jdeErrorClearEx(lpBhvrCom1,lpVoid1); 

		iPrimeraVez = 0;

		strcpy (szPantallaTitulo, "Recepcion Ordenes Compra-Encabezado");

		szCamposEtiquetas[0] = "Compania       ";
		szCamposEtiquetas[1] = "Ubicacion      ";
		szCamposEtiquetas[2] = "Tipo Documento ";
		szCamposEtiquetas[3] = "Nro.Documento  ";

		iCamposCantidad = 4;//Cuatro campos a desplegar, por ahora...

		//***Allocate and set data structures
		memset((void *) &dsF4312BeginDoc,(int) _J('\0'),sizeof(DSDF4312Z1A));	
		memset((void *) &dsF4312EditLine,(int) _J('\0'),sizeof(DSDF4312Z1B));
		memset((void *) &dsF4312EndDoc,(int) _J('\0'),sizeof(DSDF4312Z1C));
		memset((void *) &dsF4311ClearWorkFiles,(int) _J('\0'),sizeof(DSDF4311Z1E));	
		memset((void *) &dsF4312ClearWorkFile,(int) _J('\0'),sizeof(DSDF4312Z1D));	
		memset((void *) &dsDecimalsTriggerGetbyCOCRCD,(int) _J('\0'),sizeof(DSD1100007));

		iOCindex = 0; //seteamos el índice de OCs a 0...

		memset((void *) &dsF4311Arr,(int) _J('\0'),sizeof(stF4311rc01)*CacheSize);
		memset((void *) &dsVarBuf,(int) _J('\0'),sizeof(stVarBuf)*DocQty);
		memset((void *) &dsF4312BeginDocArr,(int) _J('\0'),sizeof(DSDF4312Z1A)*DocQty);
		memset((void *) &dsF4312EndDocArr,(int) _J('\0'),sizeof(DSDF4312Z1C)*DocQty);

		// Se Utilizan los Indices por Campo para controlarlos mas Abajo
		iIdxCo = 0; iIdxLoc = 1; iIdxDT = 2; iIdxDoc = 3;
		iRecordsF4311Read = 0;
		
		do {  //while (iRecordsF4311Read == 0); Loop para obtener una Orden correcta ó muchas OCs...
			iErrorCode = 0;
			jdeErrorClearEx(lpBhvrCom,lpVoid); 
			jdeErrorClearEx(lpBhvrCom1,lpVoid1); 
			if (iUnicaVez == 0){ //Todo esto es para la primera vez...
				//***Compañia
				jdeStrcpy(LszDocumentCompanyBuf,dsRetrieveCompanyFromBusUnit.szCompany);
				jdeFromUnicode(szCamposContenido[iIdxCo],LszDocumentCompanyBuf,6,UTF8); // Inicialmente vale la compañía asociada al usuario...
				iCamposPosiciones[iIdxCo] = strlen(szCamposContenido[iIdxCo]);

				//*** Ubicación por defecto
				jdeFromUnicode(szCamposContenido[iIdxLoc],LszUbicacionDfltBuf,21,UTF8); // inicialmente vale la ubicación por degault...
				iCamposPosiciones[iIdxLoc] = strlen(szCamposContenido[iIdxLoc]);

				//***Tipo Documento
				jdeFromUnicode(szCamposContenido[iIdxDT],LszPOOrderTypeBuf,3,UTF8);
				iCamposPosiciones[iIdxDT] = strlen(szCamposContenido[iIdxDT]);

				//***Nro de Documento
				iDocumentNumberBuf = 0;   //Inizializar Nro de Doc Y Vble a Utilizar en el Select
				ZeroMathNumeric(&mnTemp0Buf);

				//***Allocate and set data structures
				lpValidateUDC = &dsValidateUDC;	
			}

			do { //Loop para obtener una  compañía, ubicacion y tipo doc correcto...
				jdeErrorClearEx(lpBhvrCom,lpVoid); 
				iErrorCode = 0;
				iSalir = 0;

				if (iUnicaVez == 0 || iUnicaVez == 1){
					
					//memset(LszUbicacionDfltBuf,'\0',sizeof(LszUbicacionDfltBuf));
					memset(LszString01,'\0',sizeof(LszString01));
					memset(LszLocationBuf,'\0',sizeof(LszLocationBuf));
					memset (lpValidateUDC,(int)_J('\0'),sizeof(dsValidateUDC));
					jdeStrcpy(LszLocationBuf,_J(""));
									
					iDocumentNumberBuf = 0;   //Inizializar Nro de Doc 
			
					//Muestra Pantalla solicitud de Campos para Compañía, , Tipo y Nro de Orden...
					int iCamposOffset1 = (iUnicaVez == 0) ? iCamposOffset : (iCamposOffset + 3);
					OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset1, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
							iInicioCampos, iUltimaLinea, iCReqTAB,
							iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
							iDbgFlg, dlg);

					memset(iCamposErrores,'\0',sizeof(iCamposErrores));
					memset(szPantallaStatusLine,' ',sizeof(szPantallaStatusLine));
					szPantallaStatusLine[sizeof(szPantallaStatusLine) - 1] = '\0';

					if (iUnicaVez == 0){//Todo esto es para la primera vez...
						//***Compañia para Documento...
						jdeToUnicode(LszDocumentCompanyBuf,szCamposContenido[iIdxCo],6,UTF8);
						LszDocumentCompanyBuf[5] = _J('\0');
						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszDocumentCompanyBuf (%ls)...\n"),LszDocumentCompanyBuf);	
						jdeFflush(dlg);
						
						//Validar la Compañia Ingresada  
						jdeStrcpy(dsF0010RetrieveCompanyConstant.szCompany,LszDocumentCompanyBuf);
						dsF0010RetrieveCompanyConstant.cSuppressErrorMessage = _J('1');
						idResult = jdeCallObject(_J("F0010RetrieveCompanyConstant"),NULL,lpBhvrCom,lpVoid,
									(LPVOID)&dsF0010RetrieveCompanyConstant,(CALLMAP *)NULL,(int)0, 
									(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
						if ((idResult == ER_ERROR) || (jdeStrcmp(dsF0010RetrieveCompanyConstant.szName,_J("")) == 0)){ //No devuelve error...
							iErrorCode = 208;
							iCamposErrores[iIdxCo] = 1; // Seteamos el error...
							jdeFprintf(dlg,_J("***Error(%d): F0010RetrieveCompanyConstant: (%ls)...\n"), iErrorCode,dsF0010RetrieveCompanyConstant.szErrorMessageID );
							jdeFflush(dlg);
						}
						else{
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***F0010RetrieveCompanyConstant %ls:(%lc/%ls)...\n"), 
										dsF0010RetrieveCompanyConstant.szCompany,
										dsF0010RetrieveCompanyConstant.cCurrencyConverYNAR,
										dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);
								jdeFflush(dlg);
							}
						}

						//*** Ubicacion donde se Registrara la Recepción
						jdeToUnicode(LszLocationBuf,szCamposContenido[iIdxLoc],21,UTF8);//Se copia para validar la Ubicacion Ingresada...
						LszLocationBuf[20] = _J('\0');
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***LszLocationBuf (%ls)...\n"),LszLocationBuf);
							jdeFflush(dlg);
						}
						jdeStrcpy(LszLineLocationBuf,LszLocationBuf); //Ahora no le damos chance de escoger una ubicación paea cada línea...
				
						// Se Valida la Ubicacion Ingresada
						//***Solicita y valida localidad/ubicación Por Defecto del Usuario
						//Index = 3, Keys 2 means fetch by BranchPlant and Location.
						ParseNumericString(&dsVerifyAndGetItemLocation.mnIndex,_J("3"));
						ParseNumericString(&dsVerifyAndGetItemLocation.mnKeys,_J("2")); 
						dsVerifyAndGetItemLocation.cReturnRecord  = _J('0'); //Solo validar, no queremos data...
						dsVerifyAndGetItemLocation.cCallType = _J('1'); //Devolver Error si no hay registro...
						jdeStrcpy(dsVerifyAndGetItemLocation.szBranchPlant,LszDocBranchPlantBuf);
						jdeStrcpy(dsVerifyAndGetItemLocation.szLocation,LszLocationBuf);
						jdeCallObject(_J("VerifyAndGetItemLocation"), NULL, lpBhvrCom, lpVoid,
									(LPVOID)&dsVerifyAndGetItemLocation,(CALLMAP *)NULL,(int)0, 
									(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
						jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){
							iErrorCode = 206;
							iCamposErrores[iIdxLoc] = 1; // Seteamos el error...
							jdeFprintf(dlg,_J("***Error(%d): VerifyAndGetItemLocation: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
							jdeFflush(dlg);
						}

						//***Tipo Documento 

						i = 0;
						while (szCamposContenido[iIdxDT][i]){
							szCamposContenido[iIdxDT][i] = toupper(szCamposContenido[iIdxDT][i]);
							i++;
						}

						jdeToUnicode(LszDocumentTypeBuf,szCamposContenido[iIdxDT],3,UTF8);
						LszDocumentTypeBuf[2]=_J('\0');
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***LszDocumentTypeBuf (%ls)...\n"),LszDocumentTypeBuf);	
							jdeFflush(dlg);
						}

						// Valida el Tipo de Documento
						jdeStrcpy(lpValidateUDC->SY,_J("00"));
						jdeStrcpy(lpValidateUDC->RT,_J("DT"));
						jdeStrcpy(lpValidateUDC->KY,LszDocumentTypeBuf);
						if (jdeValidateUDC(NULL, lpValidateUDC) == FALSE){  //Invalid UDC!!!						
							iErrorCode = 210;
							iCamposErrores[iIdxDT] = 1; // Seteamos el error...
							jdeFprintf(dlg,_J("***Error(%d): Description: jdeValidateUDC (%s)...\n"),iErrorCode,
											LszDocumentTypeBuf);
							jdeFflush(dlg);
						}else{

							jdeToUnicode(LszDocumentTypeBuf,szCamposContenido[iIdxDT],3,UTF8);
							LszDocumentTypeBuf[2]=_J('\0');
						}
					}

					iIdxDoc = (iUnicaVez == 0)? 3 : 0;//Las siguientes veces la pantalla es mas pequeña...

					//***Numero de Documento
					if (strlen(szCamposContenido[iIdxDoc]) == 0) {
						iSalir = 1;
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***Break de Nro. de Documento...\n"));
							jdeFflush(dlg);
						}
						break; // Nos vamos!!!...
					}else {
						iSalir = 0;
						sscanf(szCamposContenido[iIdxDoc],"%d",&iDocumentNumberBuf);
						IntToMathNumeric(iDocumentNumberBuf,&mnTempBuf);
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***iDocumentNumberBuf (%d)...\n"),iDocumentNumberBuf);
							jdeFflush(dlg);
						}
						j = 0;
						for(i=0;((i <= iOCindex) && (j == 0));i++){//Verificamos que la OC no este repetida...
							if (iDbgFlg > 0) {
								jdeToUnicode(LszString01,mnTempBuf.String,DIM(LszString01),UTF8);
								jdeToUnicode(LszString02,dsVarBuf[i].pddoco.String,DIM(LszString02),UTF8);
								jdeFprintf(dlg,_J("***Verificando OC (%ls/%ls)...\n"),LszString01,LszString02);
								jdeFflush(dlg);
							}
							if (MathCompare(&mnTempBuf,&dsVarBuf[i].pddoco) == 0) j = 1;
						}
						if (j == 1){
							iErrorCode = 226;
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***Warn(%d): OC repetida...\n"),iErrorCode);
								jdeFflush(dlg);
							}
							iPrimeraVez = 0;
							memset(szCamposContenido,'\0',sizeof(szCamposContenido));
							strcpy(szCamposContenido[iIdxDoc],"");
							iCamposPosiciones[iIdxDoc] = strlen(szCamposContenido[iIdxDoc]);
							continue;//Repetido.. pedir otro...
						}
					}

				} //if (iUnicaVez == 0 || iUnicaVez == 1)
			
			} while (iErrorCode != 0);

			if (iUnicaVez == 2) {
				MathNumericToInt(&dsVarBuf2[iOCindex2].pddoco, &iDocumentNumberBuf);
				IntToMathNumeric(iDocumentNumberBuf,&mnTempBuf);  
				if (iDbgFlg > 0) {
						jdeFprintf(dlg,_J("***iDocumentNumberBuf 2!!! (%d)...\n"),iDocumentNumberBuf);
						jdeFflush(dlg);
					}
				iOCindex2++;//Siguiente OC guardada...
			}

			if ((iDocumentNumberBuf == 0) && (iOCindex == 0)) goto lbFIN; //No hay Orden de Compra->FIN!!!
			if ((iDocumentNumberBuf == 0) && (iOCindex > 0)) break; //Ya no mas OCs.. ahora a capturar items...

			//Actualiza la clausula Where con Document(OrderNo, Invoice, etc)...
			jdeNIDcpy(lpSelect[0].Item1.szDict, NID_DOCO);//Document(OrderNo, Invoice, etc) == mnTempBuf
			jdeNIDcpy(lpSelect[0].Item1.szTable, NID_F4311);
			lpSelect[0].Item1.idInstance = (ID)0;
			jdeNIDcpy(lpSelect[0].Item2.szDict, _J(""));
			jdeNIDcpy(lpSelect[0].Item2.szTable, _J(""));
			lpSelect[0].Item2.idInstance = (ID)0;
			lpSelect[0].lpValue = (void *)&mnTempBuf;
			lpSelect[0].nValues = (short)1;
			lpSelect[0].nAndOr = JDEDB_ANDOR_AND;
			lpSelect[0].nCmp = JDEDB_CMP_EQ;
			
			//Actualiza la clausula Where con Order Type...
			jdeNIDcpy(lpSelect[1].Item1.szDict, NID_DCTO);//Order Type == LszDocumentTypeBuf
			jdeNIDcpy(lpSelect[1].Item1.szTable, NID_F4311);
			lpSelect[1].Item1.idInstance = (ID)0;
			jdeNIDcpy(lpSelect[1].Item2.szDict, _J(""));
			jdeNIDcpy(lpSelect[1].Item2.szTable, _J(""));
			lpSelect[1].Item2.idInstance = (ID)0;
			lpSelect[1].lpValue = (void *)LszDocumentTypeBuf;
			lpSelect[1].nValues = (short)1;
			lpSelect[1].nAndOr = JDEDB_ANDOR_AND;
			lpSelect[1].nCmp = JDEDB_CMP_EQ;

			//Actualiza la clausula Where con Order Company...
			jdeNIDcpy(lpSelect[2].Item1.szDict, NID_KCOO);//Order Company == LszDocumentCompanyBuf
			jdeNIDcpy(lpSelect[2].Item1.szTable, NID_F4311);
			lpSelect[2].Item1.idInstance = (ID)0;
			jdeNIDcpy(lpSelect[2].Item2.szDict, _J(""));
			jdeNIDcpy(lpSelect[2].Item2.szTable, _J(""));
			lpSelect[2].Item2.idInstance = (ID)0;
			lpSelect[2].lpValue = (void *)LszDocumentCompanyBuf;
			lpSelect[2].nValues = (short)1;
			lpSelect[2].nAndOr = JDEDB_ANDOR_AND;
			lpSelect[2].nCmp = JDEDB_CMP_EQ;

			//***
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Construyendo Selection Tabla F4311...\n"));
			//*** Amazing Global Vzla- WOVALLES -INICIO - 26/07/2012 ***
			//*** Se Modifica la Busqueda a Cinco Campos
			//idResult = JDB_SetSelection(hRequest,lpSelect,(short)(6),JDEDB_SET_REPLACE);
			idResult = JDB_SetSelection(hRequestF4311,lpSelect,(short)(5),JDEDB_SET_REPLACE);
			//*** Amazing Global Vzla - Modificado Por: Williams Ovalles(WOVALLES)- FIN - 04/07/2012
			if (idResult == JDEDB_FAILED){
				iErrorCode = 228;
				iCamposErrores[1] = 1; // Seteamos el error...
				jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection (F4311) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}
			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F4311...\n"));
			idResult = JDB_SelectKeyed(hRequestF4311,(ID) 0,(void *)NULL,(short)0);
			if (idResult == JDEDB_FAILED){
				iErrorCode = 211;
				jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed (F4311) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}

			//iRecordsF4311Read = 0;
			i = 0;

			do {//Carga en Cache los regitros de la Orden de Compra escogida...
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla F4311...\n"));		
				idResult = JDB_Fetch(hRequestF4311,&dsF4311Arr[iRecordsF4311Read],(int)0);
				if (idResult == JDEDB_FAILED)jdeFprintf(dlg,_J("***JDB_Fetch (EOF F4311) (%d) ...\n"),iRecordsF4311Read);
					// idResult=1 --> SI Encontro Registro
					// idResult=0 --> NO Encontro Registro
				else {
					if ((jdeStrcmp(dsF4311Arr[iRecordsF4311Read].pdnxtr,LszIncomingStatus1Buf) == 0) ||//El registro status OK...
						(jdeStrcmp(dsF4311Arr[iRecordsF4311Read].pdnxtr,LszIncomingStatus2Buf) == 0) ||
						(jdeStrcmp(dsF4311Arr[iRecordsF4311Read].pdnxtr,LszIncomingStatus3Buf) == 0)){
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***%d)Producto (%ls-%ls)...\n"),iRecordsF4311Read,
									dsF4311Arr[iRecordsF4311Read].pdaitm,dsF4311Arr[iRecordsF4311Read].pddsc1);

							jdeToUnicode(LszString01,dsF4311Arr[iRecordsF4311Read].pddoco.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Order Document(OrderNo, Invoice, etc)(%ls)...\n"),iRecordsF4311Read,
									LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iRecordsF4311Read,
									dsF4311Arr[iRecordsF4311Read].pddcto);
							jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iRecordsF4311Read,
									dsF4311Arr[iRecordsF4311Read].pdkcoo);
							jdeStrcpy(LszDocBranchPlantBuf,dsF4311Arr[iRecordsF4311Read].pdmcu); // El almacén es el de la OC...
							jdeFprintf(dlg,_J("***Cache[%d] Order Branch Plant(%ls)...\n"),iRecordsF4311Read,
									dsF4311Arr[iRecordsF4311Read].pdmcu);

							jdeToUnicode(LszString01,dsF4311Arr[iRecordsF4311Read].pdlnid.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iRecordsF4311Read,
									LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Ubicacion(%ls)...\n"),iRecordsF4311Read,
									dsF4311Arr[iRecordsF4311Read].pdlocn);

							jdeToUnicode(LszString01,dsF4311Arr[iRecordsF4311Read].pduorg.String,DIM(LszString01),UTF8);
							jdeToUnicode(LszString02,dsF4311Arr[iRecordsF4311Read].pduopn.String,DIM(LszString02),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Cantidad(%ls/%ls)...\n"),iRecordsF4311Read,
							LszString01, LszString02);
							
							jdeToUnicode(LszString01,dsF4311Arr[iRecordsF4311Read].pdprrc.String,DIM(LszString01),UTF8);
							jdeToUnicode(LszString02,dsF4311Arr[iRecordsF4311Read].pdfrrc.String,DIM(LszString02),UTF8);							
							jdeFprintf(dlg,_J("***Cache[%d] Precio (%ls/%ls/%ls)...\n"),iRecordsF4311Read, 
									LszString01,dsF4311Arr[iRecordsF4311Read].pdcrcd,LszString02);
						}
						dsF4311Arr[iRecordsF4311Read].iStatusRegistro = 0;
						iRecordsF4311Read++; //Una líneas más para el caché...
						i++; // Un registro más para esta OC...
						if (iRecordsF4311Read == CacheSize){
							iErrorCode = 212;
							jdeFprintf(dlg,_J("***Error(%d): iRecordsF4311Read > CacheSize!!!\n"),iErrorCode);
							jdeFflush(dlg);
							goto lbFIN;
						}else 
							iRecordsAvailCache = iRecordsF4311Read;
					}
				}
			} while (idResult != JDEDB_FAILED);

			if (i == 0){  //No encontramos nada!!!						
				iErrorCode = 224;
				iCamposErrores[iIdxDoc] = 1; // Seteamos el error...
				jdeFprintf(dlg,_J("***Error(%d): No se encontraron registros para esa OC (%d)...\n"),iErrorCode,
								iDocumentNumberBuf);
				jdeFflush(dlg);
			}
			else{//Tenemos una OC adicional...
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Si se encontraron registros para esa OC (%d)...\n"),iDocumentNumberBuf);
					jdeFflush(dlg);
				}

				j = iRecordsF4311Read - 1;
				IntToMathNumeric(iDocumentNumberBuf,&dsVarBuf[iOCindex].pddoco);
				dsVarBuf[iOCindex].iRecordsAvailCache = iRecordsF4311Read;
				dsVarBuf[iOCindex].iEditLineLines = 0;
				ZeroMathNumeric(&dsVarBuf[iOCindex].mnF4111JobNumber);
				ZeroMathNumeric(&dsVarBuf[iOCindex].mnF0911JobNumber);
				jdeStrcpy(dsVarBuf[iOCindex].szTransactionCurrency,dsF4311Arr[j].pdcrcd);
				iOCindex++;//Siguiente OC!!!

				//Reformateamos la pantalla para solo manejar el Nro.Documento...

				if (iUnicaVez == 0){// Estamos  a punto de dejar la pirmar vez: las siguientes OCs...

					iUnicaVez = 1;

					memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
					memset(szCamposContenido,'\0',sizeof(szCamposContenido));
					memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
					memset(iCamposErrores,'\0',sizeof(iCamposErrores));
					memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
					memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

					iPrimeraVez = 0;			

					strcpy (szPantallaTitulo, "Recepcion Ordenes Compra-Encabezado");

					szCamposEtiquetas[0] = "Nro.Documento  ";

					strcpy(szCamposContenido[0],"");
					iCamposPosiciones[0] = strlen(szCamposContenido[0]);

					iCamposCantidad = 1;
				};
				if (iUnicaVez == 1){
					iPrimeraVez = 0;
					memset(szCamposContenido,'\0',sizeof(szCamposContenido));
					strcpy(szCamposContenido[0],"");
					iCamposPosiciones[0] = strlen(szCamposContenido[0]);
				};

			}
                                                                                                                                                                                                                              			jdeFflush(dlg);
		} while ((iRecordsF4311Read == 0) //No se pudo leer ningún registro con esa Orden de Compra->Solicitar otro...
			|| (iDocumentNumberBuf > 0));//Se introdujo una OC y queremos más...
		
		jdeFflush(dlg);

		if ((iDocumentNumberBuf == 0) && (iOCindex == 0)) goto lbFIN; //No hay Orden de Compra->FIN!!!

		//*************************************************************************
		//***Procesa F4312BeginDoc                                              ***
		//*************************************************************************
		//Asignaciones para F4312BeginDoc:
		//· mnJobNumber															ok ""
		//· szComputerIhD			Asignado por la aplicación					ok GetAuditInfo
		//· cActionCode															ok 'A'
		//· cProcessEdits														ok '1'
		//· szProgramID															ok 'EP4312'
		//· cCurrencyProcessingFlag												ok F0010RetrieveCompanyConstant
		//· mnOrderNumber			Introducido por el operador					ok
		//· szOrderType				Introducido por el operador					ok INI
		//· szOrderCompany			Introducido por el operador					ok RetrieveCompanyFromBusUnit
		//· szOrderSuffix														ok '000'
		//· cCurrencyMode			Salida de la función (Header PO)			--
		//· mnReceiptNumber														-- No se está asignando
		//· mnSupplier				Salida de la fución							--
		//· szBaseCurrency			Salida de la función						--
		//· szPOCurrencyCode		Salida de la fubción						--
		//· mnExchangeRate			Salida de la función						--
		//· jdReceiptDate			Introducido por el operador					ok INI
		//· jdGLDate				Introducido por el operador					ok INI
		//· szHoldCode				Salida de la función						--
		//· szUserID				Asignado por la aplicación					ok GetAuditInfo
		//· szPOVersion															ok INI
		//· mnBatchNumber														ok '0'
		//· cBatchStatus														¡!
		//· cPostOutOfBalance													¡!
		//· jdBatchDate															ok ''
		//· cOption					No Asignado									ok INI 1 o 7
		//· cReceiptsByPO			No Asignado									-- '1' 
		//· CCalledFromEDI														--
		//· mnTriangulationRateFromCurrec										¡!
		//· mnTriangulationRateToCurrecncy										¡!
		//· cCurrencyConversionMethod											¡!
		
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***F4312BeginDoc...\n"));	
		for (i=0; i < iOCindex; i++){
			jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);		
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Asignado dsF4312BeginDoc #%d OC-%ls...\n"),i,LszString01);
				jdeFflush(dlg);
			}

			memset((void *) &dsGetPurchaseHeaderColumns,(int)(_J('\0')),sizeof(DSD4300130B));
			MathCopy(&dsGetPurchaseHeaderColumns.mnOrderNumber,&dsVarBuf[i].pddoco);
			jdeStrcpy(dsGetPurchaseHeaderColumns.szOrderType,LszDocumentTypeBuf);
			jdeStrcpy(dsGetPurchaseHeaderColumns.szOrderDocumentCompany,LszDocumentCompanyBuf);
			//jdeStrcpy(dsGetPurchaseHeaderColumns.szOrderSuffix,dsVarBuf[i].pdsfxo);
			idResult = jdeCallObject(_J("GetPurchaseHeaderColumns"),NULL,lpBhvrCom,lpVoid,&dsGetPurchaseHeaderColumns,
					(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
			if (idResult == ER_ERROR){
				iErrorCode = 232;
				jdeFprintf(dlg,_J("***Error(%d): GetPurchaseHeaderColumns:...\n"), iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}

			ZeroMathNumeric(&dsF4312BeginDocArr[i].mnJobNumber);
			jdeStrcpy (dsF4312BeginDocArr[i].szComputerID,dsGetAuditInfo.szWorkstation_UserId);
			dsF4312BeginDocArr[i].cActionCode = _J('A');
			dsF4312BeginDocArr[i].cProcessEdits = _J('1');
			jdeStrcpy(dsF4312BeginDocArr[i].szProgramID,_J("EP4312"));
			//dsF4312BeginDocArr[i].cCurrencyProcessingFlag = dsF0010RetrieveCompanyConstant.cCurrencyConverYNAR;
			dsF4312BeginDocArr[i].cCurrencyProcessingFlag = LcCurrencyFlag;
			MathCopy(&dsF4312BeginDocArr[i].mnOrderNumber,&dsVarBuf[i].pddoco);
			jdeStrcpy(dsF4312BeginDocArr[i].szOrderType,LszDocumentTypeBuf);
			jdeStrcpy(dsF4312BeginDocArr[i].szOrderCompany,LszDocumentCompanyBuf);
			jdeStrcpy(dsF4312BeginDocArr[i].szOrderSuffix,_J("000"));

			//· cCurrencyMode			Salida de la función (Header PO) ver mas abajo... ***Modificado 20/08/2012 - Luis Capriles
			//· mnReceiptNumber	
			//· mnSupplier				Salida de la fución							
			//· szBaseCurrency			Salida de la función... ***Modificado 20/08/2012 - Luis Capriles
			jdeStrcpy (dsF4312BeginDocArr[i].szBaseCurrency,dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);
			//· szPOCurrency			Salida de la fubción
			jdeStrcpy (dsF4312BeginDocArr[i].szPOCurrency,dsVarBuf[i].szTransactionCurrency);
			//· mnExchangeRate			Salida de la función
			MathCopy(&dsF4312BeginDocArr[i].mnExchangeRate,&dsGetPurchaseHeaderColumns.mnExchangeRate);
			if (jdeStrcmp(dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom,dsF4312BeginDocArr[i].szPOCurrency) == 0) 
				dsF4312BeginDocArr[i].cCurrencyMode = _J('D');
			else 
				dsF4312BeginDocArr[i].cCurrencyMode = _J('F');
			DeformatDate(&dsF4312BeginDocArr[i].jdReceiptDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
			DeformatDate(&dsF4312BeginDocArr[i].jdGLDate,LszDocGLDateBuf,(JCHAR*) _J("DSMSE"));
			//· szHoldCode				Salida de la función
			jdeStrcpy (dsF4312BeginDocArr[i].szUserID,dsGetAuditInfo.szUserName);
			jdeStrcpy(dsF4312BeginDocArr[i].szPOVersion,LszP4312VersionBuf);
			ZeroMathNumeric(&dsF4312BeginDocArr[i].mnBatchNumber);
			//· cBatchStatus														
			//· cPostOutOfBalance
			//· jdBatchDate	
			dsF4312BeginDocArr[i].cOption = LcP4312LineOptionBuf;
			dsF4312BeginDocArr[i].cReceiptsByPO = _J('1');
			//· CCalledFromEDI														
			//· mnTriangulationRateFromCurrec										
			//· mnTriangulationRateToCurrecncy										
			//· cCurrencyConversionMethod

			//***F4312BeginDoc  
			idResult = jdeCallObject(_J("F4312BeginDoc"),NULL,lpBhvrCom1,lpVoid1, 
									(void *)&dsF4312BeginDocArr[i],(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
			jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
			while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
				if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )){
					iErrorCode = 213;
					jdeFprintf(dlg,_J("***Error(%d): F4312BeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				}
				else{
					jdeFprintf(dlg,_J("***Warn(%d): F4312BeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				}
			}
			if ((idResult == ER_ERROR) && (iErrorCode == 0)){ 
				iErrorCode = 213;
				jdeFprintf(dlg,_J("***Error(%d): F4312BeginDoc...\n"),iErrorCode);
				jdeFflush(dlg);
			}
			

		}//for... Así, hacemos todos los BeginDocs...
		
		iErrorCode1 = 0;
		if (iErrorCode == 0) { // BeginDoc está OK, seguir con EditLine y EndDoc ...  

			//*************************************************************************
			//***F4312EditLine                                                      ***
			//*************************************************************************
			//Asignaciones para F4312EditLine:
			//· mnJobNumber			.											ok F4312BeginDoc
			//· szComputerID		.											ok  GetAuditInfo
			//· cActionCode			.											ok '1'
			//· cProcessEdits													ok '1'
			//· szProgramID			.											ok EP4312
			//· cCurrencyProcessingFlag											ok F4312BeginDoc
			//· mnOderNumber		Introducido por el Operador					ok F4312BeginDoc
			//· szOrderType			Introducido por el Operador					ok F4312BeginDoc
			//· szOrderKeyCompany	Introducido por el Operador					ok F4312BeginDoc	
			//· szOrderSuffix													ok Cache dsF4311Arr
			//· mnLineNumber													ok Cache dsF4311Arr
			//· mnReceiptLineNumber												¡!
			//· cReceiptRecWritten												ok ''
			//· cOption															ok 1 o 7
			//· szPOVersion														ok INI
			//· mnQuantityReceived	Introducido por el Operador					ok
			//· szReceiptUOM		Introducido por el Operador					ok
			//· mnUnitCostReceived	Introducido por el Operador					ok
			//· mnForeignUnitCostReceived										¡!
			//· mnAmountReceived												¡!
			//· mnForeignAmountReceived											!!
			//· szReceiptBranch		Introducido por el Operador					ok GetDefaultBranch
			//· szLocationDatabase	Introducido por el Usuario					ok
			//· szLotNumber														--
			//· cLotStatus														--
			//· mnLotPotency													--
			//· szLotGrade														--
			//· jdLotExpirationDate												--
			//· szLotRetention													--
			//· szSupplierLot													--
			//· szAssetID														--
			//· szLandedCostRule												--
			//· szReasonCode													¡! INI
			//· szContainerID													--
			//· szVendorRemark													--
			//· szSubledger														--
			//· cSubledgerType													--
			//· jdGLDate			Introducido por el operador					ok INI
			//· jdReceiptDate		Introducido por el operador					ok INI
			//· szDomesticCurrencyCode											ok F4312BeginDocument
			//· szPOCurrencyCode												ok F4312BeginDocument
			//· mnExchangeRate													ok F4312BeginDocument
			//· cCurrencyMode													ok F4312BeginDocument
			//· cFirstOfMultiLine												--
			//· cLandedCostCode													--
			//· mnBatchNumber													ok F4312BeginDocument
			//· szBatchType														¡!
			//· jdBatchDate														ok INI
			//· mnReceiptDoc													¡!
			//· mnSupplier														--
			//· szSupplierName													--
			//· cInventoryInterface												ok 'Y'
			//· cGLInterface													ok 'Y'
			//· cTextLine														¡!
			//· szQuantityBucketToUpdate										¡!
			//· cReceiptAcknowledgement											--
			//· cPaymentEligible												--
			//· mnF4111JobNumber												ok 0
			//· mnF0911JobNumber												ok 0
			//· cF4111Written													--
			//· mnQtyCompleted													--
			//· mnQtyScrapped													--
			//· szP3103UOM														--
			//· szOperationStatus												--
			//· mnLastJELine													¡!
			//· szGLRemark				No Asignado								--
			//· szInventoryTransRemark	No Asignado								--
			//· cCalledFromRouting		No Asignado								--
			//· mnF4311JonbNumber												¡! OJO
			//· cCalledFromEDI			No Asignado								--
			//· mnWMSLineNumber													ok 0
			//· mnLandedCJobNo													¡!
			//· szMemoLotField1													--
			//· szMemoLotField2													--
			//· mnTriangulationRateFromCurren									¡!
			//· mnTriangulationRateToCurrency									¡!
			//· cCurrencyConversionMethod										¡!
			//· mnAmbientVolume													--
			//· szAmbientVolumeUOM												--
			//· mnStandardVolume												--
			//· szStandardVolumeUOM												--
			//· mnWeightQuantity												--
			//· mnWeightQuantityUOM												--
			//· mnTemperature													--
			//· cTemperatureType												--
			//· cDensityType													--
			//· mnDensityTemperature											--
			//· cDensityTemperatureType											--
			//· mnVolumeCorrectionFcator										--
			//· cProgramStatus													¡!
			//· mnTransQtyInPrimaryForBulk										--
			//· szPrimaryUOMForBulk												--
			//· mnDisplayDensity												--
			//· mnTransactionTime												--
			//· cManagerialAnalysisType1										--
			//· szManagerialAnalysisCode1										--
			//· cManagerialAnalysisType2										--
			//· szManagerialAnalysisCode2										--
			//· cManagerialAnalysisType3										--
			//· szManagerialAnalysisCode3										--
			//· cManagerialAnalysisType4										--
			//· szManagerialAnalysisCode4										--
			//· szAgreementNumber												--
			//· mnAgreementSupplement											--
			//· cBatchStatus													¡!
			//· cPostOutOfBalance												¡!
			//· mnWareHouseResSeq												--
			//· mnF43092Jobnumber		No Asignado								--
			//· mnShipmentNumber												¡!
			//· mnTransportationJobNumber	

			iEditLineLines = 0; // No hay líneas en el documento de salida...

			ZeroMathNumeric(&mnF4111JobNumberBuf);
			ZeroMathNumeric(&mnF0911JobNumberBuf);

			OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 2ra pantalla para mostrar la 2,5...
			
			do { //while (iRecordsAvailCache > 0); Procesar mientras haya registros en el Cache...

				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Inicializando 2,5 pantalla para la etiqueta de codigo de barras...\n"));
					jdeFflush(dlg);
				}
				memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
				memset(szCamposContenido,'\0',sizeof(szCamposContenido));
				memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
				memset(iCamposErrores,'\0',sizeof(iCamposErrores));
				memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
				memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

				do {
					iErrorCode = 0;

					iPrimeraVez = 0;			
					strcpy (szPantallaTitulo, "Recepcion Ordenes Compra-Detalle    ");
					szCamposEtiquetas[0] = "Codigo Barra  ";
					strcpy(szCamposContenido[0],"");
					iCamposPosiciones[0] = strlen(szCamposContenido[0]);
					iCamposCantidad = 1;
					memset(szCamposContenido,'\0',sizeof(szCamposContenido));
					strcpy(szCamposContenido[0],"");
					iCamposPosiciones[0] = strlen(szCamposContenido[0]);

					OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
							iInicioCampos, iUltimaLinea, iCReqTAB,
							iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
							iDbgFlg, dlg);

					memset(iCamposErrores,'\0',sizeof(iCamposErrores));
					memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));

					//***Codigo Etiqueta:(Producto/lote/etc...) 
					//01=Codigo producto
					//10=Lote
					//17=Fecha vencimiento
					if (strlen(szCamposContenido[0]) == 0) {
							iSalir = 1;
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***Break de Etiqueta Larga...\n"));
								jdeFflush(dlg);
							}
							break;
					} 
					else iSalir = 0;

					jdeToUnicode(LszTempBuf,szCamposContenido[0],sizeof(LszTempBuf)-1,UTF8);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Etiqueta (%ls)...\n"),LszTempBuf);

					// Se Utilizan los Indices por Campo para controlarlos mas Abajo
					iIdxProd = 0; iIdxCant = 2; iIdxLot = 3; iIdxFVcto = 4;
					strcpy(szTempBuf,szCamposContenido[0]);

					//Si la etiqueta está defectuosa, ó no se encuentra el CB ó no está en el cache->error..
					iErrorCode1 = ProcesaCB(szTempBuf,szCamposContenido[iIdxProd],szCamposContenido[iIdxLot],szCamposContenido[iIdxFVcto],
											iUPClenBuf,iSCClenBuf,iDbgFlg,dlg);
			
					if (iErrorCode1 == 1){
						iCamposErrores[0] = 1; // Seteamos el error...
						iErrorCode = 227;
						jdeToUnicode(LszTempBuf,szTempBuf,sizeof(LszTempBuf)-1,UTF8);
						jdeFprintf(dlg,_J("***Error(%d): Etiqueta Invalida: %ls...\n"),iErrorCode,LszTempBuf);
						jdeFflush(dlg);
					}
					else{
						memset(LszLineItemBuf,'\0',sizeof(LszLineItemBuf));
						memset(LszUOMdefaultBuf,'\0',sizeof(LszUOMdefaultBuf));
						memset(LszUOMstdConv,'\0',sizeof(LszUOMstdConv));
						memset(LszItemDescriptionBuf,'\0',sizeof(LszItemDescriptionBuf));
						ZeroMathNumeric(&mnItemShortIDBuf);
						memset(LszUsrEntry,'\0',sizeof(LszUsrEntry));
						jdeToUnicode(LszUsrEntry,szCamposContenido[iIdxProd],25,UTF8);
						if ((iErrorCode = OWDCgbc(hEnv,hUser,LszUsrEntry,LszLineItemBuf,&mnItemShortIDBuf,
												LszUOMdefaultBuf,LszUOMstdConv,LszCrossRefTypeCodeBuf,iUPClenBuf,iSCClenBuf,
												LszItemDescriptionBuf,iGetItemMasterBy,dsGetCrossReferenceFields.c3rdItemNoSymbol,
												iDbgFlg,dlg)) != 0) iCamposErrores[iIdxProd] = 1; //Seteamos el error... Codigo malo

						if (iErrorCode != 0){ //Hubo error... vamos a quitar ceros para ver...
							iCamposErrores[iIdxProd] = 1;
							iErrorCode = 0;

							const char* firstNonSpace = szCamposContenido[iIdxProd];
							while(*firstNonSpace != '\0' && (isspace(*firstNonSpace) || firstNonSpace[0] == '0')){
								++firstNonSpace;
							}
										
							strcpy(szCamposContenido[iIdxProd],firstNonSpace);
							jdeToUnicode(LszUsrEntry,szCamposContenido[iIdxProd],25,UTF8);

							if ((iErrorCode = OWDCgbc(hEnv,hUser,LszUsrEntry,LszLineItemBuf,&mnItemShortIDBuf,
													LszUOMdefaultBuf,LszUOMstdConv,LszCrossRefTypeCodeBuf,iUPClenBuf,iSCClenBuf,
													LszItemDescriptionBuf,iGetItemMasterBy,dsGetCrossReferenceFields.c3rdItemNoSymbol,
													iDbgFlg,dlg)) != 0) iCamposErrores[iIdxProd] = 1; //Seteamos el error... Codigo malo

							if (iDbgFlg > 0) {
								jdeFprintf(dlg,_J("***LszLineItemBuf 2(%ls)...\n"),LszLineItemBuf);
								jdeFprintf(dlg,_J("***LszUOMdefaultBuf 2(%ls)...\n"),LszUOMdefaultBuf);
								jdeToUnicode(LszString01,mnItemShortIDBuf.String,DIM(LszString01),UTF8);
								jdeFprintf(dlg,_J("***mnItemShortIDBuf 2(%ls)...\n"),LszString01);
								jdeFflush(dlg);
							}
						}
						if (iErrorCode == 0){
							//Armamos la descripción y el NART...
							iIdxDescr = 1;
							jdeFromUnicode(szCamposContenido[iIdxDescr],LszItemDescriptionBuf,20,UTF8);
							szCamposContenido[iIdxDescr][20] = '/';

							memset((void *) &dsGetItemMasterByShortItem,(int) _J('\0'),sizeof(DSDX4101B));
							MathCopy(&dsGetItemMasterByShortItem.mnShortItemNumber,&mnItemShortIDBuf);
							dsGetItemMasterByShortItem.cReturnPtr = _J('1');
							dsGetItemMasterByShortItem.cSuppressErrorMsg = _J('1');
							idResult = jdeCallObject (_J("GetItemMasterByShortItem"), NULL,lpBhvrCom,lpVoid,
														(LPVOID)&dsGetItemMasterByShortItem,(CALLMAP*)NULL,(int)0,
														(JCHAR*)NULL,(JCHAR*)NULL,(int)0);
							if (idResult == ER_ERROR){
								iErrorCode = 235;
								jdeFprintf(dlg,_J("***Error(%d): GetItemMasterByShortItem:...\n"), iErrorCode);
								jdeFflush(dlg);
								goto lbFIN;
							} 
							lpdsF4101 = (LPF4101)jdeRemoveDataPtr(hUser,(unsigned long)dsGetItemMasterByShortItem.idF4101LongRowPtr);
							jdeFromUnicode(szCamposContenido[iIdxDescr] + 21,lpdsF4101->imdraw,21,UTF8);//DRAW: NART...
							if (lpdsF4101 != (LPF4101) NULL){
								jdeFree((LPVOID)lpdsF4101);
								lpdsF4101 = (LPF4101)NULL;
							}
						}
					}

					iItemFoundInCache = 0;
					iError1 = 1; 

					for (i = 0;((i < iRecordsF4311Read) && (iItemFoundInCache == 0) && (iErrorCode == 0)); i++){
						iError1 = 1; 

						if ((jdeStrcmp((JCHAR *)mnItemShortIDBuf.String,(JCHAR *)dsF4311Arr[i].pditm.String) == 0) && 
							(dsF4311Arr[i].iStatusRegistro != 2)) iError1 = 0; //Mostrar registros no procesados

						if (iError1 == 0){ //Mostrar registro seleccionado...
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***dsF4311Arr[%d].pdaitm (%ls)-Primer Paso...\n"),i,dsF4311Arr[i].pdaitm);
								jdeFflush(dlg);
							}
							iItemFoundInCache++;
						}
					}
					if (iError1 == 1) { //No se encontró en el cache
						iCamposErrores[iIdxProd] = 1; //Seteamos el error... Codigo malo
					}

					if (iItemFoundInCache == 0) { //No se encontró en el cache
						iErrorCode = 223;		
						jdeFprintf(dlg,_J("***Error(%d): No se encontró en el cache-Primer Paso...\n"),iErrorCode);
						jdeFflush(dlg);
					}

				} while (iErrorCode != 0);

				if (iSalir > 0){
					if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***Break de Etiqueta Larga 2...\n"));
							jdeFflush(dlg);
						};
					break; //Queremos subir a perdir otro OC!!!!
				}

				memset((void *) &dsF4312EditLine,(int) _J('\0'),sizeof(DSDF4312Z1B));
				ZeroMathNumeric(&dsF4312EditLine.mnF4111JobNumber);
				ZeroMathNumeric(&dsF4312EditLine.mnF0911JobNumber);
				ZeroMathNumeric(&dsF4312EditLine.mnLandedCJobNo);
				ZeroMathNumeric(&dsF4312EditLine.mnLastJELine);


				/* Modificado por: Williams Ovalles - Amazing Global - 06/07/2012 - INICIO */
				/* MANEJO DE PANTALLA Para solicitar los Datos de Entrada Por Item         */
				//*************************************************************************
				//***Comienza Procesamiento del Detalle                                 ***
				//*************************************************************************
				//*** NO Limpiamos la pantalla para poder Vizualizar la Orden Procesada en Pantalla
				//OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 2da pantalla para mostrar la 3era...

				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Inicializando 3era pantalla Datos Detalle...\n"));
					jdeFflush(dlg);
				}

				memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
				//memset(szCamposContenido,'\0',sizeof(szCamposContenido));
				memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
				memset(iCamposErrores,'\0',sizeof(iCamposErrores));
				memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
				memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));
				
				//iPrimeraVez = 0;
				
				strcpy (szPantallaTitulo, "Recepcion Ordenes Compra-Detalle    ");

				szCamposEtiquetas[0] = "Producto       ";
				szCamposEtiquetas[1] = "Desc./NART     ";
				szCamposEtiquetas[2] = "Cantidad       ";
				szCamposEtiquetas[3] = "Lote           ";
				szCamposEtiquetas[4] = "Fecha Vcto     ";

				iCamposCantidad = 5; // Se Desplegaran 4 Campos: Producto, cantidad, Lote, Fecha Vcto

				memset((void *) &LszLineItemBuf,(int) _J('\0'),sizeof(LszLineItemBuf));//Limpiamos Producto por primera vez....
				memset((void *) &LszLineLotBuf,(int) _J('\0'),sizeof(LszLineLotBuf));//Limpiamos Lote por primera vez....

				// Se Utilizan los Indices por Campo para controlarlos mas Abajo
				iIdxProd = 0; iIdxDescr = 1; iIdxCant = 2; iIdxLot = 3; iIdxFVcto = 4;

				memset((void *) &LszLineItemBuf,(int) _J('\0'),sizeof(LszLineItemBuf)); //Limpiamos Producto...
				memset((void *) &mnCantIntroducidaBuf,(int) _J('\0'),sizeof(MATH_NUMERIC));	//Limpiamos Cantidad...
				memset((void *) &LszLineLotBuf,(int) _J('\0'),sizeof(LszLineLotBuf));//Limpiamos Lote...
				AdvanceDate(&jFechaTemp,&dsGetAuditInfo.jdDate,iLotExpirationDate,0);
				FormatDate(LszLotExpirationDate,&jFechaTemp,(JCHAR*) NULL);

				do {//while (iItemFoundInCache == 0); Loop para Solicitar producto a procesar... 

					if (iDbgFlg > 0){
						jdeFprintf(dlg,_J("***Inicializando 3ra pantalla datos entrada por Línea...\n"));
						jdeFflush(dlg);
					}

					iPrimeraVez = 0;
					iLineaEscogida = 0;
					iErrorCode = 0;
					jdeErrorClearEx(lpBhvrCom,lpVoid);

					do { //Loop para validar codigo barra y demás..
						//***Producto
						//jdeFromUnicode(szCamposContenido[iIdxProd],LszLineItemBuf, 25,UTF8);
						iCamposPosiciones[iIdxProd] = strlen(szCamposContenido[iIdxProd]);
						iCamposPosiciones[iIdxDescr] = strlen(szCamposContenido[iIdxDescr]);
						iCamposErrores[iIdxDescr] = 3;//No queremos inverse video...

						//***Cantidad									
						sprintf(szCamposContenido[iIdxCant],"%d",iLineQtyBuf);
						iCamposPosiciones[iIdxCant] = strlen(szCamposContenido[iIdxCant]);
						iCamposErrores[iIdxCant] = 2; //Nos queremos posicionar acá...
						
						//***Lote
						//jdeFromUnicode(szCamposContenido[iIdxLot],LszLineLotBuf,30,UTF8);
						iCamposPosiciones[iIdxLot] = strlen(szCamposContenido[iIdxLot]);

						//*** Fecha Vcto.												
						if (strlen(szCamposContenido[iIdxFVcto]) == 0){//La etiqueta larga no suministró nada...
							jdeFromUnicode(szCamposContenido[iIdxFVcto],LszLotExpirationDate,10,UTF8);					
							iCamposPosiciones[iIdxFVcto] = strlen(szCamposContenido[iIdxFVcto]);
						}

lbEditLineERR: //Por ahora tenemos una etiqueta...

						//(iCamposOffset + 4), (iStatusOffset -4)
						OWDCmp02 (&iPrimeraVez, szPantallaTitulo, (iCamposOffset + 1), iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
										iInicioCampos, iUltimaLinea, iCReqTAB,
										iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
										iDbgFlg, dlg);
						memset(iCamposErrores,'\0',sizeof(iCamposErrores));
						memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));	

						//***Codigo Producto...
						memset(LszUsrEntry,'\0',sizeof(LszUsrEntry));
						jdeToUnicode(LszUsrEntry,szCamposContenido[iIdxProd],25,UTF8);
						if (jdeStrlen(LszUsrEntry) == 0 ) {
							iSalir = 1;
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***Break de Etiqueta 1...\n"));
								jdeFflush(dlg);
							};
							break;  //Ya no mas: salir!!!
						}
						else iSalir = 0;

						//jdeStrcpy(LszTempBuf,_J("                         "));//ItemMasterItem Left Justified padded with blanks...
						//jdeStrcpy(LszLineItemBuf,LszUsrEntry);
						//jdeStrcat(LszLineItemBuf,LszTempBuf + jdeStrlen(LszUsrEntry));
						//if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszLineItemBuf(%ls)...\n"),LszUsrEntry);
							
						memset(LszLineItemBuf,'\0',sizeof(LszLineItemBuf));
						memset(LszUOMdefaultBuf,'\0',sizeof(LszUOMdefaultBuf));
						memset(LszUOMstdConv,'\0',sizeof(LszUOMstdConv));
						memset(LszItemDescriptionBuf,'\0',sizeof(LszItemDescriptionBuf));
						ZeroMathNumeric(&mnItemShortIDBuf);

						if ((iErrorCode = OWDCgbc(hEnv,hUser,LszUsrEntry,LszLineItemBuf,&mnItemShortIDBuf,
												LszUOMdefaultBuf,LszUOMstdConv,LszCrossRefTypeCodeBuf,iUPClenBuf,iSCClenBuf,
												LszItemDescriptionBuf,iGetItemMasterBy,dsGetCrossReferenceFields.c3rdItemNoSymbol,
												iDbgFlg,dlg)) != 0) iCamposErrores[iIdxProd] = 1; //Seteamos el error... Codigo malo

						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***LszLineItemBuf (%ls)...\n"),LszLineItemBuf);
							jdeFprintf(dlg,_J("***LszUOMdefaultBuf (%ls)...\n"),LszUOMdefaultBuf);
							jdeToUnicode(LszString01,mnItemShortIDBuf.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***mnItemShortIDBuf (%ls)...\n"),LszString01);
							jdeFflush(dlg);
						}

					} while (iErrorCode != 0); //Loop para validar codigo barra...

					if (iSalir > 0){
						if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***Break de Etiqueta 2...\n"));
								jdeFflush(dlg);
							};
						break; //Queremos subir a perdir otro OC!!!!
					}
					jdeStrcpy(LszLineUMBuf,LszUOMdefaultBuf);//LszUOMdefaultBuf tiene un valor devuelto por OWDCgbc...	

					//***La cantidad...
					fLineQtyBuf = atof(szCamposContenido[iIdxCant]);
					DoubleToMathNumeric(fLineQtyBuf,&mnCantIntroducidaBuf);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***fLineQtyBuf(%lf)...\n"),fLineQtyBuf);
					if (fLineQtyBuf == 0 ) {
						iSalir = 1;
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***Break de Etiqueta 1a...\n"));
							jdeFflush(dlg);
						};
						break;  //Ya no mas: otro!!!
					}
					else iSalir = 0;
					
					//Buscamos el item leido en el cache...
					iItemFoundInCache = 0;
					/*  Manejo de la Cantidad en la UM de la OC...
					for (i = 0;(i < iRecordsF4311Read) && (iItemFoundInCache == 0); i++){
						iError1 = 1; iError2 = 1;
						MathAdd(&mnCantRecepcionarTemp, 
							&dsF4311Arr[i].CantAcc,&mnCantIntroducidaBuf);//Acumulamos la recepcion...

						//if (strcmp(mnItemShortIDBuf.String,dsF4311Arr[i].pditm.String) == 0) iError1 = 0;
						if ((jdeStrcmp((JCHAR *)mnItemShortIDBuf.String,(JCHAR *)dsF4311Arr[i].pditm.String) == 0) && 
							(dsF4311Arr[i].iStatusRegistro != 2)) iError1 = 0; //Mostrar registros no procesados
						if (MathCompare(&dsF4311Arr[i].pduopn, &mnCantRecepcionarTemp) >= 0) iError2 = 0;

						if ((iError1 == 0) && (iError2 == 0)){ //Mostrar registro seleccionado...
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***dsF4311Arr[%d].pdaitm (%ls)...\n"),i,dsF4311Arr[i].pdaitm);
								jdeFflush(dlg);
							}
							iLineaEscogida = i;//Si es la única linea del Cache, esta es... Si no, agarramos la primera...
							dsF4311Arr[i].iStatusRegistro = 1;
							iItemFoundInCache++;
						}
					}
					*/
					for (i = 0;(i < iRecordsF4311Read) && (iItemFoundInCache == 0); i++){
						iError1 = 1; iError2 = 1;

						//if (strcmp(mnItemShortIDBuf.String,dsF43092Arr[i].pditm.String) == 0) iError1 = 0;
						if ((jdeStrcmp((JCHAR *)mnItemShortIDBuf.String,(JCHAR *)dsF4311Arr[i].pditm.String) == 0) && 
							(dsF4311Arr[i].iStatusRegistro != 2)) iError1 = 0; //Mostrar registros no procesados

						if ((iError1 == 0)){
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***Encontrado dsF4311Arr[%d].pdaitm 1-Segundo Paso (%ls)...\n"),i,dsF4311Arr[i].pdaitm);
								jdeFflush(dlg);
							}
							j = i;
							iItemFoundInCache++;
						}
					}

					if (iError1 == 0){//Lo encontramos.. vamos a validar la cantidad...

						// Convertimos la UM de la transacción en la UM de la OC porque no se aceptan UM distiemtas...
						memset ((void *)(&dsGetItemUoMConversionFactor), (int)(_J('\0')), sizeof(DSD4000520));
						jdeStrcpy (dsGetItemUoMConversionFactor.szFromUnitOfMeasure,LszLineUMBuf); //UM de la transacción...
						jdeStrcpy (dsGetItemUoMConversionFactor.szToUnitOfMeasure,dsF4311Arr[j].pduom); // UM de la OC 
						MathCopy (&dsGetItemUoMConversionFactor.mnShortItemNumber, &mnItemShortIDBuf);
						MathCopy (&dsGetItemUoMConversionFactor.mnQuantityToConvert,&mnCantIntroducidaBuf);

						jdeCallObject(_J("GetItemUoMConversionFactor"),NULL,lpBhvrCom,lpVoid,&dsGetItemUoMConversionFactor,
										(LPCALLMAP)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
						jdeErrorSetToFirstEx(lpBhvrCom,lpVoid);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){							
							iErrorCode = 234;
							jdeFprintf(dlg,_J("***Error(%d): GetItemUoMConversionFactor: %ls(%ls->%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc,
										LszLineUMBuf,dsF4311Arr[j].pduom);
							jdeFflush(dlg);
							iCamposErrores[iIdxCant] = 1; //Seteamos el error... Cantidad mala
							strcpy(szPantallaStatusLine,"Error** Conversión NO existe...");
						}
						FormatMathNumeric(LszString01,&mnCantIntroducidaBuf);
						FormatMathNumeric(LszString02,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);
						MathCopy (&mnCantIntroducidaBuf,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);//Llevamos la cantidad de la traqnsacción a la UM de la línea...

						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemUoMConversionFactor - UOMtr->UOM (%ls/%ls)(%s/%s)(...\n"),
													LszLineUMBuf,dsF4311Arr[j].pduom,LszString01,LszString02);

						MathAdd(&mnCantRecepcionarTemp, 
								&dsF4311Arr[j].CantAcc,&mnCantIntroducidaBuf);//Acumulamos la recepcion...

						if (MathCompare(&dsF4311Arr[j].pduopn, &mnCantRecepcionarTemp) >= 0) iError2 = 0;//Verificamos que no recibimos de más...
						FormatMathNumeric(LszString01,&dsF4311Arr[j].pduopn);
						FormatMathNumeric(LszString02,&mnCantRecepcionarTemp);
						MathCopy (&mnCantIntroducidaBuf,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);//Llevamos la cantidad de la traqnsacción a la UM de la línea...

						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemUoMConversionFactor - Saldo OC(%d) (%s/%s)(...\n"),
													j,LszString01,LszString02);

					}					
					if ((iError1 == 0) && (iError2 == 0) && (iErrorCode == 0)){ //Mostrar registro seleccionado...
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***dsF4311Arr[%d].pdaitm (%ls) 2-Segundo Paso...\n"),j,dsF4311Arr[j].pdaitm);
							jdeFflush(dlg);
						}
						iLineaEscogida = j;//Si es la única linea del Cache, esta es... Si no, agarramos la primera...
						dsF4311Arr[j].iStatusRegistro = 1;
						iItemFoundInCache++;
					}
					if (iError1 == 1) { //No se encontró en el cache
						iCamposErrores[iIdxProd] = 1; //Seteamos el error... Codigo malo

					}
					if (iError2 == 1) { //No se encontró en el cache
						iCamposErrores[iIdxCant] = 1; //Seteamos el error... Cantidad mala
						strcpy(szPantallaStatusLine,"Error** Cantidad muy grande...");
					}

					if ((iItemFoundInCache == 0) || (iError1 == 1) || (iError2 == 1) || (iErrorCode != 0)) { //No se encontró en el cache o hubo un error...
						iErrorCode = 223;	
						iItemFoundInCache = 0;
						jdeFprintf(dlg,_J("***Error(%d): No se encontró en el cache-Segundo Paso...\n"),iErrorCode);
						jdeFflush(dlg);
					}

					if (iItemFoundInCache == 2) {  //Pasó las dos pruebas...

						//Buscamos la OC para accesar su datos...
						for (iOCprocesada=0; iOCprocesada < DocQty; iOCprocesada++){//Localizamos la OC de donde viene esta línea...
							if (jdeStrcmp((JCHAR *)dsVarBuf[iOCprocesada].pddoco.String,(JCHAR *)dsF4311Arr[iLineaEscogida].pddoco.String) == 0) break;
						}

						//***Lotes...
						if(LcLotProcess == _J('1')){ //Procesamiento de Lotes...
							if (iDbgFlg > 0) {
								jdeFprintf(dlg,_J("***Procesando Lotes...\n"));
								jdeFflush(dlg);
							}
							iErrorCode = 0;
							jdeErrorClearEx(lpBhvrCom,lpVoid); 
							jdeToUnicode(LszLineLotBuf,szCamposContenido[iIdxLot],31,UTF8);
							//jdeToUnicode(LszLotExpirationDate,szCamposContenido[iIdxFVcto],31,UTF8);
							iErrorCode1 = FormateaFecha(szCamposContenido[iIdxFVcto],LszLotExpirationDate);

							DeformatDate(&jFechaTemp,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
							DeformatDate(&jFechaTemp1,LszLotExpirationDate,(JCHAR*) _J("DSMSE"));
							nDateDifference = DateDifference(&jFechaTemp,&jFechaTemp1);

							if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszLotExpirationDate(%ls) ...\n"),LszLotExpirationDate);
							if ((iErrorCode1 != 0) || (nDateDifference < 0)) {// Indica que la fecha es nula...
								iErrorCode1 = 910; //Seteamos pr si menor, con el mismo error de invalida...
								if (iErrorCode == 0) iErrorCode = iErrorCode1;
								iCamposErrores[iIdxFVcto] = 1; // Seteamos el error...
								jdeFprintf(dlg,_J("***Error(%d): Fecha Vencimiento Lote Invalida (%s)...\n"),iErrorCode1,LszLotExpirationDate);
							}

							if ((jdeStrcmp(LszLineLotBuf,_J("                              ")) != 0) &&
								(jdeStrcmp(LszLineLotBuf,_J("")) !=0) && 
								(LcValidarLote == _J('1'))){////Queremos validar la Lote (F4108)...
								MathCopy(&dsGetLotMasterByLotNumber.mnShortItemNumber,&mnItemShortIDBuf);
								jdeStrcpy(dsGetLotMasterByLotNumber.szBranchPlant,dsF4311Arr[iLineaEscogida].pdmcu); // LC-2012: LszDocBranchPlantBuf);
								dsGetLotMasterByLotNumber.cCallType = _J('1');
								ParseNumericString (&dsGetLotMasterByLotNumber.mnIndex,_J("1"));
								ParseNumericString (&dsGetLotMasterByLotNumber.mnKeys,_J("3"));
								dsGetLotMasterByLotNumber.cReturnF4108Pointer = _J('0') ;//No queremos data...
								dsGetLotMasterByLotNumber.idF4108LongRowPtr   = (ID) NULL ;	
								if (iDbgFlg > 0) {
									jdeFprintf(dlg,_J("***Validando Lote...\n"));
									jdeFflush(dlg);
								}
								jdeStrcpy (dsGetLotMasterByLotNumber.szLot,LszLineLotBuf);							
								jdeCallObject(_J("GetLotMasterByLotNumber"),NULL,lpBhvrCom,lpVoid,&dsGetLotMasterByLotNumber,
											(CALLMAP*) NULL,(int) 0,(JCHAR*) NULL,(JCHAR*) NULL,(int) 0);
								jdeErrorSetToFirstEx(lpBhvrCom,lpVoid);
								while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){							
										iErrorCode = 214;
										iCamposErrores[iIdxLot] = 1; //Seteamos el error...
										jdeFprintf(dlg,_J("***Error(%d): GetLotMasterByLotNumber: %ls(%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc,LszLineLotBuf);
										jdeFflush(dlg);
								}
								
								iErrorCode1 = FormateaFecha(szCamposContenido[iIdxFVcto],LszLotExpirationDate);
								if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszLotExpirationDate(%ls) ...\n"),LszLotExpirationDate);
								if (iErrorCode1 != 0) {// Indica que la fecha es nula...
									if (iErrorCode == 0) iErrorCode = iErrorCode1;
									iCamposErrores[iIdxFVcto] = 1; // Seteamos el error...
									jdeFprintf(dlg,_J("***Error(%d): Fecha Vencimiento Lote Invalida (%s)...\n"),iErrorCode1,LszLotExpirationDate);
								}

							}
							if (iDbgFlg > 0) {
								jdeFprintf(dlg,_J("***Lote introducido(%ls)...\n"),LszLineLotBuf);
								jdeFflush(dlg);
							}					
						} // if(LcLotProcess == '1') //Procesamiento de Lotes...
						else{
							//No hay procesamiento Lotes (INI)
	  						if (iDbgFlg > 0) {
								jdeFprintf(dlg,_J("***No se proceso Lotes..."));
								jdeFflush(dlg);
							}
						}

					}

					jdeFflush(dlg);

				} while ((iItemFoundInCache == 0) || (iErrorCode != 0));//No se encontró producto en Cache, solicitar otro...
				jdeFflush(dlg);

				if (iSalir > 0){
					if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***Break de Etiqueta 3...\n"));
							jdeFflush(dlg);
						};
					//break; //Queremos subir a perdir otras OCs!!!!
					goto lbBREAK3;
				}			

				//Buscamos la OC para accesar su datos...
				for (iOCprocesada=0; iOCprocesada < DocQty; iOCprocesada++){//Localizamos la OC de donde viene esta línea...
					if (jdeStrcmp((JCHAR *)dsVarBuf[iOCprocesada].pddoco.String,(JCHAR *)dsF4311Arr[iLineaEscogida].pddoco.String) == 0) break;
				}

				if (iDbgFlg > 0) {
					jdeToUnicode(LszString01,dsVarBuf[iOCprocesada].pddoco.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***OC[%d] en proceso(%ls)...\n"),iOCprocesada,LszString01);
					jdeFflush(dlg);
				}

				//Modificado Luis Capriles, 13/10/2014 - Manejo Estado del Lote... INICIO
				memset ((void *)(&dsGetItemBranchMfgData), (int)(_J('\0')), sizeof(DSD4000920));
				//memset ((void *)(&dsLotMasterUpdate), (int)(_J('\0')), sizeof(DSDX4108A));
				jdeStrcpy(dsGetItemBranchMfgData.szBranch,dsF4311Arr[iLineaEscogida].pdmcu);
				MathCopy(&dsGetItemBranchMfgData.mnShortItemNumber,&dsF4311Arr[iLineaEscogida].pditm);
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemBranchMfgData...\n")); 
				idResult = jdeCallObject( _J("GetItemBranchMfgData"),(LPFNBHVR)NULL,lpBhvrCom,lpVoid,&dsGetItemBranchMfgData,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);  
				if (idResult == ER_ERROR){
					iErrorCode = 236;
					jdeFprintf(dlg,_J("***Error(%d): GetItemBranchMfgData:...\n"), iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				} 
				/**
				jdeStrcpy(dsLotMasterUpdate.szBranch,dsF43092Arr[iLineaEscogida].pdmcu);
				MathCopy(&dsLotMasterUpdate.mnShortItemNumber,&dsF43092Arr[iLineaEscogida].pditm);
				jdeStrcpy(dsLotMasterUpdate.szLotNumber,LszLineLotBuf);
				dsLotMasterUpdate.cLotStatus = dsGetItemBranchMfgData.cLotStatusCode;
				dsLotMasterUpdate.cMode = '2';
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LotMasterUpdate(%ls,%lc)...\n"),dsLotMasterUpdate.szLotNumber,dsLotMasterUpdate.cLotStatus); 
				idResult = jdeCallObject( _J("LotMasterUpdate"),(LPFNBHVR)NULL,lpBhvrCom,lpVoid,&dsLotMasterUpdate,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);  
				if (idResult == ER_ERROR){
					iErrorCode = 237;
					jdeFprintf(dlg,_J("***Error(%d): LotMasterUpdate:...\n"), iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}**/
				//Modificado Luis Capriles, 13/10/2014 - Manejo Estado del Lote... FIN


				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF4312EditLine...\n"));

				MathCopy(&dsF4312EditLine.mnJobNumber,&dsF4312BeginDocArr[iOCprocesada].mnJobNumber);
				jdeStrcpy(dsF4312EditLine.szComputerID,dsGetAuditInfo.szWorkstation_UserId);
				dsF4312EditLine.cActionCode = _J('A');
				dsF4312EditLine.cProcessEdits = _J('1');
				jdeStrcpy(dsF4312EditLine.szProgramID,_J("EP4312"));
				dsF4312EditLine.cCurrencyProcessingFlag = dsF4312BeginDocArr[iOCprocesada].cCurrencyProcessingFlag;
				MathCopy(&dsF4312EditLine.mnOrderNumber,&dsF4312BeginDocArr[iOCprocesada].mnOrderNumber);
				jdeStrcpy(dsF4312EditLine.szOrderType,dsF4312BeginDocArr[iOCprocesada].szOrderType);
				jdeStrcpy(dsF4312EditLine.szOrderKeyCompany,dsF4312BeginDocArr[iOCprocesada].szOrderCompany);
				jdeStrcpy(dsF4312EditLine.szOrderSuffix,dsF4311Arr[iLineaEscogida].pdsfxo);
				MathCopy(&dsF4312EditLine.mnLineNumber,&dsF4311Arr[iLineaEscogida].pdlnid);

				//Modificado Luis Capriles, 22/08/2012 - Manejo multiples recepciones una línea...
				//MathCopy(&dsF4312EditLine.mnReceiptLineNumber,&dsF4311Arr[iLineaEscogida].NumLinRC);
				ZeroMathNumeric(&dsF4312EditLine.mnReceiptLineNumber);

				//Modificado Luis Capriles, 22/08/2012 - Manejo multiples recepciones una línea...
				//dsF4312EditLine.cReceiptRecWritten = dsF4311Arr[iLineaEscogida].F43121ZWritten; 
				dsF4312EditLine.cReceiptRecWritten = _J(' ');

				dsF4312EditLine.cOption = LcP4312LineOptionBuf;
				jdeStrcpy(dsF4312EditLine.szPOVersion,dsF4312BeginDocArr[iOCprocesada].szPOVersion);

				//jdeStrcpy(dsF4312EditLine.szReceiptUOM,LszLineUMBuf);
				jdeStrcpy(dsF4312EditLine.szReceiptUOM,dsF4311Arr[iLineaEscogida].pduom);//La recepción no acepta diferentes UOM... 

				MathCopy(&dsF4312EditLine.mnQuantityReceived, &mnCantIntroducidaBuf);

				if (dsF4312BeginDocArr[iOCprocesada].cCurrencyMode == _J('D')) {//Muestra el precio en la moneda de la Orden... 
					MathCopy(&dsF4312EditLine.mnUnitCostReceived,&dsF4311Arr[iLineaEscogida].pdprrc );
					MathCopy(&dsF4312EditLine.mnForeignUnitCostReceived,&dsF4311Arr[iLineaEscogida].pdprrc );
					ZeroMathNumericEx (&dsF4312EditLine.mnForeignUnitCostReceived,CURRENCY_KEEP);
				}
				else {
					MathCopy(&dsF4312EditLine.mnForeignUnitCostReceived,&dsF4311Arr[iLineaEscogida].pdfrrc );
					MathCopy(&dsF4312EditLine.mnUnitCostReceived,&dsF4311Arr[iLineaEscogida].pdfrrc );
					ZeroMathNumericEx (&dsF4312EditLine.mnUnitCostReceived,CURRENCY_KEEP);
				}

				ZeroMathNumeric(&dsF4312EditLine.mnAmountReceived);
				ZeroMathNumeric(&dsF4312EditLine.mnForeignAmountReceived);

				jdeStrcpy(dsDecimalsTriggerGetbyCOCRCD.szCompany,dsF4312BeginDocArr[iOCprocesada].szOrderCompany);
				jdeStrcpy(dsDecimalsTriggerGetbyCOCRCD.szTransactionCurrencyCode,dsF4312BeginDocArr[iOCprocesada].szPOCurrency);
				MathCopy(&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount01, &dsF4312EditLine.mnAmountReceived);
				MathCopy(&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount02, &dsF4312EditLine.mnForeignAmountReceived);
				idResult = jdeCallObject( _J("DecimalsTriggerGetbyCOCRCD"),(LPFNBHVR)NULL,lpBhvrCom,lpVoid,
					&dsDecimalsTriggerGetbyCOCRCD,(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				if (idResult == ER_ERROR){
					iErrorCode = 222;		
					jdeFprintf(dlg,_J("***Error(%d): DecimalsTriggerGetbyCOCRCD...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}
				MathCopy(&dsF4312EditLine.mnAmountReceived,&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount01);
				MathCopy(&dsF4312EditLine.mnForeignAmountReceived,&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount02);

				jdeStrcpy(dsF4312EditLine.szReceiptBranch,dsF4311Arr[iLineaEscogida].pdmcu); //jdeStrcpy(dsF4312EditLine.szReceiptBranch,LszDocBranchPlantBuf);
				jdeStrcpy(dsF4312EditLine.szLocationDatabase,LszLineLocationBuf);
				jdeStrcpy(dsF4312EditLine.szLotNumber,LszLineLotBuf);													
				//· cLotStatus	
				dsF4312EditLine.cLotStatus = dsGetItemBranchMfgData.cLotStatusCode;
				//· mnLotPotency													
				//· szLotGrade														
				DeformatDate(&dsF4312EditLine.jdLotExpirationDate,LszLotExpirationDate,(JCHAR*) _J("DSMSE"));
				//· szLotRetention													
				//· szSupplierLot													
				//· szAssetID														
				//· szLandedCostRule												
				//· szReasonCode													
				//· szContainerID													
				//· szVendorRemark			
				//· szSubledger														
				//· cSubledgerType		
				DeformatDate(&dsF4312EditLine.jdGLDate,LszDocGLDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF4312EditLine.jdReceiptDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				jdeStrcpy(dsF4312EditLine.szDomesticCurrencyCode,dsF4312BeginDocArr[iOCprocesada].szBaseCurrency);
				jdeStrcpy(dsF4312EditLine.szPOCurrencyCode,dsF4312BeginDocArr[iOCprocesada].szPOCurrency);
				//· mnExchangeRate
				MathCopy(&dsF4312EditLine.mnExchangeRate,&dsF4312BeginDocArr[iOCprocesada].mnExchangeRate);

				dsF4312EditLine.cCurrencyMode = dsF4312BeginDocArr[iOCprocesada].cCurrencyMode;
				//· cFirstOfMultiLine
				dsF4312EditLine.cFirstOfMultiLine = _J('1');
				//· cLandedCostCode	
				MathCopy(&dsF4312EditLine.mnBatchNumber,&dsVarBuf[iOCprocesada].mnBatchNumber);
				jdeStrcpy(dsF4312EditLine.szBatchType,dsVarBuf[iOCprocesada].szBatchType);
				DeformatDate(&dsF4312EditLine.jdBatchDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				//· mnReceiptDoc
				MathCopy(&dsF4312EditLine.mnSupplier,&dsF4312BeginDocArr[iOCprocesada].mnSupplier);
				//· szSupplierName	
				dsF4312EditLine.cInventoryInterface = _J('Y');
				dsF4312EditLine.cGLInterface = _J('Y');
				//· cTextLine
				//· szQuantityBucketToUpdate
				//· cReceiptAcknowledgement
				//· cPaymentEligiblemn
				MathCopy(&dsF4312EditLine.mnF4111JobNumber,&dsVarBuf[iOCprocesada].mnF4111JobNumber);
				MathCopy(&dsF4312EditLine.mnF0911JobNumber,&dsVarBuf[iOCprocesada].mnF0911JobNumber);
				dsF4312EditLine.cF4111Written = dsF4311Arr[iLineaEscogida].F4111Written;
				//· mnQtyCompleted													
				//· mnQtyScrapped													
				//· szP3103UOM														
				//· szOperationStatus												
				//· mnLastJELine 
				MathCopy(&dsF4312EditLine.mnLastJELine,&dsF4311Arr[iLineaEscogida].JElnid);
				//· szGLRemark										
				//· szInventoryTransRemark								
				//· cCalledFromRouting									
				//· mnF4311JonbNumber
				//· cCalledFromEDI
				//· mnWMSLineNumber
				//· szMemoLotField1													
				//· szMemoLotField2													
				//· mnTriangulationRateFromCurren									
				//· mnTriangulationRateToCurrency									
				//· cCurrencyConversionMethod										
				//· mnAmbientVolume													
				//· szAmbientVolumeUOM												
				//· mnStandardVolume												
				//· szStandardVolumeUOM												
				//· mnWeightQuantity												
				//· mnWeightQuantityUOM												
				//· mnTemperature													
				//· cTemperatureType												
				//· cDensityType														
				//· mnDensityTemperature											
				//· cDensityTemperatureType											
				//· mnVolumeCorrectionFcator										
				//· cProgramStatus													
				//· mnTransQtyInPrimaryForBulk										
				//· szPrimaryUOMForBulk												
				//· mnDisplayDensity												
				//· mnTransactionTime												
				//· cManagerialAnalysisType1										
				//· szManagerialAnalysisCode1										
				//· cManagerialAnalysisType2										
				//· szManagerialAnalysisCode2										
				//· cManagerialAnalysisType3										
				//· szManagerialAnalysisCode3										
				//· cManagerialAnalysisType4										
				//· szManagerialAnalysisCode4										
				//· szAgreementNumber												
				//· mnAgreementSupplement	
				//· cBatchStatus													
				//· cPostOutOfBalance												
				//· mnWareHouseResSeq												
				//· mnF43092Jobnumber										
				//· mnShipmentNumber												
				//· mnTransportationJobNumber	
				//***

				jdeStrcpy(dsF4312EditLine.szSecondaryUOM,_J(""));
				ZeroMathNumeric(&dsF4312EditLine.mnSecondaryQty);

				if (iDbgFlg > 0) {
					jdeToUnicode(LszString01,dsF4311Arr[iLineaEscogida].pduorg.String,DIM(LszString01),UTF8);
					jdeToUnicode(LszString02,dsF4311Arr[iLineaEscogida].pduopn.String,DIM(LszString02),UTF8);
					jdeFprintf(dlg,_J("***%d)Producto (%ls-%ls) Cantidad (%ls/%ls) Ubicacion(%ls)...\n"),
						iLineaEscogida,dsF4311Arr[iLineaEscogida].pdaitm,dsF4311Arr[iLineaEscogida].pddsc1,
						LszString01,LszString02,dsF4311Arr[iLineaEscogida].pdlocn); 
					jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szOrderKeyCompany);
					jdeToUnicode(LszString01,dsF4312EditLine.mnOrderNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iLineaEscogida,
							LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szOrderType);
					jdeToUnicode(LszString01,dsF4312EditLine.mnLineNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iLineaEscogida,
							LszString01);
					jdeToUnicode(LszString01,dsF4312EditLine.mnQuantityReceived.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] mnQuantityReceived(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] szReceiptUOM(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szReceiptUOM);
					jdeToUnicode(LszString01,dsF4312EditLine.mnUnitCostReceived.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Precio(%ls/%lc %ls)...\n"),iLineaEscogida,LszString01,
							dsF4312BeginDocArr[iOCprocesada].cCurrencyMode,dsF4312EditLine.szPOCurrencyCode);
					jdeFprintf(dlg,_J("***Cache[%d] Location(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szLocationDatabase);
					jdeFprintf(dlg,_J("***Cache[%d] Line LotBuf(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szLotNumber);
					jdeToUnicode(LszString01,dsF4312EditLine.mnBatchNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Batch Number(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] Batch Type(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szBatchType);
					jdeFflush(dlg);
				}

				//***F4312EditLine 
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***dsF4312EditLine...\n")); 
				idResult = jdeCallObject( _J("F4312EditLine"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF4312EditLine,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				jdeErrorSetToFirstEx(lpBhvrCom1, lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)) {
					if (idResult == ER_ERROR){
						iErrorCode = 215;
						jdeFprintf(dlg,_J("***Error(%d): F4312EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc); 
					}
					else {
						jdeFprintf(dlg,_J("***WARN(%d): F4312EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					}
					jdeFflush(dlg);
				}
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 215;
					jdeFprintf(dlg,_J("***Error(%d): F4312EditLine...\n"),iErrorCode);
					jdeFflush(dlg);
				}						
				if (iErrorCode == 0){

					if (iDbgFlg > 0) {
						jdeToUnicode(LszString01,dsF4311Arr[iLineaEscogida].pduorg.String,DIM(LszString01),UTF8);
						jdeToUnicode(LszString02,dsF4311Arr[iLineaEscogida].pduopn.String,DIM(LszString02),UTF8);
						jdeFprintf(dlg,_J("***%d)Producto (%ls-%ls) Cantidad (%ls/%ls) Ubicacion(%ls)...\n"),
							iLineaEscogida,dsF4311Arr[iLineaEscogida].pdaitm,dsF4311Arr[iLineaEscogida].pddsc1,
							LszString01,LszString02,dsF4311Arr[iLineaEscogida].pdlocn); 
						jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szOrderKeyCompany);
						jdeToUnicode(LszString01,dsF4312EditLine.mnOrderNumber.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iLineaEscogida,
								LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szOrderType);
						jdeToUnicode(LszString01,dsF4312EditLine.mnLineNumber.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iLineaEscogida,
								LszString01);
						jdeToUnicode(LszString01,dsF4312EditLine.mnQuantityReceived.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] mnQuantityReceived(%ls)...\n"),iLineaEscogida,LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] szReceiptUOM(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szReceiptUOM);
						jdeToUnicode(LszString01,dsF4312EditLine.mnUnitCostReceived.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Precio(%ls/%lc %ls)...\n"),iLineaEscogida,LszString01,
								dsF4312BeginDocArr[iOCprocesada].cCurrencyMode,dsF4312EditLine.szPOCurrencyCode);
						jdeFprintf(dlg,_J("***Cache[%d] Location(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szLocationDatabase);
						jdeFprintf(dlg,_J("***Cache[%d] Line LotBuf(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szLotNumber);
						jdeToUnicode(LszString01,dsF4312EditLine.mnBatchNumber.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Batch Number(%ls)...\n"),iLineaEscogida,LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] Batch Type(%ls)...\n"),iLineaEscogida,dsF4312EditLine.szBatchType);
						jdeFflush(dlg);
					}

//					iRecordsAvailCache--; // Procesamos eternamente mientras haya mercancía...

					MathCopy(&dsF4311Arr[iLineaEscogida].NumLinRC,&dsF4312EditLine.mnReceiptLineNumber);
					dsF4311Arr[iLineaEscogida].F43121ZWritten  = dsF4312EditLine.cReceiptRecWritten;
					dsF4311Arr[iLineaEscogida].F4111Written  = dsF4312EditLine.cF4111Written;
					MathCopy(&dsVarBuf[iOCprocesada].mnF4111JobNumber,&dsF4312EditLine.mnF4111JobNumber);
					MathCopy(&dsVarBuf[iOCprocesada].mnF0911JobNumber,&dsF4312EditLine.mnF0911JobNumber);
					MathCopy(&dsF4311Arr[iLineaEscogida].JElnid,&dsF4312EditLine.mnLastJELine);
					MathCopy(&dsVarBuf[iOCprocesada].mnBatchNumber,&dsF4312EditLine.mnBatchNumber);
					jdeStrcpy(dsVarBuf[iOCprocesada].szBatchType,dsF4312EditLine.szBatchType);
					MathAdd(&dsF4311Arr[iLineaEscogida].CantAcc,
							&dsF4311Arr[iLineaEscogida].CantAcc,&mnCantIntroducidaBuf);//Acumulamos la recepcion...

					if (MathCompare(&dsF4311Arr[iLineaEscogida].pduopn,&dsF4311Arr[iLineaEscogida].CantAcc) == 0){
//						iRecordsAvailCache--; // Procesamos eternamente mientras haya mercancía...
						dsF4311Arr[iLineaEscogida].iStatusRegistro = 2;//Realmente no hace falta...
					}

					OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 3ra pantalla para mostrar la 2,5da...

					dsVarBuf[iOCprocesada].iEditLineLines = 1;
				}else{
					iCamposErrores[iIdxProd] = 1;
					iCamposErrores[iIdxLot] = 1;
					iCamposErrores[iIdxFVcto] = 1;

					goto lbEditLineERR;// Por ahora es lo mejor que podemos hacer!!!
				}

lbBREAK3: //Break cuando cantidad es 0...
				if (iSalir > 0){
					OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 3ra pantalla para mostrar la 2,5da...
					iSalir = 0;
				};

				jdeErrorClearEx(lpBhvrCom1,lpVoid1);
		
			}while (iRecordsAvailCache > 0); //Procesar mientras haya registros en el Cache (eternamente)...	

			//***Determinar si seguir procesando...

			//Buscamos las OC para determinar si hay EndDocs por hacer...
			iEditLineLines = 0;
			for (i=0; i < DocQty; i++){//Localizamos la OC de donde viene esta línea...
				if (dsVarBuf[i].iEditLineLines > 0) { //Por lo menos un EndDoc!! eso basta...
					iEditLineLines = 1;
					if (iDbgFlg > 0) {
						jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***OC[%d] pendiente de EndDoc(%ls)...\n"),i,LszString01);
						jdeFflush(dlg);
					}
				}
			}

			if (iEditLineLines == 1){
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Inicializando 4ta panttalla datos Aceptacion orden...\n"));
					jdeFflush(dlg);
				}

				memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
				memset(szCamposContenido,'\0',sizeof(szCamposContenido));
				memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
				memset(iCamposErrores,'\0',sizeof(iCamposErrores));
				memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
				memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

				iPrimeraVez = 0;			

				strcpy (szPantallaTitulo, "Recepcion Ordenes Compra          ");

				szCamposEtiquetas[0] = "Procesar Recepcion?(S/N)...";

				strcpy(szCamposContenido[0],"S");
				iCamposPosiciones[0] = strlen(szCamposContenido[0]);

				iCamposCantidad = 1;

				OWDCmp02 (&iPrimeraVez,szPantallaTitulo,(iCamposOffset + 9),iCamposCantidad,(iStatusOffset -9),iInicioEtiquetas, 
									(iInicioCampos + 11),iUltimaLinea,iCReqTAB,
									iCamposPosiciones,szCamposEtiquetas,szCamposContenido,iCamposErrores,szPantallaStatusLine,
									iDbgFlg,dlg);

				szCamposContenido[0][0] = tolower(szCamposContenido[0][0]); szCamposContenido[0][1] = '\0';
				jdeToUnicode(LszUsrEntry,szCamposContenido[0],2,UTF8);
			}
		
			if ((iEditLineLines == 1) && (LszUsrEntry[0] == LcYesNoBuf)) { //Por lo menos una línea EditLine OK...

				//*************************************************************************
				//Procesa F4312EndDoc                                                   ***
				//*************************************************************************				
				//Asignaciones para F4312EndDoc:
				//· mnJobNumer														ok
				//· szComputerID													ok GetAuditInfo
				//· cActionCode														ok 1
				//· cProcessEdits													ok 1
				//· szProgramID														ok EP4312
				//· szPOVersion														ok INI
				//· cCurrencyProcesingFlag											¡!
				//· mnF4111JobNumber												¡!
				//· mnF0911JobNumber												¡!
				//· mnBatchNumber													¡!
				//· szBatchType														¡!
				//· cBatchStatus													¡!
				//· cCurrencyMode													¡!
				//· szUserID														ok GetAuditInfo
				//· szZeroBalRemark				No Asignado							--
				//· szTaxRemark					No Asignado							--
				//· szLandedCostRemark			No Asignado							--
				//· mnDocumentNumber												¡!
				//· cCalledFromEDI				No Asignado							--
				//· mnLandedCJobN													¡!
				//· szReceiptTravelerNumber											--
				//· mnLCF0911JobNumber												¡!
				//· cTransIntransitFlag			No Asignado							--

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF4312EndDoc...\n"));

				iErrorCode1 = 0;

				for (i=0; i < iOCindex; i++){//Para cada OC, hacer si EndDoc...
					iErrorCode = 0;
					if (dsVarBuf[i].iEditLineLines > 0) {//Esta OC tiene líneas...
						if (iDbgFlg > 0) {
							jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***OC[%d] procesando EndDoc(%ls)...\n"),i,LszString01);
							jdeFflush(dlg);
						}

						//***Determina Numero de Recepción...LszReceiptDocumentTypeBuf
						jdeStrcpy(dsReturnFYPN.szCompany,dsF4312BeginDocArr[i].szOrderCompany);
						JDEDATECopy(&dsReturnFYPN.jdTransactionDate,&dsF4312BeginDocArr[i].jdReceiptDate);
						idResult = jdeCallObject(_J("ReturnFYPN"),NULL,lpBhvrCom,(void *)lpVoid,&dsReturnFYPN,
													(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL, (int)0);
						if (idResult == ER_ERROR){
							iErrorCode1 = 1;
							iErrorCode = 216;
							jdeFprintf(dlg,_J("***Error(%d): ReturnFYPN:...\n"), iErrorCode);
							jdeFflush(dlg);
							//goto lbFIN;
							continue;
						} 

						jdeStrcpy(dsX0010GetNextNumber.szSystemCode,_J("43"));
						ParseNumericString(&dsX0010GetNextNumber.mnNextNumberingIndexNo,_J("7"));
						jdeStrcpy(dsX0010GetNextNumber.szCompanyKey,dsF4312BeginDocArr[i].szOrderCompany);
						jdeStrcpy(dsX0010GetNextNumber.szDocumentType,LszReceiptDocumentTypeBuf);
						MathCopy(&dsX0010GetNextNumber.mnCentury,&dsReturnFYPN.mnReturnCentury);
						MathCopy(&dsX0010GetNextNumber.mnFiscalYear1,&dsReturnFYPN.mnReturnFiscalYear);
						idResult = jdeCallObject(_J("X0010GetNextNumber"),NULL,lpBhvrCom,(void *)lpVoid,&dsX0010GetNextNumber,
													(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL, (int)0);
						if (idResult == ER_ERROR){
							iErrorCode1 = 1;
							iErrorCode = 217;
							jdeFprintf(dlg,_J("***Error(%d): X0010GetNextNumber:...\n"), iErrorCode);
							jdeFflush(dlg);
							//goto lbFIN;
							continue;
						} 

						MathCopy(&dsF4312EndDocArr[i].mnJobNumber,&dsF4312BeginDocArr[i].mnJobNumber);
						jdeStrcpy(dsF4312EndDocArr[i].szComputerID,dsGetAuditInfo.szWorkstation_UserId);
						dsF4312EndDocArr[i].cActionCode = _J('A');
						dsF4312EndDocArr[i].cProcessEdits = _J('1');
						jdeStrcpy(dsF4312EndDocArr[i].szProgramID,_J("EP4312"));
						jdeStrcpy(dsF4312EndDocArr[i].szPOVersion,dsF4312BeginDocArr[i].szPOVersion);
						dsF4312EndDocArr[i].cCurrencyProcessingFlag = dsF4312BeginDocArr[i].cCurrencyProcessingFlag;
		  				MathCopy(&dsF4312EndDocArr[i].mnF4111JobNumber,&dsVarBuf[i].mnF4111JobNumber);
						MathCopy(&dsF4312EndDocArr[i].mnF0911JobNumber,&dsVarBuf[i].mnF0911JobNumber);
						MathCopy(&dsF4312EndDocArr[i].mnBatchNumber,&dsVarBuf[i].mnBatchNumber);
						jdeStrcpy(dsF4312EndDocArr[i].szBatchType,dsVarBuf[i].szBatchType);
						//· cBatchStatus
						//· cCurrencyMode
						jdeStrcpy (dsF4312EndDocArr[i].szUserID,dsGetAuditInfo.szUserName);
						//· szZeroBalRemark	
						//· szTaxRemark	
						//· szLandedCostRemark
						MathCopy(&dsF4312EndDocArr[i].mnDocumentNumber,&dsX0010GetNextNumber.mnNextNumber001);
						//· cCalledFromEDI
						//· mnLandedCJobN
						//· mnLCF0911JobNumber
						//· cTransIntransitFlag

						if (iDbgFlg > 0){
							jdeToUnicode(LszString01,dsF4312EndDocArr[i].mnDocumentNumber.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Numero Recepcion(%ls)...\n"),LszString01);
							jdeFflush(dlg);
						}
						
						idResult = jdeCallObject(_J("F4312EndDoc"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsF4312EndDocArr[i],
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
						jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
							iErrorCode1 = 1;
							iErrorCode = 218;
							jdeFprintf(dlg,_J("***Error(%d): F4312EndDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
							jdeFflush(dlg);
						} 				
						if ((idResult == ER_ERROR) && (iErrorCode == 0)){
							iErrorCode1 = 1;
							iErrorCode = 218;
							jdeFprintf(dlg,_J("***Error(%d): F4312EndDoc...\n"),iErrorCode);
							jdeFflush(dlg);
						}
						//if (iErrorCode != 0) goto lbFIN;
						if (iErrorCode1 != 0) continue;

						//***Cierra el batch...
						MathCopy(&dsFSCloseBatch.mnBatchnumber,&dsF4312EndDocArr[i].mnBatchNumber);
						jdeStrcpy(dsFSCloseBatch.szBatchtype,dsF4312EndDocArr[i].szBatchType);
						dsFSCloseBatch.cChangeBatchStatus = _J(' ');
						dsFSCloseBatch.cOverrideMode = _J('A');

						jdeToUnicode(LszString01,dsFSCloseBatch.mnBatchnumber.String,DIM(LszString01),UTF8);
						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***dsFSCloseBatch: %ls %ls...\n"), dsFSCloseBatch.szBatchtype, LszString01);
						idResult = jdeCallObject(_J("FSCloseBatch"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsFSCloseBatch,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
						jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
							iErrorCode = 219;
							jdeFprintf(dlg,_J("***Error(%d): dsFSCloseBatch: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
							jdeFflush(dlg);
						} 				
						if ((idResult == ER_ERROR) && (iErrorCode == 0)){
							iErrorCode = 219;
							jdeFprintf(dlg,_J("***Error(%d): dsFSCloseBatch...\n"),iErrorCode);
							jdeFflush(dlg);
						}
					}else{
						if (iDbgFlg > 0) {
							jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***OC[%d] obviando EndDoc(%ls)...\n"),i,LszString01);
							jdeFflush(dlg);
						}
					}
						
				}//for...

				if (iErrorCode1 != 0) goto lbFIN; // Si por lo menos ocurrió un error, terminar..

				if (iErrorCode1 == 0){//EndDoc OK, seguir...
					//*************************************************************************
					//Procesa F4311ClearWorkFiles                                           ***
					//*************************************************************************				
					//Asignaciones para F4311ClearWorkFiles:
					//· szComputerID												ok GetAuditInfo
					//· mnJobNumber				No Asigndo							ok F4312BeginDoc
					//· cClearHeaderFile											ok 1
					//· cClearDetailFile											ok 1
					//· cErrorInClear			No Asignado							--
					//· mnLineNumber			No Asignado							--
					//· cUseWorkFiles												ok 1
					//· szOrderSuffix			No Asignado							--
					//· mnProcessID													¡!
					//· mnTransactionID												¡!

					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF4311ClearWorkFiles...\n"));

					iErrorCode1 = 0;

					for (i=0; i < iOCindex; i++){//Para cada OC, hacer si EndDoc...
						iErrorCode = 0;
						memset((void *) &dsF4311ClearWorkFiles,(int) _J('\0'),sizeof(DSDF4311Z1E));	
						if (dsVarBuf[i].iEditLineLines > 0) {//Esta OC tiene líneas...
							if (iDbgFlg > 0) {
								jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
								jdeFprintf(dlg,_J("***OC[%d] procesando F4311ClearWorkFiles(%ls)...\n"),i,LszString01);
								jdeFflush(dlg);
							}
							jdeStrcpy(dsF4311ClearWorkFiles.szComputerID,dsGetAuditInfo.szWorkstation_UserId);
							MathCopy(&dsF4311ClearWorkFiles.mnJobNumber,&dsF4312BeginDocArr[i].mnJobNumber);
							dsF4311ClearWorkFiles.cClearHeaderFile = _J('1');
							dsF4311ClearWorkFiles.cClearDetailFile = _J('1');
							//· cErrorInClear			No Asignado							
							//· mnLineNumber			No Asignado	
							dsF4311ClearWorkFiles.cUseWorkFiles = _J('1');
							//· szOrderSuffix
							//· mnProcessID
							//· mnTransactionID

							idResult = jdeCallObject(_J("F4311ClearWorkFiles"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsF4311ClearWorkFiles,
											(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
							jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
							while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
								iErrorCode1 = 1;
								iErrorCode = 220;
								jdeFprintf(dlg,_J("***Error(%d): F4311ClearWorkFiles: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
								jdeFflush(dlg);
							} 
							if ((idResult == ER_ERROR) && (iErrorCode == 0)){
								iErrorCode1 = 1;
								iErrorCode = 220;
								jdeFprintf(dlg,_J("***Error(%d): F4311ClearWorkFiles...\n"),iErrorCode);
								jdeFflush(dlg);
							}
						}
						else{
							if (iDbgFlg > 0) {
								jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
								jdeFprintf(dlg,_J("***OC[%d] obviando F4311ClearWorkFiles(%ls)...\n"),i,LszString01);
								jdeFflush(dlg);
							}
						}
					}//for...

					if (iErrorCode1 != 0) goto lbFIN;
						
					if (iErrorCode1 == 0) {//F4311ClearWorkFiles OK, seguir...
						//*************************************************************************
						//Procesa F4312ClearWorkFile                                            ***
						//*************************************************************************				
						//Asignaciones para F4312ClearWorkFile:
						//· szComputerID									ok GetAuditInfo
						//· mnJobNumber										ok F4312BeginDoc
						//· cErrorInClear				No Asignado			--
						//· mnPOLineNumber				No Asignado			--
						//· mnReceiptLineNumber			No Asignado			--
						//· mnOrderNumber				No Asignado			--
						//· szOrderType					No Asignado			--
						//· szOrderCompany				No Asignado			--
						//· szOrderSuffix				No Asignado			--
						//· szPOVersion										ok
						//· mnLandedCostJobNumber							¡!
						//· cReversalOption									¡!
						//· mnF411JobNumber									ok 
						//· mnF0911JobNuber									ok

						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF4312ClearWorkFile...\n"));

						iErrorCode1 = 0;

						for (i=0; i < iOCindex; i++){//Para cada OC, hacer si EndDoc...
							iErrorCode = 0;
							memset((void *) &dsF4312ClearWorkFile,(int) _J('\0'),sizeof(DSDF4312Z1D));	
							if (dsVarBuf[i].iEditLineLines > 0) {//Esta OC tiene líneas...
								if (iDbgFlg > 0) {
									jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
									jdeFprintf(dlg,_J("***OC[%d] procesando F4312ClearWorkFile(%ls)...\n"),i,LszString01);
									jdeFflush(dlg);
								}

								jdeStrcpy(dsF4312ClearWorkFile.szComputerID,dsGetAuditInfo.szWorkstation_UserId);
								MathCopy(&dsF4312ClearWorkFile.mnJobNumber,&dsF4312BeginDocArr[i].mnJobNumber);
								//· cErrorInClear				
								//· mnPOLineNumber		
								//· mnReceiptLineNumber	
								//· mnOrderNumber		
								//· szOrderType			
								//· szOrderCompany				
								//· szOrderSuffix		
								jdeStrcpy(dsF4312ClearWorkFile.szPOVersion,dsF4312BeginDocArr[i].szPOVersion);									
								//· mnLandedCostJobNumber							
								//· cReversalOption		
								MathCopy(&dsF4312ClearWorkFile.mnF4111JobNumber,&dsVarBuf[i].mnF4111JobNumber);
								MathCopy(&dsF4312ClearWorkFile.mnF0911JobNumber,&dsVarBuf[i].mnF0911JobNumber);

								idResult = jdeCallObject(_J("F4312ClearWorkFile"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsF4312ClearWorkFile,
												(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
								jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
								while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
									iErrorCode1 = 1;
									iErrorCode = 221;
									jdeFprintf(dlg,_J("***Error(%d): F4312ClearWorkFile: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
									jdeFflush(dlg);
								} 
								if ((idResult == ER_ERROR) && (iErrorCode == 0)){
									iErrorCode1 = 1;
									iErrorCode = 221;
									jdeFprintf(dlg,_J("***Error(%d): F4312ClearWorkFile...\n"),iErrorCode);
									jdeFflush(dlg);
								}
							}else{
								if (iDbgFlg > 0) {
									jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
									jdeFprintf(dlg,_J("***OC[%d] obviando F4312ClearWorkFile(%ls)...\n"),i,LszString01);
									jdeFflush(dlg);
								}
							}
						}//for...
						
						if (iErrorCode1 != 0) goto lbFIN;

					}//if (iErrorCode == 0) F4311ClearWorkFiles OK, seguir...
				}//if (iErrorCode == 0) EndDoc OK, seguir...
			} //if (iEditLineLines == 1) Por lo menos una línea EditLine OK...
		} //if (iErrorCode == 0)  BeginDoc está OK, seguir con EditLine y EndDoc ...
		if ((iErrorCode != 0) || (iErrorCode1 != 0)) goto lbFIN;

		iOCindex2 = 0;
		if (iEditLineLines == 1){//Si procdesamos algo, damos oportunidad de repetir abreviado, sin pedir OCs...
			memset((void *) &dsVarBuf2,(int) _J('\0'),sizeof(stVarBuf2)*DocQty);
			for (i=0; i < iOCindex; i++){//Copiamos las OC para simular la entrada...
				MathCopy(&dsVarBuf2[i].pddoco,&dsVarBuf[i].pddoco);
			}
			ZeroMathNumeric(&dsVarBuf2[i].pddoco);
			ZeroMathNumeric(&dsVarBuf2[i+1].pddoco);		
			iUnicaVez = 2;
		}
		else iUnicaVez = 0;//Pedir nuevamente OC...

	} while (TRUE);

	//*************************************************************************
	// Clean up the lpBhvrCom, lpVoid and free user and environment 
	//*************************************************************************


lbFIN:
	JDB_CloseTable(hRequestF4311);
lbFIN0:
	OWDCmp90 (iDbgFlg, dlg);//Limpiamos para mostrar menu...
	jdeFflush(dlg);

	jdeErrorClearEx(lpBhvrCom,lpVoid); 
	jdeFree(((LPCG_BHVR)lpVoid)->lpErrorEventKey);
	jdeErrorTerminateEx(((LPCG_BHVR)lpVoid)->lpHdr);
	jdeFreeBusinessFunctionParms(lpBhvrCom,lpVoid);

	jdeErrorClearEx(lpBhvrCom1,lpVoid1); 
	jdeFree(((LPCG_BHVR)lpVoid1)->lpErrorEventKey);
	jdeErrorTerminateEx(((LPCG_BHVR)lpVoid1)->lpHdr);
	jdeFreeBusinessFunctionParms(lpBhvrCom1,lpVoid1);

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit OWDCrc01(%d)...\n"), iErrorCode);
	jdeFflush(dlg);

	if (iDbgFlg == 1) jdeFclose(dlg);

	return iErrorCode;
}

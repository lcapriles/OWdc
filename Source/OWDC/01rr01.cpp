// 01rr01.cpp 
//
//Creado:		Luis Capriles,		Fecha:	18/12/2013 
//Modificado:	Luis Capriles,		Fecha:	13/10/2014 - Manejo Estado del Lote...
//
//El programa rr01 es una versión reducida del programa P43250-Ruteo Recepciones Compras.
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
#include "rr01.h"
#include <stdio.h>
#include "jde.h"

#include "nxt43092.h"	// F43902EditDoc, F43902EditLine, F43092EndDoc, F43092ClearWorkFile
#include "B0900049.h"	// F0911FSBeginDoc, F0911FSEditLine, F0911FSEndDoc
#include "N4301430.h"	// F43121UpdateLineSplitInRecRout

#include "xt4312z1.h"	// F4312BeginDoc, F43092EditLine, F4312EndDoc, F4312ClearWorkFile
#include "xt4111z1.h"   // F4111ClearDetailStack 

#include "b9800100.h"	// GetAuditInfo
#include "B4000370.h"	// F40095GetDefaultBranch
#include "B0000130.h"	// RetrieveCompanyFromBusUnit
#include "B4000150.h"	// GetBranchConstants
#include "xf41021.h"	// VerifyAndGetItemLocation
#include "b4000310.h"	// FormatLocation
#include "b4001050.h"	// GetCrossReferenceFields
#include "n0000563.h"	// F0010RetrieveCompanyConstant
#include "B0000065.h"	// X0010GetNextNumber
#include "X0903.H"		// ReturnFYPN
#include "xt4311z1.h"	// F43092ClearWorkFiles
#include "b4000610.h"	// GetLotMasterByLotNumber
#include "b0000042.h"	// FSCloseBatch
#include "F43121.h"		// PO Receiver File
#include "B1100007.h"	// DecimalsTriggerGetbyCOCRCD
#include "N1200310.h"	// F0101GetAddressBookDesc
#include "B0000004.h"	// BatchOpenOnInitialization
#include "B4300130.h"	// GetPurchaseHeaderColumns
#include "B4001040.h"	// GetItemMasterDescUOM
#include "B4000520.h"	// GetItemUoMConversionFactor
#include "x4101.h"		// GetItemMasterByShortItem, GetItemMasterBy3rdItem, GetItemMasterBy2ndItem
#include "B4000920.h"	// GetItemBranchMfgData
#include "X4108.h"		// LotMasterUpdate

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

__declspec(dllexport) int OWDCrr01(HENV hEnv,HUSER hUser)
{

	HREQUEST			hRequestV43092JA	= (HREQUEST)NULL;;
	LPJDEERROR_RECORD	ErrorRec	= NULL;
	ID					idResult, idResult1;
	LPCG_BHVR			lpVoid		= NULL;
	LPCG_BHVR			lpVoid1		= NULL;
	LPBHVRCOM			lpBhvrCom	= NULL;
	LPBHVRCOM			lpBhvrCom1	= NULL;
	ERROR_EVENT_KEY		EventKeyLocal;
	LPVOID				lpGetCol	= (LPVOID)NULL;
	DBREF				dbGetCol	= { 0 };

	LPF4101				lpdsF4101 = (LPF4101) NULL;

	DSDF4312Z1D			dsF4312ClearWorkFile; 
	DSDXT4111Z1I		dsF4111ClearDetailStack;
	DSDX43092C			dsF43092EditDoc;
	DSDX43092A			dsF43092EditLine;
	DSDX43092B			dsF43092EndDoc;
	DSD0900049A			dsF0911FSBeginDoc;
	DSD0900049B			dsF0911FSEditLine;
	DSD0900049D			dsF0911FSEndDoc;
	DSD4301430			dsF43121UpdateLineSplitInRecRout; 
	DSD4000150			dsGetBranchConstants;
	DSD4000232			dsF40095GetDefaultBranch;
	DSD0000130			dsRetrieveCompanyFromBusUnit;
	DSDXF41021C			dsVerifyAndGetItemLocation;
	DSD4001050			dsGetCrossReferenceFields;
	DSD4000310A			dsFormatLocation;
	DSD9800100			dsGetAuditInfo;
	DSD0000563			dsF0010RetrieveCompanyConstant;
	DSD0000065A			dsX0010GetNextNumber;
	DSDXX00023			dsReturnFYPN;
	LPFORMDSUDC			lpValidateUDC;
	FORMDSUDC			dsValidateUDC;
	DSD4000610			dsGetLotMasterByLotNumber;
	DSD0000042B			dsFSCloseBatch;
	DSD1100007			dsDecimalsTriggerGetbyCOCRCD;
	DSD1200310			dsF0101GetAddressBookDesc;
	DSD0000004			dsBatchOpenOnInitialization;
	DSD4300130B			dsGetPurchaseHeaderColumns;
	DSMF4101			dsGetItemMasterDescUOM;
	DSD4000520			dsGetItemUoMConversionFactor  = {0};
	DSDX4101B			dsGetItemMasterByShortItem;
	DSD4000920			dsGetItemBranchMfgData;
	DSDX4108A			dsLotMasterUpdate;
	
	typedef struct {	//Implementa el cache: una linea por cada item de las OC...
		JCHAR			pdkcoo[6];	//Order Company 
		MATH_NUMERIC	pddoco;		//Document(OrderNo, Invoice, etc)
		JCHAR			pddcto[3];	//Order Type
		JCHAR			pdsfxo[4];	//Order Suffix
		MATH_NUMERIC	pdlnid;		//Line Number
		MATH_NUMERIC	pdnlin;		//Receipt Line Number
		MATH_NUMERIC	pddoc;		//Document(OrderNo, Invoice, etc)
		JCHAR			pddct[3];	//Order Type
		MATH_NUMERIC	pdan8;		//Supplier
		JCHAR			pdmcu[13];	//Business Unit
		MATH_NUMERIC	pdoprs;		//OperationSequence
		JCHAR			pdupib[5];	//UpdateItemBalanceBucket
		MATH_NUMERIC	pditm;		//IdentifierShortID
		JCHAR			pdaitm[26];	//Identifier3rdItem
		JCHAR			pdlocn[21];	//Location
		JCHAR			pdlotn[31]; //Lot
		JCHAR			pdlnty[3];	//Line Type
		JCHAR			pdnxtr[4];	//Incoming Status Next
		JCHAR			pduom[3];	//UOM as Input
		MATH_NUMERIC	pdqtyo;		//Units Quantity at Operation
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
		JCHAR			F4111ZWritten;
		JCHAR			F4311ZWritten;
		MATH_NUMERIC	JElnid;		
		int				iStatusRegistro;		//0=no procesado, 1=visualizado, 2=procesado
	} stF43092rr01;

	typedef struct { 
		MATH_NUMERIC	pddoco;		//Document(OrderNo, Invoice, etc)
		JCHAR			pddcto[3];	//Order Type
		MATH_NUMERIC	pdan8;		//Supplier
		int				iEditLineLines;	//Flag para indicar que la OC tiene lineas para EndDoc..
		int				iRecordsAvailCache;//Cantidad de líneas de la OC
		MATH_NUMERIC	mnF0911JobNumber;
		MATH_NUMERIC	mnF4111JobNumber;
		MATH_NUMERIC	mnF43121JobNumber;
		MATH_NUMERIC	mnF43092Jobnumber;
		MATH_NUMERIC	mnF4311JobNumber;
		MATH_NUMERIC	mnLCJobNumber;
		MATH_NUMERIC	mnLCF0911JobNumber;
		MATH_NUMERIC	mnLastLineNumber;
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

	stF43092rr01		dsF43092Arr[CacheSize];		//El chache: Arreglo de líneas...
	stVarBuf			dsVarBuf[DocQty];			//Arreglo de variables...
	stVarBuf2			dsVarBuf2[DocQty];			//Arreglo de variables para procesar por segunda vez...
	DSD0900049A			dsF0911FSBeginDocArr[DocQty];//Arreglo de BeginDocs...
//	DSDX43092C			dsF43092EditDocArr[DocQty];	//Arreglo de EditDocs...
//	DSDX43092B			dsF43092EndDocArr[DocQty];	//Arreglo de EndDocs...

	SELECTSTRUCT		lpSelect[5];	// Se Modifica porque en BSN se Obtiene la MCU de la Orden de Compra
	MATH_NUMERIC		mnTempBuf,mnTemp0Buf,mnItemShortIDBuf,mnCantRecepcionarTemp,mnCantRemanenteTemp,
						mnCantIntroducidaBuf, mnNuevaLinea;

	FILE * ini;
	FILE * dlg;

	JCHAR LszLinea[80],LszLin1[64],LszLin2[64],LszUsrEntry[128],

		 LszDocTransDateBuf[16],LszDocGLDateBuf[16],LszDocBranchPlantBuf[16],LszLocationBuf[21],LszLineLocationBuf[21],
		 LszLineLotBuf[31],LszLineItemBuf[26],LszLineUMBuf[3],LszUOMdefaultBuf[3],LszUOMstdConv[3],LszLotExpirationDate[16],

		 LszPOOrderTypeBuf[3],LszReceiptDocumentTypeBuf[3],LszOperationCodeFromBuf[5],LszOperationCodeToBuf[5],LszP43250VersionBuf[16],LszP4312VersionBuf[16],
		 LszIncomingStatus1Buf[4],LszIncomingStatus2Buf[4],LszIncomingStatus3Buf[4],LszOutgoingStatusPartialBuf[4],LszOutgoingStatusClosingBuf[4],
		 LszOutgoingStatusCancelingBuf[4],LszP0900049JEVersionBuf[16],LszCrossRefTypeCodeBuf[3],LszDocumentCompanyBuf[6],LszP4310VersionBuf[16],LszP43214VersionBuf[16],
		 LszDocumentTypeBuf[3],LszItemDescriptionBuf[31],LszUbicacionDfltBuf[21], 

		 LszString01[64],LszString02[64],LszString03[64],LszString04[64],LszTempBuf[128],

		 LcDecimalCharBuf,LcP4312LineOptionBuf,LcSummarizedF0911Buf,LcLotProcess,LcValidarUbicacion,LcValidarLote,LcCurrencyFlag,
		 LcYesNoBuf,LcTempBuf,LcTemp1Buf; 

	JDEDATE	jFechaTemp,jFechaTemp1 ;

	int	 iDocumentNumberBuf,iRecordsF43092Read,iLineaEscogida,iUPClenBuf,iSCClenBuf,iCantLotesProc,iNumLotes,iGetItemMasterBy,iLineQtyBuf,
		 iLotExpirationDate,
		 iErrorCode,iErrorCode1,iDbgFlg,i,j,iSalir,iError1,iError2,
		 iPrimeraVez,iCamposOffset,iCamposCantidad,iStatusOffset,iInicioEtiquetas,iInicioCampos,iUltimaLinea,iCReqTAB,iUnicaVez,
		 iCamposPosiciones[64], iCamposErrores[64],
		 iItemFoundInCache,iRecordsAvailCache,iEditLineLines,iOCindex,iOCindex2,iOCprocesada;

	long nDateDifference;

	int	 iIdxCo = 0, iIdxLoc = 0, iIdxDT = 0, iIdxDoc = 0,
		 iIdxProd = 0, iIdxDescr = 0, iIdxCant = 0, iIdxLot = 0, iIdxFVcto = 0;

	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128],
		 * szDummy1,szTempBuf[128],*szTempBuf1,*szTempBuf2;

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
	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCrr01_D"));
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

	ini = jdeFopen(_J("OWDCrr01.ini"),_J("r"));
	if (!ini){
		jdeFprintf(stderr,_J("***Error abriendo INI (OWDCrr01.ini)...\n"));
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
		if(jdeStrcmp(LszLin1,_J("OperationCodeFrom")) == 0){
			jdeStrcpy(LszOperationCodeFromBuf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: OperationCodeFrom (%ls)...\n"),  
										LszOperationCodeFromBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("OperationCodeTo")) == 0){
			jdeStrcpy(LszOperationCodeToBuf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: OperationCodeTo (%ls)...\n"),  
										LszOperationCodeToBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P43250Version")) == 0){
			jdeStrcpy(LszP43250VersionBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P43250Version (%ls)...\n"),LszP43250VersionBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P4312Version")) == 0){
			jdeStrcpy(LszP4312VersionBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P4312Version (%ls)...\n"),LszP4312VersionBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P4310Version")) == 0){
			jdeStrcpy(LszP4310VersionBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P4310Version (%ls)...\n"),LszP4310VersionBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P43214Version")) == 0){
			jdeStrcpy(LszP43214VersionBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P43214Version (%ls)...\n"),LszP43214VersionBuf);
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
		if(jdeStrcmp(LszLin1,_J("CurrencyFlag")) == 0){
			LcCurrencyFlag = LszLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: CurrencyFlag (%lc)...\n"),
										LcCurrencyFlag);
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
	//F0911FSBeginDoc, F43902EditLine, F43092EndDoc
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
	memset((void *) &dsGetPurchaseHeaderColumns,(int)(_J('\0')),sizeof(DSD4300130B));
	memset((void *) &dsGetItemMasterDescUOM,(int)(_J('\0')),sizeof(DSMF4101));

	ZeroMathNumeric(&mnTemp0Buf);  

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
	//Abre la tabla F43092

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Abriendo Tabla V43092JA...\n"));
	idResult = JDB_OpenView (hUser,_J("V5543092"),(JCHAR *)NULL,&hRequestV43092JA);

	if (idResult == JDEDB_FAILED){
		iErrorCode = 209;
		jdeFprintf(dlg,_J("***Error(%d): JDB_OpenView (V43092JA) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN0;
	}

	//Construye Where del select...
	JDB_ClearSelection(hRequestV43092JA);

	//***Se seleccionan registros que cumplan con:
	jdeNIDcpy(lpSelect[3].Item1.szDict, NID_ACTO);//Active Operation == Y
	jdeNIDcpy(lpSelect[3].Item1.szTable, _J("F43092"));
	lpSelect[3].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelect[3].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelect[3].Item2.szTable, _J(""));
	lpSelect[3].Item2.idInstance = (ID)0;
	lpSelect[3].lpValue = (void *)_J("Y") ;
	lpSelect[3].nValues = (short)1;
	lpSelect[3].nAndOr = JDEDB_ANDOR_AND;
	lpSelect[3].nCmp = JDEDB_CMP_EQ;

	jdeNIDcpy(lpSelect[4].Item1.szDict, NID_OPRC);//Operation Code == LszOperationCodeFromBuf
	jdeNIDcpy(lpSelect[4].Item1.szTable, _J("F43092"));
	lpSelect[4].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelect[4].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelect[4].Item2.szTable, _J(""));
	lpSelect[4].Item2.idInstance = (ID)0;
	lpSelect[4].lpValue = (void *)&LszOperationCodeFromBuf;
	lpSelect[4].nValues = (short)1;
	lpSelect[4].nAndOr = JDEDB_ANDOR_AND;
	lpSelect[4].nCmp = JDEDB_CMP_EQ;

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
		memset((void *) &dsF4312ClearWorkFile,(int) _J('\0'),sizeof(DSDF4312Z1D));	
		memset((void *) &dsF4111ClearDetailStack,(int) _J('\0'),sizeof(DSDXT4111Z1I));
		memset((void *) &dsF43092EditDoc,(int) _J('\0'),sizeof(DSDX43092C));	
		memset((void *) &dsF43092EditLine,(int) _J('\0'),sizeof(DSDX43092A));
		memset((void *) &dsF43092EndDoc,(int) _J('\0'),sizeof(DSDX43092B));
		memset((void *) &dsF0911FSBeginDoc,(int) _J('\0'),sizeof(DSD0900049A));	
		memset((void *) &dsF0911FSEditLine,(int) _J('\0'),sizeof(DSD0900049B));
		memset((void *) &dsF0911FSEndDoc,(int) _J('\0'),sizeof(DSD0900049D));
		memset((void *) &dsF43121UpdateLineSplitInRecRout,(int) _J('\0'),sizeof(DSD4301430));

		memset((void *) &dsDecimalsTriggerGetbyCOCRCD,(int) _J('\0'),sizeof(DSD1100007));

		iOCindex = 0; //seteamos el índice de OCs a 0...

		memset((void *) &dsF43092Arr,(int) _J('\0'),sizeof(stF43092rr01)*CacheSize);
		memset((void *) &dsVarBuf,(int) _J('\0'),sizeof(stVarBuf)*DocQty);
		memset((void *) &dsF0911FSBeginDocArr,(int) _J('\0'),sizeof(DSD0900049A)*DocQty);
//		memset((void *) &dsF43092EndDocArr,(int) _J('\0'),sizeof(DSDX43092B)*DocQty);

		// Se Utilizan los Indices por Campo para controlarlos mas Abajo
		iIdxCo = 0; iIdxLoc = 1; iIdxDT = 2; iIdxDoc = 3;
		iRecordsF43092Read = 0;
		
		do {  //while (iRecordsF43092Read == 0); Loop para obtener una Orden correcta ó muchas OCs...
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
					}//if (iUnicaVez == 0)

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
			jdeNIDcpy(lpSelect[0].Item1.szTable, _J("F43092"));
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
			jdeNIDcpy(lpSelect[1].Item1.szTable, _J("F43092"));
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
			jdeNIDcpy(lpSelect[2].Item1.szTable, _J("F43092"));
			lpSelect[2].Item1.idInstance = (ID)0;
			jdeNIDcpy(lpSelect[2].Item2.szDict, _J(""));
			jdeNIDcpy(lpSelect[2].Item2.szTable, _J(""));
			lpSelect[2].Item2.idInstance = (ID)0;
			lpSelect[2].lpValue = (void *)LszDocumentCompanyBuf;
			lpSelect[2].nValues = (short)1;
			lpSelect[2].nAndOr = JDEDB_ANDOR_AND;
			lpSelect[2].nCmp = JDEDB_CMP_EQ;

			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Construyendo Selection Tabla V43092JA...\n"));
			
			idResult = JDB_SetSelection(hRequestV43092JA,lpSelect,(short)(5),JDEDB_SET_REPLACE);

			if (idResult == JDEDB_FAILED){
				iErrorCode = 228;
				iCamposErrores[1] = 1; // Seteamos el error...
				jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection (V43092JA) failed...\n"),iErrorCode); 
				jdeFflush(dlg);
				goto lbFIN;
			}
			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla V43092JA...\n"));
			idResult = JDB_SelectKeyed(hRequestV43092JA,(ID) 0,(void *)NULL,(short)0);
			if (idResult == JDEDB_FAILED){
				iErrorCode = 211;
				jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed (V43092JA) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}
			
			//iRecordsF43092Read = 0;
			i = 0;

			do {//Carga en Cache los regitros de la Orden de Compra escogida...
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla V43092JA...\n"));		
				idResult = JDB_Fetch(hRequestV43092JA, NULL,(int)0);
				if (idResult == JDEDB_FAILED)jdeFprintf(dlg,_J("***JDB_Fetch (EOF V43092JA) (%d) ...\n"),iRecordsF43092Read);
					// idResult=1 --> SI Encontro Registro
					// idResult=0 --> NO Encontro Registro
				else {
					jdeNIDcpy( dbGetCol.szDict , _J("KCOO") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdkcoo, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdkcoo));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("DOCO") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pddoco, (LPMATH_NUMERIC)lpGetCol);
					}
					jdeNIDcpy( dbGetCol.szDict , _J("DOC") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pddoc, (LPMATH_NUMERIC)lpGetCol);
					}
					jdeNIDcpy( dbGetCol.szDict , _J("DCTO") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pddcto, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pddcto));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("DCT") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pddct, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pddct));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("SFXO") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdsfxo, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdsfxo));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("AN8") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pdan8, (LPMATH_NUMERIC)lpGetCol);
					}
					jdeNIDcpy( dbGetCol.szDict , _J("LNID") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pdlnid, (LPMATH_NUMERIC)lpGetCol);
					}
					jdeNIDcpy( dbGetCol.szDict , _J("NLIN") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pdnlin, (LPMATH_NUMERIC)lpGetCol);
					}
					jdeNIDcpy( dbGetCol.szDict , _J("MCU") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdmcu, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdmcu));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("ITM") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pditm, (LPMATH_NUMERIC)lpGetCol);
					}
					jdeNIDcpy( dbGetCol.szDict , _J("OPRS") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pdoprs, (LPMATH_NUMERIC)lpGetCol);
					}
					jdeNIDcpy( dbGetCol.szDict , _J("AITM") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdaitm, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdaitm));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("UPIB") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdupib, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdupib));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("LOCN") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdlocn, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdlocn));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("LOTN") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdlotn, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdlotn));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("NXTR") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdnxtr, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdnxtr));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("LNTY") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdlnty, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdlnty));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("UOM") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pduom, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pduom));
					}
					//nits Quantity at Operation... pdqtyo
					jdeNIDcpy( dbGetCol.szDict , _J("QTYO") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43092") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pdqtyo, (LPMATH_NUMERIC)lpGetCol);
						//Units Open Quantity... pduopn
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pduopn, (LPMATH_NUMERIC)lpGetCol);
					}			
					jdeNIDcpy( dbGetCol.szDict , _J("CRCD") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						jdeStrncpyTerminate(dsF43092Arr[iRecordsF43092Read].pdcrcd, (JCHAR*)lpGetCol, DIM(dsF43092Arr[iRecordsF43092Read].pdcrcd));
					}
					jdeNIDcpy( dbGetCol.szDict , _J("PRRC") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pdprrc, (LPMATH_NUMERIC)lpGetCol);					
					}
					jdeNIDcpy( dbGetCol.szDict , _J("FRRC") );
					jdeNIDcpy( dbGetCol.szTable , _J("F43121") );
					dbGetCol.idInstance = 0L;
					lpGetCol = JDB_GetTableColValue(hRequestV43092JA, dbGetCol);
					if (lpGetCol == (LPVOID)NULL)  goto lbFIN; 
					else {
						MathCopy( &dsF43092Arr[iRecordsF43092Read].pdfrrc, (LPMATH_NUMERIC)lpGetCol);
					}
					//
					jdeStrcpy(dsDecimalsTriggerGetbyCOCRCD.szCompany,dsF43092Arr[iRecordsF43092Read].pdkcoo);
					jdeStrcpy(dsDecimalsTriggerGetbyCOCRCD.szTransactionCurrencyCode,dsF43092Arr[iRecordsF43092Read].pdcrcd);
					MathCopy(&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount01, &dsF43092Arr[iRecordsF43092Read].pdprrc);
					MathCopy(&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount02, &dsF43092Arr[iRecordsF43092Read].pdfrrc);
					idResult1 = jdeCallObject( _J("DecimalsTriggerGetbyCOCRCD"),(LPFNBHVR)NULL,lpBhvrCom,lpVoid,
						&dsDecimalsTriggerGetbyCOCRCD,(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
					if (idResult1 == ER_ERROR){
						iErrorCode = 230;		
						jdeFprintf(dlg,_J("***Error(%d): DecimalsTriggerGetbyCOCRCD...\n"),iErrorCode);
						jdeFflush(dlg);
						goto lbFIN;
					}
					MathCopy(&dsF43092Arr[iRecordsF43092Read].pdprrc,&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount01);
					MathCopy(&dsF43092Arr[iRecordsF43092Read].pdfrrc,&dsDecimalsTriggerGetbyCOCRCD.mnDomesticAmount02);
					//UoM Primary... pduom1
					//Units Primary UoM... pdpqor

					if ((jdeStrcmp(dsF43092Arr[iRecordsF43092Read].pdnxtr,LszIncomingStatus1Buf) == 0) ||//El registro status OK...
						(jdeStrcmp(dsF43092Arr[iRecordsF43092Read].pdnxtr,LszIncomingStatus2Buf) == 0) ||
						(jdeStrcmp(dsF43092Arr[iRecordsF43092Read].pdnxtr,LszIncomingStatus3Buf) == 0)){
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***%d)Producto (%ls)...\n"),iRecordsF43092Read,
									dsF43092Arr[iRecordsF43092Read].pdaitm);

							jdeToUnicode(LszString01,dsF43092Arr[iRecordsF43092Read].pddoco.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Order Document(OrderNo, Invoice, etc)(%ls)...\n"),iRecordsF43092Read,
									LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iRecordsF43092Read,
									dsF43092Arr[iRecordsF43092Read].pddcto);
							jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iRecordsF43092Read,
									dsF43092Arr[iRecordsF43092Read].pdkcoo);
							jdeStrcpy(LszDocBranchPlantBuf,dsF43092Arr[iRecordsF43092Read].pdmcu); // El almacén es el de la OC...
							jdeFprintf(dlg,_J("***Cache[%d] Order Branch Plant(%ls)...\n"),iRecordsF43092Read,
									dsF43092Arr[iRecordsF43092Read].pdmcu);

							jdeToUnicode(LszString01,dsF43092Arr[iRecordsF43092Read].pdlnid.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iRecordsF43092Read,
									LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Ubicacion(%ls)...\n"),iRecordsF43092Read,
									dsF43092Arr[iRecordsF43092Read].pdlocn);

							jdeToUnicode(LszString01,dsF43092Arr[iRecordsF43092Read].pduorg.String,DIM(LszString01),UTF8);
							jdeToUnicode(LszString02,dsF43092Arr[iRecordsF43092Read].pdqtyo.String,DIM(LszString02),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Cantidad(%ls/%ls)...\n"),iRecordsF43092Read,
							LszString01, LszString02);
							
							jdeToUnicode(LszString01,dsF43092Arr[iRecordsF43092Read].pdprrc.String,DIM(LszString01),UTF8);
							jdeToUnicode(LszString02,dsF43092Arr[iRecordsF43092Read].pdfrrc.String,DIM(LszString02),UTF8);							
							jdeFprintf(dlg,_J("***Cache[%d] Precio (%ls/%ls/%ls)...\n"),iRecordsF43092Read, 
									LszString01,dsF43092Arr[iRecordsF43092Read].pdcrcd,LszString02);
						}
						dsF43092Arr[iRecordsF43092Read].iStatusRegistro = 0;
						iRecordsF43092Read++; //Una líneas más para el caché...
						i++; // Un registro más para esta OC...
						if (iRecordsF43092Read == CacheSize){
							iErrorCode = 212;
							jdeFprintf(dlg,_J("***Error(%d): iRecordsF43092Read > CacheSize!!!\n"),iErrorCode);
							jdeFflush(dlg);
							goto lbFIN;
						}else 
							iRecordsAvailCache = iRecordsF43092Read;
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
				j = iRecordsF43092Read - 1;
				IntToMathNumeric(iDocumentNumberBuf,&dsVarBuf[iOCindex].pddoco);
				jdeStrcpy(dsVarBuf[iOCindex].pddcto,dsF43092Arr[j].pddcto);// El último procesado nos sirve para obtener datos comunes...
				MathCopy(&dsVarBuf[iOCindex].pdan8,&dsF43092Arr[j].pdan8);
				dsVarBuf[iOCindex].iRecordsAvailCache = iRecordsF43092Read;
				dsVarBuf[iOCindex].iEditLineLines = 0;
				ZeroMathNumeric(&dsVarBuf[iOCindex].mnF4111JobNumber);
				jdeStrcpy(dsVarBuf[iOCindex].szTransactionCurrency,dsF43092Arr[j].pdcrcd);
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
		} while ((iRecordsF43092Read == 0) //No se pudo leer ningún registro con esa Orden de Compra->Solicitar otro...
			|| (iDocumentNumberBuf > 0));//Se introdujo una OC y queremos más...
		
		jdeFflush(dlg);

		if ((iDocumentNumberBuf == 0) && (iOCindex == 0)) goto lbFIN; //No hay Orden de Compra->FIN!!!

		//*************************************************************************
		//***Procesa F0911FSBeginDoc                                              ***
		//*************************************************************************
		//Asignaciones para F0911FSBeginDoc:
		//· mnJobNumber															ok "" ok
		//· cDocAction															ok 'A' ok
		//· cEditInstructionFlag												ok '1' ok
		//. cErrorFlag				Salida de la fución							--
		//. cTypeDoc															ok 'J' ok
		//. mnLastLineNumber													ok '0' ok
		//. szCompanyKey			Introducido por el operador					ok     ok
		//. szDocumentType			Asignado por la aplicación					ok INI -- ReceiptDocumentType=OV ok
		//. mnDocNumber															--  ok
		//. jdGLDate				Introducido por el operador					ok INI ok
		//. szLedgerType														-- ok
		//· mnBatchNumber														ok ??? ok 
		//. szBatchType															ok 'O' ??? ok
		//· jdBatchDate															ok TODAY ???
		//. jdBatchSystemDate													--
		//. mnBatchTime															--
		//. szTransactionCurrency	Introducido por el operador					ok ok
		//. mnCurrencyRate														??	???
		//. szExplanation			Asignado por la aplicación					ok -- F0101GetAddressBookDesc ok
		//. szReference1
		//. szHomeBusinessUnit
		//. szInvoiceNumber
		//. jdInvoiceDate
		//. cReverseVoid			Asignado por la aplicación					ok ' ' ok
		//. jdHistoricalDate
		//. mnHistoricalRate
		//. cCurrencyMode			Asignado por la aplicación					ok ok
		//. szBaseCoCurrency		Asignado por la aplicación					ok dsF0010RetrieveCompanyConstant ok
		//. cMultiCurrencyInterco
		//. mnAmountToDistribute
		//. mnCurAmountToDistribute
		//· cCurrencyFlag														ok F0010RetrieveCompanyConstant ok
		//. cHeaderChangedFlag
		//. cModelFlag
		//. szJEVersion															ok INI -- P0900049JEVersion=ZJDE0001
		//. cExchangeRateDateFlag
		//. cPaymentInstrument
		//. cConfigHubTransactionType
		//. szBaseCompany
		
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***F0911FSBeginDoc...\n"));	
		for (i=0; i < iOCindex; i++){
			jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);		
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Asignado F0911FSBeginDoc #%d OC-%ls...\n"),i,LszString01);
				jdeFflush(dlg);
			}

			memset((void *) &dsGetPurchaseHeaderColumns,(int)(_J('\0')),sizeof(DSD4300130B));
			MathCopy(&dsGetPurchaseHeaderColumns.mnOrderNumber,&dsVarBuf[i].pddoco);
			jdeStrcpy(dsGetPurchaseHeaderColumns.szOrderType,dsVarBuf[i].pddcto);
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

			ZeroMathNumeric(&dsF0911FSBeginDocArr[i].mnJobNumber);
			dsF0911FSBeginDocArr[i].cDocAction = _J('A');
			dsF0911FSBeginDocArr[i].cEditInstructionFlag = _J('1');
			dsF0911FSBeginDocArr[i].cTypeDoc = _J('J');
			ZeroMathNumeric(&dsF0911FSBeginDocArr[i].mnLastLineNumber);
			jdeStrcpy(dsF0911FSBeginDocArr[i].szCompanyKey,LszDocumentCompanyBuf);
			jdeStrcpy(dsF0911FSBeginDocArr[i].szDocumentType,LszReceiptDocumentTypeBuf);
			ZeroMathNumeric(&dsF0911FSBeginDocArr[i].mnDocNumber);
			DeformatDate(&dsF0911FSBeginDocArr[i].jdGLDate,LszDocGLDateBuf,(JCHAR*) _J("DSMSE"));
			//. szLedgerType
			jdeStrcpy(dsF0911FSBeginDocArr[i].szBatchType,_J("O"));
			memset((void *) &dsBatchOpenOnInitialization,(int)(_J('\0')),sizeof(DSD0000004));
			jdeStrcpy(dsBatchOpenOnInitialization.szBatchType,dsF0911FSBeginDocArr[i].szBatchType);
			idResult = jdeCallObject(_J("BatchOpenOnInitialization"),NULL,lpBhvrCom,lpVoid,&dsBatchOpenOnInitialization,
					(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
			if (idResult == ER_ERROR){
				iErrorCode = 226;
				jdeFprintf(dlg,_J("***Error(%d): BatchOpenOnInitialization:...\n"), iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}
			//ZeroMathNumeric(&dsF0911FSBeginDocArr[i].mnBatchNumber);
			MathCopy(&dsF0911FSBeginDocArr[i].mnBatchNumber,&dsBatchOpenOnInitialization.mnBatchNumber);
			JDEDATECopy(&dsF0911FSBeginDocArr[i].jdBatchDate,&dsBatchOpenOnInitialization.jdBatchDate);
			
			jdeStrcpy(dsF0911FSBeginDocArr[i].szTransactionCurrency,dsVarBuf[i].szTransactionCurrency);
			if (jdeStrcmp(dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom,dsVarBuf[i].szTransactionCurrency) == 0) 
				dsF0911FSBeginDocArr[i].cCurrencyMode = _J('D');
			else 
				dsF0911FSBeginDocArr[i].cCurrencyMode = _J('F');
			jdeStrcpy(dsF0911FSBeginDocArr[i].szBaseCoCurrency,dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);
			MathCopy(&dsF0911FSBeginDocArr[i].mnCurrencyRate,&dsGetPurchaseHeaderColumns.mnExchangeRate);
			memset((void *) &dsF0101GetAddressBookDesc,(int)(_J('\0')),sizeof(DSD1200310));
			MathCopy(&dsF0101GetAddressBookDesc.mnAddressNumber,&dsVarBuf[i].pdan8);
			idResult = jdeCallObject(_J("F0101GetAddressBookDesc"),NULL,lpBhvrCom,lpVoid,&dsF0101GetAddressBookDesc,
					(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
			if (idResult == ER_ERROR){
				iErrorCode = 225;
				jdeFprintf(dlg,_J("***Error(%d): F0101GetAddressBookDesc:...\n"), iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			} 
			jdeStrcpy(dsF0911FSBeginDocArr[i].szExplanation,dsF0101GetAddressBookDesc.szNameAlpha);
			dsF0911FSBeginDocArr[i].cReverseVoid = _J(' ');
			//dsF0911FSBeginDocArr[i].cCurrencyFlag = dsF0010RetrieveCompanyConstant.cCurrencyConverYNAR;
			dsF0911FSBeginDocArr[i].cCurrencyFlag = LcCurrencyFlag;
			jdeStrcpy(dsF0911FSBeginDocArr[i].szJEVersion,LszP0900049JEVersionBuf);

			//***F0911FSBeginDoc  
			idResult = jdeCallObject(_J("F0911FSBeginDoc"),NULL,lpBhvrCom1,lpVoid1, 
									(void *)&dsF0911FSBeginDocArr[i],(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
			jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
			while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
				if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )) {
					iErrorCode = 213;
					jdeFprintf(dlg,_J("***Error(%d): F0911FSBeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				}
				else{
					jdeFprintf(dlg,_J("***Warn(%d): F0911FSBeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				}
			}
			if ((idResult == ER_ERROR) && (iErrorCode == 0)){ 
				iErrorCode = 213;
				jdeFprintf(dlg,_J("***Error(%d): F0911FSBeginDoc...\n"),iErrorCode);
				jdeFflush(dlg);
			}

			if (iErrorCode == 0) {
				jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);		
				jdeToUnicode(LszString02,dsF0911FSBeginDocArr[i].mnBatchNumber.String,DIM(LszString02),UTF8);
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Creado F0911FSBeginDoc/Batch #%d OC-%ls/%ls...\n"),i,LszString01,LszString02);
					jdeFflush(dlg);
				}
			}

		}//for... Así, hacemos todos los BeginDocs...
		
		iErrorCode1 = 0;
		if (iErrorCode == 0) { // BeginDoc está OK, seguir con EditLine y EndDoc ...  

			//*************************************************************************
			//***F43902EditLine                                                      ***
			//*************************************************************************
			//Asignaciones para F43092EditLine:
			//. szUserID			Asignado por la aplicación					ok GetAuditInfo
			//· szComputerID		Asignado por la aplicación   				ok GetAuditInfo
			//· szProgramID			.											ok EP43250
			//· mnOderNumber		Introducido por el Operador					ok stVarBuf[iOCprocesada].pddoco
			//· szOrderType			Introducido por el Operador					ok stVarBuf[iOCprocesada].pddcto
			//· szOrderCompany		Introducido por el Operador					ok dsF0911FSBeginDocArr[iOCprocesada].szCompanyKey
			//· szOrderSuffix													ok Cache dsF43092Arr[i].pdsfxo
			//· mnOrderLineNumber												ok Cache dsF43092Arr[i].pdlnid
			//· mnReceiptLineNumber 											ok Cache dsF43092Arr[i].pdnlin
			//. szOperationCodeFrom Asignado por la aplicación					ok LszOperationCodeFromBuf
			//. szOperationCodeTo   Asignado por la aplicación					ok LszOperationCodeToBuf
			//. szUnitOfMeasureFrom												ok Cache dsF43092Arr[i].pduom
			//. szP43250Version													ok INI LszP43250VersionBuf
			//. cReverseDispositionFlag											ok _J(' ')
			//. mnQuantityAtOperation											ok Cache dsF43092Arr[i].pdqtyo
			//. mnQuantityToMove	Introducido por el Operador					ok mnCantIntroducidaBuf
			//. mnQuantityAdjusted												ok 0
			//. mnQuantityRejected												ok 0
			//. mnQuantityReturned												ok 0
			//. mnQuantityReworked												ok 0
			//. mnQuantityScrapped												ok 0
			//. szReasonCodeAdj													ok _J("    ")
			//. szReasonCodeRej													ok _J("    ")
			//. szReasonCodeRet													ok _J("    ")
			//. szReasonCodeRew													ok _J("    ")
			//. szReasonCodeScr													ok _J("    ")
			//· jdReceiptDate		Introducido por el operador					ok INI
			//· jdGLDate			Introducido por el operador					ok INI
			//· jdBatchDate														ok dsF0911FSBeginDocArr[iOCprocesada].jdBatchDate
			//· mnBatchNumber													ok dsF0911FSBeginDocArr[iOCprocesada].mnBatchNumber
			//. cBatchStatus													??
			//. szBatchType														ok dsF0911FSBeginDocArr[iOCprocesada].szBatchType
			//· szDomesticCurrencyCode											ok dsF0911FSBeginDocArr[iOCprocesada].szBaseCoCurrency
			//· cCurrencyMode													ok dsF0911FSBeginDocArr[iOCprocesada].cCurrencyMode
			//· mnExchangeRate													ok dsF0911FSBeginDocArr[iOCprocesada].mnExchangeRate
			//. mnF43092Jobnumber												ok stVarBuf[iOCprocesada].mnF43092Jobnumber
			//. mnF43121JobNumber												ok stVarBuf[iOCprocesada].mnF43121JobNumber
			//. cF43121ZRecordWritten											ok Cache dsF43092Arr[i].F43121ZWritten
			//. szP4312Version													ok INI LszP4312VersionBuf
			//. cF4111ZRecordWritten											ok Cache dsF43092Arr[i].F4111ZWritten
			//. mnF4311JobNumber												ok stVarBuf[iOCprocesada].mnF4311JobNumber
			//. cF4311ZRecordWritten											ok stVarBuf[iOCprocesada].F4311ZWritten
			//. szP4310Version													ok INI LszP4310VersionBuf
			//. mnF0911JobNumber												ok stVarBuf[iOCprocesada].mnF0911JobNumber
			//. mnLastJELineNumber												ok stVarBuf[iOCprocesada].mnLastJELineNumber
			//. cMovementControl
			//. mnShortItemNumber												ok dCache sF43092Arr[i].pditm
			//. mnTrnToPrimaryCnvFactor
			//. mnPrchToPrimaryCnvFactor
			//. mnF4111JobNumber												ok Cache dsF43092Arr[i].mnF4111JobNumber
			//. jdLedgerUpdateDate
			//. mnLedgerTimeUpdated
			//. cSelectionExit													ok 1 o 7 P4312LineOption=4
			//. jdDateToday														ok dsGetAuditInfo.jdDate
			//. mnTimeOfDay														ok dsGetAuditInfo.mnTime
			//. szUnknownItemNumber
			//. mnPOLineNumber													ok 0
			//. jdOrderDate
			//. jdRequestDate
			//. jdPromisedDate
			//. szP43214Version													ok INI LszP43214VersionBuf			

			iEditLineLines = 0; // No hay líneas en el documento de salida...

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

					for (i = 0;((i < iRecordsF43092Read) && (iItemFoundInCache == 0) && (iErrorCode == 0)); i++){
						iError1 = 1; 

						if ((jdeStrcmp((JCHAR *)mnItemShortIDBuf.String,(JCHAR *)dsF43092Arr[i].pditm.String) == 0) && 
							(dsF43092Arr[i].iStatusRegistro != 2)) iError1 = 0; //Mostrar registros no procesados

						if (iError1 == 0){ //Mostrar registro seleccionado...
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***dsF43092Arr[%d].pdaitm (%ls)-Primer Paso...\n"),i,dsF43092Arr[i].pdaitm);
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

				memset((void *) &dsF43092EditLine,(int) _J('\0'),sizeof(DSDX43092A));
				ZeroMathNumeric(&dsF43092EditLine.mnF43092Jobnumber);
				ZeroMathNumeric(&dsF43092EditLine.mnF43121JobNumber);
				ZeroMathNumeric(&dsF43092EditLine.mnF4311JobNumber);
				ZeroMathNumeric(&dsF43092EditLine.mnF0911JobNumber);
				ZeroMathNumeric(&dsF43092EditLine.mnF4111JobNumber);
				ZeroMathNumeric(&dsF43092EditLine.mnMvmtDispJEJobNumber);
				ZeroMathNumeric(&dsF43092EditLine.mnLandedCJobNo);
				ZeroMathNumeric(&dsF43092EditLine.mnLastJELineNumber);

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

				iCamposCantidad = 5; // Se Desplegaran 5 Campos: Producto, cantidad, Lote, Fecha Vcto

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
					for (i = 0;(i < iRecordsF43092Read) && (iItemFoundInCache == 0); i++){
						iError1 = 1; iError2 = 1;
//						MathAdd(&mnCantRecepcionarTemp, 
//							&dsF43092Arr[i].CantAcc,&mnCantIntroducidaBuf);//Acumulamos la recepcion...

						//if (strcmp(mnItemShortIDBuf.String,dsF43092Arr[i].pditm.String) == 0) iError1 = 0;
						if ((jdeStrcmp((JCHAR *)mnItemShortIDBuf.String,(JCHAR *)dsF43092Arr[i].pditm.String) == 0) && 
							(dsF43092Arr[i].iStatusRegistro != 2)) iError1 = 0; //Mostrar registros no procesados
//						if (MathCompare(&dsF43092Arr[i].pdqtyo, &mnCantRecepcionarTemp) >= 0) iError2 = 0;

//						if ((iError1 == 0) && (iError2 == 0)){} //Mostrar registro seleccionado...
						if ((iError1 == 0)){
							if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***Encontrado dsF43092Arr[%d].pdaitm 1 (%ls)-Segundo Paso...\n"),i,dsF43092Arr[i].pdaitm);
								jdeFflush(dlg);
							}
//							iLineaEscogida = i;//Si es la única linea del Cache, esta es... Si no, agarramos la primera...
							j = i;
//							dsF43092Arr[i].iStatusRegistro = 1;
							iItemFoundInCache++;
						}
					}

					if (iError1 == 0){//Lo encontramos.. vamos a validar la cantidad...

						// Convertimos la UM de la transacción en la UM de la OC porque no se aceptan UM distiemtas...
						memset ((void *)(&dsGetItemUoMConversionFactor), (int)(_J('\0')), sizeof(DSD4000520));
						jdeStrcpy (dsGetItemUoMConversionFactor.szFromUnitOfMeasure,LszLineUMBuf); //UM de la transacción...
						jdeStrcpy (dsGetItemUoMConversionFactor.szToUnitOfMeasure,dsF43092Arr[j].pduom); // UM de la OC 
						MathCopy (&dsGetItemUoMConversionFactor.mnShortItemNumber, &mnItemShortIDBuf);
						MathCopy (&dsGetItemUoMConversionFactor.mnQuantityToConvert,&mnCantIntroducidaBuf);

						jdeCallObject(_J("GetItemUoMConversionFactor"),NULL,lpBhvrCom,lpVoid,&dsGetItemUoMConversionFactor,
										(LPCALLMAP)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
						jdeErrorSetToFirstEx(lpBhvrCom,lpVoid);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){							
							iErrorCode = 234;
							jdeFprintf(dlg,_J("***Error(%d): GetItemUoMConversionFactor: %ls(%ls->%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc,
										LszLineUMBuf,dsF43092Arr[j].pduom);
							jdeFflush(dlg);
							iCamposErrores[iIdxCant] = 1; //Seteamos el error... Cantidad mala
							strcpy(szPantallaStatusLine,"Error** Conversión NO existe...");
						}
						FormatMathNumeric(LszString01,&mnCantIntroducidaBuf);
						FormatMathNumeric(LszString02,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);
						MathCopy (&mnCantIntroducidaBuf,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);//Llevamos la cantidad de la traqnsacción a la UM de la línea...

						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemUoMConversionFactor - UOMtr->UOM (%ls/%ls)(%s/%s)(...\n"),
													LszLineUMBuf,dsF43092Arr[j].pduom,LszString01,LszString02);

						MathAdd(&mnCantRecepcionarTemp, 
								&dsF43092Arr[j].CantAcc,&mnCantIntroducidaBuf);//Acumulamos la recepcion...

						if (MathCompare(&dsF43092Arr[j].pdqtyo, &mnCantRecepcionarTemp) >= 0) iError2 = 0;//Verificamos que no recibimos de más...
						FormatMathNumeric(LszString01,&dsF43092Arr[j].pdqtyo);
						FormatMathNumeric(LszString02,&mnCantRecepcionarTemp);
						MathCopy (&mnCantIntroducidaBuf,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);//Llevamos la cantidad de la traqnsacción a la UM de la línea...

						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemUoMConversionFactor - Saldo OC(%d) (%s/%s)(...\n"),
													j,LszString01,LszString02);

					}
					
					if ((iError1 == 0) && (iError2 == 0) && (iErrorCode == 0)){ //Mostrar registro seleccionado...
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***dsF43092Arr[%d].pdaitm 2 (%ls)-Segundo Paso...\n"),i,dsF43092Arr[i].pdaitm);
							jdeFflush(dlg);
						}
						iLineaEscogida = j;//Si es la única linea del Cache, esta es... Si no, agarramos la primera...
						dsF43092Arr[j].iStatusRegistro = 1;
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
							if (jdeStrcmp((JCHAR *)dsVarBuf[iOCprocesada].pddoco.String,(JCHAR *)dsF43092Arr[iLineaEscogida].pddoco.String) == 0) break;
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
								jdeFprintf(dlg,_J("***Error(%d): Fecha Vencimiento Lote Invalida (%ls)...\n"),iErrorCode1,LszLotExpirationDate);
							}

							if ((jdeStrcmp(LszLineLotBuf,_J("                              ")) != 0) &&
								(jdeStrcmp(LszLineLotBuf,_J("")) !=0) && 
								(LcValidarLote == _J('1'))){////Queremos validar la Lote (F4108)...
								MathCopy(&dsGetLotMasterByLotNumber.mnShortItemNumber,&mnItemShortIDBuf);
								jdeStrcpy(dsGetLotMasterByLotNumber.szBranchPlant,dsF43092Arr[iLineaEscogida].pdmcu); // LC-2012: LszDocBranchPlantBuf);
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
				for (iOCprocesada=0; iOCprocesada < DocQty; iOCprocesada++){//Localizamos la OC (iOCprocesada) de donde viene esta línea...
					if (jdeStrcmp((JCHAR *)dsVarBuf[iOCprocesada].pddoco.String,(JCHAR *)dsF43092Arr[iLineaEscogida].pddoco.String) == 0) break;
				}

				if (iDbgFlg > 0) {
					jdeToUnicode(LszString01,dsVarBuf[iOCprocesada].pddoco.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***OC[%d] en proceso(%ls)...\n"),iOCprocesada,LszString01);
					jdeFflush(dlg);
				}
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF43092EditLine...\n"));

				//**Ahora hay que hacer el split para conseguir el cambio de ubicación y lote...
				//. cMatchType														ok '1'
				//. mnOrderNumber													ok dsF43092EditLine.mnOrderNumber
				//. szOrderType														ok dsF43092EditLine.szOrderType
				//. szOrderCompany													ok dsF43092EditLine.szOrderCompany
				//. szOrderSuffix													ok dsF43092EditLine.szOrderSuffix
				//. mnLineNumber													ok dsF43092EditLine.mnOrderLineNumber
				//. mnExistingReceiptLineNumber										ok dsF43092EditLine.mnReceiptLineNumber
				//. szCurrentBranchPlant											ok dsF43092Arr[iLineaEscogida].pdmcu
				//. mnOperationSequence												ok dsF43092Arr[iLineaEscogida].pdoprs
				//. mnShortItemNumber												ok dsF43092EditLine.mnShortItemNumber
				//. szLocation														ok dsF43092Arr[iLineaEscogida].pdlocn
				//. szLot															ok dsF43092Arr[iLineaEscogida].pdlotn
				//. mnMovementQuantity												ok mnCantIntroducidaBuf
				//. szUpdateItemBalanceBucket										ok dsF43092Arr[iLineaEscogida].pdupib
				//. szProgramId														ok [EP43250]
				//. mnNewReceiptLineNumber											ok 0
				//. szNewBranchPlant												ok dsF43092Arr[iLineaEscogida].pdmcu
				//. mnDocumentNumber												ok 0
				//. mnLotPotency													ok 0
				//. szLotGrade														ok ' '
				//. cLotStatusCode													ok ' '
				//. szSupplierLotNumber												ok ' '
				//. szMemoLotField1													ok ' '
				//. szMemoLotField2													ok ' '
				//. szMoveUnitOfMeasure												ok jdeStrcpy(dsF43092EditLine.szUnitOfMeasureFrom ???
				//. szLotDescription												ok LszItemDescriptionBuf
				//. jdLotExpiration													ok LszLotExpirationDate
				//. mnSupplierNumber												ok dsF43092Arr[iLineaEscogida].pdan8
				//. jdBestBeforeDate												ok LszLotExpirationDate
				//. jdSellByDate													ok LszLotExpirationDate
				//. jdBasedOnDate													ok LszDocTransDateBuf
				//. jdOnHandDate													ok LszDocTransDateBuf
				//. jdLotEffectivityDate											ok LszDocTransDateBuf
				//. jdUserLotDate1..5												ok LszDocTransDateBuf 

				dsF43121UpdateLineSplitInRecRout.cMatchType = _J('1');
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnOrderNumber,&dsVarBuf[iOCprocesada].pddoco);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szOrderType,dsVarBuf[iOCprocesada].pddcto);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szOrderCompany,dsF0911FSBeginDocArr[iOCprocesada].szCompanyKey);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szOrderSuffix,dsF43092Arr[iLineaEscogida].pdsfxo);
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnLineNumber,&dsF43092Arr[iLineaEscogida].pdlnid); 
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnExistingReceiptLineNumber,&dsF43092Arr[iLineaEscogida].pdnlin);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szCurrentBranchPlant,dsF43092Arr[iLineaEscogida].pdmcu);
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnOperationSequence,&dsF43092Arr[iLineaEscogida].pdoprs);
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnShortItemNumber,&dsF43092Arr[iLineaEscogida].pditm);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szLocation,LszLineLocationBuf);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szLot,LszLineLotBuf);
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnMovementQuantity,&mnCantIntroducidaBuf);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szUpdateItemBalanceBucket,dsF43092Arr[iLineaEscogida].pdupib);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szProgramId,_J("EP43250"));
				ZeroMathNumeric(&dsF43121UpdateLineSplitInRecRout.mnNewReceiptLineNumber); 
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szNewBranchPlant,dsF43092Arr[iLineaEscogida].pdmcu);
				//ZeroMathNumeric(&dsF43121UpdateLineSplitInRecRout.mnDocumentNumber); 
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnDocumentNumber,&dsF43092Arr[iLineaEscogida].pddoc);
				ZeroMathNumeric(&dsF43121UpdateLineSplitInRecRout.mnLotPotency);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szLotGrade,_J(" ")); dsF43121UpdateLineSplitInRecRout.cLotStatusCode = _J(' ');
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szSupplierLotNumber,_J(" ")); jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szMemoLotField1,_J(" "));
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szMemoLotField2,_J(" "));
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szMoveUnitOfMeasure,dsF43092Arr[iLineaEscogida].pduom);
				jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szLotDescription,LszItemDescriptionBuf);				
				MathCopy(&dsF43121UpdateLineSplitInRecRout.mnSupplierNumber,&dsF43092Arr[iLineaEscogida].pdan8);

				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdLotExpiration,LszLotExpirationDate,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdOnHandDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdBestBeforeDate,LszLotExpirationDate,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdSellByDate,LszLotExpirationDate,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdLotEffectivityDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdBasedOnDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdUserLotDate1,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdUserLotDate2,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdUserLotDate3,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdUserLotDate4,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43121UpdateLineSplitInRecRout.jdUserLotDate5,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));

				if (iDbgFlg > 0) { 
					jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF43121UpdateLineSplitInRecRout.szOrderCompany);
					jdeFprintf(dlg,_J("***Cache[%d] Order BP(%ls)...\n"),iLineaEscogida,dsF43121UpdateLineSplitInRecRout.szCurrentBranchPlant);
					jdeToUnicode(LszString01,dsF43121UpdateLineSplitInRecRout.mnOrderNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls %ls.%ls)...\n"),iLineaEscogida,
							dsF43121UpdateLineSplitInRecRout.szOrderType,LszString01,dsF43121UpdateLineSplitInRecRout.szOrderSuffix);
					jdeToUnicode(LszString01,dsF43121UpdateLineSplitInRecRout.mnLineNumber.String,DIM(LszString01),UTF8);
					jdeToUnicode(LszString02,dsF43121UpdateLineSplitInRecRout.mnExistingReceiptLineNumber.String,DIM(LszString02),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls/%ls)...\n"),iLineaEscogida, LszString01, LszString02);
					jdeToUnicode(LszString01,dsF43121UpdateLineSplitInRecRout.mnMovementQuantity.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] mnMovementQuantity(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFflush(dlg);
				}


				//***F43121UpdateLineSplitInRecRout (nuevo)...
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***F43121UpdateLineSplitInRecRout por recibido...\n")); 
				idResult = jdeCallObject( _J("F43121UpdateLineSplitInRecRout"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF43121UpdateLineSplitInRecRout,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				jdeErrorSetToFirstEx(lpBhvrCom1, lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)) {
					if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )) {
						iErrorCode = 229;
						jdeFprintf(dlg,_J("***Error(%d): F43121UpdateLineSplitInRecRout: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc); 
					}
					else {
						jdeFprintf(dlg,_J("***WARN(%d): F43121UpdateLineSplitInRecRout: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					}
					jdeFflush(dlg);
				}

				MathCopy(&mnNuevaLinea,&dsF43121UpdateLineSplitInRecRout.mnNewReceiptLineNumber); //Nos recordamos de la línea creada parea el EditLine...

				MathSubtract(&mnCantRemanenteTemp, 
											&dsF43092Arr[iLineaEscogida].pdqtyo,&mnCantRecepcionarTemp);//Calculamos lo que queda en el paso... 
				if (MathCompare(&mnCantRemanenteTemp, &mnTemp0Buf) >= 0){
					//***F43121UpdateLineSplitInRecRout (restante)...
					MathCopy(&dsF43121UpdateLineSplitInRecRout.mnNewReceiptLineNumber,&dsF43092Arr[iLineaEscogida].pdnlin); 
					jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szLocation,dsF43092Arr[iLineaEscogida].pdlocn);
					jdeStrcpy(dsF43121UpdateLineSplitInRecRout.szLot,dsF43092Arr[iLineaEscogida].pdlotn);
					MathCopy(&dsF43121UpdateLineSplitInRecRout.mnMovementQuantity,&mnCantRemanenteTemp);

				if (iDbgFlg > 0) { 
					jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF43121UpdateLineSplitInRecRout.szOrderCompany);
					jdeFprintf(dlg,_J("***Cache[%d] Order BP(%ls)...\n"),iLineaEscogida,dsF43121UpdateLineSplitInRecRout.szCurrentBranchPlant);
					jdeToUnicode(LszString01,dsF43121UpdateLineSplitInRecRout.mnOrderNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls %ls.%ls)...\n"),iLineaEscogida,
							dsF43121UpdateLineSplitInRecRout.szOrderType,LszString01,dsF43121UpdateLineSplitInRecRout.szOrderSuffix);
					jdeToUnicode(LszString01,dsF43121UpdateLineSplitInRecRout.mnLineNumber.String,DIM(LszString01),UTF8);
					jdeToUnicode(LszString02,dsF43121UpdateLineSplitInRecRout.mnNewReceiptLineNumber.String,DIM(LszString02),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls/%ls)...\n"),iLineaEscogida, LszString01, LszString02);
					jdeToUnicode(LszString01,dsF43121UpdateLineSplitInRecRout.mnMovementQuantity.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] mnMovementQuantity(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFflush(dlg);
				}

					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***F43121UpdateLineSplitInRecRout por restante...\n")); 
					idResult = jdeCallObject( _J("F43121UpdateLineSplitInRecRout"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF43121UpdateLineSplitInRecRout,
											(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
					jdeErrorSetToFirstEx(lpBhvrCom1, lpVoid1);
					while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)) {
						if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )) {
							iErrorCode = 231;
							jdeFprintf(dlg,_J("***Error(%d): F43121UpdateLineSplitInRecRout 2: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc); 
						}
						else {
							jdeFprintf(dlg,_J("***WARN(%d): F43121UpdateLineSplitInRecRout 2: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						}
						jdeFflush(dlg);
					}
					jdeFflush(dlg);
				}

				//Modificado Luis Capriles, 13/10/2014 - Manejo Estado del Lote... INICIO
				memset ((void *)(&dsGetItemBranchMfgData), (int)(_J('\0')), sizeof(DSD4000920));
				memset ((void *)(&dsLotMasterUpdate), (int)(_J('\0')), sizeof(DSDX4108A));
				jdeStrcpy(dsGetItemBranchMfgData.szBranch,dsF43092Arr[iLineaEscogida].pdmcu);
				MathCopy(&dsGetItemBranchMfgData.mnShortItemNumber,&dsF43092Arr[iLineaEscogida].pditm);
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemBranchMfgData...\n")); 
				idResult = jdeCallObject( _J("GetItemBranchMfgData"),(LPFNBHVR)NULL,lpBhvrCom,lpVoid,&dsGetItemBranchMfgData,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);  
				if (idResult == ER_ERROR){
					iErrorCode = 236;
					jdeFprintf(dlg,_J("***Error(%d): GetItemBranchMfgData:...\n"), iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				} 

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
				}
				//Modificado Luis Capriles, 13/10/2014 - Manejo Estado del Lote... FIN

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF43092EditLine por recibido...\n")); 
				dsF43092Arr[iLineaEscogida].F43121ZWritten = _J(' ');
				dsF43092Arr[iLineaEscogida].F4111ZWritten = _J(' ');
				dsF43092Arr[iLineaEscogida].F4311ZWritten = _J(' ');

				jdeStrcpy(dsF43092EditLine.szUserID,dsGetAuditInfo.szUserName);
				jdeStrcpy(dsF43092EditLine.szComputerID,dsGetAuditInfo.szWorkstation_UserId);
				jdeStrcpy(dsF43092EditLine.szProgramID,_J("EP43250"));
				MathCopy(&dsF43092EditLine.mnMvmtDispJEJobNumber,&dsF0911FSBeginDocArr[iOCprocesada].mnJobNumber); 
				MathCopy(&dsF43092EditLine.mnOrderNumber,&dsVarBuf[iOCprocesada].pddoco);
				jdeStrcpy(dsF43092EditLine.szOrderType,dsVarBuf[iOCprocesada].pddcto);
				jdeStrcpy(dsF43092EditLine.szOrderCompany,dsF0911FSBeginDocArr[iOCprocesada].szCompanyKey);
				jdeStrcpy(dsF43092EditLine.szOrderSuffix,dsF43092Arr[iLineaEscogida].pdsfxo);
				MathCopy(&dsF43092EditLine.mnOrderLineNumber,&dsF43092Arr[iLineaEscogida].pdlnid);  
				MathCopy(&dsF43092EditLine.mnReceiptLineNumber,&mnNuevaLinea);
				//ZeroMathNumeric(&dsF43092EditLine.mnReceiptLineNumber);
				jdeStrcpy(dsF43092EditLine.szOperationCodeFrom,LszOperationCodeFromBuf);
				jdeStrcpy(dsF43092EditLine.szOperationCodeTo,LszOperationCodeToBuf);
				//jdeStrcpy(dsF43092EditLine.szUnitOfMeasureFrom,LszLineUMBuf);//
				jdeStrcpy(dsF43092EditLine.szUnitOfMeasureFrom,dsF43092Arr[iLineaEscogida].pduom);//El split no acepta diferentes UOM... 
				jdeStrcpy(dsF43092EditLine.szP43250Version,LszP43250VersionBuf);
				dsF43092EditLine.cReverseDispositionFlag = _J(' ');
				//MathCopy(&dsF43092EditLine.mnQuantityAtOperation,&dsGetItemUoMConversionFactor.mnQuantityToConvert);
				MathCopy(&dsF43092EditLine.mnQuantityAtOperation,&mnCantIntroducidaBuf);//El split no acepta diferentes UOM...
				//MathCopy(&dsF43092EditLine.mnQuantityToMove,&dsGetItemUoMConversionFactor.mnQuantityToConvert);
				MathCopy(&dsF43092EditLine.mnQuantityToMove,&mnCantIntroducidaBuf);//El split no acepta diferentes UOM...
				ZeroMathNumeric(&dsF43092EditLine.mnQuantityAdjusted); jdeStrcpy(dsF43092EditLine.szReasonCodeAdj,_J("    "));
				ZeroMathNumeric(&dsF43092EditLine.mnQuantityRejected); jdeStrcpy(dsF43092EditLine.szReasonCodeRej,_J("    "));
				ZeroMathNumeric(&dsF43092EditLine.mnQuantityReturned); jdeStrcpy(dsF43092EditLine.szReasonCodeRet,_J("    "));
				ZeroMathNumeric(&dsF43092EditLine.mnQuantityReworked); jdeStrcpy(dsF43092EditLine.szReasonCodeRew,_J("    "));
				ZeroMathNumeric(&dsF43092EditLine.mnQuantityScrapped); jdeStrcpy(dsF43092EditLine.szReasonCodeScr,_J("    "));
				DeformatDate(&dsF43092EditLine.jdReceiptDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE")); 
				DeformatDate(&dsF43092EditLine.jdGLDate,LszDocGLDateBuf,(JCHAR*) _J("DSMSE"));
				DeformatDate(&dsF43092EditLine.jdBatchDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE")); 
				MathCopy(&dsF43092EditLine.mnBatchNumber,&dsF0911FSBeginDocArr[iOCprocesada].mnBatchNumber);
				jdeStrcpy(dsF43092EditLine.szBatchType,dsF0911FSBeginDocArr[iOCprocesada].szBatchType);
				jdeStrcpy(dsF43092EditLine.szDomesticCurrencyCode,dsF0911FSBeginDocArr[iOCprocesada].szBaseCoCurrency); 
				dsF43092EditLine.cCurrencyMode = dsF0911FSBeginDocArr[iOCprocesada].cCurrencyMode;
				MathCopy(&dsF43092EditLine.mnExchangeRate,&dsF0911FSBeginDocArr[iOCprocesada].mnCurrencyRate);
				MathCopy(&dsF43092EditLine.mnF43092Jobnumber,&dsVarBuf[iOCprocesada].mnF43092Jobnumber);
				MathCopy(&dsF43092EditLine.mnF43121JobNumber,&dsVarBuf[iOCprocesada].mnF43121JobNumber);
				dsF43092EditLine.cF43121ZRecordWritten = dsF43092Arr[iLineaEscogida].F43121ZWritten;
				jdeStrcpy(dsF43092EditLine.szP4312Version,LszP4312VersionBuf);
				dsF43092EditLine.cF4111ZRecordWritten = dsF43092Arr[iLineaEscogida].F4111ZWritten;
				MathCopy(&dsF43092EditLine.mnF4311JobNumber,&dsVarBuf[iOCprocesada].mnF4311JobNumber);
				dsF43092EditLine.cF4311ZRecordWritten = dsF43092Arr[iLineaEscogida].F4311ZWritten;
				jdeStrcpy(dsF43092EditLine.szP4310Version,LszP4310VersionBuf);
				MathCopy(&dsF43092EditLine.mnF0911JobNumber,&dsVarBuf[iOCprocesada].mnF0911JobNumber);
				MathCopy(&dsF43092EditLine.mnLastJELineNumber,&dsVarBuf[iOCprocesada].mnLastLineNumber);
				MathCopy(&dsF43092EditLine.mnShortItemNumber,&dsF43092Arr[iLineaEscogida].pditm);
				MathCopy(&dsF43092EditLine.mnF4111JobNumber,&dsVarBuf[iOCprocesada].mnF4111JobNumber); 
				dsF43092EditLine.cSelectionExit = LcP4312LineOptionBuf;
				JDEDATECopy(&dsF43092EditLine.jdBatchDate,&dsGetAuditInfo.jdDate);
				JDEDATECopy(&dsF43092EditLine.jdDateToday,&dsGetAuditInfo.jdDate);
				MathCopy(&dsF43092EditLine.mnTimeOfDay,&dsGetAuditInfo.mnTime);
				ZeroMathNumeric(&dsF43092EditLine.mnPOLineNumber);
				jdeStrcpy(dsF43092EditLine.szP43214Version,LszP43214VersionBuf); 


				if (iDbgFlg > 0) {
					jdeToUnicode(LszString01,dsF43092Arr[iLineaEscogida].pduorg.String,DIM(LszString01),UTF8);
					jdeToUnicode(LszString02,dsF43092Arr[iLineaEscogida].pdqtyo.String,DIM(LszString02),UTF8);
					jdeFprintf(dlg,_J("***%d)Producto (%ls) Cantidad (%ls/%ls) Ubicacion(%ls)...\n"),
						iLineaEscogida,dsF43092Arr[iLineaEscogida].pdaitm,
						LszString01,LszString02,dsF43092Arr[iLineaEscogida].pdlocn); 
					jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szOrderCompany);
					jdeToUnicode(LszString01,dsF43092EditLine.mnOrderNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iLineaEscogida,
							LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szOrderType);
					jdeToUnicode(LszString01,dsF43092EditLine.mnOrderLineNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iLineaEscogida,
							LszString01);
					jdeToUnicode(LszString01,dsF43092EditLine.mnQuantityToMove.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] mnQuantityReceived(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] szReceiptUOM(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szUnitOfMeasureFrom);
//					jdeToUnicode(LszString01,dsF43092EditLine.mnUnitCostReceived.String,DIM(LszString01),UTF8);
//					jdeFprintf(dlg,_J("***Cache[%d] Precio(%ls/%lc %ls)...\n"),iLineaEscogida,LszString01,
//							dsF0911FSBeginDocArr[iOCprocesada].cCurrencyMode,dsF43092EditLine.szPOCurrencyCode);
//					jdeFprintf(dlg,_J("***Cache[%d] Location(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szLocationDatabase);
//					jdeFprintf(dlg,_J("***Cache[%d] Line LotBuf(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szLotNumber);
					jdeToUnicode(LszString01,dsF43092EditLine.mnBatchNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Batch Number(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] Batch Type(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szBatchType);
					jdeFflush(dlg);
				}

				//***F43092EditLine 
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***dsF43092EditLine 1...\n")); 

				idResult = jdeCallObject( _J("F43092EditLine"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF43092EditLine,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				jdeErrorSetToFirstEx(lpBhvrCom1, lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)) {
					if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )) {
						iErrorCode = 215;
						jdeFprintf(dlg,_J("***Error(%d): F43092EditLine 1: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc); 
					}
					else {
						jdeFprintf(dlg,_J("***WARN(%d): F43092EditLine 1: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					}
					jdeFflush(dlg);
				}
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 215;
					jdeFprintf(dlg,_J("***Error(%d): F43092EditLine 1...\n"),iErrorCode);
					jdeFflush(dlg);
				}

				LcTempBuf = dsF43092EditLine.cModeProcessing;

				if (iErrorCode == 0){
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF43092EditLine por restante...\n"));
					MathCopy(&dsF43092EditLine.mnReceiptLineNumber,&dsF43092Arr[iLineaEscogida].pdnlin);
					MathCopy(&dsF43092EditLine.mnQuantityAtOperation,&mnCantRemanenteTemp);
					MathCopy(&dsF43092EditLine.mnQuantityToMove,&mnCantRemanenteTemp);
					dsF43092EditLine.cF43121ZRecordWritten = _J(' ');
					dsF43092EditLine.cF4111ZRecordWritten = _J(' ');
					dsF43092EditLine.cF4311ZRecordWritten = _J(' ');
					ZeroMathNumeric(&dsF43092EditLine.mnTrnToPrimaryCnvFactor);	
					ZeroMathNumeric(&dsF43092EditLine.mnPrchToPrimaryCnvFactor);  
					dsF43092EditLine.cSelectionExit = _J(' '); 

					//***F43092EditLine  
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***F43092EditLine 2...\n")); 
					idResult = jdeCallObject( _J("F43092EditLine"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF43092EditLine,
											(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
					jdeErrorSetToFirstEx(lpBhvrCom1, lpVoid1);
					while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)) {
						if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )) {
							iErrorCode = 215;
							jdeFprintf(dlg,_J("***Error(%d): F43092EditLine 2: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc); 
						}
						else {
							jdeFprintf(dlg,_J("***WARN(%d): F43092EditLine 2: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						}
						jdeFflush(dlg);
					}
					if ((idResult == ER_ERROR) && (iErrorCode == 0)){
						iErrorCode = 215;
						jdeFprintf(dlg,_J("***Error(%d): F43092EditLine 2...\n"),iErrorCode);
						jdeFflush(dlg);
					}
				}
			
				if (iErrorCode == 0){ //Los dos EditLine están OK...  Calculamos costos...
					//***F43092EditDoc
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignando dsF43092EditDoc ...\n")); 
					memset((void *) &dsF43092EditDoc,(int) _J('\0'),sizeof(DSDX43092C));
					MathCopy(&dsF43092EditDoc.mnF43121Jobnumber,&dsF43092EditLine.mnF43121JobNumber);
					jdeStrcpy(dsF43092EditDoc.szComputerID,dsGetAuditInfo.szWorkstation_UserId);
					dsF43092EditDoc.cActionCode1 = _J('1');
					dsF43092EditDoc.cProcessEdits = _J('1');
					jdeStrcpy(dsF43092EditDoc.szProgramId,_J("EP43250"));
					jdeStrcpy(dsF43092EditDoc.szUserId,dsGetAuditInfo.szUserName);
					jdeStrcpy(dsF43092EditDoc.szVersion,LszP4312VersionBuf);
					ZeroMathNumeric(&dsF43092EditDoc.mnLandedCostJobNumber);
					ZeroMathNumeric(&dsF43092EditDoc.mnLCF0911JobNumber);
					MathCopy(&dsF43092EditDoc.mnOrderNumber,&dsVarBuf[iOCprocesada].pddoco);
					jdeStrcpy(dsF43092EditDoc.szOrderType,dsVarBuf[iOCprocesada].pddcto);
					jdeStrcpy(dsF43092EditDoc.szCompanyKeyOrderNo,dsF0911FSBeginDocArr[iOCprocesada].szCompanyKey);
					DeformatDate(&dsF43092EditDoc.jdGLDate,LszDocGLDateBuf,(JCHAR*) _J("DSMSE"));
					MathCopy(&dsF43092EditDoc.mnBatchNumber,&dsF0911FSBeginDocArr[iOCprocesada].mnBatchNumber);
					jdeStrcpy(dsF43092EditDoc.szBatchType,dsF0911FSBeginDocArr[iOCprocesada].szBatchType);
					dsF43092EditDoc.cCurrencyMode = _J('D');
					jdeStrcpy(dsF43092EditDoc.szCurrencyCode,dsF0911FSBeginDocArr[iOCprocesada].szBaseCoCurrency);
					dsF43092EditDoc.cReversalOption = _J('1');
					jdeStrcpy(dsF43092EditDoc.szP43250Version,LszP43250VersionBuf);
					MathCopy(&dsF43092EditDoc.mnLineNumber,&dsF43092Arr[iLineaEscogida].pdlnid); 
					dsF43092EditDoc.cModeProcessing = LcTempBuf;

					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***F43092EditDoc...\n")); 
					idResult = jdeCallObject( _J("F43092EditDoc"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF43092EditDoc,
											(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
					jdeErrorSetToFirstEx(lpBhvrCom1, lpVoid1);
					while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)) {
						if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )) {
							iErrorCode = 233;
							jdeFprintf(dlg,_J("***Error(%d): dsF43092EditDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc); 
						}
						else {
							jdeFprintf(dlg,_J("***WARN(%d): dsF43092EditDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						}
						jdeFflush(dlg);
					}
					if ((idResult == ER_ERROR) && (iErrorCode == 0)){
						iErrorCode = 233;
						jdeFprintf(dlg,_J("***Error(%d): dsF43092EditDoc...\n"),iErrorCode);
						jdeFflush(dlg);
					}
				}
				

				if (iErrorCode == 0){ //Los dos EditLine están OK...
					if (iDbgFlg > 0) {
						jdeToUnicode(LszString01,dsF43092Arr[iLineaEscogida].pduorg.String,DIM(LszString01),UTF8);
						jdeToUnicode(LszString02,dsF43092Arr[iLineaEscogida].pdqtyo.String,DIM(LszString02),UTF8);
						jdeFprintf(dlg,_J("***%d)Producto (%ls) Cantidad (%ls/%ls) Ubicacion(%ls)...\n"),
							iLineaEscogida,dsF43092Arr[iLineaEscogida].pdaitm,
							LszString01,LszString02,dsF43092Arr[iLineaEscogida].pdlocn); 
						jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szOrderCompany);
						jdeToUnicode(LszString01,dsF43092EditLine.mnOrderNumber.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iLineaEscogida,
								LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szOrderType);
						jdeToUnicode(LszString01,dsF43092EditLine.mnOrderLineNumber.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iLineaEscogida,
								LszString01);
						jdeToUnicode(LszString01,dsF43092EditLine.mnQuantityToMove.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] mnQuantityToMove(%ls)...\n"),iLineaEscogida,LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] szReceiptUOM(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szUnitOfMeasureFrom);
//						jdeToUnicode(LszString01,dsF43092EditLine.mnUnitCostReceived.String,DIM(LszString01),UTF8);
//						jdeFprintf(dlg,_J("***Cache[%d] Precio(%ls/%lc %ls)...\n"),iLineaEscogida,LszString01,
//								dsF0911FSBeginDocArr[iOCprocesada].cCurrencyMode,dsF43092EditLine.szPOCurrencyCode);
//						jdeFprintf(dlg,_J("***Cache[%d] Location(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szLocationDatabase);
//						jdeFprintf(dlg,_J("***Cache[%d] Line LotBuf(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szLotNumber);
						jdeToUnicode(LszString01,dsF43092EditLine.mnBatchNumber.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Batch Number(%ls)...\n"),iLineaEscogida,LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] Batch Type(%ls)...\n"),iLineaEscogida,dsF43092EditLine.szBatchType);
						jdeFflush(dlg);
					}

//					iRecordsAvailCache--; // Procesamos eternamente mientras haya mercancía...

					MathCopy(&dsF43092Arr[iLineaEscogida].NumLinRC,&dsF43092EditLine.mnReceiptLineNumber);
					dsF43092Arr[iLineaEscogida].F43121ZWritten = dsF43092EditLine.cF43121ZRecordWritten;
					dsF43092Arr[iLineaEscogida].F4111ZWritten = dsF43092EditLine.cF4111ZRecordWritten;
					dsF43092Arr[iLineaEscogida].F4311ZWritten = dsF43092EditLine.cF4311ZRecordWritten;

					MathCopy(&dsVarBuf[iOCprocesada].mnF4111JobNumber,&dsF43092EditLine.mnF4111JobNumber);
					MathCopy(&dsVarBuf[iOCprocesada].mnF0911JobNumber,&dsF43092EditLine.mnF0911JobNumber);
					MathCopy(&dsVarBuf[iOCprocesada].mnF43121JobNumber,&dsF43092EditLine.mnF43121JobNumber);
					MathCopy(&dsVarBuf[iOCprocesada].mnF43092Jobnumber,&dsF43092EditLine.mnF43092Jobnumber);
					MathCopy(&dsVarBuf[iOCprocesada].mnF4311JobNumber,&dsF43092EditLine.mnF4311JobNumber);

					MathCopy(&dsVarBuf[iOCprocesada].mnLCJobNumber,&dsF43092EditDoc.mnLandedCostJobNumber);
					MathCopy(&dsVarBuf[iOCprocesada].mnLCF0911JobNumber,&dsF43092EditDoc.mnLCF0911JobNumber);

//					MathCopy(&dsF43092Arr[iLineaEscogida].JElnid,&dsF43092EditLine.mnLastJELine);

					MathCopy(&dsVarBuf[iOCprocesada].mnBatchNumber,&dsF43092EditLine.mnBatchNumber);
					jdeStrcpy(dsVarBuf[iOCprocesada].szBatchType,dsF43092EditLine.szBatchType);
					MathAdd(&dsF43092Arr[iLineaEscogida].CantAcc,
							&dsF43092Arr[iLineaEscogida].CantAcc,&mnCantIntroducidaBuf);//Acumulamos la recepcion...

					if (MathCompare(&dsF43092Arr[iLineaEscogida].pdqtyo,&dsF43092Arr[iLineaEscogida].CantAcc) == 0){
//						iRecordsAvailCache--; // Procesamos eternamente mientras haya mercancía...
						dsF43092Arr[iLineaEscogida].iStatusRegistro = 2;//Realmente no hace falta...
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
				//Procesa F43092EndDoc                                                   ***
				//*************************************************************************				
				//Asignaciones para F43092EndDoc:
				//· mnF43092JobNumber												ok dsVarBuf[i].mnF43092Jobnumber;
				//· szComputerID													ok GetAuditInfo
				//· jdGLDate														ok dsF0911FSBeginDocArr[i].jdGLDate
				//· jdReceiptDate													ok LszDocTransDateBuf
				//· mnF43121JobNumber												ok dsVarBuf[i].mnF43121JobNumber
				//· cF43121ZRecordWritten											ok INI
				//· szP4312Version													ok INI
				//· cF4111ZRecordWritten											ok dsVarBuf[i]
				//· mnF4311JobNumber												ok dsVarBuf[i]
				//· cF4311ZRecordWritten											ok dsVarBuf[i]
				//· szP4310Version													ok INI
				//· mnF0911JobNumber												ok dsVarBuf[i]
				//· mnF4111JobNumber												ok dsVarBuf[i]
				//· jdDateToday														ok GetAuditInfo
				//· mnTimeOfDay														ok GetAuditInfo
				//· szP43214Version													ok INI
				//· mnDocVoucherInvoiceE											ok 0
				//· szP43250Version													ok INI
				//· mnLCJobNumber													ok dsVarBuf[i]
				//· mnLCF0911JobNumber												ok dsVarBuf[i]
				//· mnMvmtDispJEJobNumber											ok dsF0911FSBeginDocArr[i]
				//· mnBatchNumber													ok dsF0911FSBeginDocArr
				//· szReceiptDocType												ok INI

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado Fds43092EndDoc...\n"));

				iErrorCode1 = 0;

				for (i=0; i < iOCindex; i++){//Para cada OC, hacer si EndDoc...
					iErrorCode = 0;
					if (dsVarBuf[i].iEditLineLines > 0) {//Esta OC tiene líneas...
						if (iDbgFlg > 0) {
							jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***OC[%d] procesando EndDoc(%ls)...\n"),i,LszString01);
							jdeFflush(dlg);
						};

						memset((void *) &dsF43092EndDoc,(int) _J('\0'),sizeof(DSDX43092B));

						MathCopy(&dsF43092EndDoc.mnF43092JobNumber,&dsVarBuf[i].mnF43092Jobnumber);
						jdeStrcpy(dsF43092EndDoc.szComputerID,dsGetAuditInfo.szWorkstation_UserId);
						JDEDATECopy(&dsF43092EndDoc.jdGLDate,&dsF0911FSBeginDocArr[i].jdGLDate);
						DeformatDate(&dsF43092EndDoc.jdReceiptDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE")); 
						MathCopy(&dsF43092EndDoc.mnF43121JobNumber,&dsVarBuf[i].mnF43121JobNumber);
						dsF43092EndDoc.cF43121ZRecordWritten = dsF43092Arr[i].F43121ZWritten = _J(' ');
						jdeStrcpy(dsF43092EndDoc.szP4312Version,LszP4312VersionBuf);
						dsF43092EndDoc.cF4111ZRecordWritten = dsF43092Arr[i].F4111ZWritten = _J(' ');
						MathCopy(&dsF43092EndDoc.mnF4311JobNumber,&dsVarBuf[i].mnF4311JobNumber);
						dsF43092EndDoc.cF4311ZRecordWritten = dsF43092Arr[iLineaEscogida].F4311ZWritten = _J(' ');
						jdeStrcpy(dsF43092EndDoc.szP4310Version,LszP4310VersionBuf);
						MathCopy(&dsF43092EndDoc.mnF0911JobNumber,&dsVarBuf[i].mnF0911JobNumber);
						MathCopy(&dsF43092EndDoc.mnF4111JobNumber,&dsVarBuf[i].mnF4111JobNumber);
						JDEDATECopy(&dsF43092EndDoc.jdDateToday,&dsGetAuditInfo.jdDate);
						MathCopy(&dsF43092EndDoc.mnTimeOfDay,&dsGetAuditInfo.mnTime);
						jdeStrcpy(dsF43092EndDoc.szP43214Version,LszP43214VersionBuf); 
						ZeroMathNumeric(&dsF43092EndDoc.mnDocVoucherInvoiceE);
						jdeStrcpy(dsF43092EndDoc.szP43250Version,LszP43250VersionBuf);
						MathCopy(&dsF43092EndDoc.mnLCJobNumber,&dsVarBuf[i].mnLCJobNumber); 
						MathCopy(&dsF43092EndDoc.mnLCF0911JobNumber,&dsVarBuf[i].mnLCF0911JobNumber); 
						MathCopy(&dsF43092EndDoc.mnMvmtDispJEJobNumber,&dsF0911FSBeginDocArr[i].mnJobNumber);
						MathCopy(&dsF43092EndDoc.mnBatchNumber,&dsF0911FSBeginDocArr[i].mnBatchNumber);
						jdeStrcpy(dsF43092EndDoc.szReceiptDocType,LszReceiptDocumentTypeBuf);
						
						idResult = jdeCallObject(_J("F43092EndDoc"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsF43092EndDoc,
												(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
						jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
							iErrorCode1 = 1;
							iErrorCode = 218;
							jdeFprintf(dlg,_J("***Error(%d): F43092EndDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
							jdeFflush(dlg);
						} 				
						if ((idResult == ER_ERROR) && (iErrorCode == 0)){
							iErrorCode1 = 1;
							iErrorCode = 218;
							jdeFprintf(dlg,_J("***Error(%d): F43092EndDoc...\n"),iErrorCode);
							jdeFflush(dlg);
						}
						//if (iErrorCode != 0) goto lbFIN;
						if (iErrorCode1 != 0) continue;

						//***Cierra el batch...
						memset((void *) &dsFSCloseBatch,(int)(_J('\0')),sizeof(DSD0000042B)); 
						MathCopy(&dsFSCloseBatch.mnBatchnumber,&dsF43092EndDoc.mnBatchNumber);
						jdeStrcpy(dsFSCloseBatch.szBatchtype,dsF0911FSBeginDocArr[i].szBatchType);
						dsFSCloseBatch.cChangeBatchStatus = _J(' ');
						dsFSCloseBatch.cOverrideMode = _J('A');

						jdeToUnicode(LszString01,dsFSCloseBatch.mnBatchnumber.String,DIM(LszString01),UTF8);
						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***FSCloseBatch: %ls %ls...\n"), dsFSCloseBatch.szBatchtype, LszString01);
						idResult = jdeCallObject(_J("FSCloseBatch"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsFSCloseBatch,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
						jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
							iErrorCode = 219;
							jdeFprintf(dlg,_J("***Error(%d): FSCloseBatch: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
							jdeFflush(dlg);
						} 				
						if ((idResult == ER_ERROR) && (iErrorCode == 0)){
							iErrorCode = 219;
							jdeFprintf(dlg,_J("***Error(%d): FSCloseBatch...\n"),iErrorCode);
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

					//***

					if (iErrorCode1 != 0) goto lbFIN;
						
					if (iErrorCode1 == 0) {//F43092ClearWorkFiles OK, seguir...
						//*************************************************************************
						//Procesa F4312ClearWorkFile                                            ***
						//*************************************************************************				
						//Asignaciones para F4312ClearWorkFile:
						//· szComputerID									ok GetAuditInfo
						//· mnJobNumber										ok dsVarBuf[i]
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
						//· mnF411JobNumber									ok dsVarBuf[i]
						//· mnF0911JobNuber									ok dsVarBuf[i]

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
								MathCopy(&dsF4312ClearWorkFile.mnJobNumber,&dsVarBuf[i].mnF43121JobNumber);
								//· cErrorInClear				
								//· mnPOLineNumber		
								//· mnReceiptLineNumber	
								//· mnOrderNumber		
								//· szOrderType			
								//· szOrderCompany				
								//· szOrderSuffix		
								jdeStrcpy(dsF4312ClearWorkFile.szPOVersion,LszP4310VersionBuf);									
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

						//*************************************************************************
						//Procesa F4111ClearDetailStack                                         ***
						//*************************************************************************				
						//Asignaciones para F4111ClearDetailStack:
						//· szComputerID									ok GetAuditInfo
						//. mnFromLineNumber
						//. mnThruLineNumber
						//. mnPOReceiptNumber
						//. cClearHeaderFile
						//. mnF0911JobNumber
						//. cProcessCloseBatch								ok '1'
						//. mnBatchNumber
						//. cBatchStatus
						//. mnInvJobNumbe									ok F4312BeginDoc
						//· mnOrderNumber				No Asignado			--
						//· szOrderType					No Asignado			--
						//· szOrderKeyCompany			No Asignado			--

						if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Asignado dsF4111ClearDetailStack...\n"));

						iErrorCode1 = 0;

						for (i=0; i < iOCindex; i++){//Para cada OC, hacer si EndDoc...
							iErrorCode = 0;
							memset((void *) &dsF4111ClearDetailStack,(int) _J('\0'),sizeof(DSDXT4111Z1I));	
							if (dsVarBuf[i].iEditLineLines > 0) {//Esta OC tiene líneas...
								if (iDbgFlg > 0) {
									jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
									jdeFprintf(dlg,_J("***OC[%d] procesando dsF4111ClearDetailStack(%ls)...\n"),i,LszString01);
									jdeFflush(dlg);
								}

								jdeStrcpy(dsF4111ClearDetailStack.szComputerID,dsGetAuditInfo.szWorkstation_UserId);
								MathCopy(&dsF4111ClearDetailStack.mnInvJobNumber,&dsVarBuf[i].mnF43121JobNumber);
								dsF4111ClearDetailStack.cProcessCloseBatch = _J('1');

								idResult = jdeCallObject(_J("F4111ClearDetailStack"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsF4111ClearDetailStack,
														(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
								jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
								while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
									iErrorCode1 = 1;
									iErrorCode = 221;
									jdeFprintf(dlg,_J("***Error(%d): F4111ClearDetailStack: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
									jdeFflush(dlg);
								} 
								if ((idResult == ER_ERROR) && (iErrorCode == 0)){
									iErrorCode1 = 1;
									iErrorCode = 221;
									jdeFprintf(dlg,_J("***Error(%d): F4111ClearDetailStack...\n"),iErrorCode);
									jdeFflush(dlg);
								}
							}else{
								if (iDbgFlg > 0) {
									jdeToUnicode(LszString01,dsVarBuf[i].pddoco.String,DIM(LszString01),UTF8);
									jdeFprintf(dlg,_J("***OC[%d] obviando F4111ClearDetailStack(%ls)...\n"),i,LszString01);
									jdeFflush(dlg);
								}
							}
						}//for...

					}//if (iErrorCode == 0) F43092ClearWorkFiles OK, seguir...
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
	JDB_CloseView(hRequestV43092JA);
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

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit OWDCrr01(%d)...\n"), iErrorCode);
	jdeFflush(dlg);

	if (iDbgFlg == 1) jdeFclose(dlg);

	return iErrorCode;
}

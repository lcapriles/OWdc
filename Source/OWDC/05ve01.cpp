// 05ve01.cpp
//
//Creado:		Luis Capriles,		Fecha:	03/12/2003 
//Modificado:	Luis Capriles,		Fecha:	28/12/2009 - Conversiona Unicode.
//Modificado:	Luis Capriles,		Fecha:	28/12/2009 - Uso de curses para manejo de salida. 
//Modificado:	Luis Capriles,		Fecha:	08/02/2010 - Cambio orden datos detalle.
//Modificado:	Luis Capriles,		Fecha:	15/07/2014 - Cambio orden campos...
//
#include "ve01.h"
#include <stdio.h>
#include "jde.h"

#include "F4211.h" 

#include "n4200790.h"	// ShipConfirmEndDoc, F42UI05EditLine, F42UI05DeleteCache

#include "b9800100.h"	// GetAuditInfo
#include "B4000370.h"	// F40095GetDefaultBranch
#include "B0000130.h"	// RetrieveCompanyFromBusUnit
#include "B4000150.h"	// GetBranchConstants
#include "xf41021.h"	// VerifyAndGetItemLocation
#include "b4000310.h"	// FormatLocation
#include "b4001050.h"	// GetCrossReferenceFields
#include "b4101220.h"	// CalculateAvailability
#include "x4101.h"		// GetItemMasterByShortItem, GetItemMasterBy3rdItem, GetItemMasterBy2ndItem

void OWDCmp02 (int * primeraVez, char * pantallaTitulo, int camposOffset, int camposCantidad, 
				int statusOffset, int inicioEtiquetas, int inicioCampos, int ultimaLinea, int CReqTAB,
				int camposPosiciones[], char * camposEtiquetas[], char camposContenido[][128], int camposErrores[],
				char * pantallaStatusLine, int iDbgFlg, FILE * dlg); //Manejo de la pantalla de entrada datos...

void OWDCmp90 (int iDbgFlg, FILE * dlg); //Terminar Manejo de la pantalla curses...

int OWDCgbc(HENV hEnv, HUSER hUser, JCHAR LszUsrEntry[128], JCHAR LszLineItemBuf[26], MATH_NUMERIC * mnItemShortIDBuf,
			JCHAR LszUOMdefaultBuf[3], JCHAR LszUOMstdConv[3], JCHAR LszCrossRefTypeCodeBuf[3], int iUPClenBuf, int iSCClenBuf,
			JCHAR szItemDescription[31], int iGetItemMasterBy, JCHAR c3rdItemNoSymbol, int iDbgFlg, FILE * dlg); //Valida código producto...

int ProcesaCB(char * szCodigoBarra, char * szCBprod,char * szCBlote,char * szCBfecha,int iUPClenBuf,int iSCClenBuf,int iDbgFlg, FILE * dlg);

int FormateaFecha(char * szFechaI, JCHAR * LszFechaO);  


__declspec(dllexport) int OWDCve01(HENV hEnv,HUSER hUser)  
{

	HREQUEST			hRequest	= (HREQUEST)NULL;;
	LPJDEERROR_RECORD	ErrorRec	= NULL;
	ID					idResult;
	LPCG_BHVR			lpVoid		= NULL;
	LPCG_BHVR			lpVoid1		= NULL;
	LPBHVRCOM			lpBhvrCom	= NULL;
	LPBHVRCOM			lpBhvrCom1	= NULL;
	ERROR_EVENT_KEY		EventKeyLocal;

	DSD4200790A			dsF42UI05EditLine;
	DSD4200790B			dsShipConfirmEndDoc;
	DSD4200790C			dsF42UI05DeleteCache;
	DSD4000150			dsGetBranchConstants;
	DSD4000232			dsF40095GetDefaultBranch;
	DSD0000130			dsRetrieveCompanyFromBusUnit;
	DSDXF41021C			dsVerifyAndGetItemLocation;
	DSD4001050			dsGetCrossReferenceFields;
	DSD4000310A			dsFormatLocation;
	DSD9800100			dsGetAuditInfo;
	DSD4101220			dsCalculateAvailability;
	DSDX4101B			dsGetItemMasterByShortItem;
	
	typedef struct {
		JCHAR			sdkcoo[6];			//Order Company           
		MATH_NUMERIC	sddoco;				//Document(OrderNo, Invoice, etc)
		JCHAR			sddcto[3];			//Order Type
		MATH_NUMERIC	sdlnid;				//Line Number
		JCHAR			sdmcu[13];			//Business Unit
		MATH_NUMERIC	sditm;				//Short Number
		JCHAR			sdaitm[26];			//Identifier3rdItem
		JCHAR			sdlocn[21];			//Location
		JCHAR			sdlotn[31];			//Lot
		JCHAR			sddsc1[31];			//Description Line 1
		JCHAR			sdlnty[3];			//Line Type
		JCHAR			sduom[3];			//UOM as Input 
		MATH_NUMERIC	sdsoqs;				//Quantity Shipped
		MATH_NUMERIC	sdsobk;				//Units - Qty Backorderd/Held
		int				iStatusRegistro;	//0=no procesado, 1=visualizado, 2=procesado
		MATH_NUMERIC	mnCantEmbarcar;		//Units - Qty Acumulado de QuantityEntered para verificar < soqs
		MATH_NUMERIC	mnCantIntroPadre;	//Units - Qty QuantityEntered del padre.
		MATH_NUMERIC	mnLineNumberPadre;	//Line Number del padre
		MATH_NUMERIC	mnLineNumberHijo;	//Line Number del hijo
		JCHAR			szLocationPadre[21];//Location del padre
		JCHAR			szLotPadre[31];		//Lot del padre
		JCHAR			szUOMPadre[3];		//UOM as Input del padre

		int				iNumLotes;			//Flag Lote Padre/Hijo
	} stF4211ce01;

#define CacheSize 1024

	stF4211ce01			dsF4211Buf[CacheSize];
	NID					szTableIDF4211 = NID_F4211;
	ID					idIndexID5F4211 = ID_F4211_PICK_SLIP_NUMBER;
	NID					szF4211ColumnsArray[14] =	{NID_KCOO,NID_DOCO,NID_DCTO,NID_LNID,NID_MCU,NID_ITM,NID_AITM,
													NID_LOCN,NID_LOTN,NID_DSC1,NID_LNTY,NID_UOM,NID_SOQS,NID_SOBK};
	KEY5_F4211			dsF4211Key5;
	SELECTSTRUCT		lpSelectF4211[9];
	LPF4108				lpdsF4108 = (LPF4108) NULL;
	LPF4101				lpdsF4101 = (LPF4101) NULL;
	MATH_NUMERIC		mnTempBuf,mnTemp0Buf,mnTemp0001Buf,mnCantIntroducidaBuf,mnCantEmbarcarBuf,
						mnCantEmbarcarTemp,mnItemShortIDBuf;
	FILE * ini;
	FILE * dlg;

	JCHAR LszLinea[80],LszLin1[64],LszLin2[64],LszUsrEntry[128],

		 LszDocTransDateBuf[16],LszDocBranchPlantBuf[16],LszLineLocationBuf[21],
		 LszLineItemBuf[26],LszLineUMBuf[3],LszUOMdefaultBuf[3],LszUOMstdConv[3],LszLineLotBuf[31],
		 LszItemDescriptionBuf[31], LszUbicacionDfltBuf[21], LszLocationBuf[21],

		 LszP4205VersionNameBuf[16],LszStatusNextFromBuf[4],LszStatusNextThruBuf[4],LszCrossRefTypeCodeBuf[3],
		 LszEtiquetaCodigo[64],

		 LszString01[128],LszString02[64],LszTempBuf[128],* LszDummy0,

		 LcYesNoBuf,LcDecimalCharBuf,LcLotProcess,LcPreventOverShipping,LcShipFromNegative,LcTemp1Buf;

	int	 iPickSlipNumberBuf,iRecordsF4211Read,iLineaEscogida,iUPClenBuf,iSCClenBuf, iError1,iError2,
		 iErrorCode,iErrorCode1,iDbgFlg,iItemFoundInCache,iRecordsAvailCache,iEditLineLines,iCantLotesProc,iNumLotes,i,j, 
		 iPrimeraVez, iCamposOffset, iCamposCantidad, iStatusOffset, iInicioEtiquetas, iInicioCampos, iUltimaLinea, iCReqTAB,
		 iCamposPosiciones[64], iCamposErrores[64],iSalir,iGetItemMasterBy,iLineQtyBuf;

	int	 iIdxCo = 0, iIdxLoc = 0, iIdxDT = 0, iIdxDoc = 0,
		 iIdxProd = 0, iIdxCant = 0, iIdxLot = 0, iIdxUM = 0, iIdxFVcto = 0, iIdxDescr = 0;

	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128],
		 * szDummy1,szTempBuf[128];

	double fLineQtyBuf,fLineQty1Buf;

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
	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCce01_D"));
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
	
	ini = jdeFopen(_J("OWDCce01.ini"),_J("r"));
	if (!ini){
		jdeFprintf(dlg,_J("***Error abriendo INI (OWDCce01.ini)...\n"));
        return 300;
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
			iDbgFlg = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Debug (%d)...\n"),iDbgFlg);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("DecimalChar")) == 0){
			if (jdeStrcmp(LszLin2,_J(",")) == 0) LcDecimalCharBuf = _J(','); else  LcDecimalCharBuf = _J('.');
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: DecimalChar (%lc)...\n"),LcDecimalCharBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("Yes")) == 0){
			LcYesNoBuf = LszLin2[0];
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Yes (%lc)...\n"),LcYesNoBuf);
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
		if(jdeStrcmp(LszLin1,_J("FechaTran")) == 0){
			jdeStrcpy(LszDocTransDateBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: FechaTran (%ls)...\n"),LszDocTransDateBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("P4205VersionName")) == 0){
			jdeStrcpy(LszP4205VersionNameBuf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P4205VersionName (%ls)...\n"),
										LszP4205VersionNameBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("StatusNextFrom")) == 0){
			jdeStrcpy(LszStatusNextFromBuf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: StatusNextFrom (%ls)...\n"),
										LszStatusNextFromBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("StatusNextThru")) == 0){
			jdeStrcpy(LszStatusNextThruBuf,LszLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: StatusNextThru (%ls)...\n"),
										LszStatusNextThruBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("UbicacionDflt")) == 0){
			jdeStrcpy(LszUbicacionDfltBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: UbicacionDflt (%ls)...\n"),LszUbicacionDfltBuf);
			continue;
		};

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
			jdeStrcpy(LszCrossRefTypeCodeBuf,LszLin2);			
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
		if(jdeStrcmp(LszLin1,_J("PreventOverShipping")) == 0){
			LcPreventOverShipping = LszLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: PreventOverShipping (%lc)...\n"),
										LcPreventOverShipping);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("ShipFromNegative")) == 0){
			LcShipFromNegative = LszLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ShipFromNegative (%lc)...\n"),
										LcShipFromNegative);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("LineQty")) == 0){
			iLineQtyBuf = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LineQty (%d)...\n"),
										iLineQtyBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("GetItemMasterBy")) == 0){
			iGetItemMasterBy = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: GetItemMasterBy (%d)...\n"),
										iGetItemMasterBy);
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
	//ShipConfirmEndDoc, F42UI05EditLine
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
	memset((void *) &dsCalculateAvailability,(int) _J('\0'),sizeof(DSD4101220));
	memset((void *) &dsGetCrossReferenceFields,(int) _J('\0'),sizeof(DSD4001050));
	memset((void *) &dsFormatLocation,(int)(_J('\0')),sizeof(DSD4000310A));
	memset((void *) &dsCalculateAvailability,(int) _J('\0'),sizeof(DSD4101220));
	
	if (LcDecimalCharBuf == _J(',')) ParseNumericString(&mnTemp0001Buf,_J("0,001"));
	else ParseNumericString(&mnTemp0001Buf,_J("0.001"));

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Determina Almacen, Compania, Maquina, Fecha y ProcOpt...\n"));
	//***Obtiene Uasuario, máquina, fecha	
	idResult = jdeCallObject(_J("GetAuditInfo"),NULL,lpBhvrCom,lpVoid,&dsGetAuditInfo,
					(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
	if (idResult == ER_ERROR){
		iErrorCode = 316;
		jdeFprintf(dlg,_J("***Error(%d): GetAuditInfo:...\n"));
		jdeFflush(dlg);
		goto lbFIN;
	} 

	//***Busca Almacén por default del usuario
	idResult = jdeCallObject(_J("GetDefaultBranch"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsF40095GetDefaultBranch, 
						(CALLMAP *)NULL, (int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 317;
		jdeFprintf(dlg,_J("***Error(%d): GetDefaultBranch:...\n"));
		jdeFflush(dlg);
		goto lbFIN;
	}
	jdeStrcpy(LszDocBranchPlantBuf,dsF40095GetDefaultBranch.szBranch);
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetDefaultBranch (%ls)...\n"),LszDocBranchPlantBuf);

	//***Busca Compañía de Centro de Costo
	jdeStrcpy(dsRetrieveCompanyFromBusUnit.szCostCenter,LszDocBranchPlantBuf);
	idResult = jdeCallObject(_J("RetrieveCompanyFromBusUnit"),NULL,lpBhvrCom,lpVoid,
						(LPVOID)&dsRetrieveCompanyFromBusUnit,(CALLMAP *)NULL,(int)0, 
						(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 318;
		jdeFprintf(dlg,_J("***Error(%d): RetrieveCompanyFromBusUnit:...\n"));
		jdeFflush(dlg);
		goto lbFIN;
	}
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***RetrieveCompanyFromBusUnit (%ls)...\n"),
								dsRetrieveCompanyFromBusUnit.szCompany);
	
	//***Busca Decimales de la Cantidad	...
//	jdeStrcpy(dsGetDataDictionaryDefinition.szDataDictionaryItem,_J("UORG"));
//	idResult = jdeCallObject(_J("GetDataDictionaryDefinition"),NULL,lpBhvrCom,lpVoid,
//						(LPVOID)&dsGetDataDictionaryDefinition,(CALLMAP *)NULL,(int)0, 
//						(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
//	if (idResult == ER_ERROR){
//		iErrorCode = 319;
//		jdeFprintf(dlg,_J("***Error(%d): GetDataDictionaryDefinition:...\n"), iErrorCode);
//		jdeFflush(dlg);
//		goto lbFIN;
//	} 
//	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetDataDictionaryDefinition (%ls)...\n"),dsGetDataDictionaryDefinition.mnDisplayDecimals.String);
//	MathNumericToInt(&dsGetDataDictionaryDefinition.mnDisplayDecimals, &iCantDecQty); ***No funciona en Rattan
//	iCantDecQty = atoi(dsGetDataDictionaryDefinition.mnDisplayDecimals.String);

	//*************************************************************************
	//***Solicita Datos de Entrada                                          ***
	//*************************************************************************

	if (iDbgFlg > 0)jdeFprintf(dlg,_J("***Inicializando 1ra pantalla datos entrada...\n"));

	memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
	memset(szCamposContenido,'\0',sizeof(szCamposContenido));
	memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
	memset(iCamposErrores,'\0',sizeof(iCamposErrores));
	memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));

	iPrimeraVez = 0;

	strcpy (szPantallaTitulo, "Captura Despacho Almacen");

	szCamposEtiquetas[0] = "F. Transaccion ";
	szCamposEtiquetas[1] = "Ubicacion      ";

	iCamposCantidad = 2;//dos campos a desplegar, por ahora...

	//*** Fecha Transaccion
	if (jdeStrcmp(LszDocTransDateBuf,_J("0")) == 0) FormatDate(LszDocTransDateBuf,&dsGetAuditInfo.jdDate,(JCHAR*) NULL);
	jdeFromUnicode(szCamposContenido[0],LszDocTransDateBuf,11,UTF8);
	iCamposPosiciones[0] = strlen(szCamposContenido[0]);

	//Modificado uis Capriles,15/07/2014 - Cambio orden campos... INICIO
	//***Ubicacion
	jdeFromUnicode(szCamposContenido[1],LszUbicacionDfltBuf,21,UTF8);//La ubicación del F4211...
	iCamposPosiciones[1] = strlen(szCamposContenido[1]);
/*
	//*** Almacen  por defecto del usuario
	jdeFromUnicode(szCamposContenido[1],LszDocBranchPlantBuf,12,UTF8);
	iCamposPosiciones[1] = strlen(szCamposContenido[1]);
*/
		//Modificado uis Capriles,15/07/2014 - Cambio orden campos... FIN
	do {  //Loop para validar ubicación y fechas...
	
		memset(LszDocTransDateBuf,'\0',sizeof(LszDocTransDateBuf));
		memset(LszLocationBuf,'\0',sizeof(LszLocationBuf));

		OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
					iInicioCampos, iUltimaLinea, iCReqTAB,
					iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
					iDbgFlg, dlg);

		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,' ',sizeof(szPantallaStatusLine));
		szPantallaStatusLine[sizeof(szPantallaStatusLine) - 1] = '\0';
		
		//***Fecha Transaccion
		//jdeToUnicode(LszDocTransDateBuf,szCamposContenido[0],11,UTF8);
		//if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszDocTransDateBuf (%ls)...\n"),LszDocTransDateBuf);
		iErrorCode = FormateaFecha(szCamposContenido[0],LszDocTransDateBuf);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszDocTransDateBuf(%ls) ...\n"),LszDocTransDateBuf);
		if (iErrorCode != 0) {// Indica que la fecha es nula...
			iCamposErrores[0] = 1; // Seteamos el error...
			jdeFprintf(dlg,_J("***Error(%d): Fecha Transaccion Invalida (%s)...\n"),iErrorCode,LszDocTransDateBuf);
		}

		//Modificado uis Capriles,15/07/2014 - Cambio orden campos... INICIO
/*
		jdeToUnicode(LszString01,szCamposContenido[1],13,UTF8);//Sacamos una copia para comparar...
		LszString01[12]=_J('\0');
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszDocBranchPlantBuf (%ls)...\n"),LszString01);
		//***Valida almacén
		dsGetBranchConstants.cReturnPtr = _J('0'); //Solo validar, no queremos data...
		dsGetBranchConstants.cCallType = _J('1'); //Devolver Error si no hay registro...
		jdeErrorClearEx(lpBhvrCom,lpVoid);

		if (jdeStrcmp(LszString01,LszDocBranchPlantBuf) == 0 ) break; //No se cambió el default...
		
		//BranchPlant Right Justified padded with blanks
		jdeStrcpy(LszTempBuf, _J("            "));
		jdeStrcpy(LszTempBuf + (12 - jdeStrlen(LszString01)),LszString01) ;
		jdeStrcpy(dsGetBranchConstants.szBranchPlant,LszTempBuf);
	
		jdeCallObject(_J("GetBranchConstants"),NULL, lpBhvrCom,lpVoid,
							(LPVOID)&dsGetBranchConstants,(CALLMAP *)NULL,(int)0, 
							(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
		jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
		while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){
			iErrorCode = 301;
			jdeFprintf(dlg,_J("***Error(%d): GetBranchConstants: (%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc);
			jdeFflush(dlg);
			iCamposErrores[1] = 1; // Seteamos el error...
		} 
		if (iErrorCode != 0) continue;

		jdeStrcpy(LszDocBranchPlantBuf,LszTempBuf);
		jdeErrorClearEx(lpBhvrCom,lpVoid);
		//Actualiza Compañía de Centro de Costo recién introducido
		jdeStrcpy(dsRetrieveCompanyFromBusUnit.szCostCenter,LszDocBranchPlantBuf);
		jdeCallObject(_J("RetrieveCompanyFromBusUnit"),NULL,lpBhvrCom,lpVoid,
							(LPVOID)&dsRetrieveCompanyFromBusUnit,(CALLMAP *)NULL,(int)0, 
							(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
		jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
		while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){
			iErrorCode = 302;
			jdeFprintf(dlg,_J("***Error(%d): RetrieveCompanyFromBusUnit: (%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc);
			jdeFflush(dlg);
			iCamposErrores[1] = 1; // Seteamos el error...
		} 
*/
		//*** Ubicacion donde se registrara el Despacho...
		jdeToUnicode(LszLocationBuf,szCamposContenido[1],21,UTF8);//Se copia para validar la Ubicacion Ingresada...
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
			iErrorCode = 331;
			iCamposErrores[1] = 1; // Seteamos el error...
			jdeFprintf(dlg,_J("***Error(%d): VerifyAndGetItemLocation: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
			jdeFflush(dlg);
		}
		//Modificado uis Capriles,15/07/2014 - Cambio orden campos... FIN

	} while (iErrorCode != 0);

	OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 1ra pantalla para mostrar la 2da...

	// Determina Símbolo para identificar 3rd Inv Number...
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Determina dsGetCrossReferenceFields...\n"));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szSystemCode,(const JCHAR *)_J(" "));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szBranchPlant,(const JCHAR *)LszDocBranchPlantBuf);
	dsGetCrossReferenceFields.cSuppressErrorMsg = _J('1');
	idResult = jdeCallObject(_J("GetCrossReferenceFields"), NULL,lpBhvrCom,lpVoid,
	                      (LPVOID)&dsGetCrossReferenceFields,(LPCALLMAP)NULL,(int)0,
						  (JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if((idResult == ER_ERROR) || (jdeStrcmp(dsGetCrossReferenceFields.szErrorMsgID,_J(" ")) != 0)){
		iErrorCode = 303;
		jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
		while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){				
			jdeFprintf(dlg,_J("***Error(%d): GetCrossReferenceFields: (%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc);
			jdeFflush(dlg);
		} 
		goto lbFIN;
	}
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***dsGetCrossReferenceFields (%lc)...\n"),dsGetCrossReferenceFields.c3rdItemNoSymbol);


	//*************************************************************************
	//***Carga en Cache Registros de Pick Slip                              ***
	//*************************************************************************
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Carga en Cache Registros de Pick Slip...\n"));
	//Abre la tabla F4211
	memset((void *)(&dsF4211Key5),(int)(_J('\0')),sizeof(dsF4211Key5));

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Abriendo Tabla F4211...\n"));
	idResult = JDB_OpenTable (hUser,NID_F4211,ID_F4211_PICK_SLIP_NUMBER,szF4211ColumnsArray,
	                               (ushort)(14),(JCHAR *)NULL,&hRequest);
	if (idResult == JDEDB_FAILED){
		iErrorCode = 304;
		jdeFprintf(dlg,_J("***Error(%d): JDB_OpenTable (F4211) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	}

	//Construye Where del select...
	JDB_ClearSelection(hRequest);

	//***Se seleccionan registros que cumplan con:
	ZeroMathNumeric(&mnTemp0Buf);
	LcTemp1Buf = _J('1');

	jdeNIDcpy(lpSelectF4211[1].Item1.szDict, NID_NXTR);//Status Next >= szStatusNextFrom
	jdeNIDcpy(lpSelectF4211[1].Item1.szTable, NID_F4211);
	lpSelectF4211[1].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF4211[1].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF4211[1].Item2.szTable, _J(""));
	lpSelectF4211[1].Item2.idInstance = (ID)0;
	lpSelectF4211[1].lpValue = LszStatusNextFromBuf;
	lpSelectF4211[1].nValues = (short)1;
	lpSelectF4211[1].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF4211[1].nCmp = JDEDB_CMP_GE;

	jdeNIDcpy(lpSelectF4211[2].Item1.szDict, NID_NXTR);//Status Thru <= szStatusNextThru
	jdeNIDcpy(lpSelectF4211[2].Item1.szTable, NID_F4211);
	lpSelectF4211[2].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF4211[2].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF4211[2].Item2.szTable, _J(""));
	lpSelectF4211[2].Item2.idInstance = (ID)0;
	lpSelectF4211[2].lpValue = LszStatusNextThruBuf;
	lpSelectF4211[2].nValues = (short)1;
	lpSelectF4211[2].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF4211[2].nCmp = JDEDB_CMP_LE;
	
	jdeNIDcpy(lpSelectF4211[3].Item1.szDict, NID_SOQS);//Cantidad > 0
	jdeNIDcpy(lpSelectF4211[3].Item1.szTable, NID_F4211);
	lpSelectF4211[3].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF4211[3].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF4211[3].Item2.szTable, _J(""));
	lpSelectF4211[3].Item2.idInstance = (ID)0;
	lpSelectF4211[3].lpValue = &mnTemp0Buf;
	lpSelectF4211[3].nValues = (short)1;
	lpSelectF4211[3].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF4211[3].nCmp = JDEDB_CMP_GT;

	jdeNIDcpy(lpSelectF4211[4].Item1.szDict, NID_SONE);//Cantidad Futura == 0
	jdeNIDcpy(lpSelectF4211[4].Item1.szTable, NID_F4211);
	lpSelectF4211[4].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF4211[4].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF4211[4].Item2.szTable, _J(""));
	lpSelectF4211[4].Item2.idInstance = (ID)0;
	lpSelectF4211[4].lpValue = &mnTemp0Buf;
	lpSelectF4211[4].nValues = (short)1;
	lpSelectF4211[4].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF4211[4].nCmp = JDEDB_CMP_EQ;

	jdeNIDcpy(lpSelectF4211[5].Item1.szDict, NID_SO02);//SO status 02 != '1'
	jdeNIDcpy(lpSelectF4211[5].Item1.szTable, NID_F4211);
	lpSelectF4211[5].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF4211[5].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF4211[5].Item2.szTable, _J(""));
	lpSelectF4211[5].Item2.idInstance = (ID)0;
	lpSelectF4211[5].lpValue = &LcTemp1Buf;
	lpSelectF4211[5].nValues = (short)1;
	lpSelectF4211[5].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF4211[5].nCmp = JDEDB_CMP_NE;

	jdeNIDcpy(lpSelectF4211[6].Item1.szDict, NID_SWMS);//In WareHouse != '1'
	jdeNIDcpy(lpSelectF4211[6].Item1.szTable, NID_F4211);
	lpSelectF4211[6].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF4211[6].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF4211[6].Item2.szTable, _J(""));
	lpSelectF4211[6].Item2.idInstance = (ID)0;
	lpSelectF4211[6].lpValue = &LcTemp1Buf;
	lpSelectF4211[6].nValues = (short)1;
	lpSelectF4211[6].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF4211[6].nCmp = JDEDB_CMP_NE;

	//Modificado Luis Capriles,15/07/2014 - Cambio orden campos... INICIO
	/*
	jdeNIDcpy(lpSelectF4211[7].Item1.szDict, NID_MCU);//BranchPlant == LszDocBranchPlantBuf
	jdeNIDcpy(lpSelectF4211[7].Item1.szTable, NID_F4211); 
	lpSelectF4211[7].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF4211[7].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF4211[7].Item2.szTable, _J(""));
	lpSelectF4211[7].Item2.idInstance = (ID)0;
	lpSelectF4211[7].lpValue = LszDocBranchPlantBuf;
	lpSelectF4211[7].nValues = (short)1;
	lpSelectF4211[7].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF4211[7].nCmp = JDEDB_CMP_EQ;
	*/
	//Modificado Luis Capriles,15/07/2014 - Cambio orden campos... FIN

	do{ //while (TRUE);

		//***Allocate and set data structures
		memset(dsF4211Buf,'\0',sizeof(dsF4211Buf));
		memset((void *) &dsF42UI05DeleteCache,(int) _J('\0'),sizeof(DSD4200790C));
		memset((void *) &dsShipConfirmEndDoc,(int) _J('\0'),sizeof(DSD4200790B));	
		memset((void *) &dsF42UI05EditLine,(int) _J('\0'),sizeof(DSD4200790A));

		ZeroMathNumeric(&dsF42UI05EditLine.mnJobnumberA);

		iSalir = 0;

		if (iDbgFlg > 0) {
			jdeFprintf(dlg,_J("***Inicializando 2da pantalla datos entrada...\n"));
			jdeFflush(dlg);
		}

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));

		iPrimeraVez = 0;

		strcpy (szPantallaTitulo, "Captura Despacho Almacen");

		szCamposEtiquetas[0] = "Nro.PickSlip   ";

		iCamposCantidad = 1;//un campos a desplegar, por ahora...

		//***PickSlip...
		//szCamposContenido[0]
	
		do {  //while (iRecordsF4211Read == 0); Loop para obtener un pick slip correcto...
			if (iDbgFlg > 0) {
				jdeFprintf(dlg,_J("***Loop Carga en Cache Registros de Pick Slip...\n"));
				jdeFflush(dlg);
			}

			iErrorCode = 0;
			iPickSlipNumberBuf = 0;

			OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
							iInicioCampos, iUltimaLinea, iCReqTAB,
							iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
							iDbgFlg, dlg);

			memset(iCamposErrores,'\0',sizeof(iCamposErrores));
			memset(szPantallaStatusLine,' ',sizeof(szPantallaStatusLine));
			szPantallaStatusLine[sizeof(szPantallaStatusLine) - 1] = '\0';

			//***PickSlip
			if (strlen(szCamposContenido[0]) == 0) {
				iSalir = 1;
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Break de PickSlip...\n"));
					jdeFflush(dlg);
				}
				break;
			} 
			else iSalir = 0;

			strcpy(szDummy,szCamposContenido[0]);//Sacamos una copia para usarla en la 4ta pantalla...
			sscanf(szCamposContenido[0],"%d",&iPickSlipNumberBuf);
			IntToMathNumeric(iPickSlipNumberBuf,&mnTempBuf);
			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***iPickSlipNumberBuf (%d)...\n"),iPickSlipNumberBuf);
			
			//Actualiza la clausula Where con Número de Pick Slip...
			jdeNIDcpy(lpSelectF4211[0].Item1.szDict, NID_PSN);//Pick Slip Number == mnTempBuf
			jdeNIDcpy(lpSelectF4211[0].Item1.szTable, NID_F4211);
			lpSelectF4211[0].Item1.idInstance = (ID)0;
			jdeNIDcpy(lpSelectF4211[0].Item2.szDict, _J(""));
			jdeNIDcpy(lpSelectF4211[0].Item2.szTable, _J(""));
			lpSelectF4211[0].Item2.idInstance = (ID)0;
			lpSelectF4211[0].lpValue = &mnTempBuf;
			lpSelectF4211[0].nValues = (short)1;
			lpSelectF4211[0].nAndOr = JDEDB_ANDOR_AND;
			lpSelectF4211[0].nCmp = JDEDB_CMP_EQ;
			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Construyendo Selection Tabla F4211...\n"));
			idResult = JDB_SetSelection(hRequest,lpSelectF4211,(short)(7),JDEDB_SET_REPLACE);
			if (idResult == JDEDB_FAILED){
				iErrorCode = 305;
				jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection (F4211) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}
			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F4211...\n"));
			idResult = JDB_SelectKeyed(hRequest,(ID) 0,(void *)NULL,(short)0);
			if (idResult == JDEDB_FAILED){
				iErrorCode = 306;
				jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed (F4211) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}
			
			iRecordsF4211Read = 0;
			do {//Carga en Cache los regitros del Pick Slip escogido...
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla F4211...\n"));		
				idResult = JDB_Fetch(hRequest,&dsF4211Buf[iRecordsF4211Read],(int)0); 

				if (idResult == JDEDB_FAILED)jdeFprintf(dlg,_J("***JDB_Fetch (EOF F4211) (%d) ...\n"),iRecordsF4211Read);
				else {
					if (iDbgFlg > 0){
						jdeFprintf(dlg,_J("***%d)Producto (%ls-%ls) Ubicacion(%ls)...\n"),iRecordsF4211Read,
								dsF4211Buf[iRecordsF4211Read].sdaitm,dsF4211Buf[iRecordsF4211Read].sddsc1,
								dsF4211Buf[iRecordsF4211Read].sdlocn); 
						jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls) MCU(%ls)...\n"),iRecordsF4211Read,
								dsF4211Buf[iRecordsF4211Read].sdkcoo, dsF4211Buf[iRecordsF4211Read].sdmcu);
						jdeToUnicode(LszString01,dsF4211Buf[iRecordsF4211Read].sddoco.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iRecordsF4211Read,
								LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iRecordsF4211Read,
								dsF4211Buf[iRecordsF4211Read].sddcto);
						jdeToUnicode(LszString01,dsF4211Buf[iRecordsF4211Read].sdlnid.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iRecordsF4211Read,
								LszString01);
						jdeToUnicode(LszString01,dsF4211Buf[iRecordsF4211Read].sdsoqs.String,DIM(LszString01),UTF8);
						jdeFprintf(dlg,_J("***Cache[%d] Quantity Shipped(%ls)...\n"),iRecordsF4211Read,
								LszString01);
						jdeFprintf(dlg,_J("***Cache[%d] UOM As Input(%ls)...\n"),iRecordsF4211Read,
								dsF4211Buf[iRecordsF4211Read].sduom);
						jdeFflush(dlg);
					}
					dsF4211Buf[iRecordsF4211Read].iStatusRegistro = 0;
					iRecordsF4211Read++;
					if (iRecordsF4211Read == CacheSize){
						iErrorCode = 307;
						jdeFprintf(dlg,_J("***Error(%d): iRecordsF4211Read > CacheSize!!!\n"),iErrorCode);
						jdeFflush(dlg);
						goto lbFIN;
					}else 
						iRecordsAvailCache = iRecordsF4211Read;
				}
			} while (idResult != JDEDB_FAILED);
		} while (iRecordsF4211Read == 0);//No se pudo leer ningún registro con ese PickSlip->Solicitar otro...
		
		if (iPickSlipNumberBuf == 0){
			if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Break de PickSlip: (iPickSlipNumbreBuf == 0)...\n"));
					jdeFflush(dlg);
				}
			goto lbFIN; //No hay Pick Slip!!! Equivalente a iSalir == 1
		}

		//*************************************************************************
		//***Procesa Registros del Pick Slip                                    ***
		//*************************************************************************
		iEditLineLines = 0; // No hay líneas en el documento de salida...
		
		do { //while (iRecordsAvailCache > 0); Procesar mientras haya registros en el Cache... Cambiado a TRUE...

			OWDCmp90 (iDbgFlg, dlg);//Limpiamos para mostrar la 3ra...

			iErrorCode = 0;
			iErrorCode1 =0;

			do {//while (iItemFoundInCache == 0); Solicita producto a procesar...
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Inicializando 3ra pantalla datos entrada...\n"));
					jdeFflush(dlg);
				}

				memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
				memset(szCamposContenido,'\0',sizeof(szCamposContenido));
				memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
				memset(iCamposErrores,'\0',sizeof(iCamposErrores));
				memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));	
				
				iPrimeraVez = 0;

				strcpy (szPantallaTitulo, "Captura Despacho Almacen");

				szCamposEtiquetas[0] = "Codigo Barra  ";

				iCamposCantidad = 1;//un  campo a desplegar...	
				
				memset(LszEtiquetaCodigo,'\0',sizeof(LszEtiquetaCodigo));

//				do { //Loop para validar codigo barra... }
				iErrorCode = 0;
				memset(LszLineItemBuf,'\0',sizeof(LszLineItemBuf));
				memset(LszLineLotBuf,'\0',sizeof(LszLineLotBuf));
				memset(LszLineUMBuf,'\0',sizeof(LszLineUMBuf));

				OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
								iInicioCampos, iUltimaLinea, iCReqTAB,
								iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
								iDbgFlg, dlg);

				memset(iCamposErrores,'\0',sizeof(iCamposErrores));
				memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));

				//***Codigo Etiqueta:(Producto/lote/UM/linea) 
				//01=Codigo producto
				//10=Lote
				//17=Fecha vencimiento
				if (strlen(szCamposContenido[0]) == 0) {
						iSalir++;
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
				//iIdxProd = 0; iIdxLot = 1; iIdxFVcto = 9;
				//iIdxProd = 0; iIdxUM = 2; iIdxCant = 3; iIdxLot = 4; 
				iIdxProd = 0; iIdxDescr = 1; iIdxCant = 2; iIdxLot = 3; 
				strcpy(szTempBuf,szCamposContenido[0]);

				//Si la etiqueta está defectuosa, ó no se encuentra el CB ó no está en el cache->error..
				iErrorCode1 = ProcesaCB(szTempBuf,szCamposContenido[iIdxProd],szCamposContenido[iIdxLot],szCamposContenido[iIdxFVcto],
					iUPClenBuf,iSCClenBuf,iDbgFlg,dlg);
			
				if (iErrorCode1 == 1){
					iCamposErrores[0] = 1; // Seteamos el error...
					iErrorCode = 308;
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
											iDbgFlg,dlg)) != 0) iCamposErrores[0] = 1; //Seteamos el error... Codigo malo
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
							iErrorCode = 332;
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

				//Modificado uis Capriles,15/07/2014 - Cambio orden campos... INICIO
				/*
				} while (iErrorCode != 0); //Loop para validar codigo barra...
				if (iSalir > 0){
					if (iDbgFlg > 0){
								jdeFprintf(dlg,_J("***Break de Etiqueta 2...\n"));
								jdeFflush(dlg);
							};
					break; //Queremos subir a perdir otro PickSlip!!!!
				}
				*/
				//Modificado Luis Capriles,15/07/2014 - Cambio orden campos... FIN

				iItemFoundInCache = 0;
				iError1 = 1; 
				 
				for (i = 0;((i < iRecordsF4211Read) && (iItemFoundInCache == 0) && (iErrorCode == 0)); i++){//Ahora queremos encontrar el producto en el pedido...					
					if ((jdeStrcmp(LszLineItemBuf,dsF4211Buf[i].sdaitm) == 0) && 
						(dsF4211Buf[i].iStatusRegistro != 2)) iError1 = 0; //Mostrar registros no procesados
						
					if (iError1 == 0) { 
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***dsF4211Buf[%d].sdaitm (%ls)-Primer Paso...\n"),i,dsF4211Buf[i].sdaitm);
							jdeFflush(dlg);
						}
						iItemFoundInCache++;
					}
				}
				if (iError1 == 1) {//No se encontró en el cache
					iCamposErrores[iIdxProd] = 1; //Seteamos el error... Codigo malo
				}

				if (iItemFoundInCache == 0) { //No se encontró en el cache
					iErrorCode = 333;		
					jdeFprintf(dlg,_J("***Error(%d): No se encontró en el cache-Primer Paso...\n"),iErrorCode);
					jdeFflush(dlg);
				}

			//Modificado uis Capriles,15/07/2014 - Cambio orden campos... INICIO
			} while (iErrorCode != 0); //Loop para validar codigo barra...
			if (iSalir > 0){
				if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***Break de Etiqueta 2...\n"));
							jdeFflush(dlg);
						};
				break; //Queremos subir a perdir otro PickSlip!!!!
			}

			/*
			if (iSalir > 0){
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Break de Etiqueta 3...\n"));
					jdeFflush(dlg);
				};
				break;//Queremos subir a perdir otro PickSlip!!!!
			}
			

			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***iLineaEscogida(%d)...\n"),iLineaEscogida);
				jdeFflush(dlg);
			}
			*/
			//Modificado Luis Capriles,15/07/2014 - Cambio orden campos... FIN

			//OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 3da pantalla para mostrar la 4ta...

			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Inicializando 4ta pantallas datos entrada detalle...\n"));
				jdeFflush(dlg);
			}

			memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
			//memset(szCamposContenido,'\0',sizeof(szCamposContenido));
			memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
			memset(iCamposErrores,'\0',sizeof(iCamposErrores));
			memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));

			//iPrimeraVez = 8;//Solo pintar!!!! Sin inverse video...
			iPrimeraVez = 0;

			strcpy (szPantallaTitulo, "Captura Despacho Almacen-Detalle    ");

			/*
			//szCamposEtiquetas[0] = "PickSlip....";
			//szCamposEtiquetas[1] = " ";
			//szCamposEtiquetas[2] = "Producto....";
			//szCamposEtiquetas[3] = "Lote...";
			//szCamposEtiquetas[4] = "UM...";
			szCamposEtiquetas[0] = "Producto       ";
			szCamposEtiquetas[1] = "Lote           ";
			szCamposEtiquetas[2] = "UM             ";

			iCamposCantidad = 3;//Tres campos a desplegar, por ahora...!!!
			*/
			szCamposEtiquetas[0] = "Producto       ";
			szCamposEtiquetas[1] = "Desc./NART     ";
			szCamposEtiquetas[2] = "Cantidad       ";
			szCamposEtiquetas[3] = "Lote           ";


			iCamposCantidad = 4; // Se Desplegaran 4 Campos: Producto, Cantidad, Lote
			iIdxProd = 0; iIdxDescr = 1; iIdxCant = 2; iIdxLot = 3; 
			//Modificado Luis Capriles,15/07/2014 - Cambio orden campos... FIN
			memset((void *) &LszLineItemBuf,(int) _J('\0'),sizeof(LszLineItemBuf));//Limpiamos Producto por primera vez....
			memset((void *) &LszLineLotBuf,(int) _J('\0'),sizeof(LszLineLotBuf));//Limpiamos Lote por primera vez....
			memset((void *) &mnCantIntroducidaBuf,(int) _J('\0'),sizeof(MATH_NUMERIC));	//Limpiamos Cantidad...

			do {//while (iItemFoundInCache == 0); Loop para Solicitar producto a procesar... 

				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Inicializando 3ra pantalla datos entrada por Línea...\n"));
					jdeFflush(dlg);
				}

				iPrimeraVez = 0;
				iLineaEscogida = 0;
				iErrorCode = 0;

				do { //Loop para validar codigo barra y demás..

					//***PickSlip
					//strcpy(szCamposContenido[0],szDummy);//Nos recordamos del PickSlip...
					//iCamposPosiciones[0] = strlen(szCamposContenido[0]);
					//strcpy(szCamposContenido[1],"");// Una línea en blanco...

					//***Producto
					//jdeFromUnicode(szCamposContenido[iIdxProd],LszLineItemBuf,25,UTF8);
					iCamposPosiciones[iIdxProd] = strlen(szCamposContenido[iIdxProd]);
					iCamposPosiciones[iIdxDescr] = strlen(szCamposContenido[iIdxDescr]);
					iCamposErrores[iIdxDescr] = 3;//No queremos inverse video...

					//***Cantidad									
					sprintf(szCamposContenido[iIdxCant],"%d",iLineQtyBuf);
					iCamposPosiciones[iIdxCant] = strlen(szCamposContenido[iIdxCant]);
					iCamposErrores[iIdxCant] = 2; //Nos queremos posicionar acá...

					//***Lote 
					iCamposPosiciones[iIdxLot] = strlen(szCamposContenido[iIdxLot]);

					//***UM
					//jdeFromUnicode(szCamposContenido[iIdxUM],LszUOMdefaultBuf,2,UTF8);
					//iCamposPosiciones[iIdxUM] = strlen(szCamposContenido[iIdxUM]);

lbEditLineERR: //Por ahora tenemos una etiqueta..

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

				for (i = 0;(i < iRecordsF4211Read) && (iItemFoundInCache == 0); i++){
					iError1 = 1; iError2 = 1;
					if ((jdeStrcmp(LszLineItemBuf,dsF4211Buf[i].sdaitm) == 0) && 
						(dsF4211Buf[i].iStatusRegistro != 2)) iError1 = 0; //Mostrar registros no procesados...

					if ((iError1 == 0)){
						if (iDbgFlg > 0){
							jdeFprintf(dlg,_J("***Encontrado dsF4211Buf[%d].sdaitm 1-Segundo Paso (%ls)...\n"),i,dsF4211Buf[i].sdaitm); 
							jdeFflush(dlg);
						}
						j = i;
						iItemFoundInCache++;
					}
				}
				
				if (iError1 == 0){//Lo encontramos.. vamos a validar la cantidad...

	//Modificado:	Luis Capriles,		Fecha:	08/02/2010 - Cambio orden datos detalle.
	//FIN Modificación - 08/02/2010	
					
					//***Proceso  Lotes...
					iNumLotes = dsF4211Buf[j].iNumLotes;//Flag de Lotes...
					iErrorCode = 0;
					iErrorCode1 = 0;

					if(LcLotProcess == _J('1')){ //Procesamiento de Lotes...
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***Procesando Lotes (flg:%d)...\n"),iNumLotes);
							jdeFflush(dlg);
						}
							
						iErrorCode = 0;
						jdeErrorClearEx(lpBhvrCom,lpVoid);

						MathCopy(&dsCalculateAvailability.mnShortItemNumber,&dsF4211Buf[j].sditm);
						jdeStrcpy(dsCalculateAvailability.szBranchPlant,dsF4211Buf[j].sdmcu);
						jdeStrcpy(dsCalculateAvailability.szLocation,LszLineLocationBuf);
						jdeStrcpy(dsCalculateAvailability.szLotNumber,LszLineLotBuf);
						jdeStrcpy(dsCalculateAvailability.szPrimaryUOM,LszUOMdefaultBuf);
						jdeStrcpy(dsCalculateAvailability.szSecondaryUOM,LszUOMdefaultBuf);
						jdeStrcpy(dsCalculateAvailability.szStandardUOMFlag,LszUOMstdConv);
						dsCalculateAvailability.cSuppressErrorMessages = _J('0');
						ParseNumericString(&dsCalculateAvailability.mnIndex,_J("1"));					
						ParseNumericString(&dsCalculateAvailability.mnKeys,_J("4"));

						idResult = jdeCallObject(_J("CalculateAvailability"),NULL,lpBhvrCom,lpVoid,
										(LPVOID)&dsCalculateAvailability,(CALLMAP *)NULL,(int)0, 
										(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
						jdeErrorSetToFirstEx(lpBhvrCom,lpVoid);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){							
							iErrorCode = 310;
							iErrorCode1 = 1;
							jdeFprintf(dlg,_J("***Error(%d): CalculateAvailability: %ls(%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc,LszLineLotBuf);
							jdeFflush(dlg);

							jdeSprintf(LszString01,_J("***Error(%d): CalculateAvailability: %ls(%ls)...\n"),iErrorCode,
										ErrorRec->lpszShortDesc,LszLineLotBuf);
							jdeFromUnicode(szPantallaStatusLine,LszString01,31,UTF8);
						} 
						if (iErrorCode > 0) continue;//Queremos mostrar este error!!!!

						if (iDbgFlg > 0) {
							jdeToUnicode(LszString01,dsCalculateAvailability.mnQuantityAvailable.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Lote introducido disponibilidad(%ls/%ls)...\n"),LszLineLotBuf,LszString01);
							jdeFflush(dlg);
						}														

						DoubleToMathNumeric(fLineQtyBuf,&mnCantIntroducidaBuf);
					
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***fLineQtyBuf Cantidad Lote (%lf)...\n"),fLineQtyBuf);
							jdeFflush(dlg);
						}

						if ((MathCompare(&mnCantIntroducidaBuf,&dsCalculateAvailability.mnQuantityAvailable) >= 1) && 
							(LcShipFromNegative == _J('1'))){ //Existencia insuficiente o cantidad incorrecta...
							iErrorCode = 311;
							iErrorCode1 = 1;
							jdeToUnicode(LszString01,mnCantIntroducidaBuf.String,DIM(LszString01),UTF8);
							jdeToUnicode(LszString02,dsCalculateAvailability.mnQuantityAvailable.String,DIM(LszString02),UTF8);
							jdeFprintf(dlg,_J("***Error(%d): Disponibilidad Insuficiente (%ls>%ls)...\n"),iErrorCode,
										LszString01,LszString02);
							jdeFflush(dlg);
							fLineQty1Buf = MathNumericToDouble(&dsCalculateAvailability.mnQuantityAvailable);
							sprintf(szPantallaStatusLine,"***Error: Ped.(%f)>Disp.(%f)...",fLineQtyBuf,fLineQty1Buf);
						}

						MathAdd(&mnCantEmbarcarTemp,&dsF4211Buf[j].mnCantEmbarcar,&mnCantIntroducidaBuf);

						if (MathCompare(&mnCantEmbarcarTemp,&dsF4211Buf[j].sdsoqs) >= 1){
							iErrorCode = 312;
							iErrorCode1 = 1;
							jdeToUnicode(LszString01,mnCantEmbarcarTemp.String,DIM(LszString01),UTF8);
							jdeToUnicode(LszString02,dsF4211Buf[j].sdsoqs.String,DIM(LszString02),UTF8);
							jdeFprintf(dlg,_J("***Error(%d): Cantidad Acumulada Incorrecta(%ls>%ls)...\n"),iErrorCode,
										LszString01,LszString02);
							jdeFflush(dlg);
							fLineQtyBuf = MathNumericToDouble(&mnCantEmbarcarTemp);
							fLineQty1Buf = MathNumericToDouble(&dsF4211Buf[j].sdsoqs);
							sprintf(szPantallaStatusLine,"***Error: Acum.(%f)>Disp.(%f)...\n",fLineQtyBuf,fLineQty1Buf);
						}


						if (iErrorCode1 == 0){//No hay errores: procesar EditLine y acumulados...

							iError2 = 0;
						
							iNumLotes++;
							//iNumLotes = 1;

							MathCopy(&dsF42UI05EditLine.mnEnteredShipQuantity,&mnCantIntroducidaBuf);
							
							if (iNumLotes == 1){//Registro Padre...
								dsF42UI05EditLine.cRecordWritten = _J('0');
								dsF42UI05EditLine.cActionCode = _J('P');
								MathCopy(&dsF42UI05EditLine.mnLineNumber,&dsF4211Buf[j].sdlnid);
								MathCopy(&dsF4211Buf[j].mnLineNumberHijo,&dsF4211Buf[j].sdlnid);
								MathCopy(&dsF42UI05EditLine.mnOriginal_LineNumber_3,&dsF42UI05EditLine.mnLineNumber);

								MathCopy(&dsF42UI05EditLine.mnMultiLocParentOrderQuantity,&dsF4211Buf[j].sdsoqs);
								MathSubtract(&dsF42UI05EditLine.mnMultiLocQtyShipVariance,
									&dsF42UI05EditLine.mnEnteredShipQuantity,&dsF4211Buf[j].sdsoqs);
								if (iDbgFlg > 0) {
									jdeToUnicode(LszString01,dsF42UI05EditLine.mnLineNumber.String,DIM(LszString01),UTF8);
									jdeToUnicode(LszString02,dsF42UI05EditLine.mnOriginal_LineNumber_3.String,DIM(LszString01),UTF8);
									jdeFprintf(dlg,_J("***Asignando F42UI05--info Lote Padre (múltiples) L:%ls/O:%ls...\n"),LszString01,LszString02);
									jdeFflush(dlg);
								}
							}
							if (iNumLotes > 1){//Registro Hijo...								
								dsF42UI05EditLine.cRecordWritten = _J('0');  
								dsF42UI05EditLine.cActionCode = _J('S');
								MathAdd(&dsF42UI05EditLine.mnLineNumber,&mnTemp0001Buf,&dsF4211Buf[j].mnLineNumberHijo);//Calculamos linea hija...							
								MathCopy(&dsF42UI05EditLine.mnOriginal_LineNumber_3,&dsF4211Buf[j].mnLineNumberPadre);//No.Linea del padre...
								ZeroMathNumeric(&dsF42UI05EditLine.mnMultiLocParentOrderQuantity);
								ZeroMathNumeric(&dsF42UI05EditLine.mnMultiLocQtyShipVariance);
								if (iDbgFlg > 0) {
									jdeToUnicode(LszString01,dsF42UI05EditLine.mnOriginal_LineNumber_3.String,DIM(LszString01),UTF8);
									jdeToUnicode(LszString02,dsF42UI05EditLine.mnLineNumber.String,DIM(LszString02),UTF8);
									jdeFprintf(dlg,_J("***Asignando F42UI05--info Lote Hijo (múltiples) P:%ls/H:%ls...\n"),LszString01,LszString02);
									jdeFflush(dlg);
								}
							}
						}

					} // if(LcLotProcess == '1') //Procesamiento de Lotes...
					else{
						//No hay procesamiento Lotes (INI)
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***Asignando F42UI05--info Lote (ninguno)..."));
							jdeFflush(dlg);
						}
						dsF42UI05EditLine.cRecordWritten = _J('0');
						dsF42UI05EditLine.cActionCode = _J('C');
						MathCopy(&dsF42UI05EditLine.mnLineNumber,&dsF4211Buf[j].sdlnid);
						MathCopy(&dsF42UI05EditLine.mnOriginal_LineNumber_3,&dsF42UI05EditLine.mnLineNumber);
						MathCopy(&dsF42UI05EditLine.mnEnteredShipQuantity,&mnCantIntroducidaBuf);
						ZeroMathNumeric(&dsF42UI05EditLine.mnMultiLocParentOrderQuantity);
						ZeroMathNumeric(&dsF42UI05EditLine.mnMultiLocQtyShipVariance);
					}

				}

				if ((iError1 == 0) && (iError2 == 0) && (iErrorCode == 0)){ //Mostrar registro seleccionado...
					if (iDbgFlg > 0){
						jdeFprintf(dlg,_J("***dsF4211Buf[%d].sdaitm (%ls) 2-Segundo Paso...\n"),j,dsF4211Buf[j].sdaitm);
						jdeFflush(dlg);
					}
					iLineaEscogida = j;//Si es la única linea del Cache, esta es... Si no, agarramos la primera...
					dsF4211Buf[j].iStatusRegistro = 1;
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
					iErrorCode = 330;	
					iItemFoundInCache = 0;
					jdeFprintf(dlg,_J("***Error(%d): No se encontró en el cache-Segundo Paso...\n"),iErrorCode);
					jdeFflush(dlg);
				}

			} while ((iItemFoundInCache == 0) || (iErrorCode != 0));//No se encontró producto en Cache, solicitar otro...
			jdeFflush(dlg);

			if (iErrorCode1 == 0){

				//*************************************************************************
				//***F42UI05EditLine                                                    ***
				//*************************************************************************
				//Asignaciones para F42UI05 Edit Line:
				//· mnGridColumn											NA
				//· cProcessEdits											NA
				//· cErrorConditios											ok 			
				//· cUpdateWrite											ok '1'	
				//· cRecordWritten											ok '0'		
				//· mnOrderNumber			Introducido por el operador		ok			
				//· szOrderType												ok Cache		
				//· szOrderCompay											ok Cache							
				//· mnLineNumber											ok Cache			
				//· szBranchPlant											ok Cache			
				//· szLocation				Introducido por el operador		ok Cache									
				//· szLot					Introducido por el operador		ok Cache
				//· mnEnteredShipQuantity	Introducido por el operador		ok	
				//· szEnteredLineType										ok Cache		
				//· szContainerID											--
				//· mnCarrier												--
				//· mnUnitPrice												NA
				//· mnUnitCost												NA
				//· msExtendedCost											NA
				//· mnExtendedPrice											NA
				//· jdActualDeliveryDate									NA
				//· szProgramID												ok OWDCCE
				//· mnLastLineNumber										NA
				//· cWritelfFlag											ok '1'
				//· mnReferenceLineNumber									NA
				//· szDescription											ok Cache
				//· mnJobNumberA											--	
				//· szUserID												ok dsGetAuditInfo 
				//· szWorkStationID											ok dsGetAuditInfo
				//· szVersion												ok INI		
				//· cActionCode												ok 'C/P/S'	
				//· cAllowHeldLots											--
				//· cModeProcessing											--
				//· msTRAN_ShipmentNumber									NA			
				//· ldTRAN_DeliveryDate										NA
				//· cTRAN_InventoryAffectFlag								NA
				//· mnTRAN_OverrideDocumentType								NA	
				//· szTRAN_OerrideInventoryDocNum							NA	
				//· szTRAN_OverrideNextStatus								NA
				//· cTRAN_OverrideLeaveShippable							NA
				//· BULK													-- No se vá a procesar
				//· szOrigina_CompanyKeyOrderNo								ok
				//· szOriginal_OrderType2									ok
				//· nOriginal_OrderNumber									ok
				//· mnOriginal_LineNumber_3									ok LineNumber Padre si Hijo
				//· mnXT4111LineNumber										--
				//· mnB4200310LineNumber									--
				//· szUnitOfMeasureAsInput	Introducido por el operador		ok 	
				//· mnMultiLocQtyShipVariance								ok 0 ??
				//· mMltiParentOrderQuantity								ok EnteredShipQuantity ??
				//· mnAddresNumberShipTo									--
				//· szLineType_2											ok Cache
				//· mnContractSupplementDistri								--
				//· szContractNumberDistributi								--
				//· szTRAN_PrimaryVehicled									--
				//· szTARN_RegistrationLicenseNum							--
				//· szComputerID											ok GetAuditInfo 

				jdeStrcpy(dsF42UI05EditLine.szProgramID,_J("EP4205"));
				dsF42UI05EditLine.cWriteIfWarning = _J('1');
				dsF42UI05EditLine.cModeProcessing = _J('1'); 
				jdeStrcpy(dsF42UI05EditLine.szComputerID,dsGetAuditInfo.szWorkstation_UserId); 
				jdeStrcpy(dsF42UI05EditLine.szWorkStationId,dsGetAuditInfo.szWorkstation_UserId);
				jdeStrcpy(dsF42UI05EditLine.szUserId,dsGetAuditInfo.szUserName);
						
				jdeStrcpy(dsFormatLocation.szFileFormatLocation,LszLineLocationBuf);
				jdeStrcpy(dsFormatLocation.szCostCenter,dsF4211Buf[iLineaEscogida].sdmcu);
				dsFormatLocation.cMode = _J('1');//File Format to Display Format
				dsFormatLocation.cValidate = _J('4');//NO validar...
				idResult = jdeCallObject(_J("FormatLocation"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsFormatLocation,
								(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				
				jdeErrorSetToFirstEx(lpBhvrCom,lpVoid);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){
					iErrorCode = 313;
					jdeFprintf(dlg,_J("***Error(%d): FormatLocation: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				}
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 313;
					jdeFprintf(dlg,_J("***Error(%d): FormatLocation...\n"),iErrorCode);
					jdeFflush(dlg);
				}

				if (iErrorCode > 0) continue;//Queremos mostrar este error!!!!
				
				MathCopy(&dsF42UI05EditLine.mnOrderNumber,&dsF4211Buf[iLineaEscogida].sddoco);
				jdeStrcpy(dsF42UI05EditLine.szOrderType,dsF4211Buf[iLineaEscogida].sddcto);
				jdeStrcpy(dsF42UI05EditLine.szOrderCompany,dsF4211Buf[iLineaEscogida].sdkcoo);
				jdeStrcpy(dsF42UI05EditLine.szBranchPlant,dsF4211Buf[iLineaEscogida].sdmcu);
				jdeStrcpy(dsF42UI05EditLine.szLocation,dsFormatLocation.szDisplayFormatLocation);
				jdeStrcpy(dsF42UI05EditLine.szLot,LszLineLotBuf);
				jdeStrcpy(dsF42UI05EditLine.szUnitOfMeasureAsInput,LszLineUMBuf);
				jdeStrcpy(dsF42UI05EditLine.szEnteredLineType,dsF4211Buf[iLineaEscogida].sdlnty);
				jdeStrcpy(dsF42UI05EditLine.szLineType_2,dsF4211Buf[iLineaEscogida].sdlnty);
				DeformatDate(&dsF42UI05EditLine.jdActualDeliveryDate,LszDocTransDateBuf,(JCHAR*) _J("DSMSE"));
				jdeStrcpy(dsF42UI05EditLine.szVersion,LszP4205VersionNameBuf);
				
				jdeStrcpy(dsF42UI05EditLine.szOriginal_CompanyKeyOrderNo,dsF42UI05EditLine.szOrderCompany);
				jdeStrcpy(dsF42UI05EditLine.szOriginal_OrderType2,dsF42UI05EditLine.szOrderType);  
				MathCopy(&dsF42UI05EditLine.mnOriginal_OrderNumber,&dsF42UI05EditLine.mnOrderNumber);  
				
				ZeroMathNumeric(&dsF42UI05EditLine.mnUnitPrice);
				ZeroMathNumeric(&dsF42UI05EditLine.mnUnitCost);
				ZeroMathNumeric(&dsF42UI05EditLine.mnExtendedPrice);
				ZeroMathNumeric(&dsF42UI05EditLine.mnExtendedCost);
				
				if (iDbgFlg > 0){
					jdeToUnicode(LszString01,dsF4211Buf[iLineaEscogida].sdsoqs.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***%d)Producto (%ls-%ls) Cantidad (%ls/%ls) Ubicacion(%ls)...\n"),
						iLineaEscogida,dsF4211Buf[iLineaEscogida].sdaitm,dsF4211Buf[iLineaEscogida].sddsc1,
						LszString01,dsF4211Buf[iLineaEscogida].sduom,dsF4211Buf[iLineaEscogida].sdlocn); 
					jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szOrderCompany);
					jdeToUnicode(LszString01,dsF42UI05EditLine.mnOrderNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szOrderType);
					jdeToUnicode(LszString01,dsF42UI05EditLine.mnLineNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] Identifier3rdItem(%ls)...\n"),iLineaEscogida,dsF4211Buf[iLineaEscogida].sdaitm);
					jdeToUnicode(LszString01,dsF42UI05EditLine.mnEnteredShipQuantity.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] Entered Ship Quantity(%ls)...\n"),iLineaEscogida,LszString01);
					jdeToUnicode(LszString01,dsF42UI05EditLine.mnMultiLocParentOrderQuantity.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Cache[%d] MultiLoc Parent Order Quantity(%ls)...\n"),iLineaEscogida,LszString01);
					FormatMathNumeric(LszString01,&dsF42UI05EditLine.mnMultiLocQtyShipVariance);
					jdeFprintf(dlg,_J("***Cache[%d] MultiLoc Qty Ship Variance(%ls)...\n"),iLineaEscogida,LszString01);
					jdeFprintf(dlg,_J("***Cache[%d] Unit Of MeasureAsInput(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szUnitOfMeasureAsInput);
					jdeFprintf(dlg,_J("***Cache[%d] Location(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szLocation);
					jdeFprintf(dlg,_J("***Cache[%d] Line LotBuf(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szLot);
					jdeFprintf(dlg,_J("***Cache[%d] Action Code(%lc)...\n"),iLineaEscogida,dsF42UI05EditLine.cActionCode);
					jdeFflush(dlg);
				}
				
				if (iDbgFlg > 0) {
					jdeFprintf(dlg,_J("***F42UI05EditLine...\n"));
					jdeFflush(dlg);
				}
				idResult = jdeCallObject( _J("F42UI05EditLine"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF42UI05EditLine,
											(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);

				jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
					if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )){
						iErrorCode = 314;
						iErrorCode1 = 1;
						jdeFprintf(dlg,_J("***Error(%d): F42UI05EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						jdeFflush(dlg);
						jdeSprintf(LszString01,_J("***Error(%d): F42UI05EditLine: %ls..."),iErrorCode,
									ErrorRec->lpszShortDesc);
						jdeFromUnicode(szPantallaStatusLine,LszString01,31,UTF8);
					}
					else{
						jdeFprintf(dlg,_J("***Warn(%d): F42UI05EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						jdeFflush(dlg);
					}
				}
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 314;
					iErrorCode1 = 1;
					jdeFprintf(dlg,_J("***Error(%d): F42UI05EditLine...\n"),iErrorCode);
					jdeFflush(dlg);
					strcpy(szPantallaStatusLine,"***Error(314): F42UI05EditLine..."); 
				}

				if (iErrorCode == 0){

					if (iDbgFlg > 0) {
						jdeFprintf(dlg,_J("***F42UI05EditLine OK...\n"));
						jdeFflush(dlg);
					}
					
					iEditLineLines = 1;//Por lo menos una linea OK!!!!
					
					MathCopy(&dsF4211Buf[iLineaEscogida].mnCantEmbarcar,&mnCantEmbarcarTemp);//Actualizamos la CantEmbarcar en el buffer...
					dsF4211Buf[iLineaEscogida].iNumLotes = iNumLotes;//Actualizamos  Flag de Lotes (Padre/Hijo) en el buffer...

					if (MathCompare(&dsF4211Buf[iLineaEscogida].mnCantEmbarcar,&dsF4211Buf[iLineaEscogida].sdsoqs) ==0){
						iRecordsAvailCache--;// Un registro disponible menos en el Cache...
						dsF4211Buf[iLineaEscogida].iStatusRegistro = 2; //Marcar Cache para no procesar otra vez...
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***iRecordsAvailCache(%d/%d)...\n"),iRecordsAvailCache,iLineaEscogida);
							jdeFflush(dlg);
						}							
					}

					if (dsF42UI05EditLine.cActionCode == _J('P')){//Se pudo procesar el padre..actualizamos el chache...
						MathCopy(&dsF4211Buf[iLineaEscogida].mnLineNumberPadre,&dsF42UI05EditLine.mnLineNumber);
						MathCopy(&dsF4211Buf[iLineaEscogida].mnCantIntroPadre,&dsF42UI05EditLine.mnEnteredShipQuantity);
						jdeStrcpy(dsF4211Buf[iLineaEscogida].szLocationPadre,dsF42UI05EditLine.szLocation);
						jdeStrcpy(dsF4211Buf[iLineaEscogida].szLotPadre,LszLineLotBuf);
						jdeStrcpy(dsF4211Buf[iLineaEscogida].szUOMPadre,LszLineUMBuf);
					}

					if (dsF42UI05EditLine.cActionCode == _J('S')){//Se pudo procesar el hijo..recalculemos al padre...
//							memset((void *) &dsF42UI05EditLine,(int) _J('\0'),sizeof(DSD4200790A));
						MathCopy(&dsF4211Buf[iLineaEscogida].mnLineNumberHijo,&dsF42UI05EditLine.mnLineNumber);

						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***Procesando al padre del hijo: borrado...\n"));  
							jdeFflush(dlg);
						}

						dsF42UI05EditLine.cRecordWritten = _J('1');
						dsF42UI05EditLine.cActionCode = _J('D');
						MathCopy(&dsF42UI05EditLine.mnLineNumber,&dsF4211Buf[iLineaEscogida].mnLineNumberPadre);
						MathCopy(&dsF42UI05EditLine.mnOriginal_LineNumber_3,&dsF42UI05EditLine.mnLineNumber);

						if (iDbgFlg > 0){
							jdeToUnicode(LszString01,dsF4211Buf[iLineaEscogida].sdsoqs.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***%d)Producto Padre Borrado (%ls-%ls) Cantidad (%ls/%ls) Ubicacion(%ls)...\n"),
								iLineaEscogida,dsF4211Buf[iLineaEscogida].sdaitm,dsF4211Buf[iLineaEscogida].sddsc1,
								LszString01,dsF4211Buf[iLineaEscogida].sduom,dsF4211Buf[iLineaEscogida].sdlocn); 
							jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szOrderCompany);
							jdeToUnicode(LszString01,dsF42UI05EditLine.mnOrderNumber.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iLineaEscogida,LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szOrderType);
							jdeToUnicode(LszString01,dsF42UI05EditLine.mnLineNumber.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iLineaEscogida,LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Identifier3rdItem(%ls)...\n"),iLineaEscogida,dsF4211Buf[iLineaEscogida].sdaitm);
							jdeFprintf(dlg,_J("***Cache[%d] Action Code(%lc)...\n"),iLineaEscogida,dsF42UI05EditLine.cActionCode);
							jdeFflush(dlg);
						}

						idResult = jdeCallObject( _J("F42UI05EditLine"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF42UI05EditLine,
													(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);

						jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
							iErrorCode = 320;
							iErrorCode1 = 1;
							jdeFprintf(dlg,_J("***Error(%d): F42UI05EditLine Padre: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
							jdeFflush(dlg);
							jdeFromUnicode(szPantallaStatusLine,ErrorRec->lpszShortDesc,31,UTF8);
						}
						if ((idResult == ER_ERROR) && (iErrorCode == 0)){
							iErrorCode = 320;
							iErrorCode1 = 1;
							jdeFprintf(dlg,_J("***Error(%d): F42UI05EditLine Padre...\n"),iErrorCode);
							jdeFflush(dlg);
							strcpy(szPantallaStatusLine,"***Error(320): F42UI05EditLine...");
						}

						if (iErrorCode > 0) continue;//Queremos mostrar este error!!!!

						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***Procesando al padre del hijo: actualizacion...\n"));
							jdeFflush(dlg);
						}

						dsF42UI05EditLine.cRecordWritten = _J('0');
						dsF42UI05EditLine.cActionCode = _J('P');
						MathCopy(&dsF42UI05EditLine.mnLineNumber,&dsF4211Buf[iLineaEscogida].mnLineNumberPadre);
						MathCopy(&dsF42UI05EditLine.mnOriginal_LineNumber_3,&dsF42UI05EditLine.mnLineNumber);
						MathCopy(&dsF42UI05EditLine.mnEnteredShipQuantity,&dsF4211Buf[iLineaEscogida].mnCantIntroPadre);
						
						MathSubtract(&dsF42UI05EditLine.mnMultiLocQtyShipVariance,
							&dsF4211Buf[iLineaEscogida].mnCantEmbarcar,&dsF4211Buf[iLineaEscogida].sdsoqs);
						MathSubtract(&dsF42UI05EditLine.mnMultiLocParentOrderQuantity,
							&dsF4211Buf[iLineaEscogida].mnCantIntroPadre,&dsF42UI05EditLine.mnMultiLocQtyShipVariance);

						ZeroMathNumeric(&dsF42UI05EditLine.mnUnitPrice);
						ZeroMathNumeric(&dsF42UI05EditLine.mnUnitCost);

						dsF42UI05EditLine.cWriteIfWarning = _J('1');
						dsF42UI05EditLine.cModeProcessing = _J('1');

						jdeStrcpy(dsF42UI05EditLine.szLocation,dsF4211Buf[iLineaEscogida].szLocationPadre);
						jdeStrcpy(dsF42UI05EditLine.szLot,dsF4211Buf[iLineaEscogida].szLotPadre);
						jdeStrcpy(dsF42UI05EditLine.szUnitOfMeasureAsInput,dsF4211Buf[iLineaEscogida].szUOMPadre);
						jdeStrcpy(dsF42UI05EditLine.szLineType_2,dsF4211Buf[iLineaEscogida].sdlnty);
					
						if (iDbgFlg > 0){
							jdeToUnicode(LszString01,dsF4211Buf[iLineaEscogida].sdsoqs.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***%d)Producto Padre (%ls-%ls) Cantidad (%ls/%ls) Ubicacion(%ls)...\n"),
								iLineaEscogida,dsF4211Buf[iLineaEscogida].sdaitm,dsF4211Buf[iLineaEscogida].sddsc1,
								LszString01,dsF4211Buf[iLineaEscogida].sduom,dsF4211Buf[iLineaEscogida].sdlocn); 
							jdeFprintf(dlg,_J("***Cache[%d] Order Company(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szOrderCompany);
							jdeToUnicode(LszString01,dsF42UI05EditLine.mnOrderNumber.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Document(OrderNo, Invoice, etc)(%ls)...\n"),iLineaEscogida,LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Order Type(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szOrderType);
							jdeToUnicode(LszString01,dsF42UI05EditLine.mnLineNumber.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Line Number(%ls)...\n"),iLineaEscogida,LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Identifier3rdItem(%ls)...\n"),iLineaEscogida,dsF4211Buf[iLineaEscogida].sdaitm);
							jdeToUnicode(LszString01,dsF42UI05EditLine.mnEnteredShipQuantity.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] Entered Ship Quantity(%ls)...\n"),iLineaEscogida,LszString01);
							jdeToUnicode(LszString01,dsF42UI05EditLine.mnMultiLocParentOrderQuantity.String,DIM(LszString01),UTF8);
							jdeFprintf(dlg,_J("***Cache[%d] MultiLoc Parent Order Quantity(%ls)...\n"),iLineaEscogida,LszString01);
							FormatMathNumeric(LszString01,&dsF42UI05EditLine.mnMultiLocQtyShipVariance);
							jdeFprintf(dlg,_J("***Cache[%d] MultiLoc Qty Ship Variance(%ls)...\n"),iLineaEscogida,LszString01);
							jdeFprintf(dlg,_J("***Cache[%d] Unit Of MeasureAsInput(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szUnitOfMeasureAsInput);
							jdeFprintf(dlg,_J("***Cache[%d] Location(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szLocation);
							jdeFprintf(dlg,_J("***Cache[%d] Line LotBuf(%ls)...\n"),iLineaEscogida,dsF42UI05EditLine.szLot);
							jdeFprintf(dlg,_J("***Cache[%d] Action Code(%lc)...\n"),iLineaEscogida,dsF42UI05EditLine.cActionCode);
							jdeFflush(dlg);
						}
					
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("***F42UI05EditLine Padre...\n"));
							jdeFflush(dlg);
						}
						idResult = jdeCallObject( _J("F42UI05EditLine"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF42UI05EditLine,
													(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);

						jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
							if ((jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ) && (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Informational:"), 14) != 0 )){
								iErrorCode = 321;
								iErrorCode1 = 1;
								jdeFprintf(dlg,_J("***Error(%d): F42UI05EditLine Padre: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
								jdeFflush(dlg);
								jdeFromUnicode(szPantallaStatusLine,ErrorRec->lpszShortDesc,31,UTF8);
							}
							else{
								jdeFprintf(dlg,_J("***Warn(%d): F42UI05EditLine Padre: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
								jdeFflush(dlg);
							}
						}
						if ((idResult == ER_ERROR) && (iErrorCode == 0)){
							iErrorCode = 321;
							iErrorCode1 = 1;
							jdeFprintf(dlg,_J("***Error(%d): F42UI05EditLine Padre...\n"),iErrorCode);
							jdeFflush(dlg);
							strcpy(szPantallaStatusLine,"***Error(321): F42UI05EditLine...");
						}
					}
				}// if (iErrorCode == 0)... EdiLine OK
			}// if (iErrorCode1 == 0)

			jdeErrorClearEx(lpBhvrCom1,lpVoid1); 

			if (iDbgFlg > 0) {
				jdeFprintf(dlg,_J("***F42UI05EditLine sts iErrorCode, iErrorCode1 (%d/%d)...\n"),iErrorCode,iErrorCode1);
				jdeFflush(dlg);
			}

		//} while (iErrorCode1 != 0);
		
		} while (TRUE);//El usuario decide cuándo terminar...

		//***Determinar si seguir procesando...
		if (iEditLineLines == 1){
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Inicializando 6ta pantalla datos Aceptacion Despacho...\n"));
				jdeFflush(dlg);
			}

			memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
			memset(szCamposContenido,'\0',sizeof(szCamposContenido));
			memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
			memset(iCamposErrores,'\0',sizeof(iCamposErrores));
			memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));

			iPrimeraVez = 0;			

			strcpy (szPantallaTitulo, "Captura Despacho Almacen");

			szCamposEtiquetas[0] = "Procesar Embarque?(S/N)...";  

			strcpy(szCamposContenido[0],"S");
			iCamposPosiciones[0] = strlen(szCamposContenido[0]);

			iCamposCantidad = 1;

			OWDCmp02 (&iPrimeraVez, szPantallaTitulo, (iCamposOffset + 9), iCamposCantidad, (iStatusOffset -9),iInicioEtiquetas, 
								iInicioCampos + 11, iUltimaLinea, iCReqTAB,
								iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
								iDbgFlg, dlg);

			szCamposContenido[0][0] = tolower(szCamposContenido[0][0]); szCamposContenido[0][1] = '\0';
			jdeToUnicode(LszUsrEntry,szCamposContenido[0],2,UTF8);
		}

		if ((iEditLineLines == 1) && (LszUsrEntry[0] == LcYesNoBuf)){ //Por lo menos una línea EditLine OK...
			
			//*************************************************************************
			//***ShipConfirmEndDoc                                                  ***
			//*************************************************************************
			//Asignaciones para Shipment Confirmation End Document:
			//· mnJobNumber													ok F42UI05EditLine
			//· szComputerId												ok GetAuditInfo
			//· cErrorCondition				 
			//· szBackOrderNextStatus										NA
			//· szUserId													ok GetAuditInfo
			//· szR42565Version												ok INI
			//· cTRANSyncEndDoc												ok '1'
			//· mTRANJobsNumber												ok '0'
			//· szTRANShipNumbers											ok ' '
			//· zP4205Version												ok INI
			
			jdeStrcpy(dsShipConfirmEndDoc.szComputerId,dsGetAuditInfo.szWorkstation_UserId);
			jdeStrcpy(dsShipConfirmEndDoc.szUserId,dsGetAuditInfo.szUserName);
			dsShipConfirmEndDoc.cTRANSyncEndDoc = _J('1');
			
			MathCopy(&dsShipConfirmEndDoc.mnJobNumber,&dsF42UI05EditLine.mnJobnumberA);
			ZeroMathNumeric(&dsShipConfirmEndDoc.mTRANJobsNumber);
			ZeroMathNumeric(&dsShipConfirmEndDoc.mnShipmentNumber);
			jdeStrcpy(dsShipConfirmEndDoc.szP4205Version,LszP4205VersionNameBuf);
			jdeStrcpy(dsShipConfirmEndDoc.szR42565Version,LszP4205VersionNameBuf);
			
			if (iDbgFlg > 0) {
				jdeFprintf(dlg,_J("***ShipConfirmEndDoc...\n"));
				jdeFflush(dlg);
			}
			idResult = jdeCallObject(_J("ShipConfirmEndDoc"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsShipConfirmEndDoc,
				(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				
			jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
			while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
				iErrorCode = 315;			
				jdeFprintf(dlg,_J("***Error(%d): ShipConfirmEndDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
				jdeFflush(dlg);
			}
			if ((idResult == ER_ERROR) && (iErrorCode == 0)){
				iErrorCode = 315;			
				jdeFprintf(dlg,_J("***Error(%d): ShipConfirmEndDoc...\n"),iErrorCode);
				jdeFflush(dlg);
			}
			jdeFflush(dlg);

			if (iErrorCode != 0) goto lbFIN;

			if (iDbgFlg > 0){
				jdeToUnicode(LszString01,dsShipConfirmEndDoc.mnShipmentNumber.String,DIM(LszString01),UTF8);
				jdeFprintf(dlg,_J("***Numero Embarque(%ls)...\n"),LszString01);
				jdeFflush(dlg);
			}

			//*************************************************************************
			//***F42UI05DeleteCache                                                 ***
			//*************************************************************************
			//Asignaciones para DeleteCache:
			//mnJobNumber;               
			//szComputerId												
			//mnLineNumberFrom											
			//mnLineNumberThru				 
			//cModeofCommitmentClear										
			//szProgramId
			
			MathCopy(&dsF42UI05DeleteCache.mnJobNumber,&dsF42UI05EditLine.mnJobnumberA);
			jdeStrcpy(dsF42UI05DeleteCache.szComputerId,dsGetAuditInfo.szWorkstation_UserId);
			dsF42UI05DeleteCache.cModeofCommitmentClear = _J(' ');
			jdeStrcpy(dsF42UI05DeleteCache.szProgramId,_J("EP4205"));

			if (iDbgFlg > 0) {
					jdeFprintf(dlg,_J("***F42UI05DeleteCache...\n"));
					jdeFflush(dlg);
				}
				idResult = jdeCallObject(_J("F42UI05DeleteCache"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsShipConfirmEndDoc,
					(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
					
				jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
					iErrorCode = 322;			
					jdeFprintf(dlg,_J("***Error(%d): F42UI05DeleteCache: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				}
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 322;			
					jdeFprintf(dlg,_J("***Error(%d): F42UI05DeleteCache...\n"),iErrorCode);
					jdeFflush(dlg);
				}
			jdeFflush(dlg);

			if (iErrorCode != 0) goto lbFIN;

		}

		OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 6ta pantalla para mostrar la 1ra...

	} while (TRUE);

	//*************************************************************************
	// Clean up the lpBhvrCom, lpVoid and free user and environment 
	//*************************************************************************
lbFIN:

	OWDCmp90 (iDbgFlg, dlg);//Limpiamos para mostrar menu...

	jdeErrorClearEx(lpBhvrCom,lpVoid); 
	jdeFree(((LPCG_BHVR)lpVoid)->lpErrorEventKey);
	jdeErrorTerminateEx(((LPCG_BHVR)lpVoid)->lpHdr);
	jdeFreeBusinessFunctionParms(lpBhvrCom,lpVoid);

	jdeErrorClearEx(lpBhvrCom1,lpVoid1); 
	jdeFree(((LPCG_BHVR)lpVoid1)->lpErrorEventKey);
	jdeErrorTerminateEx(((LPCG_BHVR)lpVoid1)->lpHdr);
	jdeFreeBusinessFunctionParms(lpBhvrCom1,lpVoid1);

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit OWDCce01(%d)...\n"), iErrorCode);
	jdeFflush(dlg);

	if (iDbgFlg == 1) jdeFclose(dlg);

	return iErrorCode;
}

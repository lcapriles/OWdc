// 04tc01.cpp
//
//Creado:		Luis Capriles,		Fecha:	16/10/2012 
//Modificado:	Luis Capriles,		Fecha:	16/10/2012 - Descripción y UM ...
//Modificado:	Luis Capriles,		Fecha:	05/11/2012 - Manejo de multiples ini, varios LOCATION ...
//Modificado:	Luis Capriles,		Fecha:	03/07/2013 - Manejo de MCU Principal ...
//Modificado:	Luis Capriles,		Fecha:	13/09/2013 - Manejo de Fecha Vencimiento Lote ...
//
//El programa tc01 permite realizar 1ra y 2da toma de inventarios.
//
#include "tc01.h"  
#include <stdio.h>
#include "jde.h"

#include "F55DC02.h"
#include "F55DC021.h"

#include "F41021.h"
#include "F4102.h"

#include "b9800100.h"	// GetAuditInfo
#include "x4101.h"		// GetItemMasterByShortItem, GetItemMasterBy3rdItem, GetItemMasterBy2ndItem
#include "B4000370.h"	// F40095GetDefaultBranch
#include "B0000130.h"	// RetrieveCompanyFromBusUnit
#include "B4000150.h"	// GetBranchConstants
#include "xf41021.h"	// VerifyAndGetItemLocation
#include "b4000310.h"	// FormatLocation
#include "b4001050.h"	// GetCrossReferenceFields
#include "n0000563.h"	// F0010RetrieveCompanyConstant
#include "b4000610.h"	// GetLotMasterByLotNumber
#include "B4000520.h"	// GetItemUoMConversionFactor
#include "B1100007.h"	// DecimalsTriggerGetbyCOCRCD 
#include "x00022.h"		// GetNextUniqueKeyID
#include "b41021.h"		// CalculateQtyOnHand
#include "x4101.h"		// GetItemMasterByShortItem, GetItemMasterBy3rdItem, GetItemMasterBy2ndItem
#include "B4000520.h"	// GetItemUoMConversionFactor

void OWDCmp02 (int * primeraVez,char * pantallaTitulo,int camposOffset,int camposCantidad, 
				int statusOffset,int inicioEtiquetas,int inicioCampos,int ultimaLinea,int CReqTAB,
				int camposPosiciones[],char * camposEtiquetas[],char camposContenido[][128],int camposErrores[],
				char * pantallaStatusLine,int iDbgFlg,FILE * dlg); //Manejo de la pantalla de entrada datos...

void OWDCmp90 (int iDbgFlg, FILE * dlg); //Terminar Manejo de la pantalla curses...

int OWDCgbc(HENV hEnv,HUSER hUser,JCHAR LszUsrEntry[128],JCHAR LszLineItemBuf[26],MATH_NUMERIC * mnItemShortIDBuf,
			JCHAR LszUOMdefaultBuf[3],JCHAR LszUOMstdConv[3],JCHAR LszCrossRefTypeCodeBuf[3],int iUPClenBuf,int iSCClenBuf,
			JCHAR szItemDescription[31],int iGetItemMasterBy, JCHAR c3rdItemNoSymbol, int iDbgFlg,FILE * dlg); //Valida código producto...

int ProcesaCB(char * szCodigoBarra, char * szCBprod,char * szCBlote,char * szCBfecha,int iUPClenBuf,int iSCClenBuf,int iDbgFlg, FILE * dlg);

int FormateaFecha(char * szFechaI, JCHAR * LszFechaO);


__declspec(dllexport) int OWDCtc01(HENV hEnv,HUSER hUser,JCHAR LszUsrID[16],char cIniFlag)
{
	LPJDEERROR_RECORD	ErrorRec	= NULL;
	ID					idResult;
	LPCG_BHVR			lpVoid		= NULL;
	LPBHVRCOM			lpBhvrCom	= NULL;
	ERROR_EVENT_KEY		EventKeyLocal;

	DSD4000150			dsGetBranchConstants;
	DSD4000232			dsF40095GetDefaultBranch;
	DSD0000130			dsRetrieveCompanyFromBusUnit;
	DSDXF41021C			dsVerifyAndGetItemLocation;
	DSD4001050			dsGetCrossReferenceFields;
	DSD4000310A			dsFormatLocation;
	DSD9800100			dsGetAuditInfo;
	DSD0000563			dsF0010RetrieveCompanyConstant;
	LPFORMDSUDC			lpValidateUDC;
	FORMDSUDC			dsValidateUDC;
	DSD4000610			dsGetLotMasterByLotNumber;
	DSD4000520			dsGetItemUoMConversionFactor;
	DSD1100007			dsDecimalsTriggerGetbyCOCRCD;
	DSDX00022A			dsX00022	= {0};
	DSD41021A			dsB41021	= {0};
	DSDX4101B			dsGetItemMasterByShortItem;
	DSDX4101C			dsGetItemMasterBy2ndItem;
	DSDX4101D			dsGetItemMasterBy3rdItem;

	
	MATH_NUMERIC		mnItemShortIDBuf;

	HREQUEST			hRequestF55DC02 = (HREQUEST)NULL;
	F55DC02				dsF55DC02,dsF55DC02dummy;
	KEY1_F55DC02		dsF55DC02Key1;
	SELECTSTRUCT		lpSelectF55DC02[4];

	HREQUEST			hRequestF55DC021 = (HREQUEST)NULL;
	F55DC021			dsF55DC021,dsF55DC021dummy;
	KEY1_F55DC021		dsF55DC021Key1;
	SELECTSTRUCT		lpSelectF55DC021[4];

	//HREQUEST			hRequestF41021 = (HREQUEST)NULL;
	//F41021			dsF41021,dsF41021dummy;
	//KEY6_F41021		dsF41021Key6;
	//SELECTSTRUCT		lpSelectF41021[6];

	HREQUEST			hRequestF4102 = (HREQUEST)NULL;
	F4102				dsF4102,dsF4102dummy;
	KEY2_F4102			dsF4102Key2;
	SELECTSTRUCT		lpSelectF4102[6];

	LPF4101				lpdsF4101 = (LPF4101) NULL;

	FILE * ini;
	FILE * dlg;

	MATH_NUMERIC		mnCantIntroducidaBuf;
						
	JCHAR LszLinea[80],LszLin1[64],LszLin2[64],LszUsrEntry[128],

		 LszDocBranchPlantBuf[16],LszLineLocationBuf[21],LszUbicacionDfltBuf[21],szUOMdefaultBuf[3],
		 LszLineLotBuf[31],LszLineItemBuf[26],LszLineUMBuf[3],LszUOMdefaultBuf[3],LszUOMprimaryBuf[3],LszUOMstdConv[3],LszLotExpirationDate[17],

		 LszCrossRefTypeCodeBuf[3],LszDocumentCompanyBuf[6],LszItemDescriptionBuf[31], 

		 LszString01[64],LszString02[64],LszString03[64],LszString04[64],LszTempBuf[128],

		 LcDecimalCharBuf,
		 LcYesNoBuf,LcTempBuf,LcTemp1Buf; 

	JDEDATE	jFechaTemp;
 
	int	 iUPClenBuf,iSCClenBuf,iGetItemMasterBy,iLineQtyBuf,iErrorValidarLote,
		 iErrorCode,iErrorCode1,iDbgFlg,i,j,iSalir,iError1,iError2,iProdNoExiste,
		 iPrimeraVez,iCamposOffset,iCamposCantidad,iStatusOffset,iInicioEtiquetas,iInicioCampos,iUltimaLinea,iCReqTAB,iUnicaVez,
		 iCamposPosiciones[64],iCamposErrores[64];
		 
	int	 iIdxProd = 0,iIdxDescr = 0,iIdxNART = 0,iIdxUM = 0,iIdxCant = 0,iIdxLot = 0,iIdxFVcto = 0,iIdxLoc = 0;

	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128],
		 * szDummy1,szTempBuf[128],*szTempBuf1,*szTempBuf2,szCamposContenidoTemp[64][128];

	char menu2[16];

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
	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCtc01_D"));
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

	if (cIniFlag != '\0') {
		memset(menu2,'\0',sizeof(menu2));
		sprintf(menu2,"%s%c%s\0","OWDCtc01",cIniFlag,".ini");
		jdeToUnicode(LszTempBuf,menu2,16,UTF8);
	}else jdeToUnicode(LszTempBuf,"OWDCtc01.ini",16,UTF8);

	ini = jdeFopen(LszTempBuf,_J("r"));
	if (!ini){
		iErrorCode = 500;
		jdeFprintf(dlg,_J("***Error(%d) abriendo INI (%ls)...\n"),iErrorCode,LszTempBuf);
        goto lbFin00;
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
		if(jdeStrcmp(LszLin1,_J("GetItemMasterBy")) == 0){
			iGetItemMasterBy = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: GetItemMasterBy (%d)...\n"),
										iGetItemMasterBy);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("UbicacionDflt")) == 0){
			jdeStrcpy(LszUbicacionDfltBuf,LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: UbicacionDflt (%ls)...\n"),LszUbicacionDfltBuf);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("ErrorValidarLote")) == 0){
			iErrorValidarLote = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ErrorValidarLote (%d)...\n"),iErrorValidarLote);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("LineQty")) == 0){
			iLineQtyBuf = jdeAtoi(LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LineQty (%d)...\n"),
										iLineQtyBuf);
			continue;
		}
		jdeFflush(dlg);
	}
	jdeFclose(ini);
	jdeFflush(dlg);

	iErrorCode = 0; //No hay Errores!!!

	//*************************************************************************
	// Set up the lpBhvrCom amd lpVoid objects                              ***
	//*************************************************************************
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Seteo de lpBhvrCom, lpVoid y estructuras...\n"));

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
	memset((void *) &dsGetLotMasterByLotNumber,(int)(_J('\0')),sizeof(DSD4000610));
	lpValidateUDC = &dsValidateUDC;

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Determina Almacen, Compania, Maquina, Fecha y ProcOpt...\n"));

	//***Obtiene Usuario, máquina, fecha
	idResult = jdeCallObject(_J("GetAuditInfo"),NULL,lpBhvrCom,lpVoid,&dsGetAuditInfo,
					(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
	if (idResult == ER_ERROR){
		iErrorCode = 501;
		jdeFprintf(dlg,_J("***Error(%d): GetAuditInfo:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 

	//***Busca Almacén por default del usuario
	idResult = jdeCallObject(_J("GetDefaultBranch"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsF40095GetDefaultBranch, 
						(CALLMAP *)NULL, (int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 502;
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
		iErrorCode = 503;
		jdeFprintf(dlg,_J("***Error(%d): RetrieveCompanyFromBusUnit:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***RetrieveCompanyFromBusUnit (%ls)...\n"),dsRetrieveCompanyFromBusUnit.szCompany);

	// Determina Símbolo para identificar 3rd Inv Number...
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Determina dsGetCrossReferenceFields...\n"));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szSystemCode,(const JCHAR *)_J(" "));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szBranchPlant,(const JCHAR *)LszDocBranchPlantBuf);
	dsGetCrossReferenceFields.cSuppressErrorMsg = _J('1');
	idResult = jdeCallObject(_J("GetCrossReferenceFields"), NULL,lpBhvrCom,lpVoid,
	                      (LPVOID)&dsGetCrossReferenceFields,(LPCALLMAP)NULL,(int)0,
						  (JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if((idResult == ER_ERROR) || (jdeStrcmp(dsGetCrossReferenceFields.szErrorMsgID,_J(" ")) != 0)){
		iErrorCode = 505;
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
	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Abriendo Tabla F55DC02,F55DC021 y F41021...\n"));
		jdeFflush(dlg);
	}
	idResult = JDB_OpenTable(hUser,NID_F55DC02,ID_F55DC02_PK,NULL,(ushort)(0),
							(JCHAR *)NULL,&hRequestF55DC02);
	if (idResult == JDEDB_FAILED){
		iErrorCode = 508;
		jdeFprintf (dlg,_J("***Error(%d): JDB_OpenTable(F55DC02) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN0;
	}
 
	idResult = JDB_OpenTable(hUser,NID_F55DC021,ID_F55DC021_PK,NULL,(ushort)(0),
							(JCHAR *)NULL,&hRequestF55DC021);
	if (idResult == JDEDB_FAILED){
		iErrorCode = 508;
		jdeFprintf (dlg,_J("***Error(%d): JDB_OpenTable(F55DC021) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN1;
	}

//Modificado:	Luis Capriles,		Fecha:	03/07/2013 - Manejo de MCU Principal ...
//	idResult = JDB_OpenTable(hUser,NID_F41021,ID_F41021_LOT_NUMBER__ITEM,NULL,(ushort)(0),
//							(JCHAR *)NULL,&hRequestF41021);
	idResult = JDB_OpenTable(hUser,NID_F4102,ID_F4102_ITEM__BRANCH,NULL,(ushort)(0),
							(JCHAR *)NULL,&hRequestF4102);

	if (idResult == JDEDB_FAILED){
		iErrorCode = 508;
		jdeFprintf (dlg,_J("***Error(%d): JDB_OpenTable(F4102*) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN2;
	}
	
	//Construye Where del select...

	jdeStrcpy(dsF55DC02Key1.chuser1,LszUsrID);
	ZeroMathNumeric(&dsF55DC02Key1.chcyno);

	JDB_ClearSelection(hRequestF55DC02);
	jdeNIDcpy(lpSelectF55DC02[0].Item1.szDict, NID_USER1);//Usuario/lector...
	jdeNIDcpy(lpSelectF55DC02[0].Item1.szTable, NID_F55DC02);
	lpSelectF55DC02[0].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF55DC02[0].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF55DC02[0].Item2.szTable, _J(""));
	lpSelectF55DC02[0].Item2.idInstance = (ID)0;
	lpSelectF55DC02[0].lpValue = dsF55DC02Key1.chuser1;
	lpSelectF55DC02[0].nValues = (short)1;
	lpSelectF55DC02[0].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF55DC02[0].nCmp = JDEDB_CMP_EQ;

	jdeNIDcpy(lpSelectF55DC02[1].Item1.szDict, NID_STST);//Status Conteo: Toma... Mayor que 01
	jdeNIDcpy(lpSelectF55DC02[1].Item1.szTable, NID_F55DC02);
	lpSelectF55DC02[1].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF55DC02[1].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF55DC02[1].Item2.szTable, _J(""));
	lpSelectF55DC02[1].Item2.idInstance = (ID)0;
	lpSelectF55DC02[1].lpValue = _J("01");
	lpSelectF55DC02[1].nValues = (short)1;
	lpSelectF55DC02[1].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF55DC02[1].nCmp = JDEDB_CMP_GE;

	jdeNIDcpy(lpSelectF55DC02[2].Item1.szDict, NID_USER1);//Usuario/lector...  Realmente no hace falta.. es un resto de "OR"...
	jdeNIDcpy(lpSelectF55DC02[2].Item1.szTable, NID_F55DC02);
	lpSelectF55DC02[2].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF55DC02[2].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF55DC02[2].Item2.szTable, _J(""));
	lpSelectF55DC02[2].Item2.idInstance = (ID)0;
	lpSelectF55DC02[2].lpValue = dsF55DC02Key1.chuser1;
	lpSelectF55DC02[2].nValues = (short)1;
	lpSelectF55DC02[2].nAndOr = JDEDB_ANDOR_AND; // JDEDB_ANDOR_OR;
	lpSelectF55DC02[2].nCmp = JDEDB_CMP_EQ;

	jdeNIDcpy(lpSelectF55DC02[3].Item1.szDict, NID_STST);//Status Conteo: Toma... Menor que 05
	jdeNIDcpy(lpSelectF55DC02[3].Item1.szTable, NID_F55DC02);
	lpSelectF55DC02[3].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelectF55DC02[3].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelectF55DC02[3].Item2.szTable, _J(""));
	lpSelectF55DC02[3].Item2.idInstance = (ID)0;
	lpSelectF55DC02[3].lpValue = _J("05");
	lpSelectF55DC02[3].nValues = (short)1;
	lpSelectF55DC02[3].nAndOr = JDEDB_ANDOR_AND;
	lpSelectF55DC02[3].nCmp = JDEDB_CMP_LE;

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Construyendo Selection Tabla F55DC02...\n"));
	idResult = JDB_SetSelection(hRequestF55DC02,lpSelectF55DC02,(short)(4),JDEDB_SET_REPLACE);
	if (idResult == JDEDB_FAILED){
		iErrorCode = 509;
		jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection (F55DC02) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	}
	
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F55DC02...\n"));
	idResult = JDB_SelectKeyed(hRequestF55DC02,(ID) 0,(void *)NULL,(short)0);
	if (idResult == JDEDB_FAILED){
		iErrorCode = 510;
		jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed (F55DC02) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	}

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla F55DC02...\n"));		
	idResult = JDB_Fetch(hRequestF55DC02,&dsF55DC02,(int)0);
	if (idResult == JDEDB_FAILED){//NO existe, Nos vamos...
		iErrorCode = 515;
		jdeFprintf(dlg,_J("***Error(%d): JDB_Fetch (EOF F55DC02)...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	}
	else{
		MathNumericToInt(&dsF55DC02.chcyno,&i);
		jdeFprintf(dlg,_J("***JDB_Fetch Conteo F55DC02 (%i)...\n"),i);// Listo, ya tenemos la info sobre el conteo...
		jdeFflush(dlg);
	}
	
	do { //while ( ); Procesar mientras ...

		if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Inicializando 2 pantalla para la etiqueta de codigo de barras...\n"));
			jdeFflush(dlg);
		}
		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
		memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

		OWDCmp90 (iDbgFlg, dlg);//Limpiamos la 1ra pantalla para mostrar la 2da...

		do {
			iErrorCode = 0;

			iPrimeraVez = 0;			
			strcpy(szPantallaTitulo, "Conteo Fisico    ");
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
			//iIdxProd = 0; iIdxDescr = 1; iIdxLot = 2; iIdxCant = 3; iIdxFVcto = 4;
			iIdxProd = 0; iIdxDescr = 1; iIdxLoc = 2; iIdxLot = 3; iIdxFVcto = 4; iIdxCant = 5; 
			strcpy(szTempBuf,szCamposContenido[0]);

			iErrorCode1 = ProcesaCB(szTempBuf,szCamposContenido[iIdxProd],szCamposContenido[iIdxLot],szCamposContenido[iIdxFVcto],
				iUPClenBuf,iSCClenBuf,iDbgFlg,dlg);
	
			if (iErrorCode1 == 1){
				iCamposErrores[0] = 1; // Seteamos el error...
				iErrorCode = 227;
				jdeToUnicode(LszTempBuf,szCamposContenido[0],sizeof(LszTempBuf)-1,UTF8);
				jdeFprintf(dlg,_J("***Error(%d): Etiqueta Invalida: %ls...\n"),iErrorCode,LszTempBuf);
				jdeFflush(dlg);
			}

		} while (iErrorCode != 0);

		if (iSalir > 0){
			if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Break de Etiqueta Larga 2...\n"));
					jdeFflush(dlg);
				};
			break; //...
		}
				
		if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Inicializando 2da pantalla Datos Detalle...\n"));
			jdeFflush(dlg);
		}

//Modificado:	Luis Capriles,		Fecha:	16/10/2012 - Descripción y UM ... ***INICIO**
		memset(LszLineItemBuf,'\0',sizeof(LszLineItemBuf));
		memset(LszUOMdefaultBuf,'\0',sizeof(LszUOMdefaultBuf));
		memset(LszUOMstdConv,'\0',sizeof(LszUOMstdConv));
		memset(LszItemDescriptionBuf,'\0',sizeof(LszItemDescriptionBuf));
		ZeroMathNumeric(&mnItemShortIDBuf);

		jdeToUnicode(LszUsrEntry,szCamposContenido[iIdxProd],25,UTF8);

		if ((iErrorCode = OWDCgbc(hEnv,hUser,LszUsrEntry,LszLineItemBuf,&mnItemShortIDBuf,
								LszUOMdefaultBuf,LszUOMstdConv,LszCrossRefTypeCodeBuf,iUPClenBuf,iSCClenBuf,
								LszItemDescriptionBuf,iGetItemMasterBy,dsGetCrossReferenceFields.c3rdItemNoSymbol,
								iDbgFlg,dlg)) != 0) ; //...

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
									iDbgFlg,dlg)) != 0) ; //...

			if (iDbgFlg > 0) {
				jdeFprintf(dlg,_J("***LszLineItemBuf 2(%ls)...\n"),LszLineItemBuf);
				jdeFprintf(dlg,_J("***LszUOMdefaultBuf 2(%ls)...\n"),LszUOMdefaultBuf);
				jdeToUnicode(LszString01,mnItemShortIDBuf.String,DIM(LszString01),UTF8);
				jdeFprintf(dlg,_J("***mnItemShortIDBuf 2(%ls)...\n"),LszString01);
				jdeFflush(dlg);
			}
		}


		//LszItemDescriptionBuf[20] = '\0';
		jdeFromUnicode(szCamposContenido[iIdxDescr],LszItemDescriptionBuf,20,UTF8);
		szCamposContenido[iIdxDescr][20] = '/';
		jdeFromUnicode(szCamposContenido[iIdxDescr] + 21,LszUOMdefaultBuf,2,UTF8);
		//sprintf(szCamposContenido[iIdxDescr],"%s/%s'\0'",LszItemDescriptionBuf,LszUOMdefaultBuf);
//Modificado:	Luis Capriles,		Fecha:	16/10/2012 - Descripción y UM ... ***FIN**

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		//memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
		memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));
		
		//iPrimeraVez = 0; 
		
		strcpy (szPantallaTitulo, "Conteo Fisico    ");

		szCamposEtiquetas[0] = "Producto     ";
		szCamposEtiquetas[1] = "Desc./UM     ";
		szCamposEtiquetas[2] = "Ubicacion    ";
		szCamposEtiquetas[3] = "Lote         ";
		szCamposEtiquetas[4] = "F.V.Lote     ";
		szCamposEtiquetas[5] = "Cantidad     ";

		iCamposCantidad = 6; // Se Desplegaran 5 Campos: Producto, Lote, Cantidad y además la Descripción

		memset((void *) &mnCantIntroducidaBuf,(int) _J('\0'),sizeof(MATH_NUMERIC));	//Limpiamos Cantidad...
		memset((void *) &LszLineItemBuf,(int) _J('\0'),sizeof(LszLineItemBuf));//Limpiamos Producto por primera vez....
		memset((void *) &LszLineLotBuf,(int) _J('\0'),sizeof(LszLineLotBuf));//Limpiamos Lote por primera vez....
		memset((void *) &LszLotExpirationDate,(int) _J('\0'),sizeof(LszLotExpirationDate));//Limpiamos FVLote por primera vez....
		memset((void *) &LszLineLocationBuf,(int) _J('\0'),sizeof(LszLineLocationBuf));//Limpiamos Ubicación por primera vez....

		jdeStrcpy(LszLineLocationBuf,LszUbicacionDfltBuf); 

		// Se Utilizan los Indices por Campo para controlarlos mas Abajo
		iIdxProd = 0; iIdxDescr = 1; iIdxLoc = 2; iIdxLot = 3; iIdxFVcto = 4;  iIdxCant = 5; 

		iPrimeraVez = 0;
		iErrorCode = 0;

		do { //Loop para validar codigo barra y demás..
			jdeErrorClearEx(lpBhvrCom,lpVoid);
			iErrorCode = 0;
			//***Producto
			//jdeFromUnicode(szCamposContenido[iIdxProd],LszLineItemBuf, 26,UTF8); // Viene del scanner...
			iCamposPosiciones[iIdxProd] = strlen(szCamposContenido[iIdxProd]);
			iCamposPosiciones[iIdxDescr] = 0;
			iCamposErrores[iIdxDescr] = 3;//No queremos inverse video...
				

			//***Cantidad									
			sprintf(szCamposContenido[iIdxCant],"%d",iLineQtyBuf);
			iCamposPosiciones[iIdxCant] = strlen(szCamposContenido[iIdxCant]);
			//iCamposErrores[iIdxCant] = 2; //Nos queremos posicionar acá... 

			//***Ubicacion									
			jdeFromUnicode(szCamposContenido[iIdxLoc],LszLineLocationBuf,21,UTF8);
			iCamposPosiciones[iIdxLoc] = strlen(szCamposContenido[iIdxLoc]);
			
			//***Lote
			//jdeFromUnicode(szCamposContenido[iIdxLot],LszLineLotBuf,31,UTF8); // Viene del scanner...
			iCamposPosiciones[iIdxLot] = strlen(szCamposContenido[iIdxLot]);
			iCamposErrores[iIdxLot] = 2; //Nos queremos posicionar acá...

			//***F.V.Lote
			//jdeFromUnicode(szCamposContenido[iIdxFVcto],LszLotExpirationDate,17,UTF8); // Viene del scanner...
			//iCamposPosiciones[iIdxFVcto] = strlen(szCamposContenido[iIdxFVcto]);

			//(iCamposOffset + 4), (iStatusOffset -4)
			OWDCmp02 (&iPrimeraVez, szPantallaTitulo, (iCamposOffset + 1), iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
						iInicioCampos, iUltimaLinea, iCReqTAB,
						iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
						iDbgFlg, dlg);
			memset(iCamposErrores,'\0',sizeof(iCamposErrores));
			memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));	

			//***Codigo Producto...
			jdeToUnicode(LszUsrEntry,szCamposContenido[iIdxProd],26,UTF8);
			if (jdeStrlen(LszUsrEntry) == 0 ) {
				iSalir = 1;
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Break de Etiqueta 1...\n"));
					jdeFflush(dlg);
				};
				break;  //Ya no mas: salir!!!
			}
			else iSalir = 0;

			//***La cantidad...
			fLineQtyBuf = atof(szCamposContenido[iIdxCant]);
			DoubleToMathNumeric(fLineQtyBuf,&mnCantIntroducidaBuf);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***fLineQtyBuf(%lf)...\n"),fLineQtyBuf);
			if (fLineQtyBuf == 0 ) {
				iSalir = 0; //Realmente no queremos salir.. solo ignorar: mas abajo se compara si fLineQtyBuf > 0...
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Break de Etiqueta 1a...\n"));
					jdeFflush(dlg);
				};
				break;  //Ya no mas: otro!!!
			}
			else iSalir = 0;

			//jdeStrcpy(LszTempBuf,_J("                         "));//ItemMasterItem Left Justified padded with blanks...
			//jdeStrncpy(LszLineItemBuf,LszUsrEntry,25);
			//LszLineItemBuf[25] = '\0';
			//jdeStrcat(LszLineItemBuf,LszTempBuf + jdeStrlen(LszUsrEntry));
			//if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszLineItemBuf(%ls)...\n"),LszUsrEntry);

			//***La Ubicación...
			jdeToUnicode(LszLineLocationBuf,szCamposContenido[iIdxLoc],21,UTF8);//Se copia para validar la Ubicacion Ingresada...
			LszLineLocationBuf[20] = _J('\0');
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***LszLineLocationBuf (%ls)...\n"),LszLineLocationBuf);
				jdeFflush(dlg);
			}

			//***El Lote...
			jdeToUnicode(LszLineLotBuf,szCamposContenido[iIdxLot],31,UTF8);//Se copia para validar el lote Ingresado...
			LszLineLotBuf[30] = _J('\0');
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***LszLineLotBuf (%ls)...\n"),LszLineLotBuf);
				jdeFflush(dlg);
			}

			//***La F.V. Lote...
			jdeToUnicode(LszLotExpirationDate,szCamposContenido[iIdxFVcto],17,UTF8);//Se copia para validar la F.V.lote Ingresado...
				
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
				jdeFprintf(dlg,_J("***LszLineItemBuf 1(%ls)...\n"),LszLineItemBuf);
				jdeFprintf(dlg,_J("***LszUOMdefaultBuf 1(%ls)...\n"),LszUOMdefaultBuf);
				jdeToUnicode(LszString01,mnItemShortIDBuf.String,DIM(LszString01),UTF8);
				jdeFprintf(dlg,_J("***mnItemShortIDBuf 1(%ls)...\n"),LszString01);
				jdeFflush(dlg);
			}

			if (iErrorCode != 0){ //Hubo error... vamos a quitar ceros para ver...
				iCamposErrores[iIdxProd] = 1;
				iErrorCode = 0;

				/**  Aquí no validamos esto...
				const char* firstNonSpace = szCamposContenido[iIdxProd];
				while(*firstNonSpace != '\0' && (isspace(*firstNonSpace) || firstNonSpace[0] == '0'))
				{
					++firstNonSpace;
				}
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
				*/
			}

			if (iErrorCode == 0) {//Ahora que el CB es válido, vamos a determinar el MCU de este (Item,Lote)...
				memset((void *) &dsGetItemMasterByShortItem,(int) _J('\0'),sizeof(DSDX4101B));
				MathCopy(&dsGetItemMasterByShortItem.mnShortItemNumber,&mnItemShortIDBuf);
				dsGetItemMasterByShortItem.cReturnPtr = _J('1');
				dsGetItemMasterByShortItem.cSuppressErrorMsg = _J('1');
				idResult = jdeCallObject (_J("GetItemMasterByShortItem"), NULL,lpBhvrCom,lpVoid,
											(LPVOID)&dsGetItemMasterByShortItem,(CALLMAP*)NULL,(int)0,
											(JCHAR*)NULL,(JCHAR*)NULL,(int)0);
				if (idResult == ER_ERROR){
					iErrorCode = 519;
					jdeFprintf(dlg,_J("***Error(%d): GetItemMasterByShortItem:...\n"), iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				} 
				lpdsF4101 = (LPF4101)jdeRemoveDataPtr(hUser,(unsigned long)dsGetItemMasterByShortItem.idF4101LongRowPtr);
				jdeStrcpy(LszUOMprimaryBuf,lpdsF4101->imuom1);//UOM Primaria...
				if (lpdsF4101 != (LPF4101) NULL){
					jdeFree((LPVOID)lpdsF4101);
					lpdsF4101 = (LPF4101)NULL;
				}

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemMasterByShortItem - UOM (%ls)...\n"),LszUOMprimaryBuf);

				memset ((void *)(&dsGetItemUoMConversionFactor), (int)(_J('\0')), sizeof(DSD4000520));
				jdeStrcpy (dsGetItemUoMConversionFactor.szFromUnitOfMeasure,LszUOMdefaultBuf);
				jdeStrcpy (dsGetItemUoMConversionFactor.szToUnitOfMeasure,LszUOMprimaryBuf);
				MathCopy (&dsGetItemUoMConversionFactor.mnShortItemNumber, &mnItemShortIDBuf);
				MathCopy (&dsGetItemUoMConversionFactor.mnQuantityToConvert,&mnCantIntroducidaBuf);

				jdeCallObject(_J("GetItemUoMConversionFactor"),NULL,lpBhvrCom,lpVoid,&dsGetItemUoMConversionFactor,
					(LPCALLMAP)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);

				FormatMathNumeric(LszString01,&mnCantIntroducidaBuf);
				FormatMathNumeric(LszString02,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetItemUoMConversionFactor FROM/TO (%s/%s)...\n"),LszString01,LszString02);

				//memset((void *) &dsF41021Key6,(int) _J('\0'),sizeof(dsF41021Key6));
				memset((void *) &dsF4102Key2,(int) _J('\0'),sizeof(dsF4102Key2));
				memset((void *) &dsF4102,(int) _J('\0'),sizeof(dsF4102));
				memset((void *) &dsB41021,(int) _J('\0'),sizeof(dsB41021));

				//***El lote...
				//Modificado:	Luis Capriles,		Fecha:	03/07/2013 - Manejo de MCU Principal ... ***INICIO***
				//Modificado:	Luis Capriles,		Fecha:	05/11/2012 - Manejo de multiples ini, varios LOCATION ...***INICIO***
				/*

				MathCopy(&dsF41021Key6.liitm,&mnItemShortIDBuf);
				jdeStrncpy(dsF41021Key6.lilocn,LszLineLocationBuf,DIM(dsF41021Key6.lilocn) - 1);
				jdeStrncpy(dsF41021Key6.lilotn,LszLineLotBuf,30);

				JDB_ClearSelection(hRequestF41021);
				jdeNIDcpy(lpSelectF41021[0].Item1.szDict, NID_ITM);//Código corto...
				jdeNIDcpy(lpSelectF41021[0].Item1.szTable, NID_F41021);
				lpSelectF41021[0].Item1.idInstance = (ID)0;
				jdeNIDcpy(lpSelectF41021[0].Item2.szDict, _J(""));
				jdeNIDcpy(lpSelectF41021[0].Item2.szTable, _J(""));
				lpSelectF41021[0].Item2.idInstance = (ID)0;
				lpSelectF41021[0].lpValue = &dsF41021Key6.liitm;
				lpSelectF41021[0].nValues = (short)1;
				lpSelectF41021[0].nAndOr = JDEDB_ANDOR_AND;
				lpSelectF41021[0].nCmp = JDEDB_CMP_EQ;

				jdeNIDcpy(lpSelectF41021[1].Item1.szDict, NID_LOCN);//Ubicación default...
				jdeNIDcpy(lpSelectF41021[1].Item1.szTable, NID_F41021);
				lpSelectF41021[1].Item1.idInstance = (ID)0;
				jdeNIDcpy(lpSelectF41021[1].Item2.szDict, _J(""));
				jdeNIDcpy(lpSelectF41021[1].Item2.szTable, _J(""));
				lpSelectF41021[1].Item2.idInstance = (ID)0;
				lpSelectF41021[1].lpValue = dsF41021Key6.lilocn;
				lpSelectF41021[1].nValues = (short)1;
				lpSelectF41021[1].nAndOr = JDEDB_ANDOR_AND;
				lpSelectF41021[1].nCmp = JDEDB_CMP_EQ;

				jdeNIDcpy(lpSelectF41021[2].Item1.szDict, NID_LOTN);//Lote...
				jdeNIDcpy(lpSelectF41021[2].Item1.szTable, NID_F41021);
				lpSelectF41021[2].Item1.idInstance = (ID)0;
				jdeNIDcpy(lpSelectF41021[2].Item2.szDict, _J(""));
				jdeNIDcpy(lpSelectF41021[2].Item2.szTable, _J(""));
				lpSelectF41021[2].Item2.idInstance = (ID)0;
				lpSelectF41021[2].lpValue = dsF41021Key6.lilotn;
				lpSelectF41021[2].nValues = (short)1;
				lpSelectF41021[2].nAndOr = JDEDB_ANDOR_AND;
				lpSelectF41021[2].nCmp = JDEDB_CMP_EQ;

				idResult = JDB_SetSelection(hRequestF41021,lpSelectF41021,(short)(3),JDEDB_SET_REPLACE);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 512;
					iCamposErrores[1] = 1; // Seteamos el error...
					jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection (F41021) failed...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}
				
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F41021...\n"));
				idResult = JDB_SelectKeyed(hRequestF41021,(ID) 0,(void *) NULL,(short)0);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 511;
					jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed (F41021) failed...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}
				if (iDbgFlg > 0) {
					jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla F41021...\n"));	

					jdeFflush(dlg); 
				}
				idResult = JDB_Fetch(hRequestF41021,&dsF41021,(int)0);
				*/
				//Modificado:	Luis Capriles,		Fecha:	05/11/2012 - Manejo de multiples ini, varios LOCATION ...***FIN***
				MathCopy(&dsF4102Key2.ibitm,&mnItemShortIDBuf);

				JDB_ClearSelection(hRequestF4102);
				jdeNIDcpy(lpSelectF4102[0].Item1.szDict, NID_ITM);//Código corto...
				jdeNIDcpy(lpSelectF4102[0].Item1.szTable, NID_F4102);
				lpSelectF4102[0].Item1.idInstance = (ID)0;
				jdeNIDcpy(lpSelectF4102[0].Item2.szDict, _J(""));
				jdeNIDcpy(lpSelectF4102[0].Item2.szTable, _J(""));
				lpSelectF4102[0].Item2.idInstance = (ID)0;
				lpSelectF4102[0].lpValue = &dsF4102Key2.ibitm;
				lpSelectF4102[0].nValues = (short)1;
				lpSelectF4102[0].nAndOr = JDEDB_ANDOR_AND;
				lpSelectF4102[0].nCmp = JDEDB_CMP_EQ;

				jdeNIDcpy(lpSelectF4102[1].Item1.szDict, NID_URCD);//Flag de 'Primario'...
				jdeNIDcpy(lpSelectF4102[1].Item1.szTable, NID_F4102);
				lpSelectF4102[1].Item1.idInstance = (ID)0;
				jdeNIDcpy(lpSelectF4102[1].Item2.szDict, _J(""));
				jdeNIDcpy(lpSelectF4102[1].Item2.szTable, _J(""));
				lpSelectF4102[1].Item2.idInstance = (ID)0;
				lpSelectF4102[1].lpValue = _J("  ");
				lpSelectF4102[1].nValues = (short)1;
				lpSelectF4102[1].nAndOr = JDEDB_ANDOR_AND;
				lpSelectF4102[1].nCmp = JDEDB_CMP_NE;

				//idResult = JDB_SetSelection(hRequestF41021,lpSelectF41021,(short)(3),JDEDB_SET_REPLACE);
				//Buscamos el que tiene  la marca 'P' en URCD...
				idResult = JDB_SetSelection(hRequestF4102,lpSelectF4102,(short)(2),JDEDB_SET_REPLACE);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 512;
					iCamposErrores[1] = 1; // Seteamos el error...
					jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection (F4102*) failed...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}
				//Modificado:	Luis Capriles,		Fecha:	05/11/2012 - Manejo de multiples ini, varios LOCATION ...***FIN***

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F4102*...\n"));
				//idResult = JDB_SelectKeyed(hRequestF41021,(ID) 0,(void *) NULL,(short)0);
				idResult = JDB_SelectKeyed(hRequestF4102,(ID) 0,(void *) NULL,(short)0);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 511;
					jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed (F4102*) failed...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}
				if (iDbgFlg > 0) {
					jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla F4102*...\n"));	

					jdeFflush(dlg); 
				}
				//idResult = JDB_Fetch(hRequestF41021,&dsF41021,(int)0);
				idResult = JDB_Fetch(hRequestF4102,&dsF4102,(int)0);
				//Modificado:	Luis Capriles,		Fecha:	03/07/2013 - Manejo de MCU Principal ... ***FIN***
				if (idResult == JDEDB_FAILED){//NO existe, error...
					//Intentamos con el primero...
					//Modificado:	Luis Capriles,		Fecha:	05/11/2012 - Manejo de multiples ini, varios LOCATION ...***INICIO***
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select 2 sobre Tabla F4102*...\n"));
					idResult = JDB_SetSelection(hRequestF4102,lpSelectF4102,(short)(1),JDEDB_SET_REPLACE);
					if (idResult == JDEDB_FAILED){
						iErrorCode = 512;
						iCamposErrores[1] = 1; // Seteamos el error...
						jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection 2 (F4102*) failed...\n"),iErrorCode);
						jdeFflush(dlg);
						goto lbFIN;
					}

					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select 2 sobre Tabla F4102*...\n"));
					idResult = JDB_SelectKeyed(hRequestF4102,(ID) 0,(void *) NULL,(short)0);
					if (idResult == JDEDB_FAILED){
						iErrorCode = 511;
						jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed 2 (F4102*) failed...\n"),iErrorCode);
						jdeFflush(dlg);
						goto lbFIN;
					}
					if (iDbgFlg > 0) {
						jdeFprintf(dlg,_J("***Ejecutando Fetch 2 sobre Tabla F4102*...\n"));	

						jdeFflush(dlg); 
					}
					idResult = JDB_Fetch(hRequestF4102,&dsF4102,(int)0);
					if (idResult == JDEDB_FAILED){//NO existe, error...
						if (iErrorValidarLote == 1){
							iErrorCode = 518;
							iCamposErrores[iIdxProd] = 1; //Seteamos el error...
							iCamposErrores[iIdxLot] = 1; //Seteamos el error...
							jdeFprintf(dlg,_J("***Error(%d): JDB_Fetch (EOF F4102*)...\n"),iErrorCode);
							jdeFflush(dlg);
							//goto lbFIN;
						}				 
						jdeStrcpy(dsF4102.ibmcu,_J("999999999999"));
					}else{//Tenemos el mcu pata completar (Item,Lot,MCU)... Y calcular la cantidad en mano...
						MathCopy(&dsB41021.mnShortItemNumber,&mnItemShortIDBuf);
						jdeStrncpy(dsB41021.szBranchPlant,dsF4102.ibmcu,DIM(dsB41021.szBranchPlant) - 1);
						jdeStrncpy(dsB41021.szLocation,LszLineLocationBuf,DIM(dsB41021.szLocation) - 1);
						jdeStrncpy(dsB41021.szLot,LszLineLotBuf,DIM(dsB41021.szLot) - 1);

						dsB41021.cIndex	= _J('1');
						dsB41021.cKeys = _J('4');

						idResult = jdeCallObject(_J("CalculateQtyOnHand"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsB41021,
											(CALLMAP*)NULL,(int)0,(JCHAR*)NULL,(JCHAR*)NULL,(int)0);

						jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
						while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){	
							iErrorCode = 521;
							jdeFprintf(dlg,_J("***Error(%d): CalculateQtyOnHand 2: (%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc);
							jdeFprintf(dlg,_J("***MCU/LOC/LOT 2(%ls/%ls/%ls)...\n"),dsB41021.szBranchPlant, dsB41021.szLocation, dsB41021.szLot);
							jdeFflush(dlg);
						} 
					}
				}else{//Tenemos el mcu pata completar (Item,Lot,MCU)... Y calcular la cantidad en mano...
					MathCopy(&dsB41021.mnShortItemNumber,&mnItemShortIDBuf);
					jdeStrncpy(dsB41021.szBranchPlant,dsF4102.ibmcu,DIM(dsB41021.szBranchPlant) - 1);
					jdeStrncpy(dsB41021.szLocation,LszLineLocationBuf,DIM(dsB41021.szLocation) - 1);
					jdeStrncpy(dsB41021.szLot,LszLineLotBuf,DIM(dsB41021.szLot) - 1);

					dsB41021.cIndex	= _J('1');
					dsB41021.cKeys = _J('4');

					idResult = jdeCallObject(_J("CalculateQtyOnHand"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsB41021,
										(CALLMAP*)NULL,(int)0,(JCHAR*)NULL,(JCHAR*)NULL,(int)0);

					jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
					while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){	
						iErrorCode = 521;
						jdeFprintf(dlg,_J("***Error(%d): CalculateQtyOnHand: (%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						jdeFprintf(dlg,_J("***MCU/LOC/LOT 2(%ls/%ls/%ls)...\n"),dsB41021.szBranchPlant, dsB41021.szLocation, dsB41021.szLot);
						jdeFflush(dlg);
					} 
				}
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***MCU/LOC/LOT 1(%ls/%ls/%ls)...\n"),dsF4102.ibmcu, LszLineLocationBuf, LszLineLotBuf);
					jdeFflush(dlg);
				}
				
				// Se Valida la Ubicacion Ingresada
				//***Solicita y valida localidad/ubicación Por Defecto del Usuario
				//Index = 3, Keys 2 means fetch by BranchPlant and Location.
				/*
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Validando Ubicación (%ls)...\n"),LszLineLocationBuf);
					jdeFflush(dlg);
				}
				ParseNumericString(&dsVerifyAndGetItemLocation.mnIndex,_J("3"));
				ParseNumericString(&dsVerifyAndGetItemLocation.mnKeys,_J("2")); 
				dsVerifyAndGetItemLocation.cReturnRecord  = _J('0'); //Solo validar, no queremos data...
				dsVerifyAndGetItemLocation.cCallType = _J('1'); //Devolver Error si no hay registro...
				jdeStrcpy(dsVerifyAndGetItemLocation.szBranchPlant,dsF4102.ibmcu);
				jdeStrcpy(dsVerifyAndGetItemLocation.szLocation,LszLineLocationBuf);
				jdeCallObject(_J("VerifyAndGetItemLocation"), NULL, lpBhvrCom, lpVoid,
							(LPVOID)&dsVerifyAndGetItemLocation,(CALLMAP *)NULL,(int)0, 
							(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){
					//iErrorCode = 520;
					//iCamposErrores[iIdxLoc] = 1; // Seteamos el error...
					jdeFprintf(dlg,_J("***Error(%d): VerifyAndGetItemLocation: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				}
				*/

				// Se procesa la Fecha Vencimiento Lote...
				if (strlen(szCamposContenido[iIdxFVcto]) != 0) { //Si hay fecha...
					iErrorCode1 = FormateaFecha(szCamposContenido[iIdxFVcto],LszLotExpirationDate);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszLotExpirationDate(%ls) ...\n"),LszLotExpirationDate);
					if (iErrorCode1 != 0) {// Indica que la fecha es nula...
						if (iErrorCode == 0) iErrorCode = iErrorCode1;
						iCamposErrores[iIdxFVcto] = 1; // Seteamos el error...
						jdeFprintf(dlg,_J("***Error(%d): Fecha Vencimiento Lote Invalida (%s)...\n"),iErrorCode1,LszLotExpirationDate);
					}
				}
			}
			jdeFflush(dlg);
		} while (iErrorCode != 0); //Loop para validar codigo barra...

		if (iSalir > 0){
			if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Break de Etiqueta 2...\n"));
					jdeFflush(dlg);
				};
			break; //Ya no mas: salir!!!
		}

		if (fLineQtyBuf != 0 ) {// Hay cantidad para procesa...
		
			//Tenemos el mcu pata completar (Item,Lot,MCU)...
			if (iDbgFlg > 0){ 
				FormatMathNumeric(LszString01,&dsB41021.mnQuantityOnHand);
				jdeFprintf(dlg,_J("***JDB_Fetch F4102* - MCU/QtyOH(%ls/%ls)...\n"),dsF4102.ibmcu,LszString01);
				jdeFflush(dlg);
			}
			memset((void *) &dsX00022,(int) _J('\0'),sizeof(dsX00022));

			jdeStrncpy((JCHAR *)dsX00022.szObjectName, _J("F55DC021"),DIM(dsX00022.szObjectName) - 1);
			ZeroMathNumeric(&dsX00022.mnUniqueKeyID);

			jdeCallObject(_J("GetNextUniqueKeyID"),NULL,lpBhvrCom,lpVoid,&dsX00022,
				(CALLMAP*)NULL,(int)0,(JCHAR*)NULL,(JCHAR*)NULL,(int)0);

			if (MathZeroTest(&dsX00022.mnUniqueKeyID) == 0){
				iErrorCode = 516;
				jdeFprintf(dlg,_J("***Error(%d): GetNextUniqueKeyID (F55DC021) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}
			//***Actualiza Usuario, máquina, fecha
			idResult = jdeCallObject(_J("GetAuditInfo"),NULL,lpBhvrCom,lpVoid,&dsGetAuditInfo,
							(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
			if (idResult == ER_ERROR){
				iErrorCode = 517;
				jdeFprintf(dlg,_J("***Error(%d): GetAuditInfo 2:...\n"), iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}

			FormatMathNumeric(LszString01,&dsX00022.mnUniqueKeyID);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***JDB_InsertTable F55DC021 (%s)...\n"),LszString01);

			memset((void *) &dsF55DC021,(int) _J('\0'),sizeof(F55DC021));

			MathCopy(&dsF55DC021.cdcyno,&dsF55DC02.chcyno);//El conteo...
			jdeStrcpy(dsF55DC021.cdstst,dsF55DC02.chstst);//El estado del conteo: la toma...
			MathCopy(&dsF55DC021.cditm,&mnItemShortIDBuf);//El item...
			jdeStrncpy(dsF55DC021.cdlitm,dsGetItemMasterByShortItem.sz2ndItemNumber,DIM(dsF55DC021.cdlitm) - 1);//El código de barras...
			jdeStrncpy(dsF55DC021.cdcitm,LszUsrEntry,DIM(dsF55DC021.cdcitm) - 1);//El código de barras...
			jdeStrcpy(dsF55DC021.cdmcu,dsF4102.ibmcu);//El mcu encontrado más arriba...
			jdeStrncpy(dsF55DC021.cdlocn,LszLineLocationBuf,DIM(dsF55DC021.cdlocn) - 1);//La ubicación por default...
			jdeStrcpy(dsF55DC021.cdlotn,LszLineLotBuf);//El lote...
			DeformatDate(&dsF55DC021.cdmmej,LszLotExpirationDate,(JCHAR*) _J("ASOSE")); //La FV Lote... 
			MathCopy(&dsF55DC021.cdpqoh,&dsB41021.mnQuantityOnHand);//La cantidad disponible...
			jdeStrncpy(dsF55DC021.cdtrum,LszUOMdefaultBuf,DIM(dsF55DC021.cdtrum) - 1);//La UM de la transacción... 
			MathCopy(&dsF55DC021.cdtrqt,&mnCantIntroducidaBuf);//La cantidad de la transacción...
			JDEDATECopy(&dsF55DC021.cdtrdj,&dsGetAuditInfo.jdDate);//La fecha de la transacción...
			MathCopy(&dsF55DC021.cdupmt,&dsGetAuditInfo.mnTime);//La hora de la transaccion...
			MathCopy(&dsF55DC021.cdqtyadjpr,&dsGetItemUoMConversionFactor.mnQuantityConvertedFromtoTo);//La cantidad en UM primaria...
			jdeStrcpy(dsF55DC021.cduser1,dsGetAuditInfo.szUserName);//El usuario que lee...
			jdeStrcpy(dsF55DC021.cdpid,_J("TC01"));
			jdeStrcpy(dsF55DC021.cduser,dsGetAuditInfo.szUserName);
			jdeStrcpy(dsF55DC021.cdjobn,dsGetAuditInfo.szWorkstation_UserId);
			JDEDATECopy(&dsF55DC021.cdupmj,&dsGetAuditInfo.jdDate);
			MathCopy(&dsF55DC021.cdtday,&dsGetAuditInfo.mnTime);


			MathCopy(&dsF55DC021.cdukid,&dsX00022.mnUniqueKeyID);

			idResult = JDB_InsertTable (hRequestF55DC021,NID_F55DC021,(ID)0,(void *)&dsF55DC021);
			if (idResult == JDEDB_FAILED){
				iErrorCode = 513;
				jdeFprintf(dlg,_J("***Error(%d): JDB_InsertTable (F55DC021) failed...\n"),iErrorCode);
				jdeFflush(dlg);
			goto lbFIN;
			}

		} //if (fLineQtyBuf != 0 )
		
	}while (TRUE);


	//*************************************************************************
	// Clean up the lpBhvrCom, lpVoid and free user and environment 
	//*************************************************************************

lbFIN:
	JDB_CloseTable(hRequestF4102);
lbFIN2:
	JDB_CloseTable(hRequestF55DC021);	
lbFIN1:
	JDB_CloseTable(hRequestF55DC02);	
lbFIN0:
	OWDCmp90 (iDbgFlg, dlg);//Limpiamos para mostrar menu...
	jdeFflush(dlg);

	jdeErrorClearEx(lpBhvrCom,lpVoid); 
	jdeFree(((LPCG_BHVR)lpVoid)->lpErrorEventKey);
	jdeErrorTerminateEx(((LPCG_BHVR)lpVoid)->lpHdr);
	jdeFreeBusinessFunctionParms(lpBhvrCom,lpVoid);

lbFin00:
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit OWDCvc01(%d)...\n"), iErrorCode);
	jdeFflush(dlg);

	if (iDbgFlg == 1) jdeFclose(dlg);

	return iErrorCode;
}

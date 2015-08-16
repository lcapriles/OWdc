// 03vc01.cpp 
//
//Creado:		Luis Capriles,		Fecha:	01/10/2012 
//
//El programa vc01 permite validar la codificación de los items de inventario.
//
#include "vc01.h"
#include <stdio.h>
#include "jde.h"

#include "F55DC01.h"

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
#include "B1100007.h"	// DecimalsTriggerGetbyCOCRCD

void OWDCmp02 (int * primeraVez,char * pantallaTitulo,int camposOffset,int camposCantidad, 
				int statusOffset,int inicioEtiquetas,int inicioCampos,int ultimaLinea,int CReqTAB,
				int camposPosiciones[],char * camposEtiquetas[],char camposContenido[][128],int camposErrores[],
				char * pantallaStatusLine,int iDbgFlg,FILE * dlg); //Manejo de la pantalla de entrada datos...

void OWDCmp90 (int iDbgFlg, FILE * dlg); //Terminar Manejo de la pantalla curses...

int OWDCgbc(HENV hEnv,HUSER hUser,JCHAR LszUsrEntry[128],JCHAR LszLineItemBuf[26],MATH_NUMERIC * mnItemShortIDBuf,
			JCHAR LszUOMdefaultBuf[3],JCHAR LszUOMstdConv[3],JCHAR LszCrossRefTypeCodeBuf[3],int iUPClenBuf,int iSCClenBuf,
			JCHAR szItemDescription[31],int iGetItemMasterBy, JCHAR c3rdItemNoSymbol, int iDbgFlg,FILE * dlg); //Valida código producto...

int ProcesaCB(char * szCodigoBarra, char * szCBprod,char * szCBlote,char * szCBfecha,int iUPClenBuf,int iSCClenBuf,int iDbgFlg, FILE * dlg);


__declspec(dllexport) int OWDCvc01(HENV hEnv,HUSER hUser)
{
	LPJDEERROR_RECORD	ErrorRec	= NULL;
	ID					idResult;
	LPCG_BHVR			lpVoid		= NULL;
	LPBHVRCOM			lpBhvrCom	= NULL;
	ERROR_EVENT_KEY		EventKeyLocal;

	DSD4000150			dsGetBranchConstants;
	DSDX4101B			dsGetItemMasterByShortItem;
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
	DSD1100007			dsDecimalsTriggerGetbyCOCRCD;
	
	MATH_NUMERIC		mnItemShortIDBuf;

	HREQUEST			hRequestF55DC01 = (HREQUEST)NULL;
	F55DC01				dsF55DC01,dsF55DC01dummy;
	KEY1_F55DC01		dsF55DC01Key1;
	SELECTSTRUCT		lpSelect[4];

	LPF4101				lpdsF4101 = (LPF4101) NULL;


	FILE * ini;
	FILE * dlg;

	JCHAR LszLinea[80],LszLin1[64],LszLin2[64],LszUsrEntry[128],

		 LszDocBranchPlantBuf[16],LszLocationBuf[21],LszLineLocationBuf[21],
		 LszLineItemBuf[26],LszLineUMBuf[3],LszUOMdefaultBuf[3],LszUOMstdConv[3],LszLotExpirationDate[16],

		 LszCrossRefTypeCodeBuf[3],LszDocumentCompanyBuf[6],LszItemDescriptionBuf[31], 

		 LszString01[64],LszString02[64],LszString03[64],LszString04[64],LszTempBuf[128],

		 LcDecimalCharBuf,
		 LcYesNoBuf,LcTempBuf,LcTemp1Buf; 

	JDEDATE	jFechaTemp;

	int	 iUPClenBuf,iSCClenBuf,iGetItemMasterBy,
		 iErrorCode,iErrorCode1,iDbgFlg,i,j,iSalir,iError1,iError2,iProdNoExiste,
		 iPrimeraVez,iCamposOffset,iCamposCantidad,iStatusOffset,iInicioEtiquetas,iInicioCampos,iUltimaLinea,iCReqTAB,iUnicaVez,
		 iCamposPosiciones[64], iCamposErrores[64];
		 
	int	 iIdxProd = 0, iIdxDescr = 0, iIdxNART = 0, iIdxUM = 0;

	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128],
		 * szDummy1,szTempBuf[128],*szTempBuf1,*szTempBuf2,szCamposContenidoTemp[64][128];


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
	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCvc01_D"));
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

	ini = jdeFopen(_J("OWDCvc01.ini"),_J("r"));
	if (!ini){
		jdeFprintf(stderr,_J("***Error abriendo INI (OWDCvc01.ini)...\n"));
        return 400;
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
		iErrorCode = 401;
		jdeFprintf(dlg,_J("***Error(%d): GetAuditInfo:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 

	//***Busca Almacén por default del usuario
	idResult = jdeCallObject(_J("GetDefaultBranch"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsF40095GetDefaultBranch, 
						(CALLMAP *)NULL, (int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 402;
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
		iErrorCode = 403;
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
		iErrorCode = 405;
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
		jdeFprintf(dlg,_J("***Abriendo Tabla F55DC01...\n"));
		jdeFflush(dlg);
	}
	idResult = JDB_OpenTable(hUser,NID_F55DC01,ID_F55DC01_PK,NULL,(ushort)(0),
							(JCHAR *)NULL,&hRequestF55DC01);
	if (idResult == JDEDB_FAILED){
		iErrorCode = 408;
		jdeFprintf (dlg,_J("***Error(%d): JDB_OpenTable(F55DC01) failed...\n"),iErrorCode);
		jdeFflush(dlg);
		goto lbFIN0;
	}
	
	//Construye Where del select...
	JDB_ClearSelection(hRequestF55DC01);
	jdeNIDcpy(lpSelect[0].Item1.szDict, NID_ITM);//Codigo corto...
	jdeNIDcpy(lpSelect[0].Item1.szTable, NID_F55DC01);
	lpSelect[0].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelect[0].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelect[0].Item2.szTable, _J(""));
	lpSelect[0].Item2.idInstance = (ID)0;
	//	lpSelect[0].lpValue = dsF55DC01Key1.viitm;
	lpSelect[0].nValues = (short)1;
	lpSelect[0].nAndOr = JDEDB_ANDOR_AND;
	lpSelect[0].nCmp = JDEDB_CMP_EQ;

	jdeNIDcpy(lpSelect[1].Item1.szDict, NID_CITM);//Codigo de barra...
	jdeNIDcpy(lpSelect[1].Item1.szTable, NID_F55DC01);
	lpSelect[1].Item1.idInstance = (ID)0;
	jdeNIDcpy(lpSelect[1].Item2.szDict, _J(""));
	jdeNIDcpy(lpSelect[1].Item2.szTable, _J(""));
	lpSelect[1].Item2.idInstance = (ID)0;
	//	lpSelect[1].lpValue = dsF55DC01Key1.vicitm;
	lpSelect[1].nValues = (short)1;
	lpSelect[1].nAndOr = JDEDB_ANDOR_AND;
	lpSelect[1].nCmp = JDEDB_CMP_EQ;
		
	do { //while (TRUE)...

		if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Inicializando 1ra pantalla para la etiqueta de codigo de barras...\n"));
			jdeFflush(dlg);
		}
		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
		memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

		OWDCmp90 (iDbgFlg, dlg);//Limpiamos ...

		do {
			iErrorCode = 0;
			iPrimeraVez = 0;

			strcpy (szPantallaTitulo, "Validacion Items Inventario    ");
			szCamposEtiquetas[0] = "Codigo Barra  ";
			strcpy(szCamposContenido[0],"");
			iCamposPosiciones[0] = strlen(szCamposContenido[0]);
			iCamposCantidad = 1;
			memset(szCamposContenido,'\0',sizeof(szCamposContenido));
			strcpy(szCamposContenido[0],"");
			iCamposPosiciones[0] = strlen(szCamposContenido[0]);

			OWDCmp02 (&iPrimeraVez,szPantallaTitulo,iCamposOffset,iCamposCantidad,iStatusOffset,iInicioEtiquetas, 
			iInicioCampos,iUltimaLinea,iCReqTAB,
			iCamposPosiciones,szCamposEtiquetas,szCamposContenido,iCamposErrores,szPantallaStatusLine,
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
			iIdxProd = 0; iIdxDescr = 1; iIdxNART = 2; iIdxUM = 3;

			szTempBuf1 = NULL;
			szTempBuf2 = NULL;

			strcpy(szTempBuf,szCamposContenido[0]);

			strcpy(szDummy,"");

			iErrorCode1 = ProcesaCB(szTempBuf,szCamposContenido[iIdxProd],szDummy,szDummy,
						iUPClenBuf,iSCClenBuf,iDbgFlg,dlg);

	
			if (iErrorCode1 == 1){
				iCamposErrores[0] = 1; // Seteamos el error...
				iErrorCode = 406;
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
			break; //Queremos subir...
		}


        if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Inicializando 2da pantalla Datos Detalle...\n"));
			jdeFflush(dlg);
		}

		//***Codigo Producto...  Queremos validar si existe...
		jdeToUnicode(LszUsrEntry,szCamposContenido[iIdxProd],26,UTF8);
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

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		//memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));
		memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
		memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

		strcpy (szPantallaTitulo, "Validacion Items Inventario    ");

		szCamposEtiquetas[0] = "Producto       ";
		szCamposEtiquetas[1] = "Descripcion    ";
		szCamposEtiquetas[2] = "NART           ";
		szCamposEtiquetas[3] = "UM             ";

		iCamposCantidad = 4; // Se Desplegaran 4 Campos: Producto, Descripcion, NART, UM...

		// Se Utilizan los Indices por Campo para controlarlos mas Abajo
		iIdxProd = 0; iIdxDescr = 1; iIdxNART = 2; iIdxUM = 3;
		
		if (iErrorCode == 0){ //El código escaneado existe: buscar datos para mostrar...
			iPrimeraVez = 8; //Solo display, sin campos en inverse video...
			iProdNoExiste = 0;	
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Código escaneado SI existe...\n"));
				jdeFflush(dlg);
			}

			memset((void *) &dsGetItemMasterByShortItem,(int) _J('\0'),sizeof(DSDX4101B));
			MathCopy(&dsGetItemMasterByShortItem.mnShortItemNumber,&mnItemShortIDBuf);
			dsGetItemMasterByShortItem.cReturnPtr = _J('1');
			dsGetItemMasterByShortItem.cSuppressErrorMsg = _J('1');
			idResult = jdeCallObject (_J("GetItemMasterByShortItem"), NULL,lpBhvrCom,lpVoid,
										(LPVOID)&dsGetItemMasterByShortItem,(CALLMAP*)NULL,(int)0,
										(JCHAR*)NULL,(JCHAR*)NULL,(int)0);
			if (idResult == ER_ERROR){
				iErrorCode = 407;
				jdeFprintf(dlg,_J("***Error(%d): GetItemMasterByShortItem:...\n"), iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			} 
			lpdsF4101 = (LPF4101)jdeRemoveDataPtr(hUser,(unsigned long)dsGetItemMasterByShortItem.idF4101LongRowPtr);
			jdeFromUnicode(szCamposContenido[iIdxNART],lpdsF4101->imdraw,21,UTF8);//DRAW: NART...
			if (lpdsF4101 != (LPF4101) NULL){
				jdeFree((LPVOID)lpdsF4101);
				lpdsF4101 = (LPF4101)NULL;
			}

			jdeFromUnicode(szTempBuf,LszItemDescriptionBuf,31,UTF8);
			strcpy(szCamposContenido[iIdxDescr],szTempBuf);//La descripcion...
			jdeFromUnicode(szTempBuf,LszUOMdefaultBuf,3,UTF8);
			strncpy(szCamposContenido[iIdxUM],szTempBuf,2);//La UM Xref...
			strncpy(szCamposContenido[iIdxUM] + 2,"    ",4);
			jdeFromUnicode(szTempBuf,LszUOMstdConv,3,UTF8);
			strcpy(szCamposContenido[iIdxUM] + 7,szTempBuf);//La UM STD...
			strcpy(szPantallaStatusLine,"Verifique la informacion mostrada...");

			//memset(szCamposContenidoTemp,'\0',sizeof(szCamposContenidoTemp));
			//for (i = 0;i <= iCamposCantidad;i++){
			//	strcpy(szCamposContenidoTemp[i],szCamposContenido[i]);//Sacamos una copia para más adelante...
			//}
		}
		else{//El código escaneado no existe..
			iPrimeraVez = 0; //Captura datos...
			iProdNoExiste = 1;
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Código escaneado NO existe. Error (%d)...\n"), iErrorCode);
				jdeFflush(dlg);
			}
			strcpy(szPantallaStatusLine,"Introduzca la informacion solicitada...");
		}

		do{//Mostramos o capturamos con validaciones varias...
			iErrorCode = 0;

			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Mostrando/capturando info codigo barra...\n"),LszTempBuf);	
				jdeFflush(dlg);
			}

			iCamposPosiciones[iIdxProd] = strlen(szCamposContenido[iIdxProd]);
			iCamposPosiciones[iIdxDescr] = strlen(szCamposContenido[iIdxDescr]);
			iCamposPosiciones[iIdxNART] = strlen(szCamposContenido[iIdxNART]);
			iCamposPosiciones[iIdxUM] = strlen(szCamposContenido[iIdxUM]);

			OWDCmp02 (&iPrimeraVez,szPantallaTitulo,(iCamposOffset + 1),iCamposCantidad,iStatusOffset,iInicioEtiquetas, 
				iInicioCampos,iUltimaLinea,iCReqTAB,
				iCamposPosiciones,szCamposEtiquetas,szCamposContenido,iCamposErrores,szPantallaStatusLine,
				iDbgFlg,dlg);
			memset(iCamposErrores,'\0',sizeof(iCamposErrores));
			memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));	

			i = 0;
			while (szCamposContenido[iIdxUM][i]){
				szCamposContenido[iIdxUM][i] = toupper(szCamposContenido[iIdxUM][i]);
				i++;
			}

			jdeToUnicode(LszTempBuf,szCamposContenido[iIdxUM],3,UTF8);
			LszTempBuf[2]=_J('\0');
			// Valida la UM...
			jdeStrcpy(lpValidateUDC->SY,_J("00"));
			jdeStrcpy(lpValidateUDC->RT,_J("UM"));
			jdeStrcpy(lpValidateUDC->KY,LszTempBuf);
			if (jdeValidateUDC(NULL, lpValidateUDC) == FALSE){  //Invalid UDC!!!						
				iErrorCode = 411;
				iCamposErrores[iIdxUM] = 1; // Seteamos el error...
				jdeFprintf(dlg,_J("***Error(%d): Description: jdeValidateUDC (%s)...\n"),iErrorCode,
								LszTempBuf);
				jdeFflush(dlg);
			}else{

			}

		}while(iErrorCode != 0);//Mientras haya error...

		if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Sacando copia de lo recien capturado...\n"),LszTempBuf);	
			jdeFflush(dlg);
		}
		memset(szCamposContenidoTemp,'\0',sizeof(szCamposContenidoTemp));
		for (i = 0;i <= iCamposCantidad;i++){
			strcpy(szCamposContenidoTemp[i],szCamposContenido[i]);//Sacamos una copia para más adelante...
		}

		if (iProdNoExiste == 0){//Si el codigo si  existe pedimos confirmacion...
			if (iDbgFlg > 0){
				jdeFprintf(dlg,_J("***Inicializando pantalla Pregunta al Operador...\n"));
				jdeFflush(dlg);
			}
			memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
			memset(szCamposContenido,'\0',sizeof(szCamposContenido));
			memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
			memset(iCamposErrores,'\0',sizeof(iCamposErrores));
			memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));
			memset(szPantallaTitulo,'\0',sizeof(szPantallaTitulo));

			iPrimeraVez = 0;			

			strcpy (szPantallaTitulo, "Validacion Items Inventario    ");

			szCamposEtiquetas[0] = "Correcto?(S/N)";

			strcpy(szCamposContenido[0],"N");
			iCamposPosiciones[0] = strlen(szCamposContenido[0]);

			iCamposCantidad = 1;

			OWDCmp02 (&iPrimeraVez,szPantallaTitulo,(iCamposOffset + 9),iCamposCantidad,(iStatusOffset -9),iInicioEtiquetas, 
								iInicioCampos,iUltimaLinea,iCReqTAB,
								iCamposPosiciones,szCamposEtiquetas,szCamposContenido,iCamposErrores,szPantallaStatusLine,
								iDbgFlg,dlg);

			szCamposContenido[0][0] = tolower(szCamposContenido[0][0]); szCamposContenido[0][1] = '\0';
			jdeToUnicode(LszUsrEntry,szCamposContenido[0],2,UTF8);

		}

		//***Actualiza Usuario, máquina, fecha
		idResult = jdeCallObject(_J("GetAuditInfo"),NULL,lpBhvrCom,lpVoid,&dsGetAuditInfo,
						(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
		if (idResult == ER_ERROR){
			iErrorCode = 401;
			jdeFprintf(dlg,_J("***Error(%d): GetAuditInfo 2:...\n"), iErrorCode);
			jdeFflush(dlg);
			goto lbFIN;
		}

		if ((LszUsrEntry[0] != LcYesNoBuf) && (iProdNoExiste == 0)){ //El item SI existe, pero algo está mal...
			iPrimeraVez = 1; //Captura datos, sin dibujado de etiquetas...
			iCamposCantidad = 4;
			
			for (i = 0;i <= iCamposCantidad;i++){
				strcpy(szCamposContenido[i],szCamposContenidoTemp[i]);//Restauramos los datos a mostrar...
				iCamposPosiciones[i] = 0;//iCamposPosiciones[i] = strlen(szCamposContenido[i]);
			}
					
			do{//Mostramos o capturamos con validaciones varias...
				iErrorCode = 0;

				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("**Corrigiendo info codigo barra...\n"),LszTempBuf);	
					jdeFflush(dlg);
				}

				OWDCmp02 (&iPrimeraVez, szPantallaTitulo, (iCamposOffset + 1), iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
					iInicioCampos, iUltimaLinea, iCReqTAB,
					iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
					iDbgFlg, dlg);
				memset(iCamposErrores,'\0',sizeof(iCamposErrores));
				memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));	

				i = 0;
				while (szCamposContenido[iIdxUM][i]){
					szCamposContenido[iIdxUM][i] = toupper(szCamposContenido[iIdxUM][i]);
					i++;
				}

				jdeToUnicode(LszTempBuf,szCamposContenido[iIdxUM],3,UTF8);
				LszTempBuf[2]=_J('\0');
				// Valida la UM...
				jdeStrcpy(lpValidateUDC->SY,_J("00"));
				jdeStrcpy(lpValidateUDC->RT,_J("UM"));
				jdeStrcpy(lpValidateUDC->KY,LszTempBuf);
				if (jdeValidateUDC(NULL, lpValidateUDC) == FALSE){  //Invalid UDC!!!						
					iErrorCode = 414;
					iCamposErrores[iIdxUM] = 1; // Seteamos el error...
					jdeFprintf(dlg,_J("***Error(%d): Description: jdeValidateUDC (%s)...\n"),iErrorCode,
									LszTempBuf);
					jdeFflush(dlg);
				}else{

				}

			}while(iErrorCode != 0);//Mientras haya error...

			memset(szCamposContenidoTemp,'\0',sizeof(szCamposContenidoTemp));
			for (i = 0;i <= iCamposCantidad;i++){
				strcpy(szCamposContenidoTemp[i],szCamposContenido[i]);//Sacamos una copia para más adelante...
			}

		}

		if (((LszUsrEntry[0] != LcYesNoBuf) && (iProdNoExiste == 0)) || (iProdNoExiste == 1) //En los casos de arriba...
			|| (iProdNoExiste == 0)){ //Realmente, siempre...
			memset((void *) &dsF55DC01,(int) _J('\0'),sizeof(F55DC01));

			if ((LszUsrEntry[0] != LcYesNoBuf) && (iProdNoExiste == 0)){ //Existe, pero fué corregido...
				MathCopy(&dsF55DC01.viitm,&mnItemShortIDBuf);//Si hay número corto...
				jdeStrcpy(dsF55DC01.viurcd,_J("ED"));//Existe diferente...
			}
			if ((LszUsrEntry[0] == LcYesNoBuf) && (iProdNoExiste == 0)){ //Existe, y NO  fué corregido...
				MathCopy(&dsF55DC01.viitm,&mnItemShortIDBuf);//Si hay número corto...
				jdeStrcpy(dsF55DC01.viurcd,_J("EI"));//Existe igual...
			}
			if (iProdNoExiste == 1){
				ParseNumericString(&dsF55DC01.viitm,_J("0"));//No hay número corto...
				jdeStrcpy(dsF55DC01.viurcd,_J("NE"));//No existe...
			}
			jdeToUnicode(LszTempBuf,szCamposContenidoTemp[iIdxProd],26,UTF8);//El código de barras...
			jdeStrcpy(dsF55DC01.vicitm,LszTempBuf);
			jdeToUnicode(LszTempBuf,szCamposContenidoTemp[iIdxDescr],31,UTF8);//La descripción...
			jdeStrcpy(dsF55DC01.vidsc1,LszTempBuf);
			jdeToUnicode(LszTempBuf,szCamposContenidoTemp[iIdxNART],26,UTF8);//El código NART...
			jdeStrcpy(dsF55DC01.viaitm,LszTempBuf);
			jdeToUnicode(LszTempBuf,szCamposContenidoTemp[iIdxUM],3,UTF8);//La UM...
			jdeStrcpy(dsF55DC01.viuom,LszTempBuf);
			if (strlen(szCamposContenidoTemp[iIdxProd]) == (unsigned)iSCClenBuf) dsF55DC01.viev01 = _J('B'); //Es un "bulto"...
			else dsF55DC01.viev01 = _J('I'); //Es un "item"...
			jdeStrcpy(dsF55DC01.vipid,_J("VC01"));
			jdeStrcpy(dsF55DC01.viuser,dsGetAuditInfo.szUserName);
			jdeStrcpy(dsF55DC01.vijobn,dsGetAuditInfo.szWorkstation_UserId);
			JDEDATECopy(&dsF55DC01.viupmj,&dsGetAuditInfo.jdDate);
			MathCopy(&dsF55DC01.vitday,&dsGetAuditInfo.mnTime);

			MathCopy(&dsF55DC01Key1.viitm,&dsF55DC01.viitm);
			jdeStrcpy(dsF55DC01Key1.vicitm,dsF55DC01.vicitm);

			lpSelect[0].lpValue = &dsF55DC01Key1.viitm;
			lpSelect[1].lpValue = dsF55DC01Key1.vicitm;

			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Construyendo Selection Tabla F55DC01...\n"));
			idResult = JDB_SetSelection(hRequestF55DC01,lpSelect,(short)(2),JDEDB_SET_REPLACE);
			if (idResult == JDEDB_FAILED){
				iErrorCode = 409;
				iCamposErrores[1] = 1; // Seteamos el error...
				jdeFprintf(dlg,_J("***Error(%d): JDB_SetSelection (F55DC01) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}
			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F55DC01...\n"));
			idResult = JDB_SelectKeyed(hRequestF55DC01,(ID) 0,(void *)NULL,(short)0);
			if (idResult == JDEDB_FAILED){
				iErrorCode = 410;
				jdeFprintf(dlg,_J("***Error(%d): JDB_SelectKeyed (F55DC01) failed...\n"),iErrorCode);
				jdeFflush(dlg);
				goto lbFIN;
			}

			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla F55DC01...\n"));		
			idResult = JDB_Fetch(hRequestF55DC01,&dsF55DC01dummy,(int)0);
			if (idResult == JDEDB_FAILED){//NO existe, queremos insertar...
				jdeFprintf(dlg,_J("***JDB_Fetch (EOF F55DC01)...\n"));
				idResult = JDB_InsertTable (hRequestF55DC01,NID_F55DC01,(ID)0,(void *)&dsF55DC01);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 412;
					jdeFprintf(dlg,_J("***Error(%d): JDB_InsertTable (F55DC01) failed...\n"),iErrorCode);
					jdeFflush(dlg);
				goto lbFIN;
				}
			}
			else{//SI existe, queremos actualizar...
				jdeFprintf(dlg,_J("***JDB_UpdateTable (F55DC01)...\n"));
				idResult = JDB_UpdateTable(hRequestF55DC01,NID_F55DC01,(ID)0,ID_F55DC01_PK,(void *)&dsF55DC01Key1,2,(void *)&dsF55DC01);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 413;
					jdeFprintf(dlg,_J("***Error(%d): JDB_UpdateTable (F55DC01) failed...\n"),iErrorCode);
					jdeFflush(dlg);
				goto lbFIN;
				}
			}

		}

	}while (TRUE); //Procesar mientras haya registros en el Cache (eternamente)...	



	//*************************************************************************
	// Clean up the lpBhvrCom, lpVoid and free user and environment 
	//*************************************************************************
lbFIN:
	JDB_CloseTable(hRequestF55DC01);
lbFIN0:
	OWDCmp90 (iDbgFlg, dlg);//Limpiamos para mostrar menu...
	jdeFflush(dlg);

	jdeErrorClearEx(lpBhvrCom,lpVoid); 
	jdeFree(((LPCG_BHVR)lpVoid)->lpErrorEventKey);
	jdeErrorTerminateEx(((LPCG_BHVR)lpVoid)->lpHdr);
	jdeFreeBusinessFunctionParms(lpBhvrCom,lpVoid);

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit OWDCvc01(%d)...\n"), iErrorCode);
	jdeFflush(dlg);

	if (iDbgFlg == 1) jdeFclose(dlg);

	return iErrorCode;
}

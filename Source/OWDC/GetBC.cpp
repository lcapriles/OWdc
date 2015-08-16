// GetBC.cpp 
//
//Creado:		Luis Capriles,		Fecha:	03/12/2003 
//Modificado:	Luis Capriles,		Fecha:	01/10/2008 - Conversion a UniCode
//Modificado:	Luis Capriles,		Fecha:	10/10/2008 - Búsqueda por Packaging ans Shipping
//Modificado:	Luis Capriles,		Fecha:	10/11/2009 - Búsqueda por GetItemMasterBy2ndItem
//Modificado:	Luis Capriles,		Fecha:	11/08/2012 - Manejo de iGetItemMasterBy, ahora ouede ser 0... todos
//Modificado:	Luis Capriles,		Fecha:	11/08/2012 - c3rdItemNoSymbol para manejo 3er Código...
//
//Función GetBC: Función para procesar código de producto (código de barras).  Trata de identificar el código escaneado en el siguiente orden de acuerdo al 
//parámetro iGetItemMasterBy especificado:
//	iGetItemMasterBy == 0 trata por todo
//	iGetItemMasterBy == 1 trata por xRef...
//	iGetItemMasterBy == 2 trata por 2do código de artículo
//	iGetItemMasterBy == 3 Trata por 3er código de artículo
//	iGetItemMasterBy == 4 trata por Packaging and Shipping...
//1)Trata de localizar el código de barras como XRef.  Si lo consigue devuelve valores.
//2)Trata de localizar el código por 2do Código.  Si lo consigue devuelve valores.
//3)Trata de localizar el código por 3er Código.  Si lo consigue devuelve valores.
//4)Trata de localizar el código por EditRetrieveDeletePackaging:
//4.1)Si length(Codigo Barra) == 13: Lee F4101 y devuelve valores
//4.2)Si length(Codigo Barra) == 14: Convierte SCC en UPC, Lee F4101
//4.2.1)Con EditRetrieveDeletePackaging determina la UM relacionada y devuelve valores
//    
#include <stdio.h>

#include "jde.h"

#include "x4101.h"		// GetItemMasterByShortItem, GetItemMasterBy3rdItem, GetItemMasterBy2ndItem
#include "b4100600.h"	// VerifyAndGetItemXRef
#include "n4101060.h"	// EditRetrieveDeletePackaging
#include "B4201580.h"	// CalculateandValidateCheckDigit 

__declspec(dllexport)  int OWDCgbc(HENV hEnv,HUSER hUser,JCHAR szUsrEntry[128],JCHAR szLineItemBuf[26],MATH_NUMERIC * mnItemShortIDBuf,
					  JCHAR szUOMdefaultBuf[3],JCHAR szUOMstdConv[3],JCHAR szCrossRefTypeCodeBuf[3],int iUPClenBuf,int iSCClenBuf,
					  JCHAR szItemDescription[31],int iGetItemMasterBy, JCHAR c3rdItemNoSymbol, int iDbgFlg,FILE * dlg)
{
	LPCG_BHVR			lpVoid		= NULL;
	LPBHVRCOM			lpBhvrCom	= NULL;
	ID					idResult;
	ERROR_EVENT_KEY		EventKeyLocal;

	DSDX4101B			dsGetItemMasterByShortItem;
	DSDX4101C			dsGetItemMasterBy2ndItem;
	DSDX4101D			dsGetItemMasterBy3rdItem;
	DSD4100600			dsVerifyAndGetItemXRef;
	DSD4101060H			dsEditRetrieveDeletePackaging;
	DSD4201580A			dsCalculateandValidateCheckDigit;


	typedef struct {		
		JCHAR			imupcn[14];		//UPC Number
		MATH_NUMERIC	imitm;			//IdentifierShortItem
		JCHAR           imlitm[26];		//Identifier2nd
		JCHAR           imaitm[26];		//Identifier3rd
		JCHAR			imuom1[3];		//PrimaryUOM
		JCHAR			imumup[3];		//UM UPC 
		JCHAR			imtfla[3];		//UM Std Conv
		JCHAR           imdsc1[31];		//Item Description
} stF4101UPC;

	stF4101UPC			dsF4101UPC, dsF4101UPCTemp;
	HREQUEST			hRequestF4101 = (HREQUEST)NULL;
	NID					szF4101UPCColumnsArray[8] = {NID_UPCN, NID_ITM, NID_LITM, NID_AITM, NID_UOM1, NID_UMUP, NID_TFLA, NID_DSC1 };
	KEY8_F4101			dsF4101Key8;
	SELECTSTRUCT		lpSelect[4];	

	LPF4104				lpdsF4104 = (LPF4104) NULL;
	LPF4101				lpdsF4101 = (LPF4101) NULL;

	JCHAR szTempBuf[128], LszString01[64];
	int iErrorCode, iTipoClaveUPC, iCodigoEncontrado;

	//*************************************************************************
	// Set up the lpBhvrCom amd lpVoid objects                              ***
	//*************************************************************************
	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Inicio GetBC...\n"));
		jdeFprintf(dlg,_J("***Seteo de lpBhvrCom, lpVoid y estructuras...\n"));
		jdeFflush(dlg);
	}

	jdeCreateBusinessFunctionParms(hUser,&lpBhvrCom,(LPVOID*) &lpVoid);
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


	memset((void *) &dsGetItemMasterBy2ndItem,(int) _J('\0'),sizeof(DSDX4101C));
	memset((void *) &dsGetItemMasterBy3rdItem,(int) _J('\0'),sizeof(DSDX4101D));
	memset((void *) &dsVerifyAndGetItemXRef,(int) _J('\0'),sizeof(DSD4100600));
	memset((void *) &lpdsF4104,(int)(_J('\0')),sizeof(lpdsF4104));
	memset((void *) &dsGetItemMasterByShortItem,(int) _J('\0'),sizeof(DSDX4101B));
	memset((void *) &dsEditRetrieveDeletePackaging,(int)(_J('\0')),sizeof(DSD4101060H));
	memset((void *) &dsCalculateandValidateCheckDigit,(int)(_J('\0')),sizeof(DSD4201580A));
	memset((void *) &lpdsF4101,(int)(_J('\0')),sizeof(lpdsF4101));
	memset((void *) &dsF4101UPC,_J('\0'),sizeof(stF4101UPC));
	memset((void *) &dsF4101UPCTemp,_J('\0'),sizeof(stF4101UPC));

	//*************************************************************************
	// Proceso Código Producto                                              ***
	//*************************************************************************
	
	iErrorCode = 0;
	iCodigoEncontrado = 0;
	jdeStrcpy(szUOMdefaultBuf,_J(""));
	ZeroMathNumeric(mnItemShortIDBuf);
	szUsrEntry[25] = '\0';

	if (((iGetItemMasterBy == 0) || (iGetItemMasterBy == 1))  && (iCodigoEncontrado == 0)
		&& (szCrossRefTypeCodeBuf[0] != _J(' ')) && (szCrossRefTypeCodeBuf[0] != _J('\0')) ) {//X-Ref habilitado...
		//Intenta buscar el producto en X-Ref...

		memset((void *) &dsEditRetrieveDeletePackaging,(int)(_J('\0')),sizeof(DSD4101060H));
		memset((void *) &dsCalculateandValidateCheckDigit,(int)(_J('\0')),sizeof(DSD4201580A));

		if (iDbgFlg > 0) {
			jdeFprintf(dlg,_J("***VerifyAndGetItemXRef (%ls)...\n"), szUsrEntry);
			jdeFflush(dlg);
		}
		dsVerifyAndGetItemXRef.cReturnPointerFlag = _J('1');//Si queremos data...
		dsVerifyAndGetItemXRef.cCallType = _J('1');          
		dsVerifyAndGetItemXRef.cSuppressErrorMessages = _J('1');      
		jdeStrcpy(dsVerifyAndGetItemXRef.szIndex,_J("4"));
		jdeStrcpy(dsVerifyAndGetItemXRef.szKeys,_J("3"));
		jdeStrcpy(dsVerifyAndGetItemXRef.szCustomerItemNumber,szUsrEntry);
		jdeStrcpy(dsVerifyAndGetItemXRef.szCrossRefTypeCode,szCrossRefTypeCodeBuf);
		ParseNumericString(&dsVerifyAndGetItemXRef.mnAddressNumber,_J("0"));
		idResult = jdeCallObject (_J("VerifyAndGetItemXRef"),NULL,lpBhvrCom,lpVoid,
									(LPVOID)&dsVerifyAndGetItemXRef,(CALLMAP*)NULL, (int)0,
									(JCHAR*)NULL,(JCHAR*)NULL,(int)0);
		if (idResult == ER_SUCCESS){//No error -> Lo encontró en X-Ref...
			//***
			//Extrae 2do codigo de X-Ref y UM para devolverlo en la función...
			lpdsF4104 = (LPF4104)jdeRemoveDataPtr(hUser,(unsigned long)dsVerifyAndGetItemXRef.idF4104Pointer); 
			jdeStrcpy(szLineItemBuf,lpdsF4104->ivlitm);//Devolvemos 2do código...
			jdeStrcpy(szUOMdefaultBuf,lpdsF4104->ivurcd);//UM para convertir de UCCp a 3er código...
			MathCopy(mnItemShortIDBuf,&dsVerifyAndGetItemXRef.mnShortItemNumber);
			jdeStrcpy(szItemDescription,dsVerifyAndGetItemXRef.szDescription1);
			
			//Intenta buscar el producto por código corto (X-Ref -> código corto)...
			if (iDbgFlg > 0) {
				jdeToUnicode(LszString01,dsVerifyAndGetItemXRef.mnShortItemNumber.String,DIM(LszString01),UTF8);
				jdeFprintf(dlg,_J("***GetItemMasterByShortItem...(%ls)\n"),
									LszString01);
				jdeFflush(dlg);
			}
			MathCopy(&dsGetItemMasterByShortItem.mnShortItemNumber,&dsVerifyAndGetItemXRef.mnShortItemNumber);
			dsGetItemMasterByShortItem.cReturnPtr = _J('0');
			dsGetItemMasterByShortItem.cSuppressErrorMsg = _J('1');
			idResult = jdeCallObject (_J("GetItemMasterByShortItem"), NULL,lpBhvrCom,lpVoid,
										(LPVOID)&dsGetItemMasterByShortItem,(CALLMAP*)NULL,(int)0,
										(JCHAR*)NULL,(JCHAR*)NULL,(int)0);
			if (idResult == ER_SUCCESS){//No error -> Lo encontró por código corto...
				if (iDbgFlg > 0) {
					jdeFprintf(dlg,_J("***Long.szUsrEntry/iUPClenBuf/iSCClenBuf (%d/%d/%d)...\n"),
											jdeStrlen(szUsrEntry),iUPClenBuf,iSCClenBuf);
					jdeFflush(dlg);
				}

				jdeStrcpy(szUOMstdConv,dsGetItemMasterByShortItem.szStandardUOMConversion);//La UM para devolverla a la función...
				
				iCodigoEncontrado = 1; //Tenemos código + UMs por Xref

			} //(dsGetItemMasterByShortItem.cErrorCode == '0' )
			else{
				//Error interno!!! X-Ref -> código corto que no existe...
				iErrorCode = 901;
				jdeToUnicode(LszString01,dsGetItemMasterByShortItem.mnShortItemNumber.String,DIM(LszString01),UTF8);
				jdeFprintf(dlg,_J("***Error(%d): VerifyAndGetItemXRef->GetItemMasterByShortItem (%ls/%ls)...\n"),
						iErrorCode,szLineItemBuf,LszString01);
				jdeFflush(dlg);
				goto lbFIN;
			}
		} 
	} //((iGetItemMasterBy == 0) || (iGetItemMasterBy == 1)) 


//Modificado:	Luis Capriles,		Fecha:	10/11/2009 - Búsqueda por GetItemMasterBy2ndItem
//10/11/2009 - Inicio Modificacion Luis Capriles
	if (((iGetItemMasterBy == 0) || (iGetItemMasterBy == 2)) && iCodigoEncontrado == 0) {
		//Intenta buscar el producto por 2do código...
		if (iDbgFlg > 0) {
			jdeFprintf(dlg,_J("***GetItemMasterBy2ndItem (%ls)...\n"),szUsrEntry);
			jdeFflush(dlg);
		}
		jdeStrcpy(dsGetItemMasterBy2ndItem.sz2ndItemNumber,szUsrEntry);
		dsGetItemMasterBy2ndItem.cReturnPtr = _J('1');
		idResult = jdeCallObject(_J("GetItemMasterBy2ndItem"),NULL,lpBhvrCom,lpVoid,
									&dsGetItemMasterBy2ndItem,(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,
									(JCHAR *)NULL,(int)0);

		if (dsGetItemMasterBy2ndItem.cErrorCode == _J('0')) {
			jdeStrcpy(szLineItemBuf,dsGetItemMasterBy2ndItem.sz2ndItemNumber);
			//Extrae UM para devolverlo en la función desde 2do...
			lpdsF4101 = (LPF4101)jdeRemoveDataPtr(hUser,(unsigned long)dsGetItemMasterBy2ndItem.idF4101LongRowPtr);
			jdeStrcpy(szUOMdefaultBuf,lpdsF4101->imuom1);//UOM Primaria...
			jdeStrcpy(szUOMstdConv,dsGetItemMasterBy2ndItem.szStandardUOMConversion);
			MathCopy(mnItemShortIDBuf,&dsGetItemMasterBy2ndItem.mnShortItemNumber);
			jdeStrcpy(szItemDescription,dsGetItemMasterBy2ndItem.szDescription1);
			if (iDbgFlg > 0) {
				jdeToUnicode(LszString01,mnItemShortIDBuf->String,DIM(LszString01),UTF8);
				jdeFprintf(dlg,_J("***dsGetItemMasterBy2ndItem UM/UMconv/sID(%ls/%ls/%ls)...\n"),szUOMdefaultBuf,szUOMstdConv,LszString01);
				jdeFflush(dlg);
			}

			iCodigoEncontrado = 1; // Tenemos código + UM por 2do...

		}
	}
	
	if (((iGetItemMasterBy == 0) || (iGetItemMasterBy == 3)) && iCodigoEncontrado == 0) {
		//Intenta buscar el producto por 3er código...
		if (iDbgFlg > 0) {
			jdeFprintf(dlg,_J("***GetItemMasterBy3rdItem (%ls)...\n"),szUsrEntry);
			jdeFflush(dlg);
		}
		jdeStrcpy(dsGetItemMasterBy3rdItem.sz3rdItemNumber,szUsrEntry);
		dsGetItemMasterBy3rdItem.cReturnPtr = _J('1');
		idResult = jdeCallObject(_J("GetItemMasterBy3rdItem"),NULL,lpBhvrCom,lpVoid,
									&dsGetItemMasterBy3rdItem,(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,
									(JCHAR *)NULL,(int)0);

		if (dsGetItemMasterBy3rdItem.cErrorCode == _J('0')) {
			jdeStrcpy(szLineItemBuf,dsGetItemMasterBy3rdItem.sz3rdItemNumber);
			//Extrae UM para devolverlo en la función desde 3er...
			lpdsF4101 = (LPF4101)jdeRemoveDataPtr(hUser,(unsigned long)dsGetItemMasterBy3rdItem.idF4101LongRowPtr);
			jdeStrcpy(szUOMdefaultBuf,lpdsF4101->imuom1);//UOM Primaria...
			jdeStrcpy(szUOMstdConv,dsGetItemMasterBy3rdItem.szStandardUOMConversion);
			MathCopy(mnItemShortIDBuf,&dsGetItemMasterBy3rdItem.mnShortItemNumber);
			jdeStrcpy(szItemDescription,dsGetItemMasterBy3rdItem.szDescription1);
			if (iDbgFlg > 0) {
				jdeToUnicode(LszString01,mnItemShortIDBuf->String,DIM(LszString01),UTF8);
				jdeFprintf(dlg,_J("***dsGetItemMasterBy3rdItem UM/UMconv/sID(%ls/%ls/%ls)...\n"),szUOMdefaultBuf,szUOMstdConv,LszString01);
				jdeFflush(dlg);
			}

			LszString01[0] = c3rdItemNoSymbol;
			jdeStrcpy(LszString01 + 1,szLineItemBuf);
			jdeStrcpy(szLineItemBuf,LszString01);

			if (iDbgFlg > 0)jdeFprintf(dlg,_J("***szLineItemBuf 3 (%ls)...\n"),szLineItemBuf);
			jdeFflush(dlg);

			iCodigoEncontrado = 1; // Tenemos código + UM por 3ro...

		}
	}
	
	if (((iGetItemMasterBy == 0) || (iGetItemMasterBy == 4)) && iCodigoEncontrado == 0) {
//Modificado:	Luis Capriles,		Fecha:	10/10/2008 - Búsqueda por Packaging ans Shipping
//10/10/2008 - Inicio Modificacion Luis Capriles

		//Intenta buscar Packaging and Shipping...
		if (iDbgFlg > 0) {
			jdeFprintf(dlg,_J("***Intenta buscar Packaging and Shipping... (%ls)...\n"),szUsrEntry);
			jdeFflush(dlg);
		}

		idResult = JDB_OpenTable(hUser,NID_F4101,ID_F4101_UPC_NUMBER,szF4101UPCColumnsArray,(ushort)(8),
								(JCHAR *)NULL,&hRequestF4101);
		if (idResult == JDEDB_FAILED){
			iErrorCode = 902;
			jdeFprintf (dlg,_J("***Error(%d): JDB_OpenTable(F4101) failed...\n"),iErrorCode);
			jdeFflush(dlg);
			goto lbFIN;
		}
		
		//Construye Where del select...
		JDB_ClearSelection(hRequestF4101);
		jdeNIDcpy(lpSelect[0].Item1.szDict, NID_UPCN);//Codigo UPC == szUsrEntry
		jdeNIDcpy(lpSelect[0].Item1.szTable, NID_F4101);
		lpSelect[0].Item1.idInstance = (ID)0;
		jdeNIDcpy(lpSelect[0].Item2.szDict, _J(""));
		jdeNIDcpy(lpSelect[0].Item2.szTable, _J(""));
		lpSelect[0].Item2.idInstance = (ID)0;
//		lpSelect[0].lpValue = dsF4101Key8.imupcn;
		lpSelect[0].nValues = (short)1;
		lpSelect[0].nAndOr = JDEDB_ANDOR_AND;
		lpSelect[0].nCmp = JDEDB_CMP_EQ;

		// Calculamos la clave UPC dependiendo de su longitud...
		if(jdeStrlen(szUsrEntry) <= (unsigned)iUPClenBuf){ 
			//longitud <= 13: debe ser UPC 
			iTipoClaveUPC = 13;
			jdeStrcpy(szTempBuf,szUsrEntry);
			szTempBuf[14] = _J('\0');
			jdeStrcpy(dsF4101Key8.imupcn, szTempBuf);
			//Completa Where del Select...
			lpSelect[0].lpValue = dsF4101Key8.imupcn;
		}
		else{//(strlen(szUsrEntry) <= iUPClenBuf): No es un UPC
			if(jdeStrlen(szUsrEntry) == (unsigned)iSCClenBuf){
				//logitud = 14: debe ser SCC
				iTipoClaveUPC = 14;
				//Queremos obtener el UPC a partir del SCC
				jdeStrcpy(szTempBuf,szUsrEntry);
				szTempBuf[13] = _J('\0');
				jdeStrcpy(dsCalculateandValidateCheckDigit.szUCCCode,szTempBuf + 1);
				dsCalculateandValidateCheckDigit.cSuppressErrorMessage = _J('1');
				idResult = jdeCallObject( _J("CalculateandValidateCheckDigit"),(LPFNBHVR)NULL,lpBhvrCom,lpVoid ,
											&dsCalculateandValidateCheckDigit,(CALLMAP *) NULL,(int) 0,
											(JCHAR *) NULL,(JCHAR *) NULL,(int) 0);
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***SCC->UPC (%ls, %ls)...\n"),szUsrEntry, 
						dsCalculateandValidateCheckDigit.szUCCCode);
					jdeFflush(dlg);
				}
				if (idResult == ER_ERROR){//SCC malo!!!
					iErrorCode = 903; 			
					jdeFprintf(dlg,_J("***Error(%d): CalculateandValidateCheckDigit->SCC (%ls/%ls/%ls)...\n"),iErrorCode,
										szLineItemBuf,dsCalculateandValidateCheckDigit.szUCCCode,
										szUsrEntry); 
					jdeFflush(dlg);
					goto lbFIN;
				}
				else{// Si es SCC, procesar UOM...
					jdeStrcpy(dsF4101Key8.imupcn, dsCalculateandValidateCheckDigit.szUCCCode + 5);
					//Completa Where del Select...
					lpSelect[0].lpValue = dsF4101Key8.imupcn;
				}
			}
			else{//(strlen(szLineItemBuf) == iSCClenBuf) : logitud invalida (>14)
				iErrorCode = 904;
				jdeFprintf(dlg,_J("***Error(%d): Longitud UPC/SCC invalida (%ls)...\n"),iErrorCode,szLineItemBuf); 
				jdeFflush(dlg);
				goto lbFIN;
			}
		}

		//Teniendo la clave UPC, ahora buscamos en F4101...
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Construyendo Selection Tabla F4101..\n"));	
		idResult = JDB_SetSelection(hRequestF4101,lpSelect,(short)1,JDEDB_SET_REPLACE);
		if (idResult == JDEDB_FAILED){
			iErrorCode = 905;
			jdeFprintf (dlg,_J("***Error(%d): JDB_SetSelection(F4101) failed...\n"),iErrorCode);
			jdeFflush(dlg);
			goto lbFIN;
		}				
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F4101...\n"));
		idResult = JDB_SelectKeyed(hRequestF4101,(ID) 0,(void *)NULL,(short)0);
		if (idResult == JDEDB_FAILED){
			iErrorCode = 906;
			jdeFprintf (dlg,_J("***Error(%d): JDB_SelectKeyed(F4101) failed...\n"),iErrorCode);
			jdeFflush(dlg);
			goto lbFIN;
		}
		//Se verifica existencia y unicidad del UPC en F4101...
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Fetch sobre Tabla F4101...\n"));		
		idResult = JDB_Fetch(hRequestF4101,&dsF4101UPC,(int)0);
		if (idResult != JDEDB_FAILED){
			//Si existe el UPC en F4101...
			//Si es de 13, nos quedamos con la UM...
			//Si no es de 13, hay que buscar la UM en base al primer dígito del SCC...

			MathCopy(mnItemShortIDBuf,&dsF4101UPC.imitm);
			jdeStrcpy(szUOMstdConv,dsF4101UPC.imtfla);
			jdeStrcpy(szLineItemBuf,dsF4101UPC.imlitm);//2do código...

			if (iTipoClaveUPC == 13){
				//Extrae UM para devolverlo en la función...
				jdeStrcpy(szUOMdefaultBuf,dsF4101UPC.imumup);
				jdeStrcpy(szItemDescription,dsF4101UPC.imdsc1);

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szUPC UM/Desc(%ls/%ls)...\n"),szUOMdefaultBuf,szItemDescription);
				if ((jdeStrcmp(szUOMdefaultBuf,_J("")) == 0) || 
					(jdeStrcmp(szUOMdefaultBuf,_J("  "))== 0)){//Error, no se ha podido determinar una UM!!!!... 
					//Hay que intentar XRef...								
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***No se encontro szUPC UM...\n"));
					jdeFflush(dlg);
				}
				else {

					iCodigoEncontrado = 1; // Tenemos código + UM por 2do...

				}
			}
			if (iTipoClaveUPC == 14){
				//Calcula UM para devolverlo en la función...
				if (iDbgFlg > 0) {
					jdeToUnicode(LszString01,dsF4101UPC.imitm.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***EditRetrieveDeletePackaging short/UCCp (%ls/%ls)...\n"),
										LszString01,szUsrEntry);
					jdeFflush(dlg);
				}
				jdeStrcpy((JCHAR *)(dsEditRetrieveDeletePackaging.szBranchPlant),(const JCHAR *)_J(" "));
				MathCopy (&dsEditRetrieveDeletePackaging.mnIdentifierShortItem,	&dsF4101UPC.imitm);
				dsEditRetrieveDeletePackaging.cSuppressErrorMessage = _J('1') ;   
				jdeStrcpy((JCHAR *)(dsEditRetrieveDeletePackaging.szTransactionAction),(const JCHAR *)(_J("I")));
				idResult = jdeCallObject(_J("EditRetrieveDeletePackaging"),NULL,lpBhvrCom,lpVoid, 
											&dsEditRetrieveDeletePackaging,(CALLMAP *) NULL,(int) 0,(JCHAR *) NULL,
											(JCHAR *) NULL,(int) 0);
				if (idResult == ER_ERROR){
					iErrorCode = 907;			
					jdeFprintf(dlg,_J("***Error(%d): EditRetrieveDeletePackaging (%ls/%ls)...\n"),
							iErrorCode,szLineItemBuf,szUsrEntry); 
					jdeFflush(dlg);
					goto lbFIN;
				}
				//Obtiene la UM del SCC como un offset del primer dígito del SCC
				jdeStrcpy(szUOMdefaultBuf,dsEditRetrieveDeletePackaging.szUnitofMeasureSCCPI1 + 
						(((szUsrEntry[0] - (int)_J('0'))-1) * 3));
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szSCC UM[%lc] (%ls)...\n"),szUsrEntry[0],szUOMdefaultBuf);
				if ((jdeStrcmp(szUOMdefaultBuf,_J("")) == 0) || 
					(jdeStrcmp(szUOMdefaultBuf,_J("  "))== 0)){//Error, no se ha podido determinar una UM!!!!... 								
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***No se encontro szSCC UM...\n"));
					jdeFflush(dlg);
				}
				else {

					iCodigoEncontrado = 1; // Tenemos código + UM por 2do...

				}

				MathCopy(&dsGetItemMasterByShortItem.mnShortItemNumber,&dsEditRetrieveDeletePackaging.mnIdentifierShortItem);
				dsGetItemMasterByShortItem.cReturnPtr = _J('0');
				dsGetItemMasterByShortItem.cSuppressErrorMsg = _J('1');
				idResult = jdeCallObject (_J("GetItemMasterByShortItem"), NULL,lpBhvrCom,lpVoid,
											(LPVOID)&dsGetItemMasterByShortItem,(CALLMAP*)NULL,(int)0,
											(JCHAR*)NULL,(JCHAR*)NULL,(int)0);
				if (idResult == ER_ERROR){//No error -> Lo encontró por código corto...
					iErrorCode = 908;
					jdeToUnicode(LszString01,dsGetItemMasterByShortItem.mnShortItemNumber.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***Error(%d): GetItemMasterByShortItem (%ls/%ls/%ls)...\n"),
							iErrorCode,szLineItemBuf,szUsrEntry,LszString01); 
					jdeFflush(dlg);
					goto lbFIN;
				}

				jdeStrcpy(szItemDescription,dsGetItemMasterByShortItem.szDescription1);
			}

			//Verificando Unicidad...
			idResult = JDB_Fetch(hRequestF4101,&dsF4101UPCTemp,(int)0);
			if (idResult != JDEDB_FAILED){
				//UPC repetido en F4101: Error!!!...
				iErrorCode = 909;
				jdeFprintf (dlg,_J("***Error(%d): JDB_Fetch(F4101) duplicado(%ls)...\n"),iErrorCode, 
									dsF4101Key8.imupcn);
				jdeFflush(dlg);
				goto lbFIN;
			}

			iCodigoEncontrado = 1; // Tenemos código + UM por 3ro...

		}
	} //((iGetItemMasterBy == 0) || (iGetItemMasterBy == 4))...

	if (iCodigoEncontrado == 0){
		//No encontramos nada...
		iErrorCode = 909;
		jdeFprintf(dlg,_J("***Warn(%d): No pudo ser localizado item (%ls)...\n"),iErrorCode, szUsrEntry);
		jdeFflush(dlg);
	}


lbFIN:
	//   Clean up.
	if (lpdsF4104 != (LPF4104) NULL){
		jdeFree((LPVOID)(lpdsF4104));
		lpdsF4104 = (LPF4104) NULL;
	}
	if (lpdsF4101 != (LPF4101) NULL){
		jdeFree((LPVOID)lpdsF4101);
		lpdsF4101 = (LPF4101)NULL;
	}
	if (hRequestF4101 != (HREQUEST) NULL){
		JDB_CloseTable(hRequestF4101);
		hRequestF4101 = (HREQUEST) NULL;
	}

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit OWDCgbc(%d)...\n"),iErrorCode);
	jdeFflush(dlg);

	return iErrorCode;
}
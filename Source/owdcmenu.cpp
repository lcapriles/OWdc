// owdcmenu.cpp 
//
//Creado:		Luis Capriles,		Fecha:	16/12/2003
//Modificado:	Luis Capriles,		Fecha:	01/10/2008 - Conversion a UniCode
  

#include "stdafx.h" 
#include <stdio.h>

#include <jde.h> 

int OWDCrc01(HENV hEnv,HUSER hUser); //Captura Recepcion Ordenes de Compras  
int OWDCrr01(HENV hEnv,HUSER hUser); //Captura Ruteo Ordenes de Compras 
int OWDCce01(HENV hEnv,HUSER hUser); //Captura Confirmacion Embarques    
int OWDCve01(HENV hEnv,HUSER hUser); //Captura Verificación Embarques 
int OWDCvc01(HENV hEnv,HUSER hUser); //Validacion Items Inventario 
int OWDCtc01(HENV hEnv,HUSER hUser, JCHAR LszUsrID[16],char cIniFlag); //Conteo y Tomas de Inventario

void OWDCmp01 (int * primeraVez, int * opcionEscogida, char * pantallaTitulo, int opcionesCantidad, int inicioEtiquetas, 
			   char * opcionesEtiqueta[], char * pantallaStatusLine, int iDbgFlg, FILE * dlg);//Manejo de la pantalla de menu...

void OWDCmp02 (int * primeraVez, char * pantallaTitulo, int camposOffset, int camposCantidad, 
			   int statusOffset, int inicioEtiquetas, int inicioCampos, int ultimaLinea, int CReqTAB,
			   int camposPosiciones[], char * camposEtiquetas[], char camposContenido[][128], int camposErrores[],
			   char * pantallaStatusLine, int iDbgFlg, FILE * dlg); //Manejo de la pantalla de entrada datos...

void OWDCmp90 (int iDbgFlg, FILE * dlg); //Termina Manejo de la pantalla con curses... 
        

int _cdecl main(int argc, char **argv)
{

	HENV	hEnv		= NULL; 
	HUSER	hUser		= NULL;    

	FILE *	ini;
	FILE *	mnu; 
	FILE *	dlg;   
 
	JCHAR LszLinea[80],LszLin1[64],LszLin2[64],LszUsrEntry[128],
		 LszUsrEnv[16],LszUsrID[16],LszUsrPass[16],LszUsrLeng[2],  
		 LszTempBuf[32],
		 
		 cStandAloneFlg;

	int	 iErrorCode,iDbgFlg,iOpcionSalir,i,
		 iPrimeraVez,iCamposOffset,iCamposCantidad,iStatusOffset,iInicioEtiquetas,iInicioCampos,iUltimaLinea,iCReqTAB, 
		 iCamposPosiciones[64],iCamposErrores[64],iOpcionEscogida;
	char * szCamposEtiquetas[64],szCamposContenido[64][128],szPantallaTitulo[64],szPantallaStatusLine[64],szErrMsg[30]; 
	 
	char menu1, menu2[16];

#define INIwidth 80 


	//*************************************************************************
	//***Procesamiento Archivo INI                                          ***
	//*************************************************************************


	//Contruye nombre de archivo como nombre_Daammdd_Thhmmss.log
	char timebuf[9],datebuf[9];
	JCHAR Ltimebuf[9],Ldatebuf[9],szFileNameBuf[64];

	_strtime(timebuf);
	_strdate(datebuf);
	jdeToUnicode(Ltimebuf,timebuf,DIM(Ltimebuf),UTF8);
	jdeToUnicode(Ldatebuf,datebuf,DIM(Ldatebuf),UTF8);

	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCmenu_D"));
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

	menu1 = '\0';
	if (argc > 1) {
		menu1 = argv[1][0];
		memset(menu2,'\0',sizeof(menu2));
		sprintf(menu2,"%s%c%s\0","OWDCmenu",menu1,".ini");
		jdeToUnicode(LszTempBuf,menu2,16,UTF8);
	}else jdeToUnicode(LszTempBuf,"OWDCmenu.ini",16,UTF8);

	ini = jdeFopen(LszTempBuf,_J("r"));
	if (!ini){
		iErrorCode =  101;
		jdeFprintf(dlg,_J("***Error(%d) abriendo INI (%ls)...\n"),iErrorCode,LszTempBuf);
        goto lbFIN1;
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

		if(jdeStrcmp(LszLin1,_J("Debug")) == 0){
			iDbgFlg = jdeAtoi(LszLin2);
			if (iDbgFlg == 1){

			}
			else dlg = stderr;
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Debug (%d)...\n"), iDbgFlg);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("UserID")) == 0){
			jdeStrcpy(LszUsrID, LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LszUsrID (%ls)...\n"),LszUsrID);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("UserPWD")) == 0){
			jdeStrcpy(LszUsrPass, LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LszUsrPass (%ls)...\n"),LszUsrPass);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("UserENV")) == 0){
			jdeStrcpy(LszUsrEnv, LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LszUsrEnv (%ls)...\n"),LszUsrEnv);
			continue;
		}
		if(jdeStrcmp(LszLin1,_J("Leng")) == 0){
			jdeStrcpy(LszUsrLeng, LszLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Leng (%ls)...\n"),LszUsrLeng);
			continue;
		}		
		if(jdeStrcmp(LszLin1,_J("StandAlone")) == 0){
			cStandAloneFlg = LszLin2[0];
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: StandAlone (%lc)...\n"),cStandAloneFlg); 
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
	}
	jdeFclose(ini);
	jdeFflush(dlg);            

	//*************************************************************************
	//***Fin Procesamiento INI                                              ***
	//*************************************************************************

	iErrorCode = 0; //No hay Errores!!!
	memset(szCamposContenido,'\0',sizeof(szCamposContenido));
	memset(szErrMsg,' ',sizeof(szErrMsg)); szErrMsg[29] = '\0';


	if (cStandAloneFlg == _J('1')) {//Login caracter a OW, no compartido...

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));

		iPrimeraVez = 0;

		strcpy (szPantallaTitulo, "Menu Captura Datos Inventario");
		strcpy (szPantallaStatusLine, "");

		szCamposEtiquetas[0] = "Ambiente...";
		szCamposEtiquetas[1] = "Usuario....";
		szCamposEtiquetas[2] = "Password...";

		jdeFromUnicode(szCamposContenido[0],LszUsrEnv,127,UTF8);	
		iCamposPosiciones[0] = strlen(szCamposContenido[0]);
		jdeFromUnicode(szCamposContenido[1],LszUsrID,127,UTF8);
		iCamposPosiciones[1] = strlen(szCamposContenido[1]);
		jdeFromUnicode(szCamposContenido[2],LszUsrPass,127,UTF8);
		iCamposPosiciones[2] = strlen(szCamposContenido[2]);

		iCamposCantidad = 3;//tres campos a desplegar...

		OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
					iInicioCampos, iUltimaLinea, iCReqTAB,
					iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
					iDbgFlg, dlg);

		szCamposContenido[0][15] = '\0';
		jdeToUnicode(LszUsrEnv,szCamposContenido[0],16,UTF8);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszUsrEnv(%ls)...\n"), LszUsrEnv);

		szCamposContenido[1][15] = '\0';
		jdeToUnicode(LszUsrID,szCamposContenido[1],16,UTF8);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszUsrID(%ls)...\n"), LszUsrID);
		
		szCamposContenido[2][15] = '\0';
		jdeToUnicode(LszUsrPass,szCamposContenido[2],16,UTF8);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***LszUsrPass(%ls)...\n"), LszUsrPass);

		//Initialize Environment Handle
		if(JDB_InitEnvOvrExtended(&hEnv,LszUsrEnv,LszUsrID,LszUsrPass, _J("*ALL"))!=JDEDB_PASSED){
			iErrorCode =  102;
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitEnvOvrExtended failed...\n"),iErrorCode);
			goto lbFIN2;
		}
		//Initialize User Handle
		if(JDB_InitUser(hEnv, &hUser, NULL,JDEDB_COMMIT_AUTO)!=JDEDB_PASSED){
			iErrorCode =  103;		
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitUser failed...\n"),iErrorCode);
			goto lbFIN3;
		}

		//Salir de curses...

		OWDCmp90 (iDbgFlg, dlg);
	}
	else{//Login grafico a OW, compartido...
		//if(JDB_InitEnv(&hEnv)!=JDEDB_PASSED){}
		jdeFromUnicode(szCamposContenido[0],LszUsrEnv,16,UTF8);
		jdeFromUnicode(szCamposContenido[1],LszUsrID,16,UTF8);
		if(JDB_InitEnvOvrExtended(&hEnv,LszUsrEnv,LszUsrID,LszUsrPass, _J("*ALL"))!=JDEDB_PASSED){
			iErrorCode =  102;
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitEnv failed...\n"),iErrorCode);
			goto lbFIN2;
		}
		//Initialize User Handle
		if(JDB_InitUser(hEnv, &hUser, NULL,JDEDB_COMMIT_AUTO)!=JDEDB_PASSED){
			iErrorCode =  103;		
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitUser failed...\n"),iErrorCode);
			goto lbFIN3;
		}
	}

	jdeFflush(dlg);  


	//*************************************************************************
	//***Procesamiento Archivo MNU                                          ***
	//*************************************************************************
	do{ //while (iOpcionSalir == 2); Loop para reprocesar el menú 

		if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Dibujando Ménu\n"));
			jdeFflush(dlg);
		}

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szPantallaStatusLine,'\0',sizeof(szPantallaStatusLine));

		iPrimeraVez = 0;
		iOpcionEscogida = 0;

		strcpy (szPantallaTitulo, "Menu Captura Datos Inventario");
		sprintf(szPantallaStatusLine,"%s%s-%s\0",szErrMsg,szCamposContenido[0],szCamposContenido[1]);

		iCamposCantidad = 0;   

		szCamposEtiquetas[iCamposCantidad++] = "1 - Captura Recepcion Orden de Compra"; 
		szCamposEtiquetas[iCamposCantidad++] = "2 - Captura Ruteo Orden de Compra";
		szCamposEtiquetas[iCamposCantidad++] = "3 - Captura Despachos Almacen";
		szCamposEtiquetas[iCamposCantidad++] = "4 - Verificacion Despachos Almacen";
		szCamposEtiquetas[iCamposCantidad++] = "5 - Validacion Items Inventario";
		szCamposEtiquetas[iCamposCantidad++] = "6 - Conteo Fisico Inventario";
		szCamposEtiquetas[iCamposCantidad++] = "0 - Salir";

		OWDCmp01 (&iPrimeraVez, &iOpcionEscogida, szPantallaTitulo, iCamposCantidad, iInicioEtiquetas, szCamposEtiquetas, 
					szPantallaStatusLine,iDbgFlg,dlg);

		memset(szErrMsg,' ',sizeof(szErrMsg)); szErrMsg[29] = '\0';
		
		iOpcionSalir = 0;  
//		do {//while (iOpcionSalir == 0 );Loop para validar Opción Menú... 
			switch (iOpcionEscogida){
				case 1:
					iPrimeraVez = 9;//Salir de curses
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCrc01(hEnv,hUser);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCrc01 (%d)...\n"),iErrorCode);
					jdeFflush(dlg);
					iErrorCode = 0; 
					break;
				case 2:
					iPrimeraVez = 9;//Salir de curses
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCrr01(hEnv,hUser);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCce01 (%d)...\n"),iErrorCode); 
					jdeFflush(dlg);
					iErrorCode = 0; 
					break;
				case 3:
					iPrimeraVez = 9;//Salir de curses
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCce01(hEnv,hUser);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCce01 (%d)...\n"),iErrorCode);
					jdeFflush(dlg);
					iErrorCode = 0; 
					break;
				case 4:
					iPrimeraVez = 9;//Salir de curses   
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCve01(hEnv,hUser);  
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCve01 (%d)...\n"),iErrorCode);
					jdeFflush(dlg);
					iErrorCode = 0; 
					break;
				case 5:
					iPrimeraVez = 9;//Salir de curses
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCvc01(hEnv,hUser);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCvc01 (%d)...\n"),iErrorCode);
					jdeFflush(dlg);
					iErrorCode = 0; 
					break;
				case 6:
					iPrimeraVez = 9;//Salir de curses
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCtc01(hEnv,hUser,LszUsrID,menu1);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCtc01 (%d)...\n"),iErrorCode);
					jdeFflush(dlg);
					if ((iErrorCode == 509) || (iErrorCode == 510) || (iErrorCode == 515)){
						strncpy(szErrMsg,"No hay Conteos Activos!!!",25);
					}
					iErrorCode = 0; 
					break;

				case 0:
					iOpcionSalir = 1; 
					goto lbFIN;
					break;
				default :
					iOpcionSalir = 2;
			}
//		} while (iOpcionSalir == 0);
	}while (iOpcionSalir == 2);  
	//*************************************************************************
	//***Fin Procesamiento MNU                                              ***
	//*************************************************************************

lbFIN:
	iPrimeraVez = 9;//Salir de curses  
	OWDCmp90 (iDbgFlg,dlg);
	if(hUser)JDB_FreeUser(hUser); 
lbFIN3:
	if(hEnv)JDB_FreeEnv(hEnv);
lbFIN2:
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit main (%d)...\n"), iErrorCode);
	jdeFflush(dlg); 

	if (iDbgFlg == 1) jdeFclose(dlg); 
lbFIN1:
	return iErrorCode;  
}

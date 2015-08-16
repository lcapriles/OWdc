// OWfnc.cpp 
//
//Creado:		Luis Capriles,		Fecha:	11/03/2014 
//
#include <stdio.h>

#include "jde.h"



int ProcesaCB(char * szCodigoBarra, char * szCBprod,char * szCBlote,char * szCBfecha,int iUPClenBuf,int iSCClenBuf,int iDbgFlg, FILE * dlg){
	//Procesa el formato EAN/UCC-128 del código de barras de acuerdo a un formato fijo...
	int iErrorCode1 = 1;
	char szDiasMes[12][16] = {"31", "28", "31", "30", "31", "30", "31", "31", "30", "31", "30", "31"};
	char szFechaDD[3],szFechaMM[3],szFechaAA[3],szFechaAAAA[5],szFechaTemp[16],szdummy[128];
	char * szTempBuf1 = NULL;
	char * szTempBuf2 = NULL;
	JCHAR szDummy[128];


	memset(szFechaDD,'\0',sizeof(szFechaDD));
	memset(szFechaMM,'\0',sizeof(szFechaMM));
	memset(szFechaAA,'\0',sizeof(szFechaAA));
	memset(szFechaAAAA,'\0',sizeof(szFechaAAAA));
	memset(szFechaTemp,'\0',sizeof(szFechaTemp));



	if (iDbgFlg > 0){
		jdeToUnicode(szDummy,szCodigoBarra,127,UTF8);
		jdeFprintf(dlg,_J("***ProcesaCB szCodigoBarra 0(%s)...\n"),szDummy);
		jdeFflush(dlg);
	}

	if (strlen(szCodigoBarra) == (unsigned)iUPClenBuf){
		iErrorCode1 = 0;
		strcpy(szCBprod,szCodigoBarra);
		if (iDbgFlg > 0){
			jdeToUnicode(szDummy,szCBprod,127,UTF8);
			jdeFprintf(dlg,_J("***ProcesaCB szCBprod iUPClenBuf(%s)...\n"),szDummy);
			jdeFflush(dlg);
		}
	}
	else {
		if(strlen(szCodigoBarra) == (unsigned)iSCClenBuf) {
			iErrorCode1 = 0;
			strcpy(szCBprod,szCodigoBarra);
			if (iDbgFlg > 0){
				jdeToUnicode(szDummy,szCBprod,127,UTF8);
				jdeFprintf(dlg,_J("***ProcesaCB szCBprod iSCClenBuf(%s)...\n"),szDummy);
				jdeFflush(dlg);
			}
		}
		else {
			szTempBuf2 = strtok(szCodigoBarra,"()");
			szTempBuf1 = strtok(NULL,")(");						
			while (szTempBuf2 != NULL){
				if (szTempBuf1 != NULL) {
					if (strcmp(szTempBuf2,"01") == 0){//Codigo Producto...
						iErrorCode1 = 0;
						strcpy(szCBprod,szTempBuf1);
						if (iDbgFlg > 0){
							jdeToUnicode(szDummy,szCBprod,127,UTF8);
							jdeFprintf(dlg,_J("***ProcesaCB szCBprod 1(%s)...\n"),szDummy);
							jdeFflush(dlg);
						}
					}
					if (strcmp(szTempBuf2,"10") == 0){//Codigo Lote...
						iErrorCode1 = 0;
						strcpy(szCBlote,szTempBuf1);
						if (iDbgFlg > 0){
							jdeToUnicode(szDummy,szCBlote,127,UTF8);
							jdeFprintf(dlg,_J("***ProcesaCB szCBlote 1(%s)...\n"),szDummy);
							jdeFflush(dlg);
						}
					}
					if (strcmp(szTempBuf2,"17") == 0){//Fecha Vencimiento...
						iErrorCode1 = 0;
						if (((szTempBuf1 + 4)[0] == '0') && ((szTempBuf1 + 5)[0] == '0')){//YYMM00...
							if (iDbgFlg > 0){
								jdeToUnicode(szDummy,szTempBuf1,127,UTF8);
								jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp YYMM00 1(%s)...\n"),szDummy);
								jdeFflush(dlg);
							}
							sscanf(szTempBuf1,"%2c%2c%2c",szFechaAA,szFechaMM,szFechaDD);
							sprintf(szCBfecha,"%s/%s/20%s\0",szDiasMes[atoi(szFechaMM) - 1],szFechaMM,szFechaAA);
						} 
						else if (((szTempBuf1 + 2)[0] >= '1') && ((szTempBuf1 + 3)[0] >= '3')){//YYYYMM... (porque XXYY y YY > 13!!!)
							if (iDbgFlg > 0){
								jdeToUnicode(szDummy,szTempBuf1,127,UTF8);
								jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp YYYYMM 1(%s)...\n"),szDummy);
								jdeFflush(dlg);
							}
							sscanf(szTempBuf1,"%4c%2c",szFechaAAAA,szFechaMM);
							sprintf(szCBfecha,"%s/%s/%s\0",szDiasMes[atoi(szFechaMM) - 1],szFechaMM,szFechaAAAA);
						} 
						else {//YYMMDD... 
							if (iDbgFlg > 0){
								jdeToUnicode(szDummy,szTempBuf1,127,UTF8);
								jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp YYMMDD 1(%s)...\n"),szDummy);
								jdeFflush(dlg);
							}
							sscanf(szTempBuf1,"%2c%2c%2c",szFechaAA,szFechaMM,szFechaDD);
							sprintf(szCBfecha,"%s/%s/20%s\0",szFechaDD,szFechaMM,szFechaAA);
						}
						if (iDbgFlg > 0){
							jdeToUnicode(szDummy,szCBfecha,127,UTF8);
							jdeFprintf(dlg,_J("***ProcesaCB szCBfecha 1(%s)...\n"),szDummy);
							jdeFflush(dlg);
						}
					}
				}
				else{
					iErrorCode1 = 0;
					strcpy(szCBprod,szTempBuf2);
				}
				szTempBuf2 = strtok(NULL,"()");
				szTempBuf1 = strtok(NULL,")(");	
			}
		}
	}
	if (strlen(szCBprod) > (unsigned)iSCClenBuf){//Lo anterior no bastó...  
		if (strlen(szCodigoBarra) >= 16){
			iErrorCode1 = 0;
			strncpy(szCBprod,szCodigoBarra+2,14);//producto...
			szCBprod[14] = '\0';
			if (iDbgFlg > 0){
				jdeToUnicode(szDummy,szCBprod,127,UTF8);
				jdeFprintf(dlg,_J("***ProcesaCB szCBprod 2(%s)...\n"),szDummy);
				jdeFflush(dlg);
			}
		}
		if ((strlen(szCodigoBarra) > 18) && ((szCodigoBarra + 16)[0] == '2') && ((szCodigoBarra + 17)[0] == '0')){//20: ignorar...
			//01CBprod20xx17CBfecha10CBlote
			if ((strlen(szCodigoBarra) > 22) && ((szCodigoBarra + 20)[0] == '1') && ((szCodigoBarra + 21)[0] == '7')){//17: fecha vencimiento...
				strncpy(szFechaTemp,szCodigoBarra+22,6);
				if (iDbgFlg > 0){
					jdeToUnicode(szDummy,szFechaTemp,127,UTF8);
					jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp 2a(%s)...\n"),szDummy);
					jdeFflush(dlg);
				}
			}
			if ((strlen(szCodigoBarra) > 30) && ((szCodigoBarra + 28)[0] == '1') && ((szCodigoBarra + 29)[0] == '0')){//10: lote...
				strncpy(szCBlote,szCodigoBarra+30,25);
				if (iDbgFlg > 0){
					jdeToUnicode(szDummy,szCBlote,127,UTF8);
					jdeFprintf(dlg,_J("***ProcesaCB szCBlote 2a(%s)...\n"),szDummy);
					jdeFflush(dlg);
				}
			}
			//01CBprod20xx10CBlote
			if ((strlen(szCodigoBarra) > 22) && ((szCodigoBarra + 20)[0] == '1') && ((szCodigoBarra + 21)[0] == '0')){//10: lote...
				strncpy(szCBlote,szCodigoBarra+22,25);
				if (iDbgFlg > 0){
					jdeToUnicode(szDummy,szCBlote,127,UTF8);
					jdeFprintf(dlg,_J("***ProcesaCB szCBlote 2b(%s)...\n"),szDummy);
					jdeFflush(dlg);
				}
			}
		}
		if ((strlen(szCodigoBarra) > 18) && ((szCBprod + 16)[0] == '1') && ((szCBprod + 17)[0] == '7')){//17: fecha vencimiento...
			//01CBprod17CBfecha10CBlote
			strncpy(szFechaTemp,szCodigoBarra+18,6);
			if (iDbgFlg > 0){
				jdeToUnicode(szDummy,szFechaTemp,127,UTF8);
				jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp 2b(%s)...\n"),szDummy);
				jdeFflush(dlg);
			}
			if ((strlen(szCodigoBarra) > 26) && ((szCodigoBarra + 24)[0] == '1') && ((szCodigoBarra + 25)[0] == '0')){//10: lote...
				strncpy(szCBlote,szCodigoBarra+26,25);
				if (iDbgFlg > 0){
					jdeToUnicode(szDummy,szCBlote,127,UTF8);
					jdeFprintf(dlg,_J("***ProcesaCB szCBlote 2c(%s)...\n"),szDummy);
					jdeFflush(dlg);
				}
			}
		}
		if ((strlen(szCodigoBarra) > 18) && ((szCBprod + 16)[0] == '1') && ((szCBprod + 17)[0] == '0')){//10: lote...
			//01CBprod10CBlote
			strncpy(szCBlote,szCodigoBarra+18,25);
			if (iDbgFlg > 0){
				jdeToUnicode(szDummy,szCBlote,127,UTF8);
				jdeFprintf(dlg,_J("***ProcesaCB szCBlote 2d(%s)...\n"),szDummy);
				jdeFflush(dlg);
			}
		}

		if (strlen(szFechaTemp) == 6){
			if (((szFechaTemp + 4)[0] == '0') && ((szFechaTemp + 5)[0] == '0')){//YYMM00...
				if (iDbgFlg > 0){
					jdeToUnicode(szDummy,szFechaTemp,127,UTF8);
					jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp YYMM00 2(%s)...\n"),szDummy);
					jdeFflush(dlg);
				}
				sscanf(szFechaTemp,"%2c%2c%2c",szFechaAA,szFechaMM,szFechaDD);
				sprintf(szCBfecha,"%s/%s/20%s\0",szDiasMes[atoi(szFechaMM) - 1],szFechaMM,szFechaAA);
			} 
			else if (((szFechaTemp + 2)[0] >= '1') && ((szFechaTemp + 3)[0] >= '3')){//YYYYMM... (porque XXYY y YY > 13!!!) 
				sscanf(szFechaTemp,"%4c%2c",szFechaAAAA,szFechaMM);
				sprintf(szCBfecha,"%s/%s/%s\0",szDiasMes[atoi(szFechaMM) - 1],szFechaMM,szFechaAAAA);
				if (iDbgFlg > 0){
					jdeToUnicode(szDummy,szFechaTemp,127,UTF8);
					jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp YYYYMM 2(%s)...\n"),szDummy);
					jdeFflush(dlg);
				}
			} 
			else {//YYMMDD... 
				if (iDbgFlg > 0){
					jdeToUnicode(szDummy,szFechaTemp,127,UTF8);
					jdeFprintf(dlg,_J("***ProcesaCB szFechaTemp YYMMDD 2(%s)...\n"),szDummy);
					jdeFflush(dlg);
				}
				sscanf(szFechaTemp,"%2c%2c%2c",szFechaAA,szFechaMM,szFechaDD);
				sprintf(szCBfecha,"%s/%s/20%s\0",szFechaDD,szFechaMM,szFechaAA);
			}
			if (iDbgFlg > 0){
				jdeToUnicode(szDummy,szCBfecha,127,UTF8);
				jdeFprintf(dlg,_J("***ProcesaCB szCBfecha 2(%s)...\n"),szDummy);
				jdeFflush(dlg);
			}
		}else{
			strcpy(szCBfecha,"");
			if (iDbgFlg > 0){
				jdeToUnicode(szDummy,szCBfecha,127,UTF8);
				jdeFprintf(dlg,_J("***ProcesaCB szCBfecha 2 nula(%s)...\n"),szDummy);
				jdeFflush(dlg);
			}
		}
	}

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit ProcesaCB(%d)...\n"),iErrorCode1);
	jdeFflush(dlg);

	return iErrorCode1;
}

int FormateaFecha(char * szFechaI, JCHAR * LszFechaO){
	//Dado un string de la forma ddmmaa devuelve dd/mm/aaaa...
	//Dado un string de la forma dd/mm/aa devuelve dd/mm/aaaa
	char szFechaDD[3],szFechaMM[3],szFechaAA[5],szFechaTemp[11], cDummy[2];
	JCHAR  LszString01[64];
	JDEDATE	jFechaTemp;

	memset(szFechaDD,'\0',sizeof(szFechaDD));
	memset(szFechaMM,'\0',sizeof(szFechaMM));
	memset(szFechaAA,'\0',sizeof(szFechaAA));
	memset(szFechaTemp,'\0',sizeof(szFechaTemp));
	memset(cDummy,'\0',sizeof(cDummy));

	if (strlen(szFechaI) == 6){//Suponemos ddmmyy...
			sscanf(szFechaI,"%2c%2c%2c",szFechaDD,szFechaMM,szFechaAA);
			sprintf(szFechaTemp,"%s/%s/20%s\0",szFechaDD,szFechaMM,szFechaAA);
	}
	if (strlen(szFechaI) == 8){//Suponemos dd/mm/aa y lo convertimos en dd/mm/aaaa...
		sscanf(szFechaI,"%2c%c%2c%c%2c",szFechaDD,cDummy,szFechaMM,cDummy,szFechaAA);
		sprintf(szFechaTemp,"%s/%s/20%s\0",szFechaDD,szFechaMM,szFechaAA);
	};
	if (strlen(szFechaI) == 10){//Suponemos dd/mm/aaaa...
		strcpy(szFechaTemp, szFechaI);
	};

	jdeToUnicode(LszFechaO,szFechaTemp,11,UTF8);
	szFechaI = _J('\0');
	DeformatDate(&jFechaTemp,LszFechaO,(JCHAR*) _J("DSMSE"));
	FormatDate(LszString01,&jFechaTemp,(JCHAR*) _J("DSMSE"));

	if (jdeStrcmp(LszString01,_J(" ")) == 0)  return 910;// Indica que la fecha es nula...
	else return 0;
}
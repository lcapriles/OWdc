// mp01.cpp
//
//Creado:		Luis Capriles,		Fecha:	09/11/2009 
//Modificado:	Luis Capriles,		Fecha:	01/10/2012 - OWDCmp00 y primeraVez == 8 
//Modificado:	Luis Capriles,		Fecha:	27/10/2012 - Manejo ArroW_UP, ARROW_DOWN 
//
//Funcion OWDCmp01: Funcion para manejar el monitor con CURSES dibujando un menú.  
// Cuenta con los siguientes parámetros configurables:
//	opcionesCantidad: indica la cantidad de opciones del menú
//	inicioEtiqueta: incica la columna para empezar a dibujar las etiquetas de los campos en cada línea
//	opcionesEtiqueta: arreglo con el texto de las etiquetas a desplegar
// Devuelve opcionEscogida

//Funcion OWDCmp02: Funcion para manejar el monitor con CURSES dibujando una pantalla entrada datos.  
// Cuenta con los siguientes parámetros configurables:
//  camposOffset: indica la cantidad de líenas a dejar antes de mostrar el primera líena de campos
//  camposCantidad: indica la cantidad de campos de E/S en a panatalla.  Se dibuja un campo por línea
//  campoInicioCiclo: sin uso
//  inicioEtiquetas: incica la columna para empezar a dibujar las etiquetas de los campos en cada línea
//  inicioCampos: indica la comuna para empezar a dibujar los campos en cada línea
//  ultimaLinea: indicador 1-> que al llegar a la ultimo campo la función termina; 0-> que se puede navegar por la pantalla
//						haciendo uso de TAB hasta que se oprima ENTER
//  CReqTAB: indicador 1-> que ENTER es igual a TAB, esto es, no termina a la función sino que salta al siguiente campo.  Se
//						debe especificar entonces ultimaLinea = 1!!!
//  camposEtiquetas: arreglo con el texto de las etiquetas a desplegar
// Devuelve en el arreglo camposContenido los datos introducidos 

#include "jde.h"  

#include "curses.h"
#include <string.h>
#include <stdlib.h>

//*** Funcion para quitar espacios en blanco...
void OWDCmp00(int camposCantidad,int camposPosiciones[],char camposContenido[][128],int camposErrores[],int iDbgFlg, FILE * dlg)
{
	int i;

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***endwin OWDCmp00...\n"));;
		jdeFflush(dlg);
	}

	for (i = 0; i <= (camposCantidad - 1); i++){// Procesamos los campos...
		if (camposPosiciones[i] == 0){//En principio, si vale 0 es que queremos quitar blancos automáticamente...
			if (strlen(camposContenido[i]) != 0){

				const char* firstNonSpace = camposContenido[i];

				while(*firstNonSpace != '\0' && isspace(*firstNonSpace)){//Quitamos blancos a la izquierda...
					++firstNonSpace;
				}

				int len = strlen(firstNonSpace)+1;         
				memmove(camposContenido[i], firstNonSpace, len);

				char* endOfString = camposContenido[i] + len - 2;
				while(camposContenido[i] < endOfString  && isspace(*endOfString)){//Quitamos blancos a la derecha...
					--endOfString ;
				}
				endOfString++;
				*endOfString = '\0';

				if (camposErrores[i] != 3){//Campo con normal video... Es como una descripción, no una entrada de datos...
					camposPosiciones[i] = strlen(camposContenido[i]);
				}
			}
		}
	}
	return;
}

//*** Funcion para manejar menues...
__declspec( dllexport ) void OWDCmp01 (int * primeraVez,int * opcionEscogida, char * pantallaTitulo, int opcionesCantidad, int inicioEtiquetas, 
									   char * opcionesEtiqueta[], char * pantallaStatusLine, int iDbgFlg, FILE * dlg)
{
	int i, j,  tecla;

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Inicio OWDCmp01...\n"));;
		jdeFflush(dlg);
	}

	if (*primeraVez == 9){
		if (iDbgFlg > 0) {
			jdeFprintf(dlg,_J("***endwin de OWDCmp01...\n"));;
			jdeFflush(dlg);
		}
		endwin();

		return;
	}

	strcpy(ttytype,"VT220");

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Dibujando etiquetas...\n"));;
		jdeFflush(dlg);
	}

	initscr(); //Inicializamos la ventana estandard... 
	start_color();

	init_pair(0,COLOR_WHITE, COLOR_BLACK);
	init_pair(1,COLOR_RED, COLOR_WHITE);
	init_pair(2,COLOR_GREEN, COLOR_BLACK);

	attrset(COLOR_PAIR(2));
	mvaddstr(1,inicioEtiquetas,pantallaTitulo);
	attrset(A_NORMAL);

	mvaddstr(opcionesCantidad + 7,inicioEtiquetas,pantallaStatusLine);

	for (i = 0; i <= (opcionesCantidad - 1); i++){// Dibujamos las etiquetas...
		mvaddstr(i + 4,inicioEtiquetas,opcionesEtiqueta[i]);
	}
	
	mvaddstr(opcionesCantidad + 5,(inicioEtiquetas),"Opcion a elegir..."); //Posicionamos el cursor...
	move(opcionesCantidad + 5,(inicioEtiquetas + strlen("Opcion a elegir...") + 1)); //Posicionamos el cursor...

	refresh();
	keypad(stdscr, TRUE); //Para que getch() devuelva tokens... 

	j = 1;

	while (j == 1){
		tecla = getch();
		switch(tecla){
			case KEY_BTAB: //BTAB
				break;
			case 9: //TAB
				break;
			case 10:
			case 13:
			case KEY_ENTER:
				break;
			case KEY_LEFT:
			case KEY_RIGHT:				
			case KEY_DOWN:
			case KEY_UP:
				break;
			case 8: //BACKSPACE
				break;
			default:
				 * opcionEscogida = tecla - 48;
		}

		refresh();

		switch(tecla){
			case 10:
			case 13:
			case KEY_ENTER:
				j = 2;// CR significa fin!!!...		
		}
	}
	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Fin OWDCmp01...(%d)\n"),*opcionEscogida);
		jdeFflush(dlg);
	}
}


//*** Funcion para manejar pantalla entrada datos...
__declspec( dllexport ) void OWDCmp02 (int * primeraVez, char * pantallaTitulo, int camposOffset, int camposCantidad, 
				int statusOffset, int inicioEtiquetas, int inicioCampos, int ultimaLinea, int CReqTAB,
				int camposPosiciones[], char * camposEtiquetas[], char camposContenido[][128], int camposErrores[],
				char * pantallaStatusLine, int iDbgFlg, FILE * dlg)
{
	//primeraVez == 0: dibuja estiquetas; pasa a 1...
	//primeraVez == 8: dibuja campos sin inverse video; pasa a 9...
	//primaraVez == 9: solo display tal vez con inverse video, no captura el teclado...
	//camposErrores[i] == 1: error->posicionar cursor y resaltar en rojo...
	//camposErrores[i] == 2: posicionar cursor e inverse video...
	//camposErrores[i] == 3: posicionar cursor y normal video, no procesar enter ni caracteres tipeados: una descripción...

	JCHAR szDummy[128];

	int i, j, linea, tecla;
	char campoBlanco[64];
	memset(campoBlanco,' ',sizeof(campoBlanco));
	campoBlanco[63] = '\0';

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Inicio OWDCmp02.v3..\n"));;
		jdeFflush(dlg);
	}

	OWDCmp00(camposCantidad,camposPosiciones,camposContenido,camposErrores,iDbgFlg,dlg);//Formateamos los campos...

	strcpy(ttytype,"VT220");

	if ((*primeraVez == 0) || (*primeraVez == 8) || (*primeraVez == 9)){

		if (iDbgFlg > 0) {
			jdeFprintf(dlg,_J("***Primera Vez: dibujando etiquetas...\n"));;
			jdeFflush(dlg);
		}

	    initscr(); //Inicializamos la ventana estandard...    
		start_color();

		init_pair(0,COLOR_WHITE, COLOR_BLACK);
		init_pair(1,COLOR_RED, COLOR_WHITE);
		init_pair(2,COLOR_GREEN, COLOR_BLACK);

		attrset(COLOR_PAIR(2));
		mvaddstr(1,inicioEtiquetas,pantallaTitulo);
		attrset(A_NORMAL);

		for (i = 0; i <= (camposCantidad - 1); i++){// Dibujamos las etiquetas...
			mvaddstr(i + camposOffset,inicioEtiquetas,camposEtiquetas[i]);
			mvaddstr(i + camposOffset,inicioCampos,campoBlanco);
		}	
	}

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Dibujando campos(%d)...\n"),*primeraVez);;
		jdeFflush(dlg);
	}

	linea = 0;
	j = 0;
	for (i = 0; i <= (camposCantidad - 1); i++){// Dibujamos los campos...
		if (camposErrores[i] == 1){
			beep();
			attrset(COLOR_PAIR(1)|A_BOLD|A_BLINK);  
			//attrset(A_BLINK | COLOR_RED);  
			if (j == 0) {
				linea = i;  //Nos posicionamos en el primer error...
				j = 1;
			}  
		}
		if (camposErrores[i] == 2){ 
			attrset(A_REVERSE);
			if (j == 0) {
				linea = i;  //Nos posicionamos en el campo con inverse video...
				j = 1;
			}  
		}
		if (camposErrores[i] == 3){ //Campo con normal video...
			attrset(A_NORMAL);  
		}
		if ((camposErrores[i] != 1) && (camposErrores[i] != 2) && (camposErrores[i] != 3)){ //Ninguno de los anteriores...
			if (*primeraVez == 8) attrset(A_NORMAL);
			else attrset(A_REVERSE);
		}
		mvaddstr(i + camposOffset,inicioCampos,camposContenido[i]);
	}

	if (*primeraVez == 0) *primeraVez = 1;
	if (*primeraVez == 8) *primeraVez = 9;

	attrset(A_NORMAL);
	mvaddstr(camposOffset + statusOffset,inicioEtiquetas,campoBlanco);
	mvaddstr(camposOffset + statusOffset,inicioEtiquetas,pantallaStatusLine);
	move(linea + camposOffset,(inicioCampos + camposPosiciones[linea])); //Posicionamos el cursor...
	attrset(A_REVERSE);

	refresh();
	keypad(stdscr, TRUE); //Para que getch() devuelva tokens... 

	j = 1;

	while ((j == 1) && (*primeraVez != 9)){
		tecla = getch();
		switch(tecla){
			case KEY_BTAB: //BTAB
				attrset(A_NORMAL);
				mvaddstr(linea + camposOffset,inicioCampos,campoBlanco);
				attrset(A_REVERSE);
				camposContenido[linea][camposPosiciones[linea]] = '\0';
				mvaddstr(linea + camposOffset,inicioCampos,camposContenido[linea]);
				
				linea = abs(--linea);
				move((linea % camposCantidad) + camposOffset,inicioCampos + camposPosiciones[linea % camposCantidad]);
				break;
			case 9: //TAB
				attrset(A_NORMAL);
				mvaddstr(linea + camposOffset,inicioCampos,campoBlanco);
				attrset(A_REVERSE);
				camposContenido[linea][camposPosiciones[linea]] = '\0';
				mvaddstr(linea + camposOffset,inicioCampos,camposContenido[linea]);
				
				linea = ++linea;
				move((linea % camposCantidad) + camposOffset,inicioCampos + camposPosiciones[linea % camposCantidad]);
				break;
			case 10:
			case 13:
			case KEY_ENTER:
				if (camposErrores[linea] != 3){//Campo con normal video... Es como una descripción, no una entrada de datos...
					attrset(A_NORMAL);
					mvaddstr(linea + camposOffset,inicioCampos,campoBlanco);
					attrset(A_REVERSE);
					camposContenido[linea][camposPosiciones[linea]] = '\0';
					mvaddstr(linea + camposOffset,inicioCampos,camposContenido[linea]);
				}
				
				linea = ++linea;
				move((linea % camposCantidad) + camposOffset,inicioCampos + camposPosiciones[linea % camposCantidad]);
				break;
			case KEY_LEFT:
			case KEY_RIGHT:				
			case KEY_DOWN:
				linea = ++linea;
				move((linea % camposCantidad) + camposOffset,inicioCampos + camposPosiciones[linea % camposCantidad]);
				break;
			case KEY_UP:
				linea = abs(--linea);
				move((linea % camposCantidad) + camposOffset,inicioCampos + camposPosiciones[linea % camposCantidad]);
				break;
			case 8: //BACKSPACE
				if (camposPosiciones[linea] > 0){
					camposPosiciones[linea]--;
					camposContenido[linea][camposPosiciones[linea]] = '\0';
				}
					mvaddstr(linea + camposOffset,inicioCampos,camposContenido[linea]);
				break;
			default:
				if (camposErrores[linea] != 3){//Campo con normal video... Es como una descripción, no una entrada de datos...
					camposContenido[linea][camposPosiciones[linea]] = tecla;
					camposContenido[linea][camposPosiciones[linea] + 1] = '\0';
					camposPosiciones[linea]++;
					mvaddstr(linea + camposOffset,inicioCampos,camposContenido[linea]);
				}
				
		}

		refresh();

		switch(tecla){
			case 10:
			case 13:
			case KEY_ENTER:
				if (CReqTAB == 0)  j = 2;// CR significa fin!!!...
				
		}
		if ((linea == camposCantidad) && (ultimaLinea == 1) && //Llegamos al último campo y queremos salir por (ultimaLinea == 1)...
			(tecla != KEY_DOWN) && (tecla != KEY_UP)) break;//Pero ni llegamos por medi de las flechas...
		linea = linea % camposCantidad;
	}
	move(camposOffset + camposCantidad + 1,0);

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Fin OWDCmp02...\n"));
		jdeFflush(dlg);
	}

	attrset(A_NORMAL);

	if (iDbgFlg > 0) {
		for (i = 0; i <= (camposCantidad - 1); i++){
			jdeToUnicode(szDummy,camposContenido[i],127,UTF8);
			jdeFprintf(dlg,_J("***camposContenido[%d]...(%ls)\n"),i,szDummy);
			jdeFflush(dlg);
		}
	}
}


//*** Funcion para terminar curses...
__declspec( dllexport ) void OWDCmp90(int iDbgFlg, FILE * dlg)
{

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***endwin OWDCmp90...\n"));;
		jdeFflush(dlg);
	}

	endwin();

	return;
}




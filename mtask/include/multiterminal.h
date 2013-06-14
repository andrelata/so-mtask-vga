#ifndef MULTITERMINAL_H_INCLUDED
#define MULTITERMINAL_H_INCLUDED

#define MTTYCANT 4

typedef unsigned short row[NUMCOLS];

/*
 * Estructura que contiene la informacion necesaria para definir una terminal
 */

typedef struct tty{

	int ttyNumber; 
	MsgQueue_t *key_mq; 

	/*variables de consola*/
	row *vidmem = (row *) VIDMEM;
	unsigned cur_x, cur_y, cur_attr = DEFATTR;
	bool cursor_on = true;
	bool raw;
	unsigned scrolls;

	//shellLine_t * lineBuffer;   //contiene informaciónion de la linea donde se escriben los comandos
 } tty_t;

/*
 * Estructura que contiene el vector de terminales y la terminal actual.
 * Se debe acceder a traves de las funciones del multiterminal.h
 */

typedef struct mtty{

	int ttyActual; 
	tty_t terminales[MTTYCANT];
	
 } mtty_t;

static mtty_t multiterminal;

#endif

#ifndef MULTITERMINAL_H_INCLUDED
#define MULTITERMINAL_H_INCLUDED

#define MTTYCANT 4

#define VIDMEM 0xB8000
#define NUMROWS 25
#define NUMCOLS 80
#define TABSIZE 8
#define OFFSET_LIMIT 26
#define TTY_MEM 0x2000

#define CRT_ADDR 0x3D4
#define CRT_DATA 0x3D5
#define CRT_CURSOR_START 0x0A
#define CRT_CURSOR_END 0x0B
#define CRT_CURSOR_HIGH 0x0E
#define CRT_CURSOR_LOW 0x0F

#define COM_NUM 5
#define COM_SIZE 360

#define DEFATTR ((BLACK << 12) | (LIGHTGRAY << 8))

#define KBDBUFSIZE	360

typedef unsigned short row[NUMCOLS];

/*
 * Estructura que contiene la informacion necesaria para definir una terminal
 */

typedef struct tty{

	int ttyNumber;
	MsgQueue_t *key_mq;
	row *vidmem;// = (row *) VIDMEM;
	unsigned offset;
	unsigned cur_x, cur_y, cur_attr;// = DEFATTR;
	bool cursor_on;// = true;
	bool raw;
	unsigned scrolls;
	struct commands{
		char commandList[COM_NUM][COM_SIZE];
		unsigned last;
		unsigned current;
	}commands;

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
void mt_mtty_scroll();
void mt_mtty_scrollUp();
void mt_mtty_scrollDown();
void mt_mtty_clear();

void changeTty(unsigned tty);
row * getVidmem();
unsigned getOffset();
void setCur_x(unsigned x);
unsigned getCur_x();
void decCur_x();
void setCur_y(unsigned y);
unsigned getRealCur_y();
unsigned getCur_y();
void incCur_y();
void incCur_x();
void decCur_y();
bool getCursor_On();
void setCursor_On(bool on);
void setCur_attr(unsigned attr);
unsigned getCur_attr();
unsigned getScrolls();
void incScrolls();
bool getRaw();
void setRaw(bool newValue);
void putty(unsigned char ch);
void mt_mtty_putInKbd(void *c);
bool mt_mtty_getFromKbd(void *c);
bool mt_mtty_getFromKbdTimed(void *c, unsigned timeout);
void setcursor(void);
void correctOffset();

#endif

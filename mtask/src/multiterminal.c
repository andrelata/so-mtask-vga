#include "kernel.h"
#include "multiterminal.h"

static unsigned ctrcA, ctrcD;

void
mt_mtty_init(void){
	
	int i=0;
	for(i=0; i < MTTYCANT; i++)
	{
     		multiterminal.ttyActual = i;
		multiterminal.terminales[i].cur_attr = DEFATTR;
		multiterminal.terminales[i].offset = 0;
		multiterminal.terminales[i].cur_y = 5;
		multiterminal.terminales[i].cursor_on = true;
		multiterminal.terminales[i].vidmem = (row *) (VIDMEM + TTY_MEM*i);
		mt_cons_clear();
        	multiterminal.terminales[i].key_mq = CreateMsgQueue("Input key", KBDBUFSIZE, 1, true, false);
	}
	multiterminal.ttyActual = 0;
	mt_cons_cursor(true);
	
	unsigned adSel = inb(0x3CC);

	if((adSel%2)==0){
		ctrcA=0x3B4;
		ctrcD=0x3B5;
	}else{
		ctrcA=0x3D4;
		ctrcD=0x3D5;
	}
	
unsigned off = (TTY_MEM/2)*multiterminal.ttyActual;	

	/* Start Address Low Register*/
	outb(ctrcA,0xD);
	outb(ctrcD,off);
	/* Start Address High Register*/
	outb(ctrcA,0xC);
	outb(ctrcD,off>>8);
}

row *
getVidmem(){

	return multiterminal.terminales[multiterminal.ttyActual].vidmem;

}

unsigned
getOffset(){
	return multiterminal.terminales[multiterminal.ttyActual].offset;
}

void
setCur_x(unsigned x){
	multiterminal.terminales[multiterminal.ttyActual].cur_x = x;
}

unsigned
getRealCur_x(){
	return ((multiterminal.ttyActual*TTY_MEM)%160)/2+multiterminal.terminales[multiterminal.ttyActual].cur_x;
}

unsigned
getCur_x(){
	return multiterminal.terminales[multiterminal.ttyActual].cur_x;
}

void
decCur_x(){
	multiterminal.terminales[multiterminal.ttyActual].cur_x--;
}

void
setCur_y(unsigned y){
	multiterminal.terminales[multiterminal.ttyActual].cur_y = y;
}

unsigned
getRealCur_y(){
	return (multiterminal.ttyActual*TTY_MEM)/160+multiterminal.terminales[multiterminal.ttyActual].offset+multiterminal.terminales[multiterminal.ttyActual].cur_y;
}

unsigned
getCur_y(){
	return multiterminal.terminales[multiterminal.ttyActual].cur_y;
}

void
incCur_y(){
	multiterminal.terminales[multiterminal.ttyActual].cur_y++;
}

void
incCur_x(){
	multiterminal.terminales[multiterminal.ttyActual].cur_x++;
}

void
decCur_y(){
	multiterminal.terminales[multiterminal.ttyActual].cur_y--;
}

bool
getCursor_On(){
	return multiterminal.terminales[multiterminal.ttyActual].cursor_on;
}

void
setCursor_On(bool on){
	multiterminal.terminales[multiterminal.ttyActual].cursor_on = on;
}

void
setCur_attr(unsigned attr){
	multiterminal.terminales[multiterminal.ttyActual].cur_attr = attr;
}

unsigned
getCur_attr(){
	return multiterminal.terminales[multiterminal.ttyActual].cur_attr;
}

unsigned
getScrolls(){
	return multiterminal.terminales[multiterminal.ttyActual].scrolls;
}

void
incScrolls(){
	multiterminal.terminales[multiterminal.ttyActual].scrolls++;
}

bool
getRaw(){
	return multiterminal.terminales[multiterminal.ttyActual].raw;
}

void
setRaw(bool newValue){
	multiterminal.terminales[multiterminal.ttyActual].raw = newValue;
}

void
putty(unsigned char ch){

	unsigned cur_x = getCur_x(), cur_y = getCur_y(), cur_attr = getCur_attr();
	
	multiterminal.terminales[multiterminal.ttyActual].vidmem[cur_y][cur_x++] = (ch & 0xFF) | cur_attr;

}

void
putIntottyKbd(void *c){
	PutMsgQueue(multiterminal.terminales[multiterminal.ttyActual].key_mq, c);
}

bool
getFromttyKbd(void *c){
	return GetMsgQueue(multiterminal.terminales[multiterminal.ttyActual].key_mq, c);
}

bool
getFromttyKbdTimed(void *c, unsigned timeout){
	return GetMsgQueueTimed(multiterminal.terminales[multiterminal.ttyActual].key_mq, c, timeout);
}


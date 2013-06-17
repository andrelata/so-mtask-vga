#include "kernel.h"
#include "multiterminal.h"

void
mt_mtty_init(void){
	
	int i=0;
	for(i=0; i < MTTYCANT; i++)
	{
     		multiterminal.ttyActual = i;
		mt_cons_clear();
		mt_cons_cursor(true);
        	multiterminal.terminales[i].key_mq = CreateMsgQueue("Input key", KBDBUFSIZE, 1, true, false);
		multiterminal.terminales[i].cur_attr = DEFATTR;
		multiterminal.terminales[i].cursor_on = true;
		multiterminal.terminales[i].vidmem = (row *) VIDMEM;
	}
	
}

row *
getVidmem(){

	return multiterminal.terminales[multiterminal.ttyActual].vidmem;

}

void
setCur_x(unsigned x){
	multiterminal.terminales[multiterminal.ttyActual].cur_x = x;
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
getCur_y(){
	return multiterminal.terminales[multiterminal.ttyActual].cur_y;
}

void
incCur_y(){
	multiterminal.terminales[multiterminal.ttyActual].cur_y++;
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
setCur_attr(unsigned char attr){
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


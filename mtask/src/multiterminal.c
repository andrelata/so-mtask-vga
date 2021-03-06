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
		multiterminal.terminales[i].cur_y = 0;
		multiterminal.terminales[i].cursor_on = true;
		multiterminal.terminales[i].vidmem = (row *) (VIDMEM + TTY_MEM*i);
		mt_cons_clear();
        	multiterminal.terminales[i].key_mq = CreateMsgQueue("Input key", KBDBUFSIZE, 1, true, false);
		
		multiterminal.terminales[i].commands.last = 0;
		multiterminal.terminales[i].commands.current = 0;
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
	mt_cons_changeTty(0);
}

static void
changeStartAddress(unsigned offset){
	/* Start Address Low Register*/
	outb(ctrcA,0x0D);
	outb(ctrcD,offset);
	/* Start Address High Register*/
	outb(ctrcA,0x0C);
	outb(ctrcD,offset>>8);
}

void
mt_mtty_splitScreen(){
	if(multiterminal.ttyActual != 0)
	{
		DisableInts();
		outb(ctrcA,0x18);
		if(inb(ctrcD) == 0x0F){
			outb(ctrcD,0xFF);
			outb(ctrcA,0x07);
			unsigned d = inb(ctrcD);
			d = d | 0x10;
			outb(ctrcD,d);
			outb(ctrcA,0x09);
			d = inb(ctrcD);
			d = d | 0x40;
			outb(ctrcD,d);
			printk("split-screen mode off\n");
		}else{
			outb(ctrcD,0x0F);
			outb(ctrcA,0x07);
			unsigned d = inb(ctrcD);
			printk("d: %x\n", d);
			d = d & 0xEF;
			printk("nuevo d: %x\n", d);
			outb(ctrcD,d);
			outb(ctrcA,0x09);
			d = inb(ctrcD);
			printk("d: %x\n", d);
			d = d & 0xBF;
			printk("nuevo d: %x\n", d);
			outb(ctrcD,d);
			printk("split-screen mode on\n");
			}
		/* Line Compare register Register*/
		RestoreInts();
	}
}

void
changeTty(unsigned tty){
	if(multiterminal.ttyActual != tty && tty < MTTYCANT){
		unsigned off = (TTY_MEM/2)*tty + NUMCOLS*(multiterminal.terminales[tty].offset);
		changeStartAddress(off);
		multiterminal.ttyActual = tty;
	}
}

row *
getVidmem(){

	return multiterminal.terminales[CurrentTty()].vidmem;

}

unsigned
getOffset(){
	return multiterminal.terminales[CurrentTty()].offset;
}

void
setCur_x(unsigned x){
	multiterminal.terminales[CurrentTty()].cur_x = x;
}

unsigned
getRealCur_x(){
	return ((CurrentTty()*TTY_MEM)%160)/2+multiterminal.terminales[CurrentTty()].cur_x;
}

unsigned
getCur_x(){
	return multiterminal.terminales[CurrentTty()].cur_x;
}

void
decCur_x(){
	multiterminal.terminales[CurrentTty()].cur_x--;
}

void
setCur_y(unsigned y){
	multiterminal.terminales[CurrentTty()].cur_y = y;
}

unsigned
getRealCur_y(){
	return (CurrentTty()*TTY_MEM)/160+multiterminal.terminales[CurrentTty()].cur_y;
}

unsigned
getCur_y(){
	return multiterminal.terminales[CurrentTty()].cur_y;
}

void
incCur_y(){
	multiterminal.terminales[CurrentTty()].cur_y++;
}

void
incCur_x(){
	multiterminal.terminales[CurrentTty()].cur_x++;
}

void
decCur_y(){
	multiterminal.terminales[CurrentTty()].cur_y--;
}

bool
getCursor_On(){
	return multiterminal.terminales[CurrentTty()].cursor_on;
}

void
setCursor_On(bool on){
	multiterminal.terminales[CurrentTty()].cursor_on = on;
}

void
setCur_attr(unsigned attr){
	multiterminal.terminales[CurrentTty()].cur_attr = attr;
}

unsigned
getCur_attr(){
	return multiterminal.terminales[CurrentTty()].cur_attr;
}

unsigned
getScrolls(){
	return multiterminal.terminales[CurrentTty()].scrolls;
}

void
incScrolls(){
	multiterminal.terminales[CurrentTty()].scrolls++;
}

bool
getRaw(){
	return multiterminal.terminales[CurrentTty()].raw;
}

void
setRaw(bool newValue){
	multiterminal.terminales[CurrentTty()].raw = newValue;
}

void
putty(unsigned char ch){

	unsigned cur_x = getCur_x(), cur_y = getCur_y(), cur_attr = getCur_attr();
	
	multiterminal.terminales[CurrentTty()].vidmem[cur_y][cur_x++] = (ch & 0xFF) | cur_attr;

}

static void
scrollRows(unsigned rows){
	unsigned tty = CurrentTty();
	row * vidmem = multiterminal.terminales[tty].vidmem;
	int i,j;
	for (i = rows; i < NUMROWS+OFFSET_LIMIT; i++)
		memcpy(&vidmem[i - rows], &vidmem[i], sizeof(row));
	for (i = NUMROWS+OFFSET_LIMIT-rows; i < NUMROWS+OFFSET_LIMIT; i++)
		for(j = 0; j < NUMCOLS; j++)
			vidmem[i][j] = DEFATTR;
	multiterminal.terminales[tty].cur_y = multiterminal.terminales[tty].cur_y - (rows - 1);
}

void
mt_mtty_scroll(){
	unsigned tty = CurrentTty();
	if(multiterminal.terminales[tty].cur_y < NUMROWS + OFFSET_LIMIT - 1){
		multiterminal.terminales[tty].cur_y++;
		multiterminal.terminales[tty].offset = multiterminal.terminales[tty].cur_y - (NUMROWS-1);
	}else{
		scrollRows(10);
		multiterminal.terminales[tty].offset = multiterminal.terminales[tty].cur_y - (NUMROWS-1);
	}
	if(tty==multiterminal.ttyActual){
		unsigned off = (TTY_MEM/2)*tty + NUMCOLS*(multiterminal.terminales[tty].offset);
		changeStartAddress(off);
	}
	incScrolls();
}

void
mt_mtty_scrollUp(){
	unsigned tty = multiterminal.ttyActual;
	if(multiterminal.terminales[tty].offset > 0){
		multiterminal.terminales[tty].offset--;
		unsigned off = (TTY_MEM/2)*tty + NUMCOLS*(multiterminal.terminales[tty].offset);
		changeStartAddress(off);
	}
}

void
mt_mtty_scrollDown(){
	unsigned tty = multiterminal.ttyActual;
	if(multiterminal.terminales[tty].offset < OFFSET_LIMIT){
		multiterminal.terminales[tty].offset++;
		unsigned off = (TTY_MEM/2)*tty + NUMCOLS*(multiterminal.terminales[tty].offset);
		changeStartAddress(off);
	}
}

void
mt_mtty_clear(){
	unsigned tty = CurrentTty();
	row * vidmem = multiterminal.terminales[tty].vidmem;
	unsigned short *p1 = &vidmem[0][0];
	unsigned short *p2 = &vidmem[NUMROWS+OFFSET_LIMIT][0];

	while (p1 < p2)
		*p1++ = DEFATTR;

	multiterminal.terminales[tty].cur_x = 0;
	multiterminal.terminales[tty].cur_y = 0;
	multiterminal.terminales[tty].offset = 0;
	if(tty==multiterminal.ttyActual){
		unsigned off = (TTY_MEM/2)*tty + NUMCOLS*(multiterminal.terminales[tty].offset);
		changeStartAddress(off);
	}
}

void
setcursor(void)
{
	bool cursor_on = multiterminal.terminales[multiterminal.ttyActual].cursor_on;;
	unsigned cur_y = (multiterminal.ttyActual*TTY_MEM)/160+multiterminal.terminales[multiterminal.ttyActual].cur_y;
	unsigned cur_x = ((multiterminal.ttyActual*TTY_MEM)%160)/2+multiterminal.terminales[multiterminal.ttyActual].cur_x;
	if (cursor_on)
	{
		unsigned off = cur_y * NUMCOLS + cur_x;
		outb(CRT_ADDR, CRT_CURSOR_HIGH);
		outb(CRT_DATA, off >> 8);
		outb(CRT_ADDR, CRT_CURSOR_LOW);
		outb(CRT_DATA, off);
	}
}

void
correctOffset(){
	unsigned tty = CurrentTty();
	if(multiterminal.terminales[tty].cur_y < multiterminal.terminales[tty].offset)
		multiterminal.terminales[tty].offset = multiterminal.terminales[tty].cur_y;
	else if(multiterminal.terminales[tty].cur_y > multiterminal.terminales[tty].offset + NUMROWS - 1)
		multiterminal.terminales[tty].offset = multiterminal.terminales[tty].cur_y-NUMROWS+1;
	if(tty==multiterminal.ttyActual){
		unsigned off = (TTY_MEM/2)*tty + NUMCOLS*(multiterminal.terminales[tty].offset);
		changeStartAddress(off);
	}
}

void
mt_mtty_putInKbd(void *c){
	PutMsgQueue(multiterminal.terminales[multiterminal.ttyActual].key_mq, c);
}

bool
mt_mtty_getFromKbd(void *c){
	return GetMsgQueue(multiterminal.terminales[CurrentTty()].key_mq, c);
}

bool
mt_mtty_getFromKbdTimed(void *c, unsigned timeout){
	return GetMsgQueueTimed(multiterminal.terminales[CurrentTty()].key_mq, c, timeout);
}

unsigned
getLast(){
	return multiterminal.terminales[CurrentTty()].commands.last;
}

unsigned
getCurrent(){
	return multiterminal.terminales[CurrentTty()].commands.current;
}

void 
mt_mtty_insertCommand(char* buf){

	unsigned last = getLast();
	int i,j;
	if(strcmp(buf, "\n") == 0){
		//printk("Es enter\n");
		return;
	}
	if(last == COM_NUM){
		for(i=0; i < last-1; i++){
			strcpy(multiterminal.terminales[CurrentTty()].commands.commandList[i], 
				multiterminal.terminales[CurrentTty()].commands.commandList[i+1]);
			}
			last--;
			multiterminal.terminales[CurrentTty()].commands.last = last;
	}
	
	unsigned size = strlen(buf);
	//buf[size] = '\0';
	if(size <= COM_SIZE){
		strcpy(multiterminal.terminales[CurrentTty()].commands.commandList[last], 
				buf/*, size*/);
	}else{
		//printk("me pase");
		strncpy(multiterminal.terminales[CurrentTty()].commands.commandList[last], 
				buf, COM_SIZE);
		//multiterminal.terminales[CurrentTty()].commands.commandList[last][COM_SIZE] = '\0';
	}
	multiterminal.terminales[CurrentTty()].commands.last++;
	multiterminal.terminales[CurrentTty()].commands.current = multiterminal.terminales[CurrentTty()].commands.last;

	
	/*for (i=0; i < size-1; i++) {

               if (i > COM_SIZE) {
                       break;
               }
               multiterminal.terminales[CurrentTty()].commands.commandList[last][i] = buf[i];
       }
       
       multiterminal.terminales[CurrentTty()].commands.commandList[last][i-1] = 0;*/
	
}

void 
mt_mtty_getNextCommand(char* buf){
	unsigned current = getCurrent();
	unsigned last = getLast();

	//borra la linea
	mt_mtty_cleanLine(strlen(buf));
	
	if(current != 0 ){
		current--;
		multiterminal.terminales[CurrentTty()].commands.current = current;
		
	}
	//printk("arriba:%d\n",current);
	strcpy(buf, multiterminal.terminales[CurrentTty()].commands.commandList[current]);
	mt_mtty_getCommand(buf);

}

void 
mt_mtty_getPrevCommand(char* buf){
	unsigned current = getCurrent();
	unsigned last = getLast();

	//borra la linea
	mt_mtty_cleanLine(strlen(buf));

	if(current < last-1){
		current++;
		multiterminal.terminales[CurrentTty()].commands.current = current;
		strcpy(buf, multiterminal.terminales[CurrentTty()].commands.commandList[current]);
		mt_mtty_getCommand(buf);
	}else if(current == last - 1){
		current++;
		multiterminal.terminales[CurrentTty()].commands.current = current;
		//printk("\n%s\n", buf);
		mt_mtty_cleanLine(strlen(buf));
	}
	//printk("abajo:%d\n",current);

}



void
mt_mtty_getCommand(char* buf)
{
	unsigned ch;
	int i=0;
	int size = strlen(buf);

	for(i; i<size-1; i++){
		ch = buf[i];
		mt_mtty_putInKbd(&ch);
	}
}

void
mt_mtty_cleanLine(int size)
{
	unsigned ch;

	while(size!=0){
		ch = 8;
		mt_mtty_putInKbd(&ch);
		size--;
	}
}


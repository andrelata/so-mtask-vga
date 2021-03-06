#include "kernel.h"
#include "multiterminal.h"

#define BS 0x08

typedef unsigned short row[NUMCOLS];
//static row *vidmem = (row *) VIDMEM;
//static unsigned cur_x, cur_y; //, cur_attr = DEFATTR;
//static bool cursor_on = true;
//static bool raw;
//static unsigned scrolls;


static void
scroll(void)
{
	/*int j;
	row * vidmem = getVidmem();

	for (j = 1; j < NUMROWS; j++)
		memcpy(&vidmem[j - 1], &vidmem[j], sizeof(row));
	for (j = 0; j < NUMCOLS; j++)
		vidmem[NUMROWS - 1][j] = DEFATTR;
	//scrolls++;
	incScrolls();*/
	mt_mtty_scroll();
}

static void
put(unsigned char ch)
{
	row * vidmem = getVidmem();
	unsigned cur_attr = getCur_attr(), cur_x = getCur_x(), cur_y = getCur_y();
	vidmem[cur_y][cur_x] = (ch & 0xFF) | cur_attr;
	incCur_x();
	if (cur_x+1 >= NUMCOLS)
	{
		setCur_x(0);
		if (cur_y >= NUMROWS + getOffset() - 1)
			scroll();
		else
			incCur_y();
	}
	setcursor();
	correctOffset();
}

/* Interfaz pública */

void
mt_cons_changeTty(unsigned tty){
	changeTty(tty);
	setcursor();
}

void
mt_cons_clear(void)
{
	mt_mtty_clear();
}

void
mt_cons_clreol(void)
{
	row * vidmem = getVidmem();
	unsigned cur_x = getCur_x(), cur_y = getCur_y();
	unsigned short *p1 = &vidmem[cur_y][cur_x];
	unsigned short *p2 = &vidmem[cur_y + 1][0];

	while (p1 < p2)
		*p1++ = DEFATTR;
}

void
mt_cons_clreom(void)
{
	row * vidmem = getVidmem();
	unsigned cur_x = getCur_x(), cur_y = getCur_y();
	unsigned short *p1 = &vidmem[cur_y][cur_x];
	unsigned short *p2 = &vidmem[NUMROWS][0];

	while (p1 < p2)
		*p1++ = DEFATTR;
}

unsigned
mt_cons_nrows(void)
{
	return NUMROWS;
}

unsigned
mt_cons_ncols(void)
{
	return NUMCOLS;
}

unsigned
mt_cons_nscrolls(void)
{
	return getScrolls();
}

void
mt_cons_getxy(unsigned *x, unsigned *y)
{
	unsigned cur_x = getCur_x(), cur_y = getCur_y();
	*x = cur_x;
	*y = cur_y;
}

void
mt_cons_gotoxy(unsigned x, unsigned y)
{
	if (y < NUMROWS && x < NUMCOLS)
	{
		setCur_x(x);
		setCur_y(y);
		setcursor();
	}
}

void
mt_cons_setattr(unsigned fg, unsigned bg)
{
	setCur_attr(((fg & 0xF) << 8) | ((bg & 0xF) << 12));
}

void
mt_cons_getattr(unsigned *fg, unsigned *bg)
{
	unsigned cur_attr = getCur_attr();
	*fg = (cur_attr >> 8) & 0xF;
	*bg = (cur_attr >> 12) & 0xF;
}

bool
mt_cons_cursor(bool on)
{
	bool prev = getCursor_On();
	unsigned start = on ? 14 : 1, end = on ? 15 : 0;

	outb(CRT_ADDR, CRT_CURSOR_START);
	outb(CRT_DATA, start);
	outb(CRT_ADDR, CRT_CURSOR_END);
	outb(CRT_DATA, end);
	setCursor_On(on);
	setcursor();
	return prev;
}

void
mt_cons_putc(char ch)
{
	bool raw = getRaw();
	if (raw)
	{
		put(ch);
		return;
	}

	switch (ch)
	{
		case '\t':
			mt_cons_tab();
			break;

		case '\r':
			mt_cons_cr();
			break;

		case '\n':
			mt_cons_nl();
			break;

		case BS:
			mt_cons_bs();
			break;

		default:
			put(ch);
			break;
	}
}

void
mt_cons_puts(const char *str)
{
	while (*str)
		mt_cons_putc(*str++);
}

void
mt_cons_cr(void)
{
	setCur_x(0);
	setcursor();
}

void
mt_cons_nl(void)
{
	unsigned cur_y = getCur_y();
	if (cur_y >= NUMROWS + getOffset() - 1)
		scroll();
	else
		incCur_y();
	setcursor();
}

void
mt_cons_tab(void)
{
	unsigned cur_x = getCur_x();
	unsigned nspace = TABSIZE - (cur_x % TABSIZE);
	while (nspace--)
		put(' ');
}

void
mt_cons_bs(void)
{
	unsigned cur_x = getCur_x(), cur_y = getCur_y();
	if (cur_x)
		decCur_x();
	else if (cur_y)
	{
		decCur_y();
		setCur_x(NUMCOLS - 1);
	}
	setcursor();
}

bool
mt_cons_raw(bool on)
{
	bool prev = getRaw();
	setRaw(on);
	return prev;
}

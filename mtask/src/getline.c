#include "kernel.h"

#define BS 0x08
#define ERASEBACK "\x08 \x08"

unsigned
mt_getline(char *buf, unsigned size)
{
	char *p = buf, *end = buf + size - 1;
	unsigned c;
	unsigned xi, yi, si;
	unsigned len;

	mt_cons_getxy(&xi, &yi);
	si = mt_cons_nscrolls();

	while (p < end)
		if (mt_kbd_getch(&c)){
			switch (c)
			{
				case BS:
					if (p == buf)
						break;
					if (*--p == '\t')
					{
						mt_cons_gotoxy(xi, yi - (mt_cons_nscrolls() - si));
						mt_cons_clreom();
						*p = 0;
						mt_cons_puts(buf);

					}
					else
						mt_cons_puts(ERASEBACK);
					break;

				case '\r':
				case '\n':
					mt_cons_puts("\r\n");
					*p++ = '\n';
					*p = 0;
					mt_mtty_insertCommand(buf);
					return p - buf;
				case 24: /*up*/
					//printk("flecha para arriba\n");
					mt_mtty_getNextCommand(buf);
					//printk("%d\n",getCurrent());
					//printk("trae:%s", buf);
					break;
				case 25: /*down*/
					//printk("flecha para abajo\n");
					mt_mtty_getPrevCommand(buf);
					//printk("%d\n",getCurrent());
					//printk("trae:%s", buf);
					break;
				case 9: /*tab*/
					//printk("entra1");
					//printk("buf:%s\n", buf);
					//printk("p:%s\n", p);
					//printk("p-buf:%s\n", p-buf);
					len = mt_shell_autoComplete(buf,p-buf); // chequear si es p o p-buf
					p += len;
					//printk("trae:%s\n", buf);
					break;
				default:
					*p++ = c;
					mt_cons_putc(c);
					break;
			}
		}
	mt_cons_puts("<EOB>\r\n");
	*p = 0;
	return p - buf;
}

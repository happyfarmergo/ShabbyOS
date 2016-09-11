
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        char output[2] = {'\0', '\0'};

        if (!(key & FLAG_EXT)) {
		    put_key(p_tty, key);
            if(p_tty->b_scanf)
                put_buf(p_tty, key);
        }
        else {
            int raw_code = key & MASK_RAW;
            switch(raw_code) {
            case ENTER:
			    put_key(p_tty, '\n');
                put_str(p_tty);
			    break;
            case BACKSPACE:
			put_key(p_tty, '\b');
            p_tty->buf_len--;
			break;
                case UP:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_DN);
                        }
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_UP);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* Ctrl + F1~F12 */
			if ((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) {
				select_console(raw_code - F1);
			}
			break;
                default:
                        break;
                }
        }
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}

void put_buf(TTY* p_tty, u32 key){
    p_tty->buffer[p_tty->buf_len++] = key;
}

void put_str(TTY *p_tty){
    int i;
    for(i = 0; i<p_tty->buf_len; ++i){
        p_tty->str[i] = p_tty->buffer[i];
    }
    p_tty->str[i] = '\0';
    p_tty->str_len = p_tty->buf_len;
    scanf_off(p_tty);
    // wake(p_tty->proc_waiting);
}

PUBLIC void scanf_on(TTY *p_tty){
    p_tty->b_scanf = TRUE;
    p_tty->buf_len = 0;
}

PUBLIC void scanf_off(TTY *p_tty){
    p_tty->b_scanf = FALSE;
    p_tty->buf_len = 0;
}

/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}

/*======================================================================*
                              tty_write
*======================================================================*/
PUBLIC void tty_write(TTY* p_tty, char* buf, int len)
{
        char* p = buf;
        int i = len;

        while (i) {
                out_char(p_tty->p_console, *p++);
                i--;
        }
}

/*======================================================================*
                              sys_write
*======================================================================*/
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc)
{
        tty_write(&tty_table[p_proc->nr_tty], buf, len);
        return 0;
}

//kernel
int printl(const char *fmt, ...)
{
    int i;
    char buf[256];

    va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
    i = vsprintf(buf, fmt, arg);

    disp_str("printl");

    sys_write(buf, i, proc_table);
    
    return i;
}


PRIVATE void wait_scanf(TTY *p_tty, PROCESS *p_proc){
    p_tty->proc_waiting = p_proc;
    
    // block();
    while(p_tty->b_scanf);

    p_tty->proc_waiting = NULL;
}

PUBLIC char *sys_scanf(PROCESS* p_proc){
    TTY *p_tty = tty_table + p_proc->nr_tty;
    disp_str("begin scanf");
    scanf_on(p_tty);
    wait_scanf(p_tty, p_proc);
    disp_str("end scanf");
    return p_tty->str;
}
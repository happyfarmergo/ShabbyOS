
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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
#include "proto.h"

EXTERN proc_node *h_ready, *h_waiting;
EXTERN proc_node proc_list[];
EXTERN int index_free;

PUBLIC void show_ready_list(){
    proc_node *p = h_ready;
    for(;p!=NULL; p = p->next)
        disp_str(p->kproc->p_name);
    disp_str("\n");
}


PUBLIC   void kernel_init(){
    h_ready = h_waiting = NULL;
    index_free = 0;

    int i = 0;
    proc_node * p = proc_list;
    for(; i < MAX_PROCS; ++i){
        p->kproc = p->prev = p->next = NULL;
        p++;
    }
}



PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

    kernel_init();

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
    u8              privilege;
    u8              rpl;
    int             eflags;
    int             prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
            if (i < NR_TASKS) {     /* 任务 */
                p_task    = task_table + i;
                privilege = PRIVILEGE_TASK;
                rpl       = RPL_TASK;
                prio      = HIGH;
                eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
            }
            else {                  /* 用户进程 */
                p_task    = user_proc_table + (i - NR_TASKS);
                privilege = PRIVILEGE_USER;
                rpl       = RPL_USER;
                prio      = MEDIUM;
                eflags    = 0x202; /* IF=1, bit 2 is always 1 */
            }

		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

        //add
        p_proc->priority = prio;
        p_proc->ticks = 200/(p_proc->priority);
        p_proc->status = INIT;

        change_proc_list(INIT, READY, p_proc);

		p_proc->nr_tty = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;

	}

    proc_table[1].nr_tty = 0;
    proc_table[2].nr_tty = 1;
    proc_table[3].nr_tty = 2;

	change_proc_list(READY, RUNNING, proc_table);

	init_clock();
    init_keyboard();

    k_reenter = 0;
    ticks = 0;

	restart();

	while(1){}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int i = 0;
	while (1) {
		printf("<Ticks:%x>", get_ticks());
		milli_delay(200);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
	while(1){
		printf("B");
		milli_delay(200);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	int i = 0x2000;
	while(1){
		printf("C");
		milli_delay(200);
	}
}



void clearScreen()
{
    int i;
    disp_pos=0;
    for(i=0;i<80*25;i++)
    {
        disp_str(" ");
    }
    disp_pos=0;
    
}


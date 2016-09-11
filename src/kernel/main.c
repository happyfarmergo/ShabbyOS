
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

EXTERN proc_node *h_ready[], *h_waiting;
EXTERN proc_node proc_list[];
EXTERN int index_free;
EXTERN int k;


int strcmp(char *str1,char *str2)
{
    int i;
    for (i=0; i<strlen(str1); i++)
    {
        if (i==strlen(str2)) return 1;
        if (str1[i]>str2[i]) return 1;
        else if (str1[i]<str2[i]) return -1;
    }
    return 0;
}

void strlwr(char *str)
{
    int i;
    for (i=0; i<strlen(str); i++)
    {
        if ('A'<=str[i] && str[i]<='Z') str[i]=str[i]+'a'-'A';
    }
}


PUBLIC void show_ready_list(){
    int i;
    disp_str("ready list : \n");
    for(i = REALTIME; i >= IDLE; --i){
        disp_int(i);
        disp_str(": ");
        proc_node *p = h_ready[i];
        for(;p!=NULL; p = p->next){
            disp_str(p->kproc->p_name);
            disp_int(p->kproc->ticks);
            disp_str(" ");
        }
        disp_str("\n");

    }
}

PUBLIC void show_waiting_list(){
    proc_node *p = h_waiting;
    disp_str("waiting list\n");
    while(p!=NULL){
        disp_str("\n");
        disp_str(p->kproc->p_name);
        p = p->next;
    }
}


PUBLIC   void kernel_init(){
    h_waiting = NULL;
    index_free = 0;
    k = 0;
    int i = 0;
    proc_node * p = proc_list;
    for(; i < MAX_PROCS; ++i){
        p->kproc = p->prev = p->next = NULL;
        p->index = i;
        p++;
    }
    for(i=0;i<PRIO_NUM;++i)
        h_ready[i] = NULL;
}



PUBLIC int kernel_main()
{
    clearScreen();
    displayWelcome();
    kernel_init();

    // disp_str("test");
    
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
        p_proc->ticks = p_proc->priority * 10 + 10;
        p_proc->status = INIT;

        change_proc_list(INIT, READY, p_proc);

		p_proc->nr_tty = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;

	}

    // disp_str("test");

    proc_table[1].nr_tty = 0;
    proc_table[2].nr_tty = 2;
    proc_table[3].nr_tty = 2;
    proc_table[4].nr_tty = 2;
    proc_table[5].nr_tty = 1;

	change_proc_list(READY, RUNNING, proc_table);
    // show_ready_list();
    // disp_str("kernel_main\n");

	init_clock();
    init_keyboard();

    k_reenter = 0;
    ticks = 0;

	restart();

	while(1){}
}

/*           Terminal         */


char *status_str(int status){
    if(status == RUNNING){
        return "RUNNING";
    }else if(status == READY){
        return "READY";
    }else if(status == WAITING){
        return "WAITING";
    }
    return NULL;
}

char *prio_str(int prio){
    if(prio == IDLE){
        return "IDLE";
    }else if(prio == LOW){
        return "LOW";
    }else if(prio == MEDIUM){
        return "MEDIUM";
    }else if(prio == HIGH){
        return "HIGH";
    }else if(prio == REALTIME){
        return "REALTIME";
    }
    return NULL;
}

void ps(){
    int i;
    for(i = 0; i<NR_TASKS + NR_PROCS; ++i){
        printf("%s: %s\n", proc_table[i].p_name, status_str(proc_table[i].status), prio_str(proc_table[i].priority));
    }
}

void dispatch_command(char *command){
    // disp_str("dispatch_command");
    strlwr(command);
    if(strcmp(command, "clear")==0){
        clearScreen();
        sys_clear(tty_table);

    }else if(strcmp(command, "ps")==0){
        ps();

    }else if(strcmp(command, "exit")==0){
        displayGoodBye();
        while(1);
    }else{

        printf("bad command\n");
    }
}

PUBLIC void Terminal(){
    tty_table->b_scanf = FALSE;
    while(1){
        printf("$ ");
        scanf_on(tty_table);
        while(tty_table->b_scanf);
        dispatch_command(tty_table->str);
    }
}


/*             Calculator           */

stack calStack;
char calStr[STACK_DEFAULT_SIZE];
int p_str;

boolean isdigit(ch){
    return ch<='9' && ch >='0';
}

int isp(char ch){
    if(ch == '+')return 1;
    if(ch == '-')return 2;
    if(ch == '*')return 3;
    if(ch == '/')return 4;
    if(ch == '('|| ch == ')')return 0;
    if(ch == '#')return -1;
}

void push(stack *sk, char ch){
    sk->s[sk->top++]=ch;
    assert(sk->top<STACK_DEFAULT_SIZE);
}

char top(stack *sk){
    assert(top!=0);
    return sk->s[sk->top-1];
}

char pop(stack *sk){
    assert(top!=0);
    return sk->s[--sk->top];
}

void init_stack(stack *sk){
    sk->top = 0;
}

void show_stack(){
    int i;
    for(i = 0 ; i < calStack.top; ++i){
        printf("%c", calStack.s[i]);
    }
    printf("\n");
}

char *postfix(char *str){
    init_stack(&calStack);
    char *p = str;
    char *q = calStr;
    push(&calStack, '#');
    while(calStack.top!=0&&*p!='#'){
        if(isdigit(*p)){
            *q++=*p++;
        }
        else{
            char op = top(&calStack);
            if(isp(op)<isp(*p))
                push(&calStack, *p++);
            else if(isp(op) > isp(*p) && *p!='('){
                *q++ = pop(&calStack);
                
            }else{
                char op = top(&calStack);
                if(op == '(')p++;
                else *q++=pop(&calStack);
            }
        }
    }
    while(calStack.top!=0){
        // printf("%c\n", top(&calStack));
        *q++=pop(&calStack);
    }
    *q = '\0';
    return calStr;
}

void do_operator(char op){
    int right = pop(&calStack);
    int left = pop(&calStack);
    // printf("%d %d\n", left, right);
    int value;
    switch(op){
        case '+':
            value = left + right;
            push(&calStack, value);
            break;
        case '-':
            value = left - right;
            push(&calStack, value);
            break;
        case '*':
            value = left * right;
            push(&calStack, value);
            break;
        case '/':
            if(right == 0){
                printf("divide by 0!\n");
            }
            else{
                value = left / right;
                push(&calStack, value);
            }
            break;
    }
}

void calculate(char *str){
    char *s = postfix(str);
    // printf("%s\n", s);
    // while(1);
    p_str = 0;
    init_stack(&calStack);
    char *p = s;
    while(*p!='#'){
        switch(*p){
            case '+':
            case '-':
            case '*':
            case '/':
                do_operator(*p++);
                break;
            default:
                push(&calStack, (*p-'0'));
                p++;
                // printf("stack top: %d\n", top(&calStack));
        }
    }
    printf("%d\n", top(&calStack));
}

PUBLIC void Calculator(){
    printf("This is a Calculator application\n");
    printf("The expression must end with #\n");
    TTY *p_tty = tty_table + 1;
    while(1){
        printf("> ");
        scanf_on(p_tty);
        while(p_tty->b_scanf);
        // printf("%s\n", p_tty->str);
        calculate(p_tty->str);
    }
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



void displayWelcome()
{
    clearScreen();
    
    disp_str("=============================================================================\n");
    disp_str("                            Welcome To ShabbyOS\n");
    disp_str("                                  Made By                       \n");
    disp_str("                        Lvjinhua           Yisiqi          \n");
    disp_str("=============================================================================\n");
}


void displayGoodBye()
{

    clearScreen();
    disp_str("\n\n\n\n\n");
    disp_color_str("             #####                             ######               \n", 0x1);     
    disp_color_str("            #     #  ####   ####  #####        #     # #   # ###### \n", 0x1); 
    disp_color_str("            #       #    # #    # #    #       #     #  # #  #      \n", 0x2); 
    disp_color_str("            #  #### #    # #    # #    #       ######    #   #####  \n", 0x2); 
    disp_color_str("            #     # #    # #    # #    #       #     #   #   #      \n", 0x3); 
    disp_color_str("            #     # #    # #    # #    #       #     #   #   #      \n", 0x3); 
    disp_color_str("             #####   ####   ####  #####        ######    #   ###### \n", 0x3);
    disp_str("\n\n\n");     
    disp_color_str("         ------------------------- Made BY ---------------------------\n\n",0xB);  
    disp_color_str("              ------------------Lvjinhua  147- ---------------\n\n",0xF);  
    disp_color_str("         -------------------------Goodbye-----------------------------\n\n",0xD);
}
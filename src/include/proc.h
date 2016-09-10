
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


typedef struct s_stackframe {	/* proc_ptr points here				↑ Low			*/
	u32	gs;		/* ┓						│			*/
	u32	fs;		/* ┃						│			*/
	u32	es;		/* ┃						│			*/
	u32	ds;		/* ┃						│			*/
	u32	edi;		/* ┃						│			*/
	u32	esi;		/* ┣ pushed by save()				│			*/
	u32	ebp;		/* ┃						│			*/
	u32	kernel_esp;	/* <- 'popad' will ignore it			│			*/
	u32	ebx;		/* ┃						↑栈从高地址往低地址增长*/		
	u32	edx;		/* ┃						│			*/
	u32	ecx;		/* ┃						│			*/
	u32	eax;		/* ┛						│			*/
	u32	retaddr;	/* return address for assembly code save()	│			*/
	u32	eip;		/*  ┓						│			*/
	u32	cs;		/*  ┃						│			*/
	u32	eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
	u32	esp;		/*  ┃						│			*/
	u32	ss;		/*  ┛						┷High			*/
}STACK_FRAME;


//进程状态
enum proc_status{
	READY = 0,           //就绪
	WAITING,             //等待
	RUNNING,             //运行
	INIT,                //创建
	DEAD				 //消亡
};

//进程优先级
enum proc_priority{
	IDLE = 0,            //懒惰
	LOW = 1,             //低
	MEDIUM = 2,          //中
	HIGH = 3,            //高
	REALTIME = 4        //实时
};



typedef struct s_proc {
	STACK_FRAME regs;          /* process registers saved in stack frame */

	u16 ldt_sel;               /* gdt selector giving ldt base and limit */
	DESCRIPTOR ldts[LDT_SIZE]; /* local descriptors for code and data */

    int ticks;                 /* remained ticks */
    int priority;

    int status;      		   /* proc_status */

	u32 pid;                   /* process id passed in from MM */
	char p_name[16];           /* name of the process */

	int nr_tty;
}PROCESS;

typedef PROCESS proc;

typedef struct s_task {
	task_f	initial_eip;
	int	stacksize;
	char	name[32];
}TASK;


//进程链表
typedef struct proc_node
{
	proc * kproc;        				//the process body;
	struct proc_node * prev;		 	//the previous process in the list
	struct proc_node * next;		 	//the next process in the list
}proc_node;




/* Number of tasks & procs */
#define NR_TASKS	1
#define NR_PROCS	3

/* stacks of tasks */
#define STACK_SIZE_TTY		0x8000
#define STACK_SIZE_TESTA	0x8000
#define STACK_SIZE_TESTB	0x8000
#define STACK_SIZE_TESTC	0x8000

#define STACK_SIZE_TOTAL	(STACK_SIZE_TTY + \
				STACK_SIZE_TESTA + \
				STACK_SIZE_TESTB + \
				STACK_SIZE_TESTC)


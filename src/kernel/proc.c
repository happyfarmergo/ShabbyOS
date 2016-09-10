
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

EXTERN proc_node *h_ready, *h_waiting;
EXTERN proc_node proc_list[];
EXTERN int index_free;


/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{

	// PROCESS* p;
	// int	 greatest_ticks = 0;

	// while (!greatest_ticks) {
	// 	for (p = proc_table; p < proc_table+NR_TASKS+NR_PROCS; p++) {
	// 		if (p->ticks > greatest_ticks) {
	// 			greatest_ticks = p->ticks;
	// 			p_proc_ready = p;
	// 		}
	// 	}

	// 	if (!greatest_ticks) {
	// 		for(p=proc_table;p<proc_table+NR_TASKS+NR_PROCS;p++) {
	// 			p->ticks = p->priority;
	// 		}
	// 	}
	// }

	proc_node* p = h_ready;
	proc *tar = NULL;
	int prio = 0;
	while(p){
		if(p->kproc->priority > prio){
			prio = p->kproc->priority;
			tar = p->kproc;
		}
		p = p->next;
	}

	if(tar == NULL){//就绪队列空
		//继续跑当前进程
	}
	else{			//找到优先级最高的进程
		disp_str(tar->p_name);
		change_proc_list(RUNNING, READY, p_proc_ready);//切换到就绪队列尾部
		change_proc_list(READY, RUNNING, tar);         
	}

	p_proc_ready->ticks = 200/(p_proc_ready->priority);
	
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}



//add

//添加node后更新index_free
void update_proc_index(){
	int i;
	for(i = index_free + 1; i<MAX_PROCS && proc_list[i].kproc != NULL; ++i)
		;
	assert(i < MAX_PROCS);
	index_free = i;
}


proc_node *remove_proc_node(proc_node **head, proc *p){
	assert(*head != NULL);

	proc_node *tar = *head;
	while(tar && tar->kproc != p){
		tar = tar->next;
	}
	assert(tar != NULL);

	if(tar == *head){
		*head = tar->next;
		tar->next->prev = NULL;
		// disp_str("remove at head\n");
	}
	else{
		tar->prev->next = tar->next;
		tar->next->prev = tar->prev;
		// disp_str("remove in body\n");
	}

	tar->prev = tar->next = tar->kproc = NULL;

	return tar;
}

proc_node *add_proc_node(proc_node **head, proc *p){

	// disp_str(p->p_name);

	proc_node *tar = proc_list + index_free;
	tar->kproc = p;

	// disp_int(tar - proc_list);

	if(*head == NULL){
		*head = tar;
		// disp_str("\nhead is null\n");
	}
	else{
		proc_node *tail = *head;
		while(tail->next){
			tail = tail->next;
		}
		tail->next = tar;
		tar->prev = tail;
		// disp_str("add to tail\n");
	}

	//更新index_free
	update_proc_index();

	return tar;
}

//改变进程状态 对象：list_proc
void change_proc_list(int pre_status, int next_status, proc * p){
	assert(pre_status == p->status);
	assert(pre_status != next_status);

	switch(pre_status){
		case INIT:
			assert(next_status == READY);
			add_proc_node(&h_ready, p);

			break;
		case RUNNING:
			assert(next_status == READY || next_status == WAITING);
			p_proc_ready = NULL;

			proc_node **head = next_status == WAITING?&h_waiting:&h_ready;
			add_proc_node(head, p);

			break;
		case READY:
			assert(next_status == RUNNING);

			printf("start remove");
			remove_proc_node(&h_ready, p);

			p_proc_ready = p;

			break;
		case WAITING:
			assert(next_status == READY);
			remove_proc_node(&h_waiting, p);

			add_proc_node(&h_ready, p);

			break;
		default:
			panic("change_proc_list error");
	}
	p->status = next_status;
}


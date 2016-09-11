
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

EXTERN proc_node *h_ready[], *h_waiting;
EXTERN proc_node proc_list[];
EXTERN int index_free;
EXTERN int k;
/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	k++;
	proc * tar = select_one_proc();
	
	if(tar == NULL){//就绪队列空
		init_proc();
		// disp_str("\nrun all proc");
		tar = select_one_proc();
	}
	//找到优先级最高的进程
	// disp_str(tar->p_name);	
	// disp_int(index_free);
	change_proc_list(RUNNING, READY, p_proc_ready);//切换到就绪队列
	// disp_int(index_free);

	change_proc_list(READY, RUNNING, tar);         
	// disp_int(index_free);

	// disp_str(p_proc_ready->p_name);
	// disp_str("\n");
	// if(k<=4){
	// 	disp_int(k);
	// 	show_ready_list();

	// }
}

PUBLIC void block(){
	proc * tar = select_one_proc();
	
	if(tar == NULL){//就绪队列空
		init_proc();
		// disp_str("\nrun all proc");
		tar = select_one_proc();
	}
	disp_str(tar->p_name);
	change_proc_list(RUNNING, WAITING, p_proc_ready);//切换到队列
	change_proc_list(READY, RUNNING, tar);
	show_waiting_list();   
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}



//add

void init_proc(){
	// disp_str("init_proc \n");
	int i;
	for(i = REALTIME; i >= IDLE; --i){
		proc_node * p = h_ready[i];
		while(p!=NULL){
			p->kproc->ticks = p->kproc->priority * 10 + 10;
			p = p->next;
		}
	}
}


proc *select_one_proc(){
	proc_node* p = NULL;
	proc *tar = NULL;
	int i = 0;
	for(i = REALTIME; i>=IDLE; --i){
		p = h_ready[i];

		while(p!=NULL){
			if(p->kproc->ticks > 0){
				tar = p->kproc;
				return tar;
			}
			p = p->next;
		}
	}

	return NULL;

}

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
		// disp_str(tar->kproc->p_name);
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
	index_free = tar->index;

	// disp_str("remove: ");
	// disp_int(index_free);
	return tar;
}

proc_node *add_proc_node(proc_node **head, proc *p){

	// disp_str(p->p_name);

	proc_node *tar = proc_list + index_free;
	tar->kproc = p;

	// disp_int(tar - proc_list);

	if(*head == NULL){
		*head = tar;
		tar->prev = tar->next = NULL;
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
	assert(p->priority>=IDLE && p->priority <=REALTIME);

	switch(pre_status){
		case INIT:
			assert(next_status == READY);
			add_proc_node(&h_ready[p->priority], p);

			break;
		case RUNNING:
			assert(next_status == READY || next_status == WAITING);
			p_proc_ready = NULL;

			proc_node **head = next_status == WAITING?&h_waiting:&h_ready[p->priority];
			add_proc_node(head, p);

			break;
		case READY:
			assert(next_status == RUNNING);

			// printf("start remove");
			remove_proc_node(&h_ready[p->priority], p);

			p_proc_ready = p;

			break;
		case WAITING:
			assert(next_status == READY);
			remove_proc_node(&h_waiting, p);

			add_proc_node(&h_ready[p->priority], p);

			
			break;
		default:
			panic("change_proc_list error");
	}
	p->status = next_status;
}

PUBLIC void wake(proc *p_proc){
	disp_str("WAITING to READY: \n");
			show_ready_list();
			disp_str("///////////////");
	change_proc_list(WAITING, READY, p_proc);
}
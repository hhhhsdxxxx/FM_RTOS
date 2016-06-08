/*
 * created by Henry Huang
 * thread.c
 * FM LAB
*/

#include <kernel/fm_stdtype.h>
#include "fm_thread.h"
#include "cpu_m3.h"
#include "fm_timer.h"
#include "fm_mem.h"
#include "fm_service.h"
#include "fm_config.h"
#include "fm_log.h"

#ifndef _USE_FM_MEM_
#include <stdlib.h>
#endif



extern void _fm_thread_context_switch_(fm_uint32_t from, fm_uint32_t to);
extern void _fm_thread_start_(fm_uint32_t to);
extern void _fm_interrupt_enable_(fm_uint32_t);
extern fm_uint32_t _fm_interrupt_disable_(void);

/*
 * 对内使用函数声明
 */
static fm_tcb *_highest_priority_thread(void);
//static void _find_thread(fm_tcb*, fm_tcb**, fm_tcb**, fm_bool*);
static void _push_ready(fm_tcb_ready_queue *, fm_tcb *, fm_thread_priority);
static void _pop_ready(fm_tcb_ready_queue *, fm_tcb *, fm_thread_priority);
//static void _push_wait(fm_tcb_queue *, fm_tcb *, fm_thread_priority);
//static void _pop_wait(fm_tcb_queue *, fm_tcb *, fm_thread_priority);
void enable_schedule(void);
void disable_schedule(void);
void system_thread(void);
void exit_thread(void);
static fm_bool tcb_init(fm_tcb *, void (*)(void), fm_thread_priority, fm_stack_size);
static void add_safe_thread(void);
static void init_ready_queue(fm_tcb_ready_queue *);
static void init_wait_queue(fm_tcb_queue *);
void fm_thread_from_ready_to_wait(fm_tcb *, fm_tcb_ready_queue *, fm_tcb_queue *);
void fm_thread_from_wait_to_ready(fm_tcb *, fm_tcb_ready_queue *, fm_tcb_queue *);
//void fm_thread_from_wait_to_wait(fm_tcb *, fm_tcb_queue *, fm_thread_priority, fm_thread_priority);
static void _push_list(fm_thread_list *, fm_tcb *);
static void _pop_list(fm_thread_list *, fm_tcb *);
static void _push_wait(fm_tcb_queue *, fm_tcb *);
static void _pop_wait(fm_tcb_queue *, fm_tcb *);
//static fm_bool is_available(fm_thread_t *);

fm_uint32_t fm_thread_from_address, fm_thread_to_address;
fm_thread_number global_threads_number = 0;
fm_tcb *usr_run;
fm_bool is_start = 0;


/*
 * 避免PendSV Handler重复进入
 */
fm_bool fm_thread_interrupt_flag = 0;

static fm_tcb_queue_manager sys_thread_mgr;

fm_tcb_queue_manager *sys_control(void){
	return &sys_thread_mgr;
}

fm_tcb* running_thread(void){
	return usr_run;
}

/* assistant table */
fm_thread_priority enum_table_for_priority[256] =
	{    
		0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};
static fm_thread_priority reverse_table_for_priority[8] = 
	{
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
	};

/* find the highest priority in ready queue */
static fm_tcb *_highest_priority_thread(void){
	int i;
	for(i = 0; i < 4; ++i){
		if(sys_thread_mgr.ready_queue.priority_state[i])
			break;
	}
	if(i > 3){
		return NULL;
	}
	fm_thread_priority priority = (i << 3) + enum_table_for_priority[sys_thread_mgr.ready_queue.priority_state[i]];
	return sys_thread_mgr.ready_queue.head[priority];
}

/* push ready queue */
static void _push_ready(fm_tcb_ready_queue *queue, fm_tcb *ptr, fm_thread_priority priority){
	ptr->status = READY;
#if READY_QUEUE_TYPE == 0
	if(queue->head[priority] == NULL){
		queue->head[priority] = ptr;
		ptr->next_thread = ptr->prev_thread = ptr;
		queue->priority_state[priority>>3] |= reverse_table_for_priority[priority&0x7];
	} else {
		ptr->next_thread = queue->head[priority];
		ptr->prev_thread = queue->head[priority]->prev_thread;
		queue->head[priority]->prev_thread->next_thread = ptr;
		queue->head[priority]->prev_thread = ptr;
	}
#elif READY_QUEUE_TYPE == 1
	if(queue->head[priority] == NULL){
		queue->head[priority] = queue->tail[priority] = ptr;
		queue->priority_state[priority>>3] |= reverse_table_for_priority[priority&0x7];
	} else {
		ptr->prev_thread = queue->tail[priority];
		queue->tail[priority]->next_thread = ptr;
		queue->tail[priority] = ptr;
	}
#endif
}

/* pop ready queue */
static void _pop_ready(fm_tcb_ready_queue *queue, fm_tcb *ptr, fm_thread_priority priority){
#if READY_QUEUE_TYPE == 0
	if(ptr->next_thread == ptr){
		queue->head[priority] = NULL;
		queue->priority_state[priority>>3] &= ~reverse_table_for_priority[(fm_uint32_t)priority&(fm_uint32_t)0x07];
	} else {
		ptr->prev_thread->next_thread = ptr->next_thread;
		ptr->next_thread->prev_thread = ptr->prev_thread;
		if(queue->head[priority] == ptr){
			queue->head[priority] = ptr->next_thread;
		}
	}
#elif READY_QUEUE_TYPE == 1
	if(queue->head[priority] == queue->tail[priority]){
		queue->head[priority] = queue->tail[priority] = NULL;
		queue->priority_state[priority>>3] &= ~reverse_table_for_priority[(fm_uint32_t)priority&(fm_uint32_t)0x07];
	} else {
		if(ptr->next_thread){
			ptr->next_thread->prev_thread = ptr->prev_thread;
		} else {
			queue->tail[priority] = ptr->prev_thread;
		}
		if(ptr->prev_thread){
			ptr->prev_thread->next_thread = ptr->next_thread;
		} else {
			queue->head[priority] = ptr->next_thread;
		}
	}
	ptr->prev_thread = ptr->next_thread = NULL;
#endif
}

static void _push_wait(fm_tcb_queue *queue, fm_tcb *ptr){
#if WAIT_QUEUE_TYPE == 0
	if(queue->head == NULL){
		queue->head = ptr;
		ptr->prev_thread = ptr->next_thread = ptr;
	} else {
		ptr->next_thread = queue->head;
		ptr->prev_thread = queue->head->prev_thread;
		queue->head->prev_thread->next_thread = ptr;
		queue->head->prev_thread = ptr;
	}
#elif WAIT_QUEUE_TYPE == 1
	if(queue->head == NULL){
		queue->head = ptr;
	} else {
		queue->tail->next_thread = ptr;
	}
	ptr->prev_thread = queue->tail;
	queue->tail = ptr;
#endif
}

static void _pop_wait(fm_tcb_queue *queue, fm_tcb *ptr){
#if WAIT_QUEUE_TYPE == 0
	if(ptr->next_thread == ptr){
		queue->head =  NULL;
	} else {
		ptr->prev_thread->next_thread = ptr->next_thread;
		ptr->next_thread->prev_thread = ptr->prev_thread;
		if(queue->head == ptr){
			queue->head = ptr->next_thread;
		}
	}
#elif WAIT_QUEUE_TYPE == 1
	if(ptr->prev_thread == NULL){
		queue->head = ptr->next_thread;
	} else {
		ptr->prev_thread->next_thread = ptr->next_thread;
	}
	if(ptr->next_thread == NULL){
		queue->tail = ptr->prev_thread;
	} else {
		ptr->next_thread->prev_thread = ptr->prev_thread;
	}
	ptr->next_thread = ptr->prev_thread = NULL;
#endif
}

void enable_schedule(void){
	sys_thread_mgr.schedule_state = THREAD_SCHEDULE_ENABLE;
}

void disable_schedule(void){
	sys_thread_mgr.schedule_state = THREAD_SCHEDULE_DISABLE;
}

/* when no thread run 
** priority 31
** do nothing
*/
void system_thread(void){
	fm_print_log("Safe Thread\r\n");
	while(1){
	}
}

/* thread function first run */
void exit_thread(void){
	/* thread exit */
	fm_uint32_t level = _fm_interrupt_disable_();
	usr_run->stack_top = 0;
	usr_run->priority = usr_run->init_priority;
	fm_thread_priority priority = usr_run->priority & 0x1f;
	/* delete from ready queue */
	_pop_ready(&(sys_thread_mgr.ready_queue), usr_run, priority);
	/* put into sleep */
	usr_run->status = SLEEP;
	_push_wait(&(sys_thread_mgr.sleep_queue) ,usr_run);
	usr_run = NULL;
	_fm_interrupt_enable_(level);
	fm_thread_schedule();
}

static fm_bool tcb_init(fm_tcb *p, void (*entry)(void), fm_thread_priority priority, fm_stack_size stack_size){
	p->tid = global_threads_number++;
	p->status = READY;

	p->priority = priority;
	p->init_priority = priority;

	p->entry = entry;
//	p->is_busy = 0;

	p->next_thread = NULL;
	p->prev_thread = NULL;
	p->sem_next_thread = NULL;
	p->sem_prev_thread = NULL;

#ifdef _USE_FM_THREAD_SLEEP_
	p->time_next_thread = NULL;
	p->time_prev_thread = NULL;
	p->ticks = 0;
#endif

#ifdef _USE_FM_MAIL_
	p->mailbox.head = NULL;
	p->mailbox.tail = NULL;
#endif

	p->stack_size = (fm_uint16_t)((fm_uint32_t)stack_size & 0xffff) & (~((fm_uint32_t)0x7));
	p->stack_start = fm_malloc(p->stack_size);
	if(p->stack_start == NULL){
		return 0;
	}
	p->stack_top = fm_stack_init((void*)entry, (fm_address)(p->stack_start)+p->stack_size, (void*)exit_thread);
	return 1;
}

static void add_safe_thread(void){
	fm_tcb *p = (fm_tcb*)fm_object_malloc(type_fm_tcb);
	if(p == NULL){
		return;
	}
	tcb_init(p, system_thread, 31, 256);
	_push_ready(&(sys_thread_mgr.ready_queue), p, p->priority);
}

static void init_ready_queue(fm_tcb_ready_queue *queue){
	fm_memset(&(queue->priority_state[0]), 0, (PRIORITY_SIZE>>3));
	fm_memset(&(queue->head[0]), 0, PRIORITY_SIZE*sizeof(fm_tcb*));
#if READY_QUEUE_TYPE == 1
	fm_memset(&(queue->tail[0]), 0, PRIORITY_SIZE*sizeof(fm_tcb*));
#endif
}

static void init_wait_queue(fm_tcb_queue *queue){
	queue->head = NULL;
#if WAIT_QUEUE_TYPE == 1
	queue->tail = NULL;
#endif
}

/*
 * 初始化,在create thread之前调用
 */
void fm_thread_init(){
	sys_thread_mgr.schedule_state = THREAD_SCHEDULE_ENABLE;
	usr_run = NULL;
	init_ready_queue(&(sys_thread_mgr.ready_queue));
	init_wait_queue(&(sys_thread_mgr.sleep_queue));
	init_wait_queue(&(sys_thread_mgr.wait_queue));
	add_safe_thread();
}


void fm_thread_system_start(){
	fm_tcb *to_thread = _highest_priority_thread();
	is_start = 1;
	if(usr_run != to_thread){
		usr_run = to_thread;
		usr_run->status = RUN;
		_fm_thread_start_((fm_uint32_t)(&(to_thread->stack_top)));
	}

}

/*
 * 任务调度
 */
void fm_thread_schedule(){
	if(sys_thread_mgr.schedule_state == THREAD_SCHEDULE_DISABLE)
		return;
	/* need schedule */
	fm_uint32_t level = _fm_interrupt_disable_();
	fm_tcb *the_highest = _highest_priority_thread();
	if(the_highest == NULL){
		return;
	}
	if(usr_run != the_highest){
		fm_address *from = 0, *to = 0;
		if(usr_run != NULL){
//			usr_run->status = READY;
			usr_run->status = usr_run->status == RUN ? READY : usr_run->status;
			from = &(usr_run->stack_top);
		}
		usr_run = the_highest;
		usr_run->status = RUN;
		to = &(usr_run->stack_top);
		_fm_thread_context_switch_((fm_uint32_t)from, (fm_uint32_t)to);
		/* go_to_new thread*/
	}
	_fm_interrupt_enable_(level);
}

/* create a new thread without args */
fm_bool fm_thread_create(fm_thread_t *target, void (*thread_entry)(void), fm_stack_size stack_size, fm_thread_priority priority){
	/* alloc space */
	if(target == NULL || thread_entry == NULL){
		return CREATE_THREAD_FAILED;
	}
	if(!stack_size){
		stack_size = (fm_uint16_t)DEFAULT_STACK_SIZE;
	}
	fm_tcb *p = (fm_tcb*)fm_object_malloc(type_fm_tcb);
	if(p == NULL){
		return CREATE_THREAD_FAILED;
	}
	/* initial */
	if(!tcb_init(p, thread_entry, priority, stack_size)){
		fm_free(p);
		return CREATE_THREAD_FAILED;
	}
	/* put in ready queue */
	_push_ready(&(sys_thread_mgr.ready_queue), p, priority);
	*target = p;
	return CREATE_THREAD_SUCCEED;
}

void fm_thread_from_ready_to_ready(fm_tcb *target, fm_thread_priority old_priority, fm_thread_priority new_priority){
	fm_tcb_ready_queue *queue = &(sys_thread_mgr.ready_queue);
	_pop_ready(queue, target, old_priority);
	_push_ready(queue, target, new_priority);
	target->priority = new_priority;
}

//void fm_thread_from_wait_to_wait(fm_tcb *target, fm_tcb_queue *queue, fm_thread_priority old_priority, fm_thread_priority new_priority){
//	_pop_wait(queue, target, old_priority);
//	_push_wait(queue, target, new_priority);
//}

void fm_thread_from_ready_to_wait(fm_tcb *target, fm_tcb_ready_queue *rqueue, fm_tcb_queue *wqueue){
	/* delete from ready queue */
	_pop_ready(rqueue, target, target->priority);
	/* put into wait queue */
	target->status = WAIT;
	_push_wait(wqueue, target);
}

void fm_thread_from_wait_to_ready(fm_tcb *target, fm_tcb_ready_queue *rqueue, fm_tcb_queue *wqueue){
	/* check sem */
	/* delete from wait queue */
	_pop_wait(wqueue, target);
	/* put into ready queue */
	_push_ready(rqueue, target, target->priority);
}

void fm_thread_wake(fm_thread_t *target){
	if(target == NULL || (*target)->status != SLEEP){
		return;
	}
	fm_uint32_t level = _fm_interrupt_disable_();
	(*target)->status = READY;
	(*target)->stack_top = fm_stack_init((void*)((*target)->entry), (fm_address)((*target)->stack_start)+(*target)->stack_size-1, (void*)exit_thread);
	fm_thread_from_wait_to_ready(*target, &(sys_thread_mgr.ready_queue), &(sys_thread_mgr.wait_queue));
	_fm_interrupt_enable_(level);
	fm_thread_schedule();
}

/* suspend */
void fm_thread_suspend(fm_thread_t *target){
	/* find thread */
	fm_uint32_t level = _fm_interrupt_disable_();
	if(target == NULL || !((*target)->status & (RUN | READY))){
		_fm_interrupt_enable_(level);
		return;
	}
	(*target)->status = WAIT;
	fm_thread_from_ready_to_wait(*target, &(sys_thread_mgr.ready_queue), &(sys_thread_mgr.wait_queue));
	_fm_interrupt_enable_(level);
	fm_thread_schedule();
}

/* thread suspend self */
void fm_thread_suspend_self(void){
	fm_thread_suspend(&usr_run);
}

/* wake up */
void fm_thread_resume(fm_thread_t *target){
	/* find thread */
	fm_uint32_t level = _fm_interrupt_disable_();
	if(target == NULL || !((*target)->status & WAIT)){
		fm_print_log("Fail\r\n");
		char buf[16];
		fm_snprintf(buf, 16, "RS %x\r\n", *target);
		fm_print_log(buf);
		_fm_interrupt_enable_(level);
		return;
	}
	(*target)->status = READY;
	if((*target)->ticks){
		(*target)->ticks = 0;
		_pop_list(&(sys_thread_mgr.timer_queue), *target);
	}
	fm_thread_from_wait_to_ready(*target, &(sys_thread_mgr.ready_queue), &(sys_thread_mgr.wait_queue));
	_fm_interrupt_enable_(level);
	fm_thread_schedule();
}

#ifdef _USE_FM_THREAD_CHANGE_PRIORITY_

void fm_thread_change_priority(fm_thread_t *target, fm_thread_priority new_priority){
	/* find in the ready queue */
	if(!target){
		return;
	}
	fm_uint32_t level = _fm_interrupt_disable_();
	fm_bool in_ready = 0;
	switch((*target)->status){
	case RUN:
	case READY:
		in_ready = 1;
		fm_thread_from_ready_to_ready(*target, (*target)->priority, new_priority);
		(*target)->init_priority = new_priority;
		break;
	case WAIT:
//		fm_thread_from_wait_to_wait(*target, &(sys_thread_mgr.wait_queue), (*target)->priority, new_priority);
//		(*target)->priority = (*target)->init_priority = new_priority;
//		break;
	case SLEEP:
//		fm_thread_from_wait_to_wait(*target, &(sys_thread_mgr.sleep_queue), (*target)->priority, new_priority);
		(*target)->priority = (*target)->init_priority = new_priority;
		break;
	default:
		break;
	}
	_fm_interrupt_enable_(level);
	if(in_ready){
		fm_thread_schedule();
	}
}

void fm_thread_change_priority_self(fm_thread_priority new_priority){
	fm_thread_change_priority(&(usr_run), new_priority);
}

#endif

static void _push_list(fm_thread_list *list, fm_tcb *ptr){
	if(list->head == NULL){
		list->head = ptr;
	} else {
		list->tail->time_next_thread = ptr;
	}
	ptr->time_prev_thread = list->tail;
	list->tail = ptr;
}

static void _pop_list(fm_thread_list *list, fm_tcb *ptr){
	if(ptr->time_prev_thread == NULL){
		list->head = ptr->time_next_thread;
	} else {
		ptr->time_prev_thread->time_next_thread = ptr->time_next_thread;
	}
	if(ptr->time_next_thread == NULL){
		list->tail = ptr->time_prev_thread;
	} else {
		ptr->time_next_thread->time_prev_thread = ptr->time_prev_thread;
	}
	ptr->time_next_thread = ptr->time_prev_thread = NULL;
}

void fm_thread_sleep(fm_uint32_t ticks){
	fm_uint32_t level = _fm_interrupt_disable_();
	usr_run->ticks = ticks;
	_push_list(&(sys_thread_mgr.timer_queue), usr_run);
	fm_thread_suspend(&usr_run);
	_fm_interrupt_enable_(level);
}

void irq_thread_timer(void){
	fm_tcb *p;
	fm_bool need_schedule;
	fm_uint32_t level = _fm_interrupt_disable_();
	need_schedule = 0;
	p = sys_thread_mgr.timer_queue.head;
	disable_schedule();
	while(p){
		fm_tcb *tmp = p->time_next_thread;
		--(p->ticks);
		if(!p->ticks){
			_pop_list(&(sys_thread_mgr.timer_queue), p);
			fm_thread_resume(&p);
			need_schedule = 1;
		}
		p = tmp;
	}
	enable_schedule();
	if(need_schedule){
		fm_thread_schedule();
	}
	_fm_interrupt_enable_(level);
}

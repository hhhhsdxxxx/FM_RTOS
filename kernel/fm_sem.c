/*
 * created by Henry Huang
 * FM LAB
 * fm_sem.c
 */

#include <kernel/fm_stdtype.h>
#include "fm_config.h"
#include "fm_mem.h"
#include "fm_sem.h"
#include "fm_thread.h"

#ifndef _USE_FM_MEM_SERVICE_
#include <string.h>
#endif

#ifdef _USE_FM_SEM_
static fm_scb_queue fm_sem_que;
#endif
#ifdef _USE_FM_SEM_
static fm_mutex_queue fm_mutex_que;
#endif

#if defined(_USE_FM_SEM_) || defined(_USE_FM_MUTEX_)
fm_bool need_schedule = 1;
static void _find_highest_priority(fm_tcb **, fm_tsb_queue *);
static void _remove(fm_tcb *, fm_tsb_queue *);
static void _insert(fm_tcb *, fm_tsb_queue *);
extern fm_tcb *usr_run;
extern void enable_schedule(void);
extern void disable_schedule(void);
extern fm_uint32_t _fm_interrupt_disable_(void);
extern void _fm_interrupt_enable_(fm_uint32_t);
extern void fm_thread_schedule(void);

/* find a highest priority thread in tsb queue */
static void _find_highest_priority(fm_tcb **result, fm_tsb_queue *src){
	*result = src->head;
	fm_tcb *ptr = src->head->sem_next_thread;
	while(ptr != NULL){
		if(ptr->priority < (*result)->priority){
			*result = ptr;
		}
		ptr = ptr->sem_next_thread;
	}
}

/* delete from tsb queue */
static void _remove(fm_tcb *result, fm_tsb_queue *src){
	if(result->sem_prev_thread == NULL){
		src->head = result->sem_next_thread;
	} else {
		result->sem_prev_thread->sem_next_thread = result->sem_next_thread;
	}
	if(result->sem_next_thread == NULL){
		src->tail = result->sem_prev_thread;
	} else {
		result->sem_next_thread->sem_prev_thread = result->sem_prev_thread;
	}
	result->sem_next_thread = result->sem_prev_thread = NULL;
}

static void _insert(fm_tcb *result, fm_tsb_queue *src){
	if(result == NULL){
		return;
	}
	if(src->head == NULL){
		src->head = result;
	} else {
		src->tail->sem_next_thread = result;
	}
	result->sem_prev_thread = src->tail;
	src->tail = result;
}

/* initial */
#ifndef _COMPILER_SUPPORT_STATIC_

void fm_sem_mutex_init(void){
#ifdef _USE_FM_SEM_
	fm_sem_que.head = fm_sem_que.tail = NULL;
#endif

#ifdef _USE_FM_MUTEX_
	fm_mutex_que.head = fm_mutex_que.tail = NULL;
#endif
}
#endif

#endif



#ifdef _USE_FM_SEM_

static void _insert_sem_system_list(fm_scb_queue *, _fm_sem_t *);
static void _remove_sem_system_list(fm_scb_queue *, _fm_sem_t *);

static void _insert_sem_system_list(fm_scb_queue *queue, _fm_sem_t *sem){
	if(queue->head == NULL){
		queue->head = sem;
	} else {
		queue->tail->next = sem;
	}
	sem->prev = queue->tail;
	queue->tail = sem;
}

static void _remove_sem_system_list(fm_scb_queue *queue, _fm_sem_t *sem){
	if(sem->prev == NULL){
		queue->head = sem->next;
	} else {
		sem->prev->next = sem->next;
	}
	if(sem->next == NULL){
		queue->tail = sem->prev;
	} else {
		sem->next->prev = sem->prev;
	}
	sem->prev = sem->next = NULL;
}

void fm_sem_init(fm_scb *sem, fm_sem_cnt init_count, fm_sem_cnt max_count){
	sem->count = init_count;
	sem->max_count = max_count;
	sem->wait_queue.head = sem->wait_queue.tail = NULL;
}

/*
 * semaphore section
 * fm_sem_create
 * fm_sem_delete
 * fm_sem_request
 * fm_sem_release
 * fm_sem_clear
 */

/* create */
void fm_sem_create(fm_sem_t *target, fm_sem_cnt init_count, fm_sem_cnt max_count){
	/* alloc space */
	*target = (fm_sem_t)fm_object_malloc(type_fm_sem);
	if(*target == NULL)
		return;
	/* initial */
	fm_uint32_t level = _fm_interrupt_disable_();
	fm_sem_init(&((*target)->sem), init_count, max_count);
	(*target)->next = (*target)->prev = NULL;
	_insert_sem_system_list(&(fm_sem_que), *target);
	_fm_interrupt_enable_(level);
}

/* delete */
void fm_sem_delete(fm_sem_t *target){
	/* delete from system queue */
	if(target == NULL || *target == NULL){
		return;
	}
	fm_uint32_t level = _fm_interrupt_disable_();
	_remove_sem_system_list(&(fm_sem_que), *target);
	/* free thread */
	fm_tcb *ptr = (*target)->sem.wait_queue.head;
	/* disable schedule */
	disable_schedule();
	while(ptr != NULL){
		fm_thread_resume(&ptr);
		ptr = ptr->sem_next_thread;
	}
	/* enable schedule */
	enable_schedule();
	/* free resource */
	(*target)->next = (*target)->prev = NULL;
	fm_free(*target);
	*target = NULL;
	_fm_interrupt_enable_(level);

	if(need_schedule){
		fm_thread_schedule();
	}
}

void fm_sem_iner_request(fm_scb *sem){
	fm_uint32_t level = _fm_interrupt_disable_();
	sem->count--;
	if(sem->count < 0) { /* can't be requested*/
		/* put into wait queue for semaphore */
		_insert(usr_run, &(sem->wait_queue));
		/* suspend */
		disable_schedule();
		fm_thread_suspend(&usr_run);
		enable_schedule();
		_fm_interrupt_enable_(level);
		fm_thread_schedule();
	} else {
		_fm_interrupt_enable_(level);
	}
}

/* request for semaphore */
void fm_sem_request(fm_sem_t *target){
	/* can be requested */
	if(target == NULL || *target == NULL){
		return;
	}
	fm_sem_iner_request(&(*target)->sem);
}

void fm_sem_iner_release(fm_scb *sem){
	fm_uint32_t level = _fm_interrupt_disable_();
	if(sem->count++ == sem->max_count){
		sem->count = sem->max_count;
	}
	if(sem->wait_queue.head != NULL){
		fm_tcb *result = NULL;
		_find_highest_priority(&result, &(sem->wait_queue));
		/* delete from wait queue in sem */
		_remove(result, &(sem->wait_queue));
		/* resume thread */
		disable_schedule();
		fm_thread_resume(&result);
		enable_schedule();
		_fm_interrupt_enable_(level);
		fm_thread_schedule();
	} else {
		_fm_interrupt_enable_(level);
	}
}

/* release semaphore */
void fm_sem_release(fm_sem_t *target){
	/* can be released */
	if(target == NULL || *target == NULL){
		return;
	}
	fm_sem_iner_release(&(*target)->sem);

}

/* delete all semaphore */
void fm_sem_clear(void){
	_fm_sem_t *ptr = fm_sem_que.head;
	fm_uint32_t level = _fm_interrupt_disable_();
	need_schedule = 0;
	while(ptr != NULL){
		fm_sem_delete(&ptr);
		ptr = ptr->next;
	}
	need_schedule = 1;
	_fm_interrupt_enable_(level);
	fm_thread_schedule();
}

#endif

#ifdef _USE_FM_MUTEX_

static void check_and_change_priority(fm_tcb *);

static void _insert_system_mutex_list(fm_mutex_queue *, _fm_mutex_t *);
static void _remove_system_mutex_list(fm_mutex_queue *, _fm_mutex_t *);

static void _insert_system_mutex_list(fm_mutex_queue *queue, _fm_mutex_t *mut){
	if(queue->head == NULL){
		queue->head = mut;
	} else {
		queue->tail->next = mut;
	}
	mut->prev = queue->tail;
	queue->tail = mut;
}

static void _remove_system_mutex_list(fm_mutex_queue *queue, _fm_mutex_t *mut){
	if(mut->prev == NULL){
		queue->head = mut->next;
	} else {
		mut->prev->next = mut->next;
	}
	if(mut->next == NULL){
		queue->tail = mut->prev;
	} else {
		mut->next->prev = mut->prev;
	}
	mut->prev = mut->next = NULL;
}

void fm_mutex_init(fm_mutex *mutex){
	mutex->deep = 0;
	mutex->hold = NULL;
	mutex->wait_queue.head = mutex->wait_queue.tail = NULL;
}

/*
 * mutex section
 * fm_mutex_create
 * fm_mutex_delete
 * fm_mutex_unlock
 * fm_mutex_clear
 */

/* create mutex */
void fm_mutex_create(fm_mutex_t *target){
	/* alloc space */
	if(target == NULL){
		return;
	}
	*target = (fm_mutex_t)fm_object_malloc(type_fm_mutex);
	if(*target == NULL){
		return;
	}
	/* initial */
	fm_uint32_t level = _fm_interrupt_disable_();
	fm_mutex_init(&((*target)->mutex));
	(*target)->next = NULL;
	(*target)->prev = NULL;
	/* put into system queue */
	_insert_system_mutex_list(&(fm_mutex_que), *target);
	_fm_interrupt_enable_(level);
}

/* delete mutex */
void fm_mutex_delete(fm_mutex_t *target){
	/* delete from mutex queue */
	if(target == NULL || *target == NULL){
		return;
	}
	fm_uint32_t level = _fm_interrupt_disable_();
	_remove_system_mutex_list(&(fm_mutex_que), *target);
	/* free thread */
	fm_tcb *ptr = (*target)->mutex.wait_queue.head;
	/* disable schedule */
	disable_schedule();
	while(ptr != NULL){
		fm_thread_resume(&ptr);
		ptr = ptr->sem_next_thread;
	}
	enable_schedule();
	/* free resource */
	(*target)->next = (*target)->prev = NULL;
	fm_free(*target);
	*target = NULL;
	_fm_interrupt_enable_(level);

	if(need_schedule){
		fm_thread_schedule();
	}
}

static void check_and_change_priority(fm_tcb *p){
	if(p == NULL){
		return;
	}
	if(p->priority > usr_run->priority){
		fm_thread_priority prev_priority = p->priority;
		p->priority = usr_run->priority;
		if(p->status & (READY | RUN)){
			fm_thread_from_ready_to_ready(p, prev_priority, p->priority);
		}
	}
}

void fm_mutex_iner_lock(fm_mutex *mutex){
	fm_uint32_t level = _fm_interrupt_disable_();
	if(mutex->hold == NULL){
		mutex->hold = usr_run;
		_fm_interrupt_enable_(level);
	} else {
		/* put into wait queue */
		if(mutex->hold == usr_run){
			++mutex->deep;
			_fm_interrupt_enable_(level);
			return;
		}
		_insert(usr_run, &(mutex->wait_queue));
		check_and_change_priority(mutex->hold);
		/* suspend */
		disable_schedule();
		fm_thread_suspend(&usr_run);
		enable_schedule();
		_fm_interrupt_enable_(level);
		fm_thread_schedule();
	}
}

/* lock mutex */
void fm_mutex_lock(fm_mutex_t *target){
	if(target == NULL || *target == NULL){
		return;
	}
		/* can lock */
	fm_mutex_iner_lock(&((*target)->mutex));
}

void fm_mutex_iner_unlock(fm_mutex *mutex){
	if(!usr_run){
		return;
	}
	fm_uint32_t level = _fm_interrupt_disable_();
	if(mutex->hold == usr_run){
		/* need resume thread */
		if(mutex->deep){
			--mutex->deep;
			_fm_interrupt_enable_(level);
			return;
		}
		fm_bool _need_schedule = 0;
		if(usr_run->priority != usr_run->init_priority){
			fm_thread_from_ready_to_ready(usr_run, usr_run->priority, usr_run->init_priority);
		}
		if(mutex->wait_queue.head != NULL){
			fm_tcb *result = NULL;
			_find_highest_priority(&result, &(mutex->wait_queue));
			/* delete */
			_remove(result, &(mutex->wait_queue));
			/* resume */
			mutex->hold = result;
			disable_schedule();
			fm_thread_resume(&result);
			enable_schedule();
			_need_schedule = 1;
		} else {
			mutex->hold = NULL;
		}
		_fm_interrupt_enable_(level);

		if(_need_schedule){
			fm_thread_schedule();
		}
	} else {
		_fm_interrupt_enable_(level);
	}
}

/* unlock mutex */
void fm_mutex_unlock(fm_mutex_t *target){
	/* can unlock */
	if(target == NULL || *target == NULL){
		return;
	}
	fm_mutex_iner_unlock(&(*target)->mutex);
}

/* delete all mutex */
void fm_mutex_clear(void){
	_fm_mutex_t *ptr = fm_mutex_que.head;
	fm_uint32_t level = _fm_interrupt_disable_();
	need_schedule = 0;
	while(ptr != NULL){
		fm_mutex_delete(&ptr);
		ptr = ptr->next;
	}
	need_schedule = 1;
	_fm_interrupt_enable_(level);
	fm_thread_schedule();
}

#endif

/*
---------Created by Henry Huang----------
---thread.h
---
*/
#ifndef _FM_THREAD_H_
#define _FM_THREAD_H_

#include <kernel/fm_stdtype.h>
#include "fm_config.h"

#ifdef _USE_FM_MAIL_
#include "fm_mail.h"
#endif

typedef fm_uint8_t* fm_address;
typedef fm_uint32_t fm_thread_id;
typedef fm_uint32_t fm_thread_time_tick;
typedef fm_uint32_t fm_thread_number;
typedef fm_uint16_t fm_stack_size;
typedef fm_uint8_t fm_thread_priority;
typedef fm_uint32_t fm_thread_status;
typedef fm_uint8_t fm_bool;


#define READY_QUEUE_TYPE 		0
#define WAIT_QUEUE_TYPE 		0

#define PRIORITY_SIZE 			32

/* state */
#define THREAD_UNUSED 			0
#define RUN 					1
#define READY 					2
#define WAIT 					4
#define WAIT_TIME 				8
#define SLEEP 					16

/* return for create thread */
#define CREATE_THREAD_SUCCEED 	1
#define CREATE_THREAD_FAILED 	0

/* schedule flag */
#define THREAD_SCHEDULE_ENABLE 	1
#define THREAD_SCHEDULE_DISABLE 0

/* ticks */
//#define NO_TICKS 				0xffffffff

#define CONST_CHECK_SUM 		0xdeadbeef


/* TCB */
typedef struct fm_rtos_thread_struct {
	/* thread id */
	fm_thread_id tid;
	/* thread status */
	fm_thread_status status;
	/* thread priority */
	fm_thread_priority priority;
	fm_thread_priority init_priority;
	fm_stack_size stack_size;
	fm_address stack_top;
	void *stack_start;
	/* entry routine */
	void (*entry)(void);
	/* for tcb manager */
	struct fm_rtos_thread_struct *next_thread;
	struct fm_rtos_thread_struct *prev_thread;

	/* for semaphore */
#if	defined(_USE_FM_MUTEX_) || defined(_USE_FM_SEM_)
	struct fm_rtos_thread_struct *sem_next_thread;
	struct fm_rtos_thread_struct *sem_prev_thread;
#endif

#ifdef _USE_FM_THREAD_SLEEP_
	volatile fm_thread_time_tick ticks;
	struct fm_rtos_thread_struct *time_next_thread;
	struct fm_rtos_thread_struct *time_prev_thread;
#endif

#ifdef _USE_FM_MAIL_
	fm_mail_box mailbox;
#endif
//	uint32_t checksum;
} fm_tcb;

typedef fm_tcb* fm_thread_t;

/* tcb queue ready */
typedef struct fm_rtos_thread_ready_queue_manager{
	fm_tcb *head[PRIORITY_SIZE];
#if READY_QUEUE_TYPE == 1
	fm_tcb *tail[PRIORITY_SIZE];
#endif
	/* ready queue state */
	fm_uint8_t priority_state[(PRIORITY_SIZE>>3)];
} fm_tcb_ready_queue;

/* tcb queue other */
//typedef struct fm_rtos_thread_queue_manager{
//	fm_tcb *head[PRIORITY_SIZE], *tail[PRIORITY_SIZE];
//} fm_tcb_queue;

typedef struct fm_thread_tcb_queue{
	fm_tcb *head;
#if WAIT_QUEUE_TYPE == 1
	fm_tcb *tail;
#endif
} fm_tcb_queue;

typedef struct fm_thread_timer_list{
	fm_tcb *head;
	fm_tcb *tail;
} fm_thread_list;

typedef struct fm_rtos_thread_resource_manager{
	fm_tcb_ready_queue ready_queue;
	fm_tcb_queue wait_queue, sleep_queue;
	fm_thread_list timer_queue;
	fm_bool schedule_state;
	fm_uint8_t paddings[3];
} fm_tcb_queue_manager;

void fm_thread_init(void);

/*
 * 线程创建
 * target 储存thread对象
 * thread_entry 入口函数，不支持参数
 * stack_size 堆栈大小  缺省值 0 大小为1024 bytes
 * priority 优先级
 */
fm_bool fm_thread_create(fm_thread_t *target, void (*thread_entry)(void), fm_uint16_t stack_size, fm_thread_priority priority);

/*
 * 唤醒目标线程
 */
void fm_thread_wake(fm_thread_t *target);

/*
 * 挂起对象线程
 */
void fm_thread_suspend(fm_thread_t *target);

/*
 * 自我挂起
 */
void fm_thread_suspend_self(void);

/*
 * 恢复目标线程
 */
void fm_thread_resume(fm_thread_t *target);

#ifdef _USE_FM_THREAD_CHANGE_PRIORITY_
/*
 * 改变目标线程优先级
 * target 目标
 * new_priority 新的优先级
 * 永久改变
 */
void fm_thread_change_priority(fm_thread_t *target, fm_thread_priority new_priority);

/*
 * 改变自己线程的优先级
 */
void fm_thread_change_priority_self(fm_thread_priority new_priority);
#endif

/*
 * 将当前任务挂起 指定数目 的系统时钟数
 */
void fm_thread_sleep(fm_uint32_t ticks);

/*
 * 进入调度器，函数后的代码将不会走到
 */
void fm_thread_system_start(void);

void fm_thread_schedule(void);

void fm_thread_from_ready_to_ready(fm_tcb *, fm_thread_priority, fm_thread_priority);

void irq_thread_timer(void);

fm_tcb_queue_manager *sys_control(void);

fm_tcb* running_thread(void);

#endif

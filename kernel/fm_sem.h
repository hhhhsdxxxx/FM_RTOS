#ifndef _FM_SEMAPHORE_H_
#define _FM_SEMAPHORE_H_

#include <kernel/fm_stdtype.h>
#include "fm_thread.h"
#include "fm_config.h"

/* semaphore */
typedef int fm_sem_cnt;

#if defined(_USE_FM_SEM_) || defined(_USE_FM_MUTEX_)
typedef struct {
	fm_tcb *head, *tail;
} fm_tsb_queue;
/* system initial */
#ifndef _COMPILER_SUPPORT_STATIC_
void fm_sem_mutex_init(void);
#endif

#endif

#ifdef _USE_FM_SEM_
typedef struct fm_sem_control_block{
	fm_sem_cnt count;
	fm_sem_cnt max_count;
	fm_tsb_queue wait_queue;
} fm_scb;

typedef struct fm_sem_struct{
	fm_scb sem;
	struct fm_sem_struct *prev, *next;
} _fm_sem_t;

typedef _fm_sem_t* fm_sem_t;

typedef struct fm_system_scb_manager{
	_fm_sem_t *head, *tail;
} fm_scb_queue;

void fm_sem_init(fm_scb *, fm_sem_cnt, fm_sem_cnt);

/*
 * 创建计数同步信号量
 * target 目标句柄
 * init_count 初始步数
 * max_count 最大的步数
 */
void fm_sem_create(fm_sem_t *target, fm_sem_cnt init_count, fm_sem_cnt max_count);

/*
 * 同步计数信号量删除
 * target 句柄
 */
void fm_sem_delete(fm_sem_t *target);

/*
 * 同步计数信号量申请
 * target 句柄
 */
void fm_sem_request(fm_sem_t *target);
void fm_sem_iner_request(fm_scb *sem);

/*
 * 同步计数信号量释放
 * target 句柄
 */
void fm_sem_release(fm_sem_t *target);
void fm_sem_iner_release(fm_scb *sem);

/*
 * 系统全部同步计数信号量清空
 */
void fm_sem_clear(void);

#endif

#ifdef _USE_FM_MUTEX_
/* mutex */
typedef fm_uint32_t fm_mutex_deep;

typedef struct fm_mutex_control_block{
	fm_tcb *hold;
	fm_mutex_deep deep;
	fm_tsb_queue wait_queue;
} fm_mutex;

typedef struct fm_mutex_struct{
	fm_mutex mutex;
	struct fm_mutex_struct *prev, *next;
} _fm_mutex_t;

typedef _fm_mutex_t* fm_mutex_t;

typedef struct fm_system_mutex_manager{
	_fm_mutex_t *head, *tail;
} fm_mutex_queue;


void fm_mutex_init(fm_mutex *mutex);

/*
 * 互斥信号量的创建
 * target 句柄
 */
void fm_mutex_create(fm_mutex_t *target);

/*
 * 互斥信号量的删除
 * target 句柄
 */
void fm_mutex_delete(fm_mutex_t *target);

/*
 * 互斥信号量的加锁
 * target 句柄
 */
void fm_mutex_lock(fm_mutex_t *target);
void fm_mutex_iner_lock(fm_mutex *mutex);

/*
 * 互斥信号量的解锁
 * target 句柄
 */
void fm_mutex_unlock(fm_mutex_t *target);
void fm_mutex_iner_unlock(fm_mutex *mutex);

/*
 * 系统互斥信号量的清空
 */
void fm_mutex_clear(void);

#endif

#endif

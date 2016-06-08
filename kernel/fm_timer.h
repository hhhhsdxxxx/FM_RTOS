#ifndef _FM_TIMER_H_
#define _FM_TIMER_H_

#ifdef _USE_FM_TIMER_
#include <kernel/fm_stdtype.h>
#include "fm_thread.h"

typedef fm_uint32_t fm_time_tick;

typedef struct fm_timer_struct{
	volatile fm_time_tick ticks;
	fm_time_tick init_ticks;
	fm_uint32_t state;
	void (*active_routine)(void);
} fm_timer;

typedef fm_timer* fm_timer_handler;

#ifdef _USE_FM_DYNAMIC_TIMER_
typedef struct fm_timer_dynamic_struct{
	fm_timer timer;
	struct fm_timer_dynamic_struct *next_timer, *prev_timer;
} fm_dynamic_timer;
#endif

#define TIMER_MAGIC			0xdeadbeef
#define STATIC_TIMER_NUM 	8
#define TIMER_ENABLE 		1
#define TIMER_DISABLE 		0
#define SET_UNUSE(x) 		x &= 0xfffeffff
#define SET_USE(x) 			x |= 0x00010000
#define TIMER_START(x)      x |= 0x00020000
#define TIMER_STOP(x)		x &= ~(0x00020000)
#define IS_USE(x)			(x&0x00010000)
#define IS_ACTIVE(x)		((x&0x00010000) && (x&0x00020000))
#define SET_STATE(x)		x |= 0x00000001
#define CLAER_STATE(x)		x &= 0xfffffffe
#define VALUE(x)			((fm_uint32_t)(x))


void fm_timer_init(void);

/*
 * ticks 	系统时钟数
 * routine 	回调函数，在中断中执行
 */
fm_timer_handler fm_timer_request(fm_time_tick, void (*)(void));

/*
 * 设置新的定时数
 */
void fm_timer_set_newticks(fm_timer_handler, fm_time_tick);

/*
 * 开启定时器
 * 定时器目标句柄
 */
void fm_timer_start(fm_timer_handler);

/*
 * 停止定时器
 * 定时器目标句柄
 */
void fm_timer_stop(fm_timer_handler);

/*
 * 暂停定时器
 * 定时器目标句柄
 */
void fm_timer_pause(fm_timer_handler);

/*
 * 继续定时器
 * 定时器目标句柄
 */
void fm_timer_resume(fm_timer_handler);

inline fm_uint32_t fm_timer_state(fm_timer_handler timer){
	return timer->state & 0x1;
}

/*
 * 释放对当前定时器的控制
 * 定时器目标句柄
 */
void fm_timer_release(fm_timer_handler);

#else

typedef fm_uint32_t fm_time_tick;

#endif

typedef struct fm_timer_manager_blcok{
	volatile fm_time_tick total_ticks;
#ifdef _USE_FM_TIMER_
	fm_timer timers[STATIC_TIMER_NUM];
#ifdef _USE_FM_DYNAMIC_TIMER_
	fm_dynamic_timer *head;
#endif
#endif
} fm_timer_mgr;

/*
 * 获取当前系统时钟的节拍数
 */
fm_time_tick get_system_ticks(void);

void irq_timer_handler(void);

#endif

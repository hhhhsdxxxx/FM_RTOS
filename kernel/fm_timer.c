#include "fm_config.h"
#include <kernel/fm_stdtype.h>
#include "fm_timer.h"

#ifdef _USE_FM_TIMER_

#include "fm_thread.h"
#include "fm_mem.h"

#include "fm_service.h"


#ifndef _USE_FM_MEM_
#include <stdlib.h>
#endif

#ifndef _USE_FM_MEM_SERVICE_
#include <string.h>
#endif

extern fm_uint32_t _fm_interrupt_disable_(void);
extern void _fm_interrupt_enable_(fm_uint32_t);

static fm_timer_mgr sys_timers;

#ifdef _USE_FM_DYNAMIC_TIMER_

void fm_dynamic_timer_release(fm_dynamic_timer *);
fm_timer_handler fm_dynamic_timer_request(fm_time_tick, void (*)(void));

void irq_dynamic_timer_handler(void);

fm_timer_handler fm_dynamic_timer_request(fm_time_tick ticks, void (*routine)(void)){
	fm_dynamic_timer *new_timer = (fm_dynamic_timer*)fm_object_malloc(type_fm_dynamic_timer);
	if(!new_timer){
		return (void*)0;
	}
	new_timer->timer.init_ticks = ticks;
	new_timer->timer.ticks = TIMER_MAGIC;
	new_timer->timer.active_routine = routine;
	new_timer->prev_timer = NULL;
	new_timer->next_timer = sys_timers.head;
	SET_USE(new_timer->timer.state);
	if(sys_timers.head){
		sys_timers.head->prev_timer = new_timer;
	}
	sys_timers.head = new_timer;
	return &(new_timer->timer);
}

void fm_dynamic_timer_release(fm_dynamic_timer *timer){
	if(timer == sys_timers.head){
		sys_timers.head = NULL;
	} else {
		if(timer->prev_timer){
			timer->prev_timer->next_timer = timer->next_timer;
		} else {
			sys_timers.head = timer->next_timer;
		}
		if(timer->next_timer){
			timer->next_timer->prev_timer = timer->prev_timer;
		}
	}
	fm_free(timer);
}

void irq_dynamic_timer_handler(void){
	fm_dynamic_timer *ptr = sys_timers.head;
	while(ptr){
		if(IS_ACTIVE(ptr->timer.state) && !(--(ptr->timer.ticks))){
			fm_timer_stop(&(ptr->timer));
			ptr->timer.active_routine();
		}
		ptr = ptr->next_timer;
	}
}

#endif

void fm_timer_init(){
	fm_memset(&sys_timers, 0, sizeof(fm_timer_mgr));
}

void fm_timer_release(fm_timer_handler timer){
	timer->ticks = 0;
	TIMER_STOP(timer->state);
	SET_UNUSE(timer->state);
	if(VALUE(timer) < VALUE(&(sys_timers.timers[0])) || VALUE(timer) > VALUE(&(sys_timers.timers[7]))){
		fm_dynamic_timer_release((fm_dynamic_timer*)timer);
	}
}

fm_timer_handler fm_timer_request(fm_time_tick ticks, void (*routine)(void)){
	int i = 0;
	for(; i < STATIC_TIMER_NUM; ++i){
		if(!IS_USE(sys_timers.timers[i].state)){
			sys_timers.timers[i].init_ticks = ticks;
			sys_timers.timers[i].ticks = TIMER_MAGIC;
			sys_timers.timers[i].active_routine = routine;
			SET_USE(sys_timers.timers[i].state);
			break;
		}
	}
	if(i == STATIC_TIMER_NUM){
#ifdef _USE_FM_DYNAMIC_TIMER_
		return fm_dynamic_timer_request(ticks, routine);
#else
		return (void*)0;
#endif
	}
	return &(sys_timers.timers[i]);
}

void fm_timer_start(fm_timer_handler timer){
	register fm_uint32_t level = _fm_interrupt_disable_();
	TIMER_START(timer->state);
	timer->ticks = timer->init_ticks;
	_fm_interrupt_enable_(level);
}

void fm_timer_stop(fm_timer_handler timer){
	register fm_uint32_t level = _fm_interrupt_disable_();
	TIMER_STOP(timer->state);
	timer->ticks = TIMER_MAGIC;
	_fm_interrupt_enable_(level);
}

void fm_timer_pause(fm_timer_handler timer){
	register fm_uint32_t level = _fm_interrupt_disable_();
	TIMER_STOP(timer->state);
	_fm_interrupt_enable_(level);
}

void fm_timer_resume(fm_timer_handler timer){
	register fm_uint32_t level = _fm_interrupt_disable_();
	if(timer->ticks != TIMER_MAGIC){
		TIMER_START(timer->state);
	}
	_fm_interrupt_enable_(level);
}

void fm_timer_set_newticks(fm_timer_handler timer, fm_time_tick ticks){
	register fm_uint32_t level = _fm_interrupt_disable_();
	if(!IS_ACTIVE(timer->state)){
		timer->ticks 		= TIMER_MAGIC;
		timer->init_ticks 	= ticks;
	}
	_fm_interrupt_enable_(level);
}
#else
static fm_timer_mgr sys_timers;

#endif

fm_time_tick get_system_ticks(void){
	return sys_timers.total_ticks;
}

void irq_timer_handler(void){
	int i;
	fm_uint32_t level;
	level = _fm_interrupt_disable_();
	++sys_timers.total_ticks;
#ifdef _USE_FM_TIMER_
//	_fm_interrupt_enable_(level);

	fm_timer_handler timer;
	for(i = 0; i < STATIC_TIMER_NUM; ++i){
//		level = _fm_interrupt_disable_();
		timer = &(sys_timers.timers[i]);
		if(IS_ACTIVE(timer->state) && (!(--timer->ticks))){
			fm_timer_stop(timer);
//			_fm_interrupt_enable_(level);
			timer->active_routine();
		}
	}
#ifdef _USE_FM_DYNAMIC_TIMER_
	irq_dynamic_timer_handler();
#endif
#endif
	_fm_interrupt_enable_(level);
}

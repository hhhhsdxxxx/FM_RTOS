#include "fm_thread.h"
#include "fm_timer.h"
#include "fm_int.h"
#include "cortexm/ExceptionHandlers.h"

#include <stm32f10x.h>
#include "bsp/stm32f103_driver/fm_uart.h"

extern fm_bool is_start;

extern fm_uint32_t _fm_interrupt_disable_(void);
extern void _fm_interrupt_enable_(fm_uint32_t);
volatile uint16_t interrupt_deep = 0;

void fm_enter_irq(void){
	register fm_uint32_t level = _fm_interrupt_disable_();
	++interrupt_deep;
	_fm_interrupt_enable_(level);
}

void fm_exit_irq(void){
	register fm_uint32_t level = _fm_interrupt_disable_();
	--interrupt_deep;
	_fm_interrupt_enable_(level);
}

fm_uint16_t get_interrupt_nest(void){
	return interrupt_deep;
}


void SysTick_Handler(void){
	if(is_start){
#ifdef _USE_FM_TIMER_
		irq_timer_handler();
#endif
		irq_thread_timer();
	}
}

#include "cpu_m3.h"

extern void _update_psp_(void);
extern fm_tcb *usr_run;

fm_uint32_t psp = 0;

fm_address fm_stack_init(void *entry, fm_address stack_addr, void *exit){
	fm_address sp;
	sp = stack_addr;
	sp = (fm_address)ALIGN_DOWN((fm_uint32_t)sp, 8);
	sp -= sizeof(struct stack_frame);
	struct stack_frame *stack = (struct stack_frame*)sp;
	unsigned long i = 0;
	for(; i < sizeof(struct stack_frame)/sizeof(fm_uint32_t); ++i){
		((fm_uint32_t*)stack)[i] = 0x00000000;
	}
	stack->except_reg.r0 = 0;
	stack->except_reg.r1 = 0;
	stack->except_reg.r2 = 0;
	stack->except_reg.r3 = 0;
	stack->except_reg.r12 = 0;
	stack->except_reg.lr = (fm_uint32_t)exit;
	stack->except_reg.pc = (fm_uint32_t)entry;
	stack->except_reg.psr = 0x01000000L;
	return sp;
}

fm_uint32_t get_thread_stack_used_size(void){
	if(!usr_run){
		return 0;
	}
	_update_psp_();
	return (fm_uint32_t)(usr_run->stack_size) - (psp - (fm_uint32_t)(usr_run->stack_start));
}

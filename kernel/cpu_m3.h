#ifndef _CPU_M3_FM_H_
#define _CPU_M3_FM_H_

#include <kernel/fm_stdtype.h>
#include "fm_thread.h"

typedef volatile fm_uint32_t fm_register_t;

typedef struct exception_register{
	fm_register_t r0;
	fm_register_t r1;
	fm_register_t r2;
	fm_register_t r3;
	fm_register_t r12;
	fm_register_t lr;
	fm_register_t pc;
	fm_register_t psr;
}exception_reg;

struct stack_frame{
	fm_register_t r4;
	fm_register_t r5;
	fm_register_t r6;
	fm_register_t r7;
	fm_register_t r8;
	fm_register_t r9;
	fm_register_t r10;
	fm_register_t r11;
	exception_reg except_reg;
};

#define ALIGN_DOWN(size, align)      ((size) & ~(((unsigned long)align) - 1))

fm_address fm_stack_init(void *entry, fm_address stack_addr, void *exit);
fm_uint32_t get_thread_stack_used_size(void);

#endif

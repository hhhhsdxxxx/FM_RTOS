#ifndef _FM_INT_H_
#define _FM_INT_H_

#include <kernel/fm_stdtype.h>

void fm_enter_irq(void);
void fm_exit_irq(void);
fm_uint16_t get_interrupt_nest(void);

#endif

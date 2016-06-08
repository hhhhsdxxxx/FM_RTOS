#include "fm_config.h"
#include "fm_sem.h"
#include "fm_thread.h"
#include "fm_mem.h"
#include "fm_timer.h"
#include "fm_start.h"
#include "fm_project_config.h"
#include "fm_log.h"

#ifdef _USE_STM32F10X_
#include <stm32f10x.h>
#endif

void fm_rtos_kernel_init(void){
	fm_print_log("Timer initial...\r\n");
#ifdef _USE_FM_TIMER_
	fm_timer_init();
#endif
	fm_print_log("Semaphore initial...\r\n");
	fm_sem_mutex_init();
#ifdef _USE_FM_MEM_
	extern char _Heap_Begin;
	extern char _Heap_Limit;
	fm_print_log("Memory initial...\r\n");
	fm_memheap_init((fm_mem_ptr)&_Heap_Begin, (fm_mem_ptr)&_Heap_Limit);
#endif
	fm_print_log("Thread initial...\r\n");
	fm_thread_init();
}

void fm_rtos_task_start(void){
	SysTick_Config (SystemCoreClock / TICKS_PER_SECOND);
	fm_thread_system_start();
}

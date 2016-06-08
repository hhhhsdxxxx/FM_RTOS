#include "fm_assert.h"
#include "fm_project_config.h"

#ifdef _USE_STM32F10X_
#include "bsp/stm32f103_driver/fm_uart.h"
#endif

static void assert_putmsg(const char *msg){
	fm_uart_putmsg(FM_SYSTEM_UART, msg);
	fm_uart_tx_flush(FM_SYSTEM_UART);
}

void fm_assert(int r, const char *msg){
	if(!r){
		assert_putmsg(msg);
		while(1){
			__asm("NOP");
		}
	}
}

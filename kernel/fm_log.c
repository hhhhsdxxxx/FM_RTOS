#include "fm_config.h"
#include "fm_service.h"
#include "fm_project_config.h"
#include "fm_log.h"

#ifdef _USE_STM32F10X_
#include <stm32f10x.h>
#include "bsp/stm32f103_driver/fm_uart.h"
enum fm_uart_port default_uart = FM_SYSTEM_UART;
#endif

void set_default_uart(enum fm_uart_port _uart){
	default_uart = _uart;
}

void fm_flush(void){
	fm_uart_tx_flush(default_uart);
}

void fm_write(const void *buf, int n){
	fm_uart_write(default_uart, buf, n);
}

void fm_print(const char *msg){
	fm_uart_putmsg(default_uart, msg);
	fm_uart_tx_flush(default_uart);
}

void fm_puts(const char *msg){
	fm_uart_putmsg(default_uart, msg);
	fm_uart_write(default_uart, "\r\n", 2);
	fm_uart_tx_flush(default_uart);
}

void fm_putchar(char ch){
	fm_uart_putchar(default_uart, ch);
	fm_uart_tx_flush(default_uart);
}

int fm_printf(const char *fmt, ...){
	int n;
	char buf[64];
	va_list args;
	va_start(args, fmt);
	n = fm_vsnprintf(buf, 64, fmt, args);
	va_end(args);
	fm_print(buf);
	return n;
}

char fm_getchar(void){
	return fm_uart_getchar(default_uart);
}

int fm_read(void *dest, int max_size){
	return fm_uart_read(default_uart, dest, max_size);
}

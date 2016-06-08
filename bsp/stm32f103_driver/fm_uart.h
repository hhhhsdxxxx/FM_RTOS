#ifndef _FM_UART_H_
#define _FM_UART_H_

#include "fm_project_config.h"
#include <stm32f10x_usart.h>

#ifdef _USE_STM32F10X_

#define FM_SYSTEM_UART					fm_uart1

#define UART1_PREEMPTION_INTERRUPT_PRI 	0
#define UART1_SUB_INTERRUPT_PRI 		0
#define UART1_BAUDRATE					115200
#define UART1_WORDLENGTH				USART_WordLength_8b
#define UART1_STOPBITS					USART_StopBits_1
#define UART1_PARITY					USART_Parity_No

#define UART2_PREEMPTION_INTERRUPT_PRI 	0
#define UART2_SUB_INTERRUPT_PRI 		0
#define UART2_BAUDRATE					115200
#define UART2_WORDLENGTH				USART_WordLength_8b
#define UART2_STOPBITS					USART_StopBits_1
#define UART2_PARITY					USART_Parity_No

#define UART3_PREEMPTION_INTERRUPT_PRI 	0
#define UART3_SUB_INTERRUPT_PRI 		0
#define UART3_BAUDRATE					115200
#define UART3_WORDLENGTH				USART_WordLength_8b
#define UART3_STOPBITS					USART_StopBits_1
#define UART3_PARITY					USART_Parity_No

#define UART4_PREEMPTION_INTERRUPT_PRI 	0
#define UART4_SUB_INTERRUPT_PRI 		0
#define UART4_BAUDRATE					115200
#define UART4_WORDLENGTH				USART_WordLength_8b
#define UART4_STOPBITS					USART_StopBits_1
#define UART4_PARITY					USART_Parity_No

#define DEFAULT_RECEIVE_BUFFER_SIZE		64
#define DEFAULT_SEND_BUFFER_SIZE		64

#define DEFAULT_FLUSH_SIZE				16

#define FIFO_BUFFER_SIZE				128

#define UART_MAGIC						0xdead



enum fm_uart_port{
	fm_uart1,
	fm_uart2,
	fm_uart3,
	fm_uart4,
	error
};

#endif

typedef struct fm_uart_receive_struct{
	u16 read_ptr, write_ptr;
	u16 size;
	char buffer[DEFAULT_RECEIVE_BUFFER_SIZE & ~(0x3)];
	char fifo_buf[FIFO_BUFFER_SIZE];
} fm_uart_rev_str;

typedef struct fm_uart_receive_struct_nodma{
	u16 read_ptr, write_ptr;
	u16 size;
	char buffer[(DEFAULT_RECEIVE_BUFFER_SIZE & ~(0x3)) + 2];
} fm_uart_rev_str_nd;

typedef struct fm_uart_send_str{
	u16 size;
	char buffer[(DEFAULT_SEND_BUFFER_SIZE & ~(0x3)) + 2];
} fm_uart_send_str;

typedef struct uart_control_struct{
	fm_uart_send_str 	send;
	fm_uart_rev_str 	recv;
} fm_uart_str;

typedef struct uart_control_struct_nodma{
	fm_uart_send_str 	send;
	fm_uart_rev_str_nd 	recv;
} fm_uart_str_nd;

void fm_uart_init(void);
void fm_uart_open(enum fm_uart_port);
void fm_uart_close(enum fm_uart_port);

void fm_uart_write(enum fm_uart_port, const void*, int);
void fm_uart_tx_flush(enum fm_uart_port);
int fm_uart_read(enum fm_uart_port, void*, int);
void fm_uart_rx_clear(enum fm_uart_port);

void fm_uart_putchar(enum fm_uart_port, char);
void fm_uart_putmsg(enum fm_uart_port, const char*);

char fm_uart_getchar(enum fm_uart_port);
int fm_uart_getmsg(enum fm_uart_port, char*, char);



#endif

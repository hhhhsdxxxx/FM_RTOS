#include <kernel/fm_stdtype.h>
#include "fm_project_config.h"
#include "fm_uart.h"

#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_usart.h>
#include "kernel/fm_int.h"
#include "kernel/fm_service.h"


#define UART1_GPIO_RX        GPIO_Pin_9
#define UART1_GPIO_TX        GPIO_Pin_10
#define UART1_GPIO           GPIOA

#define UART2_GPIO_RX        GPIO_Pin_2
#define UART2_GPIO_TX        GPIO_Pin_3
#define UART2_GPIO           GPIOA

#define UART3_GPIO_RX        GPIO_Pin_10
#define UART3_GPIO_TX        GPIO_Pin_11
#define UART3_GPIO           GPIOB

#define UART4_GPIO_RX        GPIO_Pin_10
#define UART4_GPIO_TX        GPIO_Pin_11
#define UART4_GPIO           GPIOC

extern fm_uint32_t _fm_interrupt_disable_(void);
extern void _fm_interrupt_enable_(fm_uint32_t);

static void RCC_config(void);
static void GPIO_config(void);
static void NVIC_config(void);
static void UART_config(void);

#ifdef _USE_FM_UART1_

#ifdef _USE_FM_UART1_RX_DMA_
static fm_uart_str uart1_control;
#else
static fm_uart_str_nd uart1_control;
#endif

#endif

#ifdef _USE_FM_UART2_

#ifdef _USE_FM_UART2_RX_DMA_
static fm_uart_str uart2_control;
#else
static fm_uart_str_nd uart2_control;
#endif

#endif

#ifdef _USE_FM_UART3_

#ifdef _USE_FM_UART3_RX_DMA_
static fm_uart_str uart3_control;
#else
static fm_uart_str_nd uart3_control;
#endif

#endif

#ifdef _USE_FM_UART4_
static fm_uart_str uart4_control;
#endif

#ifdef _USE_FM_UART_DMA_

static void DMA_config(void);

#ifdef _USE_FM_UART1_DMA_

#ifdef _USE_FM_UART1_RX_DMA_
static void uart1_rx_DMA_channel_request(void);
#endif

#ifdef _USE_FM_UART1_TX_DMA_
static void uart1_tx_DMA_channel_request(void);
#endif

#endif

#ifdef _USE_FM_UART2_DMA_
static void uart2_rx_DMA_channel_request(void);
#endif

#ifdef _USE_FM_UART3_DMA_
static void uart3_rx_DMA_channel_request(void);
#endif

#endif


static void RCC_config(void){
#ifdef _USE_FM_UART1_
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif
#ifdef _USE_FM_UART2_
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif
#ifdef _USE_FM_UART3_
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif
#ifdef _USE_FM_UART4_
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
#endif
}

static void GPIO_config(void){
	GPIO_InitTypeDef GPIO_init_str;
	GPIO_init_str.GPIO_Speed 	= GPIO_Speed_2MHz;
#ifdef _USE_FM_UART1_
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_init_str.GPIO_Pin 		= UART1_GPIO_TX;
	GPIO_Init(UART1_GPIO, &GPIO_init_str);
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_init_str.GPIO_Pin 		= UART1_GPIO_RX;
	GPIO_Init(UART1_GPIO, &GPIO_init_str);
#endif
#ifdef _USE_FM_UART2_
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_init_str.GPIO_Pin 		= UART2_GPIO_TX;
	GPIO_Init(UART2_GPIO, &GPIO_init_str);
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_init_str.GPIO_Pin 		= UART2_GPIO_RX;
	GPIO_Init(UART2_GPIO, &GPIO_init_str);
#endif
#ifdef _USE_FM_UART3_
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_init_str.GPIO_Pin 		= UART3_GPIO_TX;
	GPIO_Init(UART3_GPIO, &GPIO_init_str);
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_init_str.GPIO_Pin 		= UART3_GPIO_RX;
	GPIO_Init(UART3_GPIO, &GPIO_init_str);
#endif
#ifdef _USE_FM_UART4_
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_init_str.GPIO_Pin 		= UART4_GPIO_TX;
	GPIO_Init(UART4_GPIO, &GPIO_init_str);
	GPIO_init_str.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_init_str.GPIO_Pin 		= UART4_GPIO_RX;
	GPIO_Init(UART4_GPIO, &GPIO_init_str);
#endif
}

#ifdef _USE_FM_UART1_RX_DMA_
static void uart1_rx_DMA_channel_request(void){
	DMA_InitTypeDef DMA_init_str;
	DMA_DeInit(DMA1_Channel5);
	DMA_init_str.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);
	DMA_init_str.DMA_MemoryBaseAddr		= (u32)(uart1_control->recv.buffer);
	DMA_init_str.DMA_DIR				= DMA_DIR_PeripheralSRC;
	DMA_init_str.DMA_BufferSize			= DEFAULT_RECEIVE_BUFFER_SIZE;
	DMA_init_str.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
	DMA_init_str.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
	DMA_init_str.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_init_str.DMA_MemoryDataSize 	= DMA_MemoryDataSize_Byte;
	DMA_init_str.DMA_Mode 				= DMA_Mode_Normal;
	DMA_init_str.DMA_Priority 			= DMA_Priority_High;
	DMA_init_str.DMA_M2M 				= DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5, &DMA_init_str);
//	DMA_Cmd(DMA1_Channel5, ENABLE);
}
#endif

#ifdef _USE_FM_UART1_
void USART1_IRQHandler(void){
	fm_enter_irq();
#ifdef _USE_FM_UART1_RX_DMA_
	u16 len, flag;
	fm_uart_rev_str *uart_rx;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET){
		flag = 0;
		len = USART1->SR;
		len = USART1->DR;
		DMA_Cmd(DMA1_Channel5, DISABLE);
		len = DEFAULT_RECEIVE_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
		if(!len){
			DMA_SetCurrDataCounter(DMA1_Channel5, DEFAULT_RECEIVE_BUFFER_SIZE);
			DMA_Cmd(DMA1_Channel5, ENABLE);
			fm_exit_irq();
			return;
		}
		uart_rx = &(uart1_control.recv);
		if(uart_rx->read_ptr == UART_MAGIC){
			uart_rx->read_ptr 	= 0;
			uart_rx->write_ptr 	= 0xffff;
		}
		if(uart_rx->write_ptr < uart_rx->read_ptr && uart_rx->read_ptr < uart_rx->write_ptr + len ){
			flag = 1;
		}
		++uart_rx->write_ptr;
		if(uart_rx->write_ptr + len - 1 >= FIFO_BUFFER_SIZE){
			fm_memcpy(uart_rx->fifo_buf + uart_rx->write_ptr, uart_rx->buffer, FIFO_BUFFER_SIZE - uart_rx->write_ptr);
			len -= FIFO_BUFFER_SIZE - uart_rx->write_ptr;
			uart_rx->write_ptr = 0;
			if(len > uart_rx->read_ptr){
				flag = 1;
			}
		}
		fm_memcpy(uart_rx->fifo_buf + uart_rx->write_ptr, uart_rx->buffer, len);
		uart_rx->write_ptr += len;
		if(flag){
			uart_rx->read_ptr = uart_rx->write_ptr + 1 == FIFO_BUFFER_SIZE ? 0 : uart_rx->write_ptr + 1;
		}
		DMA_SetCurrDataCounter(DMA1_Channel5, DEFAULT_RECEIVE_BUFFER_SIZE);
		DMA_Cmd(DMA1_Channel5, ENABLE);
	}
#else
	u8 data;
	fm_uart_rev_str_nd *uart_rx;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
		uart_rx = &(uart1_control.recv);
		data = USART_ReceiveData(USART1) & 0xff;
		uart_rx->buffer[uart_rx->write_ptr] = data;
		if(++uart_rx->size == DEFAULT_RECEIVE_BUFFER_SIZE){
			uart_rx->size = DEFAULT_RECEIVE_BUFFER_SIZE;
		}
		if(uart_rx->size == DEFAULT_RECEIVE_BUFFER_SIZE){
			uart_rx->read_ptr = uart_rx->write_ptr + 1;
			if(uart_rx->read_ptr == DEFAULT_RECEIVE_BUFFER_SIZE){
				uart_rx->read_ptr = 0;
			}
		}
		if(++uart_rx->write_ptr == DEFAULT_RECEIVE_BUFFER_SIZE){
			uart_rx->write_ptr = 0;
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
#endif
	fm_exit_irq();
}
#endif

#ifdef _USE_FM_UART1_TX_DMA_
static void uart1_tx_DMA_channel_request(void){
	DMA_InitTypeDef DMA_init_str;
	DMA_DeInit(DMA1_Channel4);
	DMA_init_str.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);
	DMA_init_str.DMA_MemoryBaseAddr		= (u32)(uart1_control.send.buffer);
	DMA_init_str.DMA_DIR				= DMA_DIR_PeripheralDST;
	DMA_init_str.DMA_BufferSize			= DEFAULT_SEND_BUFFER_SIZE;
	DMA_init_str.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
	DMA_init_str.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
	DMA_init_str.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_init_str.DMA_MemoryDataSize 	= DMA_MemoryDataSize_Byte;
	DMA_init_str.DMA_Mode 				= DMA_Mode_Normal;
	DMA_init_str.DMA_Priority 			= DMA_Priority_High;
	DMA_init_str.DMA_M2M 				= DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_init_str);
	//DMA_Cmd(DMA1_Channel4, ENABLE);
}
#endif

#ifdef _USE_FM_UART2_RX_DMA_
static void uart2_rx_DMA_channel_request(void){
	DMA_InitTypeDef DMA_init_str;
	DMA_DeInit(DMA1_Channel5);
	DMA_init_str.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);
	DMA_init_str.DMA_MemoryBaseAddr		= (u32)(uart1_control->recv.buffer);
	DMA_init_str.DMA_DIR				= DMA_DIR_PeripheralSRC;
	DMA_init_str.DMA_BufferSize			= DEFAULT_RECEIVE_BUFFER_SIZE;
	DMA_init_str.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
	DMA_init_str.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
	DMA_init_str.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_init_str.DMA_MemoryDataSize 	= DMA_MemoryDataSize_Byte;
	DMA_init_str.DMA_Mode 				= DMA_Mode_Normal;
	DMA_init_str.DMA_Priority 			= DMA_Priority_High;
	DMA_init_str.DMA_M2M 				= DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5, &DMA_init_str);
//	DMA_Cmd(DMA1_Channel5, ENABLE);
}
#endif

#ifdef _USE_FM_UART2_
void USART2_IRQHandler(void){
	fm_enter_irq();
#ifdef _USE_FM_UART2_RX_DMA_
	u16 len, flag;
	fm_uart_rev_str *uart_rx;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET){
		flag = 0;
		len = USART1->SR;
		len = USART1->DR;
		DMA_Cmd(DMA1_Channel5, DISABLE);
		len = DEFAULT_RECEIVE_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
		if(!len){
			DMA_SetCurrDataCounter(DMA1_Channel5, DEFAULT_RECEIVE_BUFFER_SIZE);
			DMA_Cmd(DMA1_Channel5, ENABLE);
			fm_exit_irq();
			return;
		}
		uart_rx = &(uart2_control.recv);
		if(uart_rx->read_ptr == UART_MAGIC){
			uart_rx->read_ptr 	= 0;
			uart_rx->write_ptr 	= 0xffff;
		}
		if(uart_rx->write_ptr < uart_rx->read_ptr && uart_rx->read_ptr < uart_rx->write_ptr + len ){
			flag = 1;
		}
		++uart_rx->write_ptr;
		if(uart_rx->write_ptr + len - 1 >= FIFO_BUFFER_SIZE){
			fm_memcpy(uart_rx->fifo_buf + uart_rx->write_ptr, uart_rx->buffer, FIFO_BUFFER_SIZE - uart_rx->write_ptr);
			len -= FIFO_BUFFER_SIZE - uart_rx->write_ptr;
			uart_rx->write_ptr = 0;
			if(len > uart_rx->read_ptr){
				flag = 1;
			}
		}
		fm_memcpy(uart_rx->fifo_buf + uart_rx->write_ptr, uart_rx->buffer, len);
		uart_rx->write_ptr += len;
		if(flag){
			uart_rx->read_ptr = uart_rx->write_ptr + 1 == FIFO_BUFFER_SIZE ? 0 : uart_rx->write_ptr + 1;
		}
		DMA_SetCurrDataCounter(DMA1_Channel5, DEFAULT_RECEIVE_BUFFER_SIZE);
		DMA_Cmd(DMA1_Channel5, ENABLE);
	}
#else
	u8 data;
	fm_uart_rev_str_nd *uart_rx;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		uart_rx = &(uart2_control.recv);
		data = USART_ReceiveData(USART2) & 0xff;
		if(uart_rx->read_ptr == UART_MAGIC){
			uart_rx->read_ptr = 0;
			uart_rx->write_ptr = 0xffff;
		}
		if(++uart_rx->write_ptr == DEFAULT_RECEIVE_BUFFER_SIZE){
			uart_rx->write_ptr = 0;
		}
		uart_rx->buffer[uart_rx->write_ptr] = data;
		if(uart_rx->write_ptr == uart_rx->read_ptr){
			uart_rx->read_ptr = uart_rx->write_ptr + 1;;
		}
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
#endif
	fm_exit_irq();
}
#endif

#ifdef _USE_FM_UART2_TX_DMA_
static void uart2_tx_DMA_channel_request(void){
	DMA_InitTypeDef DMA_init_str;
	DMA_DeInit(DMA1_Channel4);
	DMA_init_str.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);
	DMA_init_str.DMA_MemoryBaseAddr		= (u32)(uart1_control.send.buffer);
	DMA_init_str.DMA_DIR				= DMA_DIR_PeripheralDST;
	DMA_init_str.DMA_BufferSize			= DEFAULT_SEND_BUFFER_SIZE;
	DMA_init_str.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
	DMA_init_str.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
	DMA_init_str.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_init_str.DMA_MemoryDataSize 	= DMA_MemoryDataSize_Byte;
	DMA_init_str.DMA_Mode 				= DMA_Mode_Normal;
	DMA_init_str.DMA_Priority 			= DMA_Priority_High;
	DMA_init_str.DMA_M2M 				= DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_init_str);
	//DMA_Cmd(DMA1_Channel4, ENABLE);
}
#endif


#ifdef _USE_FM_UART_DMA_
static void DMA_config(void){
#ifdef _USE_FM_UART1_DMA_
#ifdef _USE_FM_UART1_RX_DMA_
	uart1_rx_DMA_channel_request();
#endif
#ifdef _USE_FM_UART1_TX_DMA_
	uart1_tx_DMA_channel_request();
#endif
#endif
#ifdef _USE_FM_UART2_DMA_
#ifdef _USE_FM_UART2_RX_DMA_
	uart2_rx_DMA_channel_request();
#endif
#ifdef _USE_FM_UART2_TX_DMA_
	uart2_tx_DMA_channel_request();
#endif
#endif
#ifdef _USE_FM_UART3_DMA_
#endif
#ifdef _USE_FM_UART4_DMA_
#endif
}
#endif

static void NVIC_config(void){
	NVIC_InitTypeDef NVIC_init_str;
#ifdef _USE_FM_UART1_
	NVIC_init_str.NVIC_IRQChannel = USART1_IRQn;
	NVIC_init_str.NVIC_IRQChannelPreemptionPriority = UART1_PREEMPTION_INTERRUPT_PRI;
	NVIC_init_str.NVIC_IRQChannelSubPriority = UART1_SUB_INTERRUPT_PRI;
	NVIC_init_str.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_init_str);
#endif
#ifdef _USE_FM_UART2_
	NVIC_init_str.NVIC_IRQChannel = USART2_IRQn;
	NVIC_init_str.NVIC_IRQChannelPreemptionPriority = UART1_PREEMPTION_INTERRUPT_PRI;
	NVIC_init_str.NVIC_IRQChannelSubPriority = UART1_SUB_INTERRUPT_PRI;
	NVIC_init_str.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_init_str);
#endif
#ifdef _USE_FM_UART3_
	NVIC_init_str.NVIC_IRQChannel = USART3_IRQn;
	NVIC_init_str.NVIC_IRQChannelPreemptionPriority = UART1_PREEMPTION_INTERRUPT_PRI;
	NVIC_init_str.NVIC_IRQChannelSubPriority = UART1_SUB_INTERRUPT_PRI;
	NVIC_init_str.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_init_str);
#endif
#ifdef _USE_FM_UART4_
#endif
}

static void UART_config(void){
#ifdef _USE_FM_UART_
	USART_InitTypeDef USART_init_str;
	USART_init_str.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_init_str.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
#endif
#ifdef _USE_FM_UART1_
	USART_init_str.USART_BaudRate 	= UART1_BAUDRATE;
	USART_init_str.USART_Parity		= UART1_PARITY;
	USART_init_str.USART_StopBits	= UART1_STOPBITS;
	USART_init_str.USART_WordLength	= UART1_WORDLENGTH;
	USART_Init(USART1, &USART_init_str);
#ifdef _USE_FM_UART1_RX_DMA_
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART1, USART_IT_TC, DISABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
#else
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);
#endif
#ifdef _USE_FM_UART1_TX_DMA_
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
#endif
#endif
#ifdef _USE_FM_UART2_
	USART_init_str.USART_BaudRate 	= UART2_BAUDRATE;
	USART_init_str.USART_Parity		= UART2_PARITY;
	USART_init_str.USART_StopBits	= UART2_STOPBITS;
	USART_init_str.USART_WordLength	= UART2_WORDLENGTH;
	USART_Init(USART2, &USART_init_str);
#ifdef _USE_FM_UART2_RX_DMA_
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART2, USART_IT_TC, DISABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
#else
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);
#endif
#ifdef _USE_FM_UART2_TX_DMA_
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
#endif
#endif
#ifdef _USE_FM_UART3_
	USART_init_str.USART_BaudRate 	= UART3_BAUDRATE;
	USART_init_str.USART_Parity		= UART3_PARITY;
	USART_init_str.USART_StopBits	= UART3_STOPBITS;
	USART_init_str.USART_WordLength	= UART3_WORDLENGTH;
	USART_Init(USART3, &USART_init_str);
#ifdef _USE_FM_UART3_RX_DMA_
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART3, USART_IT_TC, DISABLE);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
#else
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);
#endif
#ifdef _USE_FM_UART1_TX_DMA_
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
#endif
#endif
#ifdef _USE_FM_UART4_
#endif
}

void fm_uart_init(void){
	RCC_config();
	GPIO_config();
	UART_config();
#ifdef _USE_FM_UART_DMA_
	DMA_config();
#endif
	NVIC_config();
	fm_uart_open(FM_SYSTEM_UART);
}

void fm_uart_open(enum fm_uart_port port){
	switch(port){
	case fm_uart1:
#ifdef _USE_FM_UART1_
#ifdef _USE_FM_UART1_RX_DMA_
		DMA_Cmd(DMA1_Channel5, DISABLE);
		DMA_SetCurrDataCounter(DMA1_Channel5, DEFAULT_RECEIVE_BUFFER_SIZE);
		DMA_Cmd(DMA1_Channel5, ENABLE);
#endif
		uart1_control.recv.read_ptr  = 0;
		uart1_control.recv.write_ptr = 0;
		uart1_control.recv.size 	 = 0;
		USART_Cmd(USART1, ENABLE);
#endif
		break;
	case fm_uart2:
#ifdef _USE_FM_UART2_
#ifdef _USE_FM_UART2_RX_DMA_
		DMA_Cmd(DMA1_Channel5, DISABLE);
		DMA_SetCurrDataCounter(DMA1_Channel5, DEFAULT_RECEIVE_BUFFER_SIZE);
		DMA_Cmd(DMA1_Channel5, ENABLE);
#endif
		USART_Cmd(USART2, ENABLE);
#endif
		break;
	case fm_uart3:
#ifdef _USE_FM_UART3_
		USART_Cmd(USART3, ENABLE);
#endif
		break;
	case fm_uart4:
#ifdef _USE_FM_UART4_
#endif
		break;
	default:
		break;
	}
}

void fm_uart_close(enum fm_uart_port port){
	switch(port){
	case fm_uart1:
#ifdef _USE_FM_UART1_
#ifdef _USE_FM_UART1_RX_DMA_
		DMA_Cmd(DMA1_Channel5, DISABLE);
#endif
		USART_Cmd(USART1, DISABLE);
#endif
		break;
	case fm_uart2:
#ifdef _USE_FM_UART2_
		USART_Cmd(USART2, DISABLE);
#endif
		break;
	case fm_uart3:
#ifdef _USE_FM_UART3_
		USART_Cmd(USART2, DISABLE);
#endif
		break;
	case fm_uart4:
#ifdef _USE_FM_UART4_
#endif
		break;
	default:
		break;
	}
}

void fm_uart_write(enum fm_uart_port uart, const void *buf, int size){
	fm_uart_send_str *tx = fm_mem_nullptr;
	switch(uart){
	case fm_uart1:
#ifdef _USE_FM_UART1_
		tx = &(uart1_control.send);
		break;
#else
		return;
#endif
	case fm_uart2:
#ifdef _USE_FM_UART2_
#else
		return;
#endif
		break;
	case fm_uart3:
#ifdef _USE_FM_UART3_
#else
		return;
#endif
		break;
	case fm_uart4:
#ifdef _USE_FM_UART4_
#else
		return;
#endif
		break;
	default:
		break;
	}
	if(tx){
		while(tx->size + size > DEFAULT_SEND_BUFFER_SIZE){
			fm_memcpy(tx->buffer + tx->size, buf, DEFAULT_SEND_BUFFER_SIZE - tx->size);
			tx->size = DEFAULT_SEND_BUFFER_SIZE;
			size -= DEFAULT_SEND_BUFFER_SIZE - tx->size;
			fm_uart_tx_flush(uart);
		}
		fm_memcpy(tx->buffer + tx->size, buf, size);
		tx->size += size;
		if(tx->size >= DEFAULT_FLUSH_SIZE){
			fm_uart_tx_flush(uart);
		}
	}
}

void fm_uart_tx_flush(enum fm_uart_port uart){
	fm_uart_send_str *tx = fm_mem_nullptr;
	int i;
	switch(uart){
	case fm_uart1:
#ifdef _USE_FM_UART1_
		tx = &uart1_control.send;
		if(!tx->size){
			return;
		}
#ifdef _USE_FM_UART1_TX_DMA_
		while(!DMA_GetFlagStatus(DMA1_FLAG_TC4));
		DMA_Cmd(DMA1_Channel4, DISABLE);
		DMA_ClearFlag(DMA1_FLAG_TC4);
		DMA_SetCurrDataCounter(DMA1_Channel4, tx->size);
		DMA_Cmd(DMA1_Channel4, ENABLE);
#else
		for(i = 0; i < tx->size; ++i){
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
			USART_SendData(USART1, tx->buffer[i]);
		}
#endif
		tx->size = 0;
		break;
#else
		return;
#endif
	case fm_uart2:
#ifdef _USE_FM_UART2_
#else
		return;
#endif
		break;
	case fm_uart3:
#ifdef _USE_FM_UART3_
#else
		return;
#endif
		break;
	case fm_uart4:
#ifdef _USE_FM_UART4_
#else
		return;
#endif
		break;
	default:
		break;
	}
}

int fm_uart_read(enum fm_uart_port uart, void *dest, int max_size){
	u16 *begin, *end, *size, available_size = 0, ret_size;
	char *input_buf, *dest_buf = (char*)dest;
	begin = end = fm_mem_nullptr;
	input_buf = fm_mem_nullptr;
	max_size &= 0xffff;
	switch(uart){
	case fm_uart1:
#ifdef _USE_FM_UART1_
#ifdef _USE_FM_UART1_RX_DMA_
		input_buf 	= uart1_control.recv.fifo_buf;
		size 		= FIFO_BUFFER_SIZE;
#else
		input_buf 	= uart1_control.recv.buffer;
#endif
		begin 		= &(uart1_control.recv.read_ptr);
		end 		= &(uart1_control.recv.write_ptr);
		size		= &(uart1_control.recv.size);
		break;
#else
		return 0;
#endif
	case fm_uart2:
#ifdef _USE_FM_UART2_
#else
		return -1;
#endif
		break;
	case fm_uart3:
#ifdef _USE_FM_UART3_
#else
		return -1;
#endif
		break;
	case fm_uart4:
#ifdef _USE_FM_UART4_
#else
		return -1;
#endif
		break;
	default:
		break;
	}
	if(begin){
		if(!*size){
			return 0;
		}
		available_size = *size < max_size ? *size : max_size;
		*size -= available_size;
		ret_size = available_size;
		if(*begin + available_size > DEFAULT_RECEIVE_BUFFER_SIZE){
			fm_memcpy(dest_buf, input_buf + *begin, DEFAULT_RECEIVE_BUFFER_SIZE - *begin);
			dest_buf += DEFAULT_RECEIVE_BUFFER_SIZE - *begin;
			available_size -= DEFAULT_RECEIVE_BUFFER_SIZE - *begin;
			*begin = 0;
		}
		fm_memcpy(dest_buf, input_buf + *begin, available_size);
		*begin += available_size;
		if(*begin == DEFAULT_RECEIVE_BUFFER_SIZE){
			*begin = 0;
		}
		return (int)ret_size;
	}
	return -1;
}

void fm_uart_rx_clear(enum fm_uart_port uart){
	register fm_uint32_t level;
	switch(uart){
	case fm_uart1:
#ifdef _USE_FM_UART1_
		level = _fm_interrupt_disable_();
		uart1_control.recv.read_ptr 	= 0;
		uart1_control.recv.write_ptr 	= 0;
		uart1_control.recv.size 		= 0;
		_fm_interrupt_enable_(level);
		break;
#else
		return;
#endif
	case fm_uart2:
#ifdef _USE_FM_UART2_
#else
		return;
#endif
		break;
	case fm_uart3:
#ifdef _USE_FM_UART3_
#else
		return;
#endif
		break;
	case fm_uart4:
#ifdef _USE_FM_UART4_
#else
		return;
#endif
		break;
	default:
		break;
	}
}

void fm_uart_putchar(enum fm_uart_port uart, char c){
	fm_uart_write(uart, &c, 1);
}

void fm_uart_putmsg(enum fm_uart_port uart, const char *str){
	fm_uart_write(uart, str, (int)fm_strlen(str));
}

char fm_uart_getchar(enum fm_uart_port uart){
	char retc = 0;
	fm_uart_read(uart, &retc, 1);
	return retc;
}

int fm_uart_getmsg(enum fm_uart_port uart, char *dest, char endchar){
	u16 begin, end, size = 0, msg_size;
	char *input_buf;
	begin = end = 0;
	input_buf = fm_mem_nullptr;
	switch(uart){
	case fm_uart1:
#ifdef _USE_FM_UART1_
#ifdef _USE_FM_UART1_RX_DMA_
		input_buf 	= uart1_control.recv.fifo_buf;
		size 		= FIFO_BUFFER_SIZE;
#else
		input_buf 	= uart1_control.recv.buffer;
#endif
		begin 		= uart1_control.recv.read_ptr;
		end 		= uart1_control.recv.write_ptr;
		size		= uart1_control.recv.size;
		break;
#else
		return;
#endif
	case fm_uart2:
#ifdef _USE_FM_UART2_
#else
		return -1;
#endif
		break;
	case fm_uart3:
#ifdef _USE_FM_UART3_
#else
		return -1;
#endif
		break;
	case fm_uart4:
#ifdef _USE_FM_UART4_
#else
		return -1;
#endif
		break;
	default:
		break;
	}
	if(input_buf){
		if(!size){
			return 0;
		}
		u16 ptr = begin;
		for(; ptr != end; ++ptr){
			if(ptr == DEFAULT_RECEIVE_BUFFER_SIZE){
				ptr = 0;
			}
			if(input_buf[ptr] == endchar){
				break;
			}
		}
		if(ptr == end){
			return 0;
		}
		msg_size = ptr + DEFAULT_RECEIVE_BUFFER_SIZE - begin + 1;
		msg_size = msg_size > size ? msg_size - DEFAULT_RECEIVE_BUFFER_SIZE : msg_size;
		if(msg_size < DEFAULT_RECEIVE_BUFFER_SIZE){
			dest[msg_size] = 0;
		}
		return fm_uart_read(uart, dest, msg_size);
	}
	return -1;
}

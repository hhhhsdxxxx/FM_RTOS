#ifndef _FM_PROJECT_CONFIG_
#define _FM_PROJECT_CONFIG_

#define _USE_STM32F10X_

/*
 *	串口通信配置
 */
#define _USE_FM_UART1_
//#define _USE_FM_UART2_
//#define _USE_FM_UART3_
//#define _USE_FM_UART4_

#define _USE_FM_UART_DMA_
#ifdef _USE_FM_UART_DMA_
#ifdef _USE_FM_UART1_
//#define _USE_FM_UART1_DMA_
#endif
#ifdef _USE_FM_UART2_
#define _USE_FM_UART2_DMA_
#endif
#ifdef _USE_FM_UART3_
#define _USE_FM_UART3_DMA_
#endif

#ifdef _USE_FM_UART1_DMA_
//#define _USE_FM_UART1_RX_DMA_
#define _USE_FM_UART1_TX_DMA_
#endif

#ifdef _USE_FM_UART2_DMA_
#define _USE_FM_UART2_RX_DMA_
#define _USE_FM_UART2_TX_DMA_
#endif

#ifdef _USE_FM_UART3_DMA_
#define _USE_FM_UART3_RX_DMA_
#define _USE_FM_UART3_TX_DMA_
#endif

#if defined(_USE_FM_UART1_) || defined(_USE_FM_UART2_) || defined(_USE_FM_UART2_) || defined(_USE_FM_UART2_)
#define _USE_FM_UART_
#endif
#endif

#endif

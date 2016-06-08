#ifndef _FM_LOG_H_
#define _FM_LOG_H_

#ifdef _USE_STM32F10X_
#include "bsp/stm32f103_driver/fm_uart.h"
#endif

/*
 * 重定向输出的默认的串口
 */
void set_default_uart(enum fm_uart_port);

/*
 * 清空串口发送缓冲区（往往配合fm_write使用）
 */
void fm_flush(void);

/*
 * 向串口发送缓冲区写入
 */
void fm_write(const void*, int);

/*
 * 串口输出目标字符串
 */
void fm_print(const char*);

/*
 * 串口输出目标字符串 加上换行符
 */
void fm_puts(const char*);

/*
 * 串口输出目标字符
 */
void fm_putchar(char);

/*
 * 同上一个函数
 */
inline void fm_print_log(const char *str){
	fm_print(str);
}

/*
 * 串口标准输入输出
 * 总字符串不超过64字节
 */
int fm_printf(const char*, ...);

/*
 * 从串口接收缓冲区接收一个字符
 */
char fm_getchar(void);

/*
 * 从串口接收缓冲区读取一串数据
 */
int fm_read(void*, int);

#endif

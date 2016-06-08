#include "bsp/stm32f103_driver/fm_uart.h"
#include "kernel/fm_log.h"
#include "test/fm_assert.h"
#include "kernel/fm_service.h"
#include "kernel/fm_thread.h"

#include "fm_uart_test.h"

static void write_test(void);
static void putchar_test(void);
static void putmsg_test(void);

static int read_test(void);
static int getchar_test(void);
static int getmsg_test(void);

static void write_test(void){
	char words[16] = "123456789abcd\r\n";
	int k = 10;
	while(k--){
		fm_uart_write(FM_SYSTEM_UART, words, 15);
	}
	fm_uart_tx_flush(FM_SYSTEM_UART);
	k = 10;
	while(k--){
		fm_write(words, 15);
	}
	fm_flush();
}

static void putchar_test(void){
	char sentence1[] = "HELLO, WORLD! GO GO GO by fm_uart_putchar!\r\n";
	char sentence2[] = "HELLO, WORLD! GO GO GO by fm_putchar!\r\n";
	const char *p;
	int k = 10;
	while(k--){
		p = sentence1;
		while(*p){
			fm_uart_putchar(FM_SYSTEM_UART, *p);
			++p;
		}
	}
	k = 10;
	while(k--){
		p = sentence2;
		while(*p){
			fm_putchar(*p);
			++p;
		}
	}
}

static void putmsg_test(void){
	char sentence1[] = "HELLO, WORLD! GO GO GO by fm_uart_putmsg!\r\n";
	char sentence2[] = "HELLO, WORLD! GO GO GO by fm_print!\r\n";
	char sentence3[] = "HELLO, WORLD! GO GO GO by fm_puts!";
	int k = 10;
	while(k--){
		fm_uart_putmsg(FM_SYSTEM_UART, sentence1);
	}
	k = 10;
	while(k--){
		fm_print(sentence2);
		fm_puts(sentence3);
	}
}


static int read_test(void){
	char ans[] = "Today is a nice day. I decide to have a date";
	char buf[64];
	fm_thread_sleep(10000);
	fm_uart_read(FM_SYSTEM_UART, buf, 64);
	if(fm_strcmp(ans, buf)){
		return 0;
	}
	fm_memset(buf, 0, 64);
	fm_puts("Read_test1 done!");
	fm_uart_rx_clear(FM_SYSTEM_UART);
	fm_thread_sleep(5000);
	fm_read(buf, 64);
	if(fm_strcmp(ans ,buf))
		return 0;
	fm_puts("Read_test2 done!");
	return 1;
}

static int getchar_test(void){
	 char ans[] = "188xxxx8888";
	 const char *p = ans;
	 char ch;
	 fm_uart_rx_clear(FM_SYSTEM_UART);
	 fm_thread_sleep(10000);
	 while((ch = fm_uart_getchar(FM_SYSTEM_UART))){
		 fm_putchar(ch);
		 if(*(p++) != ch){
			 return 0;
		 }
	 }
	 if(p == ans){
		 return 0;
	 }
	 fm_uart_rx_clear(FM_SYSTEM_UART);
	 fm_puts("Getchar_test1 done!");
	 p = ans;
	 fm_thread_sleep(10000);
	 while((ch = fm_getchar())){
		 fm_putchar(ch);
		 if(*(p++) != ch){
	 		return 0;
		 }
	 }
	 if(p == ans){
		 return 0;
	 }
	 fm_puts("Getchar_test2 done!");
	 return 1;
}

static int getmsg_test(void){
	char ans[] = "Good day, let's have a fun#";
	char buf[64];
	fm_uart_rx_clear(FM_SYSTEM_UART);
	fm_thread_sleep(10000);
	fm_uart_getmsg(FM_SYSTEM_UART, buf, '#');
	if(fm_strcmp(buf, ans))
		return 0;
	fm_puts("Getmsg_test done!");
	return 1;
}

void uart_test(void){
	write_test();
	putchar_test();
	putmsg_test();
	fm_assert(read_test(), "Read test error\r\n");
	fm_assert(getchar_test(), "Get char test error\r\n");
	fm_assert(getmsg_test(), "Get massage test error\r\n");
	fm_puts("Test finish!");
}

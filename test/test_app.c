#include "test/fm_service_test.h"
#include "test/fm_assert.h"
#include "bsp/stm32f103_driver/fm_uart.h"
#include "kernel/fm_thread.h"
#include "kernel/fm_sem.h"
#include "kernel/fm_timer.h"
#include "kernel/fm_service.h"
#include "kernel/fm_mail.h"
#include "kernel/fm_mem.h"
#include "kernel/fm_log.h"
#include "test/test_app.h"
#include "test/fm_rtos_test_config.h"

#if TEST_KEYWORD == 0

void user(void);

fm_thread_t thread;

void user(void){
	service_test();
	fm_print_log("Service test finished!\r\n");
}

void test_main(void){
	fm_thread_create(&thread, user, 2048, 2);
}


#elif TEST_KEYWORD == 10

void user1(void);
void user2(void);
void user3(void);

fm_thread_t thread[3];
fm_sem_t	sem[3];
fm_mutex_t	mutex;

char msg[3][8] = {"User1\r\n", "User2\r\n", "User3\r\n"};

volatile int test_point = 0;
volatile int test_value = 0;
unsigned int value[2] = {0};

void user1(void){
	int k = 10;
	fm_assert(test_point++ == 1, "User1-1\r\n");
	while(k--){
		fm_print(msg[0]);
		fm_thread_sleep(400);
	}
	fm_assert(test_point++ == 3, "User1-2\r\n");
	fm_sem_release(&sem[1]);
	fm_mutex_lock(&mutex);
	test_value = 18;
	fm_thread_sleep(1000);
	fm_mutex_unlock(&mutex);
	fm_assert(test_point++ == 4, "User1-3\r\n");
	fm_sem_request(&sem[0]);
	value[0] = 0xdeadbeef;
	fm_sem_release(&sem[0]);
	fm_assert(test_point++ == 5, "User1-4\r\n");
	fm_print("User1 end!\r\n");
}

void user2(void){
	fm_assert(test_point++ == 2, "User2-1\r\n");
	fm_sem_request(&sem[1]);
	fm_mutex_unlock(&mutex);
	fm_mutex_lock(&mutex);
	fm_assert(test_value == 18, "User2-2\r\n");
	fm_mutex_unlock(&mutex);
	fm_sem_request(&sem[0]);
	value[1] = 0xbeefdead;
	fm_sem_release(&sem[0]);
	fm_assert(test_point++ == 6, "User2-3\r\n");
	fm_print("User2 end!\r\n");
}

void user3(void){
	int k = 10;
	fm_assert(test_point++ == 0, "User3-1\r\n");
	while(k--){
		fm_print(msg[2]);
		fm_thread_sleep(2000);
	}
	fm_mutex_lock(&mutex);
	fm_assert(test_value == 18, "User3-2\r\n");
	fm_mutex_unlock(&mutex);
	fm_assert(value[0] == 0xdeadbeef, "User3-3\r\n");
	fm_assert(value[1] == 0xbeefdead, "User3-4\r\n");
	fm_print("User3 end!\r\n");
}

void test_main(void){
	fm_sem_create(&sem[0], 2, 2);
	fm_sem_create(&sem[1], 0, 1);
	fm_sem_create(&sem[2], 0, 2);
	fm_mutex_create(&mutex);
	fm_thread_create(&thread[0], user1, 256, 4);
	fm_thread_create(&thread[1], user2, 256, 5);
	fm_thread_create(&thread[2], user3, 256, 2);
}

#elif TEST_KEYWORD == 11

fm_thread_t thread[4];

fm_sem_t sem;
fm_mutex_t mutex;

volatile int test_point = 0;

void task_1(void);
void task_2(void);
void task_3(void);
void task_4(void);

void task_1(void){
	fm_thread_sleep(1500);
	fm_mutex_lock(&mutex);
	fm_assert(test_point++ == 1, "TP1 ERROR\r\n");
	fm_mutex_unlock(&mutex);
	fm_sem_request(&sem);
	fm_thread_suspend_self();
	fm_sem_release(&sem);
	fm_assert(test_point++ == 6, "TP6 ERROR\r\n");
}

void task_2(void){
	fm_thread_sleep(1000);
	fm_mutex_lock(&mutex);
	fm_assert(test_point++ == 2, "TP2 ERROR\r\n");
	fm_mutex_unlock(&mutex);
	fm_sem_request(&sem);
	fm_sem_release(&sem);
	fm_assert(test_point++ == 4, "TP5 ERROR\r\n");
}

void task_3(void){
	fm_thread_sleep(500);
	fm_mutex_lock(&mutex);
	fm_assert(test_point++ == 3, "TP4 ERROR\r\n");
	fm_mutex_unlock(&mutex);
	fm_sem_request(&sem);
	fm_sem_release(&sem);
	fm_assert(test_point++ == 5, "TP7 ERROR\r\n");
}

void task_4(void){
	int k = 4000, i = 100000;
	fm_mutex_lock(&mutex);
	fm_assert(test_point++ == 0, "TP0 ERROR\r\n");
	while(k--){
		i = 10000;
		while(i--){
			__asm("NOP");
		}
	}
	fm_assert(running_thread()->priority == 0, "PRIORITY ERROR(!=0)\r\n");
	fm_mutex_unlock(&mutex);
	fm_assert(running_thread()->priority == 3, "PRIORITY ERROR(!=3)\r\n");
	fm_sem_release(&sem);
	fm_sem_release(&sem);
	fm_sem_release(&sem);
	fm_sem_release(&sem);
	fm_sem_release(&sem);
	fm_sem_release(&sem);
	fm_assert(sem->sem.count == sem->sem.max_count, "CNT ERROR\r\n");
	fm_thread_resume(&thread[0]);
	fm_puts("Test finish!!!");
}

void (*entry[])(void) = {
	task_1,
	task_2,
	task_3,
	task_4
	};


void test_main(void){
	int i;
	fm_sem_create(&sem, 1, 3);
	fm_mutex_create(&mutex);
	for(i = 0; i < 4; ++i){
		fm_thread_create(&thread[i], entry[i], 256, i & 0x1f);
	}
}



#elif TEST_KEYWORD == 20

void user1(void);
void user2(void);
void user3(void);

fm_thread_t thread[3];

volatile int test_point = 0;

void user1(void){
	fm_thread_change_priority_self(16);
	fm_assert(test_point++ == 2, "User1-1\r\n");
	fm_puts("User1 end!");
}

void user2(void){
	fm_assert(test_point++ == 0, "User2-1\r\n");
	fm_thread_change_priority(&thread[2], 2);
	fm_assert(test_point++ == 4, "User2-2\r\n");
	fm_puts("User2 end!");
}

void user3(void){
	fm_assert(test_point++ == 1, "User3-1\r\n");
	fm_thread_change_priority(&thread[0], 1);
	fm_assert(test_point++ == 3, "User3-2\r\n");
	fm_puts("User3 end!");
}

void test_main(void){
	fm_thread_create(&thread[0], user1, 256, 4);
	fm_thread_create(&thread[1], user2, 256, 4);
	fm_thread_create(&thread[2], user3, 256, 4);
}

#elif TEST_KEYWORD == 30

fm_thread_t thread[3];

char error_info[] = "Out of memory\r\n";

void user1(void);
void user2(void);
void user3(void);

void user1(void){
	char msg[32] = "This is user1";
	fm_mail_handler rec;
	fm_assert(fm_send_mail(&thread[2], msg, fm_strlen(msg) + 1), error_info);
	fm_assert(fm_send_mail(&thread[2], msg, fm_strlen(msg) + 1), error_info);
	while(!(rec = fm_receive_mail_from_thread(&thread[2]))){
		fm_thread_sleep(1000);
	}
	fm_printf("User1: Receive from thread%d : %s\r\n", rec->from->tid,(char*)rec->content);
	fm_delete_mail(rec);
	fm_puts("User1 end!");
}

void user2(void){
	char msg[32] = "This is user2";
	fm_mail_handler rec;
	fm_assert(fm_send_mail(&thread[2], msg, fm_strlen(msg) + 1), error_info);
	fm_assert(fm_send_mail(&thread[2], msg, fm_strlen(msg) + 1), error_info);
	while(!(rec = fm_receive_mail_from_thread(&thread[2]))){
		fm_thread_sleep(1000);
	}
	fm_printf("User2: Receive from thread%d : %s\r\n", rec->from->tid,(char*)rec->content);
	fm_clear_mailbox();
	fm_assert(!fm_has_mail(), "TP3 error\r\n");
	fm_puts("User2 end!");
}

void user3(void){
	char msg[32] = "Mail from user3";
	fm_mail_handler rec;
	if(fm_has_mail()){
		rec = fm_receive_mail();
		fm_assert(rec != fm_mem_nullptr, "NULLPTR\r\n");
		fm_assert(!fm_strcmp((char*)rec->content, "This is user1"), "TP1 error\r\n");
		fm_printf("Get: %s\r\n", (char*)rec->content);
		fm_delete_mail(rec);
	}
	rec = fm_receive_mail_from_thread(&thread[1]);
	fm_assert(rec != fm_mem_nullptr, "NULLPTR\r\n");
	fm_assert(!fm_strcmp((char*)rec->content, "This is user2"), "TP2 error\r\n");
	fm_printf("Get: %s\r\n", (char*)rec->content);
	fm_delete_mail(rec);
	while(fm_has_mail()){
		rec = fm_receive_mail();
		fm_assert(rec != fm_mem_nullptr, "NULLPTR\r\n");
		fm_printf("Get: %s\r\n", (char*)rec->content);
		fm_delete_mail(rec);
	}
	fm_send_mail(&thread[0], msg, fm_strlen(msg)+1);
	fm_send_mail(&thread[1], msg, fm_strlen(msg)+1);
	fm_puts("User3 end!");
}

void test_main(void){
	fm_thread_create(&thread[0], user1, 512, 1);
	fm_thread_create(&thread[1], user2, 512, 2);
	fm_thread_create(&thread[2], user3, 512, 3);
}

#elif TEST_KEYWORD == 40

fm_thread_t thread[3];

void user1(void);
void user2(void);
void user3(void);
void set_state(void);
void change_priority(void);
void user3_set_state(void);
void time_out_set(void);

volatile int state = 0, test_point = 0;

#define TEST_TIMER_NUMBER 10

volatile int user3_state = 0;
fm_timer_handler timers[TEST_TIMER_NUMBER];

volatile fm_time_tick begin, total;

void set_state(void){
	state = 1;
}

void time_out_set(void){
	fm_puts("Time out!!!");
	total = get_system_ticks() - begin;
	fm_printf("total: %d\r\n", total);
}

void change_priority(void){
	fm_thread_change_priority(&thread[1], 2);
}

void user3_set_state(void){
	user3_state = 1;
}

void user1(void){
	fm_timer_handler timer = fm_timer_request(1000, set_state);
	int k = 10;
	while(k--){
		fm_timer_set_newticks(timer, (k+1) * 100);
		fm_timer_start(timer);
		while(!state){}
		state = 0;
		fm_puts("This user1");
	}
	fm_timer_release(timer);
	timer = fm_timer_request(1000, change_priority);
	fm_timer_start(timer);
	fm_assert(test_point++ == 0, "TP0 error\r\n");
	while(!state){}
	fm_assert(test_point++ == 2, "TP2 error\r\n");
	fm_timer_release(timer);
	fm_puts("User1 end!");
}

void user2(void){
	fm_timer_handler timer = fm_timer_request(1000, set_state);
	fm_timer_start(timer);
	while(!state){}
	fm_assert(test_point++ == 1, "TP1 error\r\n");
	fm_timer_release(timer);
	fm_puts("User2 end!");
}

void user3(void){
	int i;
	fm_timer_handler timeout_timer = fm_timer_request(10000, time_out_set);
	fm_timer_start(timeout_timer);
	begin = get_system_ticks();
	for(i = 0; i < TEST_TIMER_NUMBER; ++i){
		timers[i] = fm_timer_request((i+1) * 100, user3_set_state);
		fm_assert(timers[i] != fm_mem_nullptr, "NULLPTR\r\n");
	}
	fm_timer_pause(timeout_timer);
	for(i = 0; i < TEST_TIMER_NUMBER; ++i){
		fm_timer_start(timers[i]);
		while(!user3_state){}
		fm_puts("User3");
		fm_timer_release(timers[i]);
		user3_state = 0;
	}
	fm_timer_resume(timeout_timer);
	fm_puts("User3 end!");
}

void test_main(void){
	fm_thread_create(&thread[0], user1, 512, 3);
	fm_thread_create(&thread[1], user2, 512, 5);
	fm_thread_create(&thread[2], user3, 512, 16);
}

#elif TEST_KEYWORD == 50

fm_thread_t thread[3];

void user1(void);
void user2(void);
void user3(void);

volatile int test_point = 0;
volatile int test_value = 0;
fm_mutex_t mutex;

void user1(void){
	fm_thread_sleep(2000);
	fm_assert(test_point++ == 0, "TP0 error\r\n");
	fm_mutex_lock(&mutex);
	fm_assert(test_value == 1, "Test_value error\r\n");
	fm_mutex_unlock(&mutex);
	fm_puts("User1 end!");
}

void user2(void){
	fm_thread_sleep(500);
	int k = 100000, j = 1000;
	while(j--){
		k = 100000;
		while(k--){
			__asm("NOP");
		}
	}
	fm_assert(test_point++ == 2, "TP2 error\r\n");
	fm_puts("User2 end!");
}

void user3(void){
	fm_mutex_lock(&mutex);
	test_value = 1;
	fm_thread_sleep(3000);
	fm_assert(test_point++ == 1, "TP1 error\r\n");
	fm_mutex_unlock(&mutex);
	fm_assert(test_point++ == 3, "TP3 error\r\n");
	fm_puts("User3 end!");
}
void test_main(void){
	fm_mutex_create(&mutex);
	fm_thread_create(&thread[0], user1, 256, 1);
	fm_thread_create(&thread[1], user2, 256, 4);
	fm_thread_create(&thread[2], user3, 256, 16);
}

#elif TEST_KEYWORD == 60

#define TEST_TASK_NUM 8

fm_thread_t thread[TEST_TASK_NUM];


void user(void);
void another_user(void);

void user(void){
	fm_printf("This is thread %04d begin!\r\n", running_thread()->tid);
	fm_printf("This is thread %04d end!\r\n", running_thread()->tid);
}

void another_user(void){
	fm_printf("This is thread %04d begin!\r\n", running_thread()->tid);
	int i = 0;
	for(; i < TEST_TASK_NUM - 1; ++i){
		fm_thread_wake(&thread[i]);
	}
	fm_printf("This is thread %04d end!\r\n", running_thread()->tid);
}

void test_main(void){
	int i = 0;
	for(; i < TEST_TASK_NUM - 1; ++i){
		fm_thread_create(&thread[i], user, 256, i);
	}
	fm_thread_create(&thread[TEST_TASK_NUM - 1], another_user, 256, TEST_TASK_NUM);
}

#elif TEST_KEYWORD == 70

#define SWITCH_ROUNDS 	50000
#define TEST_TASK_NUM	20

fm_thread_t thread[TEST_TASK_NUM], main_thread;

void task(void);
void main_task(void);

volatile int step = 0;

void task(void){
	while(1){
		//fm_printf("This is %d\r\n", running_thread()->tid);
		//fm_puts("1111");
		fm_thread_suspend_self();
	}
}

void main_task(void){
	int i;
	fm_time_tick begin = get_system_ticks(), end;
	while(step < SWITCH_ROUNDS){
		for(i = 0; i < TEST_TASK_NUM; ++i){
			fm_thread_resume(&thread[i]);
		}
		step++;
	}
	end = get_system_ticks();
	fm_printf("Time consume: %d\r\n", end - begin);
}

void test_main(void){
	fm_assert(fm_thread_create(&main_thread, main_task, 256, 10), "CREATE ERROR");
	int i = 0;
	for(; i < TEST_TASK_NUM; ++i){
		fm_assert(fm_thread_create(&thread[i], task, 256, 5), "CREATE_ERROR");
	}
}

#elif TEST_KEYWORD == 80

fm_thread_t thread[2];

void task(void);

void task(void){
	uart_test();
}

void test_main(void){
	fm_thread_create(&thread[0], task, 1024, 2);
}

#endif

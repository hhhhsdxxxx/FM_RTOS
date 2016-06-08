#include "kernel/fm_start.h"
#include "app/app.h"
#include "project_start.h"
#include "fm_project_config.h"
#include "kernel/fm_log.h"

void board_start(void){
	fm_uart_init();
	fm_print_log("Kernel initialing:\r\n");
	fm_rtos_kernel_init();
	fm_print_log("Tasks initialing...\r\n");
	app_main();
	fm_print_log("Tasks running...\r\n");
	fm_rtos_task_start();
}

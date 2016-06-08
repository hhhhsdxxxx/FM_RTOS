/*
 * 内核功能配置
 */
#ifndef _FM_CONFIG_H_
#define _FM_CONFIG_H_

/*
 * 使用计数信号量
 */
#define _USE_FM_SEM_

/*
 * 使用互斥信号量
 */
#define _USE_FM_MUTEX_

/*
 * 允许线程在运行中更改优先级
 */
#define _USE_FM_THREAD_CHANGE_PRIORITY_
#define _USE_FM_THREAD_SLEEP_

/*
 * 使用内存debug信息
 */
//#define _USE_FM_OPENOCD_DEBUG_LOG_

/*
 * 默认的线程栈大小
 */
#define DEFAULT_STACK_SIZE 		512

/*
 * 使用定时器
 */
#define _USE_FM_TIMER_
#define _USE_FM_DYNAMIC_TIMER_

/*
 * 使用堆内存分配
 */
#define _USE_FM_MEM_

/*
 * 使用内存初始化和拷贝操作
 */
#define _USE_FM_MEM_SERVICE_
#define _USE_FM_MEM_SAFE_SERVICE_

/*
 * 使用字符串操作
 */
#define _USE_FM_STRING_SERVICE_
#define _USE_FM_STRING_SAFE_SERVICE_
#define _ENABLE_STRING_UNSAFE_
#define _ENABLE_STRSTR_KMP_

/*
 * 格式化输入输出
 */
#define _USE_FM_STDIO_SERVICE_

/*
 * mail
 */
#define _USE_FM_MAIL_

#define TICKS_PER_SECOND 1000

#endif

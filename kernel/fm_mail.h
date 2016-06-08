#ifndef _FM_MAIL_H_
#define _FM_MAIL_H_

#include "fm_config.h"

#ifdef _USE_FM_MAIL_

struct fm_rtos_thread_struct;
typedef struct fm_rtos_thread_struct fm_tcb;
typedef fm_tcb* fm_thread_t;

typedef unsigned long fm_mail_size;

typedef struct fm_mail_struct{
	void *content;
	fm_mail_size size;
	fm_tcb *from;
} fm_mail;

typedef fm_mail*	fm_mail_handler;

typedef struct fm_mail_control_block{
	fm_mail mail;
	struct fm_mail_control_block *next, *prev;
} fm_mail_block;

typedef struct _fm_mail_list{
	fm_mail_block *head, *tail;
} fm_mail_box;

/*
 * 发送邮件
 * 目标任务
 * 内容首地址
 * 内容长度
 * 返回 1 成功 0 失败
 */
int fm_send_mail(fm_thread_t *target, const void *msg, fm_mail_size size);

/*
 * 接收收到的最早一封邮件
 * 返回 邮件控制句柄
 */
fm_mail_handler fm_receive_mail(void);

/*
 * 接收指定任务发来的最早的一封邮件
 * 返回 邮件控制句柄
 */
fm_mail_handler fm_receive_mail_from_thread(fm_thread_t *target);

/*
 * 当前任务邮箱中是否有邮件
 */
int fm_has_mail(void);

/*
 * 删除收到的邮件
 * 邮件控制句柄
 * 当失去当前邮件句柄的控制（比如接收新的邮件），请务必执行该函数，否则会内存泄漏
 */
void fm_delete_mail(fm_mail_handler mail);

/*
 * 清空当前任务所有未读的邮件
 */
void fm_clear_mailbox(void);

#endif

#endif

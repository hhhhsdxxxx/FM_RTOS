#include "fm_config.h"

#ifdef _USE_FM_MAIL_

#include "fm_mail.h"
#include "fm_mem.h"
#include "fm_thread.h"
#include "fm_service.h"
#include <kernel/fm_stdtype.h>

extern fm_tcb *usr_run;
extern fm_uint32_t _fm_interrupt_disable_(void);
extern void _fm_interrupt_enable_(fm_uint32_t);

static void _push_mail_box(fm_mail_box *box, fm_mail_block *fmb);
static void _pop_mail_box(fm_mail_box *box, fm_mail_block *fmb);

#define nullptr		(void*)0

static void _push_mail_box(fm_mail_box *box, fm_mail_block *fmb){
	if(!box->head){
		box->head = fmb;
	} else {
		box->tail->next = fmb;
	}
	fmb->prev = box->tail;
	box->tail = fmb;
}

static void _pop_mail_box(fm_mail_box *box, fm_mail_block *fmb){
	if(!fmb->prev){
		box->head = fmb->next;
	} else {
		fmb->prev->next = fmb->next;
	}
	if(!fmb->next){
		box->tail = fmb;
	} else {
		fmb->next->prev = fmb->prev;
	}
	fmb->next = fmb->prev = nullptr;
}

int fm_send_mail(fm_thread_t *target, const void *msg, fm_mail_size size){
	fm_tcb *dest = *target;
	void *p = fm_malloc(sizeof(fm_mail_block)+size);
	if(!p){
		return 0;
	}
	fm_mail_block *control = (fm_mail_block*)p;
	control->next = control->prev = nullptr;
	control->mail.content = (void*)((char*)p + sizeof(fm_mail_block));
	control->mail.from    = usr_run;
	control->mail.size    = size;
	fm_memcpy(control->mail.content, msg, size);
	fm_uint32_t level = _fm_interrupt_disable_();
	_push_mail_box(&(dest->mailbox), control);
	_fm_interrupt_enable_(level);
	return 1;
}

int fm_has_mail(void){
	return usr_run->mailbox.head != nullptr;
}

fm_mail_handler fm_receive_mail(void){
	fm_mail_handler ret = nullptr;
	if(usr_run->mailbox.head){
		fm_uint32_t level = _fm_interrupt_disable_();
		fm_mail_block *fmc = usr_run->mailbox.head;
		_pop_mail_box(&(usr_run->mailbox), fmc);
		ret = &(fmc->mail);
		_fm_interrupt_enable_(level);
	}
	return ret;
}

fm_mail_handler fm_receive_mail_from_thread(fm_thread_t *target){
	fm_mail_handler ret = nullptr;
	fm_tcb *src = *target;
	fm_mail_block *ptr = usr_run->mailbox.head;
	while(ptr){
		if(ptr->mail.from == src){
			ret = &(ptr->mail);
			fm_uint32_t level = _fm_interrupt_disable_();
			_pop_mail_box(&(usr_run->mailbox), ptr);
			_fm_interrupt_enable_(level);
			return ret;
		}
		ptr = ptr->next;
	}
	return ret;
}



void fm_delete_mail(fm_mail_handler mail){
	fm_free(mail);
}

void fm_clear_mailbox(void){
	fm_mail_block *ptr, *next;
	ptr = usr_run->mailbox.head;
	fm_uint32_t level = _fm_interrupt_disable_();
	while(ptr){
		next = ptr->next;
		fm_free((void*)ptr);
		ptr = next;
	}
	usr_run->mailbox.head = usr_run->mailbox.tail = nullptr;
	_fm_interrupt_enable_(level);
}

#undef nullptr

#endif

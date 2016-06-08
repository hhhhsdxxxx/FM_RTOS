#ifndef _FM_MEMORY_MANAGER_H_
#define _FM_MEMORY_MANAGER_H_

#include "fm_config.h"

#ifndef _USE_FM_MEM_
#include <stdlib.h>
#endif

#ifdef _USE_FM_MEM_

#include <kernel/fm_stdtype.h>

typedef unsigned long 	fm_mem_size_t;
typedef fm_uint8_t* 		fm_mem_ptr;

typedef struct fm_mem_control_block{
	fm_uint16_t magic;
	fm_uint16_t used;
	fm_mem_size_t next, prev;
} fm_mcb;

#define MIN_MEM_SIZE 12
#define MEM_MAGIC 	0xdead

#define MEM_ALIGAN_UP(x)	((((fm_uint32_t)(x))+3)&(~0x3))
#define MEM_ALIGAN_DOWN(x)	(((fm_uint32_t)(x))&(~0x3))

#define SIZE_OF_STRUCT 		MEM_ALIGAN_UP(sizeof(fm_mcb))

#define fm_mem_nullptr		(void*)0

#ifdef NULL
#undef NULL
#endif

#define NULL 				fm_mem_nullptr

/*
 * 初始化堆空间结构
 */
void fm_memheap_init(fm_mem_ptr, fm_mem_ptr);

/*
 * 申请空间
 */
void *fm_malloc(fm_mem_size_t);

/*
 *	释放空间
 */
void fm_free(void *);

/*
 * 调整申请空间，可能失败
 */
void *fm_realloc(void *, fm_mem_size_t);

/*
 * 申请空间并初始化
 */
void *fm_calloc(fm_mem_size_t, fm_mem_size_t);

#else

inline void *fm_malloc(unsigned long size){
	return malloc(size);
}

inline void fm_free(void *mem){
	free(mem);
}

#endif

enum object_type {
	type_fm_tcb = 0,
	type_fm_sem,
	type_fm_mutex,
	type_fm_dynamic_timer,
	type_error
};

void *fm_object_malloc(enum object_type);

#endif

#include "fm_config.h"
#include "fm_mem.h"
#include "fm_thread.h"
#include "fm_timer.h"
#include "fm_int.h"
#include "fm_service.h"

#if defined(_USE_FM_SEM_) || defined(_USE_FM_MUTEX_)
#include "fm_sem.h"
#endif

#ifndef _USE_FM_MEM_
#include <stdlib.h>
#else

#include "fm_sem.h"

static fm_mem_ptr heap_ptr;
static fm_mcb *free_ptr;
static fm_mcb *heap_end;
static fm_mem_size_t mem_available;
static fm_mutex mem_mutex;

static void plug_holes(fm_mcb *);

static void plug_holes(fm_mcb *mem){
	fm_mcb *nmem = (fm_mcb*)&heap_ptr[mem->next];
	if(mem != nmem && !nmem->used){
		if(nmem == free_ptr){
			free_ptr = mem;
		}
		mem->next = nmem->next;
		if(nmem->next != mem_available + SIZE_OF_STRUCT)
			((fm_mcb*)&heap_ptr[nmem->next])->prev = (fm_mem_size_t)((fm_mem_ptr)mem - heap_ptr);
	}
	fm_mcb *pmem = (fm_mcb*)&heap_ptr[mem->prev];
	if(pmem != mem && !pmem->used){
		if(free_ptr == mem){
			free_ptr = pmem;
		}
		pmem->next = mem->next;
		if(mem->next != mem_available + SIZE_OF_STRUCT)
			((fm_mcb*)&heap_ptr[mem->next])->prev = (fm_mem_size_t)((fm_mem_ptr)pmem - heap_ptr);
	}

}

void fm_memheap_init(fm_mem_ptr begin, fm_mem_ptr end){
	fm_uint32_t start 		= MEM_ALIGAN_UP(begin);
	fm_uint32_t last		= MEM_ALIGAN_DOWN(end);
	if(get_interrupt_nest()){
		return;
	}
	if(last > 2 * (SIZE_OF_STRUCT) && last - 2 * (SIZE_OF_STRUCT) >= start){
		mem_available = last - start - 2 * SIZE_OF_STRUCT;
	} else {
		return;
	}
	heap_ptr 	= (fm_mem_ptr)start;
	fm_mcb *mem = (fm_mcb *)heap_ptr;
	mem->magic 	= MEM_MAGIC;
	mem->used 	= 0;
	mem->next 	= mem_available + SIZE_OF_STRUCT;
	mem->prev	= 0;

	heap_end 		= (fm_mcb *)&heap_ptr[mem->next];
	heap_end->magic = MEM_MAGIC;
	heap_end->used	= 1;
	heap_end->next	= mem_available + SIZE_OF_STRUCT;
	heap_end->prev	= mem_available + SIZE_OF_STRUCT;
	fm_mutex_init(&mem_mutex);
	free_ptr 	= (fm_mcb *)heap_ptr;
}

void *fm_malloc(fm_mem_size_t size){
	if(get_interrupt_nest() || !size){
		return fm_mem_nullptr;
	}
	fm_mem_size_t alloced = MEM_ALIGAN_UP(size);
	if(alloced < MIN_MEM_SIZE){
		alloced = MIN_MEM_SIZE;
	}
	if(alloced > mem_available){
		return fm_mem_nullptr;
	}
	fm_mutex_iner_lock(&mem_mutex);
	fm_mem_size_t ptr = 0;
	fm_mcb *mem;
	for(ptr = (fm_mem_size_t)((fm_mem_ptr)free_ptr - heap_ptr);
		ptr < mem_available - alloced;
		ptr = ((fm_mcb*)&heap_ptr[ptr])->next){
		mem = (fm_mcb*)&heap_ptr[ptr];
		if(!mem->used && mem->next - (ptr+SIZE_OF_STRUCT) >= alloced){
			if(mem->next - (ptr + SIZE_OF_STRUCT) >= alloced + SIZE_OF_STRUCT + MIN_MEM_SIZE){
				fm_mem_size_t ptr2  = ptr + SIZE_OF_STRUCT + alloced;
				fm_mcb *mem2 		= (fm_mcb*)&heap_ptr[ptr2];
				mem2->used			= 0;
				mem2->magic			= MEM_MAGIC;
				mem2->next			= mem->next;
				mem2->prev			= ptr;

				mem->next			= ptr2;
				mem->used			= 1;
				if(mem2->next != mem_available + SIZE_OF_STRUCT){
					((fm_mcb*)&heap_ptr[mem2->next])->prev = ptr2;
				}
			} else {
				mem->used = 1;
			}
			mem->magic = MEM_MAGIC;
			if(mem == free_ptr){
				while(free_ptr->used && free_ptr != heap_end){
					free_ptr = (fm_mcb*)&heap_ptr[free_ptr->next];
				}
			}
			fm_mutex_iner_unlock(&mem_mutex);
			return (void*)((fm_mem_ptr)mem + SIZE_OF_STRUCT);
		}
	}
	fm_mutex_iner_unlock(&mem_mutex);
	return fm_mem_nullptr;
}

void fm_free(void *rmem){
	if(get_interrupt_nest()){
		return;
	}
	if(rmem == fm_mem_nullptr){
		return;
	}
	/* out of bound */
	if((fm_mem_ptr)rmem < heap_ptr + SIZE_OF_STRUCT || (fm_mem_ptr)rmem >= (fm_mem_ptr)heap_end){
		return;
	}
	fm_mcb *mem = (fm_mcb*)((fm_mem_ptr)rmem - SIZE_OF_STRUCT);
	if(mem->magic != MEM_MAGIC || !mem->used){
		return;
	}
	fm_mutex_iner_lock(&mem_mutex);
	mem->used 	= 0;
	mem->magic 	= MEM_MAGIC;
	if(mem < free_ptr){
		free_ptr = mem;
	}
	plug_holes(mem);
	fm_mutex_iner_unlock(&mem_mutex);
}

void *fm_realloc(void *rmem, fm_mem_size_t new_size){
	if(get_interrupt_nest()){
		return fm_mem_nullptr;
	}
	new_size = MEM_ALIGAN_UP(new_size);
	if(new_size > mem_available){
		return fm_mem_nullptr;
	}
	if(!rmem){
		return fm_malloc(new_size);
	}
	fm_mutex_iner_lock(&mem_mutex);
	if((fm_mem_ptr)rmem < heap_ptr + SIZE_OF_STRUCT || (fm_mem_ptr)rmem >= (fm_mem_ptr)heap_end){
		fm_mutex_iner_unlock(&mem_mutex);
		return rmem;
	}
	fm_mcb *mem 		= (fm_mcb*)((fm_mem_ptr)rmem - SIZE_OF_STRUCT);
	fm_mem_size_t ptr 	= (fm_mem_size_t)((fm_mem_ptr)mem - heap_ptr);
	fm_mem_size_t size	= mem->next - ptr - SIZE_OF_STRUCT;
	if(size == new_size){
		fm_mutex_iner_unlock(&mem_mutex);
		return rmem;
	}
	if(new_size + SIZE_OF_STRUCT + MIN_MEM_SIZE < size){
		fm_mem_size_t ptr2 = ptr + new_size + SIZE_OF_STRUCT;
		fm_mcb *mem2 = (fm_mcb*)&heap_ptr[ptr2];
		mem2->magic = MEM_MAGIC;
		mem2->used 	= 0;
		mem2->prev	= ptr;
		mem2->next	= mem->next;
		mem->next	= ptr2;
		if(mem->next != mem_available + SIZE_OF_STRUCT)
			((fm_mcb*)&heap_ptr[mem->next])->prev = ptr2;
		plug_holes(mem2);
		fm_mutex_iner_unlock(&mem_mutex);
		return rmem;
	}
	fm_mutex_iner_unlock(&mem_mutex);
	void *nmem = fm_malloc(new_size);
	if(nmem){
		fm_memcpy(nmem, rmem, new_size > size ? size : new_size);
		fm_free(rmem);
	}
	return nmem;
}

void *fm_calloc(fm_mem_size_t count, fm_mem_size_t size){
	if(get_interrupt_nest()){
		return fm_mem_nullptr;
	}
	void *p = fm_malloc(count * size);
	if(p){
		fm_memset(p, 0, count * size);
	}
	return p;
}

#endif

void *fm_object_malloc(enum object_type type){
	switch(type){
	case type_fm_tcb:
		return (void*)fm_malloc(sizeof(fm_tcb));
	case type_fm_sem:
#ifdef _USE_FM_SEM_
		return (void*)fm_malloc(sizeof(_fm_sem_t));
#else
		return (void*)0;
#endif
	case type_fm_mutex:
#ifdef _USE_FM_MUTEX_
		return (void*)fm_malloc(sizeof(_fm_mutex_t));
#else
		return (void*)0;
#endif
	case type_fm_dynamic_timer:
#ifdef _USE_FM_DYNAMIC_TIMER_
		return (void*)fm_malloc(sizeof(fm_dynamic_timer));
#else
		return (void*)0;
#endif
	case type_error:
		return (void*)0;
	}
	return (void*)0;
}

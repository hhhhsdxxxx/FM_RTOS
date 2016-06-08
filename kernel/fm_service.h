#ifndef _FM_SERVICE_H_
#define _FM_SERVICE_H_

#include "fm_config.h"

#ifdef _USE_FM_MEM_SERVICE_

#include "fm_mem.h"

void *fm_memset(void *dest, char c, fm_mem_size_t size);
void *fm_memcpy(void *dest, const void *src, fm_mem_size_t size);
void *fm_memmove(void *dest, const void *src, fm_mem_size_t size);
int fm_memcmp(const void *dest, const void *src, fm_mem_size_t size);

#ifdef _USE_FM_MEM_SAFE_SERVICE_

void *fm_memset_s(void *dest, fm_mem_size_t dest_size, char c, fm_mem_size_t size);
void *fm_memcpy_s(void *dest, fm_mem_size_t dest_size, void *src, fm_mem_size_t size);
void *fm_memmove_s(void *dest, fm_mem_size_t dest_size, void *src, fm_mem_size_t size);

#endif

#else

#define fm_memset		memset
#define fm_memcpy		memcpy
#define fm_memmove		memmove

#endif

#ifdef _USE_FM_STRING_SERVICE_

typedef fm_mem_size_t 	fm_string_size;

int fm_strcmp(const char *, const char *);
int fm_strncmp(const char *, const char *, fm_string_size);
int fm_strcasecmp(const char *, const char *);
fm_string_size fm_strlen(const char *);
char *fm_strdup(const char *);
char *fm_strncpy(char *, const char *, fm_string_size);
char *fm_strncat(char *, const char *, fm_string_size);
char *fm_strstr(const char *, const char*);
#ifdef _ENABLE_STRSTR_KMP_
void fm_kmp_init(int *, int, const char*);
char *fm_strstr_kmp_struct(const char*, int, const char*, int, int *);
char *fm_strstr_kmp(const char *, const char *);
#endif
#ifdef _ENABLE_STRING_UNSAFE_
char *fm_strcpy(char *, const char *);
char *fm_strcat(char *, const char *);
#endif
#endif

#ifdef _USE_FM_STDIO_SERVICE_

typedef __builtin_va_list	__gnuc_va_list;
typedef __gnuc_va_list		va_list;
#define va_start(v, l)		__builtin_va_start(v, l)
#define va_end(v)			__builtin_va_end(v)
#define va_arg(v, l)		__builtin_va_arg(v, l)

typedef unsigned long 		fm_number_t;
int fm_atoi(char*);
int fm_vsnprintf(char *buf, fm_mem_size_t size, const char *fmt, va_list args);
int fm_snprintf(char *buf, fm_mem_size_t size, const char *fmt, ...);
#endif

#endif

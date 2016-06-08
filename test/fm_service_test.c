#include "kernel/fm_service.h"
#include "kernel/fm_mem.h"
#include "test/fm_assert.h"
#include <stm32f10x.h>
#include "bsp/stm32f103_driver/fm_uart.h"

static int memset_test_1(void);
static int memset_test_2(void);
static int memset_test_3(void);
static int memcpy_test(void);
static int memmove_test(void);
static int memcmp_test(void);
static int strcmp_test(void);
static int strncmp_test(void);
static int strcasecmp_test(void);
static int strdup_test(void);
static int strlen_test(void);
static int strncat_test(void);
static int strncpy_test(void);
static int strstr_test(void);
static int kmp_test(void);
static int atoi_test(void);
static int snprintf_test(void);

void service_test(void){
	fm_assert(memset_test_1(), "Memset_test1 error\r\n");
	fm_assert(memset_test_2(), "Memset_test2 error\r\n");
	fm_assert(memset_test_3(), "Memset_test3 error\r\n");
	fm_assert(memcpy_test(), "Memcpy_test error\r\n");
	fm_assert(memmove_test(), "Memmove_test error\r\n");
	fm_assert(memcmp_test(), "Memcmp_test error\r\n");
	fm_assert(strcmp_test(), "Strcmp_test error\r\n");
	fm_assert(strncmp_test(), "Strncmp_test error\r\n");
	fm_assert(strcasecmp_test(), "Strcasecmp_test error\r\n");
	fm_assert(strdup_test(), "Strdup_test error\r\n");
	fm_assert(strlen_test(), "Strlen_test error\r\n");
	fm_assert(strncat_test(), "Strncat_test error\r\n");
	fm_assert(strncpy_test(), "Strncpy_test error\r\n");
	fm_assert(strstr_test(), "Strstr_test error\r\n");
	fm_assert(kmp_test(), "Kmp_test error\r\n");
	fm_assert(atoi_test(), "Atoi_test error\r\n");
	fm_assert(snprintf_test(), "Strnprintf_test error\r\n");
}

static int memset_test_1(void){
	char buf[64];
	int i;
	for(i = 0; i < 64; ++i){
		buf[i] = i;
	}
	fm_memset(buf + 3, 1, 61);
	for(i = 3; i < 64; ++i){
		if(buf[i] != 1){
			return 0;
		}
	}
	return 1;
}

static int memset_test_2(void){
	char buf[64];
	int i;
	for(i = 0; i < 64; ++i){
		buf[i] = i;
	}
	fm_memset(buf, 256, 64);
	for(i = 0; i < 64; ++i){
		if(buf[i] != 0){
			return 0;
		}
	}
	return 1;
}

static int memset_test_3(void){
	char *buf;
	int i;
	buf = (char *)fm_malloc(128);
	for(i = 0; i < 128; ++i){
		buf[i] = 0xef;
	}
	fm_memset(buf, 0, 128);
	for(i = 0; i < 128; ++i){
		if(buf[i] != 0){
			fm_free(buf);
			return 0;
		}
	}
	fm_free(buf);
	return 1;
}

static int memcmp_test(void){
	char m1[16] = "done done done1";
	char m2[16] = "done done done2";
	char m3[16] = "111111111111111";
	return fm_memcmp(m1, m1, 16) == 0
			&& fm_memcmp(m2, m1, 16) == 1
			&& fm_memcmp(m1, m2, 16) == -1
			&& fm_memcmp(m1, m3, 16) == 'd' - '1';
}

static int memcpy_test(void){
	char buf[16] = "123456789abcdef";
	char dest[16], *_dest;
	_dest = (char *)fm_malloc(16);
	fm_memcpy(dest, buf, 16);
	fm_memcpy(_dest, buf, 16);
	int r = fm_memcmp(_dest, buf, 16);
	fm_free(_dest);
	return fm_memcmp(dest, buf, 16) == 0 && r == 0;
}

static int memmove_test(void){
	char buf[16] = "123456789abcdef";
	char res[16] = "123456123456789";
	char buf2[16];
	fm_memmove(buf+6, buf, 9);
	fm_memmove(buf2, res, 16);
	return fm_memcmp(buf, res, 16) == 0 && fm_memcmp(buf2, res, 16) == 0;
}

static int strcmp_test(void){
	char s1[16] = "1234";
	char s2[16] = "aaaa1234\0abc";
	return fm_strcmp(s1, s2+4) == 0 && fm_strcmp(s1, s2) < 0 && fm_strcmp(s2+9, s1) > 0;
}

static int strncmp_test(void){
	char s1[16] = "1234";
	char s2[16] = "aaaa1234\0abc";
	return fm_strncmp(s1, s2 + 4, 3) == 0 && fm_strncmp(s1, s2, 4) < 0;
}

static int strcasecmp_test(void){
	char s1[16] = "1ABcd";
	char s2[16] = "1aBCd";
	return fm_strcasecmp(s1, s2) == 0;
}

static int strdup_test(void){
	char *c, str[12] = "1abc1";
	c = fm_strdup(str);
	int r = fm_strcmp(str, c);
	fm_free(c);
	return r == 0;
}

static int strlen_test(void){
	char str[32] = "0123456789abcdef\0aaa";
	return fm_strlen(str) == 16 && fm_strlen(str+17) == 3;
}

static int strncat_test(void){
	char s1[32] = "012345678";
	char s2[32] = "9abcdef";
	char s3[16] = "012345678";
	char ans[32] = "0123456789abcdef";
	char ans2[16] = "0123456789abcde";
	fm_strncat(s1, s2, 32);
	fm_strncat(s3, s2, 16);
	return fm_strcmp(s1, ans) == 0 && fm_strcmp(s3, ans2) == 0;
}

static int strncpy_test(void){
	char s1[16];
	char s2[32] = "0123456789abcdef";
	char ans[16] = "0123456789abcde";
	fm_strncpy(s1, s2, 16);
	return fm_strcmp(s1, ans) == 0;
}

static int strstr_test(void){
	char s1[16] = "11112222abcde";
	char s2[16] = "1122";
	char s3[16] = "1113";
	return fm_strstr(s1, s2) == s1 + 2 && fm_strstr(s1, s3) == fm_mem_nullptr;
}

static int kmp_test(void){
	char s1[16] = "11223344";
	char s3[16] = "11111111";
	char s2[16] = "23";
	int kmp[2];
	fm_kmp_init(kmp, 2, s2);
	return 	fm_strstr_kmp_struct(s1, 8, s2, 2, kmp) == s1 + 3
			&& fm_strstr_kmp_struct(s3, 8, s2, 2, kmp) == fm_mem_nullptr;

}

static int atoi_test(void){
	char test[8] = "12#145";
	return fm_atoi(test) == 12 && fm_atoi(test+3) == 145;
}

static int snprintf_test(void){
	char buf[32];
	char str[16] = "HELLO";
	char answer[32] = "#HELLO#12#12#A#f#F#0012#";
	fm_snprintf(buf, 32, "#%s#%d#%i#%c#%x#%X#%04d#", str, 12, 12, 'A', 15, 15, 12);
	return fm_strncmp(buf, answer, 32) == 0;
}

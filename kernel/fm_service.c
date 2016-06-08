#include "fm_config.h"
#include "fm_service.h"
#include "fm_mem.h"

#ifdef _USE_FM_MEM_SERVICE_

void *fm_memset(void *dest, char c, fm_mem_size_t size){
#define MIN_SIZE 		sizeof(unsigned long)
#define IS_ALIGNED(x) 	(!(((unsigned long)(x))&(MIN_SIZE-1)))
	char *s = (char*)dest;
	unsigned long d = c & 0xff;
	if(size >= MIN_SIZE && IS_ALIGNED(s)){
		unsigned long buff;
		unsigned long *addr = (unsigned long *)s;
		if(MIN_SIZE == 4){
			buff = (d << 8) | d;
			buff = (buff << 16) | buff;
		} else {
			unsigned int i;
			buff = 0;
			for(i = 0; i < MIN_SIZE; ++i){
				buff = (buff << 8) | d;
			}
		}
		while(size >= 4 * MIN_SIZE){
			*addr++ = buff;
			*addr++ = buff;
			*addr++ = buff;
			*addr++ = buff;
			size -= 4 * MIN_SIZE;
		}
		while(size >= MIN_SIZE){
			*addr++ = buff;
			size -= MIN_SIZE;
		}
		s = (char*)addr;
	}
	while(size--){
		*s++ = c;
	}
	return dest;
#undef MIN_SIZE
#undef IS_ALIGNED
}

void *fm_memcpy(void *dest, const void *src, fm_mem_size_t size){
#define MIN_SIZE sizeof(unsigned long)
#define IS_ALIGNED(x, y) (!((((unsigned long)(x))&(MIN_SIZE-1)) | (((unsigned long)(y))&(MIN_SIZE-1))))
	char *d = (char*)dest;
	char *s = (char*)src;
	if(size >= MIN_SIZE && IS_ALIGNED(d, s)){
		unsigned long *addr_dest 	= (unsigned long*)d;
		unsigned long *addr_src		= (unsigned long*)s;
		while(size >= 4 * MIN_SIZE){
			*addr_dest++ = *addr_src++;
			*addr_dest++ = *addr_src++;
			*addr_dest++ = *addr_src++;
			*addr_dest++ = *addr_src++;
			size -= 4 * MIN_SIZE;
		}
		while(size >= MIN_SIZE){
			*addr_dest++ = *addr_src++;
			size -= MIN_SIZE;
		}
		d = (char*)addr_dest;
		s = (char*)addr_src;
	}
	while(size--){
		*d++ = *s++;
	}
	return dest;
#undef MIN_SIZE
#undef IS_ALIGNED
}

void *fm_memmove(void *dest, const void *src, fm_mem_size_t size){
	char *d = (char *)dest;
	char *s = (char *)src;
	if(s < d && d < s + size){
		d += size;
		s += size;
		while(size--){
			*(--d) = *(--s);
		}
	} else {
		while(size--){
			*d++ = *s++;
		}
	}
	return dest;
}

int fm_memcmp(const void *dest, const void *src, fm_mem_size_t size){
	const unsigned char *d, *s;
	int ret = 0;
	for(d = (const unsigned char*)dest, s = (const unsigned char*)src; size > 0; ++d, ++s, --size){
		if((ret = *d - *s)){
			break;
		}
	}
	return ret;
}

#ifdef _USE_FM_MEM_SAFE_SERVICE_

void *fm_memset_s(void *dest, fm_mem_size_t dest_size, char c, fm_mem_size_t size){
	if(size > dest_size){
		size = dest_size;
	}
	return fm_memset(dest, c, size);
}

void *fm_memcpy_s(void *dest, fm_mem_size_t dest_size, void *src, fm_mem_size_t size){
	if(size > dest_size){
		size = dest_size;
	}
	return fm_memcpy(dest, src, size);
}

void *fm_memmove_s(void *dest, fm_mem_size_t dest_size, void *src, fm_mem_size_t size){
	if(size > dest_size){
		size = dest_size;
	}
	return fm_memmove(dest, src, size);
}

#endif
#endif

#ifdef _USE_FM_STRING_SERVICE_

int fm_strcmp(const char *s1, const char *s2){
	while(*s1 && *s1 == *s2){
		++s1;
		++s2;
	}
	return *s1 - *s2;
}

int fm_strncmp(const char *s1, const char *s2, fm_string_size n){
	while(--n && *s1 && *s1 == *s2){
		++s1;
		++s2;
	}
	return *s1 - *s2;
}
int fm_strcasecmp(const char *s1, const char *s2){
	int sa, sb;
	do {
		sa = *s1 & 0xff;
		sb = *s2 & 0xff;
		if(sa <= 'Z' && sa >= 'A')
			sa += 'a' - 'A';
		if(sb <= 'Z' && sb >= 'A')
			sb += 'a' - 'A';
		++s1;
		++s2;
	} while(sa && sa == sb);
	return sa - sb;
}

fm_string_size fm_strlen(const char *s){
	const char *p = s;
	while(*p){
		++p;
	}
	return (fm_string_size)(p - s);
}
char *fm_strdup(const char *s){
	fm_string_size len = fm_strlen(s) + 1;
	char *re = (char*)fm_malloc(len);
	if(!re){
		return fm_mem_nullptr;
	}
	fm_memcpy(re, s, len);
	return re;
}


char *fm_strncpy(char *dest, const char *src, fm_string_size n){
	if(!n){
		return dest;
	}
	while(--n && *src){
		*dest++ = *src++;
	}
	*dest = 0;
	return dest;
}

char *fm_strncat(char *dest, const char *src, fm_string_size n){
	while(n && *dest){
		++dest;
		--n;
	}
	if(!n){
		return dest;
	}
	while(--n && *src){
		*dest++ = *src++;
	}
	*dest = 0;
	return dest;
}

char *fm_strstr(const char *s1, const char *s2){
	fm_string_size l1, l2;
	l2 = fm_strlen(s2);
	if(!l2){
		return fm_mem_nullptr;
	}
	l1 = fm_strlen(s1);
	while(l1 >= l2){
		if(!fm_memcmp(s1, s2, l2)){
			return (char*)s1;
		}
		--l1;
		++s1;
	}
	return fm_mem_nullptr;
}

#ifdef _ENABLE_STRING_UNSAFE_
char *fm_strcpy(char *dest, const char *src){
	while(*src){
		*dest++ = *src++;
	}
	*dest = 0;
	return dest;
}

char *fm_strcat(char *dest, const char *src){
	dest += fm_strlen(dest);
	while(*src){
		*dest++ = *src++;
	}
	*dest = 0;
	return dest;
}
#endif

#ifdef _ENABLE_STRSTR_KMP_

void fm_kmp_init(int *kmp, int len, const char *s){
	int prev = -1, i;
	if(!len){
		return;
	}
	kmp[0] = -1;
	for(i = 1; i < len; ++i){
		while(prev != -1 && s[prev+1] != s[i])
			prev = kmp[prev];
		if(s[prev+1] == s[i])
			++prev;
		kmp[i] = prev;
	}
}

char *fm_strstr_kmp_struct(const char *s1, int len1, const char *s2, int len2, int *kmp){
	int prev = -1, i;
	for(i = 0; i < len1; ++i){
		while(prev != -1 && s2[prev+1] != s1[i])
			prev = kmp[prev];
		if(s2[prev+1] == s1[i])
			++prev;
		if(prev == len2 - 1){
			return (char*)&s1[i - len2 + 1];
		}
	}
	return fm_mem_nullptr;
}

char *fm_strstr_kmp(const char *s1, const char *s2){
	int l1, l2;
	char *_ret = fm_mem_nullptr;
	l2 = (int)fm_strlen(s2);
	if(!l2){
		return fm_mem_nullptr;
	}
	l1 = (int)fm_strlen(s1);
	if(l1 >= l2){
		int *kmp = (int*)fm_malloc((fm_mem_size_t)(l2*sizeof(int)));
		fm_kmp_init(kmp, l2, s2);
		_ret = fm_strstr_kmp_struct(s1, l1, s2, l2, kmp);
		fm_free(kmp);
	}
	return _ret;
}

#endif

#ifdef _USE_FM_STDIO_SERVICE_

static char *fm_print_num(char*, char*, long, int, int, int);

#define IS_DIGIT(x) (((unsigned)((x)-'0'))<10)

int fm_atoi(char *s){
	int _ret = 0;
	while(*s && IS_DIGIT(*s)){
		_ret = _ret * 10 + *s - '0';
		++s;
	}
	return _ret;
}

#define ZERO_PADDING	1
#define SIGNED			2
#define SPECIAL 		4			//0x
#define SHOW_PLUS		8			//show + in head
#define LEFT			16
#define LARGE			32			//use "ABCDEF" instead of 'abcdef'
#define SPACE			64

inline int divide_number(fm_number_t *n, int base){
	fm_number_t ret = 0;
	if(base == 10){
		ret = *n % 10u;
		*n /= 10u;
	} else {
		ret = *n % 16u;
		*n /= 16u;
	}
	return (int)ret;
}

static char *fm_print_num(char *buf, char *end, long num, int width, int base, int type){
	static const char small_digits[] = "0123456789abcdef";
	static const char large_digits[] = "0123456789ABCDEF";

	char *digits;
	char sign, fill;
	int size = width, i;
	char tmp[16];
	if(type & LEFT){
		type &= ~ZERO_PADDING;
	}
	fill = type & ZERO_PADDING? '0' : ' ';
	digits = (type & LARGE) ? large_digits : small_digits;
	sign = 0;
	if(type & SIGNED){
		if(num < 0){
			sign = '-';
			num = -num;
		} else if(type & SHOW_PLUS){
			sign = '+';
		} else if(type & SPACE){
			sign = ' ';
		}
	}
	i = 0;
	if(!num){
		tmp[i++] = '0';
	} else {
		while(num){
			tmp[i++] = digits[divide_number((fm_number_t*)&num, base)];
		}
	}
	size -= i;
	if(!(type & (ZERO_PADDING | LEFT))){
		if(sign && size > 0)
			--size;
		while(size-->0){
			if(buf < end)
				*buf = ' ';
			++buf;
		}
	}
	if(sign){
		if(buf < end){
			*buf = sign;
			--size;
		}
		++buf;
	}
	if(!(type & LEFT)){
		while(size-->0){
			if(buf < end)
				*buf = fill;
			++buf;
		}
	}
	//print number
	while(i-->0){
		if(buf < end)
			*buf = tmp[i];
		++buf;
	}
	while(size-->0){
		if(buf < end)
			*buf = fill;
		++buf;
	}
	return buf;
}

inline int skip_atoi(char **s){
	int ret = 0;
	if(IS_DIGIT(**s)){
		ret = ret * 10 + *((*s)++) - '0';
	}
	return ret;
}

int fm_vsnprintf(char *buf, fm_mem_size_t size, const char *fmt, va_list args){
	fm_number_t num;
	int i, flags, width, base, len;
	char *str, *end, *s, c, qualifier;

	str = buf;
	end = buf + size;
	if(end <= buf){
		end 	= (char*)-1;
		size 	= (fm_mem_size_t)(end - buf);
	}
	for(; *fmt; ++fmt){
		if(*fmt != '%'){
			if(str < end)
				*str = *fmt;
			++str;
			continue;
		}
		// meet %, skip first one
		flags = 0;
		while(1){
			++fmt;
			if(*fmt == '-'){
				flags |= LEFT;
			} else if(*fmt == '+'){
				flags |= SHOW_PLUS;
			} else if(*fmt == ' '){
				flags |= SPACE;
			} else if(*fmt == '0'){
				flags |= ZERO_PADDING;
			} else {
				break;
			}
		}
		width = -1;
		if(IS_DIGIT(*fmt)){
			width = skip_atoi(&fmt);
		} else if(*fmt == '*'){
			++fmt;
			width = va_arg(args, int);
			if(width < 0){
				width = -width;
				flags |= LEFT;
			}
		}
		qualifier = 0;
		if(*fmt == 'h' || *fmt == 'l'){
			qualifier = *fmt;
			++fmt;
		}
		base = 10;
		switch(*fmt){
		case 'c':
			if(!(flags & LEFT)){
				while(--width > 0){
					if(str < end)
						*str = ' ';
					++str;
				}
			}
			c = (char)va_arg(args, int);
			if(str < end)
				*str = c;
			++str;
			while(--width > 0){
				if(str < end)
					*str = ' ';
				++str;
			}
			continue;
		case 's':
			s = va_arg(args, char*);
			len = (int)fm_strlen(s);
			if(!(flags & LEFT)){
				while(len < width--){
					if(str < end)
						*str = ' ';
					++str;
				}
			}
			for(i = 0; i < len; ++i){
				if(str < end)
					*str = *s;
				++str;
				++s;
			}
			while(len < width--){
				if(str < end)
					*str = ' ';
				++str;
			}
			continue;
		case 'p':
			if(width == -1){
				width = sizeof(void*) << 1;
				flags |= ZERO_PADDING;
			}
			fm_print_num(str, end, (long)va_arg(args, void*), width, 16, flags);
			continue;
		case '%':
			if(str < end)
				*str = '%';
			++str;
			continue;
		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;
		case 'd':
		case 'i':
			flags |= SIGNED;
		case 'u':
			break;
		default:
			if(str < end)
				*str = '%';
			++str;
			if(*fmt){
				if(str < end)
					*str = *fmt;
				++str;
			} else {
				--fmt;
			}
			continue;
		}
		if(qualifier == 'l'){
			num = va_arg(args, fm_number_t);
		} else if(qualifier == 'h'){
			num = (unsigned short)va_arg(args, fm_number_t);
		} else {
			num = va_arg(args, fm_number_t);
		}
		str = fm_print_num(str, end, (long)num, width, base, flags);
	}
	if(str < end)
		*str = 0;
	else
		*(end - 1) = 0;
	return str - buf;
}

int fm_snprintf(char *buf, fm_mem_size_t size, const char *fmt, ...){
	int n;
	va_list args;

	va_start(args, fmt);
	n = fm_vsnprintf(buf, size, fmt, args);
	va_end(args);

	return n;
}


#undef IS_DIGIT

#endif

#endif

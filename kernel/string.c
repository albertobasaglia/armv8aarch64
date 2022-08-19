#include <stdint.h>
#include <string.h>

char* strcpy(char* dest, const char* src)
{
	while (*src != 0) {
		*dest = *src;
		src++;
		dest++;
	}
	*dest = 0;
	return dest;
}

char* strncpy(char* dest, const char* src, size_t n)
{
	while (*src != 0 && n > 0) {
		*dest = *src;
		src++;
		dest++;
		n--;
	}
	*dest = 0;
	return dest;
}

void strset(char* dest, const char c)
{
	while (*dest != 0) {
		*dest = c;
		dest++;
	}
}

char* strcat(char* dest, const char* src)
{
	while (*dest != 0) {
		dest++;
	}
	while (*src != 0) {
		*dest = *src;
		dest++;
		src++;
	}
	*dest = 0;
	return dest;
}

char* strncat(char* dest, const char* src, size_t n)
{
	while (*dest != 0) {
		dest++;
	}
	while (*src != 0 && n > 0) {
		*dest = *src;
		dest++;
		src++;
		n--;
	}
	*dest = 0;
	return dest;
}

int strcmp(const char* s1, const char* s2)
{
	while (*s1 != 0 && (*s1 == *s2)) {
		s1++;
		s2++;
	}

	return *s1 - *s2;
}

int strlen(const char* str)
{
	int len = 0;
	while (*str != 0) {
		len++;
		str++;
	}
	return len;
}

/* void* memset(void* str, int c, size_t n) */
/* { */
/* 	char fill_value = c; */
/* 	char* str_char = (char*)str; */
/* 	for (size_t i = 0; i < n; i++) { */
/* 		str_char[i] = fill_value; */
/* 	} */
/* 	return str; */
/* } */

void* memset(void* str, int c, size_t n)
{
	uint64_t big_c = 0;
	for (int i = 0; i < 8; i++) { // 8 bytes in a uint64_t
		big_c <<= 8;
		big_c |= c;
	}
	int big_writes = n / 8;
	int small_writes = n - big_writes * 8;

	uint64_t* big_pointer = (uint64_t*)str;
	for (int i = 0; i < big_writes; i++) {
		big_pointer[i] = big_c;
	}

	char* small_pointer = (char*)str;
	small_pointer += big_writes * 8;
	for (int i = 0; i < small_writes; i++) {
		small_pointer[i] = c;
	}

	return str;
}

void strrev(char* str)
{
	int len = strlen(str);
	int left = 0;
	int right = len - 1;
	while (left < right) {
		char swap = str[left];
		str[left] = str[right];
		str[right] = swap;
		left++;
		right--;
	}
}

void* memcpy(void* restrict dest, const void* restrict src, size_t n)
{
	char* dest_char = (char*)dest;
	char* src_char = (char*)src;
	// TODO optimize (same strategy as memset)
	for (int i = 0; i < n; i++) {
		dest_char[i] = src_char[i];
	}
	return dest;
}

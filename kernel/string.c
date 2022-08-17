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

void* memset(void* str, int c, size_t n)
{
	char fill_value = c;
	char* str_char = (char*)str;
	for (size_t i = 0; i < n; i++) {
		str_char[i] = fill_value;
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

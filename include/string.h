#ifndef STRING_H
#define STRING_H

#include <stddef.h>

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
void strset(char* dest, const char c);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strlen(const char* str);
void *memset(void *str, int c, size_t n);

#endif

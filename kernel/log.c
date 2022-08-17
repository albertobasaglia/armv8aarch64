#include "timer.h"
#include "uart.h"
#include <log.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void klog(const char* str)
{
	char buffer[50];
	char prefix[15];
	memset(prefix, 0, 15);
	sprintf(prefix, "[%q] ", timer_read_systemcounter_usec());
	strcpy(buffer, prefix);
	strcat(buffer, str);
	strcat(buffer, "\n");
	put_string(buffer);
}

void klogf(const char* format, ...)
{
	char buffer[40];
	memset(buffer, 0, 40);
	va_list vars;
	va_start(vars, format);
	sprintf_internal(buffer, format, vars);
	klog(buffer);
	va_end(vars);
}

int sprintf_internal(char* str, const char* format, va_list vars)
{
	char buffer[50];
	memset(buffer, 0, 50);
	size_t inserted = 0;
	while (*format != 0) {
		if (*format == '%') {
			char type = format[1];
			if (!type) {
				// no next element to format
				return -1;
			}
			switch (type) {
			case 'q': {
				uint64_t num = va_arg(vars, uint64_t);
				uint64_to_string(buffer, num, 10);
				int inserted = strlen(buffer);
				strcat(str, buffer);
				format += 2;
				str += inserted;
				break;
			}
			default:
				*str = '?';
				str++;
				format++;
				inserted++;
				break;
			}
		} else {
			*str = *format;
			str++;
			format++;
			inserted++;
		}
	}
	*str = 0;
	return inserted;
}

int sprintf(char* str, const char* format, ...)
{
	va_list vars;
	va_start(vars, format);
	int res = sprintf_internal(str, format, vars);
	va_end(vars);
	return res;
}

void uint64_to_string(char* str, uint64_t num, int base)
{
	char buffer[50];
	int i = 0;
	while (num > 0) {
		int mod = num % base;
		buffer[i++] = mod < 10 ? mod + '0' : mod - 10 + 'a';
		num /= base;
	}
	buffer[i] = 0;
	strrev(buffer);
	strcpy(str, buffer);
}

#include <uart.h>

#define UART_BASEADDRESS 0x09000000

void put_char(char c)
{

	static char* dest = (char*)UART_BASEADDRESS;
	*dest = c;
}

void put_string(const char* str)
{
	while (*str != 0) {
		put_char(*str);
		str++;
	}
}

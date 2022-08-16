#include "uart.h"
#include <log.h>
#include <string.h>

void klog(const char* str)
{
	char buffer[50];
	char prefix[] = "[KERNEL] ";
	strcpy(buffer, prefix);
	strcat(buffer, str);
	strcat(buffer, "\n");
	put_string(buffer);
}

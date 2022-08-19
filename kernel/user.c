#include "log.h"
#include <user.h>
void init()
{
	asm volatile("svc 0");
	asm volatile("svc 1337");
	while (1)
		;
}

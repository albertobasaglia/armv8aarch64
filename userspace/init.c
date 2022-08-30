#include "sys.h"

int main()
{
	sys_log("spawning new process!");

	sys_spawn("hello.elf");

	while (1)
		;
	return 0;
}

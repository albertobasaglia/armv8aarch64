#include "sys.h"

int main()
{
	sys_log("spawning new process!");

	sys_spawn("hello.elf");

	while (1) {
		for (int i = 0; i < 10e6; i++) {
			i++;
			i--;
		}
		sys_log("hey");
	}
	return 0;
}

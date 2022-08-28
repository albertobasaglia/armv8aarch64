int sys_open(char* filename)
{
	int fd;
	asm volatile("mov x0, %1\n"
		     "svc 20\n"
		     "mov %w0, w0"
		     : "=r"(fd)
		     : "r"(filename)
		     : "w0");
	return fd;
}

void sys_ping()
{
	asm volatile("svc 17");
}

void sys_log(char* message)
{
	asm volatile("mov x0, %0\n"
		     "svc 11" ::"r"(message)
		     : "x0");
}

int main()
{
	char* test = "hello.elf";
	int fd = sys_open(test);
	sys_log("Ciuao!");

	while (1)
		;
	return 0;
}

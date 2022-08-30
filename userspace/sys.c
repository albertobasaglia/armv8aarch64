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

char sys_read(int fd)
{
	char c;
	asm volatile("mov w0, %w1\n"
		     "svc 21\n"
		     "mov %w0, w0"
		     : "=r"(c)
		     : "r"(fd)
		     : "w0");

	return c;
}

void sys_log(char* message)
{
	asm volatile("mov x0, %0\n"
		     "svc 11" ::"r"(message)
		     : "x0");
}

void sys_spawn(char* filename)
{
	asm volatile("mov x0, %0\n"
		     "svc 22" ::"r"(filename)
		     : "x0");
}

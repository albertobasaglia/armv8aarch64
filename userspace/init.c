void _start()
{
	int a = 5;
	int b = -5;
	int c = a + b;
	asm volatile("svc 0");
}

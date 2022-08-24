int main()
{
	asm volatile("svc 17");
	int a = 5;
	int b = -5;
	int c = a + b;
	asm volatile("svc 17");
	while (1)
		;
	return 0;
}

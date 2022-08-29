int main()
{
	asm volatile("svc 10");
	while (1)
		;
	return 0;
}

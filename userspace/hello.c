int main()
{
	asm volatile("svc 17");
	while (1)
		;
	return 0;
}

extern int(main)(void);

void _start()
{
	int exit_code = main();
	asm volatile("svc 10");
	while (1)
		; // Wait for termination

	/*
	 * This is still very WIP:
	 * - Decide strategy to pass back error code (maybe in x0?)
	 * - Decide syscall for process termination
	 * */
}

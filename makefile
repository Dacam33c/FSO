COMPILER = gcc
SHELL_PROG = shell_sched
SCHED_PROG = user_scheduler
CPU_BOUND_PROG = cpu_bound

all:
	$(COMPILER) $(SHELL_PROG).c -o $(SHELL_PROG)
	$(COMPILER) $(SCHED_PROG).c -o $(SCHED_PROG)
	$(COMPILER) $(CPU_BOUND_PROG).c -o $(CPU_BOUND_PROG) -lm

clean:
	rm -f $(SHELL_PROG) $(SCHED_PROG) $(CPU_BOUND_PROG) *.o *.exe

run:
	./$(SHELL_PROG)
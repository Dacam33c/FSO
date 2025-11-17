COMPILER = gcc
SHELL_PROG = shell_sched
SCHED_PROG = user_scheduler
CPU_BOUND_PROG = cpu_bound

all:
	$(COMPILER) -o $(SHELL_PROG) $(SHELL_PROG).c 
	$(COMPILER) -o $(SCHED_PROG) $(SCHED_PROG).c
	$(COMPILER) -o $(CPU_BOUND_PROG) $(CPU_BOUND_PROG).c 

clean:
	rm -f $(SHELL_PROG) $(SCHED_PROG)

run:
	./$(SHELL_PROG)
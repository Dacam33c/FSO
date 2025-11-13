COMPILER = gcc
SHELL_PROG = shell_sched
SCHED_PROG = user_scheduler

all:
	$(COMPILER) -o $(SHELL_PROG) shell_sched.c 
	$(COMPILER) -o $(SCHED_PROG) user_scheduler.c
clean:
	rm -f $(SHELL_PROG) $(SCHED_PROG)

run:
	./$(SHELL_PROG)
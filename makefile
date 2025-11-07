COMPILER = gcc
SHELL_PROG = shell_sched
SCHED_PROG = user_scheduler

all:
	$(COMPILER) -o $(SHELL_PROG) shell_sched.c 
clean:
	rm -f $(SHELL_PROG) $(SCHED_PROG)
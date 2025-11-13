#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

#define MAX_QUEUES 3
#define MAX_PROCESSES 256

typedef struct {
    pid_t pid;
    char command[128];
    int priority;
    int remaining_quantum_ms; // quanto falta do quantum atual
} pcb_t;

typedef struct {
    pcb_t items[MAX_PROCESSES];
    int size;
} rr_queue_t;

static rr_queue_t queues[MAX_QUEUES + 1];
static int numFilas;
static pid_t current_pid = -1;
static int current_priority = -1;
static int current_remaining_quantum = 0;
static int quantum_ms[MAX_QUEUES + 1];
static int fd_read;

/* Inicialização */
void scheduler_init(int n) {
    numFilas = n;
    for (int i = 1; i <= n; i++) {
        queues[i].size = 0;
        quantum_ms[i] = 10000; // cada quantum = 10s 
        printf("Fila %d criada (quantum %d ms)\n", i, quantum_ms[i]);
    }
}

/* Timer */
void start_timer(int ms) {
    struct itimerval t;
    t.it_value.tv_sec = ms / 1000;
    t.it_value.tv_usec = (ms % 1000) * 1000;
    t.it_interval.tv_sec = 0;
    t.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &t, NULL);
}

/* Escolher próximo processo */
void schedule_next() {
    for (int pr = 1; pr <= numFilas; pr++) {
        if (queues[pr].size > 0) {
            pcb_t p = queues[pr].items[0];
            // shift fila
            for (int i = 1; i < queues[pr].size; i++) {
                queues[pr].items[i - 1] = queues[pr].items[i];
            }
            queues[pr].size--;

            current_pid = p.pid;
            current_priority = pr;
            current_remaining_quantum = p.remaining_quantum_ms;

            if (current_remaining_quantum <= 0)
                current_remaining_quantum = quantum_ms[pr];

            kill(current_pid, SIGCONT);
            start_timer(current_remaining_quantum);
            printf("[scheduler] processo %d (prio %d) em execução com quantum=%dms\n",
                current_pid, pr, current_remaining_quantum);
            return;
        }
    }
    current_pid = -1;
    current_priority = -1;
    printf("[scheduler] nenhuma tarefa disponível\n");
}

/* Quantum expirado */
void handler_timer(int sig) {
    if (current_pid > 0) {
        kill(current_pid, SIGSTOP);

        pcb_t p;
        p.pid = current_pid;
        p.priority = current_priority;
        strcpy(p.command, "cpu_bound");
        p.remaining_quantum_ms = 0; // consumiu todo quantum

        queues[current_priority].items[queues[current_priority].size++] = p;
        printf("[scheduler] quantum expirado, processo %d parado\n", current_pid);
    }
    schedule_next();
}

/* Processo terminou */
void handler_child(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == current_pid) {
            printf("[scheduler] processo %d terminou\n", pid);
            current_pid = -1;
            current_priority = -1;
            schedule_next();
        }
    }
}

/* Main */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <numFilas> <fd_leitura>\n", argv[0]);
        exit(1);
    }

    numFilas = atoi(argv[1]);
    fd_read = atoi(argv[2]);

    scheduler_init(numFilas);

    signal(SIGALRM, handler_timer);
    signal(SIGCHLD, handler_child);

    char buffer[256];
    while (1) {
        int n = read(fd_read, buffer, sizeof(buffer) - 1);
        if (n <= 0) continue;
        buffer[n] = '\0';

        if (strncmp(buffer, "EXECUTE", 7) == 0) {
            char cmd[100];
            int prio;
            sscanf(buffer, "EXECUTE %s %d", cmd, &prio);

            pid_t pid = fork();
            if (pid == 0) {
                execlp(cmd, cmd, NULL);
                perror("exec");
                exit(1);
            }
            else {
                kill(pid, SIGSTOP); // começa parado
                pcb_t p;
                p.pid = pid;
                p.priority = prio;
                strcpy(p.command, cmd);
                p.remaining_quantum_ms = quantum_ms[prio];
                queues[prio].items[queues[prio].size++] = p;
                printf("[scheduler] processo '%s' criado PID=%d prio=%d\n", cmd, pid, prio);

                // Preempção por chegada de maior prioridade
                if (current_pid == -1 || prio < current_priority) {
                    if (current_pid > 0) {
                        // calcula quantum restante do atual
                        struct itimerval t;
                        getitimer(ITIMER_REAL, &t);
                        int remaining = t.it_value.tv_sec * 1000 + t.it_value.tv_usec / 1000;
                        if (remaining <= 0) remaining = 1;

                        pcb_t old;
                        old.pid = current_pid;
                        old.priority = current_priority;
                        strcpy(old.command, "cpu_bound");
                        old.remaining_quantum_ms = remaining;

                        // coloca na frente da fila
                        for (int i = queues[current_priority].size; i > 0; i--) {
                            queues[current_priority].items[i] = queues[current_priority].items[i - 1];
                        }
                        queues[current_priority].items[0] = old;
                        queues[current_priority].size++;

                        kill(current_pid, SIGSTOP);
                    }
                    schedule_next();
                }
            }
        }
        else if (strncmp(buffer, "LIST", 4) == 0) {
        }
        else if (strncmp(buffer, "EXIT", 4) == 0) {
            printf("[scheduler] encerrando...\n");
            break;
        }
    }

    close(fd_read);
    return 0;
}

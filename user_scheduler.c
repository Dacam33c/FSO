#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_QUEUES 3
#define MAX_PROCESSES 256

typedef struct {
    pid_t pid;
    char command[128];
    int priority;
    int remaining_quantum_ms;
    double start_time;
    double end_time;
    int finished; // 0 = não terminou, 1 = terminou
} pcb_t;

typedef struct {
    pid_t pid;
    double turnaround_ms;
} finished_t;

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
static double current_start_time = 0.0;
static finished_t finished[MAX_PROCESSES];
static int finished_count = 0;
static char current_command[128] = "";

/* tempo atual em ms */
double now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

/* Inicialização */
void scheduler_init(int n) {
    numFilas = n;
    for (int i = 1; i <= n; i++) {
        queues[i].size = 0;
        quantum_ms[i] = 4000; // cada quantum = 4s
        printf(">shell_sched: Fila %d criada (quantum %d ms)\n", i, quantum_ms[i]);
    }
    printf(">shell_sched: ");
    fflush(stdout);
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
            strcpy(current_command, p.command);
            for (int i = 1; i < queues[pr].size; i++) {
                queues[pr].items[i - 1] = queues[pr].items[i];
            }
            queues[pr].size--;

            current_pid = p.pid;
            current_priority = pr;
            current_remaining_quantum = p.remaining_quantum_ms;
            if (current_remaining_quantum <= 0)
                current_remaining_quantum = quantum_ms[pr];

            current_start_time = p.start_time;

            kill(current_pid, SIGCONT);
            start_timer(current_remaining_quantum);
            printf(">shell_sched: [scheduler] processo %d (prio %d) em execucao com quantum=%dms\n",
                current_pid, pr, current_remaining_quantum);
            return;
        }
    }
    current_pid = -1;
    current_priority = -1;
    printf(">shell_sched: [scheduler] nenhuma tarefa disponivel\n");
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
        p.start_time = current_start_time;
        p.end_time = 0.0;
        p.finished = 0;

        queues[current_priority].items[queues[current_priority].size++] = p;
        printf(">shell_sched: [scheduler] quantum expirado, processo %d parado\n", current_pid);
    }
    schedule_next();
}

/* Processo terminou */
void handler_child(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == current_pid) {
            double end = now_ms();
            double turnaround = end - current_start_time;
            printf(">shell_sched: [scheduler] processo %d terminou (turnaround=%.0f ms)\n",
                pid, turnaround);

            // salvar no finished
            finished[finished_count].pid = pid;
            finished[finished_count].turnaround_ms = turnaround;
            finished_count++;

            current_pid = -1;
            current_priority = -1;
            schedule_next();
        }
        else {
            // procurar nas filas
            for (int pr = 1; pr <= numFilas; pr++) {
                for (int i = 0; i < queues[pr].size; i++) {
                    if (queues[pr].items[i].pid == pid) {
                        double end = now_ms();
                        double turnaround = end - queues[pr].items[i].start_time;
                        printf(">shell_sched: [scheduler] processo %d terminou (turnaround=%.0f ms)\n",
                            pid, turnaround);

                        finished[finished_count].pid = pid;
                        finished[finished_count].turnaround_ms = turnaround;
                        finished_count++;

                        // remover da fila
                        for (int j = i + 1; j < queues[pr].size; j++)
                            queues[pr].items[j - 1] = queues[pr].items[j];
                        queues[pr].size--;
                        break;
                    }
                }
            }
        }
    }
}


/* Listagem */
void scheduler_list() {
    printf(">shell_sched: \n===== LISTAGEM DO ESCALONADOR =====\n");

    if (current_pid > 0) {
        printf("Processo atual:\n");
        printf("  PID=%d | CMD=%s | PRIO=%d | Quantum restante=%dms\n",
            current_pid, current_command, current_priority, current_remaining_quantum);
    }
    else {
        printf("Nenhum processo em execucao no momento.\n");
    }

    for (int pr = 1; pr <= numFilas; pr++) {
        printf("Fila %d:\n", pr);
        if (queues[pr].size == 0) {
            printf("  [vazia]\n");
        }
        else {
            for (int j = 0; j < queues[pr].size; j++) {
                pcb_t* p = &queues[pr].items[j];
                printf("  PID=%d | CMD=%s | Quantum restante=%dms\n",
                    p->pid, p->command, p->remaining_quantum_ms);
            }
        }
    }

    printf("===================================\n\n");
}

/* Exit Scheduler */
void scheduler_exit() {
    printf(">shell_sched: \n===== RELATORIO FINAL =====\n");

    // parar timer
    struct itimerval t = { 0 };
    setitimer(ITIMER_REAL, &t, NULL);

    // imprimir processos finalizados (PID + turnaround)
    printf("\nProcessos finalizados:\n");
    if (finished_count == 0) {
        printf("  Nenhum processo finalizado.\n");
    }
    else {
        for (int i = 0; i < finished_count; i++) {
            printf("  PID=%d | Turnaround=%.0f ms\n",
                finished[i].pid, finished[i].turnaround_ms);
        }
    }

    // processos que não terminaram (CPU e filas)
    printf("\nProcessos nao finalizados:\n");

    if (current_pid > 0) {
        printf("  Em execucao: PID=%d | PRIO=%d\n",
            current_pid, current_priority);
        kill(current_pid, SIGTERM);
    }

    for (int pr = 1; pr <= numFilas; pr++) {
        for (int j = 0; j < queues[pr].size; j++) {
            pcb_t* p = &queues[pr].items[j];
            printf("  Na fila %d: PID=%d\n", pr, p->pid);
            kill(p->pid, SIGTERM);
        }
        queues[pr].size = 0;
    }

    printf("\nEncerrando escalonador...\n");
    printf("=================================\n\n");
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
            if (prio < 1 || prio > numFilas) {
                printf(">shell_sched: [scheduler] prioridade %d invalida, nao existe fila correspondente\n", prio);
                printf(">shell_sched: ");
                fflush(stdout);
                continue;
            }

            if (access(cmd, X_OK) != 0) {
                printf(">shell_sched: [scheduler] comando '%s' nao encontrado ou nao executavel\n", cmd);
                printf(">shell_sched: ");
                fflush(stdout);
                continue; // não cria processo
            }

            pid_t pid = fork();
            if (pid == 0) {
                execlp(cmd, cmd, NULL);
                perror("exec");
                exit(1);
            }
            else {
                kill(pid, SIGSTOP);
                pcb_t p;
                p.pid = pid;
                p.priority = prio;
                strcpy(p.command, cmd);
                p.remaining_quantum_ms = quantum_ms[prio];
                p.start_time = now_ms();
                p.end_time = 0.0;
                p.finished = 0;
                queues[prio].items[queues[prio].size++] = p;
                printf(">shell_sched: [scheduler] processo '%s' criado PID=%d prio=%d\n", cmd, pid, prio);

                if (current_pid == -1 || prio < current_priority) {
                    if (current_pid > 0) {
                        double elapsed = now_ms() - current_start_time;
                        int remaining = quantum_ms[current_priority] - (int)elapsed;
                        if (remaining <= 0) remaining = 1;

                        pcb_t old;
                        old.pid = current_pid;
                        strcpy(old.command, current_command);
                        old.remaining_quantum_ms = remaining;
                        old.start_time = current_start_time;
                        old.pid = current_pid;
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
            scheduler_list();
        }
        else if (strncmp(buffer, "EXIT", 4) == 0) {
            scheduler_exit();
            break;
        }
    }

    close(fd_read);
    return 0;
}
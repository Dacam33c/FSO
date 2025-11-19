/*
Alunos:
Daniel da Cunha Pereira Luz - 211055540
Gabriel Mauricio Chagas Silva - 221017097
Joao Pedro Carvalho de Oliveira Rodrigues - 221017032
Joao Vitor Dickmann - 211042757

versao do gcc:
gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0

versão do Linux:
Linux 6.6.87.2-microsoft-standard-WSL2
Ubuntu 24.04.3 LTS"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char input[100];
    char comando[100];
    int quitRequested = 0;
    pid_t scheduler = -1;
    int fd[2]; // pipe: fd[0] leitura, fd[1] escrita

    if (pipe(fd) < 0) {
        perror("pipe");
        exit(1);
    }

    printf(">shell_sched: ");
    fflush(stdout);

    while (!quitRequested && fgets(input, sizeof(input), stdin)) {
        if (strlen(input) == 1) { // só \n
            printf(">shell_sched: ");
            fflush(stdout);
            continue;
        }

        sscanf(input, "%s", comando);

        if (strcmp(comando, "create_user_scheduler") == 0) {
            int qtdFilas;
            sscanf(input, "%*s %d", &qtdFilas);

            if (qtdFilas > 3 || qtdFilas <= 1) {
                printf(">shell_sched: numero de filas nao suportado\n");
                printf(">shell_sched: ");
                continue;
            }
            else {
                if (scheduler == -1) {
                    pid_t pid = fork();
                    if (pid == 0) { // filho
                        close(fd[1]); // fecha escrita
                        char filasStr[10];
                        sprintf(filasStr, "%d", qtdFilas);
                        // passa descritor de leitura como argumento
                        char fdStr[10];
                        sprintf(fdStr, "%d", fd[0]);
                        execl("./user_scheduler", "./user_scheduler", filasStr, fdStr, NULL);
                        perror("execl");
                        exit(1);
                    }
                    else { // pai
                        scheduler = pid;
                        close(fd[0]); // fecha leitura
                        printf(">shell_sched: scheduler criado com PID %d\n", scheduler);
                    }
                }
                else {
                    printf("scheduler ja criado PID %d\n", scheduler);
                    printf(">shell_sched: ");
                }
            }
        }

        else if (strcmp(comando, "execute_process") == 0) {
            int prioridade = -1;
            char cmd[50];
            sscanf(input, "%*s %s %d", cmd, &prioridade);

            if (scheduler != -1) {
                char msg[200];
                sprintf(msg, "EXECUTE %s %d\n", cmd, prioridade);
                write(fd[1], msg, strlen(msg));
            }

            else {
                printf(">shell_sched: scheduler ainda nao criado\n");
                printf(">shell_sched: ");
            }
        }

        else if (strcmp(comando, "list_scheduler") == 0) {
            if (scheduler != -1) {
                char msg[50] = "LIST\n";
                write(fd[1], msg, strlen(msg));
            }

            else {
                printf(">shell_sched: scheduler ainda nao criado\n");
                printf(">shell_sched: ");
            }
        }

        else if (strcmp(comando, "exit_scheduler") == 0) {
            if (scheduler != -1) {
                char msg[50] = "EXIT\n";
                write(fd[1], msg, strlen(msg));
                // espera o scheduler terminar e reaproveita
                waitpid(scheduler, NULL, 0);
                scheduler = -1;
            }
            printf(">shell_sched: Encerrando Shell...\n");
            quitRequested = 1;
        }

        else {
            printf(">shell_sched: comando nao existe\n");
            printf(">shell_sched: ");
        }

        fflush(stdout);
    }

    return 0;
}
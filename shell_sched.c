/*
Alunos:
Daniel da Cunha Pereira Luz - 211055540
Gabriel Mauricio Chagas Silva - 221017097
Joao Pedro Carvalho de Oliveira Rodrigues - 221017032
Joao Vitor Dickmann - 211042757

gcc.exe (MinGW-W64 i686-ucrt-posix-dwarf, built by Brecht Sanders, r2) 15.1.0

Distributor ID: Ubuntu
Description:    Ubuntu 24.04.2 LTS
Release:        24.04
Codename:       noble
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(){
    char input[100];
    char comando[100];
    int quitRequested = 0;
    pid_t scheduler = -1;

    printf(">shell_sched: ");
    fflush(stdout);

    while(!quitRequested && fgets(input, sizeof(input), stdin)){
        if (strlen(input) == 1) { //input vazia(contem \n)
            printf(">shell_sched: ");
            fflush(stdout);
            continue;
        }

        sscanf(input, "%s", comando);

        if(strcmp(comando, "create_user_scheduler") == 0){
            int qtdFilas;
            sscanf(input, "%*s %d", &qtdFilas);
            

            if(qtdFilas < 1){
                printf("bota o numero de filas\n");
                printf(">shell_sched: ");
                continue;
            }
            else{
                printf("serão criadas %i filas\n",qtdFilas);
                if(scheduler == -1){
                    pid_t pid = fork();
                    if(pid == 0){ //fork retorna 0 para o processo filho
                        execl("./user_scheduler", "./user_scheduler", NULL);
                        exit(0);
                    }
                    else{ //caso seja o pai:
                        scheduler = pid;
                        printf("scheduler criado com PID %d\n", scheduler);
                    }

                }
                else{
                    printf("scheduler ja criado PID %d\n", scheduler);
                }
        }

            
        }

        else if(strcmp(comando, "execute_process") == 0){
            // execute process
            int prioridade;
            sscanf(input, "%*s %d", &prioridade);
            printf("processo executado com prioridade %i\n",prioridade);

        }

        else if(strcmp(comando,"list_scheduler") == 0){
            //list scheduler
        }

        else if(strcmp(comando,"exit_scheduler") == 0){
            //quit
            quitRequested = 1;
        }else{
            printf("comando não existe :(\n");
            printf(">shell_sched: ");
            continue;
        }
    }

}
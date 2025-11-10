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
            //user scheduler
            int qtdFilas;
            sscanf(input, "%*s %d", &qtdFilas);
            printf("serão criadas %i filas\n",qtdFilas);
            
        }

        if(strcmp(comando, "execute_process") == 0){
            // execute process
            int prioridade;
            sscanf(input, "%*s %d", &prioridade);
            printf("processo executado com prioridade %i\n",prioridade);

        }

        if(strcmp(comando,"list_scheduler") == 0){
            //list scheduler
        }

        if(strcmp(comando,"exit_scheduler") == 0){
            //quit
            quitRequested = 1;
        }
    }

}
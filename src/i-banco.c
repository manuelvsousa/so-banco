/******************************************************************************************
* File Name:    i-banco.c
* Author:       Beatriz Correia (84696) / Manuel Sousa (84740)
* Revision:
* NAME:         Banco - IST/SO - 2016/2017 1º Semestre
* SYNOPSIS:     #include <stdio.h> - I/O regular
                #include <string.h>  - char strings
                #include <stdlib.h>  - exit(), atoi()
                #include <unistd.h> - fork()
                #include <signal.h> - signal(), kill()
                #include <sys/wait.h> - waitpid()
                #include "commandlinereader.h" - Prototipos das funcoes de leitura dos comandos
                #include "contas.h" - Prototipos de todas as operações relacionadas com contas
* DESCRIPTION:  funcao main (i-banco)
* DIAGNOSTICS:  tested
* USAGE:        make clean
                make all
                make run
*****************************************************************************************/

/*Bibliotecas*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include "commandlinereader.h"
#include "contas.h"
#include "pool.h"
#include "buffer.h"

/* Macros - Comandos */
#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_AGORA "agora"

/* Operações - Comandos */
#define OP_LERSALDO 0
#define OP_CREDITAR 1
#define OP_DEBITAR 2

/*Constantes*/
#define MAXARGS 3
#define MAXCHILDS 20

/*Estruturas*/

typedef struct PID{
    pid_t pid;
    int estado;
}pids;

int topo = BUFFER_SIZE-1;

/******************************************************************************************
* main()
*
* Arguments:    Nenhum
* Returns: int  0
* Description:  Leitura dos comandos do banco e criação de processos nas simulações
*****************************************************************************************/
int main (int argc, char** argv) {
    char *args[MAXARGS + 1];
    char buffer[BUFFER_SIZE];
    comando_t cmd_buffer[CMD_BUFFER_DIM];


    pthread_mutex_init(&semExtMut,NULL);

    sem_init(&semBuffer,0,0);


    int numPids = 0;
    inicializarContas();
    inicializarThreads();

    printf("Bem-vinda/o ao i-banco\n\n");
      
    while (1) {
        int numargs;
        pids pids[MAXCHILDS];
        numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);
        int estado, sairAgora = 0;
        /* EOF (end of file) do stdin ou comando "sair" , "sair agora"*/
        if (numargs < 0 || (numargs > 0  && (strcmp(args[0], COMANDO_SAIR) == 0))) {
            if (numargs < 2) {

            /* Sair Agora */    
            } else if (numargs == 2 && strcmp(args[1], COMANDO_AGORA) == 0) {
                sairAgora = 1;
                //Ciclo que vai enviar um sinal individualmente para cada Processo Filho
                for(int i = 0; i < numPids; i++){
                    if (kill(pids[i].pid, SIGUSR1) != 0) //Verifica se ocorreu um erro ao enviar um Sinal
                        printf("%s: Erro ao enviar sinal para o Processo.\n", strcat(COMANDO_SAIR , COMANDO_AGORA));
                }    
            } else {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
                continue;
            }

            /* Ciclo que vai terminar todos os Processos Filho */    
            for(int i=0;i<numPids;i++){
                
                if(waitpid(pids[i].pid,&estado,0) == -1) //Terminar Processo filho. Se ocorrer um erro vai cair no if statment
                    printf("%s: Erro ao terminar Processo.\n", (sairAgora == 1) ? strcat(COMANDO_SAIR , COMANDO_AGORA) : COMANDO_SAIR);
                if(WIFEXITED(estado) != 0){ //Se o processo sair com um exit corretamente (de que maneira for)
                    if(WEXITSTATUS(estado) == 2) //Vamos verificar se o exit retornou o termino por signal
                        printf("Simulacao terminada por signal\n");
                } 
                pids[i].estado = WIFEXITED(estado) ? 1 : -1;
            } 
            printf("i-banco vai terminar.\n--\n");
            for(int i = 0; i < numPids; i++){
                printf("FILHO TERMINADO (PID=%d; terminou %s)\n",pids[i].pid, (pids[i].estado > 0) ? "normalmente" : "abruptamente");
            }
            printf("--\n");
            sairAgora = 0;

            pthread_mutex_destroy(&semExtMut);
            sem_destroy(&posicoesComInfo);
            sem_destroy(&posicoesSemInfo);
            exit(EXIT_SUCCESS); 
    
        }

    else if (numargs == 0)
        /* Nenhum argumento; ignora e volta a pedir */
        continue;
            
    /* Debitar */
    else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
        int idConta, valor;
        if (numargs < 3) {
            printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
            continue;
        }

        idConta = atoi(args[1]);
        valor = atoi(args[2]);
        changeBuffer(cmd_buffer,idConta,valor,OP_CREDITAR,&buff_write_idx);
    }

    /* Creditar */
    else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
        int idConta, valor;
        if (numargs < 3) {
            printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
            continue;
        }

        idConta = atoi(args[1]);
        valor = atoi(args[2]);

        changeBuffer(cmd_buffer,idConta,valor,OP_CREDITAR,&buff_write_idx);
    }

    /* Ler Saldo */
    else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
        int idConta, saldo;

        if (numargs < 2) {
            printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
            continue;
        }
        idConta = atoi(args[1]);
        saldo = lerSaldo (idConta);
        changeBuffer(cmd_buffer,idConta,-1,OP_CREDITAR,&buff_write_idx);
    }

    /* Simular */
    else if (strcmp(args[0], COMANDO_SIMULAR) == 0 && numargs == 2) {
        int anos;
        pid_t pid;
        if ((anos = atoi(args[1])) <= 0){
            printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
        } else {
            pid = fork();
            if(pid < 0){ // Erro ao fazer fork do processo PAI
                printf("%s: ERRO ao criar processo.ID do fork %d\n",COMANDO_SIMULAR,pid);
                exit(EXIT_FAILURE);
            } else if (pid == 0) { //Criou Processo filho com sucesso
                simular(anos);
                exit(EXIT_SUCCESS);
            } else if (pid > 0){ // Processo PAI
                pids[numPids++].pid = pid; //Vamos guardar os PIDs de todos os processos filho que forem criados
            }
        }
        continue;
    } else {
      printf("Comando desconhecido. Tente de novo.\n");
    }

  } 
}

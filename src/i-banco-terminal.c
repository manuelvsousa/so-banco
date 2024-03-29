/******************************************************************************************
* File Name:    i-banco.c
* Author:       Beatriz Correia (84696) / Manuel Sousa (84740)
* Revision:
* NAME:         Banco - IST/SO - 2016/2017 1º Semestre
* SYNOPSIS:     #include <stdio.h> - I/O regular
                #include <signal.h> - signal(), kill()
                #include <unistd.h> - fork()
                #include <sys/wait.h> - waitpid()
                #include "contas.h" - Prototipos de todas as operações relacionadas com contas
                #include "commandlinereader.h" - Prototipos das funcoes de leitura dos comandos
                #include "parte1.h" - Prototipos das funcoes da parte1 - Defines (macros) dos comandos
                #include "parte2e3.h" - Prototipos e Estruturas usadas na entrega 2 e 3
* DESCRIPTION:  funcao main (i-banco)
* DIAGNOSTICS:  tested
* USAGE:        make clean
                make all
                make run
*****************************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "contas.h"
#include "commandlinereader.h"
#include "parte4.h"

/* Constantes */
#define MAXARGS 4


int server_to_client;
int alreadyOpened = 0;
char myfifo2[BUFFER_SIZE];
int sigpipe = 0;


/* tratamento do signal SIGPIPE que vai ser usada caso o i-banco nao esteja a funcionar */
void tratamentoSigPipe() {
    sigpipe = 1;
}

void sendComandToServer(int client_to_server , int idConta, int idConta2, int valor, int OP) {
    if (signal(SIGPIPE, tratamentoSigPipe) == SIG_ERR) {
        printf("simular: Não foi possivel tratar do sinal\n");
        exit(EXIT_FAILURE);
    }

    time_t start_t, end_t;
    double diff_t;
    char response[BUFFER_SIZE];
    time(&start_t); /* conta o tempo de execucao */

    comando_t comando;
    comando.operacao = OP;
    comando.valor = valor;
    comando.idConta = idConta;
    comando.idConta2 = idConta2;
    comando.terminalPid = getpid();

    strcpy(comando.path, myfifo2);

    /* envia o comado para i i-banco */
    write(client_to_server, &comando, sizeof(comando));

    /* Se nunca tiver aberto o pipe entao falo-a. Apenas faz este processo uma vez */
    if (alreadyOpened == 0) {
        server_to_client = open(comando.path, O_RDONLY);
        alreadyOpened++;
    }

    if ((sigpipe != 1) && (comando.operacao != OP_SIMULAR) && (comando.operacao != OP_SAIR) && (comando.operacao != OP_SAIRAGORA) && (comando.operacao != OP_SAIRTERMINAL)) {
        /* recebe a respota (output) do i-banco */
        read(server_to_client, response, BUFFER_SIZE);
        printf("%s", response);
    }
    time(&end_t);/* para de contar o tempo de execucao */
    diff_t = difftime(end_t, start_t);
    if (sigpipe == 1 && (comando.operacao != OP_SAIRTERMINAL)) {
        printf("O PIPE servidor foi fechado :( \n\n");
    }
    if (sigpipe != 1 && (comando.operacao != OP_SIMULAR) && (comando.operacao != OP_SAIR) && (comando.operacao != OP_SAIRAGORA) && (comando.operacao != OP_SAIRTERMINAL)) {
        printf("Tempo de execucao %.3f segundos\n\n", diff_t);
    }
}


/******************************************************************************************
* main()
*
* Arguments:    argc:  numero de argumentos passados a main
*               argv:  array de strings que contem o conteudo passado como argumento a main
* Returns: int  0
* Description:  Leitura dos comandos do banco e criação de processos nas simulações
*****************************************************************************************/
int main (int argc, char** argv) {
    char *args[MAXARGS + 1];
    char buffer[BUFFER_SIZE];
    int sairAgora = 0;
    int numargs = 0;
    int client_to_server;
    char *myfifo;
    if (argc == 2) {
        myfifo = argv[1];
    } else {
        /* Se nao forem passados argumentos e assumido este pipe por default */
        myfifo = "i-banco-pipe";
    }

    client_to_server = open(myfifo, O_WRONLY);

    snprintf(myfifo2, sizeof(myfifo2), "%s%d", "server_to_client_fifo_", getpid());
    mkfifo(myfifo2, 0666);

    printf("Bem-vinda/o ao i-banco\n\n");

    while (1) {
        numargs = readLineArguments(args, MAXARGS + 1, buffer, BUFFER_SIZE);

        if (numargs < 0 || (numargs > 0  && (strcmp(args[0], COMANDO_SAIR) == 0))) {
            if (numargs < 2) {

                /* Sair Agora */
            } else if (numargs == 2 && strcmp(args[1], COMANDO_AGORA) == 0) {
                sairAgora = 1;
            } else {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
                continue;
            }

            if (sairAgora == 1) {
                sendComandToServer(client_to_server, -1, -1, -1, OP_SAIRAGORA);
            } else {
                sendComandToServer(client_to_server, -1, -1, -1, OP_SAIR);
            }

        }
        else if (numargs == 0)
            /* Nenhum argumento; ignora e volta a pedir */
            continue;

        /* Debitar */
        else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
            if (numargs < 3) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
                continue;
            }
            sendComandToServer(client_to_server, atoi(args[1]), -1, atoi(args[2]), OP_DEBITAR);
        }

        /* Creditar */
        else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
            if (numargs < 3) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
                continue;
            }

            sendComandToServer(client_to_server, atoi(args[1]), -1, atoi(args[2]), OP_CREDITAR);
        }

        /* Ler Saldo */
        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
            if (numargs < 2) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
                continue;
            }
            sendComandToServer(client_to_server, atoi(args[1]), -1, -1, OP_LERSALDO);
        }

        /* Tranferir */
        else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {
            if (numargs != 4) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_TRANSFERIR);
                continue;
            }
            sendComandToServer(client_to_server, atoi(args[1]), atoi(args[2]), atoi(args[3]), OP_TRANSFERIR);
        }
        /* terminal-sair */
        else if (strcmp(args[0], COMANDO_SAIRTERMINAL) == 0) {
            if (numargs != 1) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIRTERMINAL);
                continue;
            }
            sendComandToServer(client_to_server, client_to_server, -1, -1, OP_SAIRTERMINAL);

            if (close(client_to_server) != 0) {
                printf("ERRO: close - client_to_server\n");
            }
            if (close(server_to_client) != 0) {
                printf("ERRO: close - client_to_server\n");
            }

            unlink(myfifo2);
            exit(EXIT_SUCCESS);
        }
        /* Simular */
        else if (strcmp(args[0], COMANDO_SIMULAR) == 0 && numargs == 2 && atoi(args[1]) > 0) {
            sendComandToServer(client_to_server, atoi(args[1]), -1, -1, OP_SIMULAR);
        } else {
            printf("Comando desconhecido. Tente de novo.\n");
        }

    }
}
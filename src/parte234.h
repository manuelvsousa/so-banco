/******************************************************************************************
* File Name:    parte2e3.h
* Author:       Beatriz Correia (84696) / Manuel Sousa (84740)
* Revision:
* NAME:         Banco - IST/SO - 2016/2017 1º Semestre
* SYNOPSIS:     #include <stdio.h> - I/O regular
*               #include <pthread.h>  - tarefas - pthread_mutex_t & pthread_t types, pthread_create,
				pthread_mutex_(un)lock, pthread_exit, pthread_cond_init, pthread_cond_wait
*               #include <stdlib.h>  - exit(), atoi()
*               #include <semaphore.h> - semaforos - sem_init, sem_wait, sem_destroy
*               #include <string.h> - char strings, strerror()
*               #include "contas.h" - Prototipos de todas as operações relacionadas com contas
*               #include "parte1.h" - Prototipos das funcoes da parte 1 - Defines (macros) dos comandos
*				#include "parte4.h" - Prototipos das funcoes exclusivas a parte 4
* DESCRIPTION:  Defines (macros) e Prototipos das funcoes da parte 2 e 3 (tarefas)
* DIAGNOSTICS:  OK
*****************************************************************************************/

#ifndef PARTE234_H
#define PARTE234_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include "contas.h"
#include "parte1.h"
#include "parte4.h"

/* Operações - Comandos */
#define OP_SAIR 1
#define OP_SAIRAGORA 2
#define OP_LERSALDO 3
#define OP_CREDITAR 4
#define OP_DEBITAR 5
#define OP_TRANSFERIR 6 /* Parte 3 */
#define OP_SIMULAR 7  /* Parte 4 */
#define OP_SAIRTERMINAL 8  /* Parte 4 */


/* Numero de tarefas Trabalhadoras */
#define NUM_TRABALHADORAS 3

/* Tamanho de elementos do Buffer - Definido em Funcao das tarefas trabalhadoras */
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))


/* Estrutura do buffer de comandos */
typedef struct {
	int operacao;
	int idConta;
	int idConta2;
	int valor;
	int terminalPid;
	char path[100];
} comando_t;

pthread_t tid[NUM_TRABALHADORAS]; /* Vetor que guarda os Thread ID's de todas as tarefas */

pthread_mutex_t semExMut;/* Mutex de exclusão mutua */
pthread_mutex_t threadsContas[NUM_CONTAS];/* Vetor de Mutexes que associa um Mutex a cada conta */

/* Parte 3 */
pthread_cond_t cheio; /* Variavel de condicao */
int comandosNoBuffer; /* Guarda o numero de comandos atuais no buffer */

sem_t podeProd, podeCons; /* Semáforos do sistema Produtor - Consumidor. */

int buff_write_idx; /* Cursor que guarda o indice da proxima posisao a escrever no buffer */
int buff_read_idx;  /* Cursor que guarda o indice da proxima posisao a ler no buffer */

comando_t cmd_buffer[CMD_BUFFER_DIM]; /* Buffer Circular de comandos */

/* Protótipos das Funções */
void executarComando(comando_t c);
void *lerComandos(void *args);
void inicializarThreadsSemaforosMutexes();
void produtor(comando_t comando);
void killThreadsSemaforosMutexes();

#endif
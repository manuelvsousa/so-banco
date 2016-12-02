/******************************************************************************************
* File Name:    parte4.c
* Author:       Beatriz Correia (84696) / Manuel Sousa (84740)
* Revision:
* NAME:         Banco - IST/SO - 2016/2017 1º Semestre
* SYNOPSIS:     #include "parte4.h" - Prototipos e Estruturas usadas na entrega 4
* DESCRIPTION:  fdsjnkjnsfdjndfskjnsdfjknsdfnjkfsdkjndsfknjsdfjnksdfkjn
* DIAGNOSTICS:  OK
*****************************************************************************************/

#include "parte4.h"

void escreverLog(int comando) {
	pid_t tid;
	char out[BUFFER_SIZE];
	tid = syscall(SYS_gettid);
	if (tid == -1) {
		printf("ERRO: syscall - SYS_gettid\n");
	}
	snprintf(out, sizeof(out), "%d: %s\n", tid, comandos(comando));
	write(fout, out, strlen(out));
}

char* comandos(int comando) {
	switch (comando) {
	case OP_LERSALDO:
		return COMANDO_LER_SALDO;
	case OP_CREDITAR:
		return COMANDO_CREDITAR;
	case OP_DEBITAR:
		return COMANDO_DEBITAR;
	case OP_TRANSFERIR:
		return COMANDO_TRANSFERIR;
	case OP_SIMULAR:
		return COMANDO_SIMULAR;
	case OP_SAIR:
		return COMANDO_SAIR;
	default:
		return "ERRO";
	}
}

void iniciaRedirecionarOutput() {
	char file[BUFFER_SIZE];
	snprintf(file, sizeof(file), "i-banco-sim-%d.txt",  getpid());

	out = open(file, O_TRUNC | O_WRONLY | O_CREAT, 0600);

	if (out == -1) {
		printf("ERRO: open - params: [O_TRUNC | O_RDWR | O_CREAT, 0600]\n");
	}

	if (dup2(out, fileno(stdout)) == -1) {
		printf("ERRO: dup2 - params: [out, fileno(stdout)]\n");
	}
}

void pararRedirecionarOutput() {
	if (fflush(stdout) != 0) {
		printf("ERRO: fflush - params: stdout\n");
	}

	if (close(out) != 0) {
		printf("ERRO: close - params: out\n");
	}

	if (close(save_out) != 0) {
		printf("ERRO: close - params: save_out\n");
	}
}


void escrever(int fileDescriptor, char asd[]) {
	write(fileDescriptor, asd, BUFFER_SIZE);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h> //mmap
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "prodcons.h"
#include "control.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
//==============================================

struct prodcons ProdCons;

//******************************************
// SEMAFORO_CRIAR
//
sem_t * semaphore_create(char * name, int value) {
	//==============================================
	// FUNÇÃO GENÉRICA DE CRIAÇÃO DE UM SEMÁFORO
	//

	//return so_semaphore_create(name, value);
	
    char name_uid[strlen(name) + 10];
	sprintf(name_uid, "/%s_%d", name, getuid());

	sem_t* sem = sem_open(name_uid, O_CREAT, 0xFFFFFFFF, value);
	if(sem == SEM_FAILED){
		char error_id[strlen(name_uid) + 25];
        sprintf(error_id, "sem_open %s", name_uid);
        perror(error_id);
        exit(1);
	}
	return sem;

	//==============================================
}

void prodcons_create_capacidade_portuaria() {
	//==============================================
	// CRIAR MUTEX PARA CONTROLAR O ACESSO A CAPACIDADE_PORTUARIA
	//
	// utilizar a função genérica semaphore_create
	
	//so_prodcons_create_capacidade_portuaria();

	ProdCons.capacidade_portuaria_mutex = semaphore_create(STR_SEM_CAPACIDADE_PORTUARIA_MUTEX, 1);

	//==============================================
}

void prodcons_create_buffers() {
	//==============================================
	// CRIAR SEMAFOROS PARA CONTROLAR O ACESSO AOS 3 BUFFERS
	//
	// utilizar a função genérica semaphore_create

	//so_prodcons_create_buffers();

	ProdCons.request_d_empty = semaphore_create(STR_SEM_DESCRICAO_EMPTY, Config.BUFFER_DESCRICAO);
	ProdCons.request_d_full = semaphore_create(STR_SEM_DESCRICAO_FULL, 0);
	ProdCons.request_d_mutex = semaphore_create(STR_SEM_DESCRICAO_MUTEX, 1);

	ProdCons.request_r_empty = semaphore_create(STR_SEM_PEDIDO_EMPTY, Config.BUFFER_PEDIDO);
	ProdCons.request_r_full = semaphore_create(STR_SEM_PEDIDO_FULL, 0);
	ProdCons.request_r_mutex = semaphore_create(STR_SEM_PEDIDO_MUTEX, 1);

	ProdCons.response_s_empty = semaphore_create(STR_SEM_AGENDAMENTO_EMPTY, Config.BUFFER_AGENDAMENTO);
	ProdCons.response_s_full = semaphore_create(STR_SEM_AGENDAMENTO_FULL, 0);
	ProdCons.response_s_mutex = semaphore_create(STR_SEM_AGENDAMENTO_MUTEX, 1);	

	//==============================================
}

void semaphore_destroy(char * name, void * ptr) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE UM SEMÁFORO
	//

	//so_semaphore_destroy(name, ptr);

	char name_uid[strlen(name) + 10];
	sprintf(name_uid, "/%s_%d", name, getuid());

	if(sem_close(ptr) == -1){
		char error_id[strlen(name_uid) + 25];
        sprintf(error_id, "semaphore_destroy %s", name_uid);
        perror(error_id);
		exit(2);
	}

	if(sem_unlink(name_uid) == -1){
		char error_id[strlen(name_uid) + 25];
        sprintf(error_id, "sem_unlink %s", name_uid);
        perror(error_id);
		exit(3);
	}

	//==============================================
}

void prodcons_destroy() {
	//==============================================
	// DESTRUIR SEMÁFORO E RESPETIVO NOME
	//
	// utilizar a função genérica semaphore_destroy

	//so_prodcons_destroy();

	semaphore_destroy(STR_SEM_CAPACIDADE_PORTUARIA_MUTEX, ProdCons.capacidade_portuaria_mutex);
	
	semaphore_destroy(STR_SEM_DESCRICAO_EMPTY, ProdCons.request_d_empty);
	semaphore_destroy(STR_SEM_DESCRICAO_FULL, ProdCons.request_d_full);
	semaphore_destroy(STR_SEM_DESCRICAO_MUTEX, ProdCons.request_d_mutex);

	semaphore_destroy(STR_SEM_PEDIDO_EMPTY, ProdCons.request_r_empty);
	semaphore_destroy(STR_SEM_PEDIDO_FULL, ProdCons.request_r_full);
	semaphore_destroy(STR_SEM_PEDIDO_MUTEX, ProdCons.request_r_mutex);

	semaphore_destroy(STR_SEM_AGENDAMENTO_EMPTY, ProdCons.response_s_empty);
	semaphore_destroy(STR_SEM_AGENDAMENTO_FULL, ProdCons.response_s_full);
	semaphore_destroy(STR_SEM_AGENDAMENTO_MUTEX, ProdCons.response_s_mutex);

	//==============================================
}

//******************************************
void prodcons_request_d_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER DESCRICAO
	//

	//so_prodcons_request_d_produce_begin();	

	if (sem_wait(ProdCons.request_d_empty) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_EMPTY) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_DESCRICAO_EMPTY, getuid());
        perror(error_id);
	    exit(4);
	}

    if (sem_wait(ProdCons.request_d_mutex) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_DESCRICAO_MUTEX, getuid());
        perror(error_id);
	    exit(5);
	}

	//==============================================
}

//******************************************
void prodcons_request_d_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER DESCRICAO
	//

	//so_prodcons_request_d_produce_end();

	if (sem_post(ProdCons.request_d_full) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_FULL) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_DESCRICAO_FULL, getuid());
        perror(error_id);
	    exit(6);
	}

    if (sem_post(ProdCons.request_d_mutex) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_DESCRICAO_MUTEX, getuid());
        perror(error_id);
	    exit(7);
	}

	//==============================================
}

//******************************************
void prodcons_request_d_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER DESCRICAO
	//

	//so_prodcons_request_d_consume_begin();

	if (sem_wait(ProdCons.request_d_full) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_FULL) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_DESCRICAO_FULL, getuid());
        perror(error_id);
	    exit(8);
	}

    if (sem_wait(ProdCons.request_d_mutex) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_DESCRICAO_MUTEX, getuid());
        perror(error_id);
	    exit(9);
	}

	//==============================================
}

//******************************************
void prodcons_request_d_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER DESCRICAO
	//
	
	//so_prodcons_request_d_consume_end();

	if (sem_post(ProdCons.request_d_empty) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_EMPTY) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_DESCRICAO_EMPTY, getuid());
        perror(error_id);
	    exit(10);
	}

    if (sem_post(ProdCons.request_d_mutex) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_DESCRICAO_MUTEX, getuid());
        perror(error_id);
	    exit(11);
	}

	//==============================================
}

//******************************************
void prodcons_request_r_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER PEDIDO
	//

	//so_prodcons_request_r_produce_begin();

	if (sem_wait(ProdCons.request_r_empty) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_EMPTY) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_PEDIDO_EMPTY, getuid());
        perror(error_id);
	    exit(12);
	}

    if (sem_wait(ProdCons.request_r_mutex) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_PEDIDO_MUTEX, getuid());
        perror(error_id);
	    exit(13);
	}

	//==============================================
}

//******************************************
void prodcons_request_r_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER PEDIDO
	//

	//so_prodcons_request_r_produce_end();

	if (sem_post(ProdCons.request_r_full) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_FULL) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_PEDIDO_FULL, getuid());
        perror(error_id);
	    exit(14);
	}

    if (sem_post(ProdCons.request_r_mutex) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_PEDIDO_MUTEX, getuid());
        perror(error_id);
	    exit(15);
	}

	//==============================================
}

//******************************************
void prodcons_request_r_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER PEDIDO
	//

	//so_prodcons_request_r_consume_begin();

	if (sem_wait(ProdCons.request_r_full) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_FULL) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_PEDIDO_FULL, getuid());
        perror(error_id);
	    exit(16);
	}

    if (sem_wait(ProdCons.request_r_mutex) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_PEDIDO_MUTEX, getuid());
        perror(error_id);
	    exit(17);
	}

	//==============================================
}

//******************************************
void prodcons_request_r_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER PEDIDO
	//

	//so_prodcons_request_r_consume_end();

	if (sem_post(ProdCons.request_r_empty) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_EMPTY) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_PEDIDO_EMPTY, getuid());
        perror(error_id);
	    exit(18);
	}

    if (sem_post(ProdCons.request_r_mutex) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_PEDIDO_MUTEX, getuid());
        perror(error_id);
	    exit(19);
	}

	//==============================================
}

//******************************************
void prodcons_response_s_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER AGENDAMENTO
	//

	//so_prodcons_response_s_produce_begin();

	if (sem_wait(ProdCons.response_s_empty) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_EMPTY) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_AGENDAMENTO_EMPTY, getuid());
        perror(error_id);
	    exit(20);
	}

    if (sem_wait(ProdCons.response_s_mutex) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_AGENDAMENTO_MUTEX, getuid());
        perror(error_id);
	    exit(21);
	}

	//==============================================
}

//******************************************
void prodcons_response_s_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER AGENDAMENTO
	//

	//so_prodcons_response_s_produce_end();

	if (sem_post(ProdCons.response_s_full) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_FULL) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_AGENDAMENTO_FULL, getuid());
        perror(error_id);
	    exit(22);
	}

    if (sem_post(ProdCons.response_s_mutex) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_AGENDAMENTO_MUTEX, getuid());
        perror(error_id);
	    exit(23);
	}

	//==============================================
}

//******************************************
void prodcons_response_s_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER AGENDAMENTO
	//

	//so_prodcons_response_s_consume_begin();

	if (sem_wait(ProdCons.response_s_full) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_FULL) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_AGENDAMENTO_FULL, getuid());
        perror(error_id);
	    exit(24);
	}

    if (sem_wait(ProdCons.response_s_mutex) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_AGENDAMENTO_MUTEX, getuid());
        perror(error_id);
	    exit(25);
	}

	//==============================================
}

//******************************************
void prodcons_response_s_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER AGENDAMENTO
	//

	//so_prodcons_response_s_consume_end();

	if (sem_post(ProdCons.response_s_empty) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_EMPTY) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_AGENDAMENTO_EMPTY, getuid());
        perror(error_id);
	    exit(26);
	}

    if (sem_post(ProdCons.response_s_mutex) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_AGENDAMENTO_MUTEX, getuid());
        perror(error_id);
	    exit(27);
	}

	//==============================================
}

//******************************************
void prodcons_buffers_begin() {
	//==============================================
	// GARANTIR EXCLUSÃO MÚTUA NO ACESSO AOS 3 BUFFERS
	//

	//so_prodcons_buffers_begin();

	if (sem_wait(ProdCons.request_d_mutex) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_DESCRICAO_MUTEX, getuid());
        perror(error_id);
	    exit(28);
	}

	if (sem_wait(ProdCons.request_r_mutex) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_PEDIDO_MUTEX, getuid());
        perror(error_id);
	    exit(29);
	}

	if (sem_wait(ProdCons.response_s_mutex) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_AGENDAMENTO_MUTEX, getuid());
        perror(error_id);
	    exit(30);
	}

	//==============================================
}

//******************************************
void prodcons_buffers_end() {
	//==============================================
	// FIM DA ZONA DE EXCLUSÃO MÚTUA NO ACESSO AOS 3 BUFFERS
	//

	//so_prodcons_buffers_end();

	if (sem_post(ProdCons.request_d_mutex) == -1){
		char error_id[strlen(STR_SEM_DESCRICAO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_DESCRICAO_MUTEX, getuid());
        perror(error_id);
	    exit(28);
	}

	if (sem_post(ProdCons.request_r_mutex) == -1){
		char error_id[strlen(STR_SEM_PEDIDO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_PEDIDO_MUTEX, getuid());
        perror(error_id);
	    exit(29);
	}

	if (sem_post(ProdCons.response_s_mutex) == -1){
		char error_id[strlen(STR_SEM_AGENDAMENTO_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_AGENDAMENTO_MUTEX, getuid());
        perror(error_id);
	    exit(30);
	}

	//==============================================
}

//******************************************
int prodcons_update_capacidade_portuaria(int operacao) {
	//==============================================
	// OBTER MUTEX DA CAPACIDADE PORTUARIA E ATUALIZAR CAPACIDADE PORTUARIA
	//
	// se capacidade_portuaria da operacao > 0 então decrementá-lo de uma unidade e
	//   função devolve 1
	// se capacidade_portuaria da operacao = 0 então função devolve 0

	//return so_prodcons_update_capacidade_portuaria(operacao);

    if (sem_wait(ProdCons.capacidade_portuaria_mutex) == -1){
		char error_id[strlen(STR_SEM_CAPACIDADE_PORTUARIA_MUTEX) + 25];
        sprintf(error_id, "sem_wait %s_%d", STR_SEM_CAPACIDADE_PORTUARIA_MUTEX, getuid());
        perror(error_id);
	    exit(31);
	}

	int ret = -1;

	if (Config.capacidade_portuaria[operacao] > 0){
		Config.capacidade_portuaria[operacao]--;
		ret = 1;
	} else if (Config.capacidade_portuaria[operacao] == 0){
		ret = 0;
	}

	if (sem_post(ProdCons.capacidade_portuaria_mutex) == -1){
		char error_id[strlen(STR_SEM_CAPACIDADE_PORTUARIA_MUTEX) + 25];
        sprintf(error_id, "sem_post %s_%d", STR_SEM_CAPACIDADE_PORTUARIA_MUTEX, getuid());
        perror(error_id);
	    exit(30);
	}

	return ret;

	//==============================================
}

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
#include <ctype.h>
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "sotime.h"

struct timespec t_inicial;
long intervalo_alarme;

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
extern struct statistics Ind;
//==============================================

struct timespec t_inicial;
long intervalo_alarme;

void time_begin(long intervalo) {
    //==============================================
    // INICIAR ESTRUTURA t_inicial COM VALOR DE RELOGIO (CLOCK_REALTIME)
	//
	// funções de tempo:
	// - clock_gettime() dá um resultado em nanosegundos
	// - gettimeofday()  dá um resultado em milisegundos
	// como a função clock_gettime() dá um valor mais preciso do que gettimeofday()
	// deve ser usada clock_gettime()
	//
    // fazer:
	// - se intervalo!=0 então intervalo_alarme = intervalo
	// - se intervalo!=0 então chamar time_setup_alarm();
	// - iniciar estrutura t_inicial com clock_gettime usando CLOCK_REALTIME
	
	if (intervalo != 0){
        intervalo_alarme = intervalo;
        time_setup_alarm();
    }
    clock_gettime(CLOCK_REALTIME, &t_inicial);

    //so_time_begin(intervalo);
    //==============================================
}

void time_destroy(long intervalo)
{
    //==============================================
    // DESATIVAR ALARME
    //
	// ignorar SIGALRM

	signal(SIGALRM, SIG_IGN);

    //so_time_destroy(intervalo);
    //==============================================
}

void time_setup_alarm() {
    //==============================================
    // ARMAR ALARME DE ACORDO COM intervalo_alarme (SIGNAL E SETTIMER)
    //
	// fazer:
	// - associar SIGALRM com a função time_write_log_timed
	// - usar setitimer preenchendo apenas os campos value da estrutura

	signal(SIGALRM, time_write_log_timed);

    struct itimerval timer;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = intervalo_alarme;

    setitimer(ITIMER_REAL, &timer, 0);

    //so_time_setup_alarm();
    //==============================================
}

void time_write_log_timed(int signum) {
    //==============================================
    // ESCREVER LOG NO ECRAN DE FORMA TEMPORIZADA 
    //
	// rearmar alarme chamando novamente time_setup_alarm
	// escrever para o ecrã a informação esperada

	time_setup_alarm();

    printf("\tOperacoes:		");
    int i;

    for (i = 0; i < Config.OPERATIONS; i++){
        printf("0%i	", i);
    }

    printf("\n\tCapacidade portuaria:	");

    for (i = 0; i < Config.OPERATIONS; i++){
        printf("0%i	", Config.capacidade_portuaria[i]);
    }

    printf("\n");

    //so_time_write_log_timed(signum);
    //==============================================
}

double time_difference(struct timespec t1, struct timespec t2) {
    //==============================================
    // CALCULAR A DIFERENCA, EM NANOSEGUNDOS, ENTRE t1 E t2
    // o resultado deve estar em segundos representado como um double
	// realizar as operações aritméticas necessárias para obter o resultado

    double res=0;
    if (t1.tv_nsec > t2.tv_nsec) {
        res= t1.tv_nsec - t2.tv_nsec ;
    } else {
        res = t2.tv_nsec - t1.tv_nsec;
    }
    return res * 0.000000001;

    // 	return so_time_difference(t1,t2);
    //==============================================
}

double time_untilnow() {
    //==============================================
    // CALCULAR O INTERVALO DE TEMPO ENTRE t_inicial E O INSTANTE ATUAL
    //
	// fazer:
	// - obter o tempo atual com clock_gettime
	// - chamar time_difference

    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    return time_difference(t_inicial, current);

	//return so_time_untilnow();
    //==============================================
}

void time_register(struct timespec *t) {
    //==============================================
    // REGISTAR O TEMPO ATUAL EM t (CLOCK_REALTIME)
    //
	// usar clock_gettime com CLOCK_REALTIME

    clock_gettime(CLOCK_REALTIME, t);

    // 	so_time_register(t);
    //==============================================
}

void time_processing_order() {
	//==============================================
	// ADORMECER UM NUMERO ALEATORIO DE SEGUNDOS
	// MAXIMO DE 100 MILISEGUNDOS E MINIMO 1 MILISEGUNDO
	//
	// usar funcoes usleep, rand e % (resto divisao inteira)
	
	int randomTime = (rand() %100 + 1) * 1000;
	usleep(randomTime);

    // 	so_time_processing_order();
	//==============================================
}

float time_average_statistic(int id) {
	//==============================================================
	// CALCULAR MEDIA DOS TEMPOS DOS PEDIDOS DO CLIENTE
	//
	// percorrer Ind.tempos com duplo for, usando Config.CLIENTES
	// e Config.N para indexar, ver quais os tempos do cliente id e
	// fazer a media dos que são != 0. Se o cliente não tiver tempos,
	// devolver 0.

	//return so_time_average_statistic(id);

	int com_tempos = 0;
	float sum_tempos = 0;

	for (int i = 0; i < Config.CLIENTES; i++){
		for (int j = 0; j < Config.N; j++){			
			if (Ind.tempos[i * Config.N + j].time != 0 && Ind.tempos[i * Config.N + j].cliente == id){
	 			sum_tempos = sum_tempos + Ind.tempos[i * Config.N + j].time;
	 			com_tempos++;
		 	}
		}
	}

	if (com_tempos > 0){
		return sum_tempos / com_tempos;
	} else {
		return 0;
	}

	//==============================================
}

float time_minimum_statistic(int id) {
	//==============================================================
	// CALCULAR MINIMO DOS TEMPOS DOS PEDIDOS DO CLIENTE
	//
	// percorrer Ind.tempos com duplo for, usando Config.CLIENTES
	// e Config.N para indexar, ver quais os tempos do cliente id e
	// fazer o minimo dos que são != 0. Se o cliente não tiver tempos,
	// devolver 0.

	//return so_time_minimum_statistic(id);

	// limits.h nao define FLT_MAX e time.c nao faz #include <float.h>
	// define-se entao um tempo suficientemente grande no contexto deste trabalho
	float min_tempo = 99999; 

	for (int i = 0; i < Config.CLIENTES; i++){
		for (int j = 0; j < Config.N; j++){			
			if (Ind.tempos[i * Config.N + j].time != 0 && Ind.tempos[i * Config.N + j].time < min_tempo && Ind.tempos[i * Config.N + j].cliente == id){
	 			min_tempo = Ind.tempos[i * Config.N + j].time;
		 	}
		}
	}

	if (min_tempo < 99999){
		return min_tempo;
	} else {
		return 0;
	}

	//==============================================
}

float time_maximum_statistic(int id) {
	//==============================================================
	// CALCULAR MAXIMO DO TEMPO DOS PEDIDOS DO CLIENTE
	//
	// percorrer Ind.tempos com duplo for, usando Config.CLIENTES
	// e Config.N para indexar, ver quais os tempos do cliente id e
	// fazer o maximo dos que são != 0. Se o cliente não tiver tempos,
	// devolver 0.

	//return so_time_maximum_statistic(id);

	float max_tempo = 0;

	for (int i = 0; i < Config.CLIENTES; i++){
		for (int j = 0; j < Config.N; j++){			
			if (Ind.tempos[i * Config.N + j].time != 0 && Ind.tempos[i * Config.N + j].time > max_tempo && Ind.tempos[i * Config.N + j].cliente == id){
	 			max_tempo = Ind.tempos[i * Config.N + j].time;
		 	}
		}
	}

	if (max_tempo > 0){
		return max_tempo;
	} else {
		return 0;
	}

	//==============================================
}

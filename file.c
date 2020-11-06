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
#include "memory.h"
#include "prodcons.h"
#include "file.h"
#include "sotime.h"
#include "scheduler.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
//para o log
extern struct request_d BDescricao; 	// buffer cliente-intermediario (request_description)
extern struct request_r BPedido; 	// buffer intermediario-empresa (request_operation)
extern struct response_s BAgendamento;  	// buffer empresa-cliente (response_scheduling)
//==============================================

struct file Ficheiros; // informação sobre nomes e handles de ficheiros

void file_begin(char *fic_in, char *fic_out, char *fic_log) {
	//==============================================
	// GUARDAR NOMES DOS FICHEIROS NA ESTRUTURA Ficheiros
	//
	Ficheiros.entrada = (char *) malloc(sizeof(char) * strlen(fic_in));
	strcpy(Ficheiros.entrada, fic_in);
	Ficheiros.saida = (char *) malloc(sizeof(char) * strlen(fic_out));
	strcpy(Ficheiros.saida, fic_out);
	Ficheiros.log = (char *) malloc(sizeof(char) * strlen(fic_log));
	strcpy(Ficheiros.log, fic_log);
	//so_file_begin(fic_in, fic_out, fic_log);
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE ENTRADA
	// em modo de leitura
	Ficheiros.h_in = fopen(fic_in, "r");
    
    if (Ficheiros.h_in == NULL) {
		perror("Ficheiros.h_in nao existe");
		exit(1);
	}
	//so_file_open_file_in();
	//==============================================

	// parse do ficheiro de teste
	// esta funcao prepara os campos da estrutura Config (char *)
	// com os dados do ficheiro de entrada
	if (ini_parse_file(Ficheiros.h_in, handler, &Config) < 0) {
		printf("Erro a carregar o ficheiro de teste'\n");
		exit(2);
	}

	// agora e' preciso inicializar os restantes campos da estrutura Config

	//==============================================
	// CONTAR OPERACOES
	// usar strtok para percorrer Config.list_operacoes
	// guardar resultado em Config.OPERACOES
	int count = 0;
	//E preciso duplicar a list_operacoes visto que ao usarmos o strtok ira ser destruido o Config.list_operacoes
    char *copy = strdup(Config.list_operacoes);
	char *token = strtok(Config.list_operacoes, " ");

	while(token != NULL){
		token = strtok(NULL, " ");
		count++;
	}
    //Nao existe Config.OPERACOES mas sim Config.OPERATIONS
	Config.OPERATIONS = count;
	//so_file_count_operacoes();
	//==============================================

	// iniciar memoria para o vetor com o capacidade portuaria e semaforo
	memory_create_capacidade_portuaria();
	prodcons_create_capacidade_portuaria();

	//==============================================
	// LER CAPACIDADE DE CADA EMPRESA
	// percorrer Config.list_operacoes e
	// guardar cada valor no vetor Config.capacidade_portuaria
	count = 0;
	token = strtok(copy, " ");

	while(token != NULL){
		Config.capacidade_portuaria[count] = atoi(token);
		token = strtok(NULL, " ");
		count++;
	}
	//libertar buffer alocado pelo strdup
	free(copy);
	//so_file_read_capacidade_portuaria();
	//==============================================

	//==============================================
	// LER NR DE OPERACOES DE CADA CLIENTE
	// guardar valor de Config.nr_operacoes em Config.N
    count = 0;
	token = strtok(Config.nr_operacoes, " ");

	while(token != NULL){
		Config.N = atoi(token);
		token = strtok(NULL, " ");
		count++;
	}
    //so_file_read_numero_operacoes();
	//==============================================

	//==============================================
	// CONTAR CLIENTES
	// usar strtok para percorrer Config.list_clientes
	// guardar resultado em Config.CLIENTES
	count = 0;
	token = strtok(Config.list_clientes, " ");

	while(token != NULL){
		token = strtok(NULL, " ");
		count++;
	}

	Config.CLIENTES = count;
	//so_file_count_clientes();
	//==============================================

	//==============================================
	// CONTAR INTERMEDIARIOS
	// usar strtok para percorrer Config.list_intermediarios
	// guardar resultado em Config.INTERMEDIARIOS
	count = 0;
	token = strtok(Config.list_intermediarios, " ");

	while(token != NULL){
		token = strtok(NULL, " ");
		count++;
	}
	//Nao existe Config.INTERMEDIARIOS mas sim Config.INTERMEDIARIO
	Config.INTERMEDIARIO = count;
	//so_file_count_intermediarios();
	//==============================================

	//==============================================
	// CONTAR EMPRESAS
	// usar strtok para percorrer Config.list_empresas
	// guardar resultado em Config.EMPRESAS
	count = 0;
	token = strtok(Config.list_empresas, ",");

	while(token != NULL){
		token = strtok(NULL, ",");
		count++;
	}

	Config.EMPRESAS = count;
	//so_file_count_empresas();
	//==============================================

    //Nao e preciso implementar
	so_file_read_operacoes();

	//==============================================
	// LER CAPACIDADES DOS BUFFERS
	// usar strtok para percorrer Config.list_buffers
	// guardar primeiro tamanho em Config.BUFFER_DESCRICAO
	// guardar segundo tamanho em Config.BUFFER_PEDIDO
	// guardar terceiro tamanho em Config.BUFFER_AGENDAMENTO
	Config.BUFFER_DESCRICAO = atoi(strtok(Config.list_buffers," "));
	Config.BUFFER_PEDIDO = atoi(strtok(NULL," "));
	Config.BUFFER_AGENDAMENTO = atoi(strtok(NULL," "));
	//so_file_read_capacidade_buffer();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE SAIDA (se foi especificado)
	// em modo de escrita
	if(Ficheiros.saida != NULL){
		Ficheiros.h_out = fopen(fic_out, "w");
    }
	//so_file_open_file_out();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE LOG (se foi especificado)
	// em modo de escrita
	if(Ficheiros.log != NULL){
		Ficheiros.h_log = fopen(fic_log, "w");
    }
	//so_file_open_file_log();
	//==============================================
}

void file_destroy() {
	//==============================================
	// LIBERTAR ZONAS DE MEMÓRIA RESERVADAS DINAMICAMENTE
	// que podem ser: Ficheiros.entrada, Ficheiros.saida, Ficheiros.log
	free(Ficheiros.entrada);
	free(Ficheiros.saida);
	free(Ficheiros.log);
	//so_file_destroy();
	//==============================================
}

void file_write_log_file(int etapa, int id) {
	double t_diff;

	if (Ficheiros.h_log != NULL) {

		prodcons_buffers_begin();

		// guardar timestamp
		t_diff = time_untilnow();

		//==============================================
		// ESCREVER DADOS NO FICHEIRO DE LOG
		//
		// o log deve seguir escrupulosamente o formato definido
		
		int um = ~0;
		int zero = 0;
		int i;
		
		// sequência de 4 inteiros com todos os bits a um
		fwrite(&um, sizeof(int),1,Ficheiros.h_log);
		fwrite(&um, sizeof(int),1,Ficheiros.h_log);
		fwrite(&um, sizeof(int),1,Ficheiros.h_log);
		fwrite(&um, sizeof(int),1,Ficheiros.h_log);
		
		// registo contém o tempo decorrido (double), a etapa em que foi gerado (int), 
		// o id de quem o gerou (int) (cliente, intermediário ou empresa)
		fwrite(&t_diff,sizeof(double),1,Ficheiros.h_log);
		fwrite(&etapa,sizeof(int),1,Ficheiros.h_log);
		fwrite(&id,sizeof(int),1,Ficheiros.h_log);
		
        // separa-se com uma sequência de 2 inteiros com todos os bits a 1 e mais 2 inteiros com todos os bits a 0
		fwrite(&um, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&um, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&zero, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&zero, sizeof(int), 1, Ficheiros.h_log);
        
		//e o conteúdo das posições ocupadas dos 3 buffers, respetivamente: BDescricao, BPedido e BAgendamento (múltiplos de 4 int).
		for(i=0; i<Config.BUFFER_DESCRICAO; i++){
			if(BDescricao.buffer[i].disponibilidade == 1){
				//4 identificadores que são: id da operação (int), id do cliente (int), id do intermediário (int) e id da empresa (int)
				fwrite(&BDescricao.buffer[i].id, sizeof(int), 1, Ficheiros.h_log);
				fwrite(&BDescricao.buffer[i].cliente, sizeof(int), 1, Ficheiros.h_log);
				fwrite(&BDescricao.buffer[i].intermediario, sizeof(int), 1, Ficheiros.h_log);
				fwrite(&BDescricao.buffer[i].empresa, sizeof(int), 1, Ficheiros.h_log);
			}
		}
        
        //separa-se com uma sequência de 2 inteiros com todos os bits a 1 e mais 2 inteiros com todos os bits a 0
		fwrite(&um, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&um, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&zero, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&zero, sizeof(int), 1, Ficheiros.h_log);
        
		for(i=0; i<Config.BUFFER_PEDIDO; i++) {
            if(BPedido.buffer[i].disponibilidade == 1){
                //4 identificadores que são: id da operação (int), id do cliente (int), id do intermediário (int) e id da empresa (int)
                fwrite(&BPedido.buffer[i].id, sizeof(int), 1, Ficheiros.h_log);
                fwrite(&BPedido.buffer[i].cliente, sizeof(int), 1, Ficheiros.h_log);
                fwrite(&BPedido.buffer[i].intermediario, sizeof(int), 1, Ficheiros.h_log);
                fwrite(&BPedido.buffer[i].empresa, sizeof(int), 1, Ficheiros.h_log);
            }
		}
        
        //separa-se com uma sequência de 2 inteiros com todos os bits a 1 e mais 2 inteiros com todos os bits a 0
		fwrite(&um, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&um, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&zero, sizeof(int), 1, Ficheiros.h_log);
		fwrite(&zero, sizeof(int), 1, Ficheiros.h_log);
        
		for(i=0; i<Config.BUFFER_AGENDAMENTO; i++) {
            if(BAgendamento.buffer[i].disponibilidade == 1){
                //4 identificadores que são: id da operação (int), id do cliente (int), id do intermediário (int) e id da empresa (int)
                fwrite(&BAgendamento.buffer[i].id, sizeof(int), 1, Ficheiros.h_log);
                fwrite(&BAgendamento.buffer[i].cliente, sizeof(int), 1, Ficheiros.h_log);
                fwrite(&BAgendamento.buffer[i].intermediario, sizeof(int), 1, Ficheiros.h_log);
                fwrite(&BAgendamento.buffer[i].empresa, sizeof(int), 1, Ficheiros.h_log);
            }
		}
        
		//so_file_write_log_file(etapa, id, t_diff);
		//==============================================

		prodcons_buffers_end();
	}
}

void file_write_line(char * linha) {
	//==============================================
	// ESCREVER UMA LINHA NO FICHEIRO DE SAIDA
	//
	if(Ficheiros.h_out != NULL){
		fputs(linha, Ficheiros.h_out);
    }
	//so_file_write_line(linha);
	//==============================================
}

int stricmp(const char *s1, const char *s2) {
	if (s1 == NULL)
		return s2 == NULL ? 0 : -(*s2);
	if (s2 == NULL)
		return *s1;

	char c1, c2;
	while ((c1 = tolower(*s1)) == (c2 = tolower(*s2))) {
		if (*s1 == '\0')
			break;
		++s1;
		++s2;
	}

	return c1 - c2;
}

int handler(void* user, const char* section, const char* name,
		const char* value) {
	struct configuration* pconfig = (struct configuration*) user;

#define MATCH(s, n) stricmp(section, s) == 0 && stricmp(name, n) == 0
	if (MATCH("operacoes", "capacidade_portuaria")) {
		pconfig->list_operacoes = strdup(value);
	} else if (MATCH("clientes", "operacao")) {
		pconfig->list_clientes = strdup(value);
	} else if (MATCH("clientes", "N")) {
		pconfig->nr_operacoes = strdup(value);
	} else if (MATCH("intermediarios", "list")) {
		pconfig->list_intermediarios = strdup(value);
	} else if (MATCH("empresas", "operacoes")) {
		pconfig->list_empresas = strdup(value);
	} else if (MATCH("buffers", "capacidade_buffer")) {
		pconfig->list_buffers = strdup(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}


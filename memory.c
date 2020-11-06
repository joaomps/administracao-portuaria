/*
SO-051
Ivo Veiga, nº 44865
Joao Silva, nº 48782
Manuel Tovar, nº 49522
*/

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
#include "memory.h"
#include "prodcons.h"
#include "control.h"
#include "file.h"
#include "sotime.h"
#include "scheduler.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
extern struct scheduler Schedule;
extern struct statistics Ind;
//==============================================

struct request_d BDescricao;    // buffer cliente-intermediario (request_description)
struct request_r BPedido;       // buffer intermediario-empresa (request_operation)
struct response_s BAgendamento; // buffer empresa-cliente (response_scheduling)

//******************************************
// CRIAR ZONAS DE MEMORIA
//
void *memory_create(char *name, int size)
{
    //==============================================
    // FUNÇÃO GENÉRICA DE CRIAÇÃO DE MEMÓRIA PARTILHADA
    //
    // usar getuid() para gerar nome unico na forma:
    // sprintf(name_uid,"/%s_%d", name, getuid())
    // usar name_uid em shm_open
    // usar tambem: ftruncate e mmap

    //return so_memory_create(name, size);

    char name_uid[strlen(name) + 10]; //+10 no. digitos max do uid
    sprintf(name_uid, "/%s_%d", name, getuid());

    int fd = shm_open(name_uid, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        char error_id[strlen(name_uid) + 20];
        sprintf(error_id, "shm_open_%s", name_uid);
        perror(error_id);
        exit(1);
    }

    if (ftruncate(fd, size) == -1)
    {
        char error_id[strlen(name_uid) + 20];
        sprintf(error_id, "ftruncate_%s", name_uid);
        perror(error_id);
        exit(2);
    }

    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        char error_id[strlen(name_uid) + 20];
        sprintf(error_id, "mmap_%s", name_uid);
        perror(error_id);
        exit(3);
    }

    return ptr;

    //==============================================
}
void memory_create_capacidade_portuaria()
{
    //==============================================
    // CRIAR ZONA DE MEMÓRIA PARA A CAPACIDADE DE PORTUARIA
    //
    // utilizar a função genérica memory_create(char *,int)
    // para criar ponteiro que se guarda em Config.capacidade_portuaria
    // que deve ter capacidade para um vetor unidimensional
    // com a dimensao [OPERACOES] para inteiro
    //so_memory_create_capacidade_portuaria();

    Config.capacidade_portuaria = memory_create(STR_SHM_CAPACIDADE_PORTUARIA, sizeof(int) * Config.OPERATIONS);

    //==============================================
}
void memory_create_buffers()
{
    //==============================================
    // CRIAR ZONAS DE MEMÓRIA PARA OS BUFFERS: DESCRICAO, PEDIDO e AGENDAMENTO
    //
    // utilizar a função genérica memory_create(char *,int)
    //
    // para criar ponteiro que se guarda em BAgendamento.ptr
    // que deve ter capacidade para struct pointers
    // para criar ponteiro que se guarda em BAgendamento.buffer
    // que deve ter capacidade para um vetor unidimensional
    // com a dimensao [BUFFER_AGENDAMENTO] para struct operation
    //
    // para criar ponteiro que se guarda em BPedido.ptr
    // que deve ter capacidade para struct pointers
    // para criar ponteiro que se guarda em BPedido.buffer
    // que deve ter capacidade para um vetor unidimensional
    // com a dimensao [BUFFER_PEDIDO] para struct operation
    //
    // para criar ponteiro que se guarda em BDescricao.ptr
    // que deve ter capacidade para um vetor unidimensional
    // de inteiros com a dimensao [BUFFER_DESCRICAO]
    // para criar ponteiro que se guarda em BDescricao.buffer
    // que deve ter capacidade para um vetor unidimensional
    // com a dimensao [BUFFER_DESCRICAO] para struct operation
    //so_memory_create_buffers();

    BAgendamento.ptr = memory_create(STR_SHM_AGENDAMENTO_PTR, sizeof(struct pointers));
    BAgendamento.buffer = memory_create(STR_SHM_AGENDAMENTO_BUFFER, sizeof(struct operation) * Config.BUFFER_AGENDAMENTO);

    BPedido.ptr = memory_create(STR_SHM_PEDIDO_PTR, sizeof(struct pointers));
    BPedido.buffer = memory_create(STR_SHM_PEDIDO_BUFFER, sizeof(struct operation) * Config.BUFFER_PEDIDO);

    BDescricao.ptr = memory_create(STR_SHM_DESCRICAO_PTR, sizeof(int) * Config.BUFFER_DESCRICAO);
    BDescricao.buffer = memory_create(STR_SHM_DESCRICAO_BUFFER, sizeof(struct operation) * Config.BUFFER_DESCRICAO);

    //==============================================
}
void memory_create_scheduler()
{
    //==============================================
    // CRIAR ZONA DE MEMÓRIA PARA O MAPA DE ESCALONAMENTO
    //
    // utilizar a função genérica memory_create(char *,int)
    // para criar ponteiro que se guarda em Schedule.ptr
    // que deve ter capacidade para um vetor bidimensional
    // com a dimensao [OPERACOES,EMPRESAS] para inteiro

    //so_memory_create_scheduler();

    Schedule.ptr = memory_create(STR_SHM_SCHEDULER, sizeof(int) * Config.OPERATIONS * Config.EMPRESAS);

    //==============================================
}

void memory_destroy(char *name, void *ptr, int size)
{
    //==============================================
    // FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE MEMÓRIA PARTILHADA
    //
    // usar getuid() para gerar nome unico na forma:
    // sprintf(name_uid,"/%s_%d", name, getuid())
    // usar name_uid em shm_unlink
    // usar tambem: munmap

    //so_memory_destroy(name, ptr, size);

    char name_uid[strlen(name) + 10]; //+10 no. digitos max do uid
    sprintf(name_uid, "/%s_%d", name, getuid());

    if (munmap(ptr, size) == -1)
    {
        char error_id[strlen(name_uid) + 20];
        sprintf(error_id, "munmap_%s", name_uid);
        perror(error_id);
        exit(4);
    }

    if (shm_unlink(name_uid) == -1)
    {
        char error_id[strlen(name_uid) + 20];
        sprintf(error_id, "shm_unlink_%s", name_uid);
        perror(error_id);
        exit(5);
    }

    //==============================================
}

//******************************************
// MEMORIA_DESTRUIR
//
void memory_destroy_all()
{
    //==============================================
    // DESTRUIR MAPEAMENTO E NOME DE PÁGINAS DE MEMÓRIA
    //
    // utilizar a função genérica memory_destroy(char *,void *,int)

    //so_memory_destroy_all();

    memory_destroy(STR_SHM_CAPACIDADE_PORTUARIA, Config.capacidade_portuaria, sizeof(int) * Config.OPERATIONS);

    memory_destroy(STR_SHM_AGENDAMENTO_PTR, BAgendamento.ptr, sizeof(struct pointers));
    memory_destroy(STR_SHM_AGENDAMENTO_BUFFER, BAgendamento.buffer, sizeof(struct operation) * Config.BUFFER_AGENDAMENTO);

    memory_destroy(STR_SHM_PEDIDO_PTR, BPedido.ptr, sizeof(struct pointers));
    memory_destroy(STR_SHM_PEDIDO_BUFFER, BPedido.buffer, sizeof(struct operation) * Config.BUFFER_PEDIDO);

    memory_destroy(STR_SHM_DESCRICAO_PTR, BDescricao.ptr, sizeof(int) * Config.BUFFER_DESCRICAO);
    memory_destroy(STR_SHM_DESCRICAO_BUFFER, BDescricao.buffer, sizeof(struct operation) * Config.BUFFER_DESCRICAO);

    memory_destroy(STR_SHM_SCHEDULER, Schedule.ptr, sizeof(int) * Config.OPERATIONS * Config.EMPRESAS);

    free(Ind.capacidade_inicial_portuaria);
    free(Ind.pid_clientes);
    free(Ind.pid_intermediarios);
    free(Ind.pid_empresas);
    free(Ind.clientes_servidos_por_intermediarios);
    free(Ind.clientes_servidos_por_empresas);

    memory_destroy(STR_SHM_OPREMPRESAS, Ind.agendamentos_entregues_por_empresas, sizeof(int) * Config.OPERATIONS * Config.EMPRESAS);

    free(Ind.servicos_recebidos_por_clientes);

    //==============================================
}

//******************************************
// memory_request_d_write
//
void memory_request_d_write(int id, struct operation *operacao)
{
    prodcons_request_d_produce_begin();

    // registar hora do pedido de servico
    time_register(&operacao->time_descricao);

    //==============================================
    // ESCREVER DESCRICAO DE OPERACOES NO BUFFER DESCRICOES
    //
    // procurar posicao vazia no buffer BDescricao
    // copiar conteudo relevante da estrutura operacao para
    // a posicao BDescricao.ptr do buffer BDescricao
    // conteudo: cliente, id, time_descricao
    // colocar BDescricao.ptr[n] = 1 na posicao respetiva

    //so_memory_request_d_write(id, operacao);

    for (int i = 0; i < Config.BUFFER_DESCRICAO; i++)
    {
        if (BDescricao.ptr[i] == 0)
        {
            BDescricao.buffer[i].cliente = operacao->cliente;
            BDescricao.buffer[i].id = operacao->id;
            BDescricao.buffer[i].time_descricao = operacao->time_descricao;

            BDescricao.ptr[i] = 1;

            break;
        }
    }

    //==============================================

    prodcons_request_d_produce_end();

    // informar INTERMEDIARIO de pedido de operacao
    control_cliente_submete_descricao(id);

    // log
    file_write_log_file(1, id);
}
//******************************************
// memory_request_d_read
//
int memory_request_d_read(int id, struct operation *operacao)
{
    // testar se existem clientes e se o SO_AdmPor está open
    if (control_intermediario_esperapor_descricao(id) == 0)
        return 0;

    prodcons_request_d_consume_begin();

    //==============================================
    // LER DESCRICAO DO BUFFER DE DESCRICOES
    //
    // procurar descrição de operação para a empresa no buffer BDescricao
    // copiar conteudo relevante da posicao encontrada
    // no buffer BDescricao para operacao
    // conteudo: cliente, id, time_descricao
    // colocar BDescricao.ptr[n] = 0 na posicao respetiva

    //so_memory_request_d_read(id, operacao);

    for (int i = 0; i < Config.BUFFER_DESCRICAO; i++)
    {
        if (BDescricao.ptr[i] == 1)
        {
            operacao->cliente = BDescricao.buffer[i].cliente;
            operacao->id = BDescricao.buffer[i].id;
            operacao->time_descricao = BDescricao.buffer[i].time_descricao;

            BDescricao.ptr[i] = 0;

            break;
        }
    }

    //==============================================

    // testar se existe capacidade portuaria para servir cliente
    if (prodcons_update_capacidade_portuaria(operacao->id) == 0)
    {
        operacao->disponibilidade = 0;
        prodcons_request_d_consume_end();
        return 2;
    }
    else
        operacao->disponibilidade = 1;

    prodcons_request_d_consume_end();

    // log
    file_write_log_file(2, id);

    return 1;
}

//******************************************
// memory_request_r_write
//
void memory_request_r_write(int id, struct operation *operacao)
{
    int pos, empresa;

    prodcons_request_r_produce_begin();

    // decidir a que empresa se destina
    empresa = scheduler_get_empresa(id, operacao->id);

    //==============================================
    // ESCREVER PEDIDO NO BUFFER DE PEDIDOS DE OPERACOES
    //
    // copiar conteudo relevante da estrutura operacao para
    // a posicao BDescricao.ptr->in do buffer BPedido
    // conteudo: cliente, id, disponibilidade, intermediario, empresa, time_descricao
    // e atualizar BPedido.ptr->in

    //pos = so_memory_request_r_write(id, operacao, empresa);

    BPedido.buffer[BPedido.ptr->in].cliente = operacao->cliente;
    BPedido.buffer[BPedido.ptr->in].id = operacao->id;
    BPedido.buffer[BPedido.ptr->in].disponibilidade = operacao->disponibilidade;
    BPedido.buffer[BPedido.ptr->in].intermediario = operacao->intermediario;
    BPedido.buffer[BPedido.ptr->in].empresa = operacao->empresa;
    BPedido.buffer[BPedido.ptr->in].time_descricao = operacao->time_descricao;

    BPedido.ptr->in = (BPedido.ptr->in + 1) % Config.BUFFER_PEDIDO;

    pos = BPedido.ptr->in;

    //==============================================

    prodcons_request_r_produce_end();

    // informar empresa respetiva de pedido de operacao
    control_intermediario_submete_pedido(empresa);

    // registar hora pedido (operacao)
    time_register(&BAgendamento.buffer[pos].time_pedido);

    // log
    file_write_log_file(3, id);
}
//******************************************
// memory_request_r_read
//
int memory_request_r_read(int id, struct operation *operacao)
{
    // testar se existem pedidos e se o SO_AdmPor está open
    if (control_empresa_esperapor_pedido(id) == 0)
        return 0;

    prodcons_request_r_consume_begin();

    //==============================================
    // LER PEDIDO DO BUFFER DE PEDIDOS DE OPERACOES
    //
    // copiar conteudo relevante dessa posicao BPedido.ptr->out
    // do buffer BPedido para a estrutura operacao
    // conteudo: cliente, id, disponibilidade, intermediario, time_descricao, time_pedido
    // e atualizar BPedido.ptr->out

    //so_memory_request_r_read(id, operacao);

    operacao->cliente = BPedido.buffer[BPedido.ptr->out].cliente;
    operacao->id = BPedido.buffer[BPedido.ptr->out].id;
    operacao->disponibilidade = BPedido.buffer[BPedido.ptr->out].disponibilidade;
    operacao->intermediario = BPedido.buffer[BPedido.ptr->out].intermediario;
    operacao->time_descricao = BPedido.buffer[BPedido.ptr->out].time_descricao;
    operacao->time_pedido = BPedido.buffer[BPedido.ptr->out].time_pedido;

    BPedido.ptr->out = (BPedido.ptr->out + 1) % Config.BUFFER_PEDIDO;

    //==============================================

    prodcons_request_r_consume_end();

    // log
    file_write_log_file(4, id);

    return 1;
}

//******************************************
// memory_response_s_write
//
void memory_response_s_write(int id, struct operation *operacao)
{
    int pos;

    prodcons_response_s_produce_begin();

    //==============================================
    // ESCREVER AGENDAMENTO NO BUFFER DE AGENDAMENTO
    //
    // copiar conteudo relevante da estrutura operacao para
    // a posicao BAgendamento.ptr->in do buffer BAgendamento
    // conteudo: cliente, id, disponibilidade, intermediario, empresa, time_descricao, time_pedido
    // e atualizar BAgendamento.ptr->in

    //pos = so_memory_response_s_write(id, operacao);

    BAgendamento.buffer[BAgendamento.ptr->in].cliente = operacao->cliente;
    BAgendamento.buffer[BAgendamento.ptr->in].id = operacao->id;
    BAgendamento.buffer[BAgendamento.ptr->in].disponibilidade = operacao->disponibilidade;
    BAgendamento.buffer[BAgendamento.ptr->in].intermediario = operacao->intermediario;
    BAgendamento.buffer[BAgendamento.ptr->in].empresa = operacao->empresa;
    BAgendamento.buffer[BAgendamento.ptr->in].time_descricao = operacao->time_descricao;
    BAgendamento.buffer[BAgendamento.ptr->in].time_pedido = operacao->time_pedido;

    BAgendamento.ptr->in = (BAgendamento.ptr->in + 1) % Config.BUFFER_AGENDAMENTO;

    pos = BAgendamento.ptr->in;

    //==============================================

    prodcons_response_s_produce_end();

    // informar cliente de que a agendamento esta pronta
    control_empresa_submete_agendamento(operacao->cliente);

    // registar hora pronta (agendamento)
    time_register(&BAgendamento.buffer[pos].time_agendamento);

    // log
    file_write_log_file(5, id);
}
//******************************************
// memory_response_s_read
//
void memory_response_s_read(int id, struct operation *operacao)
{
    // aguardar agendamento
    control_cliente_esperapor_agendamento(operacao->cliente);

    prodcons_response_s_consume_begin();

    //==============================================
    // LER AGENDAMENTO DO BUFFER DE AGENDAMENTO
    //
    // copiar conteudo relevante da posicao BAgendamento.ptr->out
    // do buffer BAgendamento para a estrutura operacao
    // conteudo: cliente, disponibilidade, intermediario, empresa, time_descricao, time_pedido, time_agendamento
    // e atualizar BAgendamento.ptr->out

    //so_memory_response_s_read(id, operacao);

    operacao->cliente = BAgendamento.buffer[BAgendamento.ptr->out].cliente;
    operacao->disponibilidade = BAgendamento.buffer[BAgendamento.ptr->out].disponibilidade;
    operacao->intermediario = BAgendamento.buffer[BAgendamento.ptr->out].intermediario;
    operacao->empresa = BAgendamento.buffer[BAgendamento.ptr->out].empresa;
    operacao->time_descricao = BAgendamento.buffer[BAgendamento.ptr->out].time_descricao;
    operacao->time_pedido = BAgendamento.buffer[BAgendamento.ptr->out].time_pedido;
    operacao->time_agendamento = BAgendamento.buffer[BAgendamento.ptr->out].time_agendamento;

    BAgendamento.ptr->out = (BAgendamento.ptr->out + 1) % Config.BUFFER_AGENDAMENTO;

    //==============================================

    prodcons_response_s_consume_end();

    // log
    file_write_log_file(6, id);
}

//******************************************
// MEMORIA_CRIAR_INDICADORES
//
void memory_create_statistics()
{
    //==============================================
    // CRIAR ZONAS DE MEMÓRIA PARA OS INDICADORES
    //
    // criação dinâmica de memória
    // para cada campo da estrutura indicadores

    //so_memory_create_statistics();

    Ind.capacidade_inicial_portuaria = malloc(sizeof(int) * Config.OPERATIONS);
    Ind.pid_clientes = malloc(sizeof(int) * Config.CLIENTES);
    Ind.pid_intermediarios = malloc(sizeof(int) * Config.INTERMEDIARIO);
    Ind.pid_empresas = malloc(sizeof(int) * Config.EMPRESAS);
    Ind.clientes_servidos_por_intermediarios = malloc(sizeof(int) * Config.INTERMEDIARIO);
    Ind.clientes_servidos_por_empresas = malloc(sizeof(int) * Config.EMPRESAS);

    Ind.agendamentos_entregues_por_empresas = memory_create(STR_SHM_OPREMPRESAS, sizeof(int) * Config.OPERATIONS * Config.EMPRESAS);

    Ind.servicos_recebidos_por_clientes = malloc(sizeof(int) * Config.OPERATIONS);

    // iniciar indicadores relevantes:
    // - Ind.capacidade_inicial_portuaria (c/ Config.capacidade_portuaria respetivo)
    // - Ind.clientes_servidos_por_intermediarios (c/ 0)
    // - Ind.clientes_servidos_por_empresas (c/ 0)
    // - Ind.serviços_recebidos_por_clientes (c/ 0)

    //so_memory_prepare_statistics();

    for (int i = 0; i < Config.OPERATIONS; i++)
    {
        Ind.capacidade_inicial_portuaria[i] = Config.capacidade_portuaria[i];
        Ind.servicos_recebidos_por_clientes[i] = 0;
    }

    for (int i = 0; i < Config.INTERMEDIARIO; i++)
    {
        Ind.clientes_servidos_por_intermediarios[i] = 0;
    }

    for (int i = 0; i < Config.EMPRESAS; i++)
    {
        Ind.clientes_servidos_por_empresas[i] = 0;
    }

    //==============================================
}

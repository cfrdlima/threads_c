#ifndef CYBER_CAFE_H
#define CYBER_CAFE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

// Constantes
#define NUM_PCS 10
#define NUM_VR_HEADSETS 6
#define NUM_CADEIRAS 8
#define TEMPO_SIMULACAO 480 // 8 horas em minutos
#define MAX_CLIENTES 100

// Tipos de cliente
typedef enum
{
    GAMER,      // Precisa de PC + VR, depois cadeira
    FREELANCER, // Precisa de PC + Cadeira, depois VR
    ESTUDANTE   // Precisa apenas do PC
} TipoCliente;

// Estado do cliente
typedef enum
{
    ESPERANDO,
    USANDO_RECURSOS,
    FINALIZADO
} EstadoCliente;

// TAD Cliente
typedef struct
{
    int id;
    TipoCliente tipo;
    EstadoCliente estado;
    time_t hora_chegada;
    time_t hora_inicio;
    time_t hora_fim;
    int tempo_espera;
    bool tem_pc;
    bool tem_vr;
    bool tem_cadeira;
    pthread_t thread;
} Cliente;

// TAD Gerenciador de Recursos
typedef struct
{
    // Contadores de recursos
    int pcs_disponiveis;
    int vr_disponiveis;
    int cadeiras_disponiveis;

    // Semáforos para gerenciamento de recursos
    sem_t sem_pc;
    sem_t sem_vr;
    sem_t sem_cadeira;

    // Mutex para atualizar contadores
    pthread_mutex_t mutex_recursos;

    // Variáveis para evitar deadlock
    pthread_mutex_t mutex_alocacao;
    bool esta_seguro;

    // Estatísticas
    int total_clientes;
    int clientes_atendidos;
    int clientes_nao_atendidos;
    int contador_uso_pc;
    int contador_uso_vr;
    int contador_uso_cadeira;
    long tempo_espera_total;
} GerenciadorRecursos;

// Variáveis globais
extern GerenciadorRecursos gerenciador_recursos;
extern Cliente clientes[MAX_CLIENTES];
extern pthread_mutex_t mutex_estatisticas;
extern bool simulacao_rodando;

// Protótipos de funções

// Funções do Gerenciador de Recursos
void inicializar_gerenciador_recursos();
bool solicitar_recursos(Cliente *cliente);
void liberar_recursos(Cliente *cliente);
bool verificar_seguranca(TipoCliente tipo);
void destruir_gerenciador_recursos();

// Funções do Cliente
void inicializar_cliente(Cliente *cliente, int id, TipoCliente tipo);
void *rotina_cliente(void *arg);
int gerar_duracao_aleatoria(TipoCliente tipo);

// Funções de Simulação
void iniciar_simulacao(int num_clientes);
void gerar_clientes_aleatorios(int num_clientes);
void imprimir_estatisticas();
void criar_cenario_deadlock();

#endif // CYBER_CAFE_H
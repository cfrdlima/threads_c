#include "cyber_cafe.h"

Cliente clientes[MAX_CLIENTES];
pthread_mutex_t mutex_estatisticas = PTHREAD_MUTEX_INITIALIZER;

/**
 * Inicializa um objeto Cliente com os valores iniciais.
 *
 * @param cliente Ponteiro para o objeto Cliente a ser inicializado.
 * @param id Identificador único do cliente.
 * @param tipo Tipo de cliente (GAMER, FREELANCER, ESTUDANTE).
 *
 * A função define o estado inicial do cliente como ESPERANDO,
 * registra a hora de chegada atual, e inicializa as propriedades
 * relacionadas a recursos como falsas (não alocados).
 */
void inicializar_cliente(Cliente *cliente, int id, TipoCliente tipo)
{
    cliente->id = id;
    cliente->tipo = tipo;
    cliente->estado = ESPERANDO;
    cliente->hora_chegada = time(NULL);
    cliente->hora_inicio = 0;
    cliente->hora_fim = 0;
    cliente->tempo_espera = 0;
    cliente->tem_pc = false;
    cliente->tem_vr = false;
    cliente->tem_cadeira = false;
}

// Obter tipo de cliente como string
const char *obter_tipo_cliente_str(TipoCliente tipo)
{
    switch (tipo)
    {
    case GAMER:
        return "Gamer";
    case FREELANCER:
        return "Freelancer";
    case ESTUDANTE:
        return "Estudante";
    default:
        return "Desconhecido";
    }
}

// Gerar tempo de uso aleatório baseado no tipo de cliente
int gerar_duracao_aleatoria(TipoCliente tipo)
{
    int tempo_base;

    switch (tipo)
    {
    case GAMER:
        tempo_base = 30 + rand() % 90; // 30-120 minutos
        break;
    case FREELANCER:
        tempo_base = 60 + rand() % 120; // 60-180 minutos
        break;
    case ESTUDANTE:
        tempo_base = 45 + rand() % 75; // 45-120 minutos
        break;
    default:
        tempo_base = 60;
        break;
    }

    return tempo_base;
}

/**
 * Função executada pela thread de cada cliente.
 *
 * A função tenta adquirir recursos com backoff exponencial para evitar starvation,
 * e se bem sucedida, simula o uso dos recursos por um tempo aleatório, e
 * posteriormente libera os recursos.
 *
 * Caso o cliente não consiga adquirir recursos após o número máximo de tentativas,
 * ele é considerado não atendido.
 *
 * @param arg Ponteiro para o objeto Cliente a ser executado.
 *
 * @return NULL, pois a função é uma rotina de thread.
 */
void *rotina_cliente(void *arg)
{
    Cliente *cliente = (Cliente *)arg;
    int max_tentativas = 5;
    int contagem_tentativas = 0;
    int tempo_espera = 2;

    printf("Cliente %d (%s) chegou ao cyber café.\n",
           cliente->id, obter_tipo_cliente_str(cliente->tipo));

    // Tenta adquirir recursos com backoff exponencial para evitar starvation
    while (cliente->estado == ESPERANDO && contagem_tentativas < max_tentativas && simulacao_rodando)
    {
        if (solicitar_recursos(cliente))
        {
            // Recursos adquiridos com sucesso
            cliente->estado = USANDO_RECURSOS;
            cliente->hora_inicio = time(NULL);
            cliente->tempo_espera = (int)difftime(cliente->hora_inicio, cliente->hora_chegada);

            pthread_mutex_lock(&mutex_estatisticas);
            gerenciador_recursos.clientes_atendidos++;
            gerenciador_recursos.tempo_espera_total += cliente->tempo_espera;
            pthread_mutex_unlock(&mutex_estatisticas);

            printf("Cliente %d (%s) adquiriu recursos após esperar %d segundos.\n",
                   cliente->id, obter_tipo_cliente_str(cliente->tipo), cliente->tempo_espera);

            // Imprime recursos adquiridos
            printf("Recursos para Cliente %d: PC: %s, VR: %s, Cadeira: %s\n",
                   cliente->id,
                   cliente->tem_pc ? "Sim" : "Não",
                   cliente->tem_vr ? "Sim" : "Não",
                   cliente->tem_cadeira ? "Sim" : "Não");

            // Usa recursos por um tempo aleatório
            int tempo_uso = gerar_duracao_aleatoria(cliente->tipo);
            printf("Cliente %d usará recursos por %d minutos.\n", cliente->id, tempo_uso);

            // Simula tempo usando sleep (escala reduzida para simulação)
            sleep(tempo_uso / 10);

            // Libera recursos
            liberar_recursos(cliente);
            cliente->estado = FINALIZADO;
            cliente->hora_fim = time(NULL);

            printf("Cliente %d terminou e liberou todos os recursos.\n", cliente->id);
            break;
        }
        else
        {
            // Não conseguiu adquirir recursos, espera e tenta novamente com backoff exponencial
            printf("Cliente %d esperando por recursos, tentativa %d.\n", cliente->id, contagem_tentativas + 1);
            sleep(tempo_espera);
            contagem_tentativas++;
            tempo_espera *= 2; // Backoff exponencial
        }
    }

    // Se o cliente não conseguiu ser atendido após o número máximo de tentativas
    if (cliente->estado == ESPERANDO)
    {
        pthread_mutex_lock(&mutex_estatisticas);
        gerenciador_recursos.clientes_nao_atendidos++;
        pthread_mutex_unlock(&mutex_estatisticas);

        printf("Cliente %d não pôde ser atendido após %d tentativas.\n", cliente->id, max_tentativas);
    }

    return NULL;
}
#include "cyber_cafe.h"

GerenciadorRecursos gerenciador_recursos;

void inicializar_gerenciador_recursos()
{
    // Inicializar contagens de recursos
    gerenciador_recursos.pcs_disponiveis = NUM_PCS;
    gerenciador_recursos.vr_disponiveis = NUM_VR_HEADSETS;
    gerenciador_recursos.cadeiras_disponiveis = NUM_CADEIRAS;

    // Inicializar semáforos
    sem_init(&gerenciador_recursos.sem_pc, 0, NUM_PCS);
    sem_init(&gerenciador_recursos.sem_vr, 0, NUM_VR_HEADSETS);
    sem_init(&gerenciador_recursos.sem_cadeira, 0, NUM_CADEIRAS);

    // Inicializar mutexes
    pthread_mutex_init(&gerenciador_recursos.mutex_recursos, NULL);
    pthread_mutex_init(&gerenciador_recursos.mutex_alocacao, NULL);

    // Inicializar estatísticas
    gerenciador_recursos.total_clientes = 0;
    gerenciador_recursos.clientes_atendidos = 0;
    gerenciador_recursos.clientes_nao_atendidos = 0;
    gerenciador_recursos.contador_uso_pc = 0;
    gerenciador_recursos.contador_uso_vr = 0;
    gerenciador_recursos.contador_uso_cadeira = 0;
    gerenciador_recursos.tempo_espera_total = 0;

    gerenciador_recursos.esta_seguro = true;
}

// Variante do algoritmo do banqueiro para evitar deadlock
bool verificar_seguranca(TipoCliente tipo)
{
    int pc_disponivel = gerenciador_recursos.pcs_disponiveis;
    int vr_disponivel = gerenciador_recursos.vr_disponiveis;
    int cadeira_disponivel = gerenciador_recursos.cadeiras_disponiveis;

    // Verificação de segurança simplificada baseada no tipo de cliente e necessidades de recursos
    switch (tipo)
    {
    case GAMER:
        // Gamers precisam de PC + VR (primário) e cadeira (secundário)
        return (pc_disponivel >= 1 && vr_disponivel >= 1);

    case FREELANCER:
        // Freelancers precisam de PC + Cadeira (primário) e VR (secundário)
        return (pc_disponivel >= 1 && cadeira_disponivel >= 1);

    case ESTUDANTE:
        // Estudantes precisam apenas de PC
        return (pc_disponivel >= 1);

    default:
        return false;
    }
}

bool solicitar_recursos(Cliente *cliente)
{
    pthread_mutex_lock(&gerenciador_recursos.mutex_alocacao);

    // Primeiro verifica se a alocação seria segura (evitar deadlock)
    if (!verificar_seguranca(cliente->tipo))
    {
        pthread_mutex_unlock(&gerenciador_recursos.mutex_alocacao);
        return false;
    }

    // Tenta alocar recursos baseado no tipo de cliente
    bool sucesso = false;

    switch (cliente->tipo)
    {
    case GAMER:
        // Gamers precisam de PC + VR + Cadeira (se disponível)
        if (sem_trywait(&gerenciador_recursos.sem_pc) == 0)
        {
            cliente->tem_pc = true;

            if (sem_trywait(&gerenciador_recursos.sem_vr) == 0)
            {
                cliente->tem_vr = true;

                // Tenta obter cadeira mas não bloqueia se indisponível
                if (sem_trywait(&gerenciador_recursos.sem_cadeira) == 0)
                {
                    cliente->tem_cadeira = true;
                }

                sucesso = true;
            }
            else
            {
                // Libera PC se não conseguir obter VR
                sem_post(&gerenciador_recursos.sem_pc);
                cliente->tem_pc = false;
            }
        }
        break;

    case FREELANCER:
        // Freelancers precisam de PC + Cadeira + VR (se disponível)
        if (sem_trywait(&gerenciador_recursos.sem_pc) == 0)
        {
            cliente->tem_pc = true;

            if (sem_trywait(&gerenciador_recursos.sem_cadeira) == 0)
            {
                cliente->tem_cadeira = true;

                // Tenta obter VR mas não bloqueia se indisponível
                if (sem_trywait(&gerenciador_recursos.sem_vr) == 0)
                {
                    cliente->tem_vr = true;
                }

                sucesso = true;
            }
            else
            {
                // Libera PC se não conseguir obter cadeira
                sem_post(&gerenciador_recursos.sem_pc);
                cliente->tem_pc = false;
            }
        }
        break;

    case ESTUDANTE:
        // Estudantes precisam apenas de PC
        if (sem_trywait(&gerenciador_recursos.sem_pc) == 0)
        {
            cliente->tem_pc = true;
            sucesso = true;
        }
        break;
    }

    // Atualiza contadores de recursos se a alocação for bem-sucedida
    if (sucesso)
    {
        pthread_mutex_lock(&gerenciador_recursos.mutex_recursos);

        if (cliente->tem_pc)
        {
            gerenciador_recursos.pcs_disponiveis--;
            gerenciador_recursos.contador_uso_pc++;
        }

        if (cliente->tem_vr)
        {
            gerenciador_recursos.vr_disponiveis--;
            gerenciador_recursos.contador_uso_vr++;
        }

        if (cliente->tem_cadeira)
        {
            gerenciador_recursos.cadeiras_disponiveis--;
            gerenciador_recursos.contador_uso_cadeira++;
        }

        pthread_mutex_unlock(&gerenciador_recursos.mutex_recursos);
    }

    pthread_mutex_unlock(&gerenciador_recursos.mutex_alocacao);
    return sucesso;
}

void liberar_recursos(Cliente *cliente)
{
    pthread_mutex_lock(&gerenciador_recursos.mutex_alocacao);

    pthread_mutex_lock(&gerenciador_recursos.mutex_recursos);

    // Libera PC se alocado
    if (cliente->tem_pc)
    {
        sem_post(&gerenciador_recursos.sem_pc);
        gerenciador_recursos.pcs_disponiveis++;
        cliente->tem_pc = false;
    }

    // Libera VR se alocado
    if (cliente->tem_vr)
    {
        sem_post(&gerenciador_recursos.sem_vr);
        gerenciador_recursos.vr_disponiveis++;
        cliente->tem_vr = false;
    }

    // Libera cadeira se alocada
    if (cliente->tem_cadeira)
    {
        sem_post(&gerenciador_recursos.sem_cadeira);
        gerenciador_recursos.cadeiras_disponiveis++;
        cliente->tem_cadeira = false;
    }

    pthread_mutex_unlock(&gerenciador_recursos.mutex_recursos);
    pthread_mutex_unlock(&gerenciador_recursos.mutex_alocacao);
}

void destruir_gerenciador_recursos()
{
    // Destruir semáforos
    sem_destroy(&gerenciador_recursos.sem_pc);
    sem_destroy(&gerenciador_recursos.sem_vr);
    sem_destroy(&gerenciador_recursos.sem_cadeira);

    // Destruir mutexes
    pthread_mutex_destroy(&gerenciador_recursos.mutex_recursos);
    pthread_mutex_destroy(&gerenciador_recursos.mutex_alocacao);
}
#include "cyber_cafe.h"

int main(int argc, char *argv[])
{
    // Inicializa semente aleatória
    srand(time(NULL));

    // Inicializa gerenciador de recursos
    inicializar_gerenciador_recursos();

    // Número de clientes para simular
    int num_clientes = rand() % 100; // Padrão

    // Permite argumento de linha de comando para especificar número de clientes
    if (argc > 1)
    {
        num_clientes = atoi(argv[1]);
        if (num_clientes <= 1 || num_clientes > MAX_CLIENTES)
        {
            printf("Número inválido de clientes. Usando padrão (30).\n");
            num_clientes = 30;
        }
    }

    // Primeiro demonstra um cenário potencial de deadlock e nosso mecanismo de prevenção
    criar_cenario_deadlock();

    // Inicia a simulação real
    iniciar_simulacao(num_clientes);

    // Imprime estatísticas finais
    imprimir_estatisticas();

    // Limpa
    destruir_gerenciador_recursos();
    pthread_mutex_destroy(&mutex_estatisticas);

    return 0;
}
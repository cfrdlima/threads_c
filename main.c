#include "cyber_cafe.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    pthread_t threads[NUM_CLIENTES];
    srand(time(NULL));
    iniciar_cyber_cafe();
    
    for (int i = 0; i < NUM_CLIENTES; i++) {
        Cliente *c = malloc(sizeof(Cliente));
        c->id = i + 1;
        c->tipo = rand() % 3;
        pthread_create(&threads[i], NULL, cliente_func, c);
        sleep(rand() % 3);
    }
    
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(threads[i], NULL);
    }
    
    finalizar_cyber_cafe();
    printf("\nSimulação do Cyber Café encerrada!\n");
    return 0;
}
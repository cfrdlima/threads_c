#ifndef CYBER_CAFE_H
#define CYBER_CAFE_H

#include <pthread.h>
#include <semaphore.h>

#define NUM_PCS 10
#define NUM_VR 6
#define NUM_CADEIRAS 8
#define NUM_CLIENTES 20

typedef enum { GAMER, FREELANCER, ESTUDANTE } TipoCliente;

typedef struct {
    int id;
    TipoCliente tipo;
} Cliente;

void iniciar_cyber_cafe();
void finalizar_cyber_cafe();
void *cliente_func(void *arg);

#endif // CYBER_CAFE_H
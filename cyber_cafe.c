#include "cyber_cafe.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    sem_t pcs;
    sem_t vr;
    sem_t cadeiras;
} Recursos;

Recursos recursos;

void iniciar_cyber_cafe()
{
    sem_init(&recursos.pcs, 0, NUM_PCS);
    sem_init(&recursos.vr, 0, NUM_VR_HEADSETS);
    sem_init(&recursos.cadeiras, 0, NUM_CADEIRAS);
}

void finalizar_cyber_cafe()
{
    sem_destroy(&recursos.pcs);
    sem_destroy(&recursos.vr);
    sem_destroy(&recursos.cadeiras);
}

void *cliente_func(void *arg)
{
    Cliente *c = (Cliente *)arg;
    printf("Cliente %d (%s) chegou ao Cyber Café.\n", c->id,
           c->tipo == GAMER ? "Gamer" : c->tipo == FREELANCER ? "Freelancer"
                                                              : "Estudante");

    if (c->tipo == GAMER)
    {
        sem_wait(&recursos.pcs);
        sem_wait(&recursos.vr);
        sem_wait(&recursos.cadeiras);
    }
    else if (c->tipo == FREELANCER)
    {
        sem_wait(&recursos.pcs);
        sem_wait(&recursos.cadeiras);
    }
    else
    {
        sem_wait(&recursos.pcs);
    }

    printf("Cliente %d (%s) está utilizando os recursos.\n", c->id,
           c->tipo == GAMER ? "Gamer" : c->tipo == FREELANCER ? "Freelancer"
                                                              : "Estudante");

    sleep(rand() % 5 + 1);

    if (c->tipo == GAMER)
    {
        sem_post(&recursos.pcs);
        sem_post(&recursos.vr);
        sem_post(&recursos.cadeiras);
    }
    else if (c->tipo == FREELANCER)
    {
        sem_post(&recursos.pcs);
        sem_post(&recursos.cadeiras);
    }
    else
    {
        sem_post(&recursos.pcs);
    }

    printf("Cliente %d (%s) saiu do Cyber Café.\n", c->id,
           c->tipo == GAMER ? "Gamer" : c->tipo == FREELANCER ? "Freelancer"
                                                              : "Estudante");

    free(c);
    pthread_exit(NULL);
}
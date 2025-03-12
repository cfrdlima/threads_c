#include "cyber_cafe.h"

bool simulacao_rodando = true;

void gerar_clientes_aleatorios(int num_clientes) {
    for (int i = 1; i < num_clientes; i++) {
        // Determina tipo de cliente aleatoriamente
        TipoCliente tipo = rand() % 3; // 0=GAMER, 1=FREELANCER, 2=ESTUDANTE
        
        inicializar_cliente(&clientes[i], i, tipo);
        gerenciador_recursos.total_clientes++;
        
        // Cria thread para cliente com atraso aleatório para simular tempos de chegada
        usleep((rand() % 3000) * 1000); // 0-3 segundos de atraso
        pthread_create(&clientes[i].thread, NULL, rotina_cliente, &clientes[i]);
    }
}

void imprimir_estatisticas() {
    printf("\n==== ESTATÍSTICAS DA SIMULAÇÃO DO CYBER CAFÉ ====\n");
    printf("Total de clientes: %d\n", gerenciador_recursos.total_clientes);
    printf("Clientes atendidos: %d\n", gerenciador_recursos.clientes_atendidos);
    
    // Calcula clientes não atendidos
    gerenciador_recursos.clientes_nao_atendidos = gerenciador_recursos.total_clientes - gerenciador_recursos.clientes_atendidos;
    printf("Clientes não atendidos: %d\n", gerenciador_recursos.clientes_nao_atendidos);
    
    // Utilização de recursos
    printf("\nUtilização de Recursos:\n");
    printf("PCs utilizados: %d vezes\n", gerenciador_recursos.contador_uso_pc);
    printf("Headsets VR utilizados: %d vezes\n", gerenciador_recursos.contador_uso_vr);
    printf("Cadeiras utilizadas: %d vezes\n", gerenciador_recursos.contador_uso_cadeira);
    
    // Taxa de utilização de recursos
    printf("\nTaxa de Utilização de Recursos:\n");
    printf("Utilização de PC: %.2f%%\n", 
           (double)gerenciador_recursos.contador_uso_pc / NUM_PCS / (TEMPO_SIMULACAO / 60) * 100);
    printf("Utilização de VR: %.2f%%\n", 
           (double)gerenciador_recursos.contador_uso_vr / NUM_VR_HEADSETS / (TEMPO_SIMULACAO / 60) * 100);
    printf("Utilização de Cadeira: %.2f%%\n", 
           (double)gerenciador_recursos.contador_uso_cadeira / NUM_CADEIRAS / (TEMPO_SIMULACAO / 60) * 100);
}

// Cria um cenário de deadlock para demonstração
void criar_cenario_deadlock() {
    printf("\n==== CRIANDO CENÁRIO DE DEADLOCK PARA DEMONSTRAÇÃO ====\n");
    
    // Força quase todos os recursos a serem alocados
    gerenciador_recursos.pcs_disponiveis = 1;
    gerenciador_recursos.vr_disponiveis = 1;
    gerenciador_recursos.cadeiras_disponiveis = 1;
    
    // Ajusta semáforos para corresponder ao nosso estado forçado
    for (int i = 0; i < NUM_PCS - 1; i++) {
        sem_wait(&gerenciador_recursos.sem_pc);
    }
    
    for (int i = 0; i < NUM_VR_HEADSETS - 1; i++) {
        sem_wait(&gerenciador_recursos.sem_vr);
    }
    
    for (int i = 0; i < NUM_CADEIRAS - 1; i++) {
        sem_wait(&gerenciador_recursos.sem_cadeira);
    }
    
    printf("Recursos disponíveis: PCs: 1, VR: 1, Cadeiras: 1\n");
    
    // Cria clientes com necessidades de recursos conflitantes
    Cliente gamer, freelancer;
    inicializar_cliente(&gamer, 998, GAMER);
    inicializar_cliente(&freelancer, 999, FREELANCER);
    
    printf("Criando potencial deadlock com:\n");
    printf("- Gamer (precisa de PC + VR, depois Cadeira)\n");
    printf("- Freelancer (precisa de PC + Cadeira, depois VR)\n");
    
    printf("\nSem nossa prevenção de deadlock, isso aconteceria:\n");
    printf("1. Gamer adquire PC\n");
    printf("2. Freelancer adquire Cadeira\n");
    printf("3. Gamer espera por VR\n");
    printf("4. Freelancer espera por PC\n");
    printf("5. Nenhum pode prosseguir -> DEADLOCK!\n");
    
    printf("\nCom nossa verificação de segurança, um cliente será negado recursos até que o outro complete,\n");
    printf("prevenindo a situação de deadlock.\n");
    
    // Agora demonstra que nossa solução previne deadlock
    printf("\nDemonstrando nosso mecanismo de prevenção:\n");
    
    // Tenta alocar para o gamer primeiro
    printf("Gamer tentando adquirir recursos...\n");
    bool sucesso_gamer = solicitar_recursos(&gamer);
    
    if (sucesso_gamer) {
        printf("Gamer adquiriu recursos com sucesso\n");
        
        // Mostra recursos adquiridos
        printf("Recursos para o Gamer: PC: %s, VR: %s, Cadeira: %s\n",
              gamer.tem_pc ? "Sim" : "Não",
              gamer.tem_vr ? "Sim" : "Não",
              gamer.tem_cadeira ? "Sim" : "Não");
              
        // Tenta para o freelancer
        printf("\nFreelancer tentando adquirir recursos...\n");
        bool sucesso_freelancer = solicitar_recursos(&freelancer);
        
        if (sucesso_freelancer) {
            printf("Freelancer adquiriu recursos com sucesso\n");
            
            // Mostra recursos adquiridos
            printf("Recursos para o Freelancer: PC: %s, VR: %s, Cadeira: %s\n",
                  freelancer.tem_pc ? "Sim" : "Não",
                  freelancer.tem_vr ? "Sim" : "Não",
                  freelancer.tem_cadeira ? "Sim" : "Não");
                  
            // Libera recursos
            liberar_recursos(&freelancer);
        } else {
            printf("Freelancer NEGADO recursos devido à verificação de segurança - DEADLOCK PREVENIDO!\n");
        }
        
        // Libera recursos do gamer
        liberar_recursos(&gamer);
    } else {
        printf("Gamer NEGADO recursos devido à verificação de segurança\n");
        
        // Tenta para o freelancer
        printf("\nFreelancer tentando adquirir recursos...\n");
        bool sucesso_freelancer = solicitar_recursos(&freelancer);
        
        if (sucesso_freelancer) {
            printf("Freelancer adquiriu recursos com sucesso\n");
            
            // Mostra recursos adquiridos
            printf("Recursos para o Freelancer: PC: %s, VR: %s, Cadeira: %s\n",
                  freelancer.tem_pc ? "Sim" : "Não",
                  freelancer.tem_vr ? "Sim" : "Não",
                  freelancer.tem_cadeira ? "Sim" : "Não");
                  
            // Libera recursos
            liberar_recursos(&freelancer);
        } else {
            printf("Freelancer NEGADO recursos devido à verificação de segurança\n");
        }
    }
    
    printf("\n==== FIM DA DEMONSTRAÇÃO DE DEADLOCK ====\n");
    
    // Reseta estado dos recursos
    liberar_recursos(&gamer);
    liberar_recursos(&freelancer);
    
    // Restaura estado dos recursos
    gerenciador_recursos.pcs_disponiveis = NUM_PCS;
    gerenciador_recursos.vr_disponiveis = NUM_VR_HEADSETS;
    gerenciador_recursos.cadeiras_disponiveis = NUM_CADEIRAS;
    
    // Reseta semáforos
    sem_destroy(&gerenciador_recursos.sem_pc);
    sem_destroy(&gerenciador_recursos.sem_vr);
    sem_destroy(&gerenciador_recursos.sem_cadeira);
    
    sem_init(&gerenciador_recursos.sem_pc, 0, NUM_PCS);
    sem_init(&gerenciador_recursos.sem_vr, 0, NUM_VR_HEADSETS);
    sem_init(&gerenciador_recursos.sem_cadeira, 0, NUM_CADEIRAS);
}

void iniciar_simulacao(int num_clientes) {
    printf("Iniciando Simulação do Cyber Café com %d clientes\n", num_clientes);
    gerenciador_recursos.total_clientes = num_clientes;
    
    // Gera clientes aleatórios
    gerar_clientes_aleatorios(num_clientes);
    
    // Executa simulação pelo tempo especificado
    printf("Simulação rodando por %d minutos (escala reduzida)...\n", TEMPO_SIMULACAO);
    sleep(TEMPO_SIMULACAO / 10); // Escala reduzida para propósitos de simulação
    
    // Termina simulação
    simulacao_rodando = false;
    printf("Tempo de simulação encerrado\n");
    
    // Espera todas as threads de clientes terminarem
    for (int i = 1; i < num_clientes; i++) {
        pthread_join(clientes[i].thread, NULL);
    }
}
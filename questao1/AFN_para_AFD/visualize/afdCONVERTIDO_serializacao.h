#ifndef AFD_SERIALIZACAO_H
#define AFD_SERIALIZACAO_H

#include <stdbool.h>
#define MAX_ESTADOS 50
#define MAX_ALFABETO 10

// Declaração da função de serialização
void salvarAFD_JSON(
    int num_estados,
    int tam_alfabeto,
    char alfabeto[MAX_ALFABETO],
    int transicoes[MAX_ESTADOS][MAX_ALFABETO],
    bool estadosFinais[MAX_ESTADOS],
    int estadoInicial
);

#endif

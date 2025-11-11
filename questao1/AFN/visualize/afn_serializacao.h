#ifndef AFN_SERIALIZACAO_H
#define AFN_SERIALIZACAO_H
#include <stdbool.h>
#define MAX_ESTADOS 50
#define MAX_ALFABETO 10
#define LAMBDA_CHAR 'e'


void salvarAFN_JSON(
    int num_estados,
    int tam_alfabeto,
    char alfabeto[MAX_ALFABETO],
    //  A matriz de transição agora é 3D e booleana
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS],
    bool estadosFinais[MAX_ESTADOS],
    int estadoInicial
);

#endif
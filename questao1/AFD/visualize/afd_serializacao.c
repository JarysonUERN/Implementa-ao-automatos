#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "afd_serializacao.h"

void salvarAFD_JSON(
    int num_estados,
    int tam_alfabeto,
    char alfabeto[MAX_ALFABETO],
    int transicoes[MAX_ESTADOS][MAX_ALFABETO],
    bool estadosFinais[MAX_ESTADOS],
    int estadoInicial
) {
    cJSON *json = cJSON_CreateObject();

    // Estados
    cJSON *arr_estados = cJSON_CreateArray();
    for (int i = 0; i < num_estados; i++) {
        char nome[10];
        sprintf(nome, "q%d", i);
        cJSON_AddItemToArray(arr_estados, cJSON_CreateString(nome));
    }
    cJSON_AddItemToObject(json, "estados", arr_estados);

    // Alfabeto
    cJSON *arr_alfabeto = cJSON_CreateArray();
    for (int j = 0; j < tam_alfabeto; j++) {
        char simbolo[2] = { alfabeto[j], '\0' };
        cJSON_AddItemToArray(arr_alfabeto, cJSON_CreateString(simbolo));
    }
    cJSON_AddItemToObject(json, "alfabeto", arr_alfabeto);

    // Transições
    cJSON *arr_transicoes = cJSON_CreateArray();
    for (int i = 0; i < num_estados; i++) {
        for (int j = 0; j < tam_alfabeto; j++) {
            cJSON *t = cJSON_CreateObject();
            char origem[10], destino[10], simbolo[2];
            sprintf(origem, "q%d", i);
            sprintf(destino, "q%d", transicoes[i][j]);
            simbolo[0] = alfabeto[j]; simbolo[1] = '\0';
            cJSON_AddStringToObject(t, "de", origem);
            cJSON_AddStringToObject(t, "simbolo", simbolo);
            cJSON_AddStringToObject(t, "para", destino);
            cJSON_AddItemToArray(arr_transicoes, t);
        }
    }
    cJSON_AddItemToObject(json, "transicoes", arr_transicoes);

    // Estado inicial
    char inicial_nome[10];
    sprintf(inicial_nome, "q%d", estadoInicial);
    cJSON_AddStringToObject(json, "estado_inicial", inicial_nome);

    // Finais
    cJSON *arr_finais = cJSON_CreateArray();
    for (int i = 0; i < num_estados; i++) {
        if (estadosFinais[i]) {
            char nome[10];
            sprintf(nome, "q%d", i);
            cJSON_AddItemToArray(arr_finais, cJSON_CreateString(nome));
        }
    }
    cJSON_AddItemToObject(json, "finais", arr_finais);

    // Grava arquivo
    char *saida = cJSON_Print(json);
    FILE *f = fopen("afd.json", "w");
    fprintf(f, "%s", saida);
    fclose(f);
    printf("\n[Info] Arquivo 'afd.json' gerado com sucesso!\n");

    free(saida);
    cJSON_Delete(json);
}

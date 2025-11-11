#include <stdio.h>
#include <stdlib.h>
#include "../../cJSON-1.7.19/cJSON.h"
#include "../../cJSON-1.7.19/cJSON.c"
#include "afdCONVERTIDO_serializacao.h"

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

    // Grava arquivo em pasta 'output' (cria se necessário)
    char outdir[] = "..\\output";
    // Tenta criar diretório (Windows):
    #ifdef _WIN32
    _mkdir(outdir);
    #else
    mkdir(outdir, 0755);
    #endif

    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%s\\afd.json", outdir);

    char *saida = cJSON_Print(json);
    FILE *f = fopen(outpath, "w");
    if (f == NULL) {
        // Falha ao abrir: tentar caminho relativo simples
        fprintf(stderr, "[Erro] Nao foi possivel criar o arquivo '%s' (tentando cwd).\n", outpath);
        f = fopen("afd.json", "w");
        if (f == NULL) {
            fprintf(stderr, "[Erro] Nao foi possivel criar o arquivo 'afd.json' no cwd. Saida descartada.\n");
            free(saida);
            cJSON_Delete(json);
            return;
        } else {
            fprintf(stdout, "[Info] Gravando em arquivo 'afd.json' no cwd como fallback.\n");
        }
    } else {
        printf("\n[Info] Arquivo '%s' gerado com sucesso!\n", outpath);
    }

    fprintf(f, "%s", saida);
    fclose(f);
    free(saida);
    cJSON_Delete(json);
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include "../../cJSON-1.7.19/cJSON.h"
#include "../../cJSON-1.7.19/cJSON.c"
#include "afn_serializacao.h" 

// Caractere usado para representar lambda no JSON
#define LAMBDA_CHAR 'e' 

void salvarAFN_JSON(
    int num_estados,
    int tam_alfabeto,
    char alfabeto[MAX_ALFABETO],
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS],
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
        
        
        for (int j = 0; j <= tam_alfabeto; j++) {
            
           
            cJSON *arr_destinos = cJSON_CreateArray();
            int contador_destinos = 0;

            for (int k = 0; k < num_estados; k++) {
                if (transicoes[i][j][k] == true) {
                    char destino[10];
                    sprintf(destino, "q%d", k);
                    cJSON_AddItemToArray(arr_destinos, cJSON_CreateString(destino));
                    contador_destinos++;
                }
            }

            if (contador_destinos > 0) {
                cJSON *t = cJSON_CreateObject();
                char origem[10];
                char simbolo_str[2];

              
                sprintf(origem, "q%d", i);
                cJSON_AddStringToObject(t, "de", origem);

                if (j == tam_alfabeto) {
                    simbolo_str[0] = LAMBDA_CHAR; 
                } else {
                    simbolo_str[0] = alfabeto[j]; 
                }
                simbolo_str[1] = '\0';
                cJSON_AddStringToObject(t, "simbolo", simbolo_str);
                cJSON_AddItemToObject(t, "para", arr_destinos);
                cJSON_AddItemToArray(arr_transicoes, t);

            } else {
                // Se não houver destinos, descarte o array vazio
                cJSON_Delete(arr_destinos);
            }
        }
    }
    cJSON_AddItemToObject(json, "transicoes", arr_transicoes);

    //Estado(s) inicial
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

    //Grava arquivo
    char *saida = cJSON_Print(json);
    
    FILE *f = fopen("afn.json", "w"); 
    if (f == NULL) {
        printf("\n[Erro] Nao foi possivel criar o arquivo 'afn.json'.\n");
    } else {
        fprintf(f, "%s", saida);
        fclose(f);
        printf("\nArquivo 'afn.json' gerado com sucesso!\n");
    }

    free(saida);
    cJSON_Delete(json);
}
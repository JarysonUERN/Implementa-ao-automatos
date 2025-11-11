#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../AFN/visualize/afn_serializacao.h"
#include "../AFD/visualize/afd_serializacao.h"
#include "../cJSON-1.7.19/cJSON.h"
#include "../cJSON-1.7.19/cJSON.c"
// --- Estruturas de Dados para o AFN (lido do JSON) ---
typedef struct {
    int num_estados;
    int tam_alfabeto;
    char alfabeto[MAX_ALFABETO];
    int estado_inicial; // Índice
    bool finais[MAX_ESTADOS];
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS]; // +1 para lambda
    char nomes_estados[MAX_ESTADOS][10]; // Mapeia "q0" -> 0
    int lambda_idx; // Índice da transição lambda
    
} NFA;

// --- Estruturas de Dados para o AFD (resultado) ---
typedef struct {
    int num_estados;
    int tam_alfabeto;
    char alfabeto[MAX_ALFABETO];
    int estado_inicial; // Sempre 0
    bool finais[MAX_ESTADOS];
    int transicoes[MAX_ESTADOS][MAX_ALFABETO];
    
    // Mapeia os estados do AFD para conjuntos de estados do AFN
    bool dfa_states_map[MAX_ESTADOS][MAX_ESTADOS];
} DFA;


// --- Funções Auxiliares de Mapeamento ---

int mapear_estado_nome_para_idx(NFA* nfa, const char* nome) {
    for (int i = 0; i < nfa->num_estados; i++) {
        if (strcmp(nfa->nomes_estados[i], nome) == 0) {
            return i;
        }
    }
    int idx = -1;
    if (sscanf(nome, "q%d", &idx) == 1 && idx < nfa->num_estados) {
         return idx;
    }
    return -1; // Erro
}

int mapear_simbolo_para_idx(NFA* nfa, char simbolo) {
    if (simbolo == LAMBDA_CHAR) {
        return nfa->lambda_idx;
    }
    for (int i = 0; i < nfa->tam_alfabeto; i++) {
        if (nfa->alfabeto[i] == simbolo) {
            return i;
        }
    }
    return -1; // Erro
}

void imprimir_conjunto(bool* conjunto, int n, NFA* nfa) {
    printf("[");
    bool first = true;
    for (int i = 0; i < n; i++) {
        if (conjunto[i]) {
            if (!first) printf(", ");
            printf("%s", nfa->nomes_estados[i]);
            first = false;
        }
    }
    printf("]");
}

// --- Função para imprimir o AFD resultante (NOVA) ---
void imprimir_afd(DFA* dfa, NFA* nfa) {
    printf("\n--- Tabela de Transicoes AFD Resultante ---\n");
    
    // Imprime cabeçalho do alfabeto
    printf("Estado (AFD) | Mapeamento (AFN) \t| ");
    for (int i = 0; i < dfa->tam_alfabeto; i++) {
        printf("  '%c'\t| ", dfa->alfabeto[i]);
    }
    printf("\n");
    
    // Imprime linhas de borda
    printf("----------------------------------------");
    for (int i = 0; i < dfa->tam_alfabeto; i++) {
        printf("------");
    }
    printf("\n");

    // Imprime cada estado
    for (int i = 0; i < dfa->num_estados; i++) {
        // Nome do estado (q0, q1, ...)
        printf("q%-3d          | ", i);
        
        // Mapeamento (ex: [q0, q1])
        imprimir_conjunto(dfa->dfa_states_map[i], nfa->num_estados, nfa);
        
        // Calcula o espaço do conjunto para alinhar a coluna (Final)
        int conjunto_len = 0;
        bool has_states = false;
        for(int k=0; k<nfa->num_estados; k++) {
            if(dfa->dfa_states_map[i][k]) {
                conjunto_len += 3 + (k > 9 ? 1:0); // qX,
                has_states = true;
            }
        }
        if (has_states) conjunto_len -= 2; // remove ultimo ", "
        else conjunto_len = 2; // para "[]"
        
        // Imprime (Final) se for final, alinhado
        printf("\t%*s| ", (int)(10 - conjunto_len), (dfa->finais[i] ? "(Final)" : ""));

        // Transições
        for (int j = 0; j < dfa->tam_alfabeto; j++) {
            printf("  q%-3d\t| ", dfa->transicoes[i][j]);
        }
        printf("\n");
    }
}


// --- Funções de Carregamento do AFN (JSON -> Struct) ---

bool carregarAFN_JSON(const char* caminho, NFA* nfa) {
    // Ler o arquivo
    FILE *f = fopen(caminho, "rb");
    if (f == NULL) {
        printf("[Erro] Nao foi possivel abrir o arquivo %s\n", caminho);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char*)malloc(tamanho + 1);
    fread(buffer, 1, tamanho, f);
    fclose(f);
    buffer[tamanho] = '\0';

    // Parsear o JSON
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        printf("[Erro] Falha ao parsear JSON.\n");
        free(buffer);
        return false;
    }

    // Inicializa NFA
    memset(nfa, 0, sizeof(NFA));
    nfa->lambda_idx = -1; 

    //  Ler Estados
    cJSON *estados = cJSON_GetObjectItem(json, "estados");
    cJSON *estado_json;
    nfa->num_estados = 0;
    cJSON_ArrayForEach(estado_json, estados) {
        if (nfa->num_estados < MAX_ESTADOS) {
            strncpy(nfa->nomes_estados[nfa->num_estados], estado_json->valuestring, 9);
            nfa->nomes_estados[nfa->num_estados][9] = '\0';
            nfa->num_estados++;
        }
    }

    // Ler Alfabeto
    cJSON *alfabeto = cJSON_GetObjectItem(json, "alfabeto");
    cJSON *simbolo_json;
    nfa->tam_alfabeto = 0;
    cJSON_ArrayForEach(simbolo_json, alfabeto) {
        if (nfa->tam_alfabeto < MAX_ALFABETO) {
            nfa->alfabeto[nfa->tam_alfabeto] = simbolo_json->valuestring[0];
            nfa->tam_alfabeto++;
        }
    }
    nfa->lambda_idx = nfa->tam_alfabeto; // Índice lambda é o último

    //  Ler Estado Inicial
    char *inicial_str = cJSON_GetObjectItem(json, "estado_inicial")->valuestring;
    nfa->estado_inicial = mapear_estado_nome_para_idx(nfa, inicial_str);

    //  Ler Finais
    cJSON *finais = cJSON_GetObjectItem(json, "finais");
    cJSON *final_json;
    cJSON_ArrayForEach(final_json, finais) {
        int idx = mapear_estado_nome_para_idx(nfa, final_json->valuestring);
        if (idx != -1) {
            nfa->finais[idx] = true;
        }
    }

    // Ler Transições
    cJSON *transicoes = cJSON_GetObjectItem(json, "transicoes");
    cJSON *t_json;
    cJSON_ArrayForEach(t_json, transicoes) {
        char* de_str = cJSON_GetObjectItem(t_json, "de")->valuestring;
        char* simbolo_str = cJSON_GetObjectItem(t_json, "simbolo")->valuestring;
        cJSON* para_array = cJSON_GetObjectItem(t_json, "para");
        
        int de_idx = mapear_estado_nome_para_idx(nfa, de_str);
        int simbolo_idx = mapear_simbolo_para_idx(nfa, simbolo_str[0]);
        
        if (de_idx == -1 || simbolo_idx == -1) continue;

        cJSON *para_json;
        cJSON_ArrayForEach(para_json, para_array) {
            int para_idx = mapear_estado_nome_para_idx(nfa, para_json->valuestring);
            if (para_idx != -1) {
                nfa->transicoes[de_idx][simbolo_idx][para_idx] = true;
            }
        }
    }

    printf("[Info] AFN carregado de %s com sucesso!\n", caminho);
    printf("  - Estados: %d\n", nfa->num_estados);
    printf("  - Alfabeto: %d simbolos\n", nfa->tam_alfabeto);

    cJSON_Delete(json);
    free(buffer);
    return true;
}


// --- Funções Principais da Conversão ---

void e_closure_dfs(int estado, bool* closure_set, NFA* nfa) {
    closure_set[estado] = true;
    for (int i = 0; i < nfa->num_estados; i++) {
        if (nfa->transicoes[estado][nfa->lambda_idx][i] && !closure_set[i]) {
            e_closure_dfs(i, closure_set, nfa);
        }
    }
}

void calcular_e_closure_conjunto(bool* conjunto_in, bool* conjunto_out, NFA* nfa) {
    memset(conjunto_out, false, nfa->num_estados * sizeof(bool));
    for (int i = 0; i < nfa->num_estados; i++) {
        if (conjunto_in[i]) {
            e_closure_dfs(i, conjunto_out, nfa);
        }
    }
}


void calcular_move(bool* conjunto_in, int simbolo_idx, bool* conjunto_out, NFA* nfa) {
    memset(conjunto_out, false, nfa->num_estados * sizeof(bool));
    for (int i = 0; i < nfa->num_estados; i++) {
        if (conjunto_in[i]) {
            for (int j = 0; j < nfa->num_estados; j++) {
                if (nfa->transicoes[i][simbolo_idx][j]) {
                    conjunto_out[j] = true;
                }
            }
        }
    }
}

bool comparar_conjuntos(bool* set1, bool* set2, int n) {
    return memcmp(set1, set2, n * sizeof(bool)) == 0;
}

int encontrar_ou_adicionar_dfa_estado(bool* novo_conjunto, NFA* nfa, DFA* dfa, int* fila, int* fila_fim) {
    // Procurar
    for (int i = 0; i < dfa->num_estados; i++) {
        if (comparar_conjuntos(dfa->dfa_states_map[i], novo_conjunto, nfa->num_estados)) {
            return i; // Encontrado
        }
    }

    // Não encontrado, adicionar novo
    if (dfa->num_estados >= MAX_ESTADOS) {
        printf("Erro: Limite de estados do AFD atingido (%d)!\n", MAX_ESTADOS);
        return -1;
    }

    int novo_indice = dfa->num_estados;
    memcpy(dfa->dfa_states_map[novo_indice], novo_conjunto, nfa->num_estados * sizeof(bool));
    dfa->num_estados++;
    
    // Verificar se é final
    dfa->finais[novo_indice] = false;
    for(int k = 0; k < nfa->num_estados; k++) {
        if (novo_conjunto[k] && nfa->finais[k]) {
            dfa->finais[novo_indice] = true;
            break;
        }
    }
    
    // LOG e adicionar à fila
    printf("  -> Novo estado AFD (q%d) = ", novo_indice);
    imprimir_conjunto(novo_conjunto, nfa->num_estados, nfa);
    if(dfa->finais[novo_indice]) printf(" (FINAL)\n"); else printf("\n");
    
    fila[(*fila_fim)++] = novo_indice; // Enfileira
    return novo_indice;
}

// --- Função de Conversão Principal ---

void converter_afn_para_afd(NFA* nfa, DFA* dfa) {
    printf("\n--- Iniciando Conversao AFN -> AFD ---\n");

    // Inicializar AFD
    dfa->num_estados = 0;
    dfa->tam_alfabeto = nfa->tam_alfabeto;
    memcpy(dfa->alfabeto, nfa->alfabeto, nfa->tam_alfabeto);
    dfa->estado_inicial = 0; 

    // Fila de processamento
    int fila[MAX_ESTADOS];
    int fila_inicio = 0;
    int fila_fim = 0;

    // Buffers temporários
    bool temp_set1[MAX_ESTADOS];
    bool temp_set2[MAX_ESTADOS];

    // Criar Estado Inicial do AFD
    printf("Calculando e-closure do estado inicial (%s)...\n", nfa->nomes_estados[nfa->estado_inicial]);
    memset(temp_set1, false, sizeof(temp_set1));
    temp_set1[nfa->estado_inicial] = true; // Conjunto {q0_nfa}
    
    calcular_e_closure_conjunto(temp_set1, temp_set2, nfa); // temp_set2 agora é o e-fecho(q0)
    
    encontrar_ou_adicionar_dfa_estado(temp_set2, nfa, dfa, fila, &fila_fim);


    // Loop Principal (processar fila)
    while (fila_inicio < fila_fim) {
        int idx_T = fila[fila_inicio++]; 
        bool* T = dfa->dfa_states_map[idx_T]; 

        printf("\nProcessando estado q%d = ", idx_T);
        imprimir_conjunto(T, nfa->num_estados, nfa);
        printf("\n");

        // Para cada símbolo do alfabeto (sem lambda)
        for (int i = 0; i < nfa->tam_alfabeto; i++) {
            char simbolo = nfa->alfabeto[i];
            
            // Calcular move(T, simbolo) -> temp_set1
            calcular_move(T, i, temp_set1, nfa);
            printf("  - move(q%d, '%c') = ", idx_T, simbolo);
            imprimir_conjunto(temp_set1, nfa->num_estados, nfa);
            printf("\n");

            // Calcular e-closure(move(...)) -> temp_set2 (U)
            calcular_e_closure_conjunto(temp_set1, temp_set2, nfa);
            bool* U = temp_set2;
            printf("  - e-closure(move) = ");
            imprimir_conjunto(U, nfa->num_estados, nfa);
            printf("\n");

            // Achar ou adicionar U
            int idx_U = encontrar_ou_adicionar_dfa_estado(U, nfa, dfa, fila, &fila_fim);

            // Definir transição do AFD
            if (idx_U != -1) {
                dfa->transicoes[idx_T][i] = idx_U;
                printf("  => Transicao AFD: (q%d, '%c') -> q%d\n", idx_T, simbolo, idx_U);
            }
        }
    }
    printf("\n--- Conversao Concluida ---\n");
    printf("AFN original: %d estados.\n", nfa->num_estados);
    printf("AFD resultante: %d estados.\n", dfa->num_estados);
}


// --- Main ---

int main(int argc, char *argv[]) {
    char caminho_afn[512];
    if (argc == 2) {
        strncpy(caminho_afn, argv[1], sizeof(caminho_afn)-1);
        caminho_afn[sizeof(caminho_afn)-1] = '\0';
    } else {
        strncpy(caminho_afn, "..\\..\\AFN\\output\\afn.json", sizeof(caminho_afn)-1);
        caminho_afn[sizeof(caminho_afn)-1] = '\0';
        printf("[Info] Nenhum arquivo especificado. Tentando carregar: %s\n", caminho_afn);
    }

    NFA nfa;
    DFA dfa;

    // Carregar AFN
    if (!carregarAFN_JSON(caminho_afn, &nfa)) {
        printf ("Erro: Falha ao carregar o AFN (%s).\n", caminho_afn);
        return 1;
    }

    // Converter
    converter_afn_para_afd(&nfa, &dfa);
    
    // Imprimir tabela no terminal (NOVA CHAMADA)
    imprimir_afd(&dfa, &nfa);

    char outpath[512];
    snprintf(outpath, sizeof(outpath), "AFDconvertido.JSON");

    // Função local para salvar o AFD em JSON simples (sem depender de cJSON)
    FILE *f = fopen(outpath, "w");
    if (!f) {
        fprintf(stderr, "[Erro] Nao foi possivel criar o arquivo de saida: %s\n", outpath);
        return 1;
    }

    // Escreve JSON manualmente
    fprintf(f, "{\n");
    // estados
    fprintf(f, "  \"estados\": [");
    for (int i = 0; i < dfa.num_estados; i++) {
        fprintf(f, "\"q%d\"%s", i, (i+1<dfa.num_estados)?", ":"");
    }
    fprintf(f, "],\n");

    // alfabeto
    fprintf(f, "  \"alfabeto\": [");
    for (int j = 0; j < dfa.tam_alfabeto; j++) {
        fprintf(f, "\"%c\"%s", dfa.alfabeto[j], (j+1<dfa.tam_alfabeto)?", ":"");
    }
    fprintf(f, "],\n");

    // transicoes
    fprintf(f, "  \"transicoes\": [\n");
    for (int i = 0; i < dfa.num_estados; i++) {
        for (int j = 0; j < dfa.tam_alfabeto; j++) {
            fprintf(f, "    { \"de\": \"q%d\", \"simbolo\": \"%c\", \"para\": \"q%d\" }%s\n",
                    i, dfa.alfabeto[j], dfa.transicoes[i][j], (i==dfa.num_estados-1 && j==dfa.tam_alfabeto-1)?"":" ,");
        }
    }
    fprintf(f, "  ],\n");

    // estado inicial
    fprintf(f, "  \"estado_inicial\": \"q%d\",\n", dfa.estado_inicial);

    // finais
    fprintf(f, "  \"finais\": [");
    int countF = 0;
    for (int i = 0; i < dfa.num_estados; i++) {
        if (dfa.finais[i]) {
            if (countF) fprintf(f, ", ");
            fprintf(f, "\"q%d\"", i);
            countF++;
        }
    }
    fprintf(f, "]\n");
    fprintf(f, "}\n");
    fclose(f);
    printf("Info: AFD convertido gravado em: %s\n", outpath);

    return 0;
}
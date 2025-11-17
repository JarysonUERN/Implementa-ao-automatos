#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX_ESTADOS 50
#define MAX_ALFABETO 10
#define LAMBDA_CHAR 'e'

 #include "../AFN/visualize/afn_serializacao.h"
 #include "../AFD/visualize/afd_serializacao.h"    
#include "../cJSON-1.7.19/cJSON.h"
#include "../cJSON-1.7.19/cJSON.c"


#define MAX_DFA_STATE_NAME 256 
// --- Estruturas de Dados para o AFN---
typedef struct {
  int num_estados;
    int tam_alfabeto;
    char alfabeto[MAX_ALFABETO];
    int estado_inicial;
    bool finais[MAX_ESTADOS];
    char nomes_estados[MAX_ESTADOS][10]; 
    int lambda_idx; 
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS];
} NFA;

// --- Estruturas de Dados para o AFD ---
typedef struct {
    int num_estados;
    int tam_alfabeto;
    char alfabeto[MAX_ALFABETO];
    int estado_inicial; 
    bool finais[MAX_ESTADOS];
    // Tabela de transição do AFD (2D: de, simbolo) -> destino
    int transicoes[MAX_ESTADOS][MAX_ALFABETO];
    
    // Mapeia os estados do AFD para conjuntos de estados do AFN
    bool dfa_states_map[MAX_ESTADOS][MAX_ESTADOS];

    // MUDANÇA: Campo para armazenar nomes (ex: "q0q1")
    char dfa_state_names[MAX_ESTADOS][MAX_DFA_STATE_NAME];
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
    return -1; // perseando erro
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

// --- Função para imprimir o AFD resultante  ---
void imprimir_afd(DFA* dfa, NFA* nfa) {
    printf("\n--- Tabela de Transicoes AFD Resultante ---\n");
    
    // Imprime cabeçalho do alfabeto
    printf("Estado (AFD)      | Mapeamento (AFN) \t| "); 
    for (int i = 0; i < dfa->tam_alfabeto; i++) {
        printf("  '%c'\t| ", dfa->alfabeto[i]);
    }
    printf("\n");
    
    printf("----------------------------------------");
    for (int i = 0; i < dfa->tam_alfabeto; i++) {
        printf("-----------"); 
    }
    printf("\n");

    for (int i = 0; i < dfa->num_estados; i++) {
        printf("%-17s | ", dfa->dfa_state_names[i]); 
        
        // Mapeamento (ex: [q0, q1])
        imprimir_conjunto(dfa->dfa_states_map[i], nfa->num_estados, nfa);
        
        // Calcula o espaço do conjunto para alinhar a coluna (Final)
        int conjunto_len = 0;
        bool has_states = false;
        for(int k=0; k<nfa->num_estados; k++) {
            if(dfa->dfa_states_map[i][k]) {
                conjunto_len += strlen(nfa->nomes_estados[k]) + 2; 
                has_states = true;
            }
        }
        if (has_states) conjunto_len -= 2; 
        else conjunto_len = 2; 
        
        // Imprime (Final) se for final, alinhado
        printf("\t%*s| ", (int)(10 - conjunto_len), (dfa->finais[i] ? "(Final)" : ""));

        // Transições
        for (int j = 0; j < dfa->tam_alfabeto; j++) {
            // MUDANÇA: Usar o nome concatenado do destino
            int destino_idx = dfa->transicoes[i][j];
            printf("  %-8s\t| ", dfa->dfa_state_names[destino_idx]);
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
            nfa->nomes_estados[nfa->num_estados][9] = '\0'; // Garantir terminação
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
    // Define o índice da transição lambda (ex: 'e')
    nfa->lambda_idx = nfa->tam_alfabeto; 

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
        // Trata o símbolo 'e' (lambda)
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
    // Itera sobre todos os destinos possíveis para o símbolo lambda
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

// --- MODIFICADO ---
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
    
    // --- MUDANÇA: Construir o nome do estado ---
    char* nome_atual_ptr = dfa->dfa_state_names[novo_indice];
    nome_atual_ptr[0] = '\0'; // Limpar string
    int espaco_restante = MAX_DFA_STATE_NAME;
    bool nome_vazio = true;

    for (int k = 0; k < nfa->num_estados; k++) {
        if (novo_conjunto[k]) { // Se o estado 'k' (ex: q0) está no conjunto
            nome_vazio = false;
            const char* nome_nfa = nfa->nomes_estados[k]; // Pega o nome (ex: "q0")
            int tam_nome_nfa = strlen(nome_nfa);
            
            if (espaco_restante > tam_nome_nfa) {
                strcat(nome_atual_ptr, nome_nfa); // Concatena (ex: "q0q1")
                nome_atual_ptr += tam_nome_nfa;   // Avança o ponteiro
                espaco_restante -= tam_nome_nfa;
            } else {
                break; // Parar se o nome ficar muito grande
            }
        }
    }
    // Se o conjunto era vazio (estado morto), dá um nome a ele
    if (nome_vazio) {
        snprintf(dfa->dfa_state_names[novo_indice], MAX_DFA_STATE_NAME, "qVazio");
    }
    // --- FIM DA MUDANÇA DE NOME ---


    // Verificar se é final
    dfa->finais[novo_indice] = false;
    for(int k = 0; k < nfa->num_estados; k++) {
        if (novo_conjunto[k] && nfa->finais[k]) {
            dfa->finais[novo_indice] = true;
            break;
        }
    }
    
    // LOG e adicionar à fila
    // MUDANÇA: Usar o nome novo no log
    printf("  -> Novo estado AFD (idx %d) Nome: %s = ", novo_indice, dfa->dfa_state_names[novo_indice]);
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

        // MUDANÇA: Usar nome no log
        printf("\nProcessando estado %s (idx %d) = ", dfa->dfa_state_names[idx_T], idx_T);
        imprimir_conjunto(T, nfa->num_estados, nfa);
        printf("\n");

        // Para cada símbolo do alfabeto (sem lambda)
        for (int i = 0; i < nfa->tam_alfabeto; i++) {
            char simbolo = nfa->alfabeto[i];
            
            // Calcular move(T, simbolo) -> temp_set1
            // MUDANÇA: Usar nome no log
            printf("  - move(%s, '%c') = ", dfa->dfa_state_names[idx_T], simbolo);
            calcular_move(T, i, temp_set1, nfa);
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
                // MUDANÇA: Usar nome no log
                printf("  => Transicao AFD: (%s, '%c') -> %s\n", dfa->dfa_state_names[idx_T], simbolo, dfa->dfa_state_names[idx_U]);
            }
        }
    }
    printf("\n--- Conversao Concluida ---\n");
    printf("AFN original: %d estados.\n", nfa->num_estados);
    printf("AFD resultante: %d estados.\n", dfa->num_estados);
}


// --- Main (MODIFICADA) ---

int main(int argc, char *argv[]) {
    char caminho_afn[512];
    if (argc == 2) {
        strncpy(caminho_afn, argv[1], sizeof(caminho_afn)-1);
        caminho_afn[sizeof(caminho_afn)-1] = '\0';
    } else {
        // Caminho padrão (ajuste se necessário)
        // snprintf(caminho_afn, sizeof(caminho_afn), "..\\..\\AFN\\output\\afn.json");
        snprintf(caminho_afn, sizeof(caminho_afn), "afn.json"); // Procurar na pasta local
        printf("[Info] Nenhum arquivo especificado. Tentando carregar: %s\n", caminho_afn);
    }

    NFA nfa;
    DFA dfa;

    // Carregar AFN
    if (!carregarAFN_JSON(caminho_afn, &nfa)) {
        // Tentar outro caminho padrão
        snprintf(caminho_afn, sizeof(caminho_afn), "..\\..\\AFN\\output\\afn.json");
        if (!carregarAFN_JSON(caminho_afn, &nfa)) {
             printf ("Erro: Falha ao carregar o AFN (%s). Tentei caminhos padrao.\n", caminho_afn);
             return 1;
        }
    }

    // Converter
    converter_afn_para_afd(&nfa, &dfa);
    
    // Imprimir tabela no terminal
    imprimir_afd(&dfa, &nfa);

    // --- MUDANÇA: Salvar JSON com nomes concatenados ---
    char outpath[512];
    snprintf(outpath, sizeof(outpath), "AFDconvertido.JSON");

    FILE *f = fopen(outpath, "w");
    if (!f) {
        fprintf(stderr, "[Erro] Nao foi possivel criar o arquivo de saida: %s\n", outpath);
        return 1;
    }

    // Escreve JSON manualmente (MODIFICADO)
    fprintf(f, "{\n");
    // estados
    fprintf(f, "  \"estados\": [");
    for (int i = 0; i < dfa.num_estados; i++) {
        // USA O NOME CONCATENADO
        fprintf(f, "\"%s\"%s", dfa.dfa_state_names[i], (i+1<dfa.num_estados)?", ":"");
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
    int trans_count = 0;
    for (int i = 0; i < dfa.num_estados; i++) {
        for (int j = 0; j < dfa.tam_alfabeto; j++) {
            if (trans_count > 0) fprintf(f, ",\n"); // Adiciona vírgula antes
            
            int destino_idx = dfa.transicoes[i][j]; // Pega o índice do destino
            fprintf(f, "    { \"de\": \"%s\", \"simbolo\": \"%c\", \"para\": \"%s\" }",
                    dfa.dfa_state_names[i],         // Nome 'de'
                    dfa.alfabeto[j],                
                    dfa.dfa_state_names[destino_idx]); // Nome 'para'
            trans_count++;
        }
    }
    fprintf(f, "\n  ],\n");

    // estado inicial
    // (O estado inicial é o índice 0)
    fprintf(f, "  \"estado_inicial\": \"%s\",\n", dfa.dfa_state_names[dfa.estado_inicial]);

    // finais
    fprintf(f, "  \"finais\": [");
    int countF = 0;
    for (int i = 0; i < dfa.num_estados; i++) {
        if (dfa.finais[i]) {
            if (countF) fprintf(f, ", ");
            fprintf(f, "\"%s\"", dfa.dfa_state_names[i]); // Nome do estado final
            countF++;
        }
    }
    fprintf(f, "]\n");
    fprintf(f, "}\n");
    fclose(f);
    printf("\n[Info] Arquivo 'AFDconvertido.JSON' gerado com sucesso!\n");

    return 0;
}
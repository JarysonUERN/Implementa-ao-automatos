#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <locale.h>

// Definindo limites e o simbolo para representar lambda
#define MAX_ESTADOS 50
#define MAX_ALFABETO 10
#define MAX_ALFABETO_PILHA 10
#define LAMBDA 'e' 
#define MAX_CADEIAS 10000 
#define MAX_COMPRIMENTO_CADEIA 100 
#define MAX_TAMANHO_PILHA 500
#define SIMBOLO_INICIAL_PILHA 'Z' // Não pode ser mudado facilmente
#define MAX_PUSH_STRING 100

// Estrutura da Pilha
typedef struct {
    char S[MAX_TAMANHO_PILHA];
    int topo; 
} Pilha;
// Estrutura para destinos de transições do APN
typedef struct DestinoAPN {
    int proximo_estado;
    char para_empilhar[MAX_PUSH_STRING]; // Sequência de símbolos a empilhar
    struct DestinoAPN* proximo;
} DestinoAPN;

typedef struct {
    DestinoAPN* destinos[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ALFABETO_PILHA];
} TransicoesAPN;


typedef struct ConfiguracaoAPN {
    int estado_atual;
    Pilha pilha;
    struct ConfiguracaoAPN* proximo;
} ConfiguracaoAPN;

int leNumEstados()
{
    setlocale(LC_ALL, "pt_BR.UTF-8");
    int n;
    printf("Insira quantos estados tem seu APN (1 a %d): ", MAX_ESTADOS);
    scanf("%d", &n);
    getchar(); 
    
    if (n > MAX_ESTADOS || n <= 0)
    {
        printf("Numero invalido. Usando 3 estados por padrao.\n");
        return 3;
    }
    return n;
}

// Função para ler o alfabeto de entrada
int lerAlfabeto(char alfabeto[MAX_ALFABETO])
{
    char buffer_entrada[100];
    printf("\nInsira os simbolos do alfabeto de entrada (sem espacos, ex: 01, abc): ");
    scanf("%99s", buffer_entrada);
    getchar(); 

    int tam_buffer = strlen(buffer_entrada);

    if (tam_buffer == 0)
    {
        printf("Alfabeto invalido. Usando '01' por padrao.\n");
        strcpy(alfabeto, "01");
        return 2;
    }

    int indice_unico = 0; 
    for (int i = 0; i < tam_buffer; i++)
    {
        if (indice_unico >= MAX_ALFABETO)
        {
            printf("Alfabeto muito grande. Truncando...\n");
            break;
        }

        char char_atual = buffer_entrada[i]; 
        
        if (char_atual == LAMBDA) {
            printf("  [Aviso] O simbolo '%c' e reservado para lambda. Ignorando...\n", LAMBDA);
            continue;
        }
        
        bool encontrado = false;
        for (int k = 0; k < indice_unico; k++)
        {
            if (alfabeto[k] == char_atual)
            {
                encontrado = true;
                break;
            }
        }

        if (!encontrado)
        {
            alfabeto[indice_unico] = char_atual;
            indice_unico++;
        }
    }
 
    if (indice_unico == MAX_ALFABETO)
    {
        alfabeto[MAX_ALFABETO - 1] = '\0';
    }
    else
    {
        alfabeto[indice_unico] = '\0';
    }

    int tam_final = strlen(alfabeto); 

    if (tam_final < tam_buffer)
    {
        printf("Simbolos duplicados ou excessivos removidos. Alfabeto final: '%s'\n", alfabeto);
    }
    
    return tam_final;
}


// Função para ler o alfabeto de pilha
int lerAlfabetoPilha(char alfabeto_pilha[MAX_ALFABETO_PILHA])
{
    char buffer_entrada[100];
    printf("\nInsira os simbolos do alfabeto de pilha (ex: ABX): ");
    scanf("%99s", buffer_entrada);
    getchar();

    int tam_buffer = strlen(buffer_entrada);
    int indice_unico = 0; 
    bool z_encontrado = false;
    for (int i = 0; i < tam_buffer; i++)
    {
        if (indice_unico >= MAX_ALFABETO_PILHA - 1) 
        {
            printf("Alfabeto de pilha muito grande. Truncando...\n");
            break;
        }

        char char_atual = buffer_entrada[i];
        if (char_atual == SIMBOLO_INICIAL_PILHA) {
            z_encontrado = true;
        }
        
        bool encontrado = false;
        for (int k = 0; k < indice_unico; k++) {
            if (alfabeto_pilha[k] == char_atual) {
                encontrado = true;
                break;
            }
        }

        if (!encontrado) {
            alfabeto_pilha[indice_unico] = char_atual;
            indice_unico++;
        }
    }
    if (!z_encontrado) {
        if (indice_unico >= MAX_ALFABETO_PILHA) {
             printf("[ERRO] Alfabeto de pilha cheio, não foi possível adicionar 'Z'.\n");
             indice_unico = MAX_ALFABETO_PILHA - 1;
        } else {
             printf("  [Aviso] O simbolo inicial '%c' foi adicionado ao alfabeto de pilha.\n", SIMBOLO_INICIAL_PILHA);
             alfabeto_pilha[indice_unico] = SIMBOLO_INICIAL_PILHA;
             indice_unico++;
        }
    }
    
    alfabeto_pilha[indice_unico] = '\0';

    if (indice_unico == 0) {
        printf("Alfabeto de pilha invalido. Usando '%c' por padrao.\n", SIMBOLO_INICIAL_PILHA);
        alfabeto_pilha[0] = SIMBOLO_INICIAL_PILHA;
        alfabeto_pilha[1] = '\0';
        return 1;
    }

    printf("Alfabeto de pilha final: '%s'\n", alfabeto_pilha);
    return indice_unico;
}

// --- Funções de Definição (Finais, Inicial) ---

void defineEstadosFinais(int num_estados, bool estadosFinais[MAX_ESTADOS])
{
    printf("\n--- Definicao dos Estados Finais (F) ---\n");
    printf("Quais estados sao finais? (Digite 1 para Sim, 0 para Nao)\n");

    for (int i = 0; i < num_estados; i++)
    {
        int ehFinal = -1;
        while(ehFinal != 0 && ehFinal != 1)
        {
            printf("q%d e final? (1=Sim, 0=Nao): ", i);
            scanf("%d", &ehFinal);
            getchar(); 
        }
        estadosFinais[i] = (ehFinal == 1);
    }
}

int defineEstadoInicial(int num_estados)
{
    int estado = -1;
    printf("\n--- Definicao do Estado Inicial ---\n");
    printf("Digite o numero do estado inicial (entre 0 e %d): ", num_estados - 1);
    while (1)
    {
        if (scanf("%d", &estado) != 1)
        {
            int c;
            while ((c = getchar()) != EOF && c != '\n');
            printf("  [Erro] Entrada invalida. Digite um numero entre 0 e %d: ", num_estados - 1);
            continue;
        }
        getchar(); 
        if (estado < 0 || estado >= num_estados)
        {
            printf("  [Erro] Estado invalido. Deve ser entre 0 e %d: ", num_estados - 1);
            continue;
        }
        break;
    }
    return estado;
}


// --- Funções de Definição e Exibição de Transições ---
void defineTransicoesAPN(
    int num_estados,
    int tam_alfabeto,
    int tam_alfabeto_pilha,
    char alfabeto[MAX_ALFABETO],
    char alfabeto_pilha[MAX_ALFABETO_PILHA],
    TransicoesAPN* transicoes
)
{
    memset(transicoes, 0, sizeof(TransicoesAPN));

    printf("\n--- Definicao das Transicoes do APN (δ) ---\n");
    printf("Para cada tripla (estado, simbolo_entrada, simbolo_pilha), insira os pares (proximo_estado, simbolos_a_empilhar).\n");
    printf("Digite -1 quando terminar de inserir destinos para uma tripla.\n");
    printf("Para 'simbolos_a_empilhar':\n");
    printf("  - Digite 'AB' para empilhar 'B' e depois 'A' (A fica no topo).\n");
    printf("  - Digite '%c' para NÃO empilhar nada (apenas desempilhar).\n", LAMBDA);


    for (int i = 0; i < num_estados; i++) 
    {
        for (int j = 0; j <= tam_alfabeto; j++)
        {
            for (int k = 0; k < tam_alfabeto_pilha; k++) 
            {
                char simbolo_entrada;
                if (j == tam_alfabeto) {
                    simbolo_entrada = LAMBDA;
                } else {
                    simbolo_entrada = alfabeto[j];
                }

                char simbolo_pilha = alfabeto_pilha[k];

                printf("\nDefinindo δ(q%d, %c, %c) = {...}\n", i, simbolo_entrada, simbolo_pilha);

                DestinoAPN* ultimo_destino = NULL;
                while (true)
                {
                    int proximo_estado = -2;
                    printf("  Proximo estado (-1 para parar): q");
                    
                    if (scanf("%d", &proximo_estado) != 1) {
                        int c;
                        while ((c = getchar()) != EOF && c != '\n');
                        printf("    [Erro] Entrada invalida. Tente novamente.\n");
                        continue;
                    }
                    getchar();

                    if (proximo_estado == -1) {
                        break;
                    }

                    if (proximo_estado < 0 || proximo_estado >= num_estados) {
                        printf("    [Erro] Estado invalido. Deve ser entre q0 e q%d.\n", num_estados - 1);
                        continue;
                    }

                    char simbolos_empilhar[MAX_PUSH_STRING];
                    printf("  Simbolos a empilhar (ou '%c' para desempilhar): ", LAMBDA);
                    fgets(simbolos_empilhar, sizeof(simbolos_empilhar), stdin);
                    int len = strlen(simbolos_empilhar);
                    if (len > 0 && simbolos_empilhar[len - 1] == '\n') {
                        simbolos_empilhar[len - 1] = '\0';
                    }

                    if (strcmp(simbolos_empilhar, "e") == 0 || len == 0) {
                        strcpy(simbolos_empilhar, "#"); 
                    }
                    DestinoAPN* novo_destino = (DestinoAPN*)malloc(sizeof(DestinoAPN));
                    novo_destino->proximo_estado = proximo_estado;
                    strncpy(novo_destino->para_empilhar, simbolos_empilhar, MAX_PUSH_STRING - 1);
                    novo_destino->para_empilhar[MAX_PUSH_STRING - 1] = '\0';
                    novo_destino->proximo = NULL;

                    if (transicoes->destinos[i][j][k] == NULL) {
                        transicoes->destinos[i][j][k] = novo_destino;
                    } else {
                        ultimo_destino = transicoes->destinos[i][j][k];
                        while (ultimo_destino->proximo != NULL) {
                            ultimo_destino = ultimo_destino->proximo;
                        }
                        ultimo_destino->proximo = novo_destino;
                    }

                    printf("    -> δ(q%d, %c, %c) agora contem (q%d, %s)\n", 
                           i, simbolo_entrada, simbolo_pilha, proximo_estado, simbolos_empilhar);
                }
            }
        }
    }
}

void mostraTabelaTransicoesAPN(
    int num_estados,
    int tam_alfabeto,
    int tam_alfabeto_pilha,
    char alfabeto[MAX_ALFABETO],
    char alfabeto_pilha[MAX_ALFABETO_PILHA],
    bool estadosFinais[MAX_ESTADOS],
    int estadoInicial,
    TransicoesAPN* transicoes
)
{
    printf("\n\n--- Tabela de Transicoes do APN ---\n");
    printf("Legenda: '->' inicial; '*' final.\n");
    printf("Tripla: (estado_entrada, simbolo_entrada, simbolo_pilha) -> {(estado_destino, empilhar)}\n\n");

    for (int i = 0; i < num_estados; i++)
    {
        printf("%sq%d %s\n", (i == estadoInicial) ? "->" : "  ", i, estadosFinais[i] ? "*" : "");

        for (int j = 0; j <= tam_alfabeto; j++)
        {
            char simbolo_entrada = (j == tam_alfabeto) ? LAMBDA : alfabeto[j];

            for (int k = 0; k < tam_alfabeto_pilha; k++)
            {
                char simbolo_pilha = alfabeto_pilha[k];

                if (transicoes->destinos[i][j][k] != NULL)
                {
                    printf("  δ(q%d, %c, %c) = {", i, simbolo_entrada, simbolo_pilha);
                    
                    DestinoAPN* dest = transicoes->destinos[i][j][k];
                    bool primeiro = true;
                    while (dest != NULL)
                    {
                        if (!primeiro) printf(", ");
                        printf("(q%d, %s)", dest->proximo_estado, dest->para_empilhar);
                        primeiro = false;
                        dest = dest->proximo;
                    }
                    printf("}\n");
                }
            }
        }
    }
}


// --- Funções da Pilha ---
void inicializar_pilha(Pilha* p, char simboloInicial) {
    p->topo = -1; 
    empilhar(p, simboloInicial); 
}

char ver_topo(Pilha* p) {
    if (p->topo < 0) return '\0'; 
    return p->S[p->topo];
}

void desempilhar(Pilha* p) {
    if (p->topo >= 0) {
        p->topo--;
    }
}

void empilhar(Pilha* p, char simbolo) {
    if (p->topo < MAX_TAMANHO_PILHA - 1) {
        p->topo++;
        p->S[p->topo] = simbolo;
    }
}

void empilhar_cadeia(Pilha* p, char* cadeia) {
    int len = strlen(cadeia);
    // Ordem inversa para manter a sequência correta na pilha
    for (int i = len - 1; i >= 0; i--) {
        empilhar(p, cadeia[i]);
    }
}

Pilha copiar_pilha(Pilha* origem) {
    Pilha nova;
    nova.topo = origem->topo;
    memcpy(nova.S, origem->S, (origem->topo + 1) * sizeof(char));
    return nova;
}

// --- Funções de Busca de Símbolo ---.
int encontrarIndiceSimbolo(char simbolo, char alfabeto[MAX_ALFABETO], int tam_alfabeto)
{
    for (int i = 0; i < tam_alfabeto; i++)
    {
        if (alfabeto[i] == simbolo)
        {
            return i;
        }
    }
    return -1;
}

int encontrarIndiceSimboloPilha(char simbolo, char alfabeto_pilha[MAX_ALFABETO_PILHA], int tam_alfabeto_pilha)
{
    for (int i = 0; i < tam_alfabeto_pilha; i++)
    {
        if (alfabeto_pilha[i] == simbolo)
        {
            return i;
        }
    }
    return -1;
}

//  Funçao q compara se duas pilhas são idênticas
bool pilha_igual(Pilha* p1, Pilha* p2) {
    if (p1->topo != p2->topo) {
        return false;
    }
    return memcmp(p1->S, p2->S, p1->topo + 1) == 0;
}

void adicionar_config_sem_duplicata(ConfiguracaoAPN** lista_head, int estado, Pilha pilha) {
    ConfiguracaoAPN* atual = *lista_head;

    while (atual != NULL) {
        if (atual->estado_atual == estado && pilha_igual(&atual->pilha, &pilha)) {
            return; 
        }
        atual = atual->proximo;
    }

    ConfiguracaoAPN* nova_config = (ConfiguracaoAPN*)malloc(sizeof(ConfiguracaoAPN));
    nova_config->estado_atual = estado;
    nova_config->pilha = pilha; 
    nova_config->proximo = *lista_head;
    *lista_head = nova_config;
}
void liberar_lista_config(ConfiguracaoAPN* lista_head) {
    while (lista_head != NULL) {
        ConfiguracaoAPN* temp = lista_head;
        lista_head = lista_head->proximo;
        free(temp);
    }
}
// ===================================================================
// Calcula o "Epsilon-Closure" (fecho-lambda) de um conjunto de configurações
// Modifica o conjunto (config_set) no local, adicionando todos os estados
// alcançáveis por transições 'e' (lambda).
// ===================================================================
void calcular_epsilon_closure(
    ConfiguracaoAPN** config_set, 
    TransicoesAPN* transicoes,
    int tam_alfabeto, 
    char alfabeto_pilha[MAX_ALFABETO_PILHA],
    int tam_alfabeto_pilha
) {
    bool mudanca = true;
    while (mudanca) {
        mudanca = false;
        ConfiguracaoAPN* cfg = *config_set; 
        
        while (cfg != NULL) {
            char topo = ver_topo(&cfg->pilha);
            int idx_topo = encontrarIndiceSimboloPilha(topo, alfabeto_pilha, tam_alfabeto_pilha);
            int idx_lambda = tam_alfabeto; 

            if (idx_topo != -1) {
                DestinoAPN* dest = transicoes->destinos[cfg->estado_atual][idx_lambda][idx_topo];
                
                while (dest != NULL) {
                    Pilha nova_pilha = copiar_pilha(&cfg->pilha);
                    desempilhar(&nova_pilha);
                    if (strcmp(dest->para_empilhar, "#") != 0) {
                        empilhar_cadeia(&nova_pilha, dest->para_empilhar);
                    }

                    ConfiguracaoAPN* check = *config_set;
                    bool existe = false;
                    while (check != NULL) {
                        if (check->estado_atual == dest->proximo_estado && pilha_igual(&check->pilha, &nova_pilha)) {
                            existe = true;
                            break;
                        }
                        check = check->proximo;
                    }

                    if (!existe) {
                        adicionar_config_sem_duplicata(config_set, dest->proximo_estado, nova_pilha);
                        mudanca = true; 
                    }
                    dest = dest->proximo;
                }
            }
            cfg = cfg->proximo;
        }
    }
}

// Simula o APN usando a lógica de BFS (Busca em Largura)
bool simularAPN(
    char* cadeia,
    int estadoInicial,
    bool estadosFinais[MAX_ESTADOS],
    TransicoesAPN* transicoes,
    char alfabeto[MAX_ALFABETO],
    int tam_alfabeto,
    char alfabeto_pilha[MAX_ALFABETO_PILHA],
    int tam_alfabeto_pilha,
    int num_estados
)
{
    printf("\n--- Simulacao do APN (BFS) ---\n");
    printf("Entrada: '%s'\n", (strlen(cadeia) == 0) ? "[vazio]" : cadeia);

    Pilha pilha_inicial;
    inicializar_pilha(&pilha_inicial, SIMBOLO_INICIAL_PILHA);

    ConfiguracaoAPN* current_set = NULL;
    adicionar_config_sem_duplicata(&current_set, estadoInicial, pilha_inicial);

    //  Calcula o fecho-lambda inicial
    printf("Calculando E-Closure inicial...\n");
    calcular_epsilon_closure(&current_set, transicoes, tam_alfabeto, alfabeto_pilha, tam_alfabeto_pilha);

    // Itera sobre cada símbolo da cadeia de entrada
    int tam_cadeia = strlen(cadeia);
    for (int i = 0; i < tam_cadeia; i++) {
        char simbolo_entrada = cadeia[i];
        int idx_simbolo = encontrarIndiceSimbolo(simbolo_entrada, alfabeto, tam_alfabeto);

        printf("Consumindo simbolo: '%c' (posicao %d)\n", simbolo_entrada, i);

        if (idx_simbolo == -1) {
             printf("  [REJEITADO] Simbolo '%c' nao pertence ao alfabeto.\n", simbolo_entrada);
             liberar_lista_config(current_set);
             return false;
        }

        ConfiguracaoAPN* next_set = NULL; 
        ConfiguracaoAPN* cfg = current_set;
        while (cfg != NULL) {
            char topo = ver_topo(&cfg->pilha);
            int idx_topo = encontrarIndiceSimboloPilha(topo, alfabeto_pilha, tam_alfabeto_pilha);

            if (idx_topo != -1) {
                DestinoAPN* dest = transicoes->destinos[cfg->estado_atual][idx_simbolo][idx_topo];
                
                while (dest != NULL) {
                    Pilha nova_pilha = copiar_pilha(&cfg->pilha);
                    desempilhar(&nova_pilha);
                    if (strcmp(dest->para_empilhar, "#") != 0) {
                        empilhar_cadeia(&nova_pilha, dest->para_empilhar);
                    }
                    
                    adicionar_config_sem_duplicata(&next_set, dest->proximo_estado, nova_pilha);
                    dest = dest->proximo;
                }
            }
            cfg = cfg->proximo;
        }

        liberar_lista_config(current_set); 

        if (next_set == NULL) {
            printf("  [REJEITADO] Nenhuma transicao possivel. O automato 'morreu'.\n");
            return false;
        }

        calcular_epsilon_closure(&next_set, transicoes, tam_alfabeto, alfabeto_pilha, tam_alfabeto_pilha);
        current_set = next_set; // O "próximo" vira o "atual"
    }

    printf("\nFim da cadeia. Verificando estados finais...\n");
    bool aceito = false;
    ConfiguracaoAPN* cfg = current_set;
    while (cfg != NULL) {
        printf("  Configuracao final: (q%d, topo_pilha='%c')\n", cfg->estado_atual, ver_topo(&cfg->pilha));
        
        // **ACEITAÇÃO POR ESTADO FINAL**
        if (estadosFinais[cfg->estado_atual]) {
            aceito = true;
            break;
        }
        cfg = cfg->proximo;
    }

    liberar_lista_config(current_set);

    if (aceito) {
        printf("\n[ACEITO] A cadeia foi aceita por estado final.\n");
    } else {
        printf("\n[REJEITADO] A cadeia terminou, mas nenhum estado final foi alcancado.\n");
    }

    return aceito;
}

// Função main
int main()
{
    setlocale(LC_ALL, "pt_BR.UTF-8");

    printf("========== AUTOMATO A PILHA NAO-DETERMINISTICO (APN) ==========\n\n");

    // Entrada do APN
    int num_estados = leNumEstados();
    
    char alfabeto[MAX_ALFABETO];
    int tam_alfabeto = lerAlfabeto(alfabeto);
    
    char alfabeto_pilha[MAX_ALFABETO_PILHA];
    int tam_alfabeto_pilha = lerAlfabetoPilha(alfabeto_pilha);
    
    bool estadosFinais[MAX_ESTADOS] = { false };
    defineEstadosFinais(num_estados, estadosFinais);
    
    int estadoInicial = defineEstadoInicial(num_estados);

    // Inicializa transições do APN
    TransicoesAPN transicoes;
    defineTransicoesAPN(num_estados, tam_alfabeto, tam_alfabeto_pilha, alfabeto, alfabeto_pilha, &transicoes);

    // Exibe tabela de transições
    mostraTabelaTransicoesAPN(num_estados, tam_alfabeto, tam_alfabeto_pilha, alfabeto, alfabeto_pilha, estadosFinais, estadoInicial, &transicoes);

    // Menu de simulação
    printf("\n========== MENU APN ==========\n");
    printf("0 - Sair\n");
    printf("1 - Simular uma cadeia\n");
    printf("Escolha uma opcao: ");
    
    int opcao_menu = -1;
    while(opcao_menu != 0)
    {
        printf("\nEscolha uma opcao (1=Simular, 0=Sair): ");
        if (scanf("%d", &opcao_menu) != 1) {
            opcao_menu = -1; // Invalida
        }
        getchar(); 

        if (opcao_menu == 1)
        {
            char cadeia[MAX_COMPRIMENTO_CADEIA];
            printf("\nDigite a cadeia a simular (ou Enter para cadeia vazia): ");
            fgets(cadeia, sizeof(cadeia), stdin);
            
            int len = strlen(cadeia);
            if (len > 0 && cadeia[len - 1] == '\n') {
                cadeia[len - 1] = '\0';
            }

            simularAPN(cadeia, estadoInicial, estadosFinais, &transicoes, alfabeto, tam_alfabeto, alfabeto_pilha, tam_alfabeto_pilha, num_estados);
        }
        else if (opcao_menu != 0)
        {
            printf("Opcao invalida.\n");
        }
    }

    printf("\n========== FIM DO PROGRAMA ==========\n");
    
    // Libera a memória alocada para as transições
    for (int i = 0; i < num_estados; i++) {
        for (int j = 0; j <= tam_alfabeto; j++) {
            for (int k = 0; k < tam_alfabeto_pilha; k++) {
                liberar_lista_config(transicoes.destinos[i][j][k]);
            }
        }
    }

    return 0;
}
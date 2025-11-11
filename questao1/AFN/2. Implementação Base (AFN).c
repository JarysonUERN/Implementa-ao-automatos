/*
Um AFN é formalmente definido por 5 componentes (a 5-tupla: $Q, \Sigma, \delta, q_0, F$):
$Q$ (Conjunto de Estados): Representado por num_estados.
$\Sigma$ (Alfabeto): Representado pelo array alfabeto e seu tamanho tam_alfabeto.
$\delta$ (Função de Transição): Representada pela matriz 3D 
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS].
    O índice [tam_alfabeto] é usado para transições lambda (epsilon).
    transicoes[i][j][k] = true significa que $\delta(q_i, simbolo[j])$ contém $q_k$.
$q_0$ (Estado Inicial): Representado pela variável int estadoInicial.
$F$ (Conjunto de Estados Finais): Representado pelo array booleano bool estadosFinais[MAX_ESTADOS].
*/

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>
#include "visualize/afn_serializacao.h"
#include "visualize/afn_serializacao.c" 
#include "../cJSON-1.7.19/cJSON.h"

// Definindo limites máximos para alocação estática
#define MAX_ESTADOS 50
#define MAX_ALFABETO 10
#define LAMBDA 'e' // Caractere para representar Epsilon/Lambda
#define MAX_CADEIAS 10000 // Máximo de cadeias a armazenar
#define MAX_COMPRIMENTO_CADEIA 100 // Máximo de caracteres por cadeia


// Estrutura para armazenar cadeias aceitas
typedef struct {
    char cadeias[MAX_CADEIAS][MAX_COMPRIMENTO_CADEIA];
    int total;
    int limite_atingido; // Flag indicando se atingiu o limite
} ConjuntoCadeias;


// Funções originais (reutilizadas)
int leNumEstados();
int lerAlfabeto(char alfabeto[MAX_ALFABETO]);
void defineEstadosFinais(int num_estados, bool estadosFinais[MAX_ESTADOS]);
int defineEstadoInicial(int num_estados);
int encontrarIndiceSimbolo(char simbolo, char alfabeto[MAX_ALFABETO], int tam_alfabeto);

// Funções para simulação do AFN
void defineTransicoesAFN(int num_estados, int tam_alfabeto, char alfabeto[MAX_ALFABETO], bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS]);
void mostraTabelaTransicoesAFN(int num_estados, int tam_alfabeto, char alfabeto[MAX_ALFABETO], bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS], bool estadosFinais[MAX_ESTADOS], int estadoInicial);
// A função simularAFN foi removida.

// Funções auxiliares para a simulação do AFN
void calcular_epsilon_fecho(bool fecho[MAX_ESTADOS], int num_estados, int tam_alfabeto, bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS]);
void copiar_conjunto(bool destino[MAX_ESTADOS], bool origem[MAX_ESTADOS], int num_estados);
void imprimir_conjunto(bool conjunto[MAX_ESTADOS], int num_estados);



// Função para ler o número de estados 
int leNumEstados()
{
    setlocale(LC_ALL, "pt_BR.UTF-8");
    int n;
    printf("Insira quantos estados tem o seu AFN (1 a %d): ", MAX_ESTADOS);
    scanf("%d", &n);
    getchar(); 
    
    if (n > MAX_ESTADOS || n <= 0)
    {
        printf("Número invalido. Usando 3 estados por padrao.\n");
        return 3;
    }
    return n;
}

// Função para ler o alfabeto 
int lerAlfabeto(char alfabeto[MAX_ALFABETO])
{
    char buffer_entrada[100];
    printf("\nInsira os simbolos do seu alfabeto (sem espacos, ex: 01, abc): ");
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
            printf(" Alfabeto muito grande. Truncando...\n");
            break;
        }

        char char_atual = buffer_entrada[i]; 
        
        // Verifica se é o caractere especial lambda
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
        printf(" Simbolos duplicados ou excessivos removidos. Alfabeto final: '%s'\n", alfabeto);
    }
    
    return tam_final;
}

// Função para definir estados finais (sem alterações)
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

// Função para definir estado inicial 
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

// Função para definir as transições do AFN 
void defineTransicoesAFN(int num_estados, int tam_alfabeto, char alfabeto[MAX_ALFABETO], bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS])
{
    memset(transicoes, false, sizeof(bool) * MAX_ESTADOS * (MAX_ALFABETO + 1) * MAX_ESTADOS);

    printf("\n--- Definicao das transicoes (δ) ---\n");
    printf("Para cada par (estado, simbolo), insira os estados de destino.\n");
    printf("Digite -1 quando terminar de inserir destinos para um par.\n");

    for (int i = 0; i < num_estados; i++) // Para cada estado 'i'
    {
        // O loop 'j' vai até tam_alfabeto (inclusive) para incluir lambda
        for (int j = 0; j <= tam_alfabeto; j++) // Para cada símbolo 'j' + lambda
        { 
            char simbolo;
            if (j == tam_alfabeto) {
                simbolo = LAMBDA; // 'e' para Epsilon/Lambda
            } else {
                simbolo = alfabeto[j];
            }

            printf("\nDefinindo δ(q%d, %c) = {...}\n", i, simbolo);

            // Loop para ler múltiplos estados de destino
            while (true)
            {
                int proximo_estado = -2; // Valor inicial inválido
                printf("  Destino (-1 para parar): q");
                
                if (scanf("%d", &proximo_estado) != 1) {
                    int c;
                    while ((c = getchar()) != EOF && c != '\n'); // Limpa buffer
                    printf("    [Erro] Entrada invalida. Tente novamente.\n");
                    continue;
                }
                getchar(); // Limpa o newline

                if (proximo_estado == -1) {
                    break; // Terminou de adicionar estados para este par
                }

                if (proximo_estado < 0 || proximo_estado >= num_estados) {
                    printf("    [Erro] Estado invalido. Deve ser entre q0 e q%d.\n", num_estados - 1);
                } else {
                    // Define a transição como verdadeira
                    transicoes[i][j][proximo_estado] = true;
                    printf("    -> δ(q%d, %c) agora contem q%d\n", i, simbolo, proximo_estado);
                }
            }
        }
    }
}

// Função para exibir a tabela de transições do AFN
void mostraTabelaTransicoesAFN(int num_estados, int tam_alfabeto, char alfabeto[MAX_ALFABETO], bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS], bool estadosFinais[MAX_ESTADOS], int estadoInicial)
{
    printf("\n\n--- Tabela de Transicoes (Completa) ---\n");
    printf("Legenda: '->' inicial; '*' final.\n\n");
    printf("Estado\t| ");
    for (int j = 0; j < tam_alfabeto; j++)
    {
        printf("    %c   \t| ", alfabeto[j]);
    }
    printf("    %c   \t| ", LAMBDA); // Coluna Lambda
    printf("\n");

    printf("--------+");
    for (int j = 0; j <= tam_alfabeto; j++) // +1 para lambda
    {
        printf("----------\t+-");
    }
    printf("\n");

    // Imprime o corpo da tabela
    for (int i = 0; i < num_estados; i++)
    {
        printf("%sq%d \t %s \t| ", (i == estadoInicial) ? "->" : "  ", i, estadosFinais[i] ? "*" : " ");
        
        // Loop sobre símbolos E lambda
        for (int j = 0; j <= tam_alfabeto; j++)
        {
            printf(" {");
            bool primeiro = true;
            // Loop k para encontrar todos os destinos
            for (int k = 0; k < num_estados; k++) {
                if (transicoes[i][j][k]) {
                    if (!primeiro) printf(", ");
                    printf("q%d", k);
                    primeiro = false;
                }
            }
            printf("}\t| ");
        }
        printf("\n");
    }
}


// Verifica se uma cadeia é aceita pelo AFN
bool ehCadeiaAceita(
    char* cadeia,
    int estadoInicial,
    bool estadosFinais[MAX_ESTADOS],
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS],
    char alfabeto[MAX_ALFABETO],
    int tam_alfabeto,
    int num_estados
) {
    bool estadosAtuais[MAX_ESTADOS] = { false };
    int tam_cadeia = strlen(cadeia);

    estadosAtuais[estadoInicial] = true;
    calcular_epsilon_fecho(estadosAtuais, num_estados, tam_alfabeto, transicoes);

    for (int i = 0; i < tam_cadeia; i++) {
        char char_atual = cadeia[i];
        int indice_simbolo = encontrarIndiceSimbolo(char_atual, alfabeto, tam_alfabeto);

        if (indice_simbolo == -1) {
            return false; // Símbolo inválido
        }

        bool proximosEstados[MAX_ESTADOS] = { false };
        for (int q = 0; q < num_estados; q++) {
            if (estadosAtuais[q]) {
                for (int r = 0; r < num_estados; r++) {
                    if (transicoes[q][indice_simbolo][r]) {
                        proximosEstados[r] = true;
                    }
                }
            }
        }

        calcular_epsilon_fecho(proximosEstados, num_estados, tam_alfabeto, transicoes);
        copiar_conjunto(estadosAtuais, proximosEstados, num_estados);

        // Verifica se "morreu"
        bool algum_ativo = false;
        for (int k = 0; k < num_estados; k++) {
            if (estadosAtuais[k]) {
                algum_ativo = true;
                break;
            }
        }
        if (!algum_ativo) {
            return false;
        }
    }

    // Verifica se algum estado ativo é final
    for (int i = 0; i < num_estados; i++) {
        if (estadosAtuais[i] && estadosFinais[i]) {
            return true;
        }
    }
    return false;
}

// Gera todas as cadeias aceitas até um comprimento máximo
void explorarCadeiasAceitas(
    int estadoInicial,
    bool estadosFinais[MAX_ESTADOS],
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS],
    char alfabeto[MAX_ALFABETO],
    int tam_alfabeto,
    int num_estados,
    int max_comprimento,
    ConjuntoCadeias* resultado
) {
    resultado->total = 0;
    resultado->limite_atingido = 0;

    // Gera cadeias começando do comprimento 0 (cadeia vazia)
    for (int comprimento = 0; comprimento <= max_comprimento; comprimento++) {
        if (resultado->total >= MAX_CADEIAS) {
            resultado->limite_atingido = 1;
            break;
        }

        if (comprimento == 0) {
            // Testa cadeia vazia
            char cadeia_vazia[1] = {'\0'};
            if (ehCadeiaAceita(cadeia_vazia, estadoInicial, estadosFinais, transicoes, alfabeto, tam_alfabeto, num_estados)) {
                strcpy(resultado->cadeias[resultado->total], cadeia_vazia);
                resultado->total++;
            }
        } else {
            // Gera todas as cadeias de comprimento 'comprimento'
            int total_combinacoes = 1;
            for (int i = 0; i < comprimento; i++) {
                total_combinacoes *= tam_alfabeto;
            }

            for (int combo = 0; combo < total_combinacoes; combo++) {
                if (resultado->total >= MAX_CADEIAS) {
                    resultado->limite_atingido = 1;
                    break;
                }

                // Constrói cadeia a partir do índice 'combo'
                char cadeia_temp[MAX_COMPRIMENTO_CADEIA] = {'\0'};
                int temp_combo = combo;
                for (int pos = comprimento - 1; pos >= 0; pos--) {
                    cadeia_temp[pos] = alfabeto[temp_combo % tam_alfabeto];
                    temp_combo /= tam_alfabeto;
                }
                cadeia_temp[comprimento] = '\0';

                // Verifica se a cadeia é aceita
                if (ehCadeiaAceita(cadeia_temp, estadoInicial, estadosFinais, transicoes, alfabeto, tam_alfabeto, num_estados)) {
                    strcpy(resultado->cadeias[resultado->total], cadeia_temp);
                    resultado->total++;
                }
            }

            if (resultado->limite_atingido) {
                break;
            }
        }
    }
}

// Imprime as cadeias aceitas
void imprimir_cadeias_aceitas(ConjuntoCadeias* conjunto) {
    if (conjunto->total == 0) {
        printf("Nenhuma cadeia aceita encontrada (ate o limite especificado).\n");
        return;
    }

    printf("\n=== Cadeias Aceitas ===\n");
    printf("Total encontrado: %d\n\n", conjunto->total);

    for (int i = 0; i < conjunto->total; i++) {
        if (strlen(conjunto->cadeias[i]) == 0) {
            printf("%d. [cadeia vazia - ε]\n", i + 1);
        } else {
            printf("%d. %s\n", i + 1, conjunto->cadeias[i]);
        }
    }

    if (conjunto->limite_atingido) {
        printf("\n... (limite de cadeias atingido)\n");
    }
}

// Função principal

int main()
{
    int num_estados = leNumEstados();

    char alfabeto[MAX_ALFABETO];
    int tam_alfabeto = lerAlfabeto(alfabeto);
    
    bool estadosFinais[MAX_ESTADOS] = { false };

    // [origem][simbolo + 1 para lambda][destino]
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS];
    defineTransicoesAFN(num_estados, tam_alfabeto, alfabeto, transicoes);
    
    defineEstadosFinais(num_estados, estadosFinais);
    int estadoInicial = defineEstadoInicial(num_estados);
    mostraTabelaTransicoesAFN(num_estados, tam_alfabeto, alfabeto, transicoes, estadosFinais, estadoInicial);
    

    // Menu de simulação
    int opcao_menu = -1;
    while (opcao_menu != 0 && opcao_menu != 1) { 
        printf("\n========== MENU AFN ==========\n");
        printf("0 - Nao simular\n");
        printf("1 - Explorar TODAS as cadeias aceitas\n"); 
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao_menu);
        getchar(); 
    }

    if (opcao_menu == 0) {
        printf("\nSimulacao do AFN ignorada pelo usuario.\n");
        // Serializa o AFN em formato JSON
        salvarAFN_JSON(num_estados, tam_alfabeto, alfabeto, transicoes, estadosFinais, estadoInicial);
    } 

    else if (opcao_menu == 1) { 
        printf("\n--- Exploracao de TODAS as Cadeias Aceitas ---\n");
        
        int max_comprimento = -1;
        while (max_comprimento < 0) {
            printf("Qual o comprimento maximo das cadeias a explorar? (ex: 5): ");
            if (scanf("%d", &max_comprimento) != 1) {
                int c;
                while ((c = getchar()) != EOF && c != '\n');
                printf("[Erro] Entrada invalida.\n");
                max_comprimento = -1;
            } else {
                getchar();
                if (max_comprimento < 0) {
                    printf("[Erro] Comprimento deve ser >= 0.\n");
                }
            }
        }

        printf("\nExplorando cadeias aceitas (comprimento maximo: %d)...\n", max_comprimento);
        ConjuntoCadeias resultado;
        explorarCadeiasAceitas(estadoInicial, estadosFinais, transicoes, alfabeto, tam_alfabeto, num_estados, max_comprimento, &resultado);
        
        imprimir_cadeias_aceitas(&resultado);

        // Serializa o AFN em formato JSON
        salvarAFN_JSON(num_estados, tam_alfabeto, alfabeto, transicoes, estadosFinais, estadoInicial);
        printf("\nAFN salvo em 'afn_serializado.json'.\n");
    }
    
    return 0;  
}

//Implementação da Simulação do AFN

// Função para encontrar índice do símbolo
int encontrarIndiceSimbolo(char simbolo, char alfabeto[MAX_ALFABETO], int tam_alfabeto)
{
    for (int i = 0; i < tam_alfabeto; i++)
    {
        if (alfabeto[i] == simbolo)
        {
            return i; // Retorna o índice da coluna
        }
    }
    return -1; // Símbolo não encontrado
}

// Imprime um conjunto de estados (ex: {q0, q2})
void imprimir_conjunto(bool conjunto[MAX_ESTADOS], int num_estados) {
    printf("{");
    bool primeiro = true;
    for (int i = 0; i < num_estados; i++) {
        if (conjunto[i]) {
            if (!primeiro) printf(", ");
            printf("q%d", i);
            primeiro = false;
        }
    }
    printf("}");
}


// Copia o 'origem' para o 'destino'
void copiar_conjunto(bool destino[MAX_ESTADOS], bool origem[MAX_ESTADOS], int num_estados) {
    for (int i = 0; i < num_estados; i++) {
        destino[i] = origem[i];
    }
}


// Calcula o Epsilon-Fecho de um conjunto de estados (modifica 'fecho' in-place)
void calcular_epsilon_fecho(
    bool fecho[MAX_ESTADOS], // O conjunto a ser expandido
    int num_estados,
    int tam_alfabeto,
    bool transicoes[MAX_ESTADOS][MAX_ALFABETO + 1][MAX_ESTADOS]
) {
    int pilha[MAX_ESTADOS];
    int topo_pilha = 0;

    for (int i = 0; i < num_estados; i++) {
        if (fecho[i]) {
            pilha[topo_pilha++] = i;
        }
    }

    while (topo_pilha > 0) {
        int estado_q = pilha[--topo_pilha]; // Pop

        // Encontra todos os estados 'r' alcançáveis por 'e' (lambda) a partir de 'q'
        // O índice 'e' é 'tam_alfabeto'
        for (int r = 0; r < num_estados; r++) {
            // Se há transição q --e--> r E 'r' NÃO está no fecho ainda
            if (transicoes[estado_q][tam_alfabeto][r] && !fecho[r]) {
                fecho[r] = true;    // Adiciona 'r' ao fecho
                pilha[topo_pilha++] = r; // Push 'r' para processar seus e-fechos
            }
        }
    }
}

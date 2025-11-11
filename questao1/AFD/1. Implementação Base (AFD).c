
// a. Implementar a representação de um AFD.


/*
Um AFD é formalmente definido por 5 componentes (a 5-tupla: $Q, \Sigma, \delta, q_0, F$):
$Q$ (Conjunto de Estados): Representado por num_estados.$\Sigma$ 
(Alfabeto): Representado pelo array alfabeto e seu tamanho tam_alfabeto.$\delta$ 
(Função de Transição): Perfeitamente representada pela matriz transicoes[MAX_ESTADOS][MAX_ALFABETO], onde transicoes[i][j] 
armazena o estado de destino ao sair do estado i com o símbolo j.$q_0$ 
(Estado Inicial): Representado pela variável int estadoInicial.$F$ 
(Conjunto de Estados Finais): Representado pelo array booleano bool estadosFinais[MAX_ESTADOS].
*/

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>
#include "visualize/afd_serializacao.h"
#include "visualize/afd_serializacao.c"
#include "cJSON-1.7.19/cJSON.h"
// Definindo limites máximos para alocação estática
#define MAX_ESTADOS 50
#define MAX_ALFABETO 10

// Protótipos de funções
void simularAFD(char* cadeia, int estadoInicial, bool estadosFinais[MAX_ESTADOS], int transicoes[MAX_ESTADOS][MAX_ALFABETO], char alfabeto[MAX_ALFABETO], int tam_alfabeto);
 
// Função para ler o número de estados
int leNumEstados()
{
    setlocale(LC_ALL, "pt_BR.UTF-8");
    int n;
    printf("Insira quantos estados tem o seu AFD (1 a %d): ", MAX_ESTADOS);
    scanf("%d", &n);
    getchar(); // Limpa o buffer
    
// Validação simples
    if (n > MAX_ESTADOS || n <= 0)
    {
        printf("Número invalido. Usando 3 estados por padrao.\n");
        return 3;
    }
    return n;
}

// {* Função para ler o alfabeto e remover símbolos duplicados *} //
int lerAlfabeto(char alfabeto[MAX_ALFABETO])
{
    char buffer_entrada[100]; // Buffer temporário para leitura inicial

    printf("\nInsira os simbolos do seu alfabeto (sem espacos, ex: 01, abc): ");
    scanf("%99s", buffer_entrada);
    getchar(); // Limpa o buffer

    int tam_buffer = strlen(buffer_entrada);

    // Validação de entrada vazia
    if (tam_buffer == 0)
    {
        printf("Alfabeto invalido. Usando '01' por padrao.\n");
        strcpy(alfabeto, "01");
        return 2; // Tamanho do alfabeto padrão
    }

    // Remoção de símbolos duplicados

    int indice_unico = 0; // Índice "escritor" no array 'alfabeto'

    for (int i = 0; i < tam_buffer; i++)
    { // Índice "leitor" no 'buffer_entrada'

        // Verifica o limite antes de escrever
        if (indice_unico >= MAX_ALFABETO)
        {
            printf("  [Info] Alfabeto muito grande. Truncando...\n");
            break; // Para de adicionar novos símbolos
        }

        char char_atual = buffer_entrada[i]; // Lê do buffer de entrada
        bool encontrado = false;

        // Verifica se 'char_atual' já está na parte "limpa" (array 'alfabeto')
        for (int k = 0; k < indice_unico; k++)
        {
            if (alfabeto[k] == char_atual)
            {
                encontrado = true;
                break;
            }
        }

        // Se não foi encontrado, é um símbolo único. Adiciona ele.
        if (!encontrado)
        {
            alfabeto[indice_unico] = char_atual;
            indice_unico++; // Avança o índice "escritor"
        }
    }
  
    // Se indice_unico for 10 (MAX_ALFABETO), o último índice válido é 9.
    if (indice_unico == MAX_ALFABETO)
    {
        alfabeto[MAX_ALFABETO - 1] = '\0'; // Garante que o último char seja nulo se estourar
    }
    else
    {
        alfabeto[indice_unico] = '\0'; // Finaliza a string
    }

    int tam_final = strlen(alfabeto); // O novo tamanho real do alfabeto

    if (tam_final < tam_buffer)
    {
        printf("  [Info] Simbolos duplicados ou excessivos removidos. Alfabeto final: '%s'\n", alfabeto);
    }

    // CORREÇÃO: Retorna o tamanho final e limpo
    return tam_final;
}
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
            getchar(); // Limpa o buffer
        }

        if (ehFinal == 1)
        {
            estadosFinais[i] = true;
        }
        else if (ehFinal == 0){
            estadosFinais[i] = false;
        }
    }
}

// Protótipo para evitar advertência de declaração implícita
int defineEstadoInicial(int num_estados);
void defineTransicoes(int num_estados, int tam_alfabeto, char alfabeto[MAX_ALFABETO], int transicoes[MAX_ESTADOS][MAX_ALFABETO])
{

    printf("\n--- Definicao das transicoes (δ) ---\n");
    for (int i = 0; i < num_estados; i++)
    { // Para cada estado 'i'
        for (int j = 0; j < tam_alfabeto; j++)
        { // Para cada símbolo 'j' no alfabeto

            char simbolo = alfabeto[j];
            int proximo_estado = -1;

            // Solicita o próximo estado até que um valor válido seja inserido
            while (proximo_estado < 0 || proximo_estado >= num_estados)
            {
                printf("δ(q%d, %c) = q", i, simbolo); // Ex: δ(q0, 'a') = q?
                scanf("%d", &proximo_estado);
                getchar(); // Limpa o buffer

                if (proximo_estado < 0 || proximo_estado >= num_estados)
                {
                    printf("  [Erro] Estado invalido. Deve ser entre q0 e q%d.\n", num_estados - 1);
                }
            }
            transicoes[i][j] = proximo_estado;
        }
    }
}

// Função para ler o estado inicial com validação de erros
int defineEstadoInicial(int num_estados)
{
    int estado = -1;
    printf("\n--- Definicao do Estado Inicial ---\n");
    printf("Digite o numero do estado inicial (entre 0 e %d): ", num_estados - 1);
    while (1)
    {
        if (scanf("%d", &estado) != 1)
        {
            // Entrada inválida (não-numérica). Limpa buffer e solicita novamente.
            int c;
            while ((c = getchar()) != EOF && c != '\n');
            printf("  [Erro] Entrada invalida. Digite um numero entre 0 e %d: ", num_estados - 1);
            continue;
        }
        getchar(); // limpa o newline
        if (estado < 0 || estado >= num_estados)
        {
            printf("  [Erro] Estado invalido. Deve ser entre 0 e %d: ", num_estados - 1);
            continue;
        }
        break;
    }
    return estado;
}

// Função para exibir a tabela de transições
void mostraTabelaTransicoes(int num_estados, int tam_alfabeto, char alfabeto[MAX_ALFABETO], int transicoes[MAX_ESTADOS][MAX_ALFABETO], bool estadosFinais[MAX_ESTADOS], int estadoInicial)
{
    printf("\n\n--- Tabela de Transicoes (Completa) ---\n");

    // Imprime o cabeçalho (Símbolos do Alfabeto)
    printf("Estado\t| ");
    for (int j = 0; j < tam_alfabeto; j++)
    {
        printf("  %c  \t| ", alfabeto[j]);
    }
    printf("\n");

    // Imprime a linha divisória
    printf("--------+");
    for (int j = 0; j < tam_alfabeto; j++)
    {
        printf("-----\t+-");
    }
    printf("\n");

    // Imprime a legenda indicando estados finais e inicial
    printf("Legenda: '->' indica estado inicial; '*' indica estado final.\n\n");

    // Imprime o corpo da tabela
    for (int i = 0; i < num_estados; i++)
    {
        // Marca estado inicial com '->' e estado final com '*' se aplicável
        printf("%sq%d%s\t| ", (i == estadoInicial) ? "->" : "", i, estadosFinais[i] ? "*" : ""); // Estado atual
        for (int j = 0; j < tam_alfabeto; j++)
        {
            printf(" q%d\t| ", transicoes[i][j]); // Próximo estado
        }
        printf("\n");
    }
}
int main()
{
    int num_estados = leNumEstados();

    char alfabeto[MAX_ALFABETO];
    int tam_alfabeto = lerAlfabeto(alfabeto);
    bool estadosFinais[MAX_ESTADOS] = { false };
    // Matriz para armazenar as transições
    int transicoes[MAX_ESTADOS][MAX_ALFABETO];
    // Inicializa a matriz de transições
    defineTransicoes(num_estados, tam_alfabeto, alfabeto, transicoes);
    // Definição dos estados finais
    defineEstadosFinais(num_estados, estadosFinais);
    // Definição do estado inicial
    int estadoInicial = defineEstadoInicial(num_estados);
    // Exibe a tabela de transições (passa também os estados finais e o estado inicial)
    mostraTabelaTransicoes(num_estados, tam_alfabeto, alfabeto, transicoes, estadosFinais, estadoInicial);
   int opcao;
   validacao:
   printf("\nDeseja simular uma cadeia? (1-Sim, 0-Nao): ");
    scanf("%d", &opcao);
    getchar(); // Limpa o buffer
    if (opcao == 1)
    {
        char cadeia[100];
        printf("Insira a cadeia para simular: ");
        scanf("%99s", cadeia);
        getchar(); // Limpa o buffer

        simularAFD(cadeia, estadoInicial, estadosFinais, transicoes, alfabeto, tam_alfabeto);
        goto validacao;
    }
    else if (opcao == 0)
    {
        printf("Encerrando o programa.\n");
    }
    else
    {
        printf("Opcao invalida.\n");
        goto validacao;
    }
    // Serializa o AFD em formato JSON
    salvarAFD_JSON(num_estados, tam_alfabeto, alfabeto, transicoes, estadosFinais, estadoInicial);
    return 0;
}
// b. Implementar uma função que simule a execução do AFD, verificando se uma cadeia é ACEITA ou REJEITA.

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
 void simularAFD(
    char* cadeia,
    int estadoInicial,
    bool estadosFinais[MAX_ESTADOS],
    int transicoes[MAX_ESTADOS][MAX_ALFABETO],
    char alfabeto[MAX_ALFABETO],
    int tam_alfabeto)
{
    int estadoAtual = estadoInicial;
    int tam_cadeia = strlen(cadeia);

    printf("\n--- Simulando a cadeia: '%s' ---\n", cadeia);
    printf("Percurso: q%d", estadoAtual);

    //  Processa cada símbolo da cadeia
    for (int i = 0; i < tam_cadeia; i++)
    {
        char char_atual = cadeia[i];

        // Descobre para qual coluna da tabela de transição este símbolo aponta
        int indice_simbolo = encontrarIndiceSimbolo(char_atual, alfabeto, tam_alfabeto);

        //  Validação: O símbolo pertence ao alfabeto?
        if (indice_simbolo == -1)
        {
            printf("\n[Erro] O simbolo '%c' na posicao %d nao pertence ao alfabeto.\n", char_atual, i);
            printf("Resultado: CADEIA REJEITADA (Simbolo invalido)\n");
            return; // Interrompe a simulação
        }

        // Executa a transição: δ(estadoAtual, char_atual)
        int proximo_estado = transicoes[estadoAtual][indice_simbolo];

        // Imprime o passo da simulação
        printf(" --(%c)--> q%d", char_atual, proximo_estado);

        // Atualiza o estado atual
        estadoAtual = proximo_estado;
    }

    // Fim da cadeia. Verifica o estado final.
    printf("\nProcessamento concluido. Estado final: q%d.\n", estadoAtual);

    // Verifica se o estado onde paramos é um estado final (de aceitação)
    if (estadosFinais[estadoAtual] == true)
    {
        printf("Resultado: CADEIA ACEITA\n");
    }
    else
    {
        printf("Resultado: CADEIA REJEITADA\n");
    }
}
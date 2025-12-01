# ======= DESCRIÇÃO FORMAL DA MÁQUINA DE TURING =======
#
# (
#     {q0, q_accept, q_reject},              # conjunto de estados
#     {0, 1},                                # alfabeto da máquina (símbolos da entrada)
#     {0, 1, _},                              # alfabeto da fita
#     {
#         (q0, 0) -> (q0, 1, R),
#         (q0, 1) -> (q0, 0, R),
import sys

def le_alfabeto():
    while True:
        s = input("Insira os simbolos do alfabeto (sem espacos, ex: 01 ou abc): ").strip()
        if s == "":
            print("Alfabeto invalido. Tente novamente.")
            continue
        if 'e' in s:
            print("Aviso: 'e' e reservado — removido do alfabeto.")
            s = s.replace('e', '')
        return list(s)


def le_num_estados():
    while True:
        try:
            n = int(input("Insira o numero de estados (ex: 2) -> cria q0..qN-1: ").strip())
            if n <= 0:
                print("Numero de estados deve ser > 0")
                continue
            return n
        except ValueError:
            print("Entrada invalida. Digite um numero inteiro.")


def le_estados_finais(n):
    finais = [False] * n
    print("Defina quais estados sao finais (1=Sim, 0=Nao):")
    for i in range(n):
        while True:
            v = input(f"q{i} e final? (1/0): ").strip()
            if v in ('0', '1'):
                finais[i] = (v == '1')
                break
            print("Resposta invalida. Digite 1 ou 0.")
    return finais


def le_estado_inicial(n):
    while True:
        try:
            v = int(input(f"Digite o numero do estado inicial (0..{n-1}): ").strip())
            if 0 <= v < n:
                return v
            print("Estado invalido.")
        except ValueError:
            print("Entrada invalida.")


def definir_transicoes(n, alfabeto):
    # transicoes: dict (estado, simbolo) -> (prox_estado:int, simbolo_escrito:str, direcao:'L'|'R')
    trans = {}
    simbolos = alfabeto + ['_']
    print("\n-- Definicao de transicoes --")
    print("Para cada par (estado, simbolo) digite: prox_estado simbolo_escrito direcao(L/R)")
    print("ou digite 'none' se nao houver transicao para esse par.")
    for q in range(n):
        for s in simbolos:
            while True:
                entrada = input(f"δ(q{q}, {s}) -> ").strip()
                if entrada.lower() == 'none' or entrada == '':
                    break
                partes = entrada.split()
                if len(partes) != 3:
                    print("Formato invalido. Ex: 1 0 R  ou 'none'")
                    continue
                try:
                    prox = int(partes[0])
                except ValueError:
                    print("prox_estado deve ser inteiro (ex: 0,1,...)")
                    continue
                if not (0 <= prox < n):
                    print("prox_estado fora do intervalo")
                    continue
                write = partes[1]
                if len(write) != 1:
                    print("simbolo_escrito deve ser um unico caractere")
                    continue
                dirc = partes[2].upper()
                if dirc not in ('L', 'R'):
                    print("Direcao deve ser L ou R")
                    continue
                trans[(q, s)] = (prox, write, dirc)
                break
    return trans


def simular(transicoes, estado_inicial, estados_finais, fita_entrada, max_passos=10000, verbose=True):
    fita = list(fita_entrada)
    if len(fita) == 0 or fita[-1] != '_':
        fita.append('_')
    cabeca = 0
    estado = estado_inicial
    passos = 0
    n = len(fita)

    while True:
        passos += 1
        if passos > max_passos:
            if verbose:
                print("Parada forcada: limite de passos atingido")
            return False, ''.join(fita)

        if cabeca < 0:
            fita.insert(0, '_')
            cabeca = 0
        elif cabeca >= len(fita):
            fita.append('_')

        simbolo = fita[cabeca]
        chave = (estado, simbolo)
        if chave not in transicoes:
            # sem transicao -> rejeita
            if verbose:
                print(f"Sem transicao para (q{estado}, {simbolo}) -> REJEITA")
            return False, ''.join(fita)

        prox, escrito, dirc = transicoes[chave]
        fita[cabeca] = escrito
        estado = prox
        if dirc == 'R':
            cabeca += 1
        else:
            cabeca -= 1

        if verbose:
            print('Fita:', ''.join(fita), 'Cabeca:', cabeca, 'Estado: q' + str(estado))

        if 0 <= estado < len(estados_finais) and estados_finais[estado]:
            if verbose:
                print('ACEITA!')
            return True, ''.join(fita)


def main():
    print('--- Simulador interativo de Maquina de Turing ---')
    alfabeto = le_alfabeto()
    n = le_num_estados()
    finais = le_estados_finais(n)
    inicial = le_estado_inicial(n)
    trans = definir_transicoes(n, alfabeto)

    # Escolha da fita
    print('\nEscolha da fita:')
    print('1 - Exemplo: fita aceita (0101)')
    print('0 - Exemplo: fita rejeitada (012)')
    print('2 - Digitar fita manual')
    escolha = None
    while escolha not in ('0', '1', '2'):
        escolha = input('Escolha 1, 0 ou 2: ').strip()

    if escolha == '1':
        fita = '0101'
    elif escolha == '0':
        fita = '012'
    else:
        fita = input('Digite a fita (simbolos seguidos, ex: 0101): ').strip()

    print('\n--- Iniciando simulacao ---')
    aceitou, fita_final = simular(trans, inicial, finais, fita, verbose=True)
    print('\nResultado final da fita:', fita_final)
    print('Resultado da simulacao:', 'ACEITA' if aceitou else 'REJEITADA')


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nExecucao interrompida pelo usuario')
        sys.exit(0)
 

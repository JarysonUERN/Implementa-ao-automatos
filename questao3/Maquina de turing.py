# ======= DESCRIÇÃO FORMAL DA MÁQUINA DE TURING =======
#
# (
#     {q0, q_accept, q_reject},              # conjunto de estados
#     {0, 1},                                # alfabeto da máquina (símbolos da entrada)
#     {0, 1, _},                              # alfabeto da fita
#     {
#         (q0, 0) -> (q0, 1, R),
#         (q0, 1) -> (q0, 0, R),
#         (q0, _) -> (q_accept, _, R)
#     },
#     q0,                                     # estado inicial
#     {q_accept}                              # conjunto de estados finais
# )
#
# OBS: A descrição acima segue o formato exigido:
# ({ESTADOS}, {ALFABETO MÁQUINA}, {ALFABETO FITA}, {TRANSIÇÕES}, ESTADO_INICIAL, {ESTADOS_FINAIS})
# ======================================================


# ======= IMPLEMENTAÇÃO DA MÁQUINA DE TURING =======

import sys


# Função que executa a simulação da Máquina de Turing para uma entrada dada
def simular_maquina(entrada, max_passos=1000, verbose=True):
    # Prepara fita e cabeça
    fita = list(entrada)
    # coloca um blank ao final para que a TM encontre '_' quando avançar
    if len(fita) == 0 or fita[-1] != '_':
        fita.append('_')
    cabeca = 0

    # Estados
    estado_inicial = "q0"
    estado_aceitacao = "q_accept"
    estado_rejeicao = "q_reject"
    estado_atual = estado_inicial

    # Função de transições (mesma da especificação)
    transicoes = {
        ("q0", "0"): ("q0", "1", "R"),
        ("q0", "1"): ("q0", "0", "R"),
        ("q0", "_"): ("q_accept", "_", "R"),
    }

    passos = 0

    while True:
        passos += 1
        if passos > max_passos:
            if verbose:
                print("Parada forçada: limite de passos atingido")
            return False, ''.join(fita)

        # garante que a posição da cabeça existe na fita
        if cabeca < 0:
            fita.insert(0, '_')
            cabeca = 0
        elif cabeca >= len(fita):
            fita.append('_')

        simbolo = fita[cabeca]

        if (estado_atual, simbolo) not in transicoes:
            estado_atual = estado_rejeicao
            if verbose:
                print("Rejeita! Transicao ausente para (", estado_atual, ",", simbolo, ")")
            return False, ''.join(fita)

        proximo_estado, simbolo_escrito, direcao = transicoes[(estado_atual, simbolo)]

        # Escreve na fita
        fita[cabeca] = simbolo_escrito
        estado_atual = proximo_estado

        # Move a cabeça
        if direcao == "R":
            cabeca += 1
        elif direcao == "L":
            cabeca -= 1

        if verbose:
            print("Fita:", "".join(fita), "Cabeça:", cabeca, "Estado:", estado_atual)

        # Verifica estado final
        if estado_atual == estado_aceitacao:
            if verbose:
                print("Aceita! Resultado final da fita:", "".join(fita))
            return True, ''.join(fita)
        elif estado_atual == estado_rejeicao:
            if verbose:
                print("Rejeita! Resultado parcial da fita:", "".join(fita))
            return False, ''.join(fita)


def main():
    print("Simulador de Máquina de Turing - escolha 1 (aceita) ou 0 (rejeita)")
    escolha = None
    while escolha not in ('0', '1'):
        escolha = input("Digite 1 para simular uma cadeia aceita ou 0 para simular uma cadeia rejeitada: ").strip()

    # Cadeias de exemplo
    # - cadeia aceita: usando apenas 0/1 a TM percorre e, ao encontrar '_' aceita
    cadeia_aceita = "0101"
    # - cadeia rejeitada: inclui um símbolo desconhecido '2' que não possui transições
    cadeia_rejeitada = "012"

    if escolha == '1':
        print("\nSimulando cadeia que deve ser aceita:", cadeia_aceita)
        aceitou, fita_final = simular_maquina(cadeia_aceita, verbose=True)
        print("Resultado: {}\n".format("ACEITA" if aceitou else "REJEITADA"))
    else:
        print("\nSimulando cadeia que deve ser rejeitada:", cadeia_rejeitada)
        aceitou, fita_final = simular_maquina(cadeia_rejeitada, verbose=True)
        print("Resultado: {}\n".format("ACEITA" if aceitou else "REJEITADA"))


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\nExecucao interrompida pelo usuario")
        sys.exit(0)

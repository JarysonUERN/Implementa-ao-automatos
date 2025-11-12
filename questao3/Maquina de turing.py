# ======= Definição da Máquina de Turing =======

# Fita inicial (entrada)
entrada = "0101"
fita = list(entrada)
cabeca = 0  # posição inicial da cabeça

# Estados
estado_inicial = "q0"
estado_aceitacao = "q_accept"
estado_rejeicao = "q_reject"
estado_atual = estado_inicial

# Função de transição: (estado_atual, simbolo_lido) -> (proximo_estado, simbolo_escrito, direcao)
transicoes = {
    ("q0", "0"): ("q0", "1", "R"),
    ("q0", "1"): ("q0", "0", "R"),
    ("q0", "_"): ("q_accept", "_", "R"),  # fim da fita, aceita
}

# ======= Simulação da Máquina de Turing =======

max_passos = 1000
passos = 0

while True:
    passos += 1
    if passos > max_passos:
        print("Parada forçada: limite de passos atingido")
        break

    simbolo = fita[cabeca]

    if (estado_atual, simbolo) not in transicoes:
        estado_atual = estado_rejeicao
        break

    proximo_estado, simbolo_escrito, direcao = transicoes[(estado_atual, simbolo)]

    # Escreve na fita
    fita[cabeca] = simbolo_escrito
    estado_atual = proximo_estado

    # Move a cabeça
    if direcao == "R":
        cabeca += 1
    elif direcao == "L":
        cabeca -= 1

    # Expansão dinâmica da fita
    if cabeca < 0:
        fita.insert(0, "_")
        cabeca = 0
    elif cabeca >= len(fita):
        fita.append("_")

    # Visualização passo a passo (opcional)
    print("Fita:", "".join(fita), "Cabeça:", cabeca, "Estado:", estado_atual)

    # Verifica estado final
    if estado_atual == estado_aceitacao:
        print("Aceita! Resultado final da fita:", "".join(fita))
        break
    elif estado_atual == estado_rejeicao:
        print("Rejeita! Resultado parcial da fita:", "".join(fita))
        break

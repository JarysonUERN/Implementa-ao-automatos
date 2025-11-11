import os
import json
import argparse
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches # Para criar a legenda


def visualizar_afd(caminho_json=None):
    """Carrega um AFD a partir de um arquivo JSON e o desenha com networkx/matplotlib."""
    if caminho_json is None:
        base_dir = os.path.dirname(os.path.abspath(__file__))
        caminho_json = os.path.abspath(os.path.join(base_dir, "..", "output", "AFDconvertido.JSON"))

    # Normaliza path e checa existência
    caminho_json = os.path.normpath(caminho_json)
    if not os.path.exists(caminho_json):
        print(f"Arquivo não encontrado: {caminho_json}")
        return

    # Lê o arquivo JSON gerado pelo C
    try:
        with open(caminho_json, "r", encoding="utf-8") as f:
            afd = json.load(f)
    except Exception as e:
        print(f"Erro ao ler/parsear JSON: {e}")
        return

    # Validação mínima do JSON esperado
    for chave in ("estados", "alfabeto", "transicoes", "estado_inicial", "finais"):
        if chave not in afd:
            print(f"JSON inválido: falta a chave '{chave}'")
            return

    estados = afd["estados"]
    alfabeto = afd["alfabeto"]
    transicoes = afd["transicoes"]
    estado_inicial = afd["estado_inicial"]
    finais = afd["finais"]

    # Cria um grafo direcionado
    G = nx.DiGraph()

    # Adiciona os nós
    for e in estados:
        G.add_node(e)

    # Adiciona as transições (arestas com rótulos)
    for t in transicoes:
        origem = t.get("de")
        destino = t.get("para")
        simbolo = t.get("simbolo")
        if origem is None or destino is None or simbolo is None:
            continue
        if G.has_edge(origem, destino):
            # Se já existe uma transição, concatena símbolos
            G[origem][destino]["label"] += f",{simbolo}"
        else:
            G.add_edge(origem, destino, label=str(simbolo))

    # Define posição dos nós (automático)
    pos = nx.spring_layout(G, seed=42)

    # Prepara a figura
    plt.figure(figsize=(10, 7))

    # --- NOVO: Adiciona a seta de entrada "atrás" do nó ---
    
    # Pega a coordenada exata do nó inicial
    initial_node_pos = pos[estado_inicial]
    
    # Define um ponto de início para a seta (um pouco à esquerda)
    # Ajuste o valor -0.4 para mover a seta mais para longe ou perto
    arrow_start_point = (initial_node_pos[0] - 0.4, initial_node_pos[1])

    # Adiciona a anotação (a seta)
    plt.annotate(
        "",  # Sem texto
        xy=initial_node_pos,  # Ponto de destino (o nó)
        xycoords='data',
        xytext=arrow_start_point,  # Ponto de origem (de onde a seta vem)
        textcoords='data',
        arrowprops=dict(
        arrowstyle="->",
        color="black",
        lw=5,
        mutation_scale=20,
        shrinkA=20,
        shrinkB=20,
        connectionstyle="arc3,rad=0"
        ),
        zorder=1 
    )
    

    # --- LÓGICA DE VISUALIZAÇÃO (COR/BORDA) ---
    node_colors = []
    node_borders = []
    border_widths = []

    for n in G.nodes():
        is_initial = (n == estado_inicial)
        is_final = (n in finais)

        if is_initial:
            node_colors.append("yellow")
        else:
            node_colors.append("lightblue")

        if is_final:
            node_borders.append("black")
            border_widths.append(3.0)   # Borda grossa
        else:
            node_borders.append("black")
            border_widths.append(1.0)   # Borda normal

    # Desenha o grafo (os nós ficarão por cima da seta por causa do zorder=2 padrão)
    nx.draw(
        G, pos, with_labels=True, node_size=2000,
        node_color=node_colors,
        edgecolors=node_borders,
        linewidths=border_widths,
        font_size=10, font_weight="bold", arrowsize=20, connectionstyle="arc3,rad=0.1"
    )

    # Desenha rótulos das arestas
    edge_labels = nx.get_edge_attributes(G, "label")
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=9)

    # Adiciona a legenda
    normal_patch = mpatches.Patch(color='lightblue', ec='black', lw=1, label='Estado')
    initial_patch = mpatches.Patch(color='yellow', ec='black', lw=1, label='Estado Inicial (Preenchimento)')
    final_patch = mpatches.Patch(color='lightblue', ec='black', lw=3, label='Estado Final (Borda Grossa)')
    both_patch = mpatches.Patch(color='yellow', ec='black', lw=3, label='Inicial e Final')

    plt.legend(handles=[normal_patch, initial_patch, final_patch, both_patch], loc='best')

    plt.title("Autômato Finito Determinístico (AFD)", fontsize=14)
    plt.axis("off")
    plt.show()


def main(argv=None):
    parser = argparse.ArgumentParser(description="Visualizar AFD a partir de um arquivo JSON gerado pelo programa C.")
    parser.add_argument("caminho", nargs="?", help="Caminho para o arquivo afd.json (opcional)")
    args = parser.parse_args(argv)
    visualizar_afd(args.caminho)


if __name__ == "__main__":
    main()
import os
import sys
import json
import argparse
import networkx as nx
import matplotlib.pyplot as plt


def visualizar_afd(caminho_json=None):
    """Carrega um AFD a partir de um arquivo JSON e o desenha com networkx/matplotlib.

    Se caminho_json for None, usa ../output/afd.json relativo a este arquivo.
    """
    if caminho_json is None:
        base_dir = os.path.dirname(os.path.abspath(__file__))
        caminho_json = os.path.abspath(os.path.join(base_dir, "..", "output", "afd.json"))

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

    # Adiciona os nós (garante mesmo tipo usado nas transições)
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

    # Desenha o grafo
    plt.figure(figsize=(8, 6))
    node_colors = [
        ("yellow" if n == estado_inicial else ("lightgreen" if n in finais else "lightblue"))
        for n in G.nodes()
    ]

    nx.draw(
        G, pos, with_labels=True, node_size=2000, node_color=node_colors,
        font_size=10, font_weight="bold", arrowsize=20, connectionstyle="arc3,rad=0.1"
    )

    # Desenha rótulos das arestas (símbolos do alfabeto)
    edge_labels = nx.get_edge_attributes(G, "label")
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=9)

    # Opcional: destaca estado inicial e finais (legenda já visual via cores)
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

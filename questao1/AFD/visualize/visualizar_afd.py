import json
import networkx as nx
import matplotlib.pyplot as plt

def visualizar_afd(caminho_json="afd.json"):
    # Lê o arquivo JSON gerado pelo C
    with open(caminho_json, "r") as f:
        afd = json.load(f)

    # Cria um grafo direcionado
    G = nx.DiGraph()

    estados = afd["estados"]
    alfabeto = afd["alfabeto"]
    transicoes = afd["transicoes"]
    estado_inicial = afd["estado_inicial"]
    finais = afd["finais"]

    # Adiciona os nós
    for e in estados:
        G.add_node(e)

    # Adiciona as transições (arestas com rótulos)
    for t in transicoes:
        origem = t["de"]
        destino = t["para"]
        simbolo = t["simbolo"]
        if G.has_edge(origem, destino):
            # Se já existe uma transição, concatena símbolos (para AFN)
            G[origem][destino]["label"] += f",{simbolo}"
        else:
            G.add_edge(origem, destino, label=simbolo)

    # Define posição dos nós (automático)
    pos = nx.spring_layout(G, seed=42)

    # Desenha o grafo
    plt.figure(figsize=(8, 6))
    nx.draw(
        G, pos, with_labels=True, node_size=2000, node_color="lightblue",
        font_size=10, font_weight="bold", arrowsize=20, connectionstyle="arc3,rad=0.1"
    )

    # Desenha rótulos das arestas (símbolos do alfabeto)
    edge_labels = nx.get_edge_attributes(G, "label")
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=9)

    # Destaca o estado inicial
    nx.draw_networkx_nodes(G, pos, nodelist=[estado_inicial], node_color="yellow", node_size=2500)

    # Destaca estados finais
    nx.draw_networkx_nodes(G, pos, nodelist=finais, node_color="lightgreen", node_size=2500)

    plt.title("Autômato Finito Determinístico (AFD)", fontsize=14)
    plt.axis("off")
    plt.show()

if __name__ == "__main__":
    visualizar_afd()

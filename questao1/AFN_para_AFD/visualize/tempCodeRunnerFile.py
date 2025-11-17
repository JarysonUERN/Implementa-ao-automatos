
    # Desenha o grafo (os nós ficarão por cima da seta por causa do zorder=2 padrão)
    nx.draw(
        G, pos, with_labels=True, node_size=2000,
        node_color=node_colors,
        edgecolors=node_borders,
        linewidths=border_widths,
        font_size=10, font_weight="bold", arrowsize=20, connectionstyle="arc3,rad=0.1"
    )
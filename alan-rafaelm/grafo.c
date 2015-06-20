#include <stdlib.h>
#include <cgraph.h>
#include "grafo.h"

struct lista {
  struct no *primeiro;
};

struct no {
  void *conteudo;
  struct no *proximo;
};

struct vertice {
  char *nome;
  lista vertice_lista;
};

struct aresta {
  unsigned int destino;
};

struct grafo {
  char *nome;
  int direcionado;
  int ponderado;
  unsigned int n_vertices;
  vertice vertices;
};

//------------------------------------------------------------------------------
no primeiro_no(lista l) {
  return l->primeiro;
}

//------------------------------------------------------------------------------
no proximo_no(no n) {
  return n->proximo;
}

//------------------------------------------------------------------------------
void *conteudo(no n) {
  return n->conteudo;
}

//------------------------------------------------------------------------------
int _destroi(void *p) {
  if(p != NULL) {
    free(p);
    return 0;
  }

  return 1;
}

//------------------------------------------------------------------------------
int destroi_lista(lista l, int destroi(void *)) {
  if(!l) {
    return 0;
  }

  no n, prox;

  for(n = primeiro_no(l); n != NULL; n = prox) {
    prox = proximo_no(n);

    if(destroi)
      destroi(conteudo(n));

    free(n);
  }

  free(l);
  return 1;
}

//------------------------------------------------------------------------------
char *nome_vertice(vertice v) {
  return v->nome;
}

//------------------------------------------------------------------------------
unsigned int encontra_vertice_indice(struct vertice *vertices, unsigned int n_vertices, const char *nome) {
  unsigned int i;

  /* Percorre todos os vértices da estrutura */
  for(i = 0; i < n_vertices; ++i) {
    /* Se o nome do vértice é igual ao desejado, então o retorna */
    if(strcmp(vertices[i].nome, nome) == 0) {
      return i;
    }
  }

  return -1;
}

//------------------------------------------------------------------------------
grafo le_grafo(FILE *input) {
  Agraph_t *g;
  Agnode_t *v;
  Agedge_t *e;
  struct grafo *grafo_lido;
  struct no *n;
  struct aresta *a;
  unsigned int i;

  /* Aloca a estrutura do grafo */
  grafo_lido = (struct grafo *) malloc(sizeof(struct grafo));

  if(grafo_lido != NULL) {
    /* Inicializa a estrutura do grafo */
    grafo_lido->nome = (char *) NULL;
    grafo_lido->vertices = (struct vertice *) NULL;

    /* Armazena em g o grafo lido da entrada */
    if((g = agread(input, NULL)) == NULL) {
      destroi_grafo(grafo_lido);
      return NULL;
    }

    /* Define o nome do grafo e se ele é direcionado */
    grafo_lido->direcionado = agisdirected(g);
    grafo_lido->nome = strdup(agnameof(g));
    grafo_lido->n_vertices = agnnodes(g);

    /* Aloca a quantidade de memória necessária para armazenar todos os vértices */
    grafo_lido->vertices = (struct vertice *) malloc(sizeof(struct vertice) * grafo_lido->n_vertices);

    if(grafo_lido->vertices != NULL) {
      /* Percorre todos os vértices do grafo */
      for(i = 0, v = agfstnode(g); i < grafo_lido->n_vertices; ++i, v = agnxtnode(g, v)) {
        /* Duplica na memória o nome do vértice e o atribui na estrutura.
           A duplicação é feita para evitar erros (por exemplo, se o espaço for desalocado) */
        grafo_lido->vertices[i].nome = strdup(agnameof(v));

        /* Aloca a lista de adjacência do vértice */
        grafo_lido->vertices[i].vertice_lista = (struct lista *) malloc(sizeof(struct lista));

        if(grafo_lido->vertices[i].vertice_lista != NULL) {
          grafo_lido->vertices[i].vertice_lista->primeiro = NULL;
        }
      }

      /* Percorre todos os vértices do grafo */
      for(i = 0, v = agfstnode(g); i < grafo_lido->n_vertices; ++i, v = agnxtnode(g, v)) {
        if(grafo_lido->vertices[i].vertice_lista != NULL) {
          /* Percorre todas as arestas adjacentes do vértice */
          for(e = agfstedge(g, v); e != NULL; e = agnxtedge(g, e, v)) {
            /* Aloca o novo nó da aresta */
            n = (struct no *) malloc(sizeof(struct no));

            /* Adiciona o nó da aresta na lista de adjacência do vértice */
            if(n != NULL) {
              n->proximo = grafo_lido->vertices[i].vertice_lista->primeiro;
              n->conteudo = malloc(sizeof(struct aresta));

              if(n->conteudo != NULL) {
                a = (struct aresta *) n->conteudo;

                /* Define o destino da aresta */
                if(strcmp(grafo_lido->vertices[i].nome, agnameof(aghead(e))) == 0) {
                  a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(agtail(e)));
                } else {
                  a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(aghead(e)));
                }
              }

              grafo_lido->vertices[i].vertice_lista->primeiro = n;
            }
          }
        }
      }
    }

    agclose(g);
  }

  return grafo_lido;
}

//------------------------------------------------------------------------------
int destroi_grafo(void *g) {
  struct grafo *g_ptr;

  g_ptr = (grafo) g;

  if(g_ptr != NULL) {
    /* Libera a região de memória ocupada pelo nome do grafo, se não for nula */
    if(g_ptr->nome != NULL) {
      free(g_ptr->nome);
    }

    /* Libera a região de memória ocupada pelos vértices do grafo, se não for nula */
    if(g_ptr->vertices != NULL) {
      unsigned int i;

      for(i = 0; i < g_ptr->n_vertices; ++i) {
        if(g_ptr->vertices[i].nome != NULL) {
          free(g_ptr->vertices[i].nome);
        }

        if(g_ptr->vertices[i].vertice_lista != NULL) {
          destroi_lista(g_ptr->vertices[i].vertice_lista, _destroi);
        }
      }

      free(g_ptr->vertices);
    }

    /* Libera a região de memória ocupada pela estrutura do grafo */
    free(g_ptr);
  }

  return 1;
}

//------------------------------------------------------------------------------
grafo escreve_grafo(FILE *output, grafo g) {
  struct no *n;
  struct aresta *a;
  char caractere_aresta;
  unsigned int i;

  /* Imprime na saida a definição do grafo, caso seja um grafo direcionado,
     é adicionado o prefixo "di" */
  fprintf(output, "strict %sgraph \"%s\" {\n\n", (g->direcionado) ? "di" : "", g->nome);

  /* Imprime os nomes dos vértices */
  for(i = 0; i < g->n_vertices; ++i) {
    fprintf(output, "    \"%s\"\n", g->vertices[i].nome);
  }

  fprintf(output, "\n");

  /* Se o grafo é direcionado, representamos as arestas por v -> u,
     sendo v o vértice de origem e u o vértice de destino de cada aresta.
     Caso contrário, representamos as arestas por v -- u */
  caractere_aresta = (g->direcionado) ? '>' : '-';

  /* Imprime as arestas */
  for(i = 0; i < g->n_vertices; ++i) {
    for(n = primeiro_no(g->vertices[i].vertice_lista); n != NULL; n = proximo_no(n)) {
      a = (struct aresta *) n->conteudo;

      if(g->direcionado || i < a->destino) {
        fprintf(output, "    \"%s\" -%c \"%s\"\n", g->vertices[i].nome, caractere_aresta, g->vertices[a->destino].nome);
      }
    }
  }

  fprintf(output, "}\n");
  return g;
}

//------------------------------------------------------------------------------
char *nome(grafo g) {
  return g->nome;
}

//------------------------------------------------------------------------------
unsigned int n_vertices(grafo g) {
  return g->n_vertices;
}

//------------------------------------------------------------------------------
int direcionado(grafo g) {
  return g->direcionado;
}

//------------------------------------------------------------------------------
int conexo(grafo g) {
  if(g->direcionado) {
    return 0;
  }

  return 0;
}

//------------------------------------------------------------------------------
grafo arvore_geradora_minima(grafo g) {
  if(g->direcionado) {
    return NULL;
  }

  return g;
}

//------------------------------------------------------------------------------
lista componentes(grafo g) {
  return NULL;
}

//------------------------------------------------------------------------------
lista blocos(grafo g) {
  if(g->direcionado) {
    return NULL;
  }

  return NULL;
}

//------------------------------------------------------------------------------
lista ordena(grafo g) {
  if(!g->direcionado) {
    return NULL;
  }

  return NULL;
}

//------------------------------------------------------------------------------
grafo arborescencia_caminhos_minimos(grafo g, vertice r) {
  return NULL;
}

//------------------------------------------------------------------------------
grafo distancias(grafo g) {
  return NULL;
}

//------------------------------------------------------------------------------
int fortemente_conexo(grafo g) {
  return 0;
}

//------------------------------------------------------------------------------
long int diametro(grafo g) {
  return 0;
}

//------------------------------------------------------------------------------
int main(void) {
  return ! destroi_grafo(escreve_grafo(stdout, le_grafo(stdin)));
}

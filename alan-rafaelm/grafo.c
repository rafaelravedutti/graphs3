#include <stdlib.h>
#include <limits.h>
#include <graphviz/cgraph.h>
#include "string.h"
#include "grafo.h"

struct lista {
  struct no *primeiro;
  unsigned int ref_count;
};

struct no {
  void *conteudo;
  struct no *proximo;
};

struct vertice {
  char *nome;
  lista arestas;
};

struct aresta {
  unsigned int origem;
  unsigned int destino;
  long int peso;
};

struct grafo {
  char *nome;
  int direcionado;
  int ponderado;
  vertice vertices;
  unsigned int n_vertices;
  unsigned int ref_count;
};

const long int infinito = LONG_MAX;

//------------------------------------------------------------------------------
void inicializa_lista(lista *l) {
  /* Aloca espaço pra lista */
  *l = (struct lista *) malloc(sizeof(struct lista));
  /* Inicia lista vazia */
  if(*l != NULL) {
    (*l)->primeiro = NULL;
  }
}

//------------------------------------------------------------------------------
void insere_cabeca(lista l, no n) {
  n->proximo = l->primeiro;
  l->primeiro = n;
}

//------------------------------------------------------------------------------
void insere_cabeca_conteudo(lista l, void *conteudo) {
  struct no *n;

  n = (struct no *) malloc(sizeof(struct no));

  if(n != NULL) {
    n->conteudo = conteudo;
    insere_cabeca(l, n);
  }
}

//------------------------------------------------------------------------------
int lista_contem(lista l, void *conteudo) {
  struct no *n;

  for(n = l->primeiro; n != NULL; n = n->proximo) {
    if(n->conteudo == conteudo) {
      return 1;
    }
  }

  return 0;
}

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
int _mantem(void *p) {
  return 1;
}

//------------------------------------------------------------------------------
int destroi_lista(lista l, int destroi(void *)) {
  struct no *n, *prox;

  if(l == NULL) {
    return 0;
  }

  if(l->ref_count > 1) {
    --l->ref_count;
    return 1;
  }

  for(n = l->primeiro; n != NULL; n = prox) {
    if(destroi) {
      destroi(conteudo(n));
    }

    prox = n->proximo;
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
  struct aresta *a;
  char *peso;
  char peso_string[] = "peso";
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

    /* Verifica se g é um grafo ponderado ou não */
    if(agattr(g, AGEDGE, peso_string, (char *) NULL) != NULL) {
      grafo_lido->ponderado = 1;
    } else {
      grafo_lido->ponderado = 0;
    }

    /* Aloca a quantidade de memória necessária para armazenar todos os vértices */
    grafo_lido->vertices = (struct vertice *) malloc(sizeof(struct vertice) * grafo_lido->n_vertices);

    /* Contador de ponteiros que referênciam a região */
    grafo_lido->ref_count = 1;

    if(grafo_lido->vertices != NULL) {
      /* Percorre todos os vértices do grafo */
      for(i = 0, v = agfstnode(g); i < grafo_lido->n_vertices; ++i, v = agnxtnode(g, v)) {
        /* Duplica na memória o nome do vértice e o atribui na estrutura.
           A duplicação é feita para evitar erros (por exemplo, se o espaço for desalocado) */
        grafo_lido->vertices[i].nome = strdup(agnameof(v));

        /* Aloca a lista de adjacência do vértice */
        grafo_lido->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));

        if(grafo_lido->vertices[i].arestas != NULL) {
          grafo_lido->vertices[i].arestas->primeiro = NULL;
          grafo_lido->vertices[i].arestas->ref_count = 1;
        }
      }

      /* Percorre todos os vértices do grafo */
      for(v = agfstnode(g); v != NULL; v = agnxtnode(g, v)) {
        i = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(v));

        if(grafo_lido->vertices[i].arestas != NULL) {
          /* Percorre todas as arestas adjacentes do vértice */
          for(e = agfstedge(g, v); e != NULL; e = agnxtedge(g, e, v)) {
            peso = agget(e, "peso");

            if(!grafo_lido->direcionado) {
              a = (struct aresta *) malloc(sizeof(struct aresta));
              a->peso = (peso != NULL && *peso != '\0') ? atoi(peso) : 1;
              a->origem = i;

              if(v == aghead(e)) {
                a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(agtail(e)));
              } else {
                a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(aghead(e)));
              }

              insere_cabeca_conteudo(grafo_lido->vertices[i].arestas, a);
            } else {
              if(v == agtail(e)) {
                a = (struct aresta *) malloc(sizeof(struct aresta));
                a->peso = (peso != NULL && *peso != '\0') ? atoi(peso) : 1;
                a->origem = i;
                a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(aghead(e)));
                insere_cabeca_conteudo(grafo_lido->vertices[i].arestas, a);
                insere_cabeca_conteudo(grafo_lido->vertices[a->destino].arestas, a);
              }
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
    if(g_ptr->ref_count > 1) {
      --g_ptr->ref_count;
    } else {
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

          if(g_ptr->vertices[i].arestas != NULL) {
            destroi_lista(g_ptr->vertices[i].arestas, _destroi);
          }
        }

        free(g_ptr->vertices);
      }

      /* Libera a região de memória ocupada pela estrutura do grafo */
      free(g_ptr);
    }
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
    for(n = primeiro_no(g->vertices[i].arestas); n != NULL; n = proximo_no(n)) {
      a = (struct aresta *) n->conteudo;

      if((g->direcionado && a->origem == i) || (!g->direcionado && a->origem < a->destino)) {
        fprintf(output, "    \"%s\" -%c \"%s\"", g->vertices[a->origem].nome, caractere_aresta, g->vertices[a->destino].nome);

        /* Se g é um grafo ponderado, imprime o peso da aresta */
        if(g->ponderado == 1) {
          if(a->peso == infinito) {
            fprintf(output, " [peso=oo]");
          } else {
            fprintf(output, " [peso=%ld]", a->peso);
          }
        }

        fprintf(output, "\n");
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
  struct lista *comp;
  int retorno;

  retorno = 0;

  if(g->direcionado) {
    return 0;
  }

  comp = componentes(g);

  if(comp->primeiro != NULL && comp->primeiro->proximo == NULL) {
    retorno = 1;
  }

  destroi_lista(comp, destroi_grafo);
  return retorno;
}

//------------------------------------------------------------------------------
grafo arvore_geradora_minima(grafo g) {
  if(g->direcionado) {
    return NULL;
  }

  return g;
}

//------------------------------------------------------------------------------
void _gera_componente(grafo g, lista vertices_componente, unsigned int *n_vertices_componente, unsigned int r) {
  struct no *n;
  struct aresta *a;

  if(!lista_contem(vertices_componente, g->vertices + r)) {
    insere_cabeca_conteudo(vertices_componente, g->vertices + r);

    for(n = g->vertices[r].arestas->primeiro; n != NULL; n = n->proximo) {
      a = (struct aresta *) n->conteudo;
      _gera_componente(g, vertices_componente, n_vertices_componente, a->destino);
    }

    ++(*n_vertices_componente);
  }
}

//------------------------------------------------------------------------------
grafo gera_componente(grafo g, unsigned int r) {
  struct grafo *componente;
  struct lista *vertices_componente;
  struct vertice *v;
  struct aresta *a, *aresta_componente;
  struct no *n, *no_arestas;
  unsigned int n_vertices_componente = 1, count = 0;

  inicializa_lista(&vertices_componente);
  insere_cabeca_conteudo(vertices_componente, g->vertices + r);

  /* Realiza uma busca adicionando todos os vértices do componente na lista */
  for(n = g->vertices[r].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;
    _gera_componente(g, vertices_componente, &n_vertices_componente, a->destino);
  }

  componente = (struct grafo *) malloc(sizeof(struct grafo));

  if(componente != NULL) {
    /* Inicializa a estrutura de grafo para retornar o componente conexo*/
    componente->nome = (char *) NULL;
    componente->direcionado = g->direcionado;
    componente->ponderado = g->ponderado;
    componente->n_vertices = n_vertices_componente;

    componente->vertices = (struct vertice *) malloc(sizeof(struct vertice) * n_vertices_componente);

    if(componente->vertices != NULL) {
      for(n = vertices_componente->primeiro, count = 0; n != NULL; n = n->proximo, ++count) {
        v = (struct vertice *) n->conteudo;

        componente->vertices[count].nome = strdup(v->nome);
        componente->vertices[count].arestas = (struct lista *) malloc(sizeof(struct lista));
        componente->vertices[count].arestas->primeiro = NULL;
      }

      for(n = vertices_componente->primeiro, count = 0; n != NULL; n = n->proximo, ++count) {
        v = (struct vertice *) n->conteudo;

        for(no_arestas = v->arestas->primeiro; no_arestas != NULL; no_arestas = no_arestas->proximo) {
          a = (struct aresta *) no_arestas->conteudo;
          aresta_componente = (struct aresta *) malloc(sizeof(struct aresta));

          if(aresta_componente != NULL) {
            aresta_componente->origem = count;
            aresta_componente->destino = encontra_vertice_indice(componente->vertices, componente->n_vertices, g->vertices[a->destino].nome);
            insere_cabeca_conteudo(componente->vertices[count].arestas, aresta_componente);
          }
        }
      }
    }
  }

  destroi_lista(vertices_componente, _mantem);
  return componente;
}

//------------------------------------------------------------------------------
lista componentes(grafo g) {
  struct lista *lista_componentes;
  struct grafo *componente;
  unsigned int i, v, vertices_processados;
  unsigned int *vertice_processado;

  /* Inicializa a lista de componentes */
  inicializa_lista(&lista_componentes);

  vertice_processado = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

  if(vertice_processado != NULL) {
    for(i = 0; i < g->n_vertices; ++i) {
      vertice_processado[i] = 0;
    }

    vertices_processados = 0;

    /* Enquanto não forem processados todos os vértices de G */
    while(vertices_processados < g->n_vertices) {
      /* Procura um vértice em G ainda não processado */
      for(v = 0; v < g->n_vertices && vertice_processado[v] == 1; ++v);

      /* Gera o componente a qual pertence o vértice encontrado */
      componente = gera_componente(g, v);

      if(componente != NULL) {
        /* Insere o componente na lista */
        insere_cabeca_conteudo(lista_componentes, componente);

        /* Marca como processados todos os vértices do componente */
        for(i = 0; i < componente->n_vertices; ++i) {
          vertice_processado[encontra_vertice_indice(g->vertices, g->n_vertices, componente->vertices[i].nome)] = 1;
          ++vertices_processados;
        }
      }
    }

    free(vertice_processado);
  }

  return lista_componentes;
}

//------------------------------------------------------------------------------
lista blocos(grafo g) {
  if(g->direcionado) {
    return NULL;
  }

  return NULL;
}

//------------------------------------------------------------------------------
void _ordena(grafo g, lista l, unsigned int v, unsigned char *v_processado, unsigned int *v_pai) {
  struct no *n;
  struct aresta *a;

  v_processado[v] = 1;

  for(n = g->vertices[v].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    if(a->destino != v && v_processado[a->destino] == 0) {
      v_pai[a->destino] = v;
      _ordena(g, l, a->destino, v_processado, v_pai);
    }
  }

  insere_cabeca_conteudo(l, g->vertices + v);
}

//------------------------------------------------------------------------------
lista ordena(grafo g) {
  struct lista *l;
  unsigned char *v_processado;
  unsigned int *v_pai;
  unsigned int i;

  if(!g->direcionado) {
    return NULL;
  }

  l = (struct lista *) malloc(sizeof(struct lista));

  if(l != NULL) {
    v_processado = (unsigned char *) malloc(sizeof(unsigned char) * g->n_vertices);
    v_pai = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

    if(v_processado != NULL && v_pai != NULL) {
      l->primeiro = NULL;

      for(i = 0; i < g->n_vertices; ++i) {
        v_processado[i] = 0;
        v_pai[i] = (unsigned int) -1;
      }

      for(i = 0; i < g->n_vertices; ++i) {
        if(!v_processado[i]) {
          v_pai[i] = i;
          _ordena(g, l, i, v_processado, v_pai);
        }
      }

      free(v_processado);
      free(v_pai);
    }
  }

  return l;
}

//------------------------------------------------------------------------------
grafo arborescencia_caminhos_minimos(grafo g, vertice r) {
  struct grafo *t;
  struct aresta *a, *aresta_selecionada;
  struct no *n;
  long int menor_distancia;
  unsigned int i, v;
  unsigned int *vertice_processado, *distancias;

  if((v = encontra_vertice_indice(g->vertices, g->n_vertices, r->nome)) == -1) {
    return NULL;
  }

  t = (struct grafo *) malloc(sizeof(struct grafo));

  if(t != NULL) {
    t->vertices = (struct vertice *) malloc(sizeof(struct vertice) * g->n_vertices);
    vertice_processado = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);
    distancias = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

    if(t->vertices != NULL && vertice_processado != NULL && distancias != NULL) {
      t->nome = NULL;
      t->n_vertices = g->n_vertices;
      t->ponderado = g->ponderado;
      t->direcionado = 1;

      for(i = 0; i < g->n_vertices; ++i) {
        t->vertices[i].nome = strdup(g->vertices[i].nome);
        t->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));
        t->vertices[i].arestas->primeiro = NULL;

        vertice_processado[i] = 0;
      }

      vertice_processado[v] = 1;
      distancias[v] = 0;

      do {
        aresta_selecionada = NULL;
        menor_distancia = infinito;

        for(i = 0; i < g->n_vertices; ++i) {
          if(vertice_processado[i] == 1) {
            for(n = g->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
              a = (struct aresta *) n->conteudo;

              if(vertice_processado[a->destino] == 0) {
                if(menor_distancia > distancias[i] + a->peso) {
                  menor_distancia = distancias[i] + a->peso;
                  aresta_selecionada = a;
                }
              }
            }
          }
        }

        if(aresta_selecionada != NULL) {
          vertice_processado[aresta_selecionada->destino] = 1;
          distancias[aresta_selecionada->destino] = menor_distancia;

          a = (struct aresta *) malloc(sizeof(struct aresta));

          if(a != NULL) {
            a->origem = aresta_selecionada->origem;
            a->destino = aresta_selecionada->destino;
            a->peso = aresta_selecionada->peso;

            insere_cabeca_conteudo(t->vertices[aresta_selecionada->origem].arestas, a);
          }
        }
      } while(aresta_selecionada != NULL);

      free(distancias);
      free(vertice_processado);
    }
  }

  return t;
}

//------------------------------------------------------------------------------
void computa_distancia(struct grafo *dis, struct grafo *acm, unsigned int v, struct no *n, long int d, unsigned int *v_processado) {
  struct no *p;
  struct aresta *a, *aresta_p, *aresta_distancia;

  aresta_distancia = (struct aresta *) malloc(sizeof(struct aresta));

  if(aresta_distancia != NULL) {
    a = (struct aresta *) n->conteudo;

    aresta_distancia->origem = v;
    aresta_distancia->destino = a->destino;
    aresta_distancia->peso = d;

    insere_cabeca_conteudo(dis->vertices[v].arestas, aresta_distancia);

    for(p = acm->vertices[a->destino].arestas->primeiro; p != NULL; p = p->proximo) {
      aresta_p = (struct aresta *) p->conteudo;
      computa_distancia(dis, acm, v, p, d + aresta_p->peso, v_processado);
    }

    v_processado[a->destino] = 1;
  }
}

//------------------------------------------------------------------------------
grafo distancias(grafo g) {
  struct grafo *dis, *acm;
  struct no *n;
  struct aresta *a;
  unsigned int *v_processado;
  unsigned int i, j;

  dis = (struct grafo *) malloc(sizeof(struct grafo));

  if(dis != NULL) {
    dis->vertices = (struct vertice *) malloc(sizeof(struct vertice) * g->n_vertices);
    v_processado = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

    if(dis->vertices != NULL) {
      for(i = 0; i < g->n_vertices; ++i) {
        dis->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));
        dis->vertices[i].arestas->primeiro = NULL;
        dis->vertices[i].nome = strdup(g->vertices[i].nome);
      }

      dis->nome = NULL;
      dis->n_vertices = g->n_vertices;
      dis->direcionado = g->direcionado;
      dis->ponderado = 1;

      for(i = 0; i < g->n_vertices; ++i) {
        acm = arborescencia_caminhos_minimos(g, g->vertices + i);

        for(j = 0; j < g->n_vertices; ++j) {
          v_processado[j] = 0;
        }

        v_processado[i] = 1;

        for(n = acm->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
          a = (struct aresta *) n->conteudo;
          computa_distancia(dis, acm, i, n, a->peso, v_processado);
        }

        for(j = 0; j < g->n_vertices; ++j) {
          if(v_processado[j] == 0) {
            a = (struct aresta *) malloc(sizeof(struct aresta));

            if(a != NULL) {
              a->origem = i;
              a->destino = j;
              a->peso = infinito;

              insere_cabeca_conteudo(dis->vertices[i].arestas, a);
            }
          }
        }

        destroi_grafo(acm);
      }
    }
  }

  return dis;
}

//------------------------------------------------------------------------------
void _busca_profundidade_transposta(grafo g, unsigned int v, unsigned int *t_pre, unsigned int *t_pos, unsigned int *pre, unsigned int *pos) {
  struct no *n;
  struct aresta *a;

  if(t_pre != NULL && pre != NULL) {
    pre[v] = ++(*t_pre);
  }

  for(n = g->vertices[v].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    if(a->origem != v) {
      if(pre[a->origem] == 0) {
        _busca_profundidade_transposta(g, a->origem, t_pre, t_pos, pre, pos);
      }
    }
  }

  if(t_pos != NULL && pos != NULL) {
    pos[v] = ++(*t_pos);
  }
}

//------------------------------------------------------------------------------
void _busca_profundidade(grafo g, unsigned int v, unsigned int *t_pre, unsigned int *t_pos, unsigned int *pre, unsigned int *pos) {
  struct no *n;
  struct aresta *a;

  if(t_pre != NULL && pre != NULL) {
    pre[v] = ++(*t_pre);
  }

  for(n = g->vertices[v].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    if(a->destino != v) {
      if(pre[a->destino] == 0) {
        _busca_profundidade(g, a->destino, t_pre, t_pos, pre, pos);
      }
    }
  }

  if(t_pos != NULL && pos != NULL) {
    pos[v] = ++(*t_pos);
  }
}

//------------------------------------------------------------------------------
void busca_profundidade(grafo g, unsigned int **pre, unsigned int **pos) {
  unsigned int i, t_pre = 0, t_pos = 0;

  *pre = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));
  *pos = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));

  if(*pre != NULL && *pos != NULL) {
    for(i = 0; i < g->n_vertices; ++i) {
      (*pre)[i] = 0;
    }

    for(i = 0; i < g->n_vertices; ++i) {
      if((*pre)[i] == 0) {
        _busca_profundidade(g, i, &t_pre, &t_pos, *pre, *pos);
      }
    }
  }
}

//------------------------------------------------------------------------------
int fortemente_conexo(grafo g) {
  unsigned int *pre, *pos;
  unsigned int i, v, t, max_pos, n_trees;

  busca_profundidade(g, &pre, &pos);

  if(pre != NULL && pos != NULL) {
    t = 0;
    n_trees = 0;

    for(i = 0; i < g->n_vertices; ++i) {
      pre[i] = 0;
    }

    do {
      max_pos = 0;

      for(i = g->n_vertices; i > 0; --i) {
        if(pre[i - 1] == 0 && max_pos < pos[i - 1]) {
          max_pos = pos[i - 1];
          v = i - 1;
        }
      }

      if(max_pos != 0) {
        _busca_profundidade_transposta(g, v, &t, NULL, pre, NULL);
        ++n_trees;
      }
    } while(max_pos != 0);

    free(pre);
    free(pos);
  }

  return (n_trees < 2) ? 1 : 0;
}

//------------------------------------------------------------------------------
long int diametro(grafo g) {
  struct grafo *dis;
  struct no *n;
  struct aresta *a;
  long int diametro = 0;
  unsigned int i;

  dis = distancias(g);

  for(i = 0; i < dis->n_vertices; ++i) {
    for(n = dis->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
      a = (struct aresta *) n->conteudo;

      if(diametro < a->peso && a->peso != infinito) {
        diametro = a->peso;
      }
    }
  }

  destroi_grafo(dis);
  return diametro;
}

//------------------------------------------------------------------------------
int main(void) {
  struct grafo *g, *d, *c;
  struct vertice *v;
  struct no *n;
  lista l;

  g = le_grafo(stdin);
  escreve_grafo(stdout, g);

  if((l = ordena(g)) != NULL) {
    for(n = l->primeiro; n != NULL; n = n->proximo) {
      v = (struct vertice *) n->conteudo;
      fprintf(stdout, "%s\n", v->nome);
    }

    destroi_lista(l, _destroi);
  }

  if((l = componentes(g)) != NULL) {
    for(n = l->primeiro; n != NULL; n = n->proximo) {
      c = (struct grafo *) n->conteudo;
      escreve_grafo(stdout, c);
    }

    destroi_lista(l, destroi_grafo);
  }

  d = arborescencia_caminhos_minimos(g, g->vertices);
  if(d != NULL) {
    escreve_grafo(stdout, d);
    destroi_grafo(d);
  }

  d = distancias(g);
  if(d != NULL) {
    escreve_grafo(stdout, d);
    destroi_grafo(d);
  }

  fprintf(stdout, "Diametro = %ld\n", diametro(g));

  if(fortemente_conexo(g)) {
    fprintf(stdout, "Fortemente conexo!\n");
  } else {
    fprintf(stdout, "Não é fortemente conexo!\n");
  }

  destroi_grafo(g);
  return 0;
}

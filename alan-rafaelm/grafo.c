#include <stdlib.h>
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
  struct aresta *a;
  struct no *n;
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
      for(n = vertices_componente->primeiro; n != NULL; n = n->proximo) {
        v = (struct vertice *) n->conteudo;

        componente->vertices[count].nome = strdup(v->nome);
        componente->vertices[count].arestas = v->arestas;

        ++(v->arestas->ref_count);
        ++count;
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

void insere_cabeca_conteudo(lista l, void *conteudo) ;
no primeiro_no(lista l) ;
no proximo_no(no n) ;
void *conteudo(no n) ;
char *nome_vertice(vertice v) ;
unsigned int encontra_vertice_indice(struct vertice *vertices, unsigned int n_vertices, const char *nome) ;
char *nome(grafo g) ;
unsigned int n_vertices(grafo g) ;
void _gera_bloco(grafo , unsigned int , lista, grafo) ;
lista gera_blocos(grafo , lista) ;
void _vertices_corte(grafo , unsigned int , unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int * ) ;
lista vertices_corte(grafo) ;
lista blocos(grafo) ;




//------------------------------------------------------------------------------
void _gera_bloco(grafo g, unsigned int i, lista vertices_corte, grafo bloco) {
  struct aresta *a;
  struct no *n;

  bloco->vertices[bloco->n_vertices++] = *(g->vertices + i);

  if(!lista_contem(vertices_corte, g->vertices + i)) {
    for(n = primeiro_no(g->vertices[i].arestas); n != NULL; n = proximo_no(n)) {
      a = (struct aresta *) n->conteudo;
      /* se ainda nao está no bloco */
      if(encontra_vertice_indice(bloco->vertices, bloco->n_vertices, g->vertices[a->destino].nome) == -1) {
        _gera_bloco(g, a->destino, vertices_corte, bloco);
      }
    }
  }
}

//-----------------------------------------------------------------------------
lista gera_blocos(grafo g, lista vertices_corte) {
  unsigned int i;
  lista lista_blocos;
  struct grafo *bloco;

  lista_blocos = (lista) malloc(sizeof(struct lista));
  inicializa_lista(&lista_blocos);

  if (vertices_corte == NULL) {
    insere_cabeca_conteudo(lista_blocos, g);
  } else {
    for (i = 0; i < g->n_vertices; ++i) {
      bloco = (struct grafo *) malloc(sizeof(struct grafo));
      bloco->vertices = (struct vertice *) malloc(sizeof(struct vertice) * g->n_vertices);
      bloco->nome = NULL;
      bloco->direcionado = 0;
      bloco->ponderado = g->ponderado;
      bloco->n_vertices = 0;

      _gera_bloco(g, i, vertices_corte, bloco);
      insere_cabeca_conteudo(lista_blocos, bloco);
    }
  }
  return lista_blocos;
}

//-----------------------------------------------------------------------------
void _vertices_corte(grafo g, unsigned int i, unsigned int *n_articulacoes, unsigned int *pre, unsigned int *pai, unsigned int *low, unsigned int *articulacoes, unsigned int *conta ) {
  unsigned int j;
  struct no *n;
  struct aresta *a;
  unsigned int v, i_adicionado, j_adicionado;


  /* Incrementa o contador externoe o atribui a pre[i] */
  pre[i] = ++(*conta);
  /* No começo do laço, lowpoint de i é o próprio i */
  low[i] = pre[i];
  /* Laço para percorrer cada aresta de g->vertices[i] */
  for (n = g->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;
    j = a->destino;
    /* Se a ponta de g->vertices[i] ainda não foi foi incluída na pre-ordem */
    if (pre[j] == -1) {
      /* Indica que o j é um destino de g->vertices[i]*/
      pai[j] = i;
      /* Chama recursivamente a função para computar a pre-ordem filhos de j*/
      _vertices_corte(g, j, n_articulacoes , pre, pai, low, articulacoes, conta);
      /* Se o lowerpoint do filho j é menor, então lowerpoint de i também é o de j */
      if (low[i] > low[j]){
        low[i] = low[j];
      }
      if (low[j] == pre[j]) {
        /* Achou aresta de corte */
        i_adicionado = 0;
        j_adicionado = 0;
        for(v = 0; v < *n_articulacoes && i_adicionado && j_adicionado; ++v){
          if (articulacoes[v] == i) {
            i_adicionado = 1;
          }
          if (articulacoes[v] == j) {
            j_adicionado = 1;
          }
        }
        if (!i_adicionado) {
          articulacoes[(*n_articulacoes)++] = i;
        }
        if (!j_adicionado) {
          articulacoes[(*n_articulacoes)++] = j;
        }
      }
    } else if (j!= pai[i] && low[i] > pre[j]) {
      low[i] = pre[j];
    }
  }
}

//-----------------------------------------------------------------------------
lista vertices_corte(grafo g) {
  unsigned int *pre, *pai, *low, *articulacoes;
  unsigned int i, conta = 0, n_articulacoes = 0;
  vertice v, v_copia;
  lista lista_articulacoes;
  pre = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));
  pai = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));
  low = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));
  articulacoes = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));
  
  lista_articulacoes = (lista) malloc(sizeof(struct lista));
  inicializa_lista(&lista_articulacoes);

  if (pre != NULL && pai != NULL && low != NULL && articulacoes != NULL) {
    for (i = 0; i < g->n_vertices; ++i) {
      pre[i] = -1;
      articulacoes[i] = -1;
    }
    for (i = 0; i < g->n_vertices; ++i) {
      if (pre[i] == -1) {
        pai[i] = i;
        _vertices_corte(g, i, &n_articulacoes, pre, pai, low, articulacoes, &conta);
        conta = 0;
      }
    }
  }

  for (i = 0; i < n_articulacoes; ++i) {
    v = g->vertices + articulacoes[i];
    v_copia = (vertice) malloc (sizeof(struct vertice));
    v_copia->arestas = (lista) malloc(sizeof(struct lista));

    v_copia->nome = strdup(v->nome);
    v_copia->arestas = v->arestas;

    ++(v->arestas->ref_count);

    insere_cabeca_conteudo(lista_articulacoes, v_copia);
  }

  free(pre);
  free(pai);
  free(low);
  free(articulacoes);

  if (n_articulacoes == 0){
    return NULL;
  }
  return lista_articulacoes;
}

//-----------------------------------------------------------------------------
lista blocos(grafo g) {
  if(g->direcionado) {
    return NULL;
  }

  lista vertices_de_corte, lista_blocos;
  vertices_de_corte = vertices_corte(g);
  /* Se a lista está vazia, então g só possui 1 bloco */
  if(vertices_de_corte == NULL){
    lista_blocos = (lista) malloc(sizeof(struct lista));
    inicializa_lista(&lista_blocos);
    insere_cabeca_conteudo(lista_blocos, g);
  } else {
    lista_blocos = gera_blocos(g, vertices_de_corte);
  }
  return lista_blocos;
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
  unsigned int i, v, menor_distancia;
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
      for(i = 0; i < g->n_vertices; ++i) {
        t->vertices[i].nome = strdup(g->vertices[i].nome);
        t->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));
        t->vertices[i].arestas->primeiro = NULL;
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
void computa_distancia(struct grafo *dis, struct grafo *acm, unsigned int v, struct no *n, long int d) {
  struct no *no_distancia, *p;
  struct aresta *a, *aresta_p, *aresta_distancia;

  no_distancia = (struct no *) malloc(sizeof(struct no));

  if(no_distancia != NULL) {
    a = (struct aresta *) n->conteudo;
    aresta_distancia = (struct aresta *) no_distancia->conteudo;

    aresta_distancia->origem = v;
    aresta_distancia->destino = a->destino;
    aresta_distancia->peso = d;

    no_distancia->proximo = dis->vertices[v].arestas->primeiro;
    dis->vertices[v].arestas->primeiro = no_distancia;

    for(p = acm->vertices[a->destino].arestas->primeiro; p != NULL; p = p->proximo) {
      aresta_p = (struct aresta *) p;
      computa_distancia(dis, acm, v, p, d + aresta_p->peso);
    }
  }
}

//------------------------------------------------------------------------------
grafo distancias(grafo g) {
  struct grafo *dis, *acm;
  struct no *n;
  struct aresta *a;
  unsigned int i;

  dis = (struct grafo *) malloc(sizeof(struct grafo));

  if(dis != NULL) {
    dis->vertices = (struct vertice *) malloc(sizeof(struct vertice) * g->n_vertices);

    if(dis->vertices != NULL) {
      for(i = 0; i < g->n_vertices; ++i) {
        dis->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));
        dis->vertices[i].nome = strdup(g->vertices[i].nome);
      }

      dis->n_vertices = g->n_vertices;
      dis->direcionado = 0;
      dis->ponderado = 1;

      for(i = 0; i < g->n_vertices; ++i) {
        acm = arborescencia_caminhos_minimos(g, g->vertices + i);

        for(n = acm->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
          a = (struct aresta *) n->conteudo;
          computa_distancia(dis, acm, i, n, a->peso);
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

      if(diametro < a->peso) {
        diametro = a->peso;
      }
    }
  }

  destroi_grafo(dis);
  return diametro;
}

//------------------------------------------------------------------------------
int main(void) {
  struct grafo *g, *grafo_na_lista;
  struct vertice *v;
  struct no *n;
  lista l;
  lista l_blocos;

  g = le_grafo(stdin);
  escreve_grafo(stdout, g);

  if((l = ordena(g)) != NULL) {
    for(n = l->primeiro; n != NULL; n = n->proximo) {
      v = (struct vertice *) n->conteudo;
      fprintf(stdout, "%s\n", v->nome);
    }

    destroi_lista(l, _destroi);
  }

  if(fortemente_conexo(g)) {
    fprintf(stdout, "Fortemente conexo!\n");
  } else {
    fprintf(stdout, "Não é fortemente conexo!\n");
  }

  fprintf(stderr, "\n--Imprimindo blocos:\n" );
  
  if((l_blocos = blocos(g)) != NULL) {
    fprintf(stderr, "\n--Blocos criados:\n" );
    for(n = l_blocos->primeiro; n != NULL; n = n->proximo) {
      grafo_na_lista = (struct grafo *) n->conteudo;
      escreve_grafo(stdout, grafo_na_lista);
    }

    destroi_lista(l_blocos, _destroi);
  }

  destroi_grafo(g);
  return 0;
}

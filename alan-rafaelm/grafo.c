#include <stdlib.h>
#include <limits.h>
#include <graphviz/cgraph.h>
#include "string.h"
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
};

const long int infinito = LONG_MAX;

//------------------------------------------------------------------------------
static void inicializa_lista(lista *l) {
  *l = (struct lista *) malloc(sizeof(struct lista));

  if(*l != NULL) {
    (*l)->primeiro = NULL;
  }
}

//------------------------------------------------------------------------------
static void insere_cabeca(lista l, no n) {
  /* Insere o nó na cabeça (começo) da lista */
  n->proximo = l->primeiro;
  l->primeiro = n;
}

//------------------------------------------------------------------------------
static void insere_cabeca_conteudo(lista l, void *conteudo) {
  struct no *n;

  /* Aloca o nó para o conteúdo e o insere no começo da lista */
  n = (struct no *) malloc(sizeof(struct no));

  if(n != NULL) {
    n->conteudo = conteudo;
    insere_cabeca(l, n);
  }
}

//------------------------------------------------------------------------------
static int lista_contem(lista l, void *conteudo) {
  struct no *n;

  /* Percorre os nós da lista até encontrar algum que tenha o conteúdo, se não
     existir, retorna 0 */
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
  /* Desaloca p, se p não é um ponteiro nulo */
  if(p != NULL) {
    free(p);
    return 0;
  }

  return 1;
}

//------------------------------------------------------------------------------
static int _mantem(void *p) {
  /* Mantém p, função utilizada para remover apenas a lista, mas manter
     os conteúdos alocados */
  return 1;
}

//------------------------------------------------------------------------------
int destroi_lista(lista l, int destroi(void *)) {
  struct no *n, *prox;

  if(l == NULL) {
    return 0;
  }

  /* Libera a região de memória ocupada pela lista e seus nós,
     nos conteúdos é utilizada a função definida destroi */
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
static unsigned int encontra_vertice_indice(struct vertice *vertices, unsigned int n_vertices, const char *nome) {
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

    if(grafo_lido->vertices != NULL) {
      /* Percorre todos os vértices do grafo */
      for(i = 0, v = agfstnode(g); i < grafo_lido->n_vertices; ++i, v = agnxtnode(g, v)) {
        /* Duplica na memória o nome do vértice e o atribui na estrutura.
           A duplicação é feita para evitar erros (por exemplo, se o espaço for desalocado) */
        grafo_lido->vertices[i].nome = strdup(agnameof(v));

        /* Aloca a lista de adjacência do vértice */
        grafo_lido->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));

        /* Coloca como nulo o primeiro elemento da lista de arestas (vazio) */
        if(grafo_lido->vertices[i].arestas != NULL) {
          grafo_lido->vertices[i].arestas->primeiro = NULL;
        }
      }

      /* Percorre todos os vértices do grafo */
      for(v = agfstnode(g); v != NULL; v = agnxtnode(g, v)) {
        i = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(v));

        if(grafo_lido->vertices[i].arestas != NULL) {
          /* Percorre todas as arestas adjacentes do vértice */
          for(e = agfstedge(g, v); e != NULL; e = agnxtedge(g, e, v)) {
            peso = agget(e, "peso");

            /* Se o grafo não é direcionado, insere na lista de adjacência
               de v definindo v como a origem */
            if(!grafo_lido->direcionado) {
              /* Inicializa aresta */
              a = (struct aresta *) malloc(sizeof(struct aresta));
              a->peso = (peso != NULL && *peso != '\0') ? atoi(peso) : 1;
              a->origem = i;

              /* Define destino da aresta */
              if(v == aghead(e)) {
                a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(agtail(e)));
              } else {
                a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(aghead(e)));
              }

              /* Insere a aresta na lista de adjacência de v */
              insere_cabeca_conteudo(grafo_lido->vertices[i].arestas, a);

            /* Se o grafo é direcionado, insere na lista de adjacência de v apenas se
               v é origem (ou cauda) do arco */
            } else {
              if(v == agtail(e)) {
                /* Inicializa aresta */
                a = (struct aresta *) malloc(sizeof(struct aresta));
                a->peso = (peso != NULL && *peso != '\0') ? atoi(peso) : 1;
                a->origem = i;
                a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(aghead(e)));

                /* Insere a aresta na lista de adjacência de v e do destino do arco */
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
    /* Libera a região de memória ocupada pelo nome do grafo, se não for nula */
    if(g_ptr->nome != NULL) {
      free(g_ptr->nome);
    }

    /* Libera a região de memória ocupada pelos vértices do grafo, se não for nula */
    if(g_ptr->vertices != NULL) {
      unsigned int i;

      /* Percorre todos os vértices e arestas liberando a região de memória ocupada
         pelos mesmos */
      for(i = 0; i < g_ptr->n_vertices; ++i) {
        if(g_ptr->vertices[i].nome != NULL) {
          free(g_ptr->vertices[i].nome);
        }

        if(g_ptr->vertices[i].arestas != NULL) {
          destroi_lista(g_ptr->vertices[i].arestas, _destroi);
        }
      }

      /* Libera a array de vértices */
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
    for(n = primeiro_no(g->vertices[i].arestas); n != NULL; n = proximo_no(n)) {
      a = (struct aresta *) n->conteudo;

      /* Se g é direcionado mostra o arco apenas se o vértice i é a origem, caso contrário
         imprime apenas se origem < destino, isto garante que ela será impressa apenas uma vez */
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

  /* Se g é direcionado, então retorna 0 conforme especificação */
  if(g->direcionado) {
    return 0;
  }

  /* Caso contrário, processa os componentes de g e verifica se a lista
     tem apenas o primeiro elemento definido, ou seja, se g tem apenas um
     componente (e consequentemente é conexo) */
  comp = componentes(g);

  if(comp->primeiro != NULL && comp->primeiro->proximo == NULL) {
    retorno = 1;
  }

  destroi_lista(comp, destroi_grafo);
  return retorno;
}

//------------------------------------------------------------------------------
grafo arvore_geradora_minima(grafo g) {
  struct grafo *t;
  struct aresta *a, *aresta_selecionada;
  struct no *n;
  long int menor_peso;
  unsigned int i, vertices_processados;
  unsigned int *vertice_processado;

  /* Se g é direcionado, retorna NULL conforme especificação */
  if(g->direcionado) {
    return NULL;
  }

  /* Aloca a árvore t */
  t = (struct grafo *) malloc(sizeof(struct grafo));

  if(t != NULL) {
    /* Aloca os vértices da árvore e seus estados (processado ou não) */
    t->vertices = (struct vertice *) malloc(sizeof(struct vertice) * g->n_vertices);
    vertice_processado = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

    if(t->vertices != NULL && vertice_processado != NULL) {
      /* Inicializa árvore */
      t->nome = NULL;
      t->n_vertices = g->n_vertices;
      t->ponderado = g->ponderado;
      t->direcionado = 0;

      /* Adiciona todos os vértices do grafo na árvore */
      for(i = 0; i < g->n_vertices; ++i) {
        t->vertices[i].nome = strdup(g->vertices[i].nome);
        t->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));
        t->vertices[i].arestas->primeiro = NULL;

        vertice_processado[i] = 0;
      }

      /* Começa busca a partir do vértice de id 0 */
      vertice_processado[0] = 1;
      vertices_processados = 1;

      do {
        aresta_selecionada = NULL;
        menor_peso = infinito;

        /* Varre todas as arestas da fronteira da árvore e seleciona
           a que têm o menor peso */
        for(i = 0; i < g->n_vertices; ++i) {
          if(vertice_processado[i] == 1) {
            for(n = g->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
              a = (struct aresta *) n->conteudo;

              /* Se o destino não foi processado ainda, ou seja, se ele
                 está na fronteira da árvore t */
              if(vertice_processado[a->destino] == 0) {
                /* Apenas seleciona a aresta se seu peso for menor */
                if(menor_peso > a->peso) {
                  menor_peso = a->peso;
                  aresta_selecionada = a;
                }
              }
            }
          }
        }

        /* Adiciona a aresta selecionada na árvore */
        if(aresta_selecionada != NULL) {
          /* Marca o vértice de destino da aresta como processado */
          vertice_processado[aresta_selecionada->destino] = 1;
          ++vertices_processados;

          /* Aloca a aresta a ser adicionada na origem */
          a = (struct aresta *) malloc(sizeof(struct aresta));

          if(a != NULL) {
            /* Define os dados da aresta */
            a->origem = aresta_selecionada->origem;
            a->destino = aresta_selecionada->destino;
            a->peso = aresta_selecionada->peso;

            /* Insere a aresta no vértice de origem em t */
            insere_cabeca_conteudo(t->vertices[aresta_selecionada->origem].arestas, a);
          }

          /* Aloca a aresta a ser adicionada no destino */
          a = (struct aresta *) malloc(sizeof(struct aresta));

          if(a != NULL) {
            /* Define os dados da aresta */
            a->origem = aresta_selecionada->destino;
            a->destino = aresta_selecionada->origem;
            a->peso = aresta_selecionada->peso;

            /* Insere a aresta no vértice de destino em t */
            insere_cabeca_conteudo(t->vertices[aresta_selecionada->origem].arestas, a);
          }
        }

        /* Se não foi selecionada nenhuma aresta, então encerra a busca */
      } while(aresta_selecionada != NULL);

      free(vertice_processado);
    }

    /* Se não foram processados todos os vértices, o grafo é desconexo e
       então retorna NULL conforme especificação */
    if(vertices_processados != g->n_vertices) {
      destroi_grafo(t);
      return NULL;
    }
  }

  return t;
}

//------------------------------------------------------------------------------
static void _gera_componente(grafo g, lista vertices_componente, unsigned int *n_vertices_componente, unsigned int r) {
  struct no *n;
  struct aresta *a;

  /* Insere o vértice na lista de vértices do componente apenas se
     ele ainda não estiver nela */
  if(!lista_contem(vertices_componente, g->vertices + r)) {
    insere_cabeca_conteudo(vertices_componente, g->vertices + r);

    /* Percorre todos os vizinhos do vértice e os adiciona no componente */
    for(n = g->vertices[r].arestas->primeiro; n != NULL; n = n->proximo) {
      a = (struct aresta *) n->conteudo;

      if(a->destino != r) {
        _gera_componente(g, vertices_componente, n_vertices_componente, a->destino);
      }
    }

    ++(*n_vertices_componente);
  }
}

//------------------------------------------------------------------------------
static grafo gera_componente(grafo g, unsigned int r) {
  struct grafo *componente;
  struct lista *vertices_componente;
  struct vertice *v;
  struct aresta *a, *aresta_componente;
  struct no *n, *no_arestas;
  unsigned int n_vertices_componente = 1, count;

  /* Inicializa lista dos vértices do componente */
  inicializa_lista(&vertices_componente);
  insere_cabeca_conteudo(vertices_componente, g->vertices + r);

  /* Realiza uma busca adicionando todos os vértices do componente na lista */
  for(n = g->vertices[r].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    /* Se r é origem da aresta, realiza uma busca em largura e vai
       adicionando os vértices no componente */
    if(a->destino != r) {
      _gera_componente(g, vertices_componente, &n_vertices_componente, a->destino);
    }
  }

  /* Aloca a estrutura do componente */
  componente = (struct grafo *) malloc(sizeof(struct grafo));

  if(componente != NULL) {
    /* Inicializa o componente */
    componente->nome = (char *) NULL;
    componente->direcionado = g->direcionado;
    componente->ponderado = g->ponderado;
    componente->n_vertices = n_vertices_componente;
    componente->vertices = (struct vertice *) malloc(sizeof(struct vertice) * n_vertices_componente);

    /* Percorre todos os vértices da lista */
    if(componente->vertices != NULL) {
      /* Adiciona os vértices da lista na estrutura do componente */
      for(n = vertices_componente->primeiro, count = 0; n != NULL; n = n->proximo, ++count) {
        v = (struct vertice *) n->conteudo;

        componente->vertices[count].nome = strdup(v->nome);
        componente->vertices[count].arestas = (struct lista *) malloc(sizeof(struct lista));
        componente->vertices[count].arestas->primeiro = NULL;
      }

      /* Percorre todos os vértices do componente */
      for(n = vertices_componente->primeiro, count = 0; n != NULL; n = n->proximo, ++count) {
        v = (struct vertice *) n->conteudo;

        /* Percorre todas suas arestas adjacentes */
        for(no_arestas = v->arestas->primeiro; no_arestas != NULL; no_arestas = no_arestas->proximo) {
          a = (struct aresta *) no_arestas->conteudo;

          /* Se o vértice é origem na aresta (isso garante que ela seja adicionada apenas uma vez) */
          if(v == g->vertices + a->origem) {
            /* Aloca a aresta */
            aresta_componente = (struct aresta *) malloc(sizeof(struct aresta));

            if(aresta_componente != NULL) {
              /* Define os elementos da aresta */
              aresta_componente->origem = count;
              aresta_componente->destino = encontra_vertice_indice(componente->vertices, componente->n_vertices, g->vertices[a->destino].nome);
              aresta_componente->peso = a->peso;

              /* Insere a aresta na lista de adjacência do vértice no componente */
              insere_cabeca_conteudo(componente->vertices[count].arestas, aresta_componente);
            }
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
    /* Marca todos os vértices como não processados */
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
//------------------------------------------------------------------------------
static void _ordena(grafo g, lista l, unsigned int v, unsigned char *v_processado, unsigned int *v_pai) {
  struct no *n;
  struct aresta *a;

  /* Marca o vértice v como processado */
  v_processado[v] = 1;

  /* Percorre todos os vizinhos do vértice v */
  for(n = g->vertices[v].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    /* Se o vértice não está processado, então define v como seu pai e o processa */
    if(a->destino != v && v_processado[a->destino] == 0) {
      v_pai[a->destino] = v;
      _ordena(g, l, a->destino, v_processado, v_pai);
    }
  }

  /* Adiciona a estrutura do vértice na lista l */
  insere_cabeca_conteudo(l, g->vertices + v);
}

//------------------------------------------------------------------------------
lista ordena(grafo g) {
  struct lista *l;
  unsigned char *v_processado;
  unsigned int *v_pai;
  unsigned int i;

  /* Se o grafo não é direcionado, retorna NULL conforme especificação */
  if(!g->direcionado) {
    return NULL;
  }

  /* Aloca a lista dos vértices ordenados */
  l = (struct lista *) malloc(sizeof(struct lista));

  if(l != NULL) {
    /* Aloca os campos de estado (processado ou não) e de pai dos vértices */
    v_processado = (unsigned char *) malloc(sizeof(unsigned char) * g->n_vertices);
    v_pai = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

    if(v_processado != NULL && v_pai != NULL) {
      /* Inicializa a lista l */
      l->primeiro = NULL;

      /* Marca todos os vértices não processados com pais indefinidos */
      for(i = 0; i < g->n_vertices; ++i) {
        v_processado[i] = 0;
        v_pai[i] = (unsigned int) -1;
      }

      /* Percorre todos os vértices e os ordena */
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

  /* Encontra o id do vértice raiz r no grafo g */
  if((v = encontra_vertice_indice(g->vertices, g->n_vertices, r->nome)) == -1) {
    return NULL;
  }

  /* Aloca a estrutura da arborescência */
  t = (struct grafo *) malloc(sizeof(struct grafo));

  if(t != NULL) {
    /* Aloca os vértices da arborescência, seus estados (processado ou não) e suas distâncias */
    t->vertices = (struct vertice *) malloc(sizeof(struct vertice) * g->n_vertices);
    vertice_processado = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);
    distancias = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

    if(t->vertices != NULL && vertice_processado != NULL && distancias != NULL) {
      /* Inicializa a arborescência */
      t->nome = NULL;
      t->n_vertices = g->n_vertices;
      t->ponderado = g->ponderado;
      t->direcionado = 1;

      /* Inicializa os vértices da arborescência e marca todos como não processado */
      for(i = 0; i < g->n_vertices; ++i) {
        t->vertices[i].nome = strdup(g->vertices[i].nome);
        t->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));
        t->vertices[i].arestas->primeiro = NULL;

        vertice_processado[i] = 0;
      }

      /* Marca como processado o vértice raiz e define sua distância 0 */
      vertice_processado[v] = 1;
      distancias[v] = 0;

      do {
        aresta_selecionada = NULL;
        menor_distancia = infinito;

        /* Seleciona a aresta da fronteira menor caminho, i.e. a aresta cuja
           soma da distância da raiz até ela mais o seu peso (d(r, a) + a->peso)
           seja mínima */
        for(i = 0; i < g->n_vertices; ++i) {
          if(vertice_processado[i] == 1) {
            for(n = g->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
              a = (struct aresta *) n->conteudo;

              /* Se o vértice não foi processado ainda, ou seja, faz parte da fronteira */
              if(vertice_processado[a->destino] == 0) {
                /* Seleciona a aresta apenas se tiver distância menor que a já selecionada */
                if(menor_distancia > distancias[i] + a->peso) {
                  menor_distancia = distancias[i] + a->peso;
                  aresta_selecionada = a;
                }
              }
            }
          }
        }

        if(aresta_selecionada != NULL) {
          /* Marca o vértice como processado e armazena sua distância */
          vertice_processado[aresta_selecionada->destino] = 1;
          distancias[aresta_selecionada->destino] = menor_distancia;

          /* Aloca a nova aresta da arborescência */
          a = (struct aresta *) malloc(sizeof(struct aresta));

          if(a != NULL) {
            /* Define os elementos da aresta */
            a->origem = aresta_selecionada->origem;
            a->destino = aresta_selecionada->destino;
            a->peso = aresta_selecionada->peso;

            /* Insere a aresta na arborescência */
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
static void computa_distancia(struct grafo *dis, struct grafo *acm, unsigned int v, struct no *n, long int d, unsigned int *v_processado) {
  struct no *p;
  struct aresta *a, *aresta_p, *aresta_distancia;

  /* Aloca a aresta de distância */
  aresta_distancia = (struct aresta *) malloc(sizeof(struct aresta));

  if(aresta_distancia != NULL) {
    a = (struct aresta *) n->conteudo;

    /* Define os elementos da aresta de distância a partir da aresta do nó */
    aresta_distancia->origem = v;
    aresta_distancia->destino = a->destino;
    aresta_distancia->peso = d;

    /* Insere a aresta de distância na lista de adjacência do vértice no
       grafo de distâncias */
    insere_cabeca_conteudo(dis->vertices[v].arestas, aresta_distancia);

    /* Percorre o resto dos vértices da arborescência de caminhos mínimos
       de v, acumulando seus pesos e armazenando em d (onde d é a distância da aresta atual) */
    for(p = acm->vertices[a->destino].arestas->primeiro; p != NULL; p = p->proximo) {
      aresta_p = (struct aresta *) p->conteudo;
      computa_distancia(dis, acm, v, p, d + aresta_p->peso, v_processado);
    }

    /* Marca o vértice destino da aresta como processado */
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

  /* Aloca o grafo de distâncias */
  dis = (struct grafo *) malloc(sizeof(struct grafo));

  if(dis != NULL) {
    /* Aloca os vértices do grafo de distâncias e seus estados (processado ou não) */
    dis->vertices = (struct vertice *) malloc(sizeof(struct vertice) * g->n_vertices);
    v_processado = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

    if(dis->vertices != NULL && v_processado != NULL) {
      /* Inicializa os vértices do grafo de distâncias */
      for(i = 0; i < g->n_vertices; ++i) {
        dis->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));
        dis->vertices[i].arestas->primeiro = NULL;
        dis->vertices[i].nome = strdup(g->vertices[i].nome);
      }

      /* Inicializa o grafo de distâncias */
      dis->nome = NULL;
      dis->n_vertices = g->n_vertices;
      dis->direcionado = g->direcionado;
      dis->ponderado = 1;

      /* Percorre todos os vértices do grafo g */
      for(i = 0; i < g->n_vertices; ++i) {
        /* Gera uma arborescência de caminhos mínimos para cada vértice em g */
        acm = arborescencia_caminhos_minimos(g, g->vertices + i);

        /* Marca todos os vértices como não processados */
        for(j = 0; j < g->n_vertices; ++j) {
          v_processado[j] = 0;
        }

        /* Marca o vértice raiz da árvore como processado */
        v_processado[i] = 1;

        /* Vai percorrendo a arborescência, gerando as arestas e as adicionando
           no grafo de distâncias */
        for(n = acm->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
          a = (struct aresta *) n->conteudo;
          computa_distancia(dis, acm, i, n, a->peso, v_processado);
        }

        /* Se existir algum vértice não processado, então ele não é
           alcançável a partir de i e a distância de i à ele é infinita */
        for(j = 0; j < g->n_vertices; ++j) {
          if(v_processado[j] == 0) {
            /* Aloca a aresta de distância */
            a = (struct aresta *) malloc(sizeof(struct aresta));

            if(a != NULL) {
              /* Inicializa a aresta com peso infinito */
              a->origem = i;
              a->destino = j;
              a->peso = infinito;

              /* Insere a aresta na lista de adjacência de i no grafo de distâncias */
              insere_cabeca_conteudo(dis->vertices[i].arestas, a);
            }
          }
        }

        /* Destroi a arborescência */
        destroi_grafo(acm);
      }

      free(v_processado);
    }
  }

  return dis;
}

//------------------------------------------------------------------------------
static void _busca_profundidade_transposta(grafo g, unsigned int v, unsigned int *t_pre, unsigned int *t_pos, unsigned int *pre, unsigned int *pos) {
  struct no *n;
  struct aresta *a;

  /* Define a pré-ordem de v */
  if(t_pre != NULL && pre != NULL) {
    pre[v] = ++(*t_pre);
  }

  /* Percorre todos os vizinhos de v */
  for(n = g->vertices[v].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    /* Faz a busca em profundidade transposta (i.e. considerando g transposto)
       nos vizinhos não processados */
    if(a->origem != v) {
      if(pre[a->origem] == 0) {
        _busca_profundidade_transposta(g, a->origem, t_pre, t_pos, pre, pos);
      }
    }
  }

  /* Define a pós-ordem de v */
  if(t_pos != NULL && pos != NULL) {
    pos[v] = ++(*t_pos);
  }
}

//------------------------------------------------------------------------------
static void _busca_profundidade(grafo g, unsigned int v, unsigned int *t_pre, unsigned int *t_pos, unsigned int *pre, unsigned int *pos) {
  struct no *n;
  struct aresta *a;

  /* Define a pré-ordem de v */
  if(t_pre != NULL && pre != NULL) {
    pre[v] = ++(*t_pre);
  }

  /* Percorre todos os vizinhos de v */
  for(n = g->vertices[v].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    /* Faz a busca em profundidade nos vizinhos não processados */
    if(a->destino != v) {
      if(pre[a->destino] == 0) {
        _busca_profundidade(g, a->destino, t_pre, t_pos, pre, pos);
      }
    }
  }

  /* Define a pós-ordem de v */
  if(t_pos != NULL && pos != NULL) {
    pos[v] = ++(*t_pos);
  }
}

//------------------------------------------------------------------------------
static void busca_profundidade(grafo g, unsigned int **pre, unsigned int **pos) {
  unsigned int i, t_pre = 0, t_pos = 0;

  /* Aloca os vetores de ordenação em pré-ordem e pós-ordem */
  *pre = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));
  *pos = (unsigned int *) malloc(g->n_vertices * sizeof(unsigned int));

  if(*pre != NULL && *pos != NULL) {
    /* Define todas as pré-ordens como 0 */
    for(i = 0; i < g->n_vertices; ++i) {
      (*pre)[i] = 0;
    }

    /* Percorre todos os vértices realizando a busca em profundidade */
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

  /* Realiza uma busca em profundidade em g */
  busca_profundidade(g, &pre, &pos);

  if(pre != NULL && pos != NULL) {
    t = 0;
    n_trees = 0;

    /* Define como 0 todas as pré-ordens (não serão utilizadas) */
    for(i = 0; i < g->n_vertices; ++i) {
      pre[i] = 0;
    }

    /* Faz outra busca em profundidade, desta vez em ordem descrescente
       de pós-ordem e considerando g transposto */
    do {
      max_pos = 0;

      /* Obtém o vértice de maior pós ordem não processado */
      for(i = g->n_vertices; i > 0; --i) {
        if(pre[i - 1] == 0 && max_pos < pos[i - 1]) {
          max_pos = pos[i - 1];
          v = i - 1;
        }
      }

      /* Realiza a busca a partir deste vértice */
      if(max_pos != 0) {
        _busca_profundidade_transposta(g, v, &t, NULL, pre, NULL);
        /* Aumenta o número de subgrafos gerados pela busca */
        ++n_trees;
      }
    } while(max_pos != 0);

    free(pre);
    free(pos);
  }

  /* Se existe apenas um subgrafo gerado pela busca, então g é fortemente
     conexo, caso contrário g não é fortemente conexo */
  return (n_trees < 2) ? 1 : 0;
}

//------------------------------------------------------------------------------
long int diametro(grafo g) {
  struct grafo *dis;
  struct no *n;
  struct aresta *a;
  long int diametro = 0;
  unsigned int i;

  /* Obtêm o grafo de distâncias de g */
  dis = distancias(g);

  /* Percorre todas as arestas de g */
  for(i = 0; i < dis->n_vertices; ++i) {
    for(n = dis->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
      a = (struct aresta *) n->conteudo;

      /* Se o peso da aresta é maior que o diametro e não é infinito,
         armazena-o no diametro */
      if(diametro < a->peso && a->peso != infinito) {
        diametro = a->peso;
      }
    }
  }
  /* Destroi o grafo de distâncias */
  destroi_grafo(dis);
  return diametro;
}

/*void _gera_bloco(grafo g, lista vertices_corte, lista vertices_bloco, unsigned int *n_vertices_bloco, unsigned int r) {
  struct no *n;
  struct aresta *a;

  //Chama recursivamente a função pra os vizinhos que não são vertice de corte
  if(!lista_contem(vertices_bloco, g->vertices + r)) {
    //Se g->vertices r não está no lista de vertices do bloco (Y)
    insere_cabeca_conteudo(vertices_bloco, g->vertices + r);
    //Coloca g->vertices r no lista de vertices do bloco
    if(!lista_contem(vertices_corte, g->vertices + r)) {
      //Se g->vertices r nao for certice de corte
      for(n = g->vertices[r].arestas->primeiro; n != NULL; n = n->proximo) {
        a = (struct aresta *) n->conteudo;
        _gera_bloco(g, vertices_corte, vertices_bloco, n_vertices_bloco, a->destino);
      }
    } else {
      //Chama a função recursivamente para os vizinhos
      for(n = g->vertices[r].arestas->primeiro; n != NULL; n = n->proximo) {
        a = (struct aresta *) n->conteudo;
        //Se g->vertices r for vertice de corte
        if(!lista_contem(vertices_corte, g->vertices + (a->destino))) {
          _gera_bloco(g, vertices_corte, vertices_bloco, n_vertices_bloco, a->destino);
        }
      }
    }

    ++(*n_vertices_bloco);
  }
}

//------------------------------------------------------------------------------
grafo gera_bloco(grafo g, unsigned int r, lista vertices_corte) {
  struct aresta *a, *aresta_bloco;
  struct vertice *v;
  struct no *n, *no_arestas;
  struct grafo *bloco;
  lista vertices_bloco;
  unsigned int n_vertices_bloco = 0, count = 0;

  inicializa_lista(&vertices_bloco);
  insere_cabeca_conteudo(vertices_bloco, g->vertices + r);
  n_vertices_bloco = 1;

  //Realiza uma busca adicionando todos os vértices do componente na lista
  for(n = g->vertices[r].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;
    _gera_bloco(g, vertices_corte, vertices_bloco, &n_vertices_bloco, a->destino);
  }

  bloco = (grafo) malloc(sizeof(struct grafo));
  //Percorre todos os vértices da lista
  if(bloco != NULL) {
    bloco->nome = (char *) NULL;
    bloco->direcionado = 0;
    bloco->n_vertices = 0;
    bloco->ponderado = g->ponderado;
    bloco->vertices = (struct vertice *) malloc(sizeof(struct vertice) * n_vertices_bloco);
    //Adiciona os vértices da lista na estrutura do componente
    if(bloco->vertices != NULL) {
      for(n = vertices_bloco->primeiro; n != NULL; n = n->proximo) {
        v = (struct vertice *) n->conteudo;

        bloco->vertices[count].nome = strdup(v->nome);
        bloco->vertices[count].arestas = (struct lista *) malloc(sizeof(struct lista));
        bloco->vertices[count].arestas->primeiro = NULL;

        ++count;
      }

      //Percorre todos os vértices do bloco
      for(n = vertices_bloco->primeiro, count = 0; n != NULL; n = n->proximo, ++count) {
        v = (struct vertice *) n->conteudo;

        //Percorre todas suas arestas adjacentes
        for(no_arestas = v->arestas->primeiro; no_arestas != NULL; no_arestas = no_arestas->proximo) {
          a = (struct aresta *) no_arestas->conteudo;

          //Se o vértice é origem na aresta (isso garante que ela seja adicionada apenas uma vez)
          if(v == g->vertices + a->origem) {
            //Aloca a aresta
            aresta_bloco = (struct aresta *) malloc(sizeof(struct aresta));

            if(aresta_bloco != NULL) {
              //Define os elementos da aresta
              aresta_bloco->origem = count;
              aresta_bloco->destino = encontra_vertice_indice(bloco->vertices, bloco->n_vertices, g->vertices[a->destino].nome);
              aresta_bloco->peso = a->peso;

              //Insere a aresta na lista de adjacência do vértice no bloco
              insere_cabeca_conteudo(bloco->vertices[count].arestas, aresta_bloco);
            }
          }
        }
      }
    }
  }

  destroi_lista(vertices_bloco, _mantem);
  return bloco;
}

//-----------------------------------------------------------------------------
lista gera_blocos(grafo g, lista vertices_corte) {
  lista lista_blocos;
  struct grafo *bloco;
  unsigned int i, v, v_processados;
  unsigned int *vertice_processado;

  //Inicializa a lista de blocos
  inicializa_lista(&lista_blocos);

  vertice_processado = (unsigned int *) malloc(sizeof(unsigned int) * g->n_vertices);

  if(vertice_processado != NULL) {
    for(i = 0; i < g->n_vertices; ++i) {
      vertice_processado[i] = 0;
    }

    v_processados = 0;

    //Enquanto não forem processados todos os vértices de G
    while(v_processados < g->n_vertices) {
      //Procura um vértice em G ainda não processado
      for(v = 0; v < g->n_vertices && vertice_processado[v] == 1; ++v);
      //Gera bloco no qual está o vértice
      bloco = gera_bloco(g, v, vertices_corte);
      if (lista_contem(vertices_corte, g->vertices + v)){
        vertice_processado[v] = 1;
        ++v_processados;
      } else {

        if(bloco != NULL) {
          //Insere bloco na lista
          insere_cabeca_conteudo(lista_blocos, bloco);

          //Marca como processados todos os vértices do bloco (menos)
          for(i = 0; i < bloco->n_vertices; ++i) {
            vertice_processado[encontra_vertice_indice(g->vertices, g->n_vertices, bloco->vertices[i].nome)] = 1;
            ++v_processados;
          }
        }
      }
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


  //Incrementa o contador externoe o atribui a pre[i]
  pre[i] = ++(*conta);
  //No começo do laço, lowpoint de i é o próprio i
  low[i] = pre[i];
  //Laço para percorrer cada aresta de g->vertices[i]
  for (n = g->vertices[i].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;
    j = a->destino;
    //Se a ponta de g->vertices[i] ainda não foi foi incluída na pre-ordem
    if (pre[j] == -1) {
      //Indica que o j é um destino de g->vertices[i
      pai[j] = i;
      //Chama recursivamente a função para computar a pre-ordem filhos de 
      _vertices_corte(g, j, n_articulacoes , pre, pai, low, articulacoes, conta);
      //Se o lowerpoint do filho j é menor, então lowerpoint de i também é o de j
      if (low[i] > low[j]){
        low[i] = low[j];
      }
      if (low[j] == pre[j]) {
        //Achou aresta de corte
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
    insere_cabeca_conteudo(lista_articulacoes, g->vertices + articulacoes[i]);
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
  //Se a lista está vazia, então g só possui 1 bloco
  if(vertices_de_corte == NULL){
    lista_blocos = (lista) malloc(sizeof(struct lista));
    inicializa_lista(&lista_blocos);
    insere_cabeca_conteudo(lista_blocos, g);
  } else {
    lista_blocos = gera_blocos(g, vertices_de_corte);
  }
  destroi_lista(vertices_de_corte, _mantem);
  return lista_blocos;
}
*/

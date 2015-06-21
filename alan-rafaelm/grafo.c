#include <stdlib.h>
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
};

struct grafo {
  char *nome;
  int direcionado;
  int ponderado;
  unsigned int n_vertices;
  vertice vertices;
};

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
  struct no *n, *prox;

  if(l == NULL) {
    return 0;
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
        grafo_lido->vertices[i].arestas = (struct lista *) malloc(sizeof(struct lista));

        if(grafo_lido->vertices[i].arestas != NULL) {
          grafo_lido->vertices[i].arestas->primeiro = NULL;
        }
      }

      /* Percorre todos os vértices do grafo */
      for(i = 0, v = agfstnode(g); i < grafo_lido->n_vertices; ++i, v = agnxtnode(g, v)) {
        if(grafo_lido->vertices[i].arestas != NULL) {
          /* Percorre todas as arestas adjacentes do vértice */
          for(e = agfstedge(g, v); e != NULL; e = agnxtedge(g, e, v)) {
            /* Aloca o novo nó da aresta */
            n = (struct no *) malloc(sizeof(struct no));

            /* Adiciona o nó da aresta na lista de adjacência do vértice */
            if(n != NULL) {
              n->proximo = grafo_lido->vertices[i].arestas->primeiro;
              n->conteudo = malloc(sizeof(struct aresta));

              if(n->conteudo != NULL) {
                a = (struct aresta *) n->conteudo;

                /* Define o destino da aresta */
                if(strcmp(grafo_lido->vertices[i].nome, agnameof(aghead(e))) == 0) {
                  a->origem = i;
                  a->destino = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(agtail(e)));
                } else {
                  a->destino = i;
                  a->origem = encontra_vertice_indice(grafo_lido->vertices, grafo_lido->n_vertices, agnameof(aghead(e)));
                }
              }

              grafo_lido->vertices[i].arestas->primeiro = n;
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

        if(g_ptr->vertices[i].arestas != NULL) {
          destroi_lista(g_ptr->vertices[i].arestas, _destroi);
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
    for(n = primeiro_no(g->vertices[i].arestas); n != NULL; n = proximo_no(n)) {
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

  destroi_lista(comp, _destroi);
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
lista componentes(grafo g) {
  struct lista *lista_componentes, *lista_v;
  struct grafo *componente_conexo;
  struct vertice *v_fora_vg;
  unsigned int i, n_vertices_v;

  /* Inicializa a lista de componentes */
  inicializa_lista(lista_componentes);
  /* Inicializa o conjunto de vertices */
  inicializa_lista(lista_v);
  /* Aloxa espaço para um componente conexo */
  componente_conexo = (struct grafo *) malloc(sizeof(struct grafo));

  /* Testa a alocação */
  if (componente_conexo){
    n_vertices_v = 0;
    v_fora_vg = NULL;
    /* Enquanto não tiver adicionar todos os V(G) à lista_v */
    while (n_vertices_v < g->n_vertices){
      /* Itera sobre todos os V(G) */
      for(i = 0; i < g->n_vertices, v_fora_vg == NULL; ++i) {
        /* Se não encontrar g->vertices[i].nome na lista_v, é um vértice V(G)-V */
        if (encontra_vertice_indice(lista_v, n_vertices_v, g->vertices[i].nome) == -1){
          v_fora_vg = g->vertices[i];
        }
      }
      /* Obtem 1 componente conexo de G que ainda não foi processado */
      componente_conexo = busca_c_c(g,v_fora_vg);
      /* Se encontrou algum componente conexo não processado */
      if (componente_conexo){
        /* Insere na lista de componentes */
        insere_cabeca_conteudo(lista_componentes, componente_conexo);
        /* Itera sobre cada vertice do componente conexo adicionando-o à lista_v */
        for (i = 0; i < componente_conexo->n_vertices; i++){
          insere_cabeca_conteudo(lista_v, componente_conexo->vertices[i]);
          n_vertices_v++;
        }
      }
    }
    destroi_grafo(componente_conexo);
    destroi_lista(lista_v);
    return lista_componentes;
  }
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
void _ordena(grafo g, lista l, unsigned int v, unsigned char *v_processado, unsigned int *v_pai) {
  struct no *n;
  struct aresta *a;

  v_processado[v] = 1;
  for(n = g->vertices[v].arestas->primeiro; n != NULL; n = n->proximo) {
    a = (struct aresta *) n->conteudo;

    if(a->destino != v) {
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

//----------------------------Funções extras auxiliares-------------------------
void inicializa_lista(lista * l){
  /* Aloca espaço pra lista */
  l = (struct lista *) malloc(sizeof(struct lista));
  /* Inicia lista vazia */
  if(l != NULL) {
    l->primeiro = NULL;
  }
}

//------------------------------------------------------------------------------
lista busca_c_c(grafo g, vertice r){
  struct lista *vertices_x;
  struct grafo *componente_conexo;
  struct no *n,*v_fronteira;
  struct vertice *v;
  unsigned int n_vertices_x;

  inicializa_lista(vertices_x);
  insere_cabeca_conteudo(vertices_x, r);
  n_vertices_x = 1;
  /* Obtem 1 vertice da fronteira de X se existir, se não NULL */
  v_fronteira = fronteira(grafo g, lista vertices_x, unsigned int n_vertices_x);
  while (v_fronteira){
    /* Adiciona o vertice de  */
    insere_cabeca_conteudo(vertices_x, v_fronteira);
    n_vertices_x++;
    v_fronteira = fronteira(grafo g, lista vertices_x, unsigned int n_vertices_x);
  }
  if (n_vertices_x == 1){
    destroi_lista(vertices_x);
    return NULL;
  }
  else{
    if(componente_conexo = (struct grafo *) malloc(sizeof(struct grafo))) {
      /* Inicializa a estrutura de grafo para retornar o componente conexo*/
      componente_conexo->nome = (char *) NULL;
      componente_conexo->vertices = vertices_x;
      componente_conexo->direcionado = g->direcionado;
      componente_conexo->n_vertices = (int) n_vertices_x;
      destroi_lista(vertices_x);
      return componente_conexo;
    }
  }
}

//------------------------------------------------------------------------------
/* Retorna um vértice de fronteira do conjunto X */
vertice * fronteira(grafo g, lista vertices_x, unsigned int n_vertices_x){
  struct no *n;
  struct aresta *a;
  unsigned int i;

  /* Percorre todos os vértices em X */
  for(i = 0; i < n_vertices_x; ++i) {
    /* Para cada vértice em X, percorre as arestas */
    for(n = primeiro_no(vertices_x[i].arestas); n != NULL; n = proximo_no(n)) {
      a = (struct aresta *) n->conteudo;
      /* Se não encontrar a->destino em X, então a->destino eh fronteira */
      if (encontra_vertice_indice(vertices_x, n_vertices_x, g->vertices[a->destino].nome)==-1){
        /* retorna a->destino */
        return g->vertices[a->destino];
      }
    }
  }
  return NULL;
}

//------------------------------------------------------------------------------
int main(void) {
  return ! destroi_grafo(escreve_grafo(stdout, le_grafo(stdin)));
}

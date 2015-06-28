#ifndef _GRAFO_H
#define _GRAFO_H

#include <stdio.h>

//------------------------------------------------------------------------------
// valor que representa "infinito"

const long int infinito;

//-----------------------------------------------------------------------------
// lista encadeada

typedef struct lista *lista;

//-----------------------------------------------------------------------------
// nó da lista encadeada cujo conteúdo é um void *

typedef struct no *no;

//------------------------------------------------------------------------------
// devolve o primeiro nó da lista l,
//      ou NULL, se l é vazia

no primeiro_no(lista l);

//------------------------------------------------------------------------------
// devolve o sucessor do nó n em l,
//      ou NULL, se n for o último nó de l

no proximo_no(no n);

//------------------------------------------------------------------------------
// devolve o conteúdo do nó n

void *conteudo(no n);

//------------------------------------------------------------------------------
// desaloca a lista l e todos os seus nós
// 
// se destroi != NULL invoca
//
//     destroi(conteudo(n)) 
//
// para cada nó da lista. 
//
// devolve 1 em caso de sucesso,
//      ou 0 em caso de falha

int destroi_lista(lista l, int destroi(void *));

//------------------------------------------------------------------------------
// (apontador para) estrutura de dados que representa um vértice do grafo
// 
// os vértices do grafo tem nome que são "string"s quaisquer

typedef struct vertice *vertice;

//------------------------------------------------------------------------------
// devolve o nome do vertice v

char *nome_vertice(vertice v);

//------------------------------------------------------------------------------
// grafo
// 
// o grafo pode ser
// - direcionado ou não
// - com pesos nas arestas ou não
// 
// o grafo tem um nome, que é uma "string" qualquer
// 
// num grafo com pesos nas arestas, todas as arestas tem peso
// 
// o peso de uma aresta é um long int e seu valor default é zero

typedef struct grafo *grafo;

//------------------------------------------------------------------------------
// lê um grafo no formato dot de input, usando as rotinas de libcgraph
// 
// desconsidera todos os atributos do grafo lido
// exceto o atributo "peso" nas arestas onde ocorra
// 
// devolve o grafo lido,
//      ou NULL, em caso de erro 
//
// desaloca a estrtura de dados devolvida pelas rotinas de libcgraph
// quando da leitura do grafo assim que ela não seja mais necessária

grafo le_grafo(FILE *input);  

//------------------------------------------------------------------------------
// desaloca toda a memória usada em g
// 
// devolve 1 em caso de sucesso,
//      ou 0, caso contrário
//
// g é um (void *) para que destroi_grafo() possa ser usada como argumento de
// destroi_lista()

int destroi_grafo(void *g);

//------------------------------------------------------------------------------
// escreve o g em output usando o formato dot, de forma que
// 
// 1. todos os vértices são escritos antes de todas as arestas (arcos)
// 2. se uma aresta (arco) tem peso, este deve ser escrito como um atributo
//
// devolve o grafo escrito,
//      ou NULL, em caso de erro 

grafo escreve_grafo(FILE *output, grafo g);

//------------------------------------------------------------------------------
// devolve o nome do grafo g

char *nome(grafo g);

//------------------------------------------------------------------------------
// devolve o número de vértices do grafo g

unsigned int n_vertices(grafo g);

//------------------------------------------------------------------------------
// devolve 1, se g é direcionado,
//      ou 0, caso contrário

int direcionado(grafo g);

//------------------------------------------------------------------------------
// devolve 1, se g é não direcionado e conexo,
//      ou 0, caso contrário

int conexo(grafo g); 

//------------------------------------------------------------------------------
// devolve uma árvore geradora mínima do grafo g,
//      ou NULL, se g não for um grafo não direcionado conexo

grafo arvore_geradora_minima(grafo g);

//------------------------------------------------------------------------------
// devolve uma lista de grafos onde cada grafo é um componente de g

lista componentes(grafo g);

//------------------------------------------------------------------------------
// devolve uma lista de grafos onde cada grafo é um bloco de g
//      ou NULL, se g é um grafo direcionado

lista blocos(grafo g);

//------------------------------------------------------------------------------
// devolve uma lista dos vértices de g ordenados topologicamente,
//      ou NULL se g não é um grafo direcionado ou se tem circuito direcionado

lista ordena(grafo g);

//------------------------------------------------------------------------------
// devolve uma arborescência de caminhos mínimos de g de raiz r

grafo arborescencia_caminhos_minimos(grafo g, vertice r); 

//------------------------------------------------------------------------------
// devolve um grafo com pesos, onde
//
//     - os vértices tem os mesmos nomes que os de g
//
//     - a aresta {u,v} (arco (u,v)) ocorre se v é alcançável a partir
//       de u em g
//
//     - o peso da aresta {u,v} (arco (u,v)) é a distância de u a v em g
//
// o grafo é computado usando a função arborescencia_caminhos_minimos()

grafo distancias(grafo g);

//------------------------------------------------------------------------------
// devolve 1, se g é fortemente conexo,
//      ou 0, caso contrário

int fortemente_conexo(grafo g);

//------------------------------------------------------------------------------
// devolve o diâmetro de g

long int diametro(grafo g);

#endif

#include <stdlib.h>
#include "grafo.h"

int nao_destroi_nos(void *p) {
  return 1;
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
    for(n = primeiro_no(l); n != NULL; n = proximo_no(n)) {
      v = (struct vertice *) conteudo(n);
      fprintf(stdout, "%s\n", nome_vertice(v));
    }

    destroi_lista(l, nao_destroi_nos);
  }

  if((l = componentes(g)) != NULL) {
    for(n = primeiro_no(l); n != NULL; n = proximo_no(n)) {
      c = (struct grafo *) conteudo(n);
      escreve_grafo(stdout, c);
    }

    destroi_lista(l, destroi_grafo);
  }

  d = arvore_geradora_minima(g);
  if(d != NULL) {
    escreve_grafo(stdout, d);
    destroi_grafo(d);
  }


  /*
  d = arborescencia_caminhos_minimos(g, v);
  if(d != NULL) {
    escreve_grafo(stdout, d);
    destroi_grafo(d);
  }
  */

  d = distancias(g);
  if(d != NULL) {
    escreve_grafo(stdout, d);
    destroi_grafo(d);
  }

  fprintf(stdout, "Diametro = %ld\n", diametro(g));

  if(conexo(g)) {
    fprintf(stdout, "Conexo!\n");
  } else {
    fprintf(stdout, "Não é conexo!\n");
  }

  if(fortemente_conexo(g)) {
    fprintf(stdout, "Fortemente conexo!\n");
  } else {
    fprintf(stdout, "Não é fortemente conexo!\n");
  }

  /*if((l_blocos = blocos(g)) != NULL) {
    fprintf(stderr, "\n--Blocos criados:\n" );
    for(n = primeiro_no(l); n != NULL; n = proximo_no(n)) {
      c = (struct grafo *) conteudo(n);
      escreve_grafo(stdout, c);
    }

    destroi_lista(l_blocos, destroi_grafo);
  }*/

  destroi_grafo(g);
  return 0;
}

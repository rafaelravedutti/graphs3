Trabalho de Implementação 3 de Algoritmos e Teoria dos Grafos (CI065)
Rafael Ravedutti Lucio Machado - rafaelm - GRR20135958
Alan Peterson Carvalho Silva - alan - GRR20110556

--------------------------------------------------------------------------

Informações sobre o trabalho:

A maior parte das funções foram implementadas e testadas com grafos pequenos. Na
hora de gerar uma lista de vértices em ordenação topológica (ordena) é necessário
usar na função destroi_lista uma função que não faça nada em destroi, pois os vértices
apontarão para os mesmos vértices da estrutura do grafo (assim fica mais fácil
fazer uma função vazia do que uma destroi_vertice).

O resto das funções geram estruturas independentes e portanto é necessário
desalocar a memória utilizada por elas (se for lista usar destroi_lista,
se for grafo usar destroi_grafo, por exemplo).

Para se obter um vértice a ser usado na função arborescencia_caminhos_minimos
(pois não há nenhuma função especificada para isso), pode-se alocar a estrutura
do vértice separadamente do grafo e apenas definir seu nome.

O programa valgrind foi utilizado para testar se houve memória não desalocada e
foi utilizada a opção -Wall do gcc para verificar os avisos de compilação.

#include <stdio.h>
#define TAREFAS 3
#define  ARRAY_SIZE 4

/* Ordena o vetor em ordem crescente. */
void bs(int n, int *vetor)
{
    int c = 0;
    int d;
    int troca;
    int trocou = 1;

    while (c < n - 1 && trocou)
    {
        trocou = 0;

        for (d = 0; d < n - c - 1; d++)
        {
            if (vetor[d] > vetor[d + 1])
            {
                troca = vetor[d];
                vetor[d] = vetor[d + 1];
                vetor[d + 1] = troca;
                trocou = 1;
            }
        }

        c++;
    }
}

int main(void)
{
   /* Cada linha representa uma tarefa independente. */
    int saco[TAREFAS][ARRAY_SIZE] = {
        {5, 3, 4, 1},
        {9, 2, 7, 6},
        {8, 4, 2, 3}
    };

    int tarefa;  // Identificador da tarefa.
    int i;       // Posição de um elemento dentro do vetor.

    /* Processa as tarefas uma após a outra. */
    for (tarefa = 0; tarefa < TAREFAS; tarefa++)
    {
        printf("Tarefa %d antes:", tarefa);

        /* Mostra os elementos da tarefa atual. */
        for (i = 0; i < ARRAY_SIZE; i++)
        {
            printf(" %d", saco[tarefa][i]);
        }
        printf("\n");

        /* Ordena somente o vetor da tarefa atual. */
        bs(ARRAY_SIZE, saco[tarefa]);

        printf("Tarefa %d depois:", tarefa);

        for (i = 0; i < ARRAY_SIZE; i++)
        {
            printf(" %d", saco[tarefa][i]);
        }
        printf("\n");
    }

    return 0;
}

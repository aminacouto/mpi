#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 20000

// Bubble Sort 
void bs(int n, int *vetor)
{
    int c = 0;
    int d;
    int troca;
    int trocou = 1;

    while (c < (n - 1) && trocou)
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


// Retorna o tempo atual em segundos
double tempo_atual()
{
    struct timespec tempo;

    clock_gettime(CLOCK_MONOTONIC, &tempo);

    return tempo.tv_sec + tempo.tv_nsec / 1000000000.0;
}


int main(void)
{
    int i;
    double tempo_inicio;
    double tempo_fim;

    // Ordena um único vetor
    int *vetor = malloc((size_t)ARRAY_SIZE * sizeof(*vetor));

    if (vetor == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória para o vetor.\n");
        return EXIT_FAILURE;
    }

    // Ordem decrescente: pior caso para o Bubble Sort.
    for (i = 0; i < ARRAY_SIZE; i++)
        vetor[i] = ARRAY_SIZE - i;

    // Mede somente a ordenação, sem inicialização ou impressões.
    tempo_inicio = tempo_atual();
    bs(ARRAY_SIZE, vetor);
    tempo_fim = tempo_atual();

    printf("Elementos: %d\n", ARRAY_SIZE);
    printf("Tempo da ordenação sequencial (Bubble Sort): %.6f segundos\n",
           tempo_fim - tempo_inicio);

    free(vetor);
    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 20000

// Comparação para o qsort
int comparar(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (x > y) - (x < y);
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

    // Ordem decrescente.
    for (i = 0; i < ARRAY_SIZE; i++)
        vetor[i] = ARRAY_SIZE - i;

    // Mede somente a ordenação, sem inicialização ou impressões.
    tempo_inicio = tempo_atual();
    qsort(vetor, ARRAY_SIZE, sizeof(*vetor), comparar);
    tempo_fim = tempo_atual();

    printf("Elementos: %d\n", ARRAY_SIZE);
    printf("Tempo da ordenação sequencial (qsort): %.6f segundos\n",
           tempo_fim - tempo_inicio);

    free(vetor);
    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Permite configurar os valores pela compilação em testes.sh.
#ifndef TAREFAS
#define TAREFAS 64
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE 10000
#endif


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


int main()
{
    int i, j;

    double tempo_inicio;
    double tempo_fim;

    // Aloca dinamicamente o saco de trabalho
    int *saco = malloc(
        (size_t)TAREFAS *
        ARRAY_SIZE *
        sizeof(int)
    );

    if (saco == NULL)
    {
        printf("Erro ao alocar memória para o saco de trabalho.\n");
        return 1;
    }


    // Inicia a medição no mesmo ponto da versão paralela
    tempo_inicio = tempo_atual();


    // Cria os mesmos vetores utilizados na versão paralela
    for (i = 0; i < TAREFAS; i++)
    {
        for (j = 0; j < ARRAY_SIZE; j++)
        {
            saco[i * ARRAY_SIZE + j] =
                ARRAY_SIZE - j + (i * ARRAY_SIZE);
        }
    }


    // Ordena todos os vetores sequencialmente
    for (i = 0; i < TAREFAS; i++)
    {
        bs(
            ARRAY_SIZE,
            &saco[i * ARRAY_SIZE]
        );
    }


    // Finaliza a medição
    tempo_fim = tempo_atual();


    printf("Tarefas concluídas: %d de %d\n",
           TAREFAS,
           TAREFAS);

    printf("Tempo total do processamento sequencial: %.6f segundos\n",
           tempo_fim - tempo_inicio);

    // Libera a memória
    free(saco);

    return 0;
}
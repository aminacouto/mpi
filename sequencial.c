#include <stdio.h>
#include <time.h>

#define TAREFAS 10
#define ARRAY_SIZE 10000


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
    // Saco de trabalho
    int saco[TAREFAS][ARRAY_SIZE];

    int i, j;

    double tempo_inicio;
    double tempo_fim;


    // Inicia a medição no mesmo ponto da versão paralela
    tempo_inicio = tempo_atual();


    // Cria os mesmos vetores utilizados na versão paralela
    for (i = 0; i < TAREFAS; i++)
    {
        for (j = 0; j < ARRAY_SIZE; j++)
        {
            saco[i][j] = ARRAY_SIZE - j + (i * ARRAY_SIZE);
        }
    }


    // Ordena todos os vetores sequencialmente
    for (i = 0; i < TAREFAS; i++)
    {
        bs(ARRAY_SIZE, saco[i]);
    }


    // Finaliza a medição
    tempo_fim = tempo_atual();


    printf("Tarefas concluídas: %d de %d\n",
           TAREFAS,
           TAREFAS);

    printf("Tempo total do processamento sequencial: %.6f segundos\n",
           tempo_fim - tempo_inicio);


    return 0;
}
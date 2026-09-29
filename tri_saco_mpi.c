#include <stdio.h>
#include <mpi.h>

#define TAREFAS 3
#define  ARRAY_SIZE 10

#define REQUEST 1
#define TASK    2
#define RESULT  3
#define STOP    4

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

int main(int argc, char *argv[])
{
    int my_rank;
    int proc_n;
    int i;

    /* Identificador seguido dos elementos do vetor. */
    int message[ARRAY_SIZE + 1];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    if (proc_n < 2)
    {
        if (my_rank == 0)
        {
            printf("Execute com pelo menos 2 processos.\n");
        }

        MPI_Finalize();
        return 1;
    }

    if (my_rank == 0)
    {
        int saco[TAREFAS][ARRAY_SIZE];

/* Gera cada vetor em ordem decrescente. */
for (int tarefa = 0; tarefa < TAREFAS; tarefa++)
{
    for (int elemento = 0; elemento < ARRAY_SIZE; elemento++)
    {
        saco[tarefa][elemento] =
            ARRAY_SIZE - elemento + tarefa * ARRAY_SIZE;
    }
}

        int next_task = 0;
        int active_workers = proc_n - 1;

        while (active_workers > 0)
        {
            /* Recebe pedidos ou resultados de qualquer trabalhador. */
            MPI_Recv(message, ARRAY_SIZE + 1, MPI_INT,
                     MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == REQUEST)
            {
                if (next_task < TAREFAS)
                {
                    message[0] = next_task;

                    /* Copia o vetor para depois do identificador. */
                    for (i = 0; i < ARRAY_SIZE; i++)
                    {
                        message[i + 1] = saco[next_task][i];
                    }

                    MPI_Send(message, ARRAY_SIZE + 1, MPI_INT,
                             status.MPI_SOURCE, TASK,
                             MPI_COMM_WORLD);

                    printf("Mestre: tarefa %d -> trabalhador %d.\n",
                           next_task, status.MPI_SOURCE);

                    next_task++;
                }
                else
                {
                    MPI_Send(NULL, 0, MPI_INT,
                             status.MPI_SOURCE, STOP,
                             MPI_COMM_WORLD);

                    printf("Mestre: STOP -> trabalhador %d.\n",
                           status.MPI_SOURCE);

                    active_workers--;
                }
            }
            else if (status.MPI_TAG == RESULT)
            {
                int tarefa = message[0];

                /* Guarda o vetor ordenado na sua linha original. */
                for (i = 0; i < ARRAY_SIZE; i++)
                {
                    saco[tarefa][i] = message[i + 1];
                }

                printf("Mestre: resultado da tarefa %d, trabalhador %d.\n",
                       tarefa, status.MPI_SOURCE);
            }
        }

        printf("Saco final:\n");

        for (int tarefa = 0; tarefa < TAREFAS; tarefa++)
        {
            printf("Tarefa %d:", tarefa);

            for (i = 0; i < ARRAY_SIZE; i++)
            {
                printf(" %d", saco[tarefa][i]);
            }

            printf("\n");
        }
    }
    else
    {
        while (1)
        {
            /* O trabalhador toma a iniciativa. */
            MPI_Send(NULL, 0, MPI_INT, 0, REQUEST, MPI_COMM_WORLD);

            MPI_Recv(message, ARRAY_SIZE + 1, MPI_INT,
                     0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == STOP)
            {
                break;
            }

            /* Ordena os dados, preservando o identificador. */
            bs(ARRAY_SIZE, &message[1]);

            MPI_Send(message, ARRAY_SIZE + 1, MPI_INT,
                     0, RESULT, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}

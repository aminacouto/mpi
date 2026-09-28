#include <stdio.h>
#include <mpi.h>

#define TAREFAS 3
#define ARRAY_SIZE 10

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

int main(int argc, char *argv[])
{
    int my_rank;
    int proc_n;
    int saco[TAREFAS][ARRAY_SIZE];
    int message[ARRAY_SIZE];
    int i, j;

    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    if (my_rank == 0)
    {
        for (i = 0; i < TAREFAS; i++)
        {
            for (j = 0; j < ARRAY_SIZE; j++)
            {
                saco[i][j] = ARRAY_SIZE - j + (i * ARRAY_SIZE);
            }
        }

        for (i = 0; i < TAREFAS; i++)
        {
            printf("Mestre enviando tarefa %d para escravo %d\n",
                   i, i + 1);

            MPI_Send(saco[i],
                     ARRAY_SIZE,
                     MPI_INT,
                     i + 1,
                     0,
                     MPI_COMM_WORLD);
        }

        for (i = 0; i < TAREFAS; i++)
        {
            MPI_Recv(message,
                     ARRAY_SIZE,
                     MPI_INT,
                     MPI_ANY_SOURCE,
                     MPI_ANY_TAG,
                     MPI_COMM_WORLD,
                     &status);

            printf("Mestre recebeu do escravo %d: ",
                   status.MPI_SOURCE);

            for (j = 0; j < ARRAY_SIZE; j++)
            {
                printf("%d ", message[j]);
            }

            printf("\n");
        }
    }
    else
    {
        MPI_Recv(message,
                 ARRAY_SIZE,
                 MPI_INT,
                 0,
                 MPI_ANY_TAG,
                 MPI_COMM_WORLD,
                 &status);

        bs(ARRAY_SIZE, message);

        MPI_Send(message,
                 ARRAY_SIZE,
                 MPI_INT,
                 0,
                 0,
                 MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
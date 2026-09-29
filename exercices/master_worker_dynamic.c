#include <stdio.h>
#include <mpi.h>

#define TAREFAS 7

/* Etiquetas utilizadas para identificar os tipos de mensagem. */
#define REQUEST 1
#define TASK    2
#define RESULT  3
#define STOP    4

int main(int argc, char *argv[])
{
    int my_rank;
    int proc_n;
    int saco[TAREFAS];
    int message[2]; /* [0]: identificador; [1]: valor. */
    int i;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    /* Precisamos de um mestre e pelo menos um trabalhador. */
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
        int next_task = 0;
        int active_workers = proc_n - 1;

        for (i = 0; i < TAREFAS; i++)
        {
            saco[i] = (i + 1) * 10;
        }

        /* Atende pedidos e recebe resultados de qualquer trabalhador. */
        while (active_workers > 0)
        {
            MPI_Recv(message, 2, MPI_INT,
                     MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == REQUEST)
            {
                if (next_task < TAREFAS)
                {
                    message[0] = next_task;
                    message[1] = saco[next_task];

                    printf("Mestre: tarefa %d, valor %d -> trabalhador %d.\n",
                           message[0], message[1], status.MPI_SOURCE);

                    MPI_Send(message, 2, MPI_INT,
                             status.MPI_SOURCE, TASK, MPI_COMM_WORLD);

                    next_task++;
                }
                else
                {
                    /* Sem novas tarefas: encerra este trabalhador. */
                    MPI_Send(NULL, 0, MPI_INT,
                             status.MPI_SOURCE, STOP, MPI_COMM_WORLD);

                    printf("Mestre: STOP -> trabalhador %d.\n",
                           status.MPI_SOURCE);

                    active_workers--;
                }
            }
            else if (status.MPI_TAG == RESULT)
            {
                /* Usa o identificador da tarefa para guardar o resultado. */
                saco[message[0]] = message[1];

                printf("Mestre: resultado da tarefa %d = %d, trabalhador %d.\n",
                       message[0], message[1], status.MPI_SOURCE);
            }
        }

        printf("Saco final:");

        for (i = 0; i < TAREFAS; i++)
        {
            printf(" %d", saco[i]);
        }

        printf("\n");
    }
    else
    {
        while (1)
        {
            /* Pede uma tarefa ao mestre. */
            MPI_Send(NULL, 0, MPI_INT, 0, REQUEST, MPI_COMM_WORLD);

            /* Recebe uma tarefa ou uma mensagem de encerramento. */
            MPI_Recv(message, 2, MPI_INT, 0, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == STOP)
            {
                break;
            }

            printf("Trabalhador %d: tarefa %d, valor recebido %d.\n",
                   my_rank, message[0], message[1]);

            /* Executa o trabalho. */
            message[1] = message[1] + 1;

            /* Retorna o identificador e o resultado ao mestre. */
            MPI_Send(message, 2, MPI_INT, 0, RESULT, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();

    return 0;
}

#include <stdio.h>
#include <mpi.h>

#define TAREFAS 7 /* Número de tarefas no saco de trabalho. */

int main(int argc, char *argv[])
{
    int my_rank;       /* Identificador deste processo. */
    int proc_n;        /* Número total de processos MPI. */
    int message;       /* Buffer para enviar e receber um inteiro. */
    int saco[TAREFAS]; /* Saco de trabalho. */
    int i;             /* Índice utilizado nos laços. */
    MPI_Status status; /* Informações sobre a mensagem recebida. */

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    /* Este exemplo exige um mestre e um trabalhador por tarefa. */
    if (proc_n != TAREFAS + 1)
    {
        if (my_rank == 0)
        {
            printf("Este programa precisa de %d processos.\n",
                   TAREFAS + 1);
        }

        MPI_Finalize();
        return 1;
    }

    if (my_rank == 0)
    {
        /* Inicializa o saco com os valores 10, 20, ..., 70. */
        for (i = 0; i < TAREFAS; i++)
        {
            saco[i] = (i + 1) * 10;
        }

        /* Envia uma tarefa para cada trabalhador. */
        for (i = 0; i < TAREFAS; i++)
        {
            message = saco[i];

            printf("Mestre: enviando %d ao trabalhador %d.\n",
                   message, i + 1);

            MPI_Send(&message, 1, MPI_INT, i + 1, 0,
                     MPI_COMM_WORLD);
        }

        /* Recebe os resultados sem fixar a ordem dos emissores. */
        for (i = 0; i < TAREFAS; i++)
        {
            MPI_Recv(&message, 1, MPI_INT,
                     MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            /* Guarda o resultado na posição original da tarefa. */
            saco[status.MPI_SOURCE - 1] = message;

            printf("Mestre: recebeu %d do trabalhador %d.\n",
                   message, status.MPI_SOURCE);
        }

        /* Exibe o saco após receber todos os resultados. */
        printf("Saco final:");

        for (i = 0; i < TAREFAS; i++)
        {
            printf(" %d", saco[i]);
        }

        printf("\n");
    }
    else
    {
        /* Recebe uma tarefa do mestre. */
        MPI_Recv(&message, 1, MPI_INT, 0, 0,
                 MPI_COMM_WORLD, &status);

        printf("Trabalhador %d: recebeu %d.\n", my_rank, message);

        /* Executa o trabalho. */
        message = message + 1;

        /* Devolve o resultado ao mestre. */
        MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}

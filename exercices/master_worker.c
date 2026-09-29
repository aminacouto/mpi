#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int my_rank;
    int proc_n;
    int message;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    /* Este exemplo precisa de exatamente dois processos. */
    if (proc_n != 2)
    {
        if (my_rank == 0)
        {
            printf("Execute este programa com exatamente 2 processos.\n");
        }

        MPI_Finalize();
        return 1;
    }

    if (my_rank == 0)
    {
        /* O coordenador envia um número ao trabalhador. */
        message = 20;

        printf("Coordenador: enviando %d ao trabalhador.\n", message);

        MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        /* Recebe o resultado do trabalhador. */
        MPI_Recv(&message, 1, MPI_INT, 1, 0,
                 MPI_COMM_WORLD, &status);

        printf("Coordenador: resultado recebido = %d.\n", message);
    }
    else
    {
        /* O trabalhador recebe o número do coordenador. */
        MPI_Recv(&message, 1, MPI_INT, 0, 0,
                 MPI_COMM_WORLD, &status);

        printf("Trabalhador: valor recebido = %d.\n", message);

        /* Realiza o trabalho e devolve o resultado. */
        message = message + 1;

        MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}

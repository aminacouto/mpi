#include <stdio.h>
#include <mpi.h>

/*
 * Cada processo exibe seu rank e o número total de processos.
 * A ordem de exibição das mensagens pode variar.
 */

int main(int argc, char *argv[])
{
    int my_rank;
    int proc_n;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    printf("Olá! Sou o processo %d de um total de %d processos.\n",
           my_rank, proc_n);

    MPI_Finalize();

    return 0;
}

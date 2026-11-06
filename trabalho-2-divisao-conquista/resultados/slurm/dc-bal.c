/* 
 * Compilar: mpicc dc-balc.c -lm -o dc-balc
 * O tamanho do vetor e informado como argumento na execucao.
 */
// Versão com Bubble Sort paralelo, balanceada.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

void bs(int n, int *vetor);
void merge(int a[], int size, int temp[]);
int my_topmost_level_mpi(int my_rank);
void dc_parallel_mpi(int a[], int size, int temp[], int level,
                     int my_rank, int max_rank, int tag, MPI_Comm comm);
void run_root_mpi(int a[], int size, int temp[], int max_rank,
                  int tag, MPI_Comm comm);
void run_helper_mpi(int my_rank, int max_rank, int tag, MPI_Comm comm);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int comm_size;
    int my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int max_rank = comm_size - 1;
    int tag = 123;

    if (my_rank == 0)
    {
        if (argc != 2)
        {
            fprintf(stderr, "Uso: %s tamanho_do_vetor\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int size = atoi(argv[1]);
        if (size <= 0 || size < comm_size)
        {
            fprintf(stderr, "O tamanho deve ser positivo e nao inferior ao numero de processos.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int *a = malloc((size_t)size * sizeof(*a));
        int *temp = malloc((size_t)size * sizeof(*temp));
        if (a == NULL || temp == NULL)
        {
            fprintf(stderr, "Erro ao alocar memoria para o vetor.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Ordem decrescente
        for (int i = 0; i < size; i++)
            a[i] = size - i;

        // Mede a distribuicao, a ordenacao e as intercalacoes.
        // Inicializacao, verificacao e impressoes ficam fora da medicao.
        double inicio = MPI_Wtime();
        run_root_mpi(a, size, temp, max_rank, tag, MPI_COMM_WORLD);
        double fim = MPI_Wtime();

        // Para a entrada N, N-1, ..., 1, o resultado deve ser 1, 2, ..., N.
        for (int i = 0; i < size; i++)
        {
            if (a[i] != i + 1)
            {
                fprintf(stderr, "Erro na ordenacao: posicao %d, valor %d.\n", i, a[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }

        printf("Ordenacao concluida: %d elementos.\n", size);
        printf("Processos MPI: %d\n", comm_size);
        printf("Tempo da ordenação paralela: %.6f segundos\n", fim - inicio);
        printf("Verificação: vetor ordenado.\n");

        free(temp);
        free(a);
    }
    else
    {
        run_helper_mpi(my_rank, max_rank, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

// A raiz inicia a divisao do vetor inteiro.
void run_root_mpi(int a[], int size, int temp[], int max_rank,
                  int tag, MPI_Comm comm)
{
    dc_parallel_mpi(a, size, temp, 0, 0, max_rank, tag, comm);
}

// Cada auxiliar recebe um bloco, processa e devolve ao processo que o enviou.
void run_helper_mpi(int my_rank, int max_rank, int tag, MPI_Comm comm)
{
    int level = my_topmost_level_mpi(my_rank);
    MPI_Status status;
    int size;

    MPI_Probe(MPI_ANY_SOURCE, tag, comm, &status);
    MPI_Get_count(&status, MPI_INT, &size);
    int parent_rank = status.MPI_SOURCE;

    int *a = malloc((size_t)size * sizeof(*a));
    int *temp = malloc((size_t)size * sizeof(*temp));
    if (a == NULL || temp == NULL)
    {
        fprintf(stderr, "Erro ao alocar memoria no processo %d.\n", my_rank);
        MPI_Abort(comm, 1);
    }

    MPI_Recv(a, size, MPI_INT, parent_rank, tag, comm, &status);
    dc_parallel_mpi(a, size, temp, level, my_rank, max_rank, tag, comm);
    MPI_Send(a, size, MPI_INT, parent_rank, tag, comm);

    free(temp);
    free(a);
}

// Determina em qual nivel o processo comeca a participar da divisao.
int my_topmost_level_mpi(int my_rank)
{
    int level = 0;
    while (pow(2, level) <= my_rank)
        level++;
    return level;
}

void dc_parallel_mpi(int a[], int size, int temp[], int level,
                     int my_rank, int max_rank, int tag, MPI_Comm comm)
{
    int helper_rank = my_rank + (int)pow(2, level);

    if (helper_rank > max_rank)
    {
        // Sem outro processo disponivel, ordena o bloco local inteiro.
        bs(size, a);
    }
    else
    {
        MPI_Request request;
        MPI_Status status;
        int metade = size / 2;

        // Envia a segunda metade e continua trabalhando na primeira.
        MPI_Isend(a + metade, size - metade, MPI_INT,
                  helper_rank, tag, comm, &request);
        dc_parallel_mpi(a, metade, temp, level + 1,
                         my_rank, max_rank, tag, comm);

        // Garante que o envio terminou antes de reutilizar a segunda metade.
        MPI_Wait(&request, MPI_STATUS_IGNORE);
        MPI_Recv(a + metade, size - metade, MPI_INT,
                 helper_rank, tag, comm, &status);

        // Junta as duas metades ja ordenadas.
        merge(a, size, temp);
    }
}

// Bubble Sort usado em cada bloco local.
void bs(int n, int *vetor)
{
    int trocou = 1;
    for (int c = 0; c < n - 1 && trocou; c++)
    {
        trocou = 0;
        for (int d = 0; d < n - c - 1; d++)
        {
            if (vetor[d] > vetor[d + 1])
            {
                int troca = vetor[d];
                vetor[d] = vetor[d + 1];
                vetor[d + 1] = troca;
                trocou = 1;
            }
        }
    }
}

// Intercala duas metades ordenadas e copia o resultado para o vetor.
void merge(int a[], int size, int temp[])
{
    int i1 = 0;
    int i2 = size / 2;
    int tempi = 0;

    while (i1 < size / 2 && i2 < size)
    {
        if (a[i1] <= a[i2])
            temp[tempi++] = a[i1++];
        else
            temp[tempi++] = a[i2++];
    }
    while (i1 < size / 2)
        temp[tempi++] = a[i1++];
    while (i2 < size)
        temp[tempi++] = a[i2++];

    memcpy(a, temp, (size_t)size * sizeof(*a));
}

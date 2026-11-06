// Versão paralela com Bubble Sort, desbalanceada.
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Teste árvore de 63
#define VETOR_SIZE 1000000
#define delta 31250

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

int *alocar_vetor(int tamanho)
{
    // Reserva espaço para a quantidade de elementos atribuída ao processo (cada processo possui sua própria região de memória)
    int *vetor = malloc((size_t)tamanho * sizeof(*vetor));

    if (vetor == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória para o vetor.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    return vetor;
}

// Rotina de intercalação
// Recebe as duas metades ordenadas pelos filhos e devolve um novo vetor ordenado.
int *interleaving(int vetor[], int tam)
{
    int *vetor_auxiliar;
    int i1, i2, i_aux;

    vetor_auxiliar = (int *)malloc(sizeof(int) * tam);
    if (vetor_auxiliar == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória para a intercalação.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    i1 = 0;
    i2 = tam / 2;
    // Percorre as duas metades e copia o menor elemento para o vetor auxiliar.
    for (i_aux = 0; i_aux < tam; i_aux++)
    {
        if ((i2 == tam) || ((i1 < tam / 2) && (vetor[i1] <= vetor[i2])))
            vetor_auxiliar[i_aux] = vetor[i1++];
        else
            vetor_auxiliar[i_aux] = vetor[i2++];
    }

    return vetor_auxiliar;
}

int main(int argc, char **argv)
{
int my_rank, processos, tam_vetor;
double tempo_inicio = 0.0;
double tempo_fim;
MPI_Status Status;
// Inicializa a comunicação MPI e obtém a identificação e quantidade de processos.
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
MPI_Comm_size(MPI_COMM_WORLD, &processos);

// Define os ranks do pai e dos filhos na árvore binária de processos.
// Reserva capacidade para receber o vetor; o tamanho real vem de MPI_Get_count.
int *vetor = alocar_vetor(VETOR_SIZE);
int pai = my_rank == 0 ? -1 : (my_rank - 1) / 2;
int filho_esquerda = 2 * my_rank + 1;
int filho_direita = 2 * my_rank + 2;  // pega pega o numero do processo atual (rank)

// recebo vetor

if ( my_rank != 0 )
   {
   MPI_Recv(vetor, VETOR_SIZE, MPI_INT, pai, 0, MPI_COMM_WORLD, &Status);                     // não sou a raiz, tenho pai
   MPI_Get_count(&Status, MPI_INT, &tam_vetor);  // descubro tamanho da mensagem recebida
   }
else
   {
   tam_vetor = VETOR_SIZE;               // defino tamanho inicial do vetor
   for (int i = 0; i < tam_vetor; i++)
       vetor[i] = tam_vetor - i;      // sou a raiz e portanto gero o vetor - ordem reversa
   // Mede a ordenação paralela, incluindo comunicação e intercalação.
   tempo_inicio = MPI_Wtime();
   }

// dividir ou conquistar?

if ( tam_vetor <= delta )
   bs(tam_vetor, vetor);  // conquisto
else
       {
    // dividir
    // Só divide se houver processos disponíveis para os dois filhos.
    if (filho_direita >= processos)
    {
        fprintf(stderr, "Processos insuficientes para a árvore.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    // quebrar em duas partes e mandar para os filhos
    // printf("rank %d divide %d elementos: %d -> rank %d, %d -> rank %d\n",
        //    my_rank, tam_vetor, tam_vetor / 2, filho_esquerda,
        //    tam_vetor - tam_vetor / 2, filho_direita);
    // fflush(stdout);

    MPI_Send(vetor, tam_vetor / 2, MPI_INT, filho_esquerda, 0, MPI_COMM_WORLD);  // mando metade inicial do vetor
    MPI_Send(&vetor[tam_vetor / 2], tam_vetor - tam_vetor / 2, MPI_INT, filho_direita, 0, MPI_COMM_WORLD);  // mando metade final

    // receber dos filhos

    MPI_Recv(vetor, tam_vetor / 2, MPI_INT, filho_esquerda, 0, MPI_COMM_WORLD, &Status);            
    MPI_Recv(&vetor[tam_vetor / 2], tam_vetor - tam_vetor / 2, MPI_INT, filho_direita, 0, MPI_COMM_WORLD, &Status);   

    // intercalo vetor inteiro
 
    int *vetor_auxiliar; // Resultado alocado dentro da rotina de intercalação.
    vetor_auxiliar = interleaving(vetor, tam_vetor);
    free(vetor);
    vetor = vetor_auxiliar;
    }

// mando para o pai

if ( my_rank !=0 )
   MPI_Send(vetor, tam_vetor, MPI_INT, pai, 0, MPI_COMM_WORLD);  // tenho pai, retorno vetor ordenado pra ele
else
   {
   tempo_fim = MPI_Wtime();
   printf("Ordenação concluída: %d elementos.\n", tam_vetor); // sou a raiz
   printf("Tempo da ordenação paralela: %.6f segundos\n", tempo_fim - tempo_inicio);
   }

free(vetor);
MPI_Finalize();
return EXIT_SUCCESS;
}
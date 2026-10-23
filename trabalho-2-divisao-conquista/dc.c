#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


#define VETOR_SIZE 20000
#define delta 5000


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
    int *vetor = malloc((size_t)tamanho * sizeof(*vetor));

    if (vetor == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória para o vetor.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    return vetor;
}

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
    for (i_aux = 0; i_aux < tam; i_aux++)
    {
        // Mesmo modelo do enunciado, com os limites verificados antes dos acessos.
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
MPI_Status Status;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
MPI_Comm_size(MPI_COMM_WORLD, &processos);
// Calcula o tamanho deste rank seguindo o caminho até a raiz.
int caminho[32], nivel = 0;
for (int r = my_rank; r > 0; r = (r - 1) / 2)
    caminho[nivel++] = r;
tam_vetor = VETOR_SIZE;
while (nivel > 0)
{
    int r = caminho[--nivel];
    if (tam_vetor <= delta)
    {
        MPI_Finalize();
        return EXIT_SUCCESS; // Rank excedente, sem tarefa.
    }
    tam_vetor = r % 2 ? tam_vetor / 2 : tam_vetor - tam_vetor / 2;
}
int *vetor = alocar_vetor(tam_vetor);
int pai = my_rank == 0 ? -1 : (my_rank - 1) / 2;
int filho_esquerda = 2 * my_rank + 1;
int filho_direita = 2 * my_rank + 2;  // pega pega o numero do processo atual (rank)

// recebo vetor

if ( my_rank != 0 )
   {
   MPI_Recv(vetor, tam_vetor, MPI_INT, pai, 0, MPI_COMM_WORLD, &Status);                       // não sou a raiz, tenho pai
   MPI_Get_count(&Status, MPI_INT, &tam_vetor);  // descubro tamanho da mensagem recebida
   }
else
   {
   tam_vetor = VETOR_SIZE;               // defino tamanho inicial do vetor
   for (int i = 0; i < tam_vetor; i++)
       vetor[i] = tam_vetor - i;      // sou a raiz e portanto gero o vetor - ordem reversa
   }

// dividir ou conquistar?

if ( tam_vetor <= delta )
   bs(tam_vetor, vetor);  // conquisto
else
       {
    // dividir
    if (filho_direita >= processos)
    {
        fprintf(stderr, "Processos insuficientes para a árvore.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    // quebrar em duas partes e mandar para os filhos

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
   printf("Ordenação concluída: %d elementos.\n", tam_vetor); // sou a raiz

free(vetor);
MPI_Finalize();
return EXIT_SUCCESS;
}
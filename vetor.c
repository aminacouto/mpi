// Versão 1: escravos solicitam trabalho ao mestre, que distribui dinamicamente as tarefas.

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//#define TAREFAS 64
//#define ARRAY_SIZE 10000

// Para permitir que TAREFAS e ARRAY_SIZE sejam definidos externamente pelo testes.sh.
#ifndef TAREFAS
#define TAREFAS 64
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE 10000
#endif

// Tipos de mensagens
#define TAG_PEDIDO 100
#define TAG_TRABALHO 200
#define TAG_RESULTADO 300
#define TAG_FIM 400


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


int main(int argc, char *argv[])
{
    int my_rank;
    int proc_n;

    double tempo_inicio;
    double tempo_fim;

    int i, j;

    // Próxima posição do saco que será enviada
    int proxima_tarefa = 0;

    // Número de tarefas já concluídas
    int tarefas_concluidas = 0;

    // Número de escravos que já receberam TAG_FIM
    int escravos_finalizados = 0;

    // Variável usada apenas para solicitar trabalho
    int pedido = 1;

    MPI_Status status;


    // Inicializa o MPI
    MPI_Init(&argc, &argv);

    // Descobre o rank deste processo
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Descobre quantos processos existem
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);


    // Cada posição guarda qual tarefa está sendo executada por cada escravo.
    int tarefa_do_escravo[proc_n];

    // =====================================================
    // ALOCAÇÃO DE MEMÓRIA
    // =====================================================

    // Cada processo precisa apenas de um vetor para comunicação.
    int *message = malloc((size_t)ARRAY_SIZE * sizeof(int));

    if (message == NULL)
    {
        fprintf(stderr,
                "Erro ao alocar memória para message no processo %d.\n",
                my_rank);

        MPI_Abort(MPI_COMM_WORLD, 1);
    }


    // Somente o mestre precisa armazenar o saco inteiro.
    int *saco = NULL;

    if (my_rank == 0)
    {
        saco = malloc((size_t)TAREFAS *
                      ARRAY_SIZE *
                      sizeof(int));

        if (saco == NULL)
        {
            fprintf(stderr,
                    "Erro ao alocar memória para o saco de trabalho.\n");

            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Sincroniza todos os processos antes da medição
    MPI_Barrier(MPI_COMM_WORLD);

    tempo_inicio = MPI_Wtime();

    // =====================================================
    // MESTRE
    // =====================================================

    if (my_rank == 0)
    {
        // Cria os vetores do saco de trabalho
        for (i = 0; i < TAREFAS; i++)
        {
            for (j = 0; j < ARRAY_SIZE; j++)
            {
                saco[i * ARRAY_SIZE + j] =
                    ARRAY_SIZE - j + (i * ARRAY_SIZE);
            }
        }


        // Distribuição dinâmica das tarefas
        while (escravos_finalizados < proc_n - 1)
        {
            // O mestre recebe uma mensagem de qualquer escravo.
            MPI_Recv(message,
                     ARRAY_SIZE,
                     MPI_INT,
                     MPI_ANY_SOURCE,
                     MPI_ANY_TAG,
                     MPI_COMM_WORLD,
                     &status);


            // Identifica qual escravo enviou a mensagem
            int escravo = status.MPI_SOURCE;


            // =============================================
            // ESCRAVO ESTÁ PEDINDO TRABALHO
            // =============================================

            if (status.MPI_TAG == TAG_PEDIDO)
            {
                // Ainda existem tarefas no saco
                if (proxima_tarefa < TAREFAS)
                {
                    // Registra qual tarefa será executada por este escravo.
                    tarefa_do_escravo[escravo] =
                        proxima_tarefa;


                    // Envia somente o vetor correspondente.
                    MPI_Send(
                        &saco[proxima_tarefa * ARRAY_SIZE],
                        ARRAY_SIZE,
                        MPI_INT,
                        escravo,
                        TAG_TRABALHO,
                        MPI_COMM_WORLD
                    );

                    proxima_tarefa++;
                }

                // Não existem mais tarefas para distribuir
                else
                {
                    // Envia mensagem de término.
                    MPI_Send(message,
                             1,
                             MPI_INT,
                             escravo,
                             TAG_FIM,
                             MPI_COMM_WORLD);

                    escravos_finalizados++;
                }
            }


            // =============================================
            // ESCRAVO ESTÁ DEVOLVENDO UM RESULTADO
            // =============================================

            else if (status.MPI_TAG == TAG_RESULTADO)
            {
                // Descobre qual tarefa estava sendo
                // executada por esse escravo.
                int tarefa_id =
                    tarefa_do_escravo[escravo];


                // Guarda o vetor recebido na posição
                // original correspondente dentro do saco.
                for (j = 0; j < ARRAY_SIZE; j++)
                {
                    saco[tarefa_id * ARRAY_SIZE + j] =
                        message[j];
                }

                tarefas_concluidas++;
            }
        }

        printf("\nTarefas concluídas: %d de %d\n",
               tarefas_concluidas,
               TAREFAS);
    }


    // =====================================================
    // ESCRAVOS
    // =====================================================

    else
    {
        while (1)
        {
            // Solicita uma tarefa ao mestre
            MPI_Send(&pedido,
                     1,
                     MPI_INT,
                     0,
                     TAG_PEDIDO,
                     MPI_COMM_WORLD);


            // Recebe uma resposta do mestre.
            MPI_Recv(message,
                     ARRAY_SIZE,
                     MPI_INT,
                     0,
                     MPI_ANY_TAG,
                     MPI_COMM_WORLD,
                     &status);


            // Se recebeu TAG_FIM, não há mais trabalho
            if (status.MPI_TAG == TAG_FIM)
            {
                break;
            }

            // Ordena o vetor recebido
            bs(ARRAY_SIZE, message);

            // Devolve o vetor ordenado ao mestre
            MPI_Send(message,
                     ARRAY_SIZE,
                     MPI_INT,
                     0,
                     TAG_RESULTADO,
                     MPI_COMM_WORLD);
        }
    }


    // Aguarda todos os processos concluírem antes de finalizar a medição.
    MPI_Barrier(MPI_COMM_WORLD);

    tempo_fim = MPI_Wtime();

    // Mestre mostra o tempo total
    if (my_rank == 0)
    {
        printf("Tempo total do processamento paralelo: %.6f segundos\n",
               tempo_fim - tempo_inicio);
    }

    // =====================================================
    // LIBERAÇÃO DA MEMÓRIA
    // =====================================================

    free(message);

    if (my_rank == 0)
    {
        free(saco);
    }

    // Finaliza o MPI
    MPI_Finalize();

    return 0;
}
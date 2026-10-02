#include <stdio.h>
#include <mpi.h>

#define TAREFAS 10
#define ARRAY_SIZE 10000

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

    // Saco de trabalho
    int saco[TAREFAS][ARRAY_SIZE];

    // Vetor utilizado nas comunicações
    int message[ARRAY_SIZE];

    int i, j;

    // Próxima posição do saco que será enviada
    int proxima_tarefa = 0;

    // Número de tarefas já concluídas
    int tarefas_concluidas = 0;

    // Número de escravos que já receberam TAG_FIM
    int escravos_finalizados = 0;

    // Identificador da tarefa
    int tarefa_id;

    // Variável usada apenas para solicitar trabalho
    int pedido = 1;

    MPI_Status status;


    // Inicializa o MPI
    MPI_Init(&argc, &argv);

    // Descobre o rank deste processo
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Descobre quantos processos existem
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);


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
                saco[i][j] = ARRAY_SIZE - j + (i * ARRAY_SIZE);
            }
        }

        // Distribuição dinâmica
        while (escravos_finalizados < proc_n - 1)
        {
            // Recebe uma mensagem de qualquer escravo
            MPI_Recv(&tarefa_id,
                     1,
                     MPI_INT,
                     MPI_ANY_SOURCE,
                     MPI_ANY_TAG,
                     MPI_COMM_WORLD,
                     &status);


            // Guarda quem enviou a mensagem
            int escravo = status.MPI_SOURCE; // guardar a tag no proprio vetor ou no saco, não em uma variavel, para manter a ordem do saco


            // =============================================
            // ESCRAVO ESTÁ PEDINDO TRABALHO
            // =============================================

            if (status.MPI_TAG == TAG_PEDIDO)
            {
                // Ainda existe trabalho no saco
                if (proxima_tarefa < TAREFAS)
                {
                    tarefa_id = proxima_tarefa;

                    // Envia o identificador da tarefa - não é necessário enviar o vetor inteiro, apenas o identificador
                    MPI_Send(&tarefa_id,
                             1,
                             MPI_INT,
                             escravo,
                             TAG_TRABALHO,
                             MPI_COMM_WORLD);

                    // Envia o vetor
                    MPI_Send(saco[tarefa_id],
                             ARRAY_SIZE,
                             MPI_INT,
                             escravo,
                             TAG_TRABALHO,
                             MPI_COMM_WORLD);

                    printf("Mestre enviou tarefa %d para escravo %d\n",
                           tarefa_id,
                           escravo);

                    proxima_tarefa++;
                }

                // Não existem mais tarefas para distribuir
                else
                {
                    MPI_Send(&tarefa_id,
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
                // Recebe o vetor ordenado
                MPI_Recv(message,
                         ARRAY_SIZE,
                         MPI_INT,
                         escravo,
                         TAG_RESULTADO,
                         MPI_COMM_WORLD,
                         &status);

                // Guarda o resultado na posição correta do saco de trabalho
                for (j = 0; j < ARRAY_SIZE; j++)
                {
                    saco[tarefa_id][j] = message[j];
                }

                tarefas_concluidas++;

                printf("Mestre recebeu tarefa %d do escravo %d\n",
                       tarefa_id,
                       escravo);
            }
        }

        printf("\nTarefas concluídas: %d de %d\n", tarefas_concluidas, TAREFAS);
        // Mostra o saco depois de todas as tarefas, para array size pequeno
        //printf("\nSaco final:\n");

        //for (i = 0; i < TAREFAS; i++)
        //{
            //printf("Tarefa %d: ", i);

           // for (j = 0; j < ARRAY_SIZE; j++)
           // {
               // printf("%d ", saco[i][j]);
           // }

           // printf("\n");
       // }
    }


    // =====================================================
    // ESCRAVOS
    // =====================================================

    else
    {
        while (1)
        {
           // printf("Escravo %d solicitando tarefa\n", my_rank);
            // Solicita uma tarefa ao mestre
            MPI_Send(&pedido,
                     1,
                     MPI_INT,
                     0,
                     TAG_PEDIDO,
                     MPI_COMM_WORLD);


            // Espera a resposta do mestre
            MPI_Recv(&tarefa_id,
                     1,
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


            // Recebe o vetor que deve ordenar
            MPI_Recv(message,
                     ARRAY_SIZE,
                     MPI_INT,
                     0,
                     TAG_TRABALHO,
                     MPI_COMM_WORLD,
                     &status);


            //printf("Escravo %d recebeu tarefa %d\n",
            //       my_rank,
            //       tarefa_id);


            // Ordena o vetor
            bs(ARRAY_SIZE, message);


            // Envia o identificador da tarefa concluída
            MPI_Send(&tarefa_id,
                     1,
                     MPI_INT,
                     0,
                     TAG_RESULTADO,
                     MPI_COMM_WORLD);


            // Envia o vetor ordenado
            MPI_Send(message,
                     ARRAY_SIZE,
                     MPI_INT,
                     0,
                     TAG_RESULTADO,
                     MPI_COMM_WORLD);
        }
    }


    // Finaliza o MPI
    MPI_Finalize();

    return 0;
}
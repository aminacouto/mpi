// Versão 2: Mestre envia as tarefas iniciais aos escravos. Após finalizar uma tarefa, cada escravo solicita novo trabalho ao mestre.

#include <stdio.h>
#include <mpi.h>

#define TAREFAS 64
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

    double tempo_inicio;
    double tempo_fim;

    // Saco de trabalho
    int saco[TAREFAS][ARRAY_SIZE];

    // Vetor utilizado nas comunicações
    int message[ARRAY_SIZE];

    int i, j;

    // Próxima tarefa disponível no saco
    int proxima_tarefa = 0;

    // Número de tarefas já concluídas
    int tarefas_concluidas = 0;

    // Número de escravos que receberam TAG_FIM
    int escravos_finalizados = 0;

    // Variável usada pelo escravo para solicitar nova tarefa
    int pedido = 1;

    MPI_Status status;


    // Inicializa o MPI
    MPI_Init(&argc, &argv);

    // Descobre o rank deste processo
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Descobre quantos processos existem
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);


    // Guarda qual tarefa está sendo executada por cada escravo
    int tarefa_do_escravo[proc_n];

    for (i = 0; i < proc_n; i++)
    {
        tarefa_do_escravo[i] = -1;
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
                saco[i][j] = ARRAY_SIZE - j + (i * ARRAY_SIZE);
            }
        }


        // =================================================
        // DISTRIBUIÇÃO INICIAL
        // =================================================

        // O mestre entrega uma primeira tarefa para cada escravo disponível
        for (i = 1; i < proc_n; i++)
        {
            if (proxima_tarefa < TAREFAS)
            {
                // Registra qual tarefa foi entregue ao escravo
                tarefa_do_escravo[i] = proxima_tarefa;


                // Envia o vetor diretamente para o escravo
                MPI_Send(saco[proxima_tarefa],
                         ARRAY_SIZE,
                         MPI_INT,
                         i,
                         TAG_TRABALHO,
                         MPI_COMM_WORLD);


                //printf("Mestre enviou tarefa inicial %d para escravo %d\n",
                  //     proxima_tarefa,
                    //   i);


                proxima_tarefa++;
            }

            // Caso existam mais escravos do que tarefas
            else
            {
                MPI_Send(message,
                         1,
                         MPI_INT,
                         i,
                         TAG_FIM,
                         MPI_COMM_WORLD);

                escravos_finalizados++;
            }
        }


        // =================================================
        // DISTRIBUIÇÃO DINÂMICA
        // =================================================

        while (escravos_finalizados < proc_n - 1)
        {
            // Recebe mensagem de qualquer escravo
            MPI_Recv(message,
                     ARRAY_SIZE,
                     MPI_INT,
                     MPI_ANY_SOURCE,
                     MPI_ANY_TAG,
                     MPI_COMM_WORLD,
                     &status);


            // Identifica o escravo que enviou a mensagem
            int escravo = status.MPI_SOURCE;


            // =============================================
            // ESCRAVO DEVOLVEU UM RESULTADO
            // =============================================

            if (status.MPI_TAG == TAG_RESULTADO)
            {
                // Descobre qual tarefa estava com esse escravo
                int tarefa_id = tarefa_do_escravo[escravo];


                // Guarda o resultado na posição original da tarefa dentro do saco
                for (j = 0; j < ARRAY_SIZE; j++)
                {
                    saco[tarefa_id][j] = message[j];
                }


                tarefas_concluidas++;


                //printf("Mestre recebeu tarefa %d do escravo %d\n",
                  //     tarefa_id,
                    //   escravo);
            }


            // =============================================
            // ESCRAVO SOLICITA NOVA TAREFA
            // =============================================

            else if (status.MPI_TAG == TAG_PEDIDO)
            {
                // Ainda existem tarefas no saco
                if (proxima_tarefa < TAREFAS)
                {
                    // Registra qual nova tarefa será executada por esse escravo
                    tarefa_do_escravo[escravo] = proxima_tarefa;


                    // Envia o próximo vetor
                    MPI_Send(saco[proxima_tarefa],
                             ARRAY_SIZE,
                             MPI_INT,
                             escravo,
                             TAG_TRABALHO,
                             MPI_COMM_WORLD);


                   // printf("Mestre enviou tarefa %d para escravo %d\n",
                     //      proxima_tarefa,
                       //    escravo);


                    proxima_tarefa++;
                }

                // Não existem mais tarefas disponíveis
                else
                {
                    MPI_Send(message,
                             1,
                             MPI_INT,
                             escravo,
                             TAG_FIM,
                             MPI_COMM_WORLD);


                    escravos_finalizados++;
                }
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
        // =================================================
        // PRIMEIRA TAREFA
        // =================================================

        // Espera o mestre enviar.
        MPI_Recv(message,
                 ARRAY_SIZE,
                 MPI_INT,
                 0,
                 MPI_ANY_TAG,
                 MPI_COMM_WORLD,
                 &status);


        // Caso existam mais escravos do que tarefas
        if (status.MPI_TAG != TAG_FIM)
        {
            while (1)
            {
                // Ordena o vetor recebido
                bs(ARRAY_SIZE, message);


                // Devolve o resultado ao mestre
                MPI_Send(message,
                         ARRAY_SIZE,
                         MPI_INT,
                         0,
                         TAG_RESULTADO,
                         MPI_COMM_WORLD);


                // Depois de terminar a tarefa, solicita novo trabalho
                MPI_Send(&pedido,
                         1,
                         MPI_INT,
                         0,
                         TAG_PEDIDO,
                         MPI_COMM_WORLD);


                // Espera nova tarefa ou mensagem de fim
                MPI_Recv(message,
                         ARRAY_SIZE,
                         MPI_INT,
                         0,
                         MPI_ANY_TAG,
                         MPI_COMM_WORLD,
                         &status);


                // Não existem mais tarefas
                if (status.MPI_TAG == TAG_FIM)
                {
                    break;
                }
            }
        }
    }


    // Aguarda todos os processos concluírem antes de finalizar a medição
    MPI_Barrier(MPI_COMM_WORLD);

    tempo_fim = MPI_Wtime();


    // Mestre mostra o tempo total
    if (my_rank == 0)
    {
        printf("Tempo total do processamento paralelo: %.6f segundos\n",
               tempo_fim - tempo_inicio);
    }


    // Finaliza o MPI
    MPI_Finalize();

    return 0;
}
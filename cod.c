#define TAREFAS 7 // Numero de tarefas no saco de trabalho para np = 8, processo 0 é o mestre

int my_rank;       // Identificador deste processo
int proc_n;        // Numero de processos disparados pelo usuário na linha de comando (np)
int message;       // Buffer para as mensagens 
int saco[TAREFAS]; // saco de trabalho


MPI_Init(); // funcao que inicializa o MPI, todo o código paralelo esta abaixo

my_rank = MPI_Comm_rank();  // pega pega o numero do processo atual (rank)
proc_n  = MPI_Comm_size();  // pega informação do numero de processos (quantidade total)

if ( my_rank == 0 ) // qual o meu papel: sou o mestre ou um dos escravos?
   {
   // papel do mestre

   for ( i=0 ; i < TAREFAS ; i++) // mando o trabalho para os escravos fazerem
       {
       message = saco[i];
       MPI_Send(&message, i+1); // envio trabalho saco[i] para escravo com id = i+1;
       } 

    // recebo o resultado

    for ( i=0 ; i < TAREFAS ; i++)
        {
        // recebo mensagens de qualquer emissor e com qualquer etiqueta (TAG)

        MPI_Recv(&message, MPI_ANY_SOURCE, MPI_ANY_TAG, status);  // recebo por ordem de chegada com any_source

        saco[status.MPI_SOURCE-1] = message;   // coloco mensagem no saco na posição do escravo emissor
        }
     }              
else               
     {
     // papel do escravo

     MPI_Recv(&message, 0);    // recebo do mestre

     message = message+1;      // icremento conteúdo da mensagem

     MPI_Send(&message, 0);    // retorno resultado para o mestre
     }

MPI_Finalize();
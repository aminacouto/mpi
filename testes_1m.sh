#!/bin/bash

#SBATCH --export=ALL
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --no-requeue
#SBATCH -J mpi_1m
#SBATCH -o mpi_1m.%j.out

set -euo pipefail

TAREFAS=64
ARRAY_SIZE=1000000
RESULTADOS="resultados_1m.txt"

TESTE="${1:-}"

if [[ -z "$TESTE" ]]; then
    echo "Uso: sbatch testes_1m.sh {v1_16|v1_32|v2_16|v2_32}"
    exit 1
fi

registrar() {
    echo "$1" | tee -a "$RESULTADOS"
}

registrar ""
registrar "##################################################"
registrar "Data: $(date)"
registrar "Tarefas: $TAREFAS"
registrar "ARRAY_SIZE: $ARRAY_SIZE"
registrar "Teste: $TESTE"
registrar "##################################################"

case "$TESTE" in

    v1_16)
        mpicc -DTAREFAS=$TAREFAS -DARRAY_SIZE=$ARRAY_SIZE vetor.c -o vetor

        registrar ""
        registrar "=================================================="
        registrar "Versão: Paralela V1"
        registrar "Nós: 2"
        registrar "Processos MPI: 16"
        registrar "Processos por nó: 8"
        registrar "Núcleos físicos utilizados: 16"
        registrar "HT: Não"
        registrar "Escravos: 15"
        registrar "Tarefas: $TAREFAS"
        registrar "ARRAY_SIZE: $ARRAY_SIZE"
        registrar "Data: $(date)"
        registrar "=================================================="

        srun -N 2 -n 16 \
            --ntasks-per-node=8 \
            --cpus-per-task=1 \
            --cpu-bind=cores \
            ./vetor 2>&1 | tee -a "$RESULTADOS"
        ;;

    v1_32)
        mpicc -DTAREFAS=$TAREFAS -DARRAY_SIZE=$ARRAY_SIZE vetor.c -o vetor

        registrar ""
        registrar "=================================================="
        registrar "Versão: Paralela V1"
        registrar "Nós: 2"
        registrar "Processos MPI: 32"
        registrar "Processos por nó: 16"
        registrar "Núcleos físicos utilizados: 16"
        registrar "Threads utilizadas: 32"
        registrar "HT: Sim"
        registrar "Escravos: 31"
        registrar "Tarefas: $TAREFAS"
        registrar "ARRAY_SIZE: $ARRAY_SIZE"
        registrar "Data: $(date)"
        registrar "=================================================="

        srun -N 2 -n 32 \
            --ntasks-per-node=16 \
            --cpus-per-task=1 \
            --cpu-bind=threads \
            ./vetor 2>&1 | tee -a "$RESULTADOS"
        ;;

    v2_16)
        mpicc -DTAREFAS=$TAREFAS -DARRAY_SIZE=$ARRAY_SIZE vetor.v2.c -o vetor.v2

        registrar ""
        registrar "=================================================="
        registrar "Versão: Paralela V2"
        registrar "Nós: 2"
        registrar "Processos MPI: 16"
        registrar "Processos por nó: 8"
        registrar "Núcleos físicos utilizados: 16"
        registrar "HT: Não"
        registrar "Escravos: 15"
        registrar "Tarefas: $TAREFAS"
        registrar "ARRAY_SIZE: $ARRAY_SIZE"
        registrar "Data: $(date)"
        registrar "=================================================="

        srun -N 2 -n 16 \
            --ntasks-per-node=8 \
            --cpus-per-task=1 \
            --cpu-bind=cores \
            ./vetor.v2 2>&1 | tee -a "$RESULTADOS"
        ;;

    v2_32)
        mpicc -DTAREFAS=$TAREFAS -DARRAY_SIZE=$ARRAY_SIZE vetor.v2.c -o vetor.v2

        registrar ""
        registrar "=================================================="
        registrar "Versão: Paralela V2"
        registrar "Nós: 2"
        registrar "Processos MPI: 32"
        registrar "Processos por nó: 16"
        registrar "Núcleos físicos utilizados: 16"
        registrar "Threads utilizadas: 32"
        registrar "HT: Sim"
        registrar "Escravos: 31"
        registrar "Tarefas: $TAREFAS"
        registrar "ARRAY_SIZE: $ARRAY_SIZE"
        registrar "Data: $(date)"
        registrar "=================================================="

        srun -N 2 -n 32 \
            --ntasks-per-node=16 \
            --cpus-per-task=1 \
            --cpu-bind=threads \
            ./vetor.v2 2>&1 | tee -a "$RESULTADOS"
        ;;

    *)
        echo "Teste inválido: $TESTE"
        echo "Use: v1_16, v1_32, v2_16 ou v2_32"
        exit 1
        ;;
esac

registrar ""
registrar "Fim do teste: $(date)"
#!/bin/bash

# ============================================================
# Testes de desempenho - MPI Mestre/Escravo
# ============================================================

TAREFAS=64

TAMANHOS=(10000)

# Depois:
# TAMANHOS=(100000)
# TAMANHOS=(1000000)

REPETICOES=3

RESULTADOS="resultados.txt"


# ============================================================
# PREPARA O ARQUIVO DE RESULTADOS
# ============================================================

if [ -f "$RESULTADOS" ]; then

    ULTIMO_TESTE=$(grep -o "TESTE [0-9]*" "$RESULTADOS" \
        | tail -1 \
        | awk '{print $2}')

    if [ -n "$ULTIMO_TESTE" ]; then
        TESTE=$((ULTIMO_TESTE + 1))
    else
        TESTE=1
    fi

else

    TESTE=1

    echo "==================================================" > "$RESULTADOS"
    echo "RESULTADOS DOS TESTES MPI" >> "$RESULTADOS"
    echo "==================================================" >> "$RESULTADOS"
    echo "Tarefas: $TAREFAS" >> "$RESULTADOS"
    echo "Repetições por configuração: $REPETICOES" >> "$RESULTADOS"
    echo >> "$RESULTADOS"

    # Topologia conhecida do cluster Atlântica
    echo "CONFIGURAÇÃO DO CLUSTER" >> "$RESULTADOS"
    echo "Sockets por nó: 2" >> "$RESULTADOS"
    echo "Núcleos por socket: 4" >> "$RESULTADOS"
    echo "Núcleos físicos por nó: 8" >> "$RESULTADOS"
    echo "Threads por núcleo: 2" >> "$RESULTADOS"
    echo "CPUs lógicas por nó: 16" >> "$RESULTADOS"
    echo "==================================================" >> "$RESULTADOS"
    echo >> "$RESULTADOS"

fi


# ============================================================
# LOOP PELOS TAMANHOS
# ============================================================

for ARRAY_SIZE in "${TAMANHOS[@]}"
do

    echo
    echo "=================================================="
    echo "Preparando ARRAY_SIZE = $ARRAY_SIZE"
    echo "=================================================="


    {
        echo
        echo "##################################################"
        echo "Data: $(date)"
        echo "Tarefas: $TAREFAS"
        echo "ARRAY_SIZE: $ARRAY_SIZE"
        echo "Repetições: $REPETICOES"
        echo "##################################################"
        echo
    } | tee -a "$RESULTADOS"


    # ========================================================
    # COMPILAÇÃO
    # ========================================================

    echo "Compilando versão paralela V1..."

    mpicc \
        -DTAREFAS=$TAREFAS \
        -DARRAY_SIZE=$ARRAY_SIZE \
        vetor.c \
        -o vetor

    if [ $? -ne 0 ]; then
        echo "Erro ao compilar vetor.c"
        exit 1
    fi


    echo "Compilando versão paralela V2..."

    mpicc \
        -DTAREFAS=$TAREFAS \
        -DARRAY_SIZE=$ARRAY_SIZE \
        vetor.v2.c \
        -o vetor.v2

    if [ $? -ne 0 ]; then
        echo "Erro ao compilar vetor.v2.c"
        exit 1
    fi


    echo "Compilando versão sequencial..."

    mpicc \
        -DTAREFAS=$TAREFAS \
        -DARRAY_SIZE=$ARRAY_SIZE \
        sequencial.c \
        -o sequencial

    if [ $? -ne 0 ]; then
        echo "Erro ao compilar sequencial.c"
        exit 1
    fi


    echo "Compilação concluída."
    echo


    # ========================================================
    # SEQUENCIAL
    # ========================================================

    for REP in $(seq 1 $REPETICOES)
    do

        {
            echo "=================================================="
            echo "TESTE $TESTE"
            echo "Versão: Sequencial"
            echo "Execução: $REP de $REPETICOES"
            echo "Nós: 1"
            echo "Processos MPI: 1"
            echo "Núcleos físicos utilizados: 1"
            echo "HT: Não"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive \
                -N 1 \
                -n 1 \
                --cpus-per-task=1 \
                --cpu-bind=cores \
                ./sequencial

            echo

        } | tee -a "$RESULTADOS"

        TESTE=$((TESTE + 1))

    done


    # ========================================================
    # PARALELA V1 - 16 PROCESSOS
    # ========================================================

    for REP in $(seq 1 $REPETICOES)
    do

        {
            echo "=================================================="
            echo "TESTE $TESTE"
            echo "Versão: Paralela V1"
            echo "Execução: $REP de $REPETICOES"
            echo "Nós: 2"
            echo "Processos MPI: 16"
            echo "Processos por nó: 8"
            echo "Núcleos físicos utilizados: 16"
            echo "HT: Não"
            echo "Escravos: 15"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive \
                -N 2 \
                -n 16 \
                --ntasks-per-node=8 \
                --cpus-per-task=1 \
                --cpu-bind=cores \
                ./vetor

            echo

        } | tee -a "$RESULTADOS"

        TESTE=$((TESTE + 1))

    done


    # ========================================================
    # PARALELA V1 - 32 PROCESSOS
    # ========================================================

    for REP in $(seq 1 $REPETICOES)
    do

        {
            echo "=================================================="
            echo "TESTE $TESTE"
            echo "Versão: Paralela V1"
            echo "Execução: $REP de $REPETICOES"
            echo "Nós: 2"
            echo "Processos MPI: 32"
            echo "Processos por nó: 16"
            echo "Núcleos físicos utilizados: 16"
            echo "Threads utilizadas: 32"
            echo "HT: Sim"
            echo "Escravos: 31"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive \
                -N 2 \
                -n 32 \
                --ntasks-per-node=16 \
                --cpus-per-task=1 \
                --cpu-bind=threads \
                ./vetor

            echo

        } | tee -a "$RESULTADOS"

        TESTE=$((TESTE + 1))

    done


    # ========================================================
    # PARALELA V2 - 16 PROCESSOS
    # ========================================================

    for REP in $(seq 1 $REPETICOES)
    do

        {
            echo "=================================================="
            echo "TESTE $TESTE"
            echo "Versão: Paralela V2"
            echo "Execução: $REP de $REPETICOES"
            echo "Nós: 2"
            echo "Processos MPI: 16"
            echo "Processos por nó: 8"
            echo "Núcleos físicos utilizados: 16"
            echo "HT: Não"
            echo "Escravos: 15"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive \
                -N 2 \
                -n 16 \
                --ntasks-per-node=8 \
                --cpus-per-task=1 \
                --cpu-bind=cores \
                ./vetor.v2

            echo

        } | tee -a "$RESULTADOS"

        TESTE=$((TESTE + 1))

    done


    # ========================================================
    # PARALELA V2 - 32 PROCESSOS
    # ========================================================

    for REP in $(seq 1 $REPETICOES)
    do

        {
            echo "=================================================="
            echo "TESTE $TESTE"
            echo "Versão: Paralela V2"
            echo "Execução: $REP de $REPETICOES"
            echo "Nós: 2"
            echo "Processos MPI: 32"
            echo "Processos por nó: 16"
            echo "Núcleos físicos utilizados: 16"
            echo "Threads utilizadas: 32"
            echo "HT: Sim"
            echo "Escravos: 31"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive \
                -N 2 \
                -n 32 \
                --ntasks-per-node=16 \
                --cpus-per-task=1 \
                --cpu-bind=threads \
                ./vetor.v2

            echo

        } | tee -a "$RESULTADOS"

        TESTE=$((TESTE + 1))

    done

done


# ============================================================
# FINALIZAÇÃO
# ============================================================

echo
echo "=================================================="
echo "TESTES FINALIZADOS"
echo "Resultados salvos em: $RESULTADOS"
echo "=================================================="
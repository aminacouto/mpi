#!/bin/bash

# ============================================================
# Testes de desempenho - MPI Mestre/Escravo
#
# TAREFAS fixas: 64
#
# ARRAY_SIZE:
#   10000
#   100000
#   1000000
#
# Paralelo:
#   2 nós
#   16 processos
#   32 processos
#
# Cada configuração será executada 3 vezes.
# ============================================================


# Quantidade fixa de tarefas
TAREFAS=64


# Tamanho atualmente testado
TAMANHOS=(10000)

# Depois, executar separadamente:
# TAMANHOS=(100000)
# TAMANHOS=(1000000)


# Quantidade de repetições de cada configuração
REPETICOES=3


# Arquivo onde os resultados serão armazenados
RESULTADOS="resultados.txt"


# ============================================================
# PREPARA O ARQUIVO DE RESULTADOS
# ============================================================

# Se o arquivo já existe, continua a numeração dos testes
if [ -f "$RESULTADOS" ]; then

    ULTIMO_TESTE=$(grep -o "TESTE [0-9]*" "$RESULTADOS" \
        | tail -1 \
        | awk '{print $2}')

    if [ -n "$ULTIMO_TESTE" ]; then
        TESTE=$((ULTIMO_TESTE + 1))
    else
        TESTE=1
    fi

# Se ainda não existe, cria o arquivo
else

    TESTE=1

    echo "==================================================" > "$RESULTADOS"
    echo "RESULTADOS DOS TESTES MPI" >> "$RESULTADOS"
    echo "==================================================" >> "$RESULTADOS"
    echo "Tarefas: $TAREFAS" >> "$RESULTADOS"
    echo "Repetições por configuração: $REPETICOES" >> "$RESULTADOS"
    echo "==================================================" >> "$RESULTADOS"
    echo >> "$RESULTADOS"

fi


# ============================================================
# LOOP PELOS TAMANHOS DE VETOR
# ============================================================

for ARRAY_SIZE in "${TAMANHOS[@]}"
do

    echo
    echo "=================================================="
    echo "Preparando ARRAY_SIZE = $ARRAY_SIZE"
    echo "=================================================="


    # Registra o início deste conjunto de testes
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
    # TESTE SEQUENCIAL
    # ========================================================

    for REP in $(seq 1 $REPETICOES)
    do

        {
            echo "=================================================="
            echo "TESTE $TESTE"
            echo "Versão: Sequencial"
            echo "Execução: $REP de $REPETICOES"
            echo "Nós: 1"
            echo "Processos: 1"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive -N 1 -n 1 ./sequencial

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
            echo "Processos: 16"
            echo "Escravos: 15"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive -N 2 -n 16 ./vetor

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
            echo "Processos: 32"
            echo "Escravos: 31"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive -N 2 -n 32 ./vetor

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
            echo "Processos: 16"
            echo "Escravos: 15"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive -N 2 -n 16 ./vetor.v2

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
            echo "Processos: 32"
            echo "Escravos: 31"
            echo "Tarefas: $TAREFAS"
            echo "ARRAY_SIZE: $ARRAY_SIZE"
            echo "Data: $(date)"
            echo "=================================================="

            srun --exclusive -N 2 -n 32 ./vetor.v2

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
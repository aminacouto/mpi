#!/bin/bash
# TPP2 - Medições com e sem Hyper-Threading
# Árvore fixa: 1 processos, 1.000.0003 elementos e delta = 62500
# sem HT: sbatch --job-name=tpp2-semht --exclusive -N 2 -n 16 --hint=nomultithread --time=08:00:00 medicoes-ht.sh semht
# com HT: sbatch --job-name=tpp2-comht --exclusive -N 2 -n 32 --hint=multithread --time=08:00:00 medicoes-ht.sh comht

set -euo pipefail

modo=${1:-semht}
repeticoes=3

if [[ "$modo" != "semht" && "$modo" != "comht" ]]; then
    echo "Utilize: semht ou comht"
    exit 1
fi

cd "$SLURM_SUBMIT_DIR"
mkdir -p resultados

relatorio="resultados/ht_${modo}_${SLURM_JOB_ID}.txt"
executavel="dc_ht_${SLURM_JOB_ID}"

trap 'rm -f "$executavel"' EXIT

# Compila com as mesmas opções dos experimentos anteriores
mpicc dc.c -o "$executavel"

if [[ "$modo" == "semht" ]]; then
    hint=nomultithread
    overcommit=(--overcommit)
else
    hint=multithread
    overcommit=()
fi

echo "Experimento: $modo" | tee "$relatorio"
echo "Processos MPI: 31" | tee -a "$relatorio"
echo "Vetor: 1000000" | tee -a "$relatorio"
echo "Delta: 62500" | tee -a "$relatorio"
echo "Nós: $SLURM_JOB_NODELIST" | tee -a "$relatorio"

tempos=()

for ((i=1; i<=repeticoes; i++)); do
    echo "Repetição $i" | tee -a "$relatorio"

    saida=$(srun "${overcommit[@]}" \
        -N 2 -n 31 \
        --hint="$hint" \
        --distribution=cyclic \
        "./$executavel")

    echo "$saida" | tee -a "$relatorio"

    tempo=$(echo "$saida" | awk '/Tempo da ordena/ {print $(NF-1)}')

    [[ "$tempo" =~ ^[0-9]+([.][0-9]+)?$ ]] || {
        echo "Erro ao obter tempo da repetição $i"
        exit 1
    }

    tempos+=("$tempo")
done

media=$(printf '%s\n' "${tempos[@]}" |
    awk '{soma+=$1; n++} END {printf "%.6f", soma/n}')

echo "TEMPO MÉDIO: $media s" | tee -a "$relatorio"
echo "Medições concluídas." | tee -a "$relatorio"

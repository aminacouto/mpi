#!/bin/bash
# Medições do TPP2. Executar com sbatch a partir do diretório das fontes.
# Argumentos: modo (mpi, seq, bal, semht ou comht), tamanho do vetor e repetições.
# dc-bal sem HT (4, 8 e 16 processos):
# sbatch --exclusive -N 2 -n 16 --hint=nomultithread --time=08:00:00 medicoes.sh bal 1000000 3
# dc-bal sem HT (32 processos compartilhando 16 nucleos fisicos):
# sbatch --exclusive -N 2 -n 16 --hint=nomultithread --time=08:00:00 medicoes.sh semht 1000000 3
# dc-bal com HT (32 processos):
# sbatch --exclusive -N 2 -n 32 --hint=multithread --time=08:00:00 medicoes.sh comht 1000000 3
# Paralelo:   sbatch --exclusive -N 2 -n 16 --hint=nomultithread --time=08:00:00 medicoes.sh mpi 1000000 3
# Sequencial: sbatch --exclusive -N 1 -n 1 --hint=nomultithread --time=16:00:00 medicoes.sh seq 1000000 3
# Fontes: dc.c, dc-bal.c, bs-sequencial.c e qs-sequencial.c.
# mpi: dc.c com 3, 7 e 15 processos, sem HT.
# seq: Bubble Sort e qsort sequenciais.
# bal: dc-bal.c com 4, 8 e 16 processos, sem HT.
# semht: dc-bal.c com 32 processos, sem HT, usando --overcommit.
# comht: dc-bal.c com 32 processos, usando HT.
# Os experimentos anteriores de dc.c com 31 processos continuam separados.
set -euo pipefail
export LC_ALL=C
modo=${1:-mpi}
tamanho=${2:-10000}
repeticoes=${3:-3}
[[ -n ${SLURM_JOB_ID:-} ]] || { echo 'Execute com sbatch.' >&2; exit 1; }
[[ $modo == mpi || $modo == seq || $modo == bal || $modo == semht || $modo == comht ]] || {
    echo 'Utilize mpi, seq, bal, semht ou comht.' >&2; exit 1;
}
[[ $tamanho =~ ^[1-9][0-9]*$ && $repeticoes =~ ^[1-9][0-9]*$ ]] || { echo 'Informe inteiros positivos.' >&2; exit 1; }
(( tamanho >= 8 && tamanho % 8 == 0 )) || { echo 'O tamanho deve ser múltiplo de oito.' >&2; exit 1; }
cd "${SLURM_SUBMIT_DIR:?}"
fonte_mpi=${MPI_SOURCE:-$PWD/dc.c}
fonte_balanceada="$PWD/dc-bal.c"
diretorio_sequencial=${SEQ_DIR:-$PWD}
if [[ $modo == mpi ]]; then
    [[ $SLURM_JOB_NUM_NODES == 2 ]] || { echo 'Reserve dois nós para MPI.' >&2; exit 1; }
    [[ -f $fonte_mpi ]] || { echo "Fonte não encontrada: $fonte_mpi" >&2; exit 1; }
elif [[ $modo == bal || $modo == semht || $modo == comht ]]; then
    [[ $SLURM_JOB_NUM_NODES == 2 ]] || { echo 'Reserve dois nós para MPI.' >&2; exit 1; }
    [[ -f $fonte_balanceada ]] || { echo "Fonte não encontrada: $fonte_balanceada" >&2; exit 1; }
    (( tamanho >= 32 )) || { echo 'Utilize um vetor com pelo menos 32 elementos.' >&2; exit 1; }
    if [[ $modo == comht ]]; then
        (( ${SLURM_NTASKS:-0} >= 32 )) || { echo 'Reserve 32 processos para o modo comht.' >&2; exit 1; }
    else
        (( ${SLURM_NTASKS:-0} >= 16 )) || { echo 'Reserve 16 processos para o modo sem HT.' >&2; exit 1; }
    fi
else
    for versao in bs qs; do
        [[ -f $diretorio_sequencial/${versao}-sequencial.c ]] || { echo "Fonte não encontrada: ${versao}-sequencial.c" >&2; exit 1; }
    done
fi

# Salva um relatório por modo e tamanho. Uma nova execução substitui o anterior.
mkdir -p resultados
case "$tamanho" in
    10000) rotulo=10k ;;
    100000) rotulo=100k ;;
    1000000) rotulo=1M ;;
    *) rotulo=$tamanho ;;
esac
case "$modo" in
    semht) nome_relatorio=bal_semht ;;
    comht) nome_relatorio=bal_comht ;;
    *) nome_relatorio=$modo ;;
esac
relatorio="$PWD/resultados/${nome_relatorio}_${rotulo}.txt"
# Usa o diretório compartilhado para disponibilizar os executáveis nos dois nós.
# As cópias e os executáveis são removidos ao terminar; as fontes ficam intactas.
temporario=$(mktemp -d "$PWD/.medicoes_XXXXXX")
trap 'rm -rf -- "$temporario"' EXIT
exec > >(tee "$relatorio") 2>&1
trap 'echo "ERRO na linha $LINENO. Medições incompletas; não considerar tempos ausentes como zero."' ERR
printf '========================================\nRESULTADOS DAS MEDIÇÕES — TPP2\n========================================\n'
printf 'Elementos: %s\nRepetições: %s\n' "$tamanho" "$repeticoes"
printf 'Data e hora: %s\n' "$(TZ=America/Sao_Paulo date '+%d/%m/%Y %H:%M:%S')"

# Atualiza uma única definição em uma cópia do código, nunca no original.
alterar_macro() {
    local arquivo=$1 macro=$2 valor=$3 quantidade
    quantidade=$(awk -v nome="$macro" '$1=="#define" && $2==nome {n++} END {print n+0}' "$arquivo")
    [[ $quantidade == 1 ]] || { echo "Macro $macro ausente ou duplicada." >&2; exit 1; }
    awk -v nome="$macro" -v valor="$valor" '$1=="#define" && $2==nome {print "#define " nome " " valor; next} {print}' "$arquivo" > "$arquivo.tmp"
    mv "$arquivo.tmp" "$arquivo"
}

# O programa C mede o tempo; o script apenas organiza e registra as execuções.
executar_testes() {
    local nome=$1 limite=$2 processos=$3 nos=$4 executavel=$5
    shift 5
    local repeticao registro tempo media
    local hint=nomultithread
    local opcoes_srun=(--exclusive -c 1 --cpu-bind=cores)
    if [[ $modo == comht ]]; then
        hint=multithread
        opcoes_srun=(--exclusive -c 1 --cpu-bind=threads)
    elif [[ $modo == semht ]]; then
        # Usa a mesma forma de lancamento do roteiro sem HT anterior.
        # Nao exige uma CPU exclusiva para cada um dos 32 processos.
        opcoes_srun=(--overcommit)
    fi
    local tempos=()
    printf '\n----------------------------------------\nVersão: %s\n' "$nome"
    printf 'Processos: %s\nNós utilizados: %s\nDelta: %s\n' "$processos" "$nos" "$limite"
    if [[ $modo == comht ]]; then
        echo 'HT: utilizado'
    else
        echo 'HT: não utilizado'
    fi
    for ((repeticao=1; repeticao<=repeticoes; repeticao++)); do
        registro="$temporario/saida.txt"
        if ! srun "${opcoes_srun[@]}" -N "$nos" -n "$processos" \
            --hint="$hint" --distribution=cyclic \
            "$executavel" "$@" > "$registro" 2>&1; then
            cat "$registro"
            echo 'ERRO: execução interrompida.' >&2
            return 1
        fi
        # dc-bal já verifica o resultado; exige a confirmação antes de registrar o tempo.
        if [[ $modo == bal || $modo == semht || $modo == comht ]]; then
            grep -Fq 'Verificação: vetor ordenado.' "$registro" || {
                cat "$registro"; echo 'ERRO: ordenação não confirmada.' >&2; return 1;
            }
        fi
        tempo=$(awk '/Tempo da ordena/ {print $(NF-1)}' "$registro")
        [[ $tempo =~ ^[0-9]+([.][0-9]+)?$ ]] || { cat "$registro"; echo 'ERRO: tempo ausente ou inválido.' >&2; return 1; }
        printf 'Repetição %d: %s s\n' "$repeticao" "$tempo"
        tempos+=("$tempo")
    done
    media=$(printf '%s\n' "${tempos[@]}" | awk '{soma+=$1; n++} END {printf "%.9f", soma/n}')
    printf 'Tempo médio: %s s\n' "$media"
}

if [[ $modo == mpi ]]; then
    cp "$fonte_mpi" "$temporario/original.c"
    for processos in 3 7 15; do
        folhas=$(((processos+1)/2))
        limite=$((tamanho/folhas))
        arquivo="$temporario/mpi.c"
        cp "$temporario/original.c" "$arquivo"
        alterar_macro "$arquivo" VETOR_SIZE "$tamanho"
        alterar_macro "$arquivo" delta "$limite"
        mpicc "$arquivo" -o "$temporario/programa"
        executar_testes 'MPI — Bubble Sort' "$limite" "$processos" 2 "$temporario/programa"
    done
elif [[ $modo == bal || $modo == semht || $modo == comht ]]; then
    # Esta versão recebe o tamanho como argumento; não utiliza delta.
    cp "$fonte_balanceada" "$temporario/dc-bal.c"
    mpicc "$temporario/dc-bal.c" -lm -o "$temporario/programa"
    if [[ $modo == bal ]]; then
        configuracoes=(4 8 16)
    else
        configuracoes=(32)
    fi
    for processos in "${configuracoes[@]}"; do
        executar_testes 'MPI — Bubble Sort com balanceamento' 'não se aplica' \
            "$processos" 2 "$temporario/programa" "$tamanho"
    done
else
    for versao in bs qs; do
        arquivo="$temporario/sequencial.c"
        cp "$diretorio_sequencial/${versao}-sequencial.c" "$arquivo"
        alterar_macro "$arquivo" ARRAY_SIZE "$tamanho"
        mpicc "$arquivo" -o "$temporario/programa"
        if [[ $versao == bs ]]; then nome='Bubble Sort sequencial'; else nome='qsort sequencial'; fi
        executar_testes "$nome" 'não se aplica' 1 1 "$temporario/programa"
    done
fi
echo "Medições concluídas."

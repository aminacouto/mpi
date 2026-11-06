#!/bin/bash
# TPP2 - um experimento por job, 1.000.000 de elementos.
# Para mudar de experimento: altere a seção SELEÇÃO e, se necessário,
# comente/descomente as diretivas #SBATCH do perfil correto abaixo.

# PERFIL SEQUENCIAL (ativo inicialmente)
#SBATCH --job-name=tpp2-1M
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=16:00:00
#SBATCH --exclusive
#SBATCH --hint=nomultithread
#SBATCH --output=slurm-%j.out

# PERFIL MPI (para ativar: comente --nodes=1, --ntasks=1, --time=16:00:00
# acima e retire o espaço entre # e SBATCH nas três linhas abaixo)
# SBATCH --nodes=2
# SBATCH --ntasks=16
# SBATCH --time=08:00:00

set -euo pipefail
export LC_ALL=C
cd "${SLURM_SUBMIT_DIR:?Execute com sbatch}"

# ===== SELEÇÃO: deixe APENAS UMA linha experimento= ativa =====
# experimento=bs
experimento=qs
# experimento=mpi3
# experimento=mpi7
# experimento=mpi15
# ================================================================

TAMANHO=1000000
REPETICOES=3
mkdir -p resultados
RELATORIO="resultados/${experimento}_1M_${SLURM_JOB_ID}.txt"
exec > >(tee "$RELATORIO") 2>&1

case "$experimento" in
  bs)    fonte="bs-sequencial.c"; macro="ARRAY_SIZE"; delta=""; tarefas=1; nos=1 ;;
  qs)    fonte="qs-sequencial.c"; macro="ARRAY_SIZE"; delta=""; tarefas=1; nos=1 ;;
  mpi3)  fonte="dc.c"; macro="VETOR_SIZE"; delta=500000; tarefas=3; nos=2 ;;
  mpi7)  fonte="dc.c"; macro="VETOR_SIZE"; delta=250000; tarefas=7; nos=2 ;;
  mpi15) fonte="dc.c"; macro="VETOR_SIZE"; delta=125000; tarefas=15; nos=2 ;;
  *) echo "Experimento inválido: $experimento" >&2; exit 1 ;;
esac

[[ -f "$fonte" ]] || { echo "Arquivo não encontrado: $fonte" >&2; exit 1; }
[[ "${SLURM_JOB_NUM_NODES}" -eq "$nos" ]] || { echo "ERRO: este experimento precisa de $nos nó(s). Ajuste as diretivas #SBATCH." >&2; exit 1; }
[[ "${SLURM_NTASKS:-0}" -ge "$tarefas" ]] || { echo "ERRO: reserve pelo menos $tarefas tarefas via #SBATCH." >&2; exit 1; }

TEMP=$(mktemp -d "$PWD/.tpp2_XXXXXX")
trap 'rm -rf -- "$TEMP"' EXIT
cp "$fonte" "$TEMP/fonte.c"

alterar_macro() {
  local nome="$1" valor="$2" arquivo="$TEMP/fonte.c"
  local n
  n=$(awk -v m="$nome" '$1=="#define" && $2==m {n++} END{print n+0}' "$arquivo")
  [[ "$n" -eq 1 ]] || { echo "ERRO: #define $nome ausente ou duplicado em $fonte" >&2; exit 1; }
  awk -v m="$nome" -v v="$valor" '$1=="#define" && $2==m {$0="#define " m " " v} {print}' "$arquivo" > "$TEMP/novo.c"
  mv "$TEMP/novo.c" "$arquivo"
}

alterar_macro "$macro" "$TAMANHO"
if [[ -n "$delta" ]]; then alterar_macro delta "$delta"; fi

# Mesma política de compilação para as três versões.
mpicc -O2 -Wall -Wextra "$TEMP/fonte.c" -o "$TEMP/programa"

printf 'Experimento: %s\nElementos: %d\nDelta: %s\nProcessos MPI: %d\nNós: %d\nRepetições: %d\nJob ID: %s\nOtimização: -O2\n' \
  "$experimento" "$TAMANHO" "${delta:-não se aplica}" "$tarefas" "$nos" "$REPETICOES" "$SLURM_JOB_ID"
scontrol show hostnames "$SLURM_JOB_NODELIST"

soma=0
for ((i=1; i<=REPETICOES; i++)); do
  echo "--- Repetição $i ---"
  srun --exclusive -N "$nos" -n "$tarefas" -c 1 \
    --hint=nomultithread --distribution=cyclic --cpu-bind=cores \
    "$TEMP/programa" > "$TEMP/saida.txt" 2>&1 || { cat "$TEMP/saida.txt"; exit 1; }
  cat "$TEMP/saida.txt"
  tempo=$(awk '/Tempo da ordena/ {print $(NF-1)}' "$TEMP/saida.txt" | tail -n 1)
  [[ "$tempo" =~ ^[0-9]+([.][0-9]+)?$ ]] || { echo 'ERRO: tempo não encontrado ou formato inesperado.' >&2; exit 1; }
  echo "Tempo da repetição $i: $tempo s"
  soma=$(awk -v a="$soma" -v b="$tempo" 'BEGIN {printf "%.9f", a+b}')
done
awk -v soma="$soma" -v n="$REPETICOES" 'BEGIN {printf "TEMPO MÉDIO: %.9f s\n", soma/n}'
echo "Arquivo: $RELATORIO"

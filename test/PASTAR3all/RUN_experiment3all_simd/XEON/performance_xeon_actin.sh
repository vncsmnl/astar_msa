#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../../../../" && pwd)"
SEQ="${ROOT_DIR}/seqs/Balibase/Ref1/8_long_high_id/actin.fasta"

THREADS="-t 14"

SEQ_NAME=$(basename "$SEQ")

OUT_DIR="${SCRIPT_DIR}/results"
mkdir -p "$OUT_DIR"

echo "Processando: Xeon E5-2680 v4"
echo "Sequência: $SEQ_NAME"
echo "Threads: 14"
echo "----------------------------------------"

run_test () {
    MODE=$1
    AFFINITY_ARG=$2
    OUT_FILE="${OUT_DIR}/${SEQ_NAME}.${MODE}.out"

    echo ""
    echo ">>> Rodando modo: $MODE"
    echo "Saída: $OUT_FILE"

    /usr/bin/time -v perf stat -r 3 \
        "${ROOT_DIR}/bin/msa_pastar_simd" \
        ${THREADS} ${AFFINITY_ARG} "$SEQ" \
        >> "$OUT_FILE" 2>&1
}

# ---------------------------
# 1) Without affinity
# ---------------------------
run_test "no_affinity" ""

# ---------------------------
# 2) With affinity
# (physical cores)
# ---------------------------
AFFINITY="--affinity=0,2,4,6,8,10,12,14,16,18,20,22,24,26"
run_test "affinity" "$AFFINITY"

echo ""
echo "Benchmark finalizado."
echo "Compare os arquivos em: $OUT_DIR"

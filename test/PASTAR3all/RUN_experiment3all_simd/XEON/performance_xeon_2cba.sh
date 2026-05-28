#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../../../../" && pwd)"
SEQ="${ROOT_DIR}/seqs/Balibase/Ref1/4_medium_med_id/2cba.fasta"

THREADS="-t 14"

SEQ_NAME=$(basename "$SEQ")

OUT_DIR="${SCRIPT_DIR}/results"
mkdir -p "$OUT_DIR"

run_test () {
    MODE=$1
    AFFINITY_ARG=$2
    BIN_SUFFIX=$3
    OUT_FILE="${OUT_DIR}/${SEQ_NAME}.${BIN_SUFFIX}.${MODE}.out"

    echo ""
    echo "----------------------------------------"
    echo "Sequência: $SEQ_NAME"
    echo "Threads: 14"
    echo "Versão: $BIN_SUFFIX"
    echo "----------------------------------------"
    echo ">>> Rodando modo: $MODE"
    echo "Saída: $OUT_FILE"

    /usr/bin/time -v perf stat -r 3 \
        "${ROOT_DIR}/bin/msa_pastar_${BIN_SUFFIX}" \
        ${THREADS} ${AFFINITY_ARG} "$SEQ" \
        >> "$OUT_FILE" 2>&1
}

# ---------------------------
# 1) Without affinity
# ---------------------------
#for BIN_SUFFIX in 2all 3all simd; do
#    run_test "no_affinity" "" "$BIN_SUFFIX"
#done

# ---------------------------
# 2) With affinity
# (physical cores)
# ---------------------------
AFFINITY="--affinity=0,1,2,3,4,5,6,7,8,9,10,11,12,13"
for BIN_SUFFIX in 2all 3all simd; do
    run_test "affinity" "$AFFINITY" "$BIN_SUFFIX"
done

echo ""
echo "Benchmark finalizado."
echo "Compare os arquivos em: $OUT_DIR"

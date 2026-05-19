#!/bin/bash
SEQ="./seqs/Balibase/Ref1/8_long_high_id/actin.fasta"
THREADS="-t 16"

#Affinity for Intel 13700kf
AFFINITY="--affinity=0,2,4,6,8,10,12,14,16,17,18,19,20,21,22,23"
HYBRID="8,9,8,7"
echo $HYBRID
/usr/bin/time -v perf stat -r 3 ./bin/msa_pastar ${THREADS} ${AFFINITY} --hybrid=${HYBRID} $SEQ >> ${SEQ}.map.${HYBRID}.out 2>&1

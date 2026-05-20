#!/bin/bash
SEQ="./seqs/Balibase/Ref1/4_medium_med_id/2cba.fasta"
THREADS="-t 14"

# affinity for 14 threads, to avoid hyperthreading
AFFINITY="--affinity=0,2,4,6,8,10,12,14,16,18,20,22,24,26"

echo "Xeon E5-2680 v4"
/usr/bin/time -v perf stat -r 3 ./bin/msa_pastar ${THREADS} ${AFFINITY} $SEQ >> ${SEQ}.xeon.out 2>&1

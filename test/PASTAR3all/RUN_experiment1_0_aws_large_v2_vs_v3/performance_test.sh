#!/bin/bash

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/../../.." && pwd)
cd "$REPO_ROOT" || exit 1

SEQS="seqs/Balibase/Ref1/2_short_high_id/1krn.fasta
seqs/Balibase/Ref1/2_short_high_id/1dox.fasta
seqs/Balibase/Ref1/1_short_med_id/1tgxA.fasta
seqs/Balibase/Ref1/1_short_med_id/1aab.fasta
seqs/Balibase/Ref1/2_short_high_id/1fmb.fasta
seqs/Balibase/Ref1/2_short_high_id/2mhr.fasta
seqs/Balibase/Ref1/2_short_high_id/1aho.fasta
seqs/Balibase/Ref1/1_short_med_id/3cyr.fasta
seqs/Balibase/Ref1/1_short_med_id/1csy.fasta
seqs/Balibase/Ref1/0_short_low_id/1tvxA.fasta
seqs/Balibase/Ref1/2_short_high_id/1csp.fasta
seqs/Balibase/Ref1/1_short_med_id/1hpi.fasta
seqs/Balibase/Ref1/4_medium_med_id/1ad2.fasta
seqs/Balibase/Ref1/7_long_medium_id/1adj.fasta
seqs/Balibase/Ref1/5_medium_high_id/1thm.fasta
seqs/Balibase/Ref1/5_medium_high_id/1zin.fasta
seqs/Balibase/Ref1/4_medium_med_id/1pgtA.fasta
seqs/Balibase/Ref1/0_short_low_id/1r69.fasta
seqs/Balibase/Ref1/0_short_low_id/1idy.fasta
seqs/Balibase/Ref1/1_short_med_id/1ycc.fasta
seqs/Balibase/Ref1/2_short_high_id/9rnt.fasta
seqs/Balibase/Ref1/0_short_low_id/2trx.fasta
seqs/Balibase/Ref1/5_medium_high_id/1ar5A.fasta
seqs/Balibase/Ref1/5_medium_high_id/1led.fasta
seqs/Balibase/Ref1/4_medium_med_id/1aym3.fasta
seqs/Balibase/Ref1/4_medium_med_id/1mrj.fasta
seqs/Balibase/Ref1/1_short_med_id/1pfc.fasta
seqs/Balibase/Ref1/4_medium_med_id/1ldg.fasta
seqs/Balibase/Ref1/6_long_low_id/1ped.fasta
seqs/Balibase/Ref1/7_long_medium_id/1fieA.fasta
seqs/Balibase/Ref1/2_short_high_id/2fxb.fasta
seqs/Balibase/Ref1/2_short_high_id/1fkj.fasta
seqs/Balibase/Ref1/0_short_low_id/1ubi.fasta
seqs/Balibase/Ref1/1_short_med_id/451c.fasta
seqs/Balibase/Ref1/2_short_high_id/1plc.fasta
seqs/Balibase/Ref1/3_medium_low_id/3grs.fasta
seqs/Balibase/Ref1/7_long_medium_id/1pkm.fasta
seqs/Balibase/Ref1/8_long_high_id/1ad3.fasta
seqs/Balibase/Ref1/4_medium_med_id/1pii.fasta
seqs/Balibase/Ref1/1_short_med_id/1fjlA.fasta
seqs/Balibase/Ref1/5_medium_high_id/1ppn.fasta
seqs/Balibase/Ref1/5_medium_high_id/1ezm.fasta
seqs/Balibase/Ref1/5_medium_high_id/1psyA.fasta
seqs/Balibase/Ref1/7_long_medium_id/1dlc.fasta
seqs/Balibase/Ref1/6_long_low_id/4enl.fasta
seqs/Balibase/Ref1/5_medium_high_id/1amk.fasta
seqs/Balibase/Ref1/7_long_medium_id/1gowA.fasta
seqs/Balibase/Ref1/4_medium_med_id/1gdoA.fasta
seqs/Balibase/Ref1/7_long_medium_id/1eft.fasta
seqs/Balibase/Ref1/8_long_high_id/3pgm.fasta
seqs/Balibase/Ref1/3_medium_low_id/2hsdA.fasta
seqs/Balibase/Ref1/3_medium_low_id/1uky.fasta
seqs/Balibase/Ref1/0_short_low_id/1wit.fasta
seqs/Balibase/Ref1/8_long_high_id/1rthA.fasta
seqs/Balibase/Ref1/3_medium_low_id/kinase.fasta
seqs/Balibase/Ref1/6_long_low_id/1ajsA.fasta
seqs/Balibase/Ref1/1_short_med_id/1hfh.fasta
seqs/Balibase/Ref1/7_long_medium_id/1bgl.fasta
seqs/Balibase/Ref1/6_long_low_id/1lvl.fasta
seqs/Balibase/Ref1/4_medium_med_id/1ton.fasta
seqs/Balibase/Ref1/5_medium_high_id/5ptp.fasta
seqs/Balibase/Ref1/3_medium_low_id/2pia.fasta
seqs/Balibase/Ref1/5_medium_high_id/1tis.fasta
seqs/Balibase/Ref1/8_long_high_id/1gtr.fasta
seqs/Balibase/Ref1/6_long_low_id/1cpt.fasta
seqs/Balibase/Ref1/3_medium_low_id/1bbt3.fasta
seqs/Balibase/Ref1/7_long_medium_id/1ac5.fasta
seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
seqs/Balibase/Ref1/3_medium_low_id/1sbp.fasta
seqs/Balibase/Ref1/7_long_medium_id/glg.fasta
seqs/Balibase/Ref1/8_long_high_id/actin.fasta
seqs/Balibase/Ref1/7_long_medium_id/2ack.fasta
seqs/Balibase/Ref1/3_medium_low_id/1hvA.fasta
seqs/Balibase/Ref1/4_medium_med_id/2cba.fasta
seqs/Balibase/Ref1/6_long_low_id/2myr.fasta
seqs/Balibase/Ref1/7_long_medium_id/1sesA.fasta
seqs/Balibase/Ref1/7_long_medium_id/arp.fasta
seqs/Balibase/Ref1/8_long_high_id/1gpb.fasta
seqs/Balibase/Ref1/6_long_low_id/gal4.fasta"

# Optional override for quick tests, e.g.:
# SEQS_OVERRIDE="seqs/Balibase/Ref1/2_short_high_id/1krn.fasta" ./performance_test.sh
SEQS_OVERRIDE=${SEQS_OVERRIDE:-}
if [ -n "$SEQS_OVERRIDE" ]; then
	SEQS="$SEQS_OVERRIDE"
fi

#BUCKET_NAME="pastarv3results260804"
# LOCAL_MODE=1 (default): no AWS metadata calls.
# AWS_MODE=1: try to detect EC2 instance type.
AWS_MODE=${AWS_MODE:-0}
AUTO_SHUTDOWN=${AUTO_SHUTDOWN:-0}
USE_PERF=${USE_PERF:-auto}
INSTANCE="local"

if [ "$AWS_MODE" = "1" ]; then
	INSTANCE=""

	# Try ec2-metadata first when available.
	if command -v ec2-metadata >/dev/null 2>&1; then
		INSTANCE=$(ec2-metadata --instance-type 2>/dev/null | awk '{print $2}')
	fi

	# Fallback to IMDSv2/IMDSv1 when ec2-metadata is unavailable.
	if [ -z "$INSTANCE" ] && command -v curl >/dev/null 2>&1; then
		TOKEN=$(curl -sS -m 2 -X PUT "http://169.254.169.254/latest/api/token" \
			-H "X-aws-ec2-metadata-token-ttl-seconds: 60" 2>/dev/null)

		if [ -n "$TOKEN" ]; then
			INSTANCE=$(curl -sS -m 2 -H "X-aws-ec2-metadata-token: $TOKEN" \
				"http://169.254.169.254/latest/meta-data/instance-type" 2>/dev/null)
		else
			INSTANCE=$(curl -sS -m 2 \
				"http://169.254.169.254/latest/meta-data/instance-type" 2>/dev/null)
		fi
	fi

	if [ -z "$INSTANCE" ]; then
		INSTANCE="null"
	fi
fi

# Optional manual override, e.g. THREAD_LIST="32 16 8" ./performance_test.sh
THREAD_LIST=${THREAD_LIST:-}
if [ -z "$THREAD_LIST" ]; then
	CPU_COUNT=$(getconf _NPROCESSORS_ONLN 2>/dev/null)
	case "$CPU_COUNT" in
		''|*[!0-9]*) CPU_COUNT=1 ;;
	esac
	if [ "$CPU_COUNT" -lt 1 ]; then
		CPU_COUNT=1
	fi

	THREAD_LIST=""
	for th in 64 32 16 8 4 2 1; do
		if [ "$th" -le "$CPU_COUNT" ]; then
			THREAD_LIST="$THREAD_LIST $th"
		fi
	done
	THREAD_LIST=$(echo "$THREAD_LIST" | xargs)
fi

if [ -z "$THREAD_LIST" ]; then
	echo "Erro: THREAD_LIST ficou vazio. Defina manualmente, ex.: THREAD_LIST=\"8 4 2 1\""
	exit 1
fi

for BIN in ./bin/msa_pastar_v1 ./bin/msa_pastar_v2 ./bin/msa_pastar_v3; do
	if [ ! -x "$BIN" ]; then
		echo "Erro: binario nao encontrado ou sem permissao de execucao: $BIN"
		exit 1
	fi
done

PERF_ENABLED=0
if [ "$USE_PERF" = "1" ]; then
	PERF_ENABLED=1
elif [ "$USE_PERF" = "auto" ] && command -v perf >/dev/null 2>&1; then
	if perf stat -e task-clock -a -- sleep 0 >/dev/null 2>&1; then
		PERF_ENABLED=1
	fi
fi

echo "INSTANCE=$INSTANCE"
echo "THREAD_LIST=$THREAD_LIST"
echo "PERF_ENABLED=$PERF_ENABLED"

RESULTS_DIR="results"
mkdir -p "$RESULTS_DIR"

echo $INSTANCE > "$RESULTS_DIR/instance.txt"
for SEQ in $SEQS; do
	if [ ! -f "$SEQ" ]; then
		echo "Aviso: arquivo de entrada nao encontrado, pulando: $SEQ"
		continue
	fi

	OUT_BASE="$RESULTS_DIR/$SEQ"
	mkdir -p "$(dirname "$OUT_BASE")"

	for th in $THREAD_LIST; do
		if [ "$PERF_ENABLED" = "1" ]; then
			/usr/bin/time -v perf stat -r 3 ./bin/msa_pastar_v1 -t $th $SEQ >> ${OUT_BASE}.v1.th${th}.out 2>&1
			/usr/bin/time -v perf stat -r 3 ./bin/msa_pastar_v2 -t $th $SEQ >> ${OUT_BASE}.v2.th${th}.out 2>&1
			#aws s3 cp ${OUT_BASE}.v2.th${th}.out s3://$BUCKET_NAME/$INSTANCE/
			/usr/bin/time -v perf stat -r 3 ./bin/msa_pastar_v3 -t $th $SEQ >> ${OUT_BASE}.v3.th${th}.out 2>&1
			#aws s3 cp ${OUT_BASE}.v3.th${th}.out s3://$BUCKET_NAME/$INSTANCE/
		else
			/usr/bin/time -v ./bin/msa_pastar_v1 -t $th $SEQ >> ${OUT_BASE}.v1.th${th}.out 2>&1
			/usr/bin/time -v ./bin/msa_pastar_v2 -t $th $SEQ >> ${OUT_BASE}.v2.th${th}.out 2>&1
			#aws s3 cp ${OUT_BASE}.v2.th${th}.out s3://$BUCKET_NAME/$INSTANCE/
			/usr/bin/time -v ./bin/msa_pastar_v3 -t $th $SEQ >> ${OUT_BASE}.v3.th${th}.out 2>&1
			#aws s3 cp ${OUT_BASE}.v3.th${th}.out s3://$BUCKET_NAME/$INSTANCE/
		fi
	done
done

if [ "$AUTO_SHUTDOWN" = "1" ]; then
	sudo shutdown -h now
fi

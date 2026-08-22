# Getting Started with PA-Star2

This tutorial guides you through installing dependencies, compiling the project binaries, and running your first Multiple Sequence Alignment (MSA).

## 1. Prerequisites

PA-Star2 is developed for Linux and requires a C++11 compliant compiler, Boost libraries, and Google Highway for SIMD acceleration.

On Ubuntu / Debian:

```bash
sudo apt-get update
sudo apt-get install -y build-essential make libboost-all-dev libhwy-dev
```

## 2. Compilation

Build both `msa_astar` (serial) and `msa_pastar` (parallel) executables using Make:

```bash
make clean && make -j$(nproc)
```

The compiled binaries will be placed in the `bin/` directory:
- `bin/msa_astar`: Serial A* search implementation.
- `bin/msa_pastar`: Parallel A* search implementation.

## 3. Running Your First Alignment

Benchmark sequences in FASTA format are available under `seqs/`.

### Run Serial A* Search

Execute `msa_astar` against a test dataset:

```bash
./bin/msa_astar seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

### Run Parallel A* Search

Execute `msa_pastar` utilizing multiple worker threads (e.g., 4 threads):

```bash
./bin/msa_pastar -t 4 seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

### Save Output to FASTA

Use the `-f` flag to export the resulting aligned sequences to a FASTA file:

```bash
./bin/msa_pastar -t 4 -f output.fasta seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

---
_Verified against `master`@`3794dd6` on 2026-08-22._

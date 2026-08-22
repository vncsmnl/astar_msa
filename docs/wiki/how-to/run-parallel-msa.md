# How-To: Run Parallel MSA on Symmetric & Hybrid Multi-Core CPUs

This guide explains how to configure thread count, CPU core affinity, and asymmetric workload distribution in `msa_pastar`.

## 1. Running on Symmetric Multi-Core Processors

For standard multi-core processors with homogeneous cores (e.g., AMD EPYC, standard Intel Xeon, or Apple Silicon), specify the thread count using `-t`:

```bash
# Run with 8 worker threads
./bin/msa_pastar -t 8 seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

By default, threads automatically pin to sequential core IDs `0, 1, ..., T-1`. To disable CPU affinity binding, add `--no-affinity`:

```bash
./bin/msa_pastar -t 8 --no-affinity seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

## 2. Setting Explicit Thread Affinity

Specify exact CPU core bindings with `--affinity` (or `-a`) as a comma-separated list of CPU core IDs:

```bash
# Pin 4 threads to specific physical CPU cores 0, 2, 4, 6
./bin/msa_pastar -t 4 --affinity=0,2,4,6 seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

## 3. Configuring Asymmetric / Hybrid Processors (P-Cores + E-Cores)

Modern hybrid architectures (such as Intel 12th, 13th, and 14th Gen Core processors) combine fast Performance Cores (P-cores) with efficient Efficiency Cores (E-cores). `msa_pastar` provides the `--hybrid-conf` flag to balance the workload proportionally.

### Flag Format

`--hybrid-conf=<P_cores_num>,<P_cores_size>,<E_cores_num>,<E_cores_size>`

- `P_cores_num`: Number of worker threads assigned to P-cores.
- `P_cores_size`: Weight / proportion multiplier for P-core work distribution.
- `E_cores_num`: Number of worker threads assigned to E-cores.
- `E_cores_size`: Weight / proportion multiplier for E-core work distribution.

> **Requirement**: `P_cores_num + E_cores_num` must exactly equal the total number of threads (`-t`).

### Example: Intel Core i7-13700K (8 P-Cores + 8 E-Cores = 16 Threads)

#### Symmetric 1:1 Distribution
```bash
./bin/msa_pastar -t 16 \
  --affinity=0,2,4,6,8,10,12,14,16,17,18,19,20,21,22,23 \
  --hybrid-conf=8,1,8,1 \
  seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

#### Weighted 9:7 Asymmetric Distribution
Give 9 parts of the spatial hash partitions to P-core threads for every 7 parts assigned to E-core threads:

```bash
./bin/msa_pastar -t 16 \
  --affinity=0,2,4,6,8,10,12,14,16,17,18,19,20,21,22,23 \
  --hybrid-conf=8,9,8,7 \
  seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

## 4. Tuning Spatial Hash Functions

Control how coordinates are partitioned among threads:

```bash
# Select hash type (FZORDER, FSUM, PZORDER, PSUM)
./bin/msa_pastar -t 8 --hash_type=FZORDER --hash_shift=4 seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

- `--hash_type` (`-y`):
  - `FZORDER` (default): Fast full Z-order curve bit interleaving.
  - `PZORDER`: Partial Z-order hashing.
  - `FSUM` / `PSUM`: Sum-based coordinate partitioning.
- `--hash_shift` (`-s`): Bit shift controlling the block size of the spatial hash grid.

---
_Verified against `master`@`3794dd6` on 2026-08-22._

# How-To: Build and Run PA-Star2 with Docker

PA-Star2 includes a multi-stage `Dockerfile` that packages build tools and runtime dependencies (Make, Boost, and Google Highway).

## 1. Build the Docker Image

Run the build command from the repository root:

```bash
docker build -t astar-msa .
```

The resulting image contains:
- `msa_pastar` installed at `/usr/local/bin/msa_pastar` (default container entrypoint)
- `msa_astar` installed at `/usr/local/bin/msa_astar`
- Benchmark sequences preloaded at `/app/seqs`

## 2. Run Parallel Alignment (`msa_pastar`)

Since `msa_pastar` is the default image entrypoint, pass its arguments directly:

```bash
docker run --rm astar-msa -t 4 seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

## 3. Run Serial Alignment (`msa_astar`)

Override the entrypoint to run `msa_astar`:

```bash
docker run --rm --entrypoint msa_astar astar-msa seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

## 4. Mounting Host Datasets and Output Files

Mount a host directory into `/data` inside the container to align local FASTA files and save output alignments:

```bash
docker run --rm \
  -v $(pwd)/my_sequences:/data \
  astar-msa -t 8 -f /data/output.fasta /data/input.fasta
```

---
_Verified against `master`@`3794dd6` on 2026-08-22._

# Command-Line Reference

Complete command-line interface reference for `msa_astar` and `msa_pastar`.

## Common Options

These options apply to both `msa_astar` and `msa_pastar`:

| Option | Short | Argument | Default | Description |
| :--- | :--- | :--- | :--- | :--- |
| `--help` | `-h` | None | — | Display help and usage information. |
| `--version` | `-v` | None | — | Print program version string and exit. |
| `--cost_type` | `-c` | `PAM250` \| `NUC` | `PAM250` | Scoring cost type (`NUC` for nucleotides, `PAM250` for proteins). |
| `--fasta_output`| `-f` | `<filepath>` | `""` | Write resulting multiple sequence alignment to file in FASTA format. |
| `--log_file` | `-l` | `<filepath>` | `""` | Write detailed execution log to the specified file. |
| `--verbose` | — | None | `false` | Enable verbose output with iteration details. |
| `--memory_debug`| — | None | `false` | Enable memory debug mode (disables quick process exit on completion). |
| `file.fasta` | — | `<filepath>` | — | Positional input FASTA file containing sequences to align. |

## Parallel Options (`msa_pastar` only)

Additional options supported exclusively by `msa_pastar`:

| Option | Short | Argument | Default | Description |
| :--- | :--- | :--- | :--- | :--- |
| `--threads` | `-t` | `<int>` | Hardware cores | Number of worker threads. |
| `--affinity` | `-a` | `<csv>` | `0,1,...,T-1` | Comma-separated list of CPU core IDs to pin each thread (e.g. `0,1,2,3`). |
| `--no-affinity` | — | None | `false` | Disable thread affinity setting (ignore affinity configuration). |
| `--hybrid-conf` | — | `<P_num,P_size,E_num,E_size>` | `T,1,0,0` | Workload distribution vector for hybrid CPUs. `P_num + E_num` must equal `--threads`. |
| `--hash_type` | `-y` | `FZORDER` \| `FSUM` \| `PZORDER` \| `PSUM` | `FZORDER` | Spatial hash function used for thread partitioning. |
| `--hash_shift`| `-s` | `<int>` | `16` | Bit shift value controlling spatial hash block granularity. |

---
_Verified against `master`@`3794dd6` on 2026-08-22._

# Architecture & Core Concepts

PA-Star2 solves the Multiple Sequence Alignment (MSA) problem by formulating it as finding the shortest path in an $N$-dimensional state space graph, where $N$ is the number of input sequences.

## 1. Search Space Formulation

- **State Space**: An alignment of $N$ sequences with lengths $L_1, L_2, \dots, L_N$ corresponds to an $N$-dimensional grid ranging from $(0, \dots, 0)$ (origin) to $(L_1, \dots, L_N)$ (goal).
- **Transitions**: At any coordinate $c \in \mathbb{N}^N$, there are $2^N - 1$ possible transitions corresponding to emitting a residue or inserting a gap across all non-empty subsets of sequences.
- **Path Cost**: The edge cost is determined by substitution matrices (such as PAM250 for proteins or identity scores for nucleotides) plus affine/linear gap penalties.

## 2. Parallel A* Search (`PAStar`)

In `PAStar`, the $N$-dimensional coordinate space is partitioned among worker threads:

```
+-------------------------------------------------------------+
|                     Search Space                            |
|             (N-dimensional Coordinate Grid)                 |
+-------------------------------------------------------------+
                              |
                     Spatial Hashing
                 (Z-Order / Morton Code)
                              |
        +---------------------+---------------------+
        |                                           |
        v                                           v
+-------------------+                       +-------------------+
|     Thread 0      |                       |     Thread 1      |
|  - OpenList (BMI) |                       |  - OpenList (BMI) |
|  - ClosedList     |  <-- Node Enqueue --> |  - ClosedList     |
|  - Local Queue    |                       |  - Local Queue    |
+-------------------+                       +-------------------+
```

- **Thread Ownership**: Each node coordinate maps to an owner thread via spatial hashing.
- **Node Queues**: When thread $A$ expands a node whose child belongs to thread $B$, it enqueues the child into thread $B$'s incoming queue protected by mutex/condition variables.
- **Synchronization**: Threads periodically consume incoming nodes, process expansions from their local `OpenList`, and synchronize using barriers upon reaching convergence.

## 3. Dual-Indexed Open List (`PriorityList`)

The Open List is implemented using `boost::multi_index_container` with two concurrent indices:
1. **Positional Hash Index** (`tag<pos>`): `hashed_unique` on `Coord<N>` for $O(1)$ lookups when checking if a node is already present.
2. **Priority Index** (`tag<priority>`): `ordered_non_unique` sorted ascending by $f$-score ($f = g + h$) for $O(\log M)$ extraction of the next best node.

## 4. Spatial Hashing & Load Balancing (`CoordHash`)

Spatial hashing partitions the search space while preserving spatial locality:
- **Z-Order (Morton Code)**: Interleaves coordinate bits to map multi-dimensional coordinates to a 1D space-filling curve (`HashFZorder`).
- **Hash Shift**: Granularity shift (`--hash_shift`) controls the size of contiguous spatial blocks assigned to threads.
- **Workload Distribution Vectors**: For asymmetric multi-core processors (e.g., Intel Alder Lake / Raptor Lake with Performance and Efficient cores), nodes are distributed according to custom ratios (e.g., 9:7) via `--hybrid-conf`.

## 5. 3D Trio Alignment Heuristic (`h3all`) with SIMD Acceleration

The heuristic function $h(c)$ estimates the remaining distance from coordinate $c$ to the goal $(L_1, \dots, L_N)$:
- **Trio Alignments (`TrioAlign`)**: Admissible heuristic constructed by precomputing exact optimal alignments for all 3-sequence triplets $\binom{N}{3}$.
- **SIMD Vectorization (`SIMDAligner`)**: The innermost loop of the 3D Dynamic Programming table is vectorized using **Google Highway** (`libhwy`):
  - 6 of the 7 candidate transitions are independent across the $k$-axis and evaluated concurrently in vector registers (AVX2, AVX-512, NEON).
  - The remaining serial dependency ($k+1$) is computed via a scalar backwards sweep.

---
_Verified against `master`@`3794dd6` on 2026-08-22._

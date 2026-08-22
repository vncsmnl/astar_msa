# PA-Star2 Documentation

Welcome to the PA-Star2 documentation. PA-Star2 is a high-performance C++ tool for optimal Multiple Sequence Alignment (MSA) using parallel A* graph search, SIMD acceleration via Google Highway, and workload distribution for asymmetric/hybrid multi-core processors.

## Documentation Map

- **[Getting Started](getting-started.md)** (Tutorial)
  Prerequisites, compilation steps, and running your first sequence alignment with `msa_astar` and `msa_pastar`.
- **[Architecture & Core Concepts](architecture.md)** (Explanation)
  In-depth explanation of the A* search space formulation, Z-order spatial hashing, 3D trio Dynamic Programming heuristic (`h3all`), and Highway SIMD vectorization.
- **[How-To: Run Parallel MSA on Hybrid CPUs](how-to/run-parallel-msa.md)** (How-To)
  Practical guide to configuring thread count, CPU core affinity, and workload distribution vectors across asymmetric P-cores and E-cores.
- **[How-To: Build and Run with Docker](how-to/build-and-run-docker.md)** (How-To)
  Guide to building multi-stage container images and running alignments inside Docker.
- **[CLI Reference](reference.md)** (Reference)
  Exhaustive command-line flag reference for both `msa_astar` and `msa_pastar` executables.

---
_Verified against `master`@`3794dd6` on 2026-08-22._

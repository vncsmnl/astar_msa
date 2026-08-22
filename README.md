<!-- wikikit:front-door:start -->
# PA-Star2

PA-Star is a software that performs a parallel A-Star search to solve the Multiple Sequence Alignment (MSA) problem. For comparison we also developed a serial version (msa_astar).

PA-Star2 is a new version with several improvements on the performance. A new Workload Distribution Vector Technique has been developed, where you can distribute the work among the threads on Asymmetric Processors (Like Intel 12th to 14th Gen.). More details, check our paper (Cite us section).

## Documentation

Full documentation is available in the [docs/wiki/](docs/wiki/index.md) directory:
- [Getting Started](docs/wiki/getting-started.md)
- [Architecture & Core Concepts](docs/wiki/architecture.md)
- [How-To: Run Parallel MSA on Hybrid CPUs](docs/wiki/how-to/run-parallel-msa.md)
- [How-To: Build and Run with Docker](docs/wiki/how-to/build-and-run-docker.md)
- [CLI Reference](docs/wiki/reference.md)

## Getting Started

The PA-Star has been developed for Linux. The software is stable and has been stress tested using machines with 1TB of memory. The memory requirement usually is the main issue for the MSA problem.

### Prerequisites

You need a modern C++ compiler, Make, libboost devel package, and Google Highway SIMD library. On Ubuntu, you can install them by:

```bash
sudo apt-get install build-essential make libboost-all-dev libhwy-dev
```

### Compiling

To compile, you enter the project folder and use Make:

```bash
make clean && make -j$(nproc)
```

This command works on all major Linux distributions and the `msa_astar` and `msa_pastar` binaries will be available in the `bin/` folder.

### Compiling and Running with Docker

You can also build and run the application using Docker, which handles all build and runtime dependencies (Make, Boost, and Google Highway) automatically.

**Build the Docker image:**
```bash
docker build -t astar-msa .
```

**Run the parallel version (`msa_pastar`):**
```bash
docker run --rm astar-msa -t 2 seqs/3/synthetic_veryeasy.fasta
```

**Run the serial version (`msa_astar`):**
```bash
docker run --rm --entrypoint msa_astar astar-msa seqs/3/synthetic_easy.fasta
```

## How to execute

In the project we included many examples. By default, you can run msa_astar for serial executions and msa_pastar for parallel executions using as many cores as possible in the machine.

Few examples:
```bash
# Easy test:
./bin/msa_astar seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta

# Running the parallel version with 4 threads:
./bin/msa_pastar -t 4 seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta

# Saving the output as a fasta file:
./bin/msa_pastar -f output.fasta seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta

# PA-Star2 asymmetric processor distribution:
./bin/msa_pastar -t 16 --affinity=0,2,4,6,8,10,12,14,16,17,18,19,20,21,22,23 --hybrid-conf=8,9,8,7 seqs/Balibase/Ref1/0_short_low_id/1aboA.fasta
```

### More options

```bash
./bin/msa_astar -h
./bin/msa_pastar -h
```
<!-- wikikit:front-door:end -->

### Does it work with other problems or just the multiple sequence alignment?

Yes, the A\* algorithm is useful for any pathfinding problem! But this project only implements MSA. If you need to use it on another problem, the function 'msa\_astar' is specific for the MSA problem, but the PriorityList class can be reused for any kind of graph.
You can also implement any 'best-first search' algorithm using the PriorityList class, by changing the rules on how you insert the nodes in the list.

## Cite us
Daniel Sundfeld, George Teodoro, Alba Cristina Magalhaes Alves de Melo: PA-Star2: Fast Optimal Multiple Sequence
Alignment for Asymmetric Multicore Processors. In 33rd Euromicro International Conference on Parallel, Distributed and Network-based Processing (PDP 2025), Torino, Italy, 2025, pp. 146-153

## List of previous papers
Daniel Sundfeld, Caina Razzolini, George Teodoro, Azzedine Boukerche, Alba Cristina Magalhaes Alves de Melo: PA-Star: A disk-assisted parallel A-Star strategy with locality-sensitive hash for multiple sequence alignment.  Journal of Parallel and Distributed Computing 112: 154-165 (2018)

Daniel Sundfeld, George Teodoro, Alba Cristina Magalhaes Alves de Melo: Parallel A-Star Multiple Sequence Alignment with Locality-Sensitive Hash Functions, 2015 Ninth International Conference on Complex, Intelligent, and Software Intensive Systems (CISIS 2015), Santa Catarina, Brazil, 2015, pp. 342-347

## Documentation
This is a small scientific project, but we have professional
standards on our code and documentation. You can generate
the Doxygen documentation by running 'make' command in the
'doc/' folder.

## License

This project is licensed under the MIT License

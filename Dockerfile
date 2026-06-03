# ==============================================================================
# Stage 1: Build the application (builder)
# ==============================================================================
FROM ubuntu:22.04 AS builder

# Avoid interactive prompts during apt packages installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies needed to build the C++ project
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    libhwy-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the source code (respecting .dockerignore)
COPY . .

# Configure and build the project in Release mode
# Note: By default, the compiled binaries (msa_astar and msa_pastar)
# will be placed in the /app/bin directory.
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

# ==============================================================================
# Stage 2: Create a minimal runtime image (runner)
# ==============================================================================
FROM ubuntu:22.04 AS runner

# Avoid interactive prompts during apt packages installation
ENV DEBIAN_FRONTEND=noninteractive

# Install only the runtime shared libraries (without build tools or headers)
# Note: For Ubuntu 22.04, the boost version is 1.74.0 and highway is 1.0.7.
# If you change the base image, update these packages to match the new version.
RUN apt-get update && apt-get install -y --no-install-recommends \
    libboost-program-options1.74.0 \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libhwy1.0.7 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binaries from the builder stage
COPY --from=builder /app/bin/msa_astar /usr/local/bin/msa_astar
COPY --from=builder /app/bin/msa_pastar /usr/local/bin/msa_pastar

# Copy the seqs folder for easy out-of-the-box testing/run
COPY --from=builder /app/seqs /app/seqs

# Set default execution to the parallel A-Star (msa_pastar)
ENTRYPOINT ["msa_pastar"]

# Default argument to print help if no arguments are passed
CMD ["--help"]

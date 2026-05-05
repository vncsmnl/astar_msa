cmake_minimum_required(VERSION 3.16)
project(PA-Star2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(HWY 1.0.7 REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options system filesystem)

# Optional build flags
option(ENABLE_PAIRALIGN_SCORE "Print pairwise alignment scores" OFF)

# Common compile options
set(COMMON_COMPILE_FLAGS -W -Wall -O3 -mavx2 -mfma)

if(ENABLE_PAIRALIGN_SCORE)
    list(APPEND COMMON_COMPILE_FLAGS -DPAIRALIGN_SCORE)
endif()

if(DEFINED THREADS_NUM)
    list(APPEND COMMON_COMPILE_FLAGS -DTHREADS_NUM=${THREADS_NUM})
endif()

if(DEFINED HASH_SHIFT)
    list(APPEND COMMON_COMPILE_FLAGS -DHASH_SHIFT=${HASH_SHIFT})
endif()

# Common sources (shared between both binaries)
set(COMMON_SRCS
    src/backtrace.cpp
    src/Coord.cpp
    src/CoordHash.cpp
    src/Cost.cpp
    src/HeuristicHPair.cpp
    src/msa_options.cpp
    src/Node.cpp
    src/read_fasta.cpp
    src/PairAlign.cpp
    src/TrioAlign.cpp
    src/TrioAlignSIMD.cpp
    src/Sequences.cpp
    src/TimeCounter.cpp
)

# Version header generation
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/version.h
    COMMAND ${CMAKE_SOURCE_DIR}/utils/version.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating version.h"
)

# --- msa_astar (serial) ---
add_executable(msa_astar
    src/AStar.cpp
    src/msa_astar_main.cpp
    ${COMMON_SRCS}
    ${CMAKE_SOURCE_DIR}/src/version.h
)
target_compile_options(msa_astar PRIVATE ${COMMON_COMPILE_FLAGS})
target_include_directories(msa_astar PRIVATE src /usr/include /usr/local/include)
target_link_libraries(msa_astar PRIVATE
    hwy
    Threads::Threads
    Boost::program_options
    Boost::system
    Boost::filesystem
)

# --- msa_pastar (parallel) ---
add_executable(msa_pastar
    src/msa_pastar_main.cpp
    src/PAStar.cpp
    ${COMMON_SRCS}
    ${CMAKE_SOURCE_DIR}/src/version.h
)
target_compile_options(msa_pastar PRIVATE ${COMMON_COMPILE_FLAGS})
target_include_directories(msa_pastar PRIVATE src /usr/include /usr/local/include)
target_link_libraries(msa_pastar PRIVATE
    hwy
    Threads::Threads
    Boost::program_options
    Boost::system
    Boost::filesystem
)

# Output binaries to bin/ directory
set_target_properties(msa_astar msa_pastar PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

#!/bin/bash
PASTAR1="64c6ed5b7eaebfb8cb974135e8f76d0ab1668d9a"
PASTAR2="ddccc1894d2cab648d928dfc8784dbe299eb44aa"
PASTAR3="25f31e8e25f25a42b058c4d408171c0037743ce2"
MAKEOPTS="-j 32 -B"

if [ ! -e Makefile ]; then
    echo "Please run this script on astar_msa root folder"
    exit 1
fi

mkdir -p bin

compile_version() {
    local commit="$1"
    local output_bin="$2"
    
    echo "=== Compiling $output_bin ($commit) ==="
    rm -rf src/version.h
    git checkout -f "$commit"
    
    # Ensure modern GCC compatibility (<cstdint> for uint16_t)
    if [ -f src/Coord.h ] && ! grep -q "<cstdint>" src/Coord.h; then
        sed -i '9a #include <cstdint>' src/Coord.h
    fi

    make $MAKEOPTS
    if [ $? -ne 0 ]; then
        echo "ERROR compiling $output_bin"
        git checkout -f master
        exit 1
    fi
    cp ./bin/msa_pastar "$output_bin"
}

compile_version "$PASTAR1" "./bin/msa_pastar_v1"
compile_version "$PASTAR2" "./bin/msa_pastar_v2"
compile_version "$PASTAR3" "./bin/msa_pastar_v3"

make clean
rm -rf src/version.h
git checkout -f master
echo "=== All versions compiled successfully! ==="


#!/bin/sh
#
# usage: sh tools/update-mellifera.sh /path/to/mellifera
set -eu

if [ $# -ne 1 ]; then
    echo "usage: $0 /path/to/mellifera"
    exit 1
fi

SRC=$(realpath src)
MELLIFERA="$1"

cd "${MELLIFERA}"
make wasm-go
cp mellifera.wasm "${SRC}/mellifera/wasm/mellifera.wasm"
cp wasm_exec.js   "${SRC}/mellifera/wasm/wasm_exec.js"

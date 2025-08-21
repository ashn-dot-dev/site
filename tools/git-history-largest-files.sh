#!/bin/sh
#
# Usage: git-history-largest-files.sh [number-of-files]
set -eu

COUNT=${1:-10}
BYTES_PER_GB=1073741824
BYTES_PER_MB=1048576
BYTES_PER_KB=1024

git rev-list --objects --all | \
    git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' | \
    awk '/^blob/ && $4 != "" {print $3 "\t" $4}' | \
    sort -rn | \
    head -n "$COUNT" | \
    awk -F'\t' \
        -v gb="$BYTES_PER_GB" \
        -v mb="$BYTES_PER_MB" \
        -v kb="$BYTES_PER_KB" \
        '{
            size = $1
            path = $2

            if (size >= gb) {
                printf "%.2f GB %s\n", size/gb, path
            }
            else if (size >= mb) {
                printf "%.2f MB %s\n", size/mb, path
            }
            else if (size >= kb) {
                printf "%.2f KB %s\n", size/kb, path
            }
            else {
                printf "%d B %s\n", size, path
            }
        }'

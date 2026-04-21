#!/bin/sh
#
# usage: git-history-elide-file.sh FILE
set -eu

if [ $# -ne 1 ]; then
    echo "Usage: $0 FILE"
    exit 1
fi

FILE="$1"

git filter-repo --force --path "${FILE}" --invert-paths --refs ^HEAD
git reflog expire --expire=now --all
git gc --prune=now --aggressive

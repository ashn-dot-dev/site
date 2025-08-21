#!/bin/sh
#
# Usage: git-history-elide-file.sh FILE
set -eu

if [ $# -ne 1 ]; then
    echo "Usage: $0 FILE"
    exit 1
fi

FILE="$1"

export FILE
export FILTER_BRANCH_SQUELCH_WARNING=1
git filter-branch --tree-filter '
if [ -f "${FILE}" ]; then
    echo "original file removed from git history" > "${FILE}"
fi
' --all

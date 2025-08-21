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
git filter-branch -f --tree-filter '
if git ls-files | grep -q "^${FILE}$"; then
    mkdir -p "$(dirname "${FILE}")"
    echo "original file removed from git history" > "${FILE}"
fi
' -- --all
rm -rf .git/refs/original/
git reflog expire --expire=now --all
git gc --prune=now --aggressive

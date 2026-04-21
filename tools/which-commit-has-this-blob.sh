#!/bin/sh
# Taken from https://stackoverflow.com/a/223890

OBJECT="$1" # SHA1 of the object

shift
git log "$@" --pretty=tformat:'%T %h %s' \
| while read tree commit subject ; do
    if git ls-tree -r $tree | grep -q "${OBJECT}" ; then
        echo $commit "$subject"
    fi
done

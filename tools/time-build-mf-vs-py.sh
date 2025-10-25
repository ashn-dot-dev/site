#!/bin/sh
set -eu

echo 'Running `build.bash -mf`...'
/usr/bin/time -p bash build.bash -mf >/dev/null
printf '\n'
echo 'Running `build.bash -py`...'
/usr/bin/time -p bash build.bash -py >/dev/null

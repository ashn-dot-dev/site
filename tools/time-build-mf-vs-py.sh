#!/bin/sh
echo 'Running `build.bash -mf`...'
time bash build.bash -mf >/dev/null 2>&1
printf '\n'
echo 'Running `build.bash -py`...'
time bash build.bash -py >/dev/null 2>&1

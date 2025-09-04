#!/bin/sh
set -ex
exiftool -overwrite_original -all= "$@"

#!/bin/sh
set -ex
exiftool -all= "$@"

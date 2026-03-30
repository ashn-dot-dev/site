#!/bin/sh
# Build and deploy the website localhost:8000.
set -eux
sh build.sh $@ && (cd out/ && python3 -m http.server 8000)

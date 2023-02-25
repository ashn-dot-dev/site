#!/bin/sh
# Build and deploy the website localhost:8000.
set -eux
./build.bash && (cd out/ && python3 -m http.server 8000)

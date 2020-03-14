#!/bin/sh

usage() {
    echo 'Usage: deploy-web.sh USER SERVER'
    echo 'Build and deploy the site using ssh via USER@SERVER'
}

if [ "$#" -ne 2 ]; then
    usage
    exit 1
fi

set -e
set -x

USER="$1"
SERVER="$2"

./build.bash
ssh "$USER@$SERVER" 'rm -rf /var/www/html/*'
scp -r out/* "$USER@$SERVER:/var/www/html/"

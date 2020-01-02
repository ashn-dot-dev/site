#!/bin/bash

set -e

SRC_DIR='src'
OUT_DIR='out'

#== Create Output Directory
if [ -d "${OUT_DIR}" ]; then
    rm -rf "${OUT_DIR}"
fi
mkdir -p "${OUT_DIR}"
mkdir -p "${OUT_DIR}/blog"

#== Generate Website
TEMPLATE=$(cat template.html)

make_page() {
    PAGE_TITLE="$1"
    PAGE_CONTENT="$2"

    # Non sh-compliant bash substitutions.
    PAGE_OUT="${TEMPLATE}"
    PAGE_OUT="${PAGE_OUT/<!--TITLE-->/${PAGE_TITLE}}"
    PAGE_OUT="${PAGE_OUT/<!--PAGE-->/${PAGE_CONTENT}}"

    echo "${PAGE_OUT}"
}

build_page_html() {
    SRC_PATH="$1" # Path under the src/ directory.
    TITLE="$2"    # Page title.
    echo "BUILDING PAGE: ${SRC_PATH}"

    SRC_FILE="${SRC_DIR}/${SRC_PATH}"
    OUT_FILE="${OUT_DIR}/${SRC_PATH}"

    CONTENT=$(cat "${SRC_FILE}")
    make_page "${TITLE}" "${CONTENT}" > "${OUT_FILE}"
}

cp "${SRC_DIR}/favicon.ico" "${OUT_DIR}"
cp "${SRC_DIR}/style.css" "${OUT_DIR}"

build_page_html "index.html" "ashn"
build_page_html "blog.html" "blog"

build_page_html "blog/2020-01-01-hello-world.html" "Hello World"

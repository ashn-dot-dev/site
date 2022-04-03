#!/bin/bash

set -e

NL=$'\n'
SRC_DIR='src'
OUT_DIR='out'
TIMEFORMAT="TIME: %E"

#== Create Output Directory ====================================================
if [ -d "${OUT_DIR}" ]; then
    rm -rf "${OUT_DIR}"
fi
mkdir -p "${OUT_DIR}"
mkdir -p "${OUT_DIR}/blog"

#== Website Generation Helper Utilities ========================================
TEMPLATE=$(cat template.html)

md_page_title() {
    FILE="$1"
    head -n 1 "${FILE}"
}

make_page() {
    PAGE_TITLE="$1"
    PAGE_CONTENT="$2"

    PAGE_OUT="${TEMPLATE}"
    PAGE_OUT="${PAGE_OUT/<!--TITLE-->/${PAGE_TITLE}}"
    PAGE_OUT="${PAGE_OUT/<!--PAGE-->/${PAGE_CONTENT}}"

    echo "${PAGE_OUT}"
}

build_page_from_html() {
    SRC_PATH="$1" # Path under the src/ directory.
    TITLE="$2"    # Page title.
    echo "BUILDING HTML PAGE: ${SRC_PATH}"

    SRC_FILE="${SRC_DIR}/${SRC_PATH}"
    OUT_FILE="${OUT_DIR}/${SRC_PATH}"

    CONTENT=$(cat "${SRC_FILE}")
    make_page "${TITLE}" "${CONTENT}" > "${OUT_FILE}"
}

build_page_from_md() {
    SRC_PATH="$1" # Path under the src/ directory.
    TITLE="$2"    # Page title.
    echo "BUILDING MARKDOWN PAGE: ${SRC_PATH}"

    SRC_FILE="${SRC_DIR}/${SRC_PATH}"
    OUT_FILE="${OUT_DIR}/${SRC_PATH%.md}.html"

    CONTENT=$(pandoc --preserve-tabs "${SRC_FILE}")
    make_page "${TITLE}" "${CONTENT}" > "${OUT_FILE}"
}

build_blog_page() {
    SRC_PATH="$1" # Path under the src/ directory.

    TITLE="$(md_page_title "${SRC_DIR}/${SRC_PATH}")"
    build_page_from_md "${SRC_PATH}" "${TITLE}"
}

#== Generate the Actual Website ================================================
build_html_pages() {
    echo "==== BUILDING HTML PAGES ===="
    build_page_from_html "index.html" "ashn"
    build_page_from_html "art.html" "art"
    build_page_from_html "mus.html" "music"
}

build_blog_archive_page() {
    echo "==== BUILDING BLOG ARCHIVE PAGE ===="
    BLOG_INDEX_CONTENT=$(cat "${SRC_DIR}/blog.html")
    for f in $(ls "${SRC_DIR}/blog" | sort -r); do
        [ -d "${SRC_DIR}/blog/${f}" ]            && continue # Skip directories.
        [ $(echo "${f}" | head -c 3 -) = 'wip' ] && continue # Skip WIP entries.

        echo "PROCESSING BLOG ENTRY: ${f}"
        # YYYY-MM-DD
        # 123456789A bytes should be parsed to get the date from the filename.
        F_DATE=$(echo "${f}" | head -c 10 -)
        F_TITLE=$(md_page_title "${SRC_DIR}/blog/${f}")
        F_HREF="/blog/${f%.md}.html"

        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}[ ${F_DATE} ] "
        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}<a href=\"${F_HREF}\">"
        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}  ${F_TITLE}"
        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}</a>"
        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}<br>"
    done
    make_page "archive" "${BLOG_INDEX_CONTENT}" > "${OUT_DIR}/blog.html"
}

build_blog_entry_pages() {
    echo "==== BUILDING BLOG ENTRY PAGES ===="
    for f in $(ls -C "${SRC_DIR}/blog"); do
        if [ -d "${SRC_DIR}/blog/${f}" ]; then
            cp -r "${SRC_DIR}/blog/${f}" "${OUT_DIR}/blog/${f}"
            continue
        fi
        build_blog_page "blog/${f}" &
    done
    wait
}

copy_misc_files() {
    echo "==== COPYING MISC FILES ===="
    set -x
    cp "${SRC_DIR}/favicon.ico" "${OUT_DIR}"
    cp "${SRC_DIR}/style.css" "${OUT_DIR}"
    cp -r "${SRC_DIR}/art/" "${OUT_DIR}"
    cp -r "${SRC_DIR}/mus/" "${OUT_DIR}"
    { set +x; } 2> /dev/null
}

time build_html_pages
time build_blog_archive_page
time build_blog_entry_pages
time copy_misc_files

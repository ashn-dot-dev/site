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
mkdir -p "${OUT_DIR}/tmp"

#== Website Generation Helper Utilities ========================================
TEMPLATE=$(cat template.html)

md_page_title() {
    FILE="$1"
    head -n 1 "${FILE}"
}

make_page() {
    PAGE_TITLE="$1"
    PAGE_CONTENT="$2"

    TEMP_FILE=$(mktemp)
    echo "${PAGE_CONTENT}" > "${TEMP_FILE}"

    echo "${TEMPLATE}" | \
    sed "s|<!--TITLE-->|${PAGE_TITLE}|g" | \
    sed -e '/<!--PAGE-->/{
        r '"${TEMP_FILE}"'
        d
    }'

    rm "${TEMP_FILE}"
}

build_page_from_html() {
    SRC_PATH="$1" # Path under the src/ directory.
    TITLE="$2"    # Page title.
    echo "BUILDING HTML PAGE: ${SRC_PATH}"

    SRC_FILE="${SRC_DIR}/${SRC_PATH}"
    OUT_FILE="${OUT_DIR}/${SRC_PATH}"

    CONTENT=$(cat "${SRC_FILE}")
    make_page "${TITLE}" "${CONTENT}" >"${OUT_FILE}"
}

build_page_from_md() {
    SRC_PATH="$1" # Path under the src/ directory.
    TITLE="$2"    # Page title.
    echo "BUILDING MARKDOWN PAGE: ${SRC_PATH}"

    SRC_FILE="${SRC_DIR}/${SRC_PATH}"
    OUT_FILE="${OUT_DIR}/${SRC_PATH%.md}.html"

    MARKDOWN=$(cat "${SRC_FILE}")
    MARKDOWN=$(echo "${MARKDOWN}" | sed -r 's/\[\^([1-9]+)\]:$/<span id="footnote-\1">\1./g')
    MARKDOWN=$(echo "${MARKDOWN}" | sed -r 's/\[\^([1-9]+)\]/<a href="#footnote-\1"><sup>\1<\/sup><\/a>/g')
    RENDERED=$(echo "${MARKDOWN}" | cmark --unsafe)
    make_page "${TITLE}" "${RENDERED}" >"${OUT_FILE}"
}

build_blog_page() {
    SRC_PATH="$1" # Path under the src/ directory.

    TITLE="$(md_page_title "${SRC_DIR}/${SRC_PATH}")"
    build_page_from_md "${SRC_PATH}" "${TITLE}"
}

#== Generate the Actual Website ================================================
build_main_pages() {
    echo "==== BUILDING MAIN HTML PAGES ===="
    build_page_from_html "index.html" "ashn"
    build_page_from_html "scratchpad.html" "ashn"
}

build_blog_archive_page() {
    echo "==== BUILDING BLOG ARCHIVE PAGE ===="
    BLOG_INDEX_CONTENT=$(cat "${SRC_DIR}/blog.html")
    BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}<table>"
    for f in $(ls "${SRC_DIR}/blog" | sort -r); do
        [ -d "${SRC_DIR}/blog/${f}" ]            && continue # Skip dirs
        [ "$(echo "${f}" | head -c 3)" = 'wip' ] && continue # Skip WIP posts

        echo "PROCESSING BLOG ENTRY: ${f}"
        # YYYY-MM-DD
        # 123456789A bytes should be parsed to get the date from the filename.
        F_DATE=$(echo "${f}" | head -c 10)
        F_TITLE=$(md_page_title "${SRC_DIR}/blog/${f}")
        F_HREF="/blog/${f%.md}.html"

        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}<tr>"
        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}<td>[${F_DATE}]</td>"
        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}<td><a href=\"${F_HREF}\">${F_TITLE}</a></td>"
        BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}</tr>"
    done
    BLOG_INDEX_CONTENT="${BLOG_INDEX_CONTENT}${NL}</table>"
    make_page "archive" "${BLOG_INDEX_CONTENT}" >"${OUT_DIR}/blog.html"
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

build_blog_rss_page() {
    echo "==== BUILDING BLOG RSS PAGE ===="
    RSS_CONTENT="<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
    RSS_CONTENT="${RSS_CONTENT}${NL}<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">"
    RSS_CONTENT="${RSS_CONTENT}${NL}<channel>"
    RSS_CONTENT="${RSS_CONTENT}${NL}  <title>Ashn's Blog</title>"
    RSS_CONTENT="${RSS_CONTENT}${NL}  <link>https://ashn.dev/</link>"
    RSS_CONTENT="${RSS_CONTENT}${NL}  <atom:link href=\"https://ashn.dev/rss.xml\" rel=\"self\" type=\"application/rss+xml\" />"
    RSS_CONTENT="${RSS_CONTENT}${NL}  <description>Ashn's Blog</description>"
    RSS_CONTENT="${RSS_CONTENT}${NL}  <language>en</language>"
    for f in $(ls "${SRC_DIR}/blog" | sort -r); do
        [ -d "${SRC_DIR}/blog/${f}" ]            && continue # Skip dirs
        [ "$(echo "${f}" | head -c 3)" = 'wip' ] && continue # Skip WIP posts

        # YYYY-MM-DD
        # 123456789A bytes should be parsed to get the date from the filename.
        F_DATE=$(echo "${f}" | head -c 10)
        F_TITLE=$(md_page_title "${SRC_DIR}/blog/${f}")
        F_LINK="https://ashn.dev/blog/${f%.md}.html"

        RSS_CONTENT="${RSS_CONTENT}${NL}  <item>"
        RSS_CONTENT="${RSS_CONTENT}${NL}    <title>${F_TITLE}</title>"
        RSS_CONTENT="${RSS_CONTENT}${NL}    <link>${F_LINK}</link>"
        RSS_CONTENT="${RSS_CONTENT}${NL}    <description></description>"
        RSS_CONTENT="${RSS_CONTENT}${NL}  </item>"
    done
    RSS_CONTENT="${RSS_CONTENT}${NL}</channel>"
    RSS_CONTENT="${RSS_CONTENT}${NL}</rss>"
    echo "${RSS_CONTENT}" >"${OUT_DIR}/rss.xml"
}

copy_misc_files() {
    echo "==== COPYING MISC FILES ===="
    set -x
    cp "${SRC_DIR}/favicon.ico" "${OUT_DIR}"
    cp "${SRC_DIR}/style.css" "${OUT_DIR}"
    { set +x; } 2>/dev/null
}

copy_tmp_files() {
    echo "==== COPYING TEMPORARY FILES ===="
    set -x
    cp -r "${SRC_DIR}/tmp" "${OUT_DIR}"
    { set +x; } 2>/dev/null
}

time build_main_pages
time build_blog_archive_page
time build_blog_entry_pages
time build_blog_rss_page
time copy_misc_files
time copy_tmp_files

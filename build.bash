#!/bin/bash
set -eu

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
mkdir -p "${OUT_DIR}/recipes"
mkdir -p "${OUT_DIR}/recipes/mains"
mkdir -p "${OUT_DIR}/characters"
mkdir -p "${OUT_DIR}/tmp"

#== Website Generation Helper Utilities ========================================
TEMPLATE=$(cat template.html)

md_blog_entry_title() {
    FILE="$1"
    head -n 1 "${FILE}"
}

md_recipes_entry_title() {
    FILE="$1"
    sed -n '/^# /{s/^# //p;q;}' "$FILE"
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

    MARKDOWN=$(cat "${SRC_FILE}" | python3 preprocess.py)
    MARKDOWN=$(echo "${MARKDOWN}" | sed -r 's/\[\^([1-9]+)\]:$/<span id="footnote-\1">\1./g')
    MARKDOWN=$(echo "${MARKDOWN}" | sed -r 's/\[\^([1-9]+)\]/<a href="#footnote-\1"><sup>\1<\/sup><\/a>/g')
    RENDERED=$(echo "${MARKDOWN}" | cmark --unsafe)
    make_page "${TITLE}" "${RENDERED}" >"${OUT_FILE}"
}

#== Generate the Actual Website ================================================
build_main_pages() {
    echo "==== BUILDING MAIN HTML PAGES ===="
    build_page_from_html "index.html" "ashn"
    build_page_from_html "scratchpad.html" "ashn"
}

build_blog_page() {
    echo "==== BUILDING BLOG PAGE ===="
    BLOG_PAGE_CONTENT=$(cat "${SRC_DIR}/blog.html")
    BLOG_PAGE_CONTENT="${BLOG_PAGE_CONTENT}${NL}<table>"
    for f in $(ls "${SRC_DIR}/blog" | sort -r); do
        [ -d "${SRC_DIR}/blog/${f}" ]            && continue # Skip dirs
        [ "$(echo "${f}" | head -c 3)" = 'wip' ] && continue # Skip WIP posts

        echo "PROCESSING BLOG ENTRY: ${f}"
        # YYYY-MM-DD
        # 123456789A bytes should be parsed to get the date from the filename.
        F_DATE=$(echo "${f}" | head -c 10)
        F_TITLE=$(md_blog_entry_title "${SRC_DIR}/blog/${f}")
        F_HREF="/blog/${f%.md}.html"

        BLOG_PAGE_CONTENT="${BLOG_PAGE_CONTENT}${NL}<tr>"
        BLOG_PAGE_CONTENT="${BLOG_PAGE_CONTENT}${NL}<td>[${F_DATE}]</td>"
        BLOG_PAGE_CONTENT="${BLOG_PAGE_CONTENT}${NL}<td><a href=\"${F_HREF}\">${F_TITLE}</a></td>"
        BLOG_PAGE_CONTENT="${BLOG_PAGE_CONTENT}${NL}</tr>"
    done
    BLOG_PAGE_CONTENT="${BLOG_PAGE_CONTENT}${NL}</table>"
    make_page "blog" "${BLOG_PAGE_CONTENT}" >"${OUT_DIR}/blog.html"
}

build_blog_entry_pages() {
    echo "==== BUILDING BLOG ENTRY PAGES ===="
    for f in $(ls -C "${SRC_DIR}/blog"); do
        if [ -d "${SRC_DIR}/blog/${f}" ]; then
            cp -r "${SRC_DIR}/blog/${f}" "${OUT_DIR}/blog/${f}"
            continue
        fi
        build_blog_entry_page "blog/${f}" &
    done
    wait
}

build_blog_entry_page() {
    SRC_PATH="$1" # Path under the src/ directory.

    TITLE="$(md_blog_entry_title "${SRC_DIR}/${SRC_PATH}")"
    build_page_from_md "${SRC_PATH}" "${TITLE}"
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
        F_TITLE=$(md_blog_entry_title "${SRC_DIR}/blog/${f}")
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

build_recipes_page() {
    echo "==== BUILDING RECIPES PAGE ===="
    RECIPES_PAGE_CONTENT="<h1>Recipes</h1>"

    for category in $(ls -C "${SRC_DIR}/recipes"); do
        RECIPES_PAGE_CATEGORY=$(echo "${category}" | python3 -c 'print(input().title(), end="")')
        RECIPES_PAGE_CONTENT="${RECIPES_PAGE_CONTENT}${NL}<h2>${RECIPES_PAGE_CATEGORY}</h2>"
        RECIPES_PAGE_CONTENT="${RECIPES_PAGE_CONTENT}${NL}<ul>"
        for f in $(ls -C "${SRC_DIR}/recipes/${category}" | sort); do
            [ -d "${SRC_DIR}/recipes/${category}/${f}" ] && continue # Skip dirs
            F_TITLE=$(md_recipes_entry_title "${SRC_DIR}/recipes/${category}/${f}")
            F_HREF="/recipes/${category}/${f%.md}.html"
            RECIPES_PAGE_CONTENT="${RECIPES_PAGE_CONTENT}${NL}<li><a href=\"${F_HREF}\">${F_TITLE}</a></li>"
        done
        RECIPES_PAGE_CONTENT="${RECIPES_PAGE_CONTENT}${NL}</ul>"
    done

    make_page "recipes" "${RECIPES_PAGE_CONTENT}" >"${OUT_DIR}/recipes.html"
}

build_recipes_entry_pages() {
    echo "==== BUILDING RECIPES ENTRY PAGES ===="
    TITLE="$(md_recipes_entry_title "${SRC_DIR}/${SRC_PATH}")"
    for category in $(ls -C "${SRC_DIR}/recipes"); do
        echo "CATEGORY: ${category}"
        for recipe in $(ls -C "${SRC_DIR}/recipes/${category}"); do
            case "${recipe}" in
            *.md)
                build_recipes_entry_page "recipes/${category}/${recipe}" &
                ;;
            *)
                cp -r "${SRC_DIR}/recipes/${category}/${recipe}" "${OUT_DIR}/recipes/${category}/${recipe}"
                ;;
        esac
        done
    done
    wait
}

build_recipes_entry_page() {
    SRC_PATH="$1" # Path under the src/ directory.

    TITLE="$(md_recipes_entry_title "${SRC_DIR}/${SRC_PATH}")"
    build_page_from_md "${SRC_PATH}" "${TITLE}"
}

build_characters_page() {
    echo "==== BUILD CHARACTERS PAGE ===="
    build_page_from_md "characters.md" "Characters"
}

build_characters_files() {
    echo "==== BUILD CHARACTERS FILES ===="
    set -x
    cp -r ${SRC_DIR}/characters/* ${OUT_DIR}/characters
    { set +x; } 2>/dev/null
}

build_misc_files() {
    echo "==== BUILD MISC FILES ===="
    set -x
    cp "${SRC_DIR}/favicon.ico" "${OUT_DIR}"
    cp "${SRC_DIR}/style.css" "${OUT_DIR}"
    { set +x; } 2>/dev/null
}

build_tmp_files() {
    echo "==== BUILD TEMPORARY FILES ===="
    set -x
    cp -r "${SRC_DIR}/tmp" "${OUT_DIR}"
    { set +x; } 2>/dev/null
}

time build_main_pages
time build_blog_page
time build_blog_entry_pages
time build_blog_rss_page
time build_recipes_page
time build_recipes_entry_pages
time build_characters_page
time build_characters_files
time build_misc_files
time build_tmp_files

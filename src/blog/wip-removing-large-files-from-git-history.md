Removing Large Files from a Repository's Git History
====================================================

I consider myself to be a [grug-brained developer](https://grugbrain.dev/). I
enjoy building and maintaining software systems that eschew complexity in favor
of readability and ease of understanding. This website is one of those systems:
the entire thing is built as a static site from a bunch of loose HTML and
Markdown files all jammed together using a dead simple templating system in a
Bash script, and all of the media content (images, videos, etc.) is stored
directly in the Git repository alongside the textual content. As a result of
this grug-brained repository structure, the website repository has ended up
with some rather large binary files in its Git history, many of which
correspond to media content no longer accessible on the actual deployed
website.

I put together a tool to scape this repository's commit history and dump the
largest files from that history to the terminal, sorted by size:

```sh
#!/bin/sh
#
# Usage: git-history-largest-files.sh [number-of-files]
set -eu

COUNT=${1:-10}
BYTES_PER_GB=1073741824
BYTES_PER_MB=1048576
BYTES_PER_KB=1024

git rev-list --objects --all | \
    git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' | \
    awk '/^blob/ && $4 != "" {print $3 "\t" $4}' | \
    sort -rn | \
    head -n "$COUNT" | \
    awk -F'\t' \
        -v gb="$BYTES_PER_GB" \
        -v mb="$BYTES_PER_MB" \
        -v kb="$BYTES_PER_KB" \
        '{
            size = $1
            path = $2

            if (size >= gb) {
                printf "%.2f GB %s\n", size/gb, path
            }
            else if (size >= mb) {
                printf "%.2f MB %s\n", size/mb, path
            }
            else if (size >= kb) {
                printf "%.2f KB %s\n", size/kb, path
            }
            else {
                printf "%d B %s\n", size, path
            }
        }'
```

Running the tool over this repo, we can see that a massive percentage of this
repository's size comes from this [Braille Apple video](https://vimeo.com/636580388)
that I had actually moved from this site to Vimeo a while back.

```sh
$ git gc >/dev/null 2>&1 && git count-objects -vH
count: 0
size: 0 bytes
in-pack: 1523
packs: 2
size-pack: 109.24 MiB
prune-packable: 0
garbage: 0
size-garbage: 0 bytes
$ sh tools/git-history-largest-files.sh
47.50 MB src/misc/2021-10-18-braille-apple.webm
28.72 MB src/misc/2021-10-18-braille-apple.webm
3.93 MB src/blog/2022-03-19-breaking-cookie-clicker/living-room-pi.jpg
3.82 MB src/blog/2022-03-19-breaking-cookie-clicker/supplies.jpg
3.76 MB src/blog/2022-03-19-breaking-cookie-clicker/living-room-pi-and-tv.jpg
2.13 MB src/blog/2022-03-19-breaking-cookie-clicker/cookie-clicker-final-stats.png
2.08 MB src/misc/2020-07-04-disco-descent-1-1.mp3
1.87 MB src/blog/2024-09-02-scripting-with-value-semantics-using-lumpy/lumpy-game.mp4
1.57 MB src/blog/2022-03-19-breaking-cookie-clicker/cookie-clicker-auto-clicker.png
1.18 MB src/misc/2020-08-16-dvd-screensaver/2020-08-16-dvd-screensaver.mp4
```


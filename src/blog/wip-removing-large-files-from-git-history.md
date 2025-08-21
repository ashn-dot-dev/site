Removing Large Files from a Repository's Git History
====================================================

I consider myself to be a [grug-brained developer](https://grugbrain.dev/). I
enjoy building and maintaining software systems that eschew complexity in favor
of readability and ease of understanding. This website is one of those systems.
The entire thing is built as a static site from a bunch of loose HTML and
Markdown files all jammed together using a dead simple templating system, and
all of the media content (images, videos, etc.) is stored directly in the Git
repository alongside the textual content. As a result of this grug-brained
structure, the website repository has ended up with some rather large binary
files in its Git history, many of which correspond to media content no longer
accessible on the actual deployed website.

I put together a tool to scrape this repository's commit history and dump the
largest files from that history to the terminal, sorted by size:

```sh
#!/bin/sh
#
# Usage: git-history-largest-files.sh [NUMBER-OF-FILES]
set -eu

if [ $# -ge 2 ]; then
    echo "Usage: $0 [NUMBER-OF-FILES]"
    exit 1
fi

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

I don't actually *need* to keep this `src/misc/2021-10-18-braille-apple.webm`
video around in the repository git history; the content is stored elsewhere,
and this site is much more of a living document than it is a historical
archive. So I am fine removing this file if it shrinks the repository size and
gives me more leeway to jam different media blobs in later.

Okay so now that we have a file path, let's use `git filter-branch`[^1] to
replace the binary file with a placeholder across the entire git history, and
then yeet remaining references to the old objects/commits.

```sh
#!/bin/sh
#
# Usage: git-history-elide-file.sh FILE
set -eu

if [ $# -ne 1 ]; then
    echo "Usage: $0 FILE"
    exit 1
fi

FILE="$1"

export FILE
export FILTER_BRANCH_SQUELCH_WARNING=1
git filter-branch -f --tree-filter '
if git ls-files | grep -q "^${FILE}$"; then
    mkdir -p "$(dirname "${FILE}")"
    echo "original file removed from git history" > "${FILE}"
fi
' -- --all
rm -rf .git/refs/original/
git reflog expire --expire=now --all
git gc --prune=now --aggressive
```

Running this tool we can see that it removed all of that repository bloat:

```sh
$ sh tools/git-history-elide-file.sh src/misc/2021-10-18-braille-apple.webm
Rewrite e326cb816c72b2986ab9d6a7472ee3999860936b (287/294) (9 seconds passed, remaining 0 predicted)
WARNING: Ref 'refs/heads/main' is unchanged
WARNING: Ref 'refs/remotes/github/main' is unchanged
WARNING: Ref 'refs/remotes/github/main' is unchanged
WARNING: Ref 'refs/remotes/sourcehut/main' is unchanged
Enumerating objects: 1513, done.
Counting objects: 100% (1513/1513), done.
Delta compression using up to 16 threads
Compressing objects: 100% (1495/1495), done.
Writing objects: 100% (1513/1513), done.
Total 1513 (delta 973), reused 387 (delta 0), pack-reused 0
$ git count-objects -vH
count: 0
size: 0 bytes
in-pack: 1513
packs: 1
size-pack: 33.01 MiB
prune-packable: 0
garbage: 0
size-garbage: 0 bytes
$ sh tools/git-history-largest-files.sh 5
3.93 MB src/blog/2022-03-19-breaking-cookie-clicker/living-room-pi.jpg
3.82 MB src/blog/2022-03-19-breaking-cookie-clicker/supplies.jpg
3.76 MB src/blog/2022-03-19-breaking-cookie-clicker/living-room-pi-and-tv.jpg
2.13 MB src/blog/2022-03-19-breaking-cookie-clicker/cookie-clicker-final-stats.png
2.08 MB src/misc/2020-07-04-disco-descent-1-1.mp3
```

After getting rid of the `src/misc/2021-10-18-braille-apple.webm` file, we
observe that we reduced our pack size from ~110 MB to ~33 MB. That size
reduction is good enough for now, but there are plenty of other now-deleted
files that I could remove in the future!

## Footnotes
[^1]:
Yes, we probably should be using the more modern `git-filter-repo`. And yes, I
*was* too lazy to install it on my machine for this blog post; I am grug
brained and `git filter-branch` was already on my machine and perfectly usable.

Creating a C Documentation Generator - Part 3
=============================================

Welcome to part 3 of a series in which we design and implement `cdoc` - a
source-code documentation tool for the C programming language.
In [part 2](/blog/2020-02-22-creating-cdoc-part-2.html) we defined the rough
shape of our documentation language and revamped our example file.
In this post we will write the code to parse a C source file with `cdoc`
comments and then output HTML based on those comments.

As a quick refresher we want to turn documentation that looks like this:

```c
//! @function swap
//!     Exchange two objects of equal size.
//! @param p1
//!     Pointer to the first object.
//! @param p2
//!     Pointer to the second object.
//! @param size
//!     The sizeof both objects.
//! @note
//!     Objects p1 and p2 may point to overlapping memory.
//!     E.g swap(&foo, &foo, sizeof(foo)) is valid.
void swap(void* p1, void* p2, size_t size);
```

into HTML based on these rules:

+ A **`doc`** consists of lines beginning with the characters "`//!`", divided
into one or more `section`s.
+ A **`section`** consists of a line in either of the following forms:
```c
//! @TAG
```
```c
//! @TAG NAME
```
followed by zero or more lines of HTML text.


## The Game Plan

00. Read the C source into a buffer
00. Split that buffer into lines
00. Parse `doc`(s) from every group of lines beginning with "`//!`
    + Recognize lines starting with "`//!`"
    + Define the data structures for `doc` and `section`
    + Parse lines starting with "`//!`" into `doc`s/`section`s
00. Print an HTML representation of each `doc` and its `section`s


## Read the C Source Into a Buffer
Out `do_file` function already has the basic structure in place for reading a
file.
All we really need to do is create a wrapper function around:

```c
while ((c = fgetc(fp)) != EOF) { /* do stuff */ }
```

that will build and return a heap-allocated buffer of all characters read via
`fgetc`.
This wrapper should also make sure that no `NUL` bytes are encountered and no
read errors occur, that way we know that the cstring produced contains the
entire contents of a given file.

```c
// Returns the contents of stream as a NUL-terminated heap-allocated cstring.
static char*
read_text_file(FILE* stream)
{
    char* buf = NULL;
    size_t len = 0;

    int c;
    while ((c = fgetc(stream)) != EOF)
    {
        if (c == '\0')
        {
            fputs("error: Encountered illegal NUL byte!\n", stderr);
            exit(EXIT_FAILURE);
        }
        buf = realloc(buf, len + 1);
        assert(buf != NULL);
        buf[len++] = (char)c;
    }
    if (!feof(stream))
    {
        fputs("error: Failed to read entire text file!\n", stderr);
        exit(EXIT_FAILURE);
    }

    buf = realloc(buf, len + 1);
    assert(buf != NULL);
    buf[len++] = '\0';
    return buf;
}
```

We can check that the `read_text_file` works as expected by rewriting our
pseudo-`cat` `do_file` function as:

```c
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    fputs(text, stdout);
    free(text);
}
```

and running `cdoc` against the `example.c` file (left as an exercise for the
reader).

## Split Buffer Into Lines
Once we have the contents of a file in one cstring it is almost trivial to
split it up into multiple lines.
The newline character (`'\n'`) and a cstring's NUL terminator (`'\0'`) both
take up one byte in memory, so if we replace every newline with a NUL byte then
our single large file-cstring will get transformed into a bunch of little
line-cstrings almost for free.

```c
// Transform the NUL-terminated text into a heap-allocated NULL-ended list of
// NUL-terminated cstrings by replacing newline characters with NUL bytes.
// Modifies text in place.
static char**
text_to_lines(char* text)
{
    char** lines = realloc(NULL, (strlen(text) + 1) * sizeof(char*));
    assert(lines != NULL);
    size_t count = 0;
    lines[count++] = text;

    for (; *text; ++text)
    {
        if (*text != '\n') continue;
        *text = '\0';
        lines[count++] = text + 1;
    }
    lines[count - 1] = NULL;

    return lines;
}
```

Once again we can check that things are working as expected by rewrite our
pseudo-`cat` `do_file` function using `text_to_lines`:

```c
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    char** const lines = text_to_lines(text);
    for (char** ln = lines; *ln != NULL; ++ln) { puts(*ln); }
    free(text);
    free(lines);
}
```

and running `cdoc` against the `example.c` file (again left as an exercise for
the reader).


## Recognize lines starting with "`//!`"
We are now starting to drift into the realm of parsing, so before we go any
further I want to add some parsing utilities that will help us out in these next
few sections.

First I want to add a couple more global variables to hold stateful information
about the current file being parsed.
We will put these new globals just below our declaration of `FILE* fp` just so
that we can keep all of our global state together.

```c
static FILE* fp = NULL;
char** lines; // NULL-terminated list of all lines in the file.
char** linep; // Pointer to the current line.
#define LINENO ((int)(linep - lines + 1))
```

With these new global variables we can once again rewrite the pseudo-`cat`
`do_file` as:

```c
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    lines = text_to_lines(text);
    linep = lines;
    while (*linep != NULL) puts(*linep++);
    free(text);
    free(lines);
}
```

Alright next utility I want to add is a function that checks if a character is
horizontal whitespace.
In part 2 we decided that a doc line can begin and end with any amount of
horizontal whitespace, so we should probably have a function that checks
whether a character satisfies the definition of horizontal whitespace.
For our purposes we will say that horizontal whitespace is a tab,
carriage return, or space character.
The horizontal whitespace checking function, `is_hspace`, is therefore just the
logical or of checks for each of those characters.

```c
static bool
is_hspace(int c)
{
    return c == '\t' || c == '\r' || c == ' ';
}
```

Okay last utilities I want to add are functions that will (1) check whether a
line satisfies the definition of a doc-comment-line (a line with "`//!`" as its
first non-whitespace characters) and (2) find the first non-whitespace
characters after the "`//!`" on a doc-comment-line.
Both functions take a `char` pointer and walk that pointer along the string
using `is_hspace` until non-horizontal-whitespace is found.

```
static bool
is_doc_line(char const* line)
{
    if (line == NULL) return false;
    while (is_hspace(*line)) line += 1;
    if (*line++ != '/') return false;
    if (*line++ != '/') return false;
    if (*line++ != '!') return false;
    return true;
}

static char const*
doc_content_start(char const* cp)
{
    while (is_hspace(*cp)) cp += 1; // Whitespace before '//!'
    assert(cp[0] == '/');
    assert(cp[1] == '/');
    assert(cp[2] == '!');
    cp += 3; // The '//!'
    while (is_hspace(*cp)) cp += 1; // Whitespace after '//!'
    return cp;
}
```

I explicitly added a NULL check to `is_doc_line` since our `lines` global is
NULL terminated and I do not want to run into the situation where we attempt to
dereference NULL.
We may want to remove this line later if it turns out the check is never needed,
but for now I want to stay on the side of caution.
On the other hand `doc_content_start` will probably only ever get called on a
line that has been checked with `is_doc_line`, so that function is a little bit
more lax with the error checking.
I threw some `assert`s in there, but realistically I am going to wait until we
do code cleanup before I seriously look at this function again.

Let's test our `is_hspace` and `is_doc_line` functions to make sure they are
working as expected.
We will once again modify our `do_file` function, but this time we will
(finally) do something other than emulate `cat`: we will iterate through all the
lines of a file and print only the doc-lines.

```c
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    lines = text_to_lines(text);
    linep = lines;

    while (*linep != NULL)
    {
        if (is_doc_line(*linep)) puts(*linep);
        linep += 1;
    }

    free(text);
    free(lines);
}
```
```sh
$ make clean cdoc && ./cdoc example.c
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
//! @file example.c
//!     This is a C source file used to test cdoc.
//! @license 0BSD
//! @function swap
//!     Exchange two objects of equal size.
//! @param p1
//!     Pointer to the first object.
//! @param p2
//!     Pointer to the second object.
//! @param size
//!     The sizeof both objects.
//! @note
//!     Objects p1 and p2 may point to overlapping memory.
//!     E.g swap(&foo, &foo, sizeof(foo)) is valid.
//! @struct string
//!     POD type representing an array-of-char with a known size.
//! @union foobar
//!     A data packet sent over the wire from FooBar Inc.
//! @enum color
//! @todo
//!     Add more colors.
//! @typedef colour
//!     Convenient typedef for non-American software engineers.
//! @function get_color
//! Cdoc should be lenient with whitespace.
//!     Any amount of indenting should be allowed.
//!	Even with tabs.
//! @return
//!     My favorite color.
//! @macro M_PER_KM
//! @macro KM
//!     Convert meters into kilometers.
//! @macro NUM_FOOBAR
//!     256 is a computer-ish number, right?
//! @variable foobars
```

And what do ya' know, it actually works!
I mean realistically I wouldn't be showing you this source code if it didn't
work, and I definitely had to rewrite these functions a bunch of times before
they took the form you see here, but this is a blog post so just hush hush and
pretend I am actually competent enough to write these first-try.
I also tried running the `do_file` function with
`puts(doc_content_start(*linep))` instead of `puts(*linep)` and the results
convinced me that our `doc_content_start` function also seems to be working just
fine.


## Doc and Section Data Structures
We have utilities in place for handling lines of text, but we are still missing
the data structures and supporting functions for `doc`s and `section`s.
Our definition of a `doc` is basically just an ordered sequence of `section`s,
so our `doc` data structure only needs to act as a wrapper around a `section`
array.

```c
struct doc
{
    struct section* sections; // dynamically allocated
    size_t section_count;
};
```

Sections are slightly more complicated because we have to deal with slices of
text for the TAG and NAME components of the tag line plus a slice of lines for
all of the text under the tag line.
The characters/lines we need to slice are all already in memory from when we
parsed all the lines of the file, so all we will need to do is fill out a start
and length data fields for the tag, name, and lines of text.

```c
struct section
{
    char const* tag_start;
    int tag_len;

    char const* name_start;
    int name_len; // name_len == 0 implies tag line has no name

    char** text_start;
    int text_len;
};
```


## Parsing and Printing
Okay here comes the fun fun part of the blog post: parsing and printing!
This is the real meat of the documentation generator (or really any
compiler/transpiler).
In the parsing phase of `cdoc` we are going to traverse the lines of text in our
file and attempt to map them onto our `doc` and `section` data structures.
Then in the printing phase we are going to spit out an HTML representation of
those data structures as illustrated by my sub-par ASCII art.

```txt
+------+  PARSING  +-----------+  PRINTING  +------+
| TEXT +---------->+ STRUCT(S) +----------->+ HTML |
+------+           +-----------+            +------+
```

In a real compiler™ the printing phase is usually called "code generation" or
"output", but this is our project so we can call it whatever we want.

We are going to associate `parse` and `print` functions with each of our data
structures.
Every `parse` function should attempt to parse its associated data structure
using the global parse state we defined a few sections ago.
The `parse` function should return the populated data structure on success or
print an error message and `exit` if a problem is encountered.
Every `print` function should its associated data structure as input and write an
HTML representation of that data structure to stdout.

```c
static struct doc
parse_doc(void);
static void
print_doc(struct doc const d);

static struct section
parse_section(void);
static void
print_section(struct section const s);
```

We said that a `doc` consists of one or more sections, and right now we have
those sections represented as a heap allocated array in `struct doc`, so the
`parse_doc` function will repeatedly try to parse sections and append them to
the back of the array.
We will know that we have parsed all sections of the doc when the current line
longer begins with "`//!`".

```c
static struct doc
parse_doc(void)
{
    struct doc d = {0};
    while (is_doc_line(*linep))
    {
        d.sections =
            realloc(d.sections, (d.section_count + 1) * sizeof(*d.sections));
        assert(d.sections != NULL);
        d.sections[d.section_count++] = parse_section();
    }
    return d;
}
```

The `parse_section` function is a bit beefier, but should still be relatively
easy to follow.
First we perform some basic error checking to make sure that a doc section
begins with a non-empty tag.
We then walk a character along the tag line recording TAG and NAME (if it
exists) while skipping over whitespace.
Right now there is no check for a tag line with *more* than TAG and NAME such
as:

```c
//! @foo bar extra text
```

so that might be something to add in the future.
After parsing the tag line we iterate over remaining lines until we hit the
end of the doc (`is_doc_line(*linep)` would be false) or we reach another
section (`*doc_content_start(*linep) != '@'` would be `true`).


```c
static struct section
parse_section(void)
{
    struct section s = {0};
    char const* cp = doc_content_start(*linep);
    if (*cp++ != '@')
    {
        fprintf(
            stderr,
            "error(line %d): Doc-section must begin with @<TAG>\n",
            LINENO);
        exit(EXIT_FAILURE);
    }
    if (is_hspace(*cp))
    {
        fprintf(stderr, "error(line %d): Empty doc-comment tag\n", LINENO);
        exit(EXIT_FAILURE);
    }

    // TAG
    s.tag_start = cp;
    while (!is_hspace(*cp) && (*cp != '\0')) cp += 1;
    s.tag_len = (int)(cp - s.tag_start);

    while (is_hspace(*cp)) cp += 1;

    // NAME
    s.name_start = cp;
    while (!is_hspace(*cp) && (*cp != '\0')) cp += 1;
    s.name_len = (int)(cp - s.name_start);

    // TEXT
    linep += 1;
    s.text_start = linep;
    while (is_doc_line(*linep) && *doc_content_start(*linep) != '@') linep += 1;
    s.text_len = (int)(linep - s.text_start);

    return s;
}
```


---

---

---

## Outputting HTML

```c
static void
print_doc(struct doc d);
```
```c
static void
print_section(struct section s);
```

```c
```

```sh
$ make clean cdoc > /dev/null && ./cdoc example.c
<h3>file: example.c</h3>
This is a C source file used to test cdoc.
<h3>license: 0BSD</h3>
<hr>
<h3>function: swap</h3>
Exchange two objects of equal size.
<h3>param: p1</h3>
Pointer to the first object.
<h3>param: p2</h3>
Pointer to the second object.
<h3>param: size</h3>
The sizeof both objects.
<h3>note: </h3>
Objects p1 and p2 may point to overlapping memory.
E.g swap(&foo, &foo, sizeof(foo)) is valid.
<hr>
<h3>struct: string</h3>
POD type representing an array-of-char with a known size.
<hr>
<h3>union: foobar</h3>
A data packet sent over the wire from FooBar Inc.
<hr>
<h3>enum: color</h3>
<h3>todo: </h3>
Add more colors.
<hr>
<h3>typedef: colour</h3>
Convenient typedef for non-American software engineers.
<hr>
<h3>function: get_color</h3>
Cdoc should be lenient with whitespace.
Any amount of indenting should be allowed.
Even with tabs.
<h3>return: </h3>
My favorite color.
<hr>
<h3>macro: M_PER_KM</h3>
<hr>
<h3>macro: KM</h3>
Convert meters into kilometers.
<hr>
<h3>macro: NUM_FOOBAR</h3>
256 is a computer-ish number, right?
<hr>
<h3>variable: foobars</h3>
<hr>
```


## Putting it all Together
Generated HTML can be found [here](/blog/wip-creating-cdoc-part-3/example.html).

The source code for this blog post can be found
[here(TODO)](TODO).


## Footnotes
<div id="ft1">\[1\]:
TODO
</div>

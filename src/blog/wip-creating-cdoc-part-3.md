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
We basically just want the fancy version of this:
```
int c;
while ((c = fgetc(fp)) != EOF) { fputc(c, stdout); }
```
but for adding to a buffer instead of `fputc` to `stdout`.

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

```c
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    fputs(text, stdout);
    free(text);
}
```


## Split Buffer Into Lines
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
        if (*text != '\n') { continue; }
        *text = '\0';
        lines[count++] = text + 1;
    }
    lines[count - 1] = NULL;

    return lines;
}
```

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
This gives us the same result as before.


## Recognize lines starting with "`//!`"
Might as well ad global parse state here:

```c
static FILE* fp = NULL;
char** lines; // NULL-terminated list of all lines in the file.
char** linep; // Pointer to the current line.
#define LINENO ((int)(linep - lines + 1))
```

```c
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    lines = text_to_lines(text);
    linep = lines;
    while (*linep != NULL) { puts(*linep++); }
    free(text);
    free(lines);
}
```

Horazontal whitespace parsing.
Check if is doc line.

```c
static bool
is_hspace(int c)
{
    return c == '\t' || c == '\r' || c == ' ';
}

static bool
is_doc_line(char const* line)
{
    if (line == NULL) { return false; }
    while (is_hspace(*line)) { line += 1; }
    if (*line++ != '/') { return false; }
    if (*line++ != '/') { return false; }
    if (*line++ != '!') { return false; }
    return true;
}
```

Parse only doc lines:

```c
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    lines = text_to_lines(text);
    linep = lines;

    while (*linep != NULL)
    {
        if (is_doc_line(*linep)) { puts(*linep); }
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


## Doc and Section Data Structures

```c
struct doc
{
    struct section* sections;
    size_t section_count;
};
static struct doc
parse_doc(void);
static void
free_doc(struct doc d);

struct section
{
    char const* tag_start;
    int tag_len;

    char const* name_start;
    int name_len;

    char const** text_lines;
    size_t text_line_count;
};
static struct section
parse_section(void);
static void
free_section(struct section s);
```


## Parsing Docs and Sections

```c
static char const*
doc_content_start(char const* cp)
{
    while (is_hspace(*cp)) cp += 1; // Whitespace before '//!'
    cp += 3; // The '//!'
    while (is_hspace(*cp)) cp += 1; // Whitespace after '//!'
    return cp;
}

static void
do_file(void)
{
    char* const text = read_text_file(fp);
    lines = text_to_lines(text);
    linep = lines;

    struct doc* docs = NULL;
    size_t doc_count = 0;
    while (*linep != NULL)
    {
        if (!is_doc_line(*linep))
        {
            linep += 1;
            continue;
        }
        docs = realloc(docs, (doc_count + 1) * sizeof(*docs));
        assert(docs != NULL);
        docs[doc_count++] = parse_doc();
    }

    for (size_t i = 0; i < doc_count; ++i) free_doc(docs[i]);
    free(docs);
    free(text);
    free(lines);
}

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

static void
free_doc(struct doc d)
{
    for (size_t i = 0; i < d.section_count; ++i) free_section(d.sections[i]);
    free(d.sections);
}

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

    s.tag_start = cp;
    while (!is_hspace(*cp) && (*cp != '\0')) cp += 1;
    s.tag_len = (int)(cp - s.tag_start);

    while (is_hspace(*cp)) cp += 1;

    s.name_start = cp;
    while (!is_hspace(*cp) && (*cp != '\0')) cp += 1;
    s.name_len = (int)(cp - s.name_start);

    linep += 1;
    while (is_doc_line(*linep))
    {
        if (doc_content_start(*linep)[0] == '@') break;
        s.text_lines = realloc(
            s.text_lines, (s.text_line_count + 1) * sizeof(*s.text_lines));
        s.text_lines[s.text_line_count++] = doc_content_start(*linep);
        linep += 1;
    }
    return s;
}

static void
free_section(struct section s)
{
    free(s.text_lines);
}
```


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
static void
do_file(void)
{
    char* const text = read_text_file(fp);
    lines = text_to_lines(text);
    linep = lines;

    struct doc* docs = NULL;
    size_t doc_count = 0;
    while (*linep != NULL)
    {
        if (!is_doc_line(*linep))
        {
            linep += 1;
            continue;
        }
        docs = realloc(docs, (doc_count + 1) * sizeof(*docs));
        assert(docs != NULL);
        docs[doc_count++] = parse_doc();
    }

    for (size_t i = 0; i < doc_count; ++i) print_doc(docs[i]);

    for (size_t i = 0; i < doc_count; ++i) free_doc(docs[i]);
    free(docs);
    free(text);
    free(lines);
}
```

```c
static void
print_doc(struct doc d)
{
    for (size_t i = 0; i < d.section_count; ++i) print_section(d.sections[i]);
    puts("<hr>");
}
```
```c

static void
print_section(struct section s)
{
    printf(
        "<h3>%.*s: %.*s</h3>\n",
        s.tag_len,
        s.tag_start,
        s.name_len,
        s.name_start);
    for (size_t i = 0; i < s.text_line_count; ++i) puts(s.text_lines[i]);
}
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

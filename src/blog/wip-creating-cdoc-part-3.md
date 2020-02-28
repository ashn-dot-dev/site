Creating a C Documentation Generator - Part 3
=============================================

Welcome to part 3 of a series in which we design and implement `cdoc` - a
source-code documentation tool for the C programming language.
In [part 2](/blog/2020-02-22-creating-cdoc-part-2.html) defined the rough shape
of our documentation language and revamped our example file.
In this post we will write the logic that will parse a C source file and output
HTML based on the documentation in that file.

## Reading File Into Lines

### Read file into buffer

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


### Split buffer into lines

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

## Parsing Docs

### Global parse state

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


### Extract doc lines

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

## Putting it all Together
The source code for this blog post can be found
[here(TODO)](TODO).

## Footnotes
<div id="ft1">\[1\]:
TODO
</div>

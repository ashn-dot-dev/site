Creating a C Documentation Generator - Part 5
=============================================

Welcome to part 5 of a series in which we design and implement `cdoc` - a
source code documentation tool for the C programming language.
In this post we will write the code to parse C source associated with some of
our @tagged C constructs.
This will (hopefully) be the last parsing code we write for the `cdoc` 0.1
release.

## What Are We Trying to Do?

Our declaration of `struct string` in `example.c` is currently transformed from
this C source:

```c
//! @struct string
//!     POD type representing an array-of-char with a known size.
struct string
{
    char* data;
    size_t size;
};
```

into the following HTML:

```html
<h3>struct: string</h3>
POD type representing an array-of-char with a known size.
<hr>
```

Our goal for this post is add the source code for constructs such as
`struct string` in our generated HTML output:

```html
<h3>struct: string</h3>
POD type representing an array-of-char with a known size.
<pre><code>
struct string
{
    char* data;
    size_t size;
};
</code></pre>
```

We want to do this for all `struct`s, `union`s, `enum`s, `typedef`s, variable
declarations, function prototypes, function definitions, and macros (basically
all of the C constructs we have in `example.c`).
Let's start off trying to get `struct` declarations working and then add the
other C constructs as we go along.

--------------------------------------------------------------------------------

## Add a `source` Member to `struct doc`

We will assume that the source code for any C construct can be represented as
a list of lines within a C source file.
*Technically* `struct` declarations may span multiple source files, and
*technically* multiple `struct` declarations may occur on a single line,
but this is so infrequent that the "list of lines" source code representation is
adequate for our purposes.

We create a new member, `source`, in `struct doc` to hold the list of source
lines:

```c
struct doc
{
    struct section* sections; // dynamically allocated
    size_t section_count;

    // NULL-terminated list of associated source code.
    char** source; // dynamically allocated
};
```

We then update the cleanup step in `do_file` to account for this dynamically
allocated member by changing the line:

```c
    // CLEANUP
    for (size_t i = 0; i < doc_count; ++i) free(docs[i].sections);
```

into the lines:

```c
    // CLEANUP
    for (size_t i = 0; i < doc_count; ++i)
    {
        free(docs[i].sections);
        free(docs[i].source);
    }
```

--------------------------------------------------------------------------------

## Is a `struct` Being Documented?

If you remember back in part 2 of this series we said:

> As a convention the first section of a doc should have a tag that describes
> the C construct being documented.

If we again look at `struct string` we see that its first (and only) doc-section
has the tag `struct`:

```c
//! @struct string
//!     POD type representing an array-of-char with a known size.
```

We should attempt to parse a struct if and only if the first section of a `doc`
has a tag that string-compares equal to `"struct"`.
The `section` data structure represents a tag as a slice-of-`char`, so
performing an equality string comparison against the cstring `"struct"`
will look something like:

```c
len == strlen("struct") && strncmp(start, "struct", len) == 0
```

If we jam that string comparison expression into our `parse_doc` function, we
see that it does in fact find the `struct` documentation in `example.c`:

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

    char const* const start = d.sections[0].tag_start;
    size_t const len = (size_t)d.sections[0].tag_len;
    if (len == strlen("struct") && strncmp(start, "struct", len) == 0)
    {
        puts("FOUND A STRUCT");
        exit(EXIT_SUCCESS);
    }

    return d;
}
```

```sh
$ make clean cdoc && ./cdoc example.c
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
FOUND A STRUCT
```

--------------------------------------------------------------------------------

## Parsing `struct`s

Once we have identified that a struct has been documented we will need to parse
its associated source code.
As I have mentioned before, we *really* do not want to go down the rabbit hole
of writing a C parser.
Instead of writing real `struct` parsing code™ we will use a nifty trick to
fake-kinda-sorta-close-enough parse a `struct`.
Note that any valid struct declaration (forward declaration or full declaration)
must satisfy the following:

00. The number of opening-braces in the declaration is equal to the number of
    closing-braces in the declaration 
00. The declaration ends in a semicolon

So the dumb way to parse a `struct` declaration is to just loop through the
characters of each line until a `';'` is encountered *and* the number of `'{'`
characters minus the number of `'}'` characters is zero.

```c
static char**
parse_struct_source(void)
{
    char** lines = NULL;
    size_t count = 0;

    bool parsed = false; // Are we finished parsing the source?
    int brackets = 0; // Number of '{' minus number of '}'.
    for (; !parsed; linep += 1)
    {
        if (linep == NULL) errorf("Unexpected end-of-file");
        lines = xalloc(lines, (count + 1) * sizeof(char*));
        lines[count++] = *linep;

        for (char* cp = *linep; *cp != '\0'; ++cp)
        {
            brackets += *cp == '{';
            brackets -= *cp == '}';
            if (brackets == 0 && *cp == ';') parsed = true;
        }
    }

    lines = xalloc(lines, (count + 1) * sizeof(char*));
    lines[count] = NULL;
    return lines;
}
```

We should take careful note that this method of parsing a `struct` has a fatal
flaw: comments are *not* ignored.
If the comment `/* }; */` exists within the `struct` declaration then the parsing
code will incorrectly count the `'}'` and `';'` characters even though they are
"invisible" to the C compiler.
We can always revisit this function later, so for now let's ignore this issue
and focus on getting things working.

We will plug this function into `parse_doc` where we had our `puts` + `exit`
before, and then add some logic in `print_doc` to write out the source lines:

```c
static struct doc
parse_doc(void)
{
    struct doc d = {0};
    while (is_doc_line(*linep))
    {
        d.sections =
            xalloc(d.sections, (d.section_count + 1) * sizeof(*d.sections));
        d.sections[d.section_count++] = parse_section();
    }

    char const* const start = d.sections[0].tag_start;
    size_t const len = (size_t)d.sections[0].tag_len;
    if (len == strlen("struct") && strncmp(start, "struct", len) == 0)
    {
        d.source = parse_struct_source();
    }

    return d;
}
```

```c
static void
print_doc(struct doc const d)
{
    for (size_t i = 0; i < d.section_count; ++i) print_section(d.sections[i]);
    if (d.source != NULL)
    {
        puts("<pre><code>");
        char** ln = d.source;
        while (*ln != NULL) puts(*ln++);
        puts("</code></pre>");
    }
    puts("<hr>");
}
```

If we test out this code on `example.c` we see that everything works as
expected:

```sh
$ make clean cdoc && ./cdoc example.c > example0.html
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
```

The generated `example0.html` output can be found
[here](/blog/wip-creating-cdoc-part-5/example0.html)
and contains a copy of the source for `struct string` from `example.c`.
Hooray!

--------------------------------------------------------------------------------

## Additional Declarations

In addition to `struct` declarations we also want to parse `union`, `enum`,
`typedef`, and variable declarations.
Conveniently the nifty trick for parsing `struct` declarations happens to apply
for all of these cases as well.
Rather than write a function for each of these C constructs, we can duplicate
the string comparison check for each case and reuse `parse_struct_source` for
all of them:

```c
static struct doc
parse_doc(void)
{
    struct doc d = {0};
    while (is_doc_line(*linep))
    {
        d.sections =
            xalloc(d.sections, (d.section_count + 1) * sizeof(*d.sections));
        d.sections[d.section_count++] = parse_section();
    }

    char const* const start = d.sections[0].tag_start;
    size_t const len = (size_t)d.sections[0].tag_len;
#define DOC_IS(tag) \
    (len == strlen(tag) && strncmp(start, tag, len) == 0)
    if (DOC_IS("struct") || DOC_IS("union") || DOC_IS("enum")
        || DOC_IS("typedef") || DOC_IS("variable"))
    {
        d.source = parse_struct_source();
    }
#undef DOC_IS

    return d;
}
```

I decided to use a local macro, `DOC_IS`, to avoid repetition.
The code is functionally identical to its previous incarnation, just with more
cases added to the `if`-statement.

```sh
$ make clean cdoc && ./cdoc example.c > example1.html
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
```

The generated `example1.html` output can be found
[here](/blog/wip-creating-cdoc-part-5/example1.html)
and looks like it was able to correctly parse the source for all of the C
constructs we added.

--------------------------------------------------------------------------------

## Parsing Functions

Functions require slightly different parsing code than `struct` declarations:
a function may be forward declared:

```c
int foo(double bar);
```

or defined without a forward declaration:

```c
int foo(double bar)
{
    // bla bla bla internal function stuff
}
```

We want `cdoc` to be able to handle both cases, and while our
trick for parsing `struct` declarations *would* work with a forward declared
function, that trick would fail to parse a function definition.
We will create a function that is mostly copy-pasted from `parse_struct_source`,
but will check for a `';'` or a `'{'` and stop parsing if it either is
encountered.
In the case of a function definition (`'{'` is encountered) a bonus line,
`"/* function definition... */"` will be appended to the source lines to let
the reader know that the function implementation exists, but is not presented
in generated documentation.

```c
static char**
parse_function_source(void)
{
    char** lines = NULL;
    size_t count = 0;

    bool parsed = false;
    for (; !parsed; linep += 1)
    {
        if (linep == NULL) errorf("Unexpected end-of-file");
        lines = xalloc(lines, (count + 1) * sizeof(char*));
        lines[count++] = *linep;

        for (char* cp = *linep; *cp != '\0'; ++cp)
        {
            if (*cp == ';')
            {
                parsed = true;
                break;
            }
            else if (*cp == '{')
            {
                parsed = true;
                lines = xalloc(lines, (count + 1) * sizeof(char*));
                lines[count++] = "/* function definition... */";
            }
            if (*cp == '{' || *cp == ';') parsed = true;
        }
    }

    lines = xalloc(lines, (count + 1) * sizeof(char*));
    lines[count] = NULL;
    return lines;
}
```

```c
static struct doc
parse_doc(void)
{
    struct doc d = {0};
    while (is_doc_line(*linep))
    {
        d.sections =
            xalloc(d.sections, (d.section_count + 1) * sizeof(*d.sections));
        d.sections[d.section_count++] = parse_section();
    }

    char const* const start = d.sections[0].tag_start;
    size_t const len = (size_t)d.sections[0].tag_len;
#define DOC_IS(tag) \
    (len == strlen(tag) && strncmp(start, tag, len) == 0)
    if (DOC_IS("struct") || DOC_IS("union") || DOC_IS("enum")
        || DOC_IS("typedef") || DOC_IS("variable"))
    {
        d.source = parse_struct_source();
    }
    else if (DOC_IS("function"))
    {
        d.source = parse_function_source();
    }
#undef DOC_IS

    return d;
}
```

```sh
$ make clean cdoc && ./cdoc example.c > example2.html
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
```

The generated `example2.html` output can be found
[here](/blog/wip-creating-cdoc-part-5/example2.html)
and now contains source code for the `swap` and `get_color` functions found in
`example.c`.

--------------------------------------------------------------------------------

## Parsing Macros

The last C construct we want to parse is preprocessor macros.
These are actually the easiest to parse since macros only end when the last
character of a line is not a backslash.
We will once again copy-paste the `parse_struct_source` function, this time
replacing the inner `for`-loop with a one-liner that checks the last character
of that line:

```sh
static char**
parse_macro_source(void)
{
    char** lines = NULL;
    size_t count = 0;

    bool parsed = false;
    for (; !parsed; linep += 1)
    {
        if (linep == NULL) errorf("Unexpected end-of-file");
        lines = xalloc(lines, (count + 1) * sizeof(char*));
        lines[count++] = *linep;
        parsed = (*linep)[strlen(*linep) - 1] != '\\';
    }

    lines = xalloc(lines, (count + 1) * sizeof(char*));
    lines[count] = NULL;
    return lines;
}
```

```c
static struct doc
parse_doc(void)
{
    struct doc d = {0};
    while (is_doc_line(*linep))
    {
        d.sections =
            xalloc(d.sections, (d.section_count + 1) * sizeof(*d.sections));
        d.sections[d.section_count++] = parse_section();
    }

    char const* const start = d.sections[0].tag_start;
    size_t const len = (size_t)d.sections[0].tag_len;
#define DOC_IS(tag) \
    (len == strlen(tag) && strncmp(start, tag, len) == 0)
    if (DOC_IS("struct") || DOC_IS("union") || DOC_IS("enum")
        || DOC_IS("typedef") || DOC_IS("variable"))
    {
        d.source = parse_struct_source();
    }
    else if (DOC_IS("function"))
    {
        d.source = parse_function_source();
    }
    else if (DOC_IS("macro"))
    {
        d.source = parse_macro_source();
    }
#undef DOC_IS

    return d;
}
```

There are no multi-line macros in `example.c`, so to test multi-line macro
parsing we will change the `KM` macro in `example.c` from:

```c
#define KM(meters) (meters * M_PER_KM)
```

into:

```c
#define KM(meters) \
    (meters * M_PER_KM)
```

```sh
$ make clean cdoc && ./cdoc example.c > example3.html
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
```

The generated `example3.html` output can be found
[here](/blog/wip-creating-cdoc-part-5/example3.html).
As expected the macros are parsed just fine.

--------------------------------------------------------------------------------

## Wrapping Up

The addition of these `parse_*_source` functions gives us a hacky yet functional
way to associate source code with `cdoc` documentation.
With this functionality in place `cdoc` has pretty much everything that I was
looking for in a documentation generator.
In the next post we are going to conclude this mini-series with the alpha 0.1
release of `cdoc`.
I hope to see you then!

The source code for this blog post can be found
[here](https://git.sr.ht/~ashn/cdoc/tree/ee44c0b34df0ccfe6994f9ab86dd44069cb04f55).

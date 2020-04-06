Creating a C Documentation Generator - Part 5
=============================================

Welcome to part 5 of a series in which we design and implement `cdoc` - a
source code documentation tool for the C programming language.
In this post we will write the code to parse C source associated with some of
our @tagged C constructs.
This will (hopefully) be the last parsing code we write for the initial release
of `cdoc`.

## What Are We Trying to Do?

Our declaration for `struct string` in `example.c` is currently transformed from
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

Our goal for this this post is to make it so that the generated HTML will
contain a reference copy of the `struct` declaration in addition to the parsed
`cdoc` comments:

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

We want to do this for `struct`s, `union`s, `enum`s, `typedef`s, variable
declarations, function prototypes, function definitions, and macros (basically
all of the C constructs we have in `example.c`).
Let's start off trying to get `struct` declarations working and then add the
other C constructs one-by-one as we go along.

--------------------------------------------------------------------------------

## Add a `source` Member to `struct doc`

We will assume that the source code for any C construct can be represented as
a list of lines in a C source file.
*Technically* struct declarations may span multiple source files, and
*technically* multiple struct declarations may be declared on a single line,
but this is so infrequent that the "list of lines" representation of source code
is adequate for our purposes.

We create a new member `source` in `struct doc` for this list of lines:

```c
struct doc
{
    struct section* sections; // dynamically allocated
    size_t section_count;

    // NULL-terminated list of associated source code.
    char** source; // dynamically allocated
};
```

And then we should update the cleanup step in `do_file` to account for the new
dynamically allocated member by changing the line:

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
has the tag `@struct`:

```c
//! @struct string
//!     POD type representing an array-of-char with a known size.
```

So we should attempt to parse a struct if and only if the first section of a
`doc` has a tag that string-compares equal to `"struct"`.
The `section` data structure represents a tag with a slice-of-`char`, so
performing a string comparison between that and a cstring will look something
like:

```c
len == strlen("struct") && strncmp(start, "struct", len) == 0
```

If we jam that string comparison expression into the `parse_doc` function, we
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
must satisfy the following properties:

00. The number of opening-braces is equal to the number of closing-braces
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

We should take note however that this method of parsing a struct has a fatal
flaw in that comments are *not* ignored: If the comment `// };` is contained
within the `struct` declaration then the parser will incorrectly count the
`'}'` and `';'` characters even though they are "invisible" to the C compiler.
We can always revisit this function later, so for now let's focus on getting
things working and leave the beatification for another time.

We will plug this function into `parse_doc` where we had our `puts` + `exit`
functions before, and then add some logic to `print_doc` to write out the lines
of source code associated with a `doc` if they exist:

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

In addition to struct declarations we also want to be able to parse `union`,
`enum`, `typedef` and `variable` declarations.
Conveniently the nifty trick for parsing `struct` declarations happens to apply
for all of these cases as well.
So rather than write a function for each of these C constructs, we can duplicate
the string comparison check for each of these cases and call
`parse_struct_source` for all of them:

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

I decided to use a local macro `DOC_IS` to avoid the repetition for each string
comparison, but the code is functionally identical to its previous incarnation,
just with more cases added to the `if`-statement in `parse_doc`.

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

or they may be defined without a forward declaration:

```c
int foo(double bar)
{
    // bla bla bla internal function stuff
}
```

We want `cdoc` to be able to handle documentation for both cases, and while our
trick for parsing `struct` declarations *would* work for a forward declared
function, that trick would fail to parse a function definition.
We will create a function that is mostly copy-pasted from `parse_struct_source`,
but will stop parsing check for both a `';'` or a `'{'` and stop parsing if
it either is encountered.
In the case of a function definition (`'{'` is encountered) a bonus line,
`"/* function definition... */"` will be appended to the source lines to let
the reader know that the function implementation exists, but is not presented
int the generated documentation.

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
and once again seems to be working fine, now containing source code for the
`swap` and `get_color` functions in `example.c`.

--------------------------------------------------------------------------------

## Parsing Macros

The last C construct we want to parse will be preprocessor macros.
These are actually the easiest to parse since macros only end when the last
character of a line is not a backslash.
We will once again copy-paste the `parse_struct_source` function, and replace
the inner `for`-loop with a one-liner that checks the last character of the
line:


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

With the addition of these `parse_*_source` functions we have a hacky yet
functional way to associate source code with `cdoc` documentation.
With source code association in place `cdoc` has pretty much everything that I
was looking for in a documentation generator.
We are not finished development just yet, but I would say we are in the home
stretch.
In the next post we are going to do another round cleanup, similar to what we
did in part 4, in preparation for the 0.1 release of `cdoc`.
I hope to see you then!

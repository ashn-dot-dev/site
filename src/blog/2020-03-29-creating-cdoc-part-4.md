Creating a C Documentation Generator - Part 4
=============================================

Welcome to part 4 of a series in which we design and implement `cdoc` - a
source code documentation tool for the C programming language.
In [part 3](/blog/2020-03-14-creating-cdoc-part-3.html) we finished the minimum
viable product for `cdoc`.
That post took a long time to put together and was filled with a hefty amount
of code.
I though that it would be nice to take a bit of a break and focus on some minor
improvements to what we currently have.

## Replacing `realloc`
Right now there are many places where we dynamically allocate/reallocate memory
with `realloc` and immediately `assert` that a non-`NULL` value was returned.
It is possible that `realloc` may fail[^1], so we *should* have some
form of `NULL`-check in place.
However, `assert` is not best option for doing so: `assert`s are disabled when
`NDEBUG` is defined and if we *always* want to `exit` (or `abort`) on allocation
failure then the logic for doing so should really be handled inside the
allocation function rather than at the call-site.

We will add a function `xalloc` that will behave like `realloc`, but will either
return the amount of memory requested or `exit` on failure.
Additionally, we will narrow the
[specification for realloc](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)
by stating that zero-sized allocations `free` the memory backed by the provided
pointer and *always* return `NULL`:

```c
static void*
xalloc(void* ptr, size_t size)
{
    if (size == 0) return free(ptr), NULL;
    void* const mem = realloc(ptr, size);
    if (mem == NULL)
    {
        fputs("error: Out of memory\n", stderr);
        exit(EXIT_FAILURE);
    }
    return mem;
}
```

With this function we can replace all of the lines in `cdoc.c` that look like:

```c
mem = realloc(mem, some_size);
assert(mem != NULL);
```

with one line that looks like:

```c
mem = xalloc(mem, some_size);
```

The updated source code with `xalloc` can be found
[here](https://git.sr.ht/~ashn/cdoc/tree/98ef0659cd0c7b32e1e8f351d0358c63d8da4066).


## A Better Error Handling Function
Continuing with the theme of removing duplicate code, we have a bunch of places
where we handle an error by calling `fprintf` or `fputs` followed by the call
`exit(EXIT_FAILURE)`.
This pattern appears enough times that I think it is worth making into a
function.
We always print a string `"error: "` followed by an error message that may
contain `printf`-style formatting + variadic format arguments.
It appears that our use case calls for an `errorf` function that will take a
format string `fmt` and (optional) format arguments, write everything to
`stderr`, and then `exit`:

```c
static void
errorf(char const* fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    fputs("error: ", stderr);
    vfprintf(stderr, fmt, args);
    fputs("\n", stderr);
    va_end(args);

    // Any error should immediately terminate the program.
    exit(EXIT_FAILURE);
}
```

With `errorf` defined we can replace lines that look like:

```c
fputs("error: SOME ERROR\n", stderr);
exit(EXIT_FAILURE);
```

```c
fprintf(stderr, "error(line %d): SOME ERROR\n", LINENO);
exit(EXIT_FAILURE);
```

with the lines:

```c
errorf("SOME ERROR");
```

```c
errorf("[line %d] SOME ERROR", LINENO);
```

I didn't do much planning when putting together the code for this blog post, so
even though we just wrote `xalloc` we are now going to rewrite it - replacing
the `fputs` + `exit` with a call to `errorf`.

```c
static void*
xalloc(void* ptr, size_t size)
{
    if (size == 0) return free(ptr), NULL;
    if ((ptr = realloc(ptr, size)) == NULL) errorf("Out of memory");
    return ptr;
}
```

After doing similar replacements for the rest of the error handling code in
`cdoc.c` I want to make sure that the `errorf` function actually works as
expected.
Our `example.c` file  contains only valid `cdoc` documentation, so we currently
do not have a way to test an error condition.
At some point we should probably add unit/integration tests to the project, but
let's hold off on doing that for now and instead create a temporary `bad.c` file
containing an error:

```c
$ cat bad.c
//! @
//! ^^^ Missing doc comment tag.
struct bad {
    int _;
};
```

The start of the `parse_section` function was changed from:

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
```

to:

```c
static struct section
parse_section(void)
{
    struct section s = {0};
    char const* cp = doc_content_start(*linep);
    if (*cp++ != '@')
        errorf("[line %d] Doc-section must begin with @<TAG>", LINENO);
    if (is_hspace(*cp))
        errorf("[line %d] Empty doc-comment tag", LINENO);
```

Running `cdoc` on `bad.c` should trigger a call to `errorf` in `parse_section`,
write a string equivalent to
`"error: [line 1] Empty doc-comment tag\n"` to `stdout`, and then `exit`.
Running `cdoc` on `bad.c` we get:

```sh
$ make clean cdoc && ./cdoc bad.c
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
<h3>: </h3>
^^^ Missing doc comment tag.
<hr>
```

Wait what?
Okay it seems like we have a bug.
Looking again at `parse_section`, I think the `NUL` terminator is being reached,
causing the second `if`-statement, `if (is_hspace(*cp))`, to have a falsy
condition since `NUL` is not a horizontal whitespace character.

That second `if`-statement should probably be:

```c
if (is_hspace(*cp) || *cp == '\0')
    errorf("[line %d] Empty doc-comment tag", LINENO);
```

Putting that change in and running the program again we get:

```sh
$ make clean cdoc && ./cdoc bad.c
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
error: [line 1] Empty doc-comment tag
```

Okay that's more like what I was expecting.
It looks like our `errorf` function is working correctly and it seems that we
have fixed a bug.
Sweet!

The updated source code with `errorf` and the `parse_section` bug-fix can be
found
[here](https://git.sr.ht/~ashn/cdoc/commit/2e146e2c60b2008efe50f7a6f9827180064ff1e5).


## Wrapping Up
I think the addition of two utility functions and a bug-fix is enough for this
"nice little break" blog post.
There is certainly more that can be improved, but perhaps we will leave that for
another time.
In the next post we will improve the documentation output for certain C
constructs by adding their associated source code to the generated HTML.
I hope to see you then!

## Footnotes
[^1]:
I have noticed that the Linux-centric model of overcommit-by-default memory
allocation has lead many programmers to assume that `malloc` and friends will
never return `NULL`.
At some point I would like to write an article on how this assumption is both
wrong and dangerous, but I think that sort of rant is best left for another
time.

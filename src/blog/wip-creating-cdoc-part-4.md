Creating a C Documentation Generator - Part 4
=============================================

Welcome to part 4 of a series in which we design and implement `cdoc` - a
source-code documentation tool for the C programming language.
In [part 3](/blog/2020-03-14-creating-cdoc-part-3.html) we finished the minimum
viable product for `cdoc`.
That post took a long time to put together and was filled with a lot of code, so
I though that it would be nice to take a bit of a break to make some minor
improvements what code we already have.

## Replacing `realloc`
Right now there are a bunch of places where we dynamically allocate/reallocate
memory with `realloc` and then check the result of the function call with
`assert`.
It is possible that `realloc` may fail to allocate memory and return NULL, so
we should always have some form of NULL-check[\[1\]](#ft1), but `assert` is not
the best way to do so: `assert`s are disabled when `NDEBUG` is defined and if we
*always* want to `exit` (or `abort`) on allocation failure then the logic for
doing so should really be handled inside the allocation function itself.

We will add a function `xalloc` that will behave like `realloc`, but will either
return the amount of memory requested or `exit` on out-of-memory failure.
The [specification for realloc](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)
leaves the behavior and return value for zero-sized allocations as
implementation defined, so we be more specific in our definition by saying that
a zero-sized allocation will `free` the passed in pointer and *always* return
NULL:

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
buf = realloc(buf, len + 1);
assert(buf != NULL);
```

with one line that looks like:

```c
buf = xalloc(buf, len + 1);
```

The tree with the addition of `xalloc` and the updates switching `realloc` to
`xalloc` can be found
[here](https://git.sr.ht/~ashn/cdoc/tree/98ef0659cd0c7b32e1e8f351d0358c63d8da4066).


## A Better Error Handling Function
Just like we had a bunch of places where we called `realloc` and then
immediately `assert`ed that it wasn't NULL, we also have a bunch of places where
we handle an error by calling `fprintf` or `fputs` on `stderr` with some error
message followed by the call `exit(EXIT_FAILURE)`.
This pattern appears enough times that I think it is worth making into a
function.
We always print a string `"error: "` followed by some text (which we may want
to format with a format string + variadic arguments), so the name `errorf` seems
like it would be appropriate.

The `errorf` function will take a format string `fmt` and (optional) format
arguments and then write everything to `stderr` and `exit`:

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

The `xalloc` function uses the `fputs` + `exit` pattern, so even though we just
wrote it we are now going to rewrite it using `errorf`:

```c
static void*
xalloc(void* ptr, size_t size)
{
    if (size == 0) return free(ptr), NULL;
    if ((ptr = realloc(ptr, size)) == NULL) errorf("Out of memory");
    return ptr;
}
```

After rewriting the rest of the error handling sections of code I want to make
sure that the `errorf` function actually works as expected.
Since our example contains only valid `cdoc` documentation we don't have a way
to test the `errorf` function.
At some point we should probably add unit/integration tests to the project, but
let's hold off on doing that and test the `errorf` function by creating a
temporary `bad.c` file that will have an error in it:

```c
$ cat bad.c
//! @
//! ^^^ Missing doc comment tag.
struct bad {
    int _;
};
```

This should print text equivalent to the string
`"error: [line 1] Empty doc-comment tag\n"` and `exit`.
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
Okay it looks like we have a bug.
Looking again at rewritten parse `parse_section` function there are the lines:

```c
struct section s = {0};
char const* cp = doc_content_start(*linep);
if (*cp++ != '@')
    errorf("[line %d] Doc-section must begin with @<TAG>", LINENO);
if (is_hspace(*cp))
    errorf("[line %d] Empty doc-comment tag", LINENO);
```

I think the NUL terminator is being reached and that second `if`-statement is
returning `false` since NUL is not a horizontal whitespace character.
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
It looks like our `errorf` function is working correctly and it it seems that
we have fixed a bug.
Sweet!

The tree with the addition of `errorf` and updates to `xalloc` and other
functions can be found
[here](https://git.sr.ht/~ashn/cdoc/commit/2e146e2c60b2008efe50f7a6f9827180064ff1e5).


## Footnotes
<div id="ft1">\[1\]:
A Linux-centric model of memory allocation has lead many programmers to assume
that `malloc` and friends will never return NULL.
At some point I would like to write an article on why this is both wrong and
dangerous, and how it saddens me that many systems languages including C++ (see
[P0709 R4 secion 4.3](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0709r4.pdf))
and Rust (see
[std::alloc::GlobalAlloc](https://doc.rust-lang.org/std/alloc/trait.GlobalAlloc.html))
either have or are moving towards memory models that assume memory allocation
will not fail (\*old man yells at cloud\*).

Most of the time it is not worth it for applications (such as `cdoc`) to handle
out-of-memory failures, but the ability to make that choice should always be
available when using a systems language.
I have not had a chance to play around with the Zig language yet, but it seems
that Zig does a really good job at giving the programmer control over memory
allocation and out-of-memory error handling.
</div>

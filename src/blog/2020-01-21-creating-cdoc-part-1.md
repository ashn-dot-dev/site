Creating a C Documentation Generator - Part 1
=============================================

In this series we are going to walk through the process of designing and
implementing `cdoc` - a source-code documentation tool for the C programming
language.

## Why Write a Documentation Generator
Despite the widespread use of C there is a surprising lack of
[source-code documentation tooling](https://en.wikipedia.org/wiki/\
Comparison_of_documentation_generators#Language_support) available for the
language.
What tooling does exist often requires a non-trivial amount of configuration and
does not fit well into the Unix philosophy.
I have spent a fair bit of time searching for a simple documentation generator
and have yet to find one that I am satisfied with, so I'll just create my own.

## What Do I Want In a Documentation Generator
The features (or lack thereof) that I would like to see in documentation
generation software are:

00. Trivial to build and distribute as a single static binary
    + No runtime dependencies (`python`, `ruby`, `mono`, etc.)
    + Installing should by as simple as running `make` and `cp`ing the
      resulting binary to `/usr/local/bin`
00. Portable across operating systems and architectures
00. Zero setup required for projects using the tool
    + No configuration files
    + No behavior-changing option flags
00. Follows the Unix philosophy
    + Output simple HTML
    + Assume the documentation tool will be used in command pipelines

## Initial Setup
Every project starts somewhere and in our case `cdoc` will begin with a
boilerplate `main`:

```c
#include <stdlib.h>

int
main(int argc, char** argv)
{
    return EXIT_SUCCESS;
}
```

```sh
$ c99 -o cdoc cdoc.c && ./cdoc && echo $?
0
```

While we are setting up boilerplate we might as well add a `Makefile` and
`.clang-format` to the project directory.
Neither are strictly necessary, but I like having them even if our project
consists of only one `.c` file.
Adding a `Makefile` helps to standardize the build process for all
users[\[1\]](#ft1), and a `.clang-format` will help future contributors follow
consistent style guidelines with minimal effort.

Our `Makefile` contains three targets:

00. `cdoc` - Build the `cdoc` program from source.
00. `format` - Run `clang-format` over the project source file(s) and modify them
    in-place.
00. `clean` - Remove build artifacts from the project directory.

```Makefile
.POSIX:
.SUFFIXES:
.PHONY: format clean

CC = c99
CFLAGS = -O0 -g
OBJS = cdoc.o

cdoc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

format:
	clang-format -i *.c

clean:
	rm -f cdoc $(OBJS)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
```

The contents of `.clang-format` is kind of long and not particularly
important, so just know that it exists.
If you really want to look at its inner workings you can view it
[here](https://git.sr.ht/~ashn/cdoc/tree/895edeebd33551540276a042fda9cef0911bd5b2/.clang-format).

## Parsing Command Line Arguments
When starting a new project I like to imagine what typical application use will
look like from the end-user's perspective.

+ A user invoking `cdoc` from the command line may provide one or more paths to
  C language files as input.
+ A user invoking `cdoc` from within a shell script may provide zero or more
  paths to C language files as input.
+ A user invoking `cdoc` within a command pipeline may provide the contents of a
  C language file from `stdin`.
+ A C language file is not guaranteed to have the file extensions `.h` or `.c`.

```sh
$ cdoc foo.h foo.c main.c

$ cdoc inc/*.h src/*.c

$ for f in $(ls 'src/'); do
>    [ $(echo "${f}" | head -c 4 -) = 'test' ] && continue # Skip test files.
>    cdoc "${f}"
> done

$ cat main.c | cdoc

$ cdoc foo.inc foo.c
```

If we count on the shell to handle globbing then our `cdoc` implementation can
satisfy all of the above use-cases by processing a list of zero or more `FILE`s
with the option to take input from `stdin`.

We are going for a zero-configuration approach with `cdoc`, therefore our
program does not require handling of any option parameters.
However, it would be nice if our program accepted `--help` and `--version` as
arguments so that users can get some sense of the basic program usage and
release information.

First let us `#define` the macro `VERSION` as a version-string in `MAJOR.MINOR`
format.
For now `VERSION` will be `0.0`.
We will bump this up to `0.1` at the end of this series when we release the
initial version of `cdoc`.

```c
#define VERSION "0.0"
```

Then we will define a `version` function as well as a corresponding declaration
above `main`:

```c
/* above main */

static void
version(void);
```

```c
/* below main */

static void
version(void)
{
    puts(VERSION);
    exit(EXIT_SUCCESS);
}
```

This function will be invoked when `--version` is passed as a command line
argument.

Next we define a `usage` function with a corresponding declaration above `main`
in exactly the same way we defined `version`:

```c
/* above main */

static void
usage(void);
```

```c
/* below main */

static void
usage(void)
{
    // clang-format off
    puts(
        "Usage: cdoc [OPTION]... [--] [FILE]..."                "\n"
                                                                "\n"
        "With no FILE, or when FILE is -, read standard input." "\n"
                                                                "\n"
        "Options:"                                              "\n"
        "  --help      Display usage information and exit."     "\n"
        "  --version   Display version information and exit."   "\n"
    );
    // clang-format on
    exit(EXIT_SUCCESS);
}
```

This function will be invoked when `--help` is passed as a command line
argument.

The `clang-format off` and `clang-format on` comments are required to prevent
`clang-format` from trying to rearrange the concatenated string literal being
passed to `puts`.
Normally `clang-format` is useful for these kinds of transformations, but usage
text is one of the few places where arranging string literals in the funky way
shown above actually improves readability.
C99 does not support
[raw string literals](https://en.cppreference.com/w/cpp/language/string_literal),
so this is the next best option in my opinion.

Adding argument parsing logic for `--version` and `--help` is relatively
straightforward: we do not have any single-character command line switches, so
option parsing can be handled with `strcmp` alone:

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.0"

static void
usage(void);

static void
version(void);

int
main(int argc, char** argv)
{
    bool parse_options = true;
    for (int i = 1; i < argc; ++i)
    {
        char const* const arg = argv[i];
        if (parse_options && strcmp(arg, "--help") == 0) { usage(); }
        if (parse_options && strcmp(arg, "--version") == 0) { version(); }
        if (parse_options && strcmp(arg, "--") == 0)
        {
            parse_options = false;
            continue;
        }

        printf("TODO: Process '%s'\n", arg);
    }

    return EXIT_SUCCESS;
}

static void
usage(void)
{
    // clang-format off
    puts(
        "Usage: cdoc [OPTION]... [--] [FILE]..."                "\n"
                                                                "\n"
        "With no FILE, or when FILE is -, read standard input." "\n"
                                                                "\n"
        "Options:"                                              "\n"
        "  --help      Display usage information and exit."     "\n"
        "  --version   Display version information and exit."   "\n"
    );
    // clang-format on
    exit(EXIT_SUCCESS);
}

static void
version(void)
{
    puts(VERSION);
    exit(EXIT_SUCCESS);
}
```

We track whether `--` has been encountered via the `parse_options` variable
declared just before the main loop.
If at any point we parse `--` then we will treat following arguments as literal
file paths.
In practice it is highly unlikely that someone is actually going to pass in a
file with the path `--help`, `--version`, or `--`, but handling these edge cases
the right way now is better than handling a bug report later, and the extra work
required is so trivial that there is really no reason not to do it right the
first time.

Some not-so-rigorous testing from the command line is enough to leave me
satisfied with our parsing logic:

```sh
$ make clean cdoc
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
$ ./cdoc --help
Usage: cdoc [OPTION]... [--] [FILE]...

With no FILE, or when FILE is -, read standard input.

Options:
  --help      Display usage information and exit.
  --version   Display version information and exit.

$ ./cdoc --version
0.0
~/code/cdoc$ ./cdoc foo.c bar.h baz.c
TODO: Process 'foo.c'
TODO: Process 'bar.h'
TODO: Process 'baz.c'
$ ./cdoc -- --help --version
TODO: Process '--help'
TODO: Process '--version'
```

## Writing `cat` as a Documentation Generator
The last thing we will tackle before calling it a day is replace that `TODO` in
the main loop with some actual file handling.
Hashing out the syntax and semantics of our documentation language is best left
for another blog post, so for now we will replicate the functionality of `cat`
as a placeholder for logic to be added later.
Obviously printing the contents of the entire input file is not representative
of the end-product we are trying to produce, but it will act as a good stub for
future work.

Inside our main loop we will add the following lines of code:

```c
        fp = (strcmp(arg, "-") == 0 && !parse_options) ? stdin
                                                       : fopen(arg, "rb");
        if (fp == NULL)
        {
            perror(arg);
            exit(EXIT_FAILURE);
        }
        do_file();
        fclose(fp);
```

If `-` is encountered as an argument and `--` has not yet been parsed, then we
will use `stdin` for our `FILE*` as described by the usage text.
Otherwise we will attempt to open the current argument using `fopen` and bail if
we encounter an error.

The variable `fp` will be declared above `main` as:

```c
static FILE* fp = NULL;
```

and `do_file`, our workhorse function, will be defined as:

```c
// TODO: Change this function to implement file parsing and printing logic
// rather than just emulate `cat`.
static void
do_file(void)
{
    int c;
    while ((c = fgetc(fp)) != EOF) { fputc(c, stdout); }
}
```

with a corresponding declaration above `main`.

## Putting it all Together

To make sure that everything we wrote today works as expected, we will create a
`.c` file with some meaningless definitions and use it as input to our current
`cdoc` program.

Here is the test file, `example.c`:
```c
#include <stddef.h> // size_t
#include <stdint.h> // [u]intN_t

enum colors
{
    RED = 0,
    BLUE = 1,
    GREEN = 2
};

struct string
{
    char* data;
    size_t len;
};

union vec3
{
    struct rgb
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb;

    struct xyz
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    } xyz;
};

int
foo(char const* bar, size_t n);
```

and here is the output from a full build and test run of `cdoc` against that
file:

```sh
$ make clean cdoc
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
$ ./cdoc example.c
#include <stddef.h> // size_t
#include <stdint.h> // [u]intN_t

enum colors
{
    RED = 0,
    BLUE = 1,
    GREEN = 2
};

struct string
{
    char* data;
    size_t len;
};

union vec3
{
    struct rgb
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb;

    struct xyz
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    } xyz;
};

int
foo(char const* bar, size_t n);
```

Sweet!
We have put together what *technically* counts as a documentation generator.
In the next post of this series we will define the grammar of our documentation
language and begin transpiling that language into some useful HTML.
I hope to see you then!

The source code for this blog post can be found
[here](https://git.sr.ht/~ashn/cdoc/tree/895edeebd33551540276a042fda9cef0911bd5b2).

## Footnotes
<div id="ft1">\[1\]:
Although there are many different C/C++ build systems I choose to use `make` for
all of my C/C++ projects.
Most distributions of Unix-like operating systems have some version of `make`
installed by default whereas tools like `cmake`, `xmake`, `ninja`, or
whatever else are not guaranteed the be on the end-user's machine.
For a good tutorial on writing `Makefile`s I would recommend
["A Tutorial on Portable Makefiles"](https://nullprogram.com/blog/2017/08/20/)
from Chris Wellons' blog.
</div>

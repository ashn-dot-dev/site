Testing Read Error and NUL Byte Error Handling
==============================================

I am in the middle of writing the code for part three of my "Creating a C
Documentation Generator" series.
One of the functions that is getting added to the source code of that project is
`read_text_file` - which reads and returns the contents of a text file as a
heap-allocated cstring.

A simple version of this function could be written as:

```c
static char*
read_text_file(FILE* stream)
{
    char* buf = NULL;
    size_t len = 0;

    int c;
    while ((c = fgetc(stream)) != EOF)
    {
        if (c == '\0') goto error; // NUL byte error
        buf = realloc(buf, len + 1);
        buf[len++] = (char)c;
    }
    if (!feof(stream)) goto error; // Read error

    buf = realloc(buf, len + 1);
    buf[len++] = '\0';
    return buf;

error:
    free(buf);
    return NULL;
}
```

which returns a non-NULL pointer to the cstring on success or NULL on failure.
The actual definition of this function that I am putting into `cdoc` looks a bit
different than this, but the concept is the same: if a NUL byte is encountered
or a read failure occurs then indicate that the entire read has failed.

Before I add this code to the blog post I wanted to test this function to make
sure it actually works.
The two error conditions I care about checking are (1) if a NUL byte is
encountered and (2) if a read error occurred in `fgetc`.
This is blogware so I am choosing to ignore the cases in which `stream == NULL`
or `realloc` returns NULL.

Checking the NUL byte error handling was easy: the Linux file system
conveniently has `/dev/zero` which will always spit out NUL bytes when read.
Generating a read error was a bit trickier, but thankfully
[someone on StackExchange](https://unix.stackexchange.com/questions/77492/special-file-that-causes-i-o-error/77571#77571)
has already solved this problem: a process' zero page is never mapped, so
attempting to read from the start of your own process' memory will always
trigger an IO error:

```sh
$ head --bytes=1 /proc/self/mem
head: error reading '/proc/self/mem': Input/output error
```

I threw together a quick function to test reading an entire file and fed it the
"normal" text file `test.txt`, `/dev/zero`, and `/proc/self/mem` from a `main`
driver.

```c
// example.c
// Test various failure conditions while reading a text file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static char*
read_text_file(FILE* stream)
{
    char* buf = NULL;
    size_t len = 0;

    int c;
    while ((c = fgetc(stream)) != EOF)
    {
        if (c == '\0') goto error; // NUL byte error
        buf = realloc(buf, len + 1);
        buf[len++] = (char)c;
    }
    if (!feof(stream)) goto error; // Read error

    buf = realloc(buf, len + 1);
    buf[len++] = '\0';
    return buf;

error:
    free(buf);
    return NULL;
}

static void
test_read_text_file(char const* path)
{
    FILE* const fp = fopen(path, "rb");
    assert(fp != NULL);
    char* const text = read_text_file(fp);

    printf("%s: read %s\n", path, text != NULL ? "success" : "failure");
    fclose(fp);
    free(text);
}

int main(void)
{
    test_read_text_file("test.txt");
    test_read_text_file("/dev/zero"); // NUL byte(s)
    test_read_text_file("/proc/self/mem"); // IO failure

    return EXIT_SUCCESS;
}
```

```sh
$ clang -Wall -Wextra example.c && ./a.out
test.txt: read success
/dev/zero: read failure
/proc/self/mem: read failure
```

Overall I am pretty happy with these results. I feel confident that my
`read_text_file` function will work as expected when added to `cdoc`, and I
learned that nifty trick for causing a read failure using `/proc/self/mem`.
The source code for this post can be found
[here](/blog/wip-testing-read-text-file-failure/).

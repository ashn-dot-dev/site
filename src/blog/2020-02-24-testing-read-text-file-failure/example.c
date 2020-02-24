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

struct rescode
{
    int error;
};
int
reserror(struct rescode res)
{
    return res.error;
}
#define RESCODE_SUCCESS (struct rescode){.error = 0}

int
do_thing(char const* path);
struct rescode
do_thing_rescode(char const* path);


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
example(void)
{
    int err;

    err = do_thing("/etc/network/interfaces");
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", "/etc/network/interfaces", strerror(err));
    }

    err = do_thing("/file/that/does/not/exist");
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", "/file/that/does/not/exist", strerror(err));
    }

    err = do_thing("/proc/self/mem");
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", "/proc/self/mem", strerror(err));
    }
}

static void
example_rescode(void)
{
    struct rescode res;

    res = do_thing_rescode("/etc/network/interfaces");
    if (reserror(res)) {
        fprintf(stderr, "%s: %s\n", "/etc/network/interfaces", strerror(reserror(res)));
    }

    res = do_thing_rescode("/file/that/does/not/exist");
    if (reserror(res)) {
        fprintf(stderr, "%s: %s\n", "/file/that/does/not/exist", strerror(reserror(res)));
    }

    res = do_thing_rescode("/proc/self/mem");
    if (reserror(res)) {
        fprintf(stderr, "%s: %s\n", "/proc/self/mem", strerror(reserror(res)));
    }
}

int
main(void)
{
    puts("EXAMPLE WITHOUT RESCODE");
    example();

    fputs("\n\n", stdout);

    puts("EXAMPLE WITH RESCODE");
    example_rescode();

    return EXIT_SUCCESS;
}

int
do_thing(char const* path)
{
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        return errno;
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        fputc(c, stdout);
    }
    if (ferror(fp)) {
        fclose(fp);
        return EIO;
    }

    fflush(stdout);
    fclose(fp);
    return 0;
}

struct rescode
do_thing_rescode(char const* path)
{
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        return (struct rescode){errno};
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        fputc(c, stdout);
    }
    if (ferror(fp)) {
        fclose(fp);
        return (struct rescode){EIO};
    }

    fflush(stdout);
    fclose(fp);
    return RESCODE_SUCCESS;
}

struct rescode
fopen_resode(FILE** out_fp, char const* path, char const* mode)
{
    FILE* fp = fopen(path, mode);
    if (fp == NULL) {
        return (struct rescode){errno};
    }

    *out_fp = fp;
    return RESCODE_SUCCESS;
}

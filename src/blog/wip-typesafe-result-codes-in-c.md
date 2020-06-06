Typesafe Result Codes in C
==========================


How do we indicate that an operation may fail.
Java has [exceptions](https://docs.oracle.com/javase/8/docs/api/java/lang/Exception.html),
Rust has [`Result`](https://doc.rust-lang.org/std/result/index.html),
Go has [`error`](https://golang.org/pkg/builtin/#error),
and of course C has... well C has a bunch of different ways of indicating
errors.

Let's take a look at three functions:
[`open`](https://linux.die.net/man/3/open),
[`fopen`](https://linux.die.net/man/3/fopen),
and [`sqlite3_open`](https://www.sqlite.org/c3ref/open.html).

```c
int fd = open("foo.txt", O_RDONLY);
if (fd == -1) {
    perror(NULL);
    // handle the error...
}


FILE* fp = fopen("foo.txt", "rb");
if (!fp) {
    perror(NULL);
    // handle the error...
}


sqlite3* db = NULL;
int res = sqlite3_open("foo.db", &db);
if (res != SQLITE_OK) {
    fprintf(stderr, "%s\n", sqlite3_errmsg(db));
    // handle the error...
}
```

These functions perform similar operations yet use three different mechanisms
for indicating and handling errors.
The POSIX `open` function returns a non-negative file descriptor on success, or
`-1` if an error occurred.
The standard C `fopen` function returns a handle to a `FILE` object on success,
or `NULL` if an error occurred.
The function `sqlite3_open` returns an `int` corresponding to a SQLite
[result code](https://sqlite.org/rescode.html#constraint_rowid).

--------------------------------------------------------------------------------

## Unifying Error Handling with Result Codes

```c
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
```

Turns:
```c
int err = foo(bar, baz);
if (err) {
    // handle error...
}
```
into:
```c
struct rescode res = foo(bar, baz);
if (reserror(res)) {
    // handle error...
}
```

--------------------------------------------------------------------------------

## Designing a Rescode Function

```c
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
```
```c
char const* const path = "/path/that/does/not/exist";

FILE* fp = NULL;
struct rescode res = fopen_resode(&fp, path, "rb");
if (reserror(res)) {
    printf("Failed to open '%s' - %s\n", path, strerror(res.error));
    // handle error...
}
```

--------------------------------------------------------------------------------

## Performance

Put the following code into [compiler explorer](https://godbolt.org/):

```c
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

#include <errno.h>
#include <stdlib.h>
#define SOME_OPERATION (rand() % 2)

int
__attribute__((noinline))
do_thing(void)
{
    if (SOME_OPERATION) {
        return EBUSY;
    }
    return 0;
}

struct rescode
__attribute__((noinline))
do_thing_rescode(void)
{
    if (SOME_OPERATION) {
        return (struct rescode){EBUSY};
    }
    return RESCODE_SUCCESS;
}

int main(void)
{
    int err;
    struct rescode res;

    err = do_thing();
    if (err) {
        exit(EXIT_FAILURE);
    }

    res = do_thing_rescode();
    if (reserror(res)) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
```


x86-64 clang 10.0.0 with `-O0`:

```asm
reserror:                               # @reserror
        push    rbp
        mov     rbp, rsp
        mov     dword ptr [rbp - 8], edi
        mov     eax, dword ptr [rbp - 8]
        pop     rbp
        ret
do_thing:                               # @do_thing
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        call    rand
        cdq
        mov     ecx, 2
        idiv    ecx
        cmp     edx, 0
        je      .LBB1_2
        mov     dword ptr [rbp - 4], 16
        jmp     .LBB1_3
.LBB1_2:
        mov     dword ptr [rbp - 4], 0
.LBB1_3:
        mov     eax, dword ptr [rbp - 4]
        add     rsp, 16
        pop     rbp
        ret
do_thing_rescode:                       # @do_thing_rescode
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        call    rand
        cdq
        mov     ecx, 2
        idiv    ecx
        cmp     edx, 0
        je      .LBB2_2
        mov     dword ptr [rbp - 8], 16
        jmp     .LBB2_3
.LBB2_2:
        mov     dword ptr [rbp - 8], 0
.LBB2_3:
        mov     eax, dword ptr [rbp - 8]
        add     rsp, 16
        pop     rbp
        ret
main:                                   # @main
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        mov     dword ptr [rbp - 4], 0
        call    do_thing
        mov     dword ptr [rbp - 8], eax
        cmp     dword ptr [rbp - 8], 0
        je      .LBB3_2
        mov     edi, 1
        call    exit
.LBB3_2:
        call    do_thing_rescode
        mov     dword ptr [rbp - 24], eax
        mov     eax, dword ptr [rbp - 24]
        mov     dword ptr [rbp - 16], eax
        mov     edi, dword ptr [rbp - 16]
        call    reserror
        cmp     eax, 0
        je      .LBB3_4
        mov     edi, 1
        call    exit
.LBB3_4:
        xor     edi, edi
        call    exit
```


x86-64 clang 10.0.0 with `-O2` (`-Os` outputs the exact same asm):

```asm
reserror:                               # @reserror
        mov     eax, edi
        ret
do_thing:                               # @do_thing
        push    rax
        call    rand
        shl     eax, 4
        and     eax, 16
        pop     rcx
        ret
do_thing_rescode:                       # @do_thing_rescode
        push    rax
        call    rand
        shl     eax, 4
        and     eax, 16
        pop     rcx
        ret
main:                                   # @main
        push    rax
        call    do_thing
        test    eax, eax
        jne     .LBB3_3
        call    do_thing_rescode
        test    eax, eax
        jne     .LBB3_3
        xor     edi, edi
        call    exit
.LBB3_3:
        mov     edi, 1
        call    exit
```


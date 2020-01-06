WIP: The ARRAY_LENGTH macro in C
================================

**TODO: INTRO**

```c
#include <stdio.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

int main(void)
{
    int arr0[] = {0, 1, 2};
    int arr1[] = {0, 1, 2, 3, 4, 5};

    printf("arr0 length == %zu\n", ARRAY_LENGTH(arr0));
    printf("arr1 length == %zu\n", ARRAY_LENGTH(arr0));

    return 0;
}
```
```sh
$ c99 example0.c && ./a.out
arr0 length == 3
arr1 length == 6
```

Taking a look ISO/IEC 9899:1999 section 6.5.3.4 - "The `sizeof` Operatior", we
find some useful information.

ISO/IEC 9899:1999 sec. 6.5.3.4.2:

> The `sizeof` operator yields the size (in bytes) of its operand, which may
> be an expression or the parenthesized name of a type.
> The size is determined from the type ofthe operand.
> The result is an integer.
> If the type of the operand is a variable length arraytype, the operand is
> evaluated; otherwise, the operand is not evaluated and the result is aninteger
> constant.

ISO/IEC 9899:1999 sec. 6.5.3.4.3:

> When applied to an operand that has type `char`, `unsigned char`, or
> `signed char`, (or a qualified version thereof) the result is 1.
> When applied to an operand that has array type, the result is the total number
> of bytes in the array.
> When applied to an operandthat has structure or union type, the result is the
> total number of bytes in such an object, including internal and trailing
> padding.

ISO/IEC 9899:1999 sec. 6.5.3.4.4:

> The value of the result is implementation-defined, and its type (an unsigned
> integer type) is `size_t` , defined in `<stddef.h>` (and other headers).

ISO/IEC 9899:1999 sec. 6.5.3.4.6:

> Another use of the `sizeof` operator is to compute the number of elements in
> an array:
>
> `sizeof array / sizeof array[0]`

**TODO: EXPLAIN C99 SPEC MEANING**

**TODO: SHOW EXAMPLE WITH COMPILE TIME CONTANT VIA ENUM AND RUNTIME W/ VLA**

In most of my own projects I define two variations of this macro.
```c
#define ARRAY_COUNT(array_) (sizeof(array_)/sizeof((array_)[0]))
#define CSTR_COUNT(cstr_) (ARRAY_COUNT(cstr_)-1)
```
`ARRAY_COUNT` is my preferred flavor of `ARRAY_LENGTH` and `CSTR_COUNT` is what
I use for a compile-time `strlen`.

```c
#include <stdio.h>
#include <stddef.h>

#define ARRAY_COUNT(array_) (sizeof(array_)/sizeof((array_)[0]))
#define CSTR_COUNT(cstr_) (ARRAY_COUNT(cstr_)-1)

#define FOO "foo!"

int main(void)
{
    printf("FOO: \"%s\"\n", FOO);
    printf("ARRAY_COUNT(FOO) : %zu\n", ARRAY_COUNT(FOO));
    printf("CSTR_COUNT(FOO)  : %zu\n", CSTR_COUNT(FOO));

    return 0;
}
```
```sh
$ c99 example1.c && ./a.out
FOO: "foo!"
ARRAY_COUNT(FOO) : 5
CSTR_COUNT(FOO)  : 4
```

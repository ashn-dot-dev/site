The ARRAY_LENGTH Macro in C
===========================

The number of elements in a C array can be determined through the use of the
`ARRAY_LENGTH` macro:
```c
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
```
This macro is not a part of the C standard[\[1\]](#ft1), but it is common to
find individual codebases defining `ARRAY_LENGTH` or one of its variants
(`ARRAY_LEN`, `ARRAY_SIZE`, `ARRAY_COUNT`, `COUNTOF`, `NUM_ELEMS`, etc.)
internally.

```c
// FILE: example0.c
#include <stdio.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

int main(void)
{
    int arr0[] = {1, 3, 5};
    int arr1[] = {0, 1, 2, 3, 4, 5};

    printf("arr0 length == %zu\n", ARRAY_LENGTH(arr0));
    printf("arr1 length == %zu\n", ARRAY_LENGTH(arr1));

    return 0;
}
```
```sh
$ c99 example0.c && ./a.out
arr0 length == 3
arr1 length == 6
```
---

So how does this macro work?
Fortunately for us ISO/IEC 9899:1999 (the C99 standard) conveniently spells it
all out in section 6.5.3.4 - "The `sizeof` Operator".

ISO/IEC 9899:1999 sec. 6.5.3.4.2:

> The `sizeof` operator yields the size (in bytes) of its operand, which may
> be an expression or the parenthesized name of a type.
> The size is determined from the type of the operand.
> The result is an integer.
> If the type of the operand is a variable length array type, the operand is
> evaluated; otherwise, the operand is not evaluated and the result is an
> integer constant.

ISO/IEC 9899:1999 sec. 6.5.3.4.3:

> When applied to an operand that has type `char`, `unsigned char`, or
> `signed char`, (or a qualified version thereof) the result is 1.
> When applied to an operand that has array type, the result is the total number
> of bytes in the array.
> When applied to an operand that has structure or union type, the result is the
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

Say we have an array type `T` (for example `T` as type `int[3]`).
Section 6.5.3.4.3 tells us that `sizeof T` will yield the number of bytes in an
array with type `T`.
Expression `T[0]` has type "element of `T`", so `sizeof T[0]` will yield the
number of bytes in a single element of an array with type `T`.
The `sizeof` N elements divided by the `sizeof` a single element leaves N: the
number of elements (length) of `T`.
Section 6.5.3.4.4 tells us that expressions `sizeof T` and `sizeof T[0]` both
have type `size_t`, thus the  expression `sizeof T / sizeof T[0]` is also of
type `size_t`.

As a general purpose macro
```c
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
```
is preferred over
```c
#define ARRAY_LENGTH(x) sizeof x / sizeof x[0]
```
to avoid errors due to macro parameter substitution.
Both macros are conceptually equivalent.

---

According to section 6.5.3.4.2 `ARRAY_LENGTH` will yield an integer constant for
non-VLA types. Expressions using `ARRAY_LENGTH` may be used as compile-time
integer constant expressions as long as the array in question is not a VLA.
```c
// FILE: example1.c
#include <stdio.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

static int const primes[] = {2, 3, 5, 7, 11};
enum {PRIMES_LENGTH = ARRAY_LENGTH(primes)}; /* compile time constant */

int main(void)
{
    int primes_squared[PRIMES_LENGTH];
    for (size_t i = 0; i < PRIMES_LENGTH; ++i)
    {
        primes_squared[i] = primes[i] * primes[i];
    }

    puts("PRIME | SQUARED");
    puts("---------------");
    for (size_t i = 0; i < PRIMES_LENGTH; ++i)
    {
        printf("%5d | %3d\n", primes[i], primes_squared[i]);
    }

    return 0;
}
```
```sh
$ c99 example1.c && ./a.out
PRIME | SQUARED
---------------
    2 |   4
    3 |   9
    5 |  25
    7 |  49
   11 | 121
```
---

In most of my own projects I define two variations of this macro.
```c
#define ARRAY_COUNT(array_) (sizeof(array_)/sizeof((array_)[0]))
#define CSTR_COUNT(cstr_) (ARRAY_COUNT(cstr_)-1)
```
`ARRAY_COUNT` is my preferred flavor of `ARRAY_LENGTH` and `CSTR_COUNT` is what
I use for a compile-time `strlen`.

```c
// FILE: example2.c
#include <stdio.h>

#define ARRAY_COUNT(array_) (sizeof(array_)/sizeof((array_)[0]))
#define CSTR_COUNT(cstr_) (ARRAY_COUNT(cstr_)-1)

#define FOO "foo!"

int main(void)
{
    printf("FOO: \"%s\"\n", FOO);
    printf("ARRAY_COUNT(FOO) == %zu\n", ARRAY_COUNT(FOO));
    printf("CSTR_COUNT(FOO)  == %zu\n", CSTR_COUNT(FOO));

    return 0;
}
```
```sh
$ c99 example2.c && ./a.out
FOO: "foo!"
ARRAY_COUNT(FOO) == 5
CSTR_COUNT(FOO)  == 4
```

## Footnotes
<div id="ft1">\[1\]:
Sort of...
The expression `sizeof array / sizeof array[0]` actually *does* appear in the
standard as we will see shortly.
However the `#define` for `ARRAY_LENGTH` does not.
</div>

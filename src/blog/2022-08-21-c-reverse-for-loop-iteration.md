Reverse For-Loop Iteration in C
===============================

One can reverse-iterate from `N-1` down to `0` in C using a for-loop of the
form:

```c
for (size_t i = N; i--;) {
  // code...
}
```

Example:

```c
// FILE: example.c
#include <stdio.h>

int main(void)
{
    for (size_t i = 5; i--;) {
        printf("%zu\n", i);
    }

    fputc('\n', stdout);

    int const array[] = {1, 3, 5, 9};
    for (size_t i = sizeof(array) / sizeof(array[0]); i--;) {
        printf("array element %zu: %d\n", i, array[i]);
    }
}
```
```sh
$ c99 example.c && ./a.out
4
3
2
1
0

array element 3: 9
array element 2: 5
array element 1: 3
array element 0: 1
```

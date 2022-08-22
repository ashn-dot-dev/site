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

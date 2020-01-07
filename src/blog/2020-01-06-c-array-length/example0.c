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

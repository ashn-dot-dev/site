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

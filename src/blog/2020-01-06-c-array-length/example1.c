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

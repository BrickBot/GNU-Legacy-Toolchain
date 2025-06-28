/*
 * This example tests operations on elements of type struct.
 */
#include <stdio.h>


typedef struct {
    int    a;
    long long d;
} foo;

static void
p (int a, long long d)
{
    printf ("%d %lld\n", a, d);
}

int main()
{
    foo f[1] = {{1, 2}};

    p (f[0].a, f[0].d);

    return 10;
}

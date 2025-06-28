/*
 * This example tests the qsort function, and more
 * in general function pointers passed to library functions.
 */

#include <stdlib.h>
#include <stdio.h>

static int
cmp(const void *pa, const void *pb)
{
    int a = *(const int*)pa;
    int b = *(const int*)pb;

    return (a - b);
}

int main()
{
    int a[6] = {9, -3, 4, 0, 1, -2};
    int b[6];
    int i;

    for (i=0; i < 6; ++i)
        printf("a[%d] = %d\n", i, a[i]);

    printf("\n", i, a[i]);

    qsort (a, 6, sizeof (int), cmp);

    for (i=0; i < 6; ++i)
        printf("a[%d] = %d\n", i, a[i]);

    for (i=0; i < 5; ++i)
        if (a[i] > a[i+1])
            return 1;

    return 0;
}

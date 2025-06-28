/*
 * This example tests the qsort function, and more
 * in general function pointers passed to library functions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int
cmp(const void *pa, const void *pb)
{
    int a = *(const int*)pa;
    int b = *(const int*)pb;

    return (a - b);
}

static void
my_sort (void *a, size_t nelems, size_t elemsize, int (*comp) (const void *, const void *))
{
  int i, j;
#define REF(i) ((void *) (((char *) a) + i * elemsize))
  for (i = 0; i < nelems; ++i)
    {
      for (j = 0; j < nelems; ++j)
	{
	  if (comp (REF (i), REF (j)) < 0)
	    {
	      char *t = alloca (elemsize);
	      memcpy (t, REF (i), elemsize);
	      memcpy (REF (i), REF (j), elemsize);
	      memcpy (REF (j), t, elemsize);
	    }
	}
    }
}

int main()
{
    int a[6] = {9, -3, 4, 0, 1, -2};
    int b[6];
    int i;

    for (i=0; i < 6; ++i)
        printf("a[%d] = %d\n", i, a[i]);

    printf("\n", i, a[i]);

    my_sort (a, 6, sizeof (int), cmp);

    for (i=0; i < 6; ++i)
        printf("a[%d] = %d\n", i, a[i]);

    for (i=0; i < 5; ++i)
        if (a[i] > a[i+1])
            return 1;

    return 0;
}

/*
 * This example tests operations on elements of type struct.
 */
#include <stdio.h>


typedef struct {
    int    a;
    long long d;
} foo;

static void
p (foo *f)
{
    printf ("%d %lld\n", f->a, f->d);  
}

int main()
{
    int one = 1;
    int three = 3;
    foo f[3] = {{one, 2}, {three, 4}, {5, 6}};

    p(&f[0]);
    p(&f[1]);
    p(&f[2]);

    /*printf ("&one = %p\n&three = %p\n&f = %p\n", &one, &three, &f);*/
    printf ("sizeof (one) = %d\nzizeof (three) = %d\nsizeof (f) = %d\n", (int) sizeof (one), (int) sizeof (three), (int) sizeof (f));
    int i;
    for (i = 0; i < 3; ++i)
      {
	/*printf ("&f[i] = %p, &f[i].a = %p, &f[i].d = %p\n", &f[i], &f[i].a, &f[i].d);*/
	printf ("sizeof (f[i]) = %d, sizeof (f[i].a) = %d, sizeof (f[i].d) = %d\n", (int) sizeof (f[i]), (int) sizeof (f[i].a), (int) sizeof (f[i].d));
      }

#if 0
    f[0] = f[1];

    p(&f[0]);
    p(&f[1]);
    p(&f[2]);
#endif

    return 10;
}

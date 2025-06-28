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
    p(&f[0] + 1);
    p(&f[0] + 2);

    /*printf ("&one = %p\n&three = %p\n&f = %p\n", &one, &three, &f);*/
    printf ("sizeof (one) = %d\nsizeof (three) = %d\nsizeof (f) = %d\n", (int) sizeof (one), (int) sizeof (three), (int) sizeof (f));

#if 0
    f[0] = f[1];

    p(&f[0]);
    p(&f[1]);
    p(&f[2]);
#endif

    return 10;
}

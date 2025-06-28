/*
 * This example tests operations on elements of type struct.
 */
#include <stdio.h>


typedef struct {
    int    a;
    double d;
} foo;

typedef struct {
    short s;
    foo   f;
} foo2;

int main()
{
    int one = 1;
    int three = 3;
    foo  f1[3] = {{one, 2.0}, {three, 4.0}, {5, 6.0}};
    foo2 f2 = {13, {8, 9.1}};

    printf ("%d %d %f\n", f2.s, f2.f.a, f2.f.d);
    
    if (f2.s != 13)
      return 1;

    int i;
    for (i = 0; i < 3; ++i)
      {
	printf ("%d %f\n", f1[i].a, f1[i].d);
      }
    
    f1[2] = f1[1];

    for (i = 0; i < 3; ++i)
      {
	printf ("%d %f\n", f1[i].a, f1[i].d);
      }

    f2.f = f1[0];

    printf ("%d %d %f\n", f2.s, f2.f.a, f2.f.d);
    
    if (f1[2].a == 3 && f2.f.a == 1)
      return 0;
    else
      return 10;
}

/*
 * This example tests bit fields.
 */

#include <stdio.h>

struct foo {
    char z;
    int a:7;
    unsigned int v:2;
    short b:9;
    int c:3;
    int  :0;
    int d:6;
    char x;
    double y;
};

union un {
    unsigned int ll[8];
    struct foo f;
};

__attribute__((noinline))
void
p (int i)
{
  /* printf ("%10p %d\n", (void *) i, i); */
  printf ("%d\n", i);
}

__attribute__((noinline))
void
pu (unsigned int i)
{
  /* printf ("%10p %d\n", (void *) i, i); */
  printf ("%u\n", i);
}


void w (union un *u)
{
  int i;
  for (i = 0; i < 8; ++i)
    {
      pu (u->ll[i]);     
    }
}

void i (union un *u)
{
  int i;
  for (i = 0; i < 8; ++i)
    {
      u->ll[i] = -1;
    }
}

int main()
{
    union un u;
    
    i (&u);
    w (&u);

    u.f.b = 5;

    //w (&u);

    if (u.ll[0] != 4228644863U) 
      {
	p (u.ll[0]);
	return 44;
      }

    if (u.f.b != 5) 
      {
	p (u.f.b);
	return 45;
      }

    return 0;
}

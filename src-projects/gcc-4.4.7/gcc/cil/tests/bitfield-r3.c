/*
 * This example tests bit fields.
 */

extern int printf (const char *s, ...);

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
static void
p (int i)
{
  /* printf ("%10p %d\n", (void *) i, i); */
  printf ("%d\n", i);
}

__attribute__((noinline))
static void
pu (unsigned int i)
{
  /* printf ("%10p %d\n", (void *) i, i); */
  printf ("%u\n", i);
}

__attribute__((noinline))
static void w (union un *u)
{
  int i;
  for (i = 0; i < 8; ++i)
    {
      pu (u->ll[i]);     
    }
  printf ("\n", i);
}

__attribute__((noinline))
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
    
#if 0
    p ((int) &u);
    p ((int) &u.f);
    p ((int) &u.f.x);
    p ((int) &u.ll[0]);
    p ((int) &u.ll[1]);
#endif

    u.f.a = -45;
    if (u.f.a + 45 != 0)  return 1;

    return 0;
}

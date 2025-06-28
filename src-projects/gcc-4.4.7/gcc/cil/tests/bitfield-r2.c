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

int main()
{
    union un u;

    int a = 0xabc;
    p (a);
    p (a << 4);
    p (a >> 4);

    a = 1 - a;
    p (a);
    p (a << 4);
    p (a >> 4);


    u.ll[0] = 0xfedcba98;
    u.ll[1] = 0x76543210;
    u.ll[2] = 0xfedcba98;
    u.ll[3] = 0x76543210;
    u.ll[4] = 0xfedcba98;
    u.ll[5] = 0x76543210;
    u.ll[6] = 0xfedcba98;
    u.ll[7] = 0x76543210;

#if 0
    p ((int) &u);
    p ((int) &u.f);
    p ((int) &u.f.x);
    p ((int) &u.ll[0]);
    p ((int) &u.ll[1]);
#endif

    w (&u);
    p (u.f.a);

    u.f.a = -45;
    w (&u);
    p (u.f.a);

    if (u.f.a + 45 != 0)  return 1;

    u.f.v = 3;
    w (&u);
    if (u.f.v != 3)  return 2;
    if (u.f.a + 45 != 0)  return 3;

    u.f.b = 4;
    w (&u);
    if (u.f.b != 4) 
      {
	p (u.f.b);
	return 4;
      }
    if (u.f.v != 3)  return 5;
    if (u.f.a + 45 != 0)  return 6;

    u.f.c = -4;
    w (&u);
    if (u.f.c + 4 != 0)
      {
	p (u.f.c);
	return 7;
      }

    if (u.f.b != 4)  
      {
	p (u.f.b);
	return 8;
      }
    if (u.f.v != 3)  return 9;
    if (u.f.a + 45 != 0)  return 10;

    u.f.d = 7;
    w (&u);
    if (u.f.d != 7)  return 11;
    if (u.f.c + 4 != 0)  return 12;
    if (u.f.b != 4)  return 13;
    if (u.f.v != 3)  return 14;
    if (u.f.a + 45 != 0)  return 15;

    return 0;
}

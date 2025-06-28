/* { dg-options run } */
/* { dg-options "-O2" } */

extern void abort (void);

__attribute__((noinline)) int
foo (unsigned char *x)
{
  if (x[0] != 1 || x[1] != 0x15)
    abort ();
  return 0;
}

static inline void
bar (unsigned short x)
{
  unsigned char s[2] = { x >> 8, x & 0xff };
  foo (s);
}

int
main (void)
{
  bar (0x115);
  return 0;
}

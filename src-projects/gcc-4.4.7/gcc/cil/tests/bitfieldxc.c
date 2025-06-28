
int printf (const char *, ...);

void
w (unsigned *u)
{
  printf("%u\n", *u);
}

int
m (unsigned *u)
{
  int *p;
  p = (int *) (((unsigned) u) + 2);
  *((unsigned short *) p) = 64523;
  return *u == 4228644863U;
}

int 
main (int argc, char *argv[])
{
  unsigned u;
  u = 4294967295U;
  w (&u);
  int ret = m (&u);
  w (&u);
  return ret;
}

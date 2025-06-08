#include <stdio.h>

static void 
p (int n)
{
  printf ("%d\n", n);
}

u (unsigned int n)
{
  printf ("%u\n", n);
}

static void 
q (double n)
{
  printf ("%f\n", n);
}

static void
dp (int a, int b)
{
  p (a / b);
}

static void
up (unsigned int a, unsigned int b)
{
  u (a / b);
}

static void
urp (unsigned int a, unsigned int b)
{
  u (a % b);
}

static void
dq (double a, double b)
{
  q (a / b);
}

static void
rp (int a, int b)
{
  p (a % b);
}

int 
main (int argc, char *argv[])
{
  dp (14, 3);
  dp (14, -3);
  dp (-14, 3);
  dp (-14, -3);

  dq (14.2, 3.1);
  dq (14.2, -3.1);
  dq (-14.2, 3.1);
  dq (-14.2, -3.1);

  dq (-14, -3);
  dq (-14.1, -3);
  dq (-14, -3.1);
  dq (-14, -3.1);

  int nc = 2686;
  double tot = 2.1;
  q ((double)(nc/1024)/tot);

  rp (10, 6);
  rp (-10, 6);
  rp (-10, -6);
  rp (10, -6);

  up (5, 3);
  up (5, -3);
  up (-5, 3);
  up (-5, -3);

  urp (5, 3);
  urp (5, -3);
  urp (-5, 3);
  urp (-5, -3);

  return 7 / 8;
}

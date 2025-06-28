
typedef short int16;

extern int printf (const char *, ...);

static void 
rationalScale(float factor, int16* num, int16* den)
{
  *den = 256;
  *num = (int16) (factor * 256 + 0.5);
  if (*den > *num)
    {
      printf ("[rationalScale] downscale not allowed\n");
    }
}

int
main (int argc, char *argv[])
{
  double r[] = {0, 0.1, 0.2, 0.5, 0.9, 1, 1.1, 1.2, 1.3, 12, 12.34};
  int i;
  for (i = 0; i < sizeof (r) / sizeof (r[0]); ++i)
    {
      int16 d;
      int16 n;
      double f = r[i] + 1.1;
      rationalScale (f, &n, &d);
      printf ("%5d / %3d = %f\n", n, d, f);
    }
  return 0;
}

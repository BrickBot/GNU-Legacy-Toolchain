
#include <stdio.h>
#include <math.h>

int 
main (int argc, char *argv[])
{
  float b = 113.000000;
  float c = 89095.000000;

  float s = sin (c);
  printf ("sin (%f) = %f\n", c, s);  
  float r = b * s;
  printf ("%f * %f = %f\n", b, s, r);
  float u = b * sin (c);
  printf ("%f * sin (%f) = %f\n", b, c, u);

  return 0;
}

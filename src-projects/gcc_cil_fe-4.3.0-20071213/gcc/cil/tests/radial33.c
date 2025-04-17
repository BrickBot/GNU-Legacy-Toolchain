#include <stdio.h>
#include <stdlib.h>

#ifndef IMGSIZE
#define IMGSIZE 100
#endif

#define NUMPIXELS (IMGSIZE*IMGSIZE)

__attribute__((noinline))
void
radial (unsigned char *p)
{
  unsigned int a;
  a = *p ; 
  //a = ( a == 0x7f ) ? 0x100 : a;
  a = ( a == 0x80 ) ? 0x100 : a;
  *p = a;
}

static unsigned char Input[NUMPIXELS] ;

static void
init (void)
{
  int i;
  for (i = 0; i < NUMPIXELS; ++i)
    {
      Input[i] = i;
    }
}

static void
dump (void)
{
  int i;
  for (i = 0; i < NUMPIXELS; ++i)
    {
      int j = Input[i];
      printf ("%4d", j);
    }
  
}

int main()
{
  init ();
  radial(&Input[0]);
  dump ();
  return 0;
}

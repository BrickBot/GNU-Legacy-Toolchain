#include <stdio.h>
#include <stdlib.h>

struct layer_data
{
  unsigned char Rdbfr[2048];
  unsigned char *Rdptr;
};
static struct layer_data ldd;
static struct layer_data *ld;

void
Flush_Buffer (void)
{
  const unsigned char *x = ld->Rdbfr + 2044;
  printf ("ld->Rdptr - x: %d\n", ld->Rdptr - x);
  
  if (ld->Rdptr < ld->Rdbfr + 2044)
    {
      printf ("wrong\n");
    }
  else
    {
      printf ("right\n");
    }
}

int
main (int argc, char *arv[])
{
  ld = &ldd;
  ld->Rdptr = ld->Rdbfr + 2048;
  Flush_Buffer ();
  
  return 0;
}

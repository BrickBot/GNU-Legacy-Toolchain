#include <stdio.h>
#include <stdlib.h>

struct layer_data
{
  unsigned char Rdbfr[2048];
  unsigned char *Rdptr;
};
static struct layer_data ldd;
static struct layer_data *ld;

int
Flush_Buffer (void)
{
  if (ld->Rdptr < ld->Rdbfr + 2044)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int
main (int argc, char *arv[])
{
  ld = &ldd;
  ld->Rdptr = ld->Rdbfr + 2048;
  return Flush_Buffer ();
}

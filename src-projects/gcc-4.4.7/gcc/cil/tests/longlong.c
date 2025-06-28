#include <stdio.h>
#include <string.h>

char *
reverse (char *s)
{
  char *b = s;
  char *e = s + strlen (s) - 1;
  while (b < e)
    {
      char t = *b;
      *b = *e;
      *e = t;
      ++b;
      --e;
    }
  return s;
}

void
ll2b (long long i, char *p)
{
  unsigned long long b;
  for (b = i; b; b >>= 1)
    {
      int bit = b & 0x1;
      *p = bit ? '1' : '0';
      ++p;
    }
  *p = '\0';
}

void 
p (long long i)
{
  char s[2048];
  ll2b (i, s);
  reverse (s);
  printf ("%20s\n", s);
}

int
main (int argc, char *argv[])
{
  if (argc == 1) 
    {
      long long x = 5345;
      int i;
      for (i = 0; i < 40; ++i)
	{
	  p (x);
	  x = x * 3;
	}
      for (i = 0; i < 40; ++i)
	{
	  p (x);
	  if (!(i & 0x1))
	    {
	      x = x >> (i & 0x7);
	    }
	  else
	    {
	      x = x * i + x;
	    }
	}
    }
  else
    {
      int i;
      for (i = 1; i < argc; ++i)
	{
	  p (atoll (argv [i]));
	}
    }
  return 0;
}


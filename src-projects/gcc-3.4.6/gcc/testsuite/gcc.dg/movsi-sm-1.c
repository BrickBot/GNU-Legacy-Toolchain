/* { dg-do run } */
/* { dg-options "-O2" } */
/* { dg-options "-O2 -mtune=i386" { target { { i?86-*-* x86_64-*-* } && ilp32 } } } */

int ret = 1;
char buf[128];

void
__attribute__((noinline))
bug (int arg)
{
  char str[28];

  __builtin_memcpy (str, "Bugged!", 8);

  if (arg & 0200)
    {
      __builtin_memcpy (str, "This is what we should get!", 28);
      ret = 0;
    }

  if (arg & 0100)
    __builtin_memcpy (str, "Broken!", 8);

  __builtin_sprintf (buf, "%s\n", str);
}

int
main ()
{
  bug (0200);
  if (ret)
    return ret;
  return __builtin_strcmp (buf, "This is what we should get!\n") != 0;
}

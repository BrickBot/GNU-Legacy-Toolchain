
typedef struct 
{
  volatile int w;
} S;

static S s;

void
system_wait (void)
{
  int t;
loop:
  t = s.w;
  s.w = t - 1;
  if (s.w > 0) goto loop;
}

int
main (int argc, char *argv[])
{
  s.w = 2000;
  system_wait ();
  return 0;
}

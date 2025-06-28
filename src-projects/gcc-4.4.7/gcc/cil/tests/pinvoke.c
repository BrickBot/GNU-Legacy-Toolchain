
int testfunc (int a, int b) __attribute__((pinvoke("testlib.so")));

int 
main (int argc, char *argv[])
{
  return testfunc (5, 6);
}




extern int printf(const char*, ...);

int main (int argc, char** argv)
{
    int i;

    printf("%d arguments:\n", argc);
    for (i=1; i < argc; ++i)
      printf("    arg %d: %s\n", i, argv[i]);

    return 0;
}

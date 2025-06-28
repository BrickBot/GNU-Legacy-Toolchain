/*
 * This tests function attributes.
 */


int foo(int)  __attribute__((cil_name("foo")));


int foo(int max)
{
    int i, x;

    x = 0;
    for(i=0; i < max; ++i)
        x += i;
    return x;
}


int main()
{
    int x;

    x = foo(50);
    return 0;
}

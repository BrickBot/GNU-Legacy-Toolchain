/*
 * This example tests indirect calls.
 */

int square(int a)
{
    return a * a;
}

int main()
{
    int (*f)(int);

    f = &square;

    return f(5) - 25;
}

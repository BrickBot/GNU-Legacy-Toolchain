/*
 * This example tests dereferencing pointers to elements of type struct.
 */

struct foo {
    int    n;
    double d;
};

int main()
{
    struct foo a = {3, 5.0};
    struct foo* ap = &a;
    struct foo b;

    b = *ap;
    b.n = 1;
    *ap = b;

    if (a.n == 1)
        return 0;
    else
        return 1;
}

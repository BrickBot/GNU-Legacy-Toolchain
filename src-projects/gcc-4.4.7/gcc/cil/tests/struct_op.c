/*
 * This example tests operations on elements of type struct.
 */

typedef struct {
    int    a;
    double d;
} foo;

typedef struct {
    short s;
    foo   f;
} foo2;

int main()
{
    int one = 1;
    int three = 3;
    foo  f1[3] = {{one, 2.0}, {three, 4.0}, {5, 6.0}};
    foo2 f2 = {13, {8, 9.0}};

    f1[2] = f1[1];
    f2.f = f1[0];

    if (f1[2].a == 3 && f2.f.a == 1)
        return 0;
    else
        return 1;
}

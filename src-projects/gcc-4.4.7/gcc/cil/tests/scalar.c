/*
 * This example does simple computations on int, float and double.
 */

float fadd(float a, float b)
{
    return a + b;
}

double dsub(double a, double b)
{
    return a - b;
}


int main()
{
    int    i1, i2, i3;
    float  f1, f2, f3;
    double d1, d2, d3;
    double delta, eps;

    eps = 0.000001;

    i1 = 2;
    i2 = 3;
    i3 = i1 + i2;
    if (i3 != 5)
        return 1;

    f1 = 2.1f;
    f2 = 3.2f;
    f3 = fadd(f1, f2);
    delta = f3 - 5.3;
    if (delta > eps || -delta < -eps)
        return 2;

    d1 = 1.2;
    d2 = 2.3;
    d3 = dsub(d2, d1);
    delta = dsub(d3, 1.1);
    if (delta > eps || -delta < -eps)
        return 3;

    return 0;
}

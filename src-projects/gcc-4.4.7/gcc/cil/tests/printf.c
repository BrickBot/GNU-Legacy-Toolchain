/*
 * Test basic string and call to printf.
 */

#include <stdio.h>

int main(int argc)
{
    int    a, b, c, d, e, f, g;
    float  x = 2.71828;
    double y = 3.14159;

    a = 1;
    b = 2;
    c = 3;
    d = 4;
    e = 5;
    f = 6;
    g = 7;
    printf("Hello, with lots of arguments: %d %d %d %d %d %d %d.\n",
           a, b, c, d, e, f, g);
    printf("Some floats: %e %f\n", x, y);
    return 0;
}

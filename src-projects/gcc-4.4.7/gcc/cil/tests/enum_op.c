/*
 * This example tests operations on elements of type enum.
 */

typedef enum {
    minus_one = -1,
    zero,
    two = 2,
    three,
    eleven = 11
} foo;

int main()
{
    foo two_ = two;
    foo three_ = three;
    foo f[2] = { two, three };

    f[1] = f[0];

    return f[1] - two;
}

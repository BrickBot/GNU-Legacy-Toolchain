/*
 * This example emits a some structs and unions.  No real code at this point,
 * just the sizeof to make sure that they have the expected (x86) size.
 */

struct foo {
    short s;
};

struct foo2 {
    unsigned int a;
    char c;
    double d;
    struct foo f;
    struct foo2* f2;
};

union foo3 {
    int a;
    char b;
    struct foo2 f;
};

void foo_fnct(union foo3* s)
{
}

int main()
{
    int x, y, z;

    x = sizeof(struct foo);
    y = sizeof(struct foo2);
    z = sizeof(union foo3);

    return x + y + z - 50;  /* expect 0 */
}

/*
 * This example tests bit fields.
 */

struct foo {
    char z;
    int a:7;
    unsigned int v:2;
    short b:9;
    int c:3;
    int  :0;
    int d:6;
    char x;
    double y;
};

union un {
    unsigned int ll[8];
    struct foo f;
};

int main()
{
    union un u;

    u.ll[0] = 0xfedcba98;
    u.ll[1] = 0x76543210;
    u.ll[2] = 0xfedcba98;
    u.ll[3] = 0x76543210;
    u.ll[4] = 0xfedcba98;
    u.ll[5] = 0x76543210;
    u.ll[6] = 0xfedcba98;
    u.ll[7] = 0x76543210;

    u.f.a = -45;
    if (u.f.a + 45 != 0)  return 1;

    u.f.v = 3;
    if (u.f.v != 3)  return 1;
    if (u.f.a + 45 != 0)  return 1;

    u.f.b = 4;
    if (u.f.b != 4)  return 1;
    if (u.f.v != 3)  return 1;
    if (u.f.a + 45 != 0)  return 1;

    u.f.c = -4;
    if (u.f.c + 4 != 0)  return 1;
    if (u.f.b != 4)  return 1;
    if (u.f.v != 3)  return 1;
    if (u.f.a + 45 != 0)  return 1;

    u.f.d = 7;
    if (u.f.d != 7)  return 1;
    if (u.f.c + 4 != 0)  return 1;
    if (u.f.b != 4)  return 1;
    if (u.f.v != 3)  return 1;
    if (u.f.a + 45 != 0)  return 1;

    return 0;
}

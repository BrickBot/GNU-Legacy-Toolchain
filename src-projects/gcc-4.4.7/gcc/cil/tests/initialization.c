
#include <stdio.h>

short s[] = {0x1, 0x2, 0x3};
int   i[] = {0x1, 0x2, 0x3};

struct A {
    char a1;
    int a2[4];
    long long a3;
    short a4[2];
};

struct A a[] =
    {
        {  1, { 2, 3 }, 4, { 5, 6 } },
        { 11, { 12 }, 14, { 15 } },
        { 21, 22, 23,  24, 25, 26 }
    };




int
main()
{
    printf("%d, %d, %d\n", s[0], s[1], s[2]);
    printf("%d, %d, %d\n", i[0], i[1], i[2]);

    printf("%d, {%d, %d, %d, %d}, %lld, {%d, %d}\n",
           a[0].a1, a[0].a2[0], a[0].a2[1], a[0].a2[2], a[0].a2[3], a[0].a3, a[0].a4[0], a[0].a4[1]);

    printf("%d, {%d, %d, %d, %d}, %lld, {%d, %d}\n",
           a[1].a1, a[1].a2[0], a[1].a2[1], a[1].a2[2], a[1].a2[3], a[1].a3, a[1].a4[0], a[1].a4[1]);

    printf("%d, {%d, %d, %d, %d}, %lld, {%d, %d}\n",
           a[2].a1, a[2].a2[0], a[2].a2[1], a[2].a2[2], a[2].a2[3], a[2].a3, a[2].a4[0], a[2].a4[1]);

    if (a[1].a1==11 && a[1].a4[0]==15 && a[2].a1==21 && a[2].a2[0]==22 && a[2].a2[3]==25)
        return 0;
    else
        return 1;

}

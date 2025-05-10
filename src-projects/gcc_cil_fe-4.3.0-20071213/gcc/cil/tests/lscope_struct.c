/*
 * This example tests that types declared in local scopes are properly
 * emitted, in particular without name clashes.
 */

struct foo {
    char c[4];
};

int main()
{
    struct foo f1;

    struct foo {
        int   a;
        float b;
    };

    struct foo f2;

    {
        struct foo f3;

        struct foo {
            struct foo* ptr;
        };

        struct foo f4;
    }

    return 0;
}

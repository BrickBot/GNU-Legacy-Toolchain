typedef struct {
    int a;
} foo;

int main()
{
    foo f1[1] = {{1}};
    foo f2 = {8};

    f2 = f1[0];

    return 0;
}

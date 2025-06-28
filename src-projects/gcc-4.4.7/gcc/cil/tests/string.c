/*
 * This example tests the simple manipulation of a string.
 */

int foo(char* str)
{
    char a, b, c, d;
    a = str[0];
    b = str[1];
    c = str[2];
    d = str[3];
    return str[3] == 'l';
}


int main()
{
    char* string = "Hello.";
    if (foo(string))
        return 0;
    return 1;
}

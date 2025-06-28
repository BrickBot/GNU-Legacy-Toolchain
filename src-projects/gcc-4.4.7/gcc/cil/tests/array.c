/*
 * This example tests basic arrays.
 */

int f(int* a)
{
    int i, x;

    x = 0;
    for(i=0; i < 100; ++i)
        x += a[i];
    return x;
}

int array1[100] = { 0 };
int array2[100] = { 1 };
int array3[50]  = { 1 , 3 , 5 , 6 };
int array4[]    = { 1 , 3 , 5 , 6 };
int array5[4]   = { 1 , 3 , 5 , 6 };

int main()
{
    int array[100];
    int i, x;

    int main_array1[100] = { 0 };
    int main_array2[100] = { 1 };
    int main_array3[50]  = { 1 , 3 , 5 , 6 };
    int main_array4[]    = { 1 , 3 , 5 , 6 };
    int main_array5[4]   = { 1 , 3 , 5 , 6 };

    if (   array1[0] != 0
        || array2[0] != 1
        || array3[1] != 3
        || array4[2] != 5
        || array5[3] != 6)
        return 0;

    if (   main_array1[0] != 0
        || main_array2[0] != 1
        || main_array3[1] != 3
        || main_array4[2] != 5
        || main_array5[3] != 6)
        return 0;

    for(i=0; i < 100; ++i)
        array[i] = i;

    x = f(array);

    if (x == 4950)
        return 0;

    return 1;
}

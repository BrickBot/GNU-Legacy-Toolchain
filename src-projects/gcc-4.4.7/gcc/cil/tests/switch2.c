/*
 * This example tests a switch with a GNU extension.
 */

int main(int argc, char** argv)
{
    switch (argc) {
    case 0:
    case 1:
        return 0;
    case 2:
        return 7;
    case 3:
    case 4:
    case 5:
        return 1;

    case 12 ... 23:  /* GNU extension */
        return 23;

    default:
        return 13;
    }
}

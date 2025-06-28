/*
 * This example tests C switches.
 */

int main(int argc, char** argv)
{
    switch (argc + 1) {
    case -3:
    case -2:
        return -1;
    case 0:
        return -2;
    case 2:  /* expected case */
        return 0;
    case 3:
        return 7;
    case 4:
    case 6:
    case 7:
    case 8:
        return 1;
    default:
        return 13;
    }
}

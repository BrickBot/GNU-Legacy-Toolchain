/*
 * This example tests various enum types: named and anonymous, with
 * and without typedef. Also tries function calls.
 */

enum { null, eins, zwei };

enum named_enum { rien, un, deux, trois };

typedef enum { nulla, uno, due, tre, quattro } number_it_t;

typedef enum bzh_named_enum {
    unan = 1,
    daou,
    tri,
    pevar,
    pemp,
    dek = 10
} number_bzh_t;

typedef enum ls {
    minus_one = -1,
    zero,
    one,
    five = 5,
    eleven = 11,
    twelve
} list;


list foo (list l)
{
    return l;
}

enum named_enum bar1(enum named_enum b)
{
    return b;
}

number_it_t bar2(number_it_t b)
{
    return b;
}

number_bzh_t bar3(number_bzh_t b)
{
    return b;
}


int main()
{
    if (null != 0 || eins != 1 || zwei != 2)
        return 1;

    if (bar1(rien) != 0 || bar1(un) != 1 || bar1(deux) != 2 || bar1(trois) != 3)
        return 2;

    if (   bar2(nulla) != 0 || bar2(uno) != 1 || bar2(due) != 2
        || bar2(tre) != 3   || bar2(quattro) != 4)
        return 3;

    if (   bar3(unan) != 1  || bar3(daou) != 2 || bar3(tri) != 3
        || bar3(pevar) != 4 || bar3(pemp) != 5 || bar3(dek) != 10)
        return 4;

    return foo (five) - five;
}

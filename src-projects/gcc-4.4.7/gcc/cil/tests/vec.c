/*
 * Test vectorizer.
 * Examples taken from http://gcc.gnu.org/projects/tree-ssa/vectorization.html
 */

#define MAX 10
#define M 100
#define N 100

typedef int aint __attribute__ ((__aligned__(16)));


int a[256], b[256], c[256];
int a2[M][N];


foo1()
{
    int i;

    for (i=0; i < 256; i++) {
        a[i] = b[i] + c[i];
    }
}


foo2(int n, int x)
{
    int i;

    /* feature: support for unknown loop bound  */
    /* feature: support for loop invariants  */
    for (i=0; i < n; i++) {
        b[i] = x;
    }

    /* feature: general loop exit condition  */
    /* feature: support for bitwise operations  */
    while (n--) {
        a[i] = b[i] & c[i];
        i++;
    }
}


foo3(int n, aint * __restrict__ p, aint * __restrict q)
{
    /* feature: support for (aligned) pointer accesses.  */
    while (n--) {
        *p++ = *q++;
    }
}


foo4(int n, aint * __restrict__ p, aint * __restrict__ q)
{
    int i, j;

    /* feature: support for (aligned) pointer accesses  */
    /* feature: support for constants  */
    while (n--) {
        *p++ = *q++ + 5;
    }

    /* feature: support for read accesses with a compile time known
       misalignment  */
    for (i=0; i < n; i++) {
        a[i] = b[i+1] + c[i+3];
    }

    /* feature: support for if-conversion (only in autovect-branch) */
    for (i=0; i < n; i++) {
        j = a[i];
        b[i] = (j > MAX ? MAX : 0);
    }
}


struct a {
    int ca[N];
} s;

foo5()
{
    int i;

    for (i = 0; i < N; i++) {
        /* feature: support for alignable struct access  */
        s.ca[i] = 5;
    }
}


#if 0
foo7(int x)
{
    int i;

    /* feature: support for read accesses with an unknown misalignment */
    for (i=0; i < N; i++) {
        a[i] = b[i+x];
    }
}
#endif


foo8(int x)
{
    int i,j;

    /* feature: support for multidimensional arrays  */
    for (i=0; i < M; i++) {
        for (j=0; j < N; j++) {
            a2[i][j] = x;
        }
    }
}


unsigned int ub[N], uc[N];
foo9()
{
    int i;

    /* feature: support summation reduction.
       note: in case of floats use -funsafe-math-optimizations  */
    unsigned int udiff = 0;
    for (i = 0; i < N; i++) {
        udiff += (ub[i] - uc[i]);
    }
}


/* feature: support data-types of different sizes.
   Currently only a single vector-size per target is supported;
   it can accommodate n elements such that n = vector-size/element-size
   (e.g, 4 ints, 8 shorts, or 16 chars for a vector of size 16 bytes).
   A combination of data-types of different sizes in the same loop
   requires special handling, now present in autovect-branch.
   This also include support for type conversions.  */
foo10()
{
    short *sa, *sb, *sc;
    int   *ia, *ib, *ic;
    int    i;
    for (i = 0; i < N; i++) {
        ia[i] = ib[i] + ic[i];
        sa[i] = sb[i] + sc[i];
    }

    for (i = 0; i < N; i++) {
        ia[i] = (int) sb[i];
    }
}


int main(int argc, char* argv[])
{
    return 0;
}

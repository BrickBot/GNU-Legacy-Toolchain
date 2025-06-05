/* Infinity as a constant value.   This is used for HUGE_VAL.
 * Added by Cygnus Support.
 */

#include <float.h>
#include <math.h>

/* These should never actually be used any longer, as their use in math.h was
 * removed, but they are kept here in case a user was pointing to them.
 * FIXME:  deprecate these identifiers and then delete them.  */
 
/* Float version of infinity.  */
const union __fmath __infinityf[1] = { { FLT_MAX+FLT_MAX } };

/* Double version of infinity.  */
const union __dmath __infinity[1] = { { DBL_MAX+DBL_MAX } };

/* Long double version of infinity.  */
#if defined(_HAVE_LONG_DOUBLE)
#ifdef __IEEE_BIG_ENDIAN
 #if LDBL_MANT_DIG == 24
const union __ldmath __infinityld[1] = {{{0x7f800000, 0, 0, 0}}};
 #elif LDBL_MANT_DIG == 53
const union __ldmath __infinityld[1] = {{{0x7ff00000, 0, 0, 0}}};
 #else
const union __ldmath __infinityld[1] = {{{0x7fff0000, 0, 0, 0}}};
#endif /* LDBL_MANT_DIG size  */
#else /* __IEEE_LITTLE_ENDIAN  */
 #if LDBL_MANT_DIG == 24
const union __ldmath __infinityld[1] = {{{0x7f800000, 0, 0, 0}}};
 #elif LDBL_MANT_DIG == 53
const union __ldmath __infinityld[1] = {{{0, 0x7ff00000, 0, 0}}};
 #else
const union __ldmath __infinityld[1] = {{{0, 0x80000000, 0x00007fff, 0}}};
#endif /* LDBL_MANT_DIG size  */
#endif /* __IEEE_LITTLE_ENDIAN  */
#endif

/*

   Copyright (C) 2006-2007 Free Software Foundation, Inc.
   Contributed by STMicroelectronics

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!

   Author:
     Ricardo Fernandez Pascual <ricardof@um.es>
     Andrea C. Ornstein <andrea.ornstein@st.com>

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>
*/

/* These should not be here, they are from stdlib.dll */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

/* used in stdio.h */
FILE *
__io_ftable_get_entry (int i)
{
  switch (i) {
  case 0:
    return stdin;
  case 1:
    return stdout;
  case 2:
    return stderr;
  default:
    abort ();
  }
}

/* used in errno.h */
int *
__errno__get_ptr (void)
{
  return &errno;
}

/* used in float.h */
int
__flt_rounds (void)
{
  return 1;
}
float
__flt_nan (void)
{
    return __builtin_nanf ("");
}
float
__flt_infinity (void)
{
    return __builtin_inff();
}
float
__flt_epsilon (void)
{
    return FLT_EPSILON;
}
float
__flt_minval (void)
{
    return FLT_MIN;
}
float
__flt_maxval (void)
{
    return FLT_MAX;
}
float
__flt_hugeval (void)
{
    return __builtin_huge_valf();
}
double
__dbl_epsilon (void)
{
    return DBL_EPSILON;
}
double
__dbl_minval (void)
{
    return DBL_MIN;
}
double
__dbl_maxval (void)
{
    return DBL_MAX;
}
double
__dbl_hugeval (void)
{
    return __builtin_huge_val();
}

/* Redefined in stdlib.h to avoid problems */
int
Libstd_atexit (void (*function) (void))
{
  return atexit (function);
}

void
_libstd__init_System_Void (void)
{
}

void
_libstd__fini_System_Void (void)
{
}

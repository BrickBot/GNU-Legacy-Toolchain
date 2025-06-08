/* Specific flags and argument handling of the CIL front-end.

   Copyright (C) 2006-2007 Free Software Foundation, Inc.

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

#include "config.h"
#include "system.h"
#include "gcc.h"

#include "coretypes.h"
#include "tm.h"

void
lang_specific_driver (int *in_argc, const char *const **in_argv,
                      int *in_added_libraries)
{
  int i, j;
  bool donotlink = false;
  int argc;
  const char *const *argv;

  /* The new argument list will be contained in this.   */
  const char **arglist;

  /* The total number of arguments with the new stuff.  */
  int num_args = 1;

  argc = *in_argc;
  argv = *in_argv;

  for (i = 1; i < argc; ++i)
    {
      if (!strcmp (argv[i], "-fversion"))
        {
          printf ("GCC CIL (GCC %s)\n", version_string);
          exit (0);
        }

      if (!strcmp (argv[i], "-s") || !strcmp (argv[i], "-S"))
        {
            donotlink = true;
        }
    }

  num_args = argc + 3;
  arglist = XNEWVEC (const char *, num_args);

  i = 0;
  j = 0;

  while (i < argc)
    {
      arglist[j++] = argv[i++];
    }

  if (!donotlink) {
      arglist[j++] = "-lcilsupport";
      arglist[j++] = "-ldl";
  }

  arglist[j] = NULL;

  *in_argc = j;
  *in_argv = arglist;
  *in_added_libraries = 2;
}

/* Called before linking.  Returns 0 on success and -1 on failure.  */
int
lang_specific_pre_link (void)   /* Not used for CIL.  */
{
  return 0;
}

/* Number of extra output files that lang_specific_pre_link may generate.  */
int lang_specific_extra_outfiles = 0;   /* Not used for CIL.  */

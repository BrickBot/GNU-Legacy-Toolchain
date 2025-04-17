/* CIL Compiler almost main (cil1)
   Called by GCC's toplev.c

   Copyright (C) 2006 Free Software Foundation, Inc.

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

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>

*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "flags.h"
#include "toplev.h"
#include "version.h"

#include "ggc.h"
#include "tree.h"
#include "cgraph.h"
#include "diagnostic.h"
#include "opts.h"
#include "options.h"
#include "value-prof.h"

#include "cil.h"
#include "cil-tree.h"
#include "parser.h"

static void cil_read_vm_profile_info (const char *fname);

/* Language for usage for messages.  */
const char *const language_string = "CIL front end for GCC ";

/* Prepare to handle switches.  */
unsigned int
cil_init_options (unsigned int argc ATTRIBUTE_UNUSED,
                  const char **argv ATTRIBUTE_UNUSED)
{
  return CL_CIL;
}

/* Process a switch - called by opts.c.  */
int
cil_handle_option (size_t scode, const char *arg, int value)
{
  enum opt_code code = (enum opt_code) scode;
  switch (code)
    {
    case OPT_fcompile_only_reachable:
      flag_parse_only_reachable = true;
      break;
    case OPT_ferror_unsupported:
      flag_unsupported_method_behavior = UMB_ERROR;
      break;
    case OPT_fvm_profile_info:
      cil_read_vm_profile_info (arg);
      break;
    case OPT_fcbuiltin:
      flag_no_cbuiltin = !value;
      break;
    default:
      gcc_unreachable ();
    }
  return 1;
}

/* Language dependent parser setup.  */

bool
cil_init (void)
{
#ifndef USE_MAPPED_LOCATION
  input_filename = main_input_filename;
#else
  linemap_add (&line_table, LC_ENTER, false, main_input_filename, 1);
#endif

  /* This error will not happen from GCC as it will always create a
     fake input file.  */
  if (!input_filename || input_filename[0] == ' ' || !input_filename[0])
    {
      fprintf (stderr, "No input file specified, try --help for help\n");
      exit (1);
      return false;
    }

#ifdef USE_MAPPED_LOCATION
  linemap_add (&line_table, LC_RENAME, false, "<built-in>", 1);
  linemap_line_start (&line_table, 0, 1);
#endif

  /* Disable strict aliasing rules. CIL semantics require that
     pointers of any type can alias pointer of any other types (unlike
     C), or at least that's our current understanding. Hence, strict
     aliasing is useless (and produces worng code). */
  flag_strict_aliasing = 0;

  /* Init decls, etc.  */
  cil_init_decl_processing ();

  return true;
}

/* Language dependent wrapup.  */

void
cil_finish (void)
{
}

/* Parse a file.  Debug flag doesn't seem to work. */

void
cil_parse_file (int debug_flag ATTRIBUTE_UNUSED)
{
#ifdef USE_MAPPED_LOCATION
  source_location s;
  linemap_add (&line_table, LC_RENAME, false, main_input_filename, 1);
  s = linemap_line_start (&line_table, 1, 80);
  input_location = s;
#else
  input_line = 1;
#endif

  parser_parse_file (input_filename);

  cgraph_finalize_compilation_unit ();
#ifdef USE_MAPPED_LOCATION
  linemap_add (&line_table, LC_LEAVE, false, NULL, 0);
#endif
  cgraph_optimize ();
}

static void
cil_read_vm_profile_info (const char *fname)
{
  FILE *f = fopen (fname, "r");
  if (f)
    {
      location_profile_info_init ();
      char *line = NULL;
      size_t line_max_size = 0;
      char *current_assembly = NULL;
      int current_method = -1;
      while (!feof (f))
        {
          if (getline (&line, &line_max_size, f) > 0)
            {
#define PREFIX(p,s) (!strncmp (p, s, strlen (p)))
              if (PREFIX ("Assembly: ", line))
                {
                  const char *p = line + strlen ("Assembly: ");
                  size_t len = strchr (p, '\n') ? (size_t) (strchr (p, '\n') - p) : strlen (p);
                  free (current_assembly);
                  current_assembly = xstrndup (p, len);

                  /* HACK: add .exe if it is not there */
                  if (!strstr (current_assembly, ".exe"))
                    {
                      char *s = xmalloc (strlen (current_assembly) + strlen (".exe") + 1);
                      strcpy (s, current_assembly);
                      strcat (s, ".exe");
                      current_assembly = s;
                    }
                }
              else if (PREFIX ("Method: ", line))
                {
                  const char *p = line + strlen ("Method: ");
                  current_method = atoi (p);
                }
              else if (PREFIX ("exec_count ", line))
                {
                  gcc_assert (current_assembly);
                  gcc_assert (current_method != -1);
                  unsigned int bb;
                  int count;
                  if (sscanf (line, "exec_count %x = %d", &bb, &count) == 2)
                    {
                      unsigned int fake_line = (current_method << 16) + (bb & 0xffff);
                      location_profile_info_add_bb_exec_count (current_assembly, fake_line, count);
                    }
                  else
                    {
                      error ("Wrong file format in %<%s%>", fname);
                      break;
                    }
                }
              else
                {
                  gcc_assert (current_assembly);
                  gcc_assert (current_method != -1);
                  unsigned int src;
                  unsigned int dst;
                  int count;
                  if (sscanf (line, "%x -> %x = %d", &src, &dst, &count) == 3)
                    {
                      unsigned int fake_src = (current_method << 16) + (src & 0xffff);
                      unsigned int fake_dst = (current_method << 16) + (dst & 0xffff);
                      location_profile_info_add_edge_count (current_assembly, fake_src, fake_dst, count);
                    }
                  else
                    {
                      error ("Wrong file format in %<%s%>", fname);
                      break;
                    }
                }
#undef PREFIX
            }
          else
            {
              break;
            }
        }
      free (current_assembly);
      free (line);
      location_profile_info_dump (stderr);
    }
  else
    {
      error ("Could not open %<%s%>", fname);
    }
}

#include "debug.h" /* for debug_hooks */
#include "gtype-cil.h"

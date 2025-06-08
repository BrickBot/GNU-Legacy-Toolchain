/*
    CIL Compiler common definitions (cil.h)

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

    ---------------------------------------------------------------------------

   Author:
     Ricardo Fernandez Pascual <ricardof@um.es>
     Andrea C. Ornstein <andrea.ornstein@st.com>

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>
*/

#include "input.h"
#include "tree.h"

/* parse only the methods which are reachable from the entry point */
extern bool flag_parse_only_reachable;
/* what to do when trying to compile a method that uses some unsupported feature */
typedef enum
  {
    UMB_ERROR,
    UMB_WARNING
  }
UnsupportedMethodBehavior;
extern UnsupportedMethodBehavior flag_unsupported_method_behavior;

extern bool flag_no_cbuiltin;

unsigned int cil_init_options (unsigned int, const char **);
int cil_handle_option (size_t scode, const char *arg, int value);
bool cil_init (void);
void cil_init_decl_processing (void);
void cil_finish (void);
void cil_parse_file (int debug_flag);

tree cil_type_for_size (unsigned int bits, int unsigned_p);
tree cil_unsigned_type (tree type);
tree cil_unsigned_or_pointer_type (tree type);
tree cil_signed_type (tree type);

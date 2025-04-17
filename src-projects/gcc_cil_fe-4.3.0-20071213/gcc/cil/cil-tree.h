/*
    Language-dependent trees for CIL.

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

    ---------------------------------------------------------------------------

   Author:
     Ricardo Fernandez Pascual <ricardof@um.es>

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>

*/

#ifndef GCC_CIL_TREE_H
#define GCC_CIL_TREE_H

struct lang_identifier GTY(())
{
  struct tree_identifier common;
};

struct lang_decl GTY(())
{
    char dummy;
};

struct lang_type GTY(())
{
    char dummy;
};

struct language_function GTY(())
{
    char dummy;
};

enum cil_tree_node_structure_enum {
  TS_CIL_GENERIC
};

union lang_tree_node
  GTY((desc ("TREE_CODE (&%h.generic) == IDENTIFIER_NODE")))
{
  union tree_node GTY ((tag ("0"),
                        desc ("tree_node_structure (&%h)")))
    generic;
  struct lang_identifier GTY ((tag ("1"))) identifier;
};

#endif /* GCC_CIL_TREE_H */

/* Definitions for specs for CIL

   Copyright (C) 2006-2007 Free Software Foundation, Inc.

   The format of the specs file is documented in gcc.c

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */

/*
   This is the contribution to the `default_compilers' array in gcc.c

   This file must compile with 'traditional', so no ANSI string concatenations

*/

{".exe", "@cil", NULL, 0, 0},
{".EXE", "@cil", NULL, 0, 0},
{".dll", "@cil", NULL, 0, 0},
{".DLL", "@cil", NULL, 0, 0},
{"@cil",
   "%{!E:cil1 %i %(cc1_options) %{!fsyntax-only:%(invoke_as)}}"
   , NULL , 0, 0
},

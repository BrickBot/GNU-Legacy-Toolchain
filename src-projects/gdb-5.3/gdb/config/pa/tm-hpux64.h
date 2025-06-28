/* Definitions to target GDB to HPUX on HPPA.
   Copyright 1999, 2000, 2001 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef PA_TM_HPUX64_H
#define PA_TM_HPUX64_H

#include "pa/tm-pa.h"
#include "pa/tm-pa64.h"
#include "pa64solib.h"

/* For HP-UX on PA-RISC we have an implementation
   for the exception handling target op.  */
#define CHILD_ENABLE_EXCEPTION_CALLBACK
#define CHILD_GET_CURRENT_EXCEPTION_EVENT

#ifndef TYPE_PROCEDURE
#define TYPE_PROCEDURE 3
#endif

struct gdbarch;
void pa_hpux_initialize_tdep (struct gdbarch *, int);

#endif

/* CIL evaluation stack

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

#ifndef GCC_CIL_STACK_H

#include "system.h"
#include "coretypes.h"
#include "tree.h"

typedef enum
  {
    CIL_STYPE_INT32,    /* CIL_STACK_TYPE_INT32           */
    CIL_STYPE_INT64,    /* CIL_STACK_TYPE_INT64           */
    CIL_STYPE_NINT,     /* CIL_STACK_TYPE_NATIVE_INT      */
    CIL_STYPE_REAL32,   /* CIL_STACK_TYPE_REAL32          */
    CIL_STYPE_REAL64,   /* CIL_STACK_TYPE_REAL64          */
    CIL_STYPE_MP,       /* CIL_STACK_TYPE_MANAGED_POINTER */
    CIL_STYPE_OBJECT,   /* CIL_STACK_TYPE_OBJECT          */
    CIL_STYPE_VECTOR2D,
    CIL_STYPE_VECTOR4F,
    CIL_STYPE_VECTOR2L,
    CIL_STYPE_VECTOR2UL,
    CIL_STYPE_VECTOR4I,
    CIL_STYPE_VECTOR4UI,
    CIL_STYPE_VECTOR8S,
    CIL_STYPE_VECTOR8US,
    CIL_STYPE_VECTOR16SB,
    CIL_STYPE_VECTOR16B,
    CIL_STYPE_ERROR     /* CIL_STACK_TYPE_ERROR           */
  }
CilStackType;

#define NUM_OF_CIL_STYPE (CIL_STYPE_ERROR+1)

tree build_cil_stack_cst (CilStackType ct, int x);
tree cil_stack_get_tree_type_for_cil_stack_type (CilStackType ct);
CilStackType get_cil_stack_type_for_tree (tree t);
int cil_stack_type_to_nuints (CilStackType ct);

void cil_stack_init (void);
void cil_stack_push (tree item, CilStackType type);
void cil_stack_push_infer_type (tree item);
tree cil_stack_pop (CilStackType *type);
tree cil_stack_peek (unsigned int depth, CilStackType *type);
bool cil_stack_is_empty (void);

void cil_stack_debug_dump (void);

CilStackType get_cil_stack_type_for_mono_simd_class (const char * );

#endif /* GCC_CIL_STACK_H */

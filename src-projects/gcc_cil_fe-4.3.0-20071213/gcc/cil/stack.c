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

#include "config.h"
#include "stack.h"
#include "cil.h"
#include "ggc.h"

struct stack_node GTY(())
{
  struct stack_node *prev;
  tree element;
  /*CilStackType*/ unsigned int type; /* GCC garbage collector preprocessor does not let me use the correct type here */
};

static GTY(()) struct stack_node *stack_top = NULL;

void
cil_stack_init (void)
{
  stack_top = NULL;
}

static tree
cil_stack_normalize_type (tree t, CilStackType type)
{
  if (type == CIL_STYPE_OBJECT)
    {
      return t; /* TODO: no better idea for now */
    }
  else if (type == CIL_STYPE_F && SCALAR_FLOAT_TYPE_P (TREE_TYPE (t)))
    {
      return t; /* Avoid useless conversions between float and double */
    }
  else
    {
      return convert (cil_stack_get_tree_type_for_cil_stack_type (type), t);
    }
}

void
cil_stack_push (tree item, CilStackType type)
{
  struct stack_node *node = ggc_alloc (sizeof (struct stack_node));
  node->prev = stack_top;
  node->element = cil_stack_normalize_type (item, type);
  node->type = type;
  stack_top = node;
}

static bool
cil_stack_type_tree_equal (tree t1, tree t2)
{
  return t1 == t2; /* TODO: this seems to work, but it cannot be correct, is it? */
}

/* TODO: remove this function once cil_stack_push_infer_type is removed */
static CilStackType
cil_stack_get_cil_stack_type_for_tree_type (tree t)
{
  if (t == error_mark_node)
    {
      return CIL_STYPE_ERROR;
    }
  if (cil_stack_type_tree_equal (t, cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_INT32)))
    {
      return CIL_STYPE_INT32;
    }
  else if (cil_stack_type_tree_equal (t, cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_INT64)))
    {
      return CIL_STYPE_INT64;
    }
  else if (cil_stack_type_tree_equal (t, cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_NINT)))
    {
      return CIL_STYPE_NINT;
    }
  else if (cil_stack_type_tree_equal (t, cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_F)))
    {
      return CIL_STYPE_F;
    }
  else if (POINTER_TYPE_P (t))
    {
      /* TODO: This could be a CIL_STYPE_OBJECT or a CIL_STYPE_MP also, I assume here that it is an unmanaged pointer  */
      return CIL_STYPE_NINT;
    }
  else if (INTEGRAL_TYPE_P (t))
    {
      if (TYPE_PRECISION (t) <= TYPE_PRECISION (cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_INT32)))
        {
          return CIL_STYPE_INT32;
        }
      else if (TYPE_PRECISION (t) <= TYPE_PRECISION (cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_INT64)))
        {
          return CIL_STYPE_INT64;
        }
      else
        {
          gcc_unreachable ();
        }
    }
  else if (SCALAR_FLOAT_TYPE_P (t))
    {
      return CIL_STYPE_F;
    }
  else if (TREE_CODE (t) == RECORD_TYPE || TREE_CODE (t) == UNION_TYPE)
    {
      /* should be a valuetype */
      return CIL_STYPE_OBJECT;
    }
  else
    {
      gcc_unreachable ();
    }
}

/* TODO: remove this function, using the MonoType to decide the stack type at the caller site */
void
cil_stack_push_infer_type (tree item)
{
  CilStackType type = cil_stack_get_cil_stack_type_for_tree_type (TREE_TYPE (item));
  cil_stack_push (item, type);
}

tree
cil_stack_pop (CilStackType *type)
{
  gcc_assert (stack_top);
  tree ret = stack_top->element;
  if (type)
    {
      *type = stack_top->type;
    }
  stack_top = stack_top->prev;
  return ret;
}

tree
cil_stack_peek (unsigned int depth, CilStackType *type)
{
  gcc_assert (stack_top);
  struct stack_node *ret_node = stack_top;
  unsigned int i;
  for (i = 0; i < depth; ++i)
    {
      ret_node = ret_node->prev;
      gcc_assert (ret_node);
    }
  tree ret = ret_node->element;
  if (type)
    {
      *type = ret_node->type;
    }
  return ret;
}

bool
cil_stack_is_empty (void)
{
  return stack_top == NULL;
}

void
cil_stack_debug_dump (void)
{
  struct stack_node *iter;
  for (iter = stack_top; iter; iter = iter->prev)
    {
      debug_tree (iter->element);
    }
}

tree
cil_stack_get_tree_type_for_cil_stack_type (CilStackType ct)
{
  switch (ct)
    {
    case CIL_STYPE_INT32:
      return cil_type_for_size (32, false);
    case CIL_STYPE_INT64:
      return cil_type_for_size (64, false);
    case CIL_STYPE_NINT:
      return integer_type_node;
    case CIL_STYPE_F:
      return double_type_node;
    case CIL_STYPE_MP:
      return build_pointer_type (void_type_node);
    case CIL_STYPE_OBJECT:
      gcc_unreachable (); /* TODO */
    case CIL_STYPE_ERROR:
      return error_mark_node;
    default:
      gcc_unreachable ();
    }
}

#include "debug.h"
#include "gt-cil-stack.h"

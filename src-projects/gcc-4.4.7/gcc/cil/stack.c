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
     Kevin Williams <kevin.williams@inria.fr>
     Ricardo Fernandez Pascual <ricardof@um.es>

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>
*/

#include "config.h"
#include "stack.h"
#include "cil.h"
#include "ggc.h"

#include "diagnostic.h"

struct GTY(()) stack_node
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
  else if (type == CIL_STYPE_REAL32 && SCALAR_FLOAT_TYPE_P (TREE_TYPE (t)))
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
	/* printf("\t\t\tcil_stack_push '%s' '%s'\n",tree_code_name[TREE_CODE (TREE_TYPE (item))],tree_code_name[TREE_CODE (item)]); */
	/* debug_generic_stmt (item); printf("\n\n"); */

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
  else if (cil_stack_type_tree_equal (t, cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_REAL32)))
    {
      return CIL_STYPE_REAL32;
    }
  else if (cil_stack_type_tree_equal (t, cil_stack_get_tree_type_for_cil_stack_type (CIL_STYPE_REAL64)))
    {
      return CIL_STYPE_REAL64;
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
      return CIL_STYPE_REAL32;
    }
  else if (TREE_CODE (t) == RECORD_TYPE || TREE_CODE (t) == UNION_TYPE || TREE_CODE (t) == VECTOR_TYPE)
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
  
/*  printf("\t\t\tcil_stack_pop '%s' '%s'\n",tree_code_name[TREE_CODE (TREE_TYPE (ret))],tree_code_name[TREE_CODE (ret)]); */
/*  debug_generic_stmt (ret); printf("\n\n"); */
  
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
    case CIL_STYPE_REAL32:
      return float_type_node;
    case CIL_STYPE_REAL64:
      return double_type_node;
    case CIL_STYPE_MP:
      return build_pointer_type (void_type_node);
    case CIL_STYPE_VECTOR2D:
      return build_vector_type( double_type_node  ,2);
    case CIL_STYPE_VECTOR4F:
      return build_vector_type( float_type_node ,4);
    case CIL_STYPE_VECTOR2L:
      return build_vector_type( long_integer_type_node, 2);
    case CIL_STYPE_VECTOR2UL:
      return build_vector_type( long_unsigned_type_node, 2);
    case CIL_STYPE_VECTOR4I:
      return build_vector_type( integer_type_node, 4);
    case CIL_STYPE_VECTOR4UI:
      return build_vector_type( unsigned_type_node, 4);
    case CIL_STYPE_VECTOR8S:
      return build_vector_type( short_integer_type_node, 8);
    case CIL_STYPE_VECTOR8US:
      return build_vector_type( short_unsigned_type_node, 8);
    case CIL_STYPE_VECTOR16SB:
      return build_vector_type (intQI_type_node, 16);
    case CIL_STYPE_VECTOR16B:
      return build_vector_type( unsigned_intQI_type_node, 16);
    case CIL_STYPE_OBJECT:
      gcc_unreachable (); /* TODO */
    case CIL_STYPE_ERROR:
      return error_mark_node;
    default:
      gcc_unreachable ();
    }
}

tree
build_cil_stack_cst (CilStackType ct, int x)
{
  switch (ct)
  {
  case CIL_STYPE_VECTOR2D:
    gcc_unreachable ();
    return double_type_node;
  case CIL_STYPE_VECTOR4F:
    gcc_unreachable ();
    return float_type_node;
  case CIL_STYPE_VECTOR2L:
    gcc_unreachable ();
    return long_integer_type_node;
  case CIL_STYPE_VECTOR2UL:
    gcc_unreachable ();
    return long_unsigned_type_node;
  case CIL_STYPE_VECTOR4I:
    return build_int_cst(integer_type_node,x);
  case CIL_STYPE_VECTOR4UI:
    gcc_unreachable ();
    return unsigned_type_node;
  case CIL_STYPE_VECTOR8S:
    gcc_unreachable ();
    return build_int_cst(short_integer_type_node,x);
  case CIL_STYPE_VECTOR8US:
    gcc_unreachable ();
    return short_unsigned_type_node;
  case CIL_STYPE_VECTOR16SB:
    return build_int_cst(intQI_type_node,x);
  case CIL_STYPE_VECTOR16B:
    gcc_unreachable ();
    return unsigned_intQI_type_node;
  default:
    gcc_unreachable ();
    return NULL;
  }
}

int
cil_stack_type_to_nuints (CilStackType ct)
{
  switch (ct)
    {
    case CIL_STYPE_VECTOR2D:
      return 2;
    case CIL_STYPE_VECTOR4F:
      return 4;
    case CIL_STYPE_VECTOR2L:
      return 2;
    case CIL_STYPE_VECTOR2UL:
      return 2;
    case CIL_STYPE_VECTOR4I:
      return 4;
    case CIL_STYPE_VECTOR4UI:
      return 4;
    case CIL_STYPE_VECTOR8S:
      return 8;
    case CIL_STYPE_VECTOR8US:
      return 8;
    case CIL_STYPE_VECTOR16SB:
      return 16;
    case CIL_STYPE_VECTOR16B:
      return 16;
    case CIL_STYPE_INT32:
    case CIL_STYPE_INT64:
    case CIL_STYPE_NINT:
    case CIL_STYPE_REAL32:
    case CIL_STYPE_REAL64:
    case CIL_STYPE_MP:
    case CIL_STYPE_OBJECT:
      gcc_unreachable (); /* TODO */
    case CIL_STYPE_ERROR:
      return error_mark_node;
    default:
      gcc_unreachable ();
    }
}

CilStackType
get_cil_stack_type_for_mono_simd_class (const char * called_klass_name)
{
  if(strcmp(called_klass_name,"Vector2d")==0){
    return CIL_STYPE_VECTOR2D;
  }else if(strcmp(called_klass_name,"Vector4f")==0){
    return CIL_STYPE_VECTOR4F;
  }else if(strcmp(called_klass_name,"Vector2l")==0){
    return CIL_STYPE_VECTOR2L;
  }else if(strcmp(called_klass_name,"Vector2ul")==0){
    return CIL_STYPE_VECTOR2UL;
  }else if(strcmp(called_klass_name,"Vector4i")==0){
    return CIL_STYPE_VECTOR4I;
  }else if(strcmp(called_klass_name,"Vector4ui")==0){
    return CIL_STYPE_VECTOR4UI;
  }else if(strcmp(called_klass_name,"Vector8s")==0){
    return CIL_STYPE_VECTOR8S;
  }else if(strcmp(called_klass_name,"Vector8us")==0){
    return CIL_STYPE_VECTOR8US;
  }else if(strcmp(called_klass_name,"Vector16sb")==0){
    return CIL_STYPE_VECTOR16SB;
  }else if(strcmp(called_klass_name,"Vector16b")==0){
    return CIL_STYPE_VECTOR16B;
  }else{
    error("no stack type for vector %s",called_klass_name);
    gcc_unreachable ();
    return CIL_STYPE_ERROR;
  }
}

CilStackType
get_cil_stack_type_for_tree (tree t)
{
  if (TREE_CODE (t) == VECTOR_TYPE){
    switch (GET_MODE_NUNITS (TYPE_MODE (t)))
    {
    case 2:
      switch(TREE_CODE (TREE_TYPE(t)))
      {
        case REAL_TYPE:
          return CIL_STYPE_VECTOR2D;
        case INTEGER_TYPE:
          if(TYPE_UNSIGNED (TREE_TYPE(t)))
            return CIL_STYPE_VECTOR2UL;
          else
            return CIL_STYPE_VECTOR2L;
        default:
          gcc_unreachable ();
      }
      break;
    case 4:
      switch(TREE_CODE (TREE_TYPE(t)))
      {
        case REAL_TYPE:
          return CIL_STYPE_VECTOR4F;
        case INTEGER_TYPE:
          if(TYPE_UNSIGNED (TREE_TYPE(t)))
            return CIL_STYPE_VECTOR4UI;
          else
            return CIL_STYPE_VECTOR4I;
        default:
          gcc_unreachable ();
      }
      break;
    case 8:
      if(TYPE_UNSIGNED (TREE_TYPE(t)))
        return CIL_STYPE_VECTOR8US;
      else
        return CIL_STYPE_VECTOR8S;
      break;
    case 16:
      if(TYPE_UNSIGNED (TREE_TYPE(t)))
        return CIL_STYPE_VECTOR16B;
      else
        return CIL_STYPE_VECTOR16SB;
      break;
    }
  }
  error("no stack type for tree: ");
  print_generic_expr(stderr,t,0);
  fprintf(stderr,"\n");
  gcc_unreachable ();
  return CIL_STYPE_ERROR;
}

#include "debug.h"
/* #include "gt-cil-stack.h"   // Due to incomplete definitions, include instead a copy obtained from a build of BrickBot/gcc-cli-fe */
#include "gt-static-cil-stack.h"

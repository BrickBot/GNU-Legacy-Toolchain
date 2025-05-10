/*
    Language-dependent hooks for CIL.

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

   Authors:
     Ricardo Fernandez Pascual <ricardof@um.es>
     Roberto Costa
     Andrea C. Ornstein <andrea.ornstein@st.com>

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>

*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-dump.h"
#include "tree-iterator.h"
#include "tree-gimple.h"
#include "function.h"
#include "flags.h"
#include "output.h"
#include "ggc.h"
#include "toplev.h"
#include "varray.h"
#include "langhooks-def.h"
#include "langhooks.h"
#include "target.h"
#include "opts.h"
#include "cgraph.h"
#include "convert.h"

#include "cil.h"
#include "cil-builtins.h"
#include "cil-tree.h"
#include "bindings.h"

/* Tables of information about tree codes.  */

#define DEFTREECODE(SYM, NAME, TYPE, LENGTH) TYPE,
const enum tree_code_class tree_code_type[] = {
#include "tree.def"
};
#undef DEFTREECODE

#define DEFTREECODE(SYM, NAME, TYPE, LENGTH) LENGTH,
const unsigned char tree_code_length[] = {
#include "tree.def"
};
#undef DEFTREECODE

#define DEFTREECODE(SYM, NAME, TYPE, LEN) NAME,
const char *const tree_code_name[] = {
#include "tree.def"
};
#undef DEFTREECODE

/* Number of bits in int and char - accessed by front end. TODO: see if really necessary */
unsigned int tree_code_int_size = SIZEOF_INT * HOST_BITS_PER_CHAR;
unsigned int tree_code_char_size = HOST_BITS_PER_CHAR;

/* Routines Expected by gcc:  */

/* Create the predefined scalar types of C,
   and some nodes representing standard constants (0, 1, (void *) 0).
   Initialize the global binding level.
   Make definitions for built-in primitive functions.  */
void
cil_init_decl_processing (void)
{
  cil_bindings_init ();

  build_common_tree_nodes (flag_signed_char, false);

  /* set standard type names */

  /* Define `int' and `char' last so that they are not overwritten.  */
  cil_bindings_push_type_decl (NULL, intQI_type_node);
  cil_bindings_push_type_decl (NULL, intHI_type_node);
  cil_bindings_push_type_decl (NULL, intSI_type_node);
  cil_bindings_push_type_decl (NULL, intDI_type_node);
#if HOST_BITS_PER_WIDE_INT >= 64
  cil_bindings_push_type_decl (NULL, intTI_type_node);
#endif
  cil_bindings_push_type_decl (NULL, unsigned_intQI_type_node);
  cil_bindings_push_type_decl (NULL, unsigned_intHI_type_node);
  cil_bindings_push_type_decl (NULL, unsigned_intSI_type_node);
  cil_bindings_push_type_decl (NULL, unsigned_intDI_type_node);
#if HOST_BITS_PER_WIDE_INT >= 64
  cil_bindings_push_type_decl (NULL, unsigned_intTI_type_node);
#endif

  cil_bindings_push_type_decl ("int", integer_type_node);
  cil_bindings_push_type_decl ("char", char_type_node);
  cil_bindings_push_type_decl ("long int", long_integer_type_node);
  cil_bindings_push_type_decl ("unsigned int", unsigned_type_node);
  cil_bindings_push_type_decl ("long unsigned int", long_unsigned_type_node);
  cil_bindings_push_type_decl ("long long int", long_long_integer_type_node);
  cil_bindings_push_type_decl ("long long unsigned int", long_long_unsigned_type_node);
  cil_bindings_push_type_decl ("short int", short_integer_type_node);
  cil_bindings_push_type_decl ("short unsigned int", short_unsigned_type_node);
  cil_bindings_push_type_decl ("signed char", signed_char_type_node);
  cil_bindings_push_type_decl ("unsigned char", unsigned_char_type_node);
  size_type_node = make_unsigned_type (POINTER_SIZE);
  cil_bindings_push_type_decl ("size_t", size_type_node);
  set_sizetype (size_type_node);

  build_common_tree_nodes_2 (/* short_double= */ 0);

  cil_bindings_push_type_decl ("float", float_type_node);
  cil_bindings_push_type_decl ("double", double_type_node);
  cil_bindings_push_type_decl ("long double", long_double_type_node);
  cil_bindings_push_type_decl ("void", void_type_node);

  cil_build_builtins ();

  build_common_builtin_nodes ();
  (*targetm.init_builtins) ();

  pedantic_lvalues = pedantic;
}

/* Language hooks.  */

/* copied from c_mark_addressable, stripped some things */
static bool
cil_mark_addressable (tree exp)
{
  tree x = exp;

  while (1)
    {
      switch (TREE_CODE (x))
        {
        case COMPONENT_REF:
        case ADDR_EXPR:
        case ARRAY_REF:
        case REALPART_EXPR:
        case IMAGPART_EXPR:
          x = TREE_OPERAND (x, 0);
          break;
        case CONSTRUCTOR:
          TREE_ADDRESSABLE (x) = 1;
          return true;
        case VAR_DECL:
        case CONST_DECL:
        case PARM_DECL:
        case RESULT_DECL:
        case FUNCTION_DECL:
          TREE_ADDRESSABLE (x) = 1;
          /* drops out */
        default:
          return true;
        }
    }
}

tree
cil_type_for_size (unsigned int bits, int unsigned_p)
{
  if (bits == TYPE_PRECISION (integer_type_node))
    {
      return unsigned_p ? unsigned_type_node : integer_type_node;
    }
  else if (bits == TYPE_PRECISION (signed_char_type_node))
    {
      return unsigned_p ? unsigned_char_type_node : signed_char_type_node;
    }
  else if (bits == TYPE_PRECISION (short_integer_type_node))
    {
      return unsigned_p ? short_unsigned_type_node : short_integer_type_node;
    }
  else if (bits == TYPE_PRECISION (long_integer_type_node))
    {
      return unsigned_p ? long_unsigned_type_node : long_integer_type_node;
    }
  else if (bits == TYPE_PRECISION (long_long_integer_type_node))
    {
      return unsigned_p ? long_long_unsigned_type_node : long_long_integer_type_node;
    }
  if (bits <= TYPE_PRECISION (intQI_type_node))
    {
      return unsigned_p ? unsigned_intQI_type_node : intQI_type_node;
    }
  if (bits <= TYPE_PRECISION (intHI_type_node))
    {
      return unsigned_p ? unsigned_intHI_type_node : intHI_type_node;
    }
  if (bits <= TYPE_PRECISION (intSI_type_node))
    {
      return unsigned_p ? unsigned_intSI_type_node : intSI_type_node;
    }
  if (bits <= TYPE_PRECISION (intDI_type_node))
    {
      return unsigned_p ? unsigned_intDI_type_node : intDI_type_node;
    }
  if (bits <= TYPE_PRECISION (intTI_type_node))
    {
      return unsigned_p ? unsigned_intTI_type_node : intTI_type_node;
    }
  gcc_unreachable ();
  return NULL_TREE;
}


static GTY(()) tree registered_builtin_types;

static tree
cil_type_for_mode (enum machine_mode mode, int unsigned_p)
{
  tree t;

  if (mode == TYPE_MODE (integer_type_node))
    return unsigned_p ? unsigned_type_node : integer_type_node;

  if (mode == TYPE_MODE (signed_char_type_node))
    return unsigned_p ? unsigned_char_type_node : signed_char_type_node;

  if (mode == TYPE_MODE (short_integer_type_node))
    return unsigned_p ? short_unsigned_type_node : short_integer_type_node;

  if (mode == TYPE_MODE (long_integer_type_node))
    return unsigned_p ? long_unsigned_type_node : long_integer_type_node;

  if (mode == TYPE_MODE (long_long_integer_type_node))
    return unsigned_p ? long_long_unsigned_type_node : long_long_integer_type_node;

  if (mode == QImode)
    return unsigned_p ? unsigned_intQI_type_node : intQI_type_node;

  if (mode == HImode)
    return unsigned_p ? unsigned_intHI_type_node : intHI_type_node;

  if (mode == SImode)
    return unsigned_p ? unsigned_intSI_type_node : intSI_type_node;

  if (mode == DImode)
    return unsigned_p ? unsigned_intDI_type_node : intDI_type_node;

#if HOST_BITS_PER_WIDE_INT >= 64
  if (mode == TYPE_MODE (intTI_type_node))
    return unsigned_p ? unsigned_intTI_type_node : intTI_type_node;
#endif

  if (mode == TYPE_MODE (float_type_node))
    return float_type_node;

  if (mode == TYPE_MODE (double_type_node))
    return double_type_node;

  if (mode == TYPE_MODE (long_double_type_node))
    return long_double_type_node;

  if (mode == TYPE_MODE (void_type_node))
    return void_type_node;

  if (mode == TYPE_MODE (build_pointer_type (char_type_node)))
    return (unsigned_p
            ? make_unsigned_type (GET_MODE_PRECISION (mode))
            : make_signed_type (GET_MODE_PRECISION (mode)));

  if (mode == TYPE_MODE (build_pointer_type (integer_type_node)))
    return (unsigned_p
            ? make_unsigned_type (GET_MODE_PRECISION (mode))
            : make_signed_type (GET_MODE_PRECISION (mode)));

  for (t = registered_builtin_types; t; t = TREE_CHAIN (t))
    if (TYPE_MODE (TREE_VALUE (t)) == mode)
      return TREE_VALUE (t);

  /* TODO: to be completed when complex and vectors are added*/
  if (COMPLEX_MODE_P (mode) || VECTOR_MODE_P (mode))
      return NULL_TREE;

  gcc_unreachable ();
  return NULL_TREE;
}

tree
cil_unsigned_type (tree type)
{
  gcc_assert (TREE_CODE (type) == INTEGER_TYPE);
  unsigned int bits = TYPE_PRECISION (type);
  return cil_type_for_size (bits, true);
}

tree
cil_unsigned_or_pointer_type (tree type)
{
  if (TREE_CODE (type) == POINTER_TYPE)
    {
      return type;
    }
  else
    {
      return cil_unsigned_type (type);
    }
}

tree
cil_signed_type (tree type)
{
  gcc_assert (TREE_CODE (type) == INTEGER_TYPE);
  unsigned int bits = TYPE_PRECISION (type);
  return cil_type_for_size (bits, false);
}

static void
cil_register_builtin_type (tree type, const char* name)
{
  tree decl;

  decl = build_decl (TYPE_DECL, get_identifier (name), type);
  DECL_ARTIFICIAL (decl) = 1;
  if (!TYPE_NAME (type))
    TYPE_NAME (type) = decl;
  cil_bindings_push_decl (decl);

  registered_builtin_types = tree_cons (0, type, registered_builtin_types);
}

static int
cil_global_bindings_p (void)
{
  gcc_unreachable ();
}

static void
cil_insert_block (tree block ATTRIBUTE_UNUSED)
{
  gcc_unreachable ();
}

static tree
cil_pushdecl (tree t)
{
  gcc_unreachable ();
  return cil_bindings_push_decl (t);
}

/* Return the list of declarations in the current level. Note that this list
   is in reverse order (it has to be so for back-end compatibility).  */
static tree
cil_getdecls (void)
{
  return cil_bindings_get_decls ();
}

/* 
*/
static tree
cil_builtin_function (tree decl)
{
  cil_bindings_push_decl (decl);

  return decl;
}

static void
cil_expand_function (tree fndecl)
{
  /* We have nothing special to do while expanding functions for cil.  */
  tree_rest_of_compilation (fndecl);
}

/* based on c_common_get_alias_set, hence more complex than necessary */
static HOST_WIDE_INT
cil_get_alias_set (tree t)
{
  /* Permit type-punning when accessing a union, provided the access
     is directly through the union.  For example, this code does not
     permit taking the address of a union member and then storing
     through it.  Even the type-punning allowed here is a GCC
     extension, albeit a common and useful one; the C standard says
     that such accesses have implementation-defined behavior.  */
  tree u;
  for (u = t;
       TREE_CODE (u) == COMPONENT_REF || TREE_CODE (u) == ARRAY_REF;
       u = TREE_OPERAND (u, 0))
    if (TREE_CODE (u) == COMPONENT_REF
        && TREE_CODE (TREE_TYPE (TREE_OPERAND (u, 0))) == UNION_TYPE)
      return 0;

  /* That's all the expressions we handle specially.  */
  if (!TYPE_P (t))
    return -1;

  /* The C standard guarantees that any object may be accessed via an
     lvalue that has character type.  */
  /* I think this is the part that actually matters for CIL. Probably
     it would be even better to return always 0 for integer types. */
  if (t == char_type_node
      || t == signed_char_type_node
      || t == unsigned_char_type_node)
    return 0;

  /* If it has the may_alias attribute, it can alias anything.  */
  if (lookup_attribute ("may_alias", TYPE_ATTRIBUTES (t)))
    return 0;

  /* The C standard specifically allows aliasing between signed and
     unsigned variants of the same type.  We treat the signed
     variant as canonical.  */
  if (TREE_CODE (t) == INTEGER_TYPE && TYPE_UNSIGNED (t))
    {
      tree t1 = cil_signed_type (t);

      /* t1 == t can happen for boolean nodes which are always unsigned.  */
      if (t1 != t)
        return get_alias_set (t1);
    }
  else if (POINTER_TYPE_P (t))
    {
      tree t1;

      /* Unfortunately, there is no canonical form of a pointer type.
         In particular, if we have `typedef int I', then `int *', and
         `I *' are different types.  So, we have to pick a canonical
         representative.  We do this below.

         Technically, this approach is actually more conservative that
         it needs to be.  In particular, `const int *' and `int *'
         should be in different alias sets, according to the C and C++
         standard, since their types are not the same, and so,
         technically, an `int **' and `const int **' cannot point at
         the same thing.

         But, the standard is wrong.  In particular, this code is
         legal C++:

            int *ip;
            int **ipp = &ip;
            const int* const* cipp = ipp;

         And, it doesn't make sense for that to be legal unless you
         can dereference IPP and CIPP.  So, we ignore cv-qualifiers on
         the pointed-to types.  This issue has been reported to the
         C++ committee.  */
      t1 = build_type_no_quals (t);
      if (t1 != t)
        return get_alias_set (t1);
    }

  return -1;
}

#define LANG_HOOKS_MARK_ADDRESSABLE cil_mark_addressable
#define LANG_HOOKS_TYPE_FOR_MODE cil_type_for_mode
#define LANG_HOOKS_TYPE_FOR_SIZE cil_type_for_size
#undef LANG_HOOKS_GLOBAL_BINDINGS_P
#define LANG_HOOKS_GLOBAL_BINDINGS_P cil_global_bindings_p
#undef LANG_HOOKS_INSERT_BLOCK
#define LANG_HOOKS_INSERT_BLOCK cil_insert_block
#undef LANG_HOOKS_PUSHDECL
#define LANG_HOOKS_PUSHDECL cil_pushdecl
#undef LANG_HOOKS_GETDECLS
#define LANG_HOOKS_GETDECLS cil_getdecls
#undef LANG_HOOKS_BUILTIN_FUNCTION
#define LANG_HOOKS_BUILTIN_FUNCTION cil_builtin_function
#undef LANG_HOOKS_GET_ALIAS_SET
#define LANG_HOOKS_GET_ALIAS_SET cil_get_alias_set
#undef LANG_HOOKS_PARSE_FILE
#define LANG_HOOKS_PARSE_FILE cil_parse_file
#undef LANG_HOOKS_CALLGRAPH_EXPAND_FUNCTION
#define LANG_HOOKS_CALLGRAPH_EXPAND_FUNCTION cil_expand_function

#undef LANG_HOOKS_COMMON_ATTRIBUTE_TABLE
#define LANG_HOOKS_COMMON_ATTRIBUTE_TABLE cil_common_attribute_table
#undef LANG_HOOKS_FORMAT_ATTRIBUTE_TABLE
#define LANG_HOOKS_FORMAT_ATTRIBUTE_TABLE cil_common_format_attribute_table

#undef LANG_HOOKS_REGISTER_BUILTIN_TYPE
#define LANG_HOOKS_REGISTER_BUILTIN_TYPE cil_register_builtin_type

/* Hooks unique to CIL.  */

#undef LANG_HOOKS_NAME
#define LANG_HOOKS_NAME "GCC CIL"
#undef LANG_HOOKS_INIT
#define LANG_HOOKS_INIT cil_init
#undef LANG_HOOKS_FINISH
#define LANG_HOOKS_FINISH cil_finish
#undef LANG_HOOKS_INIT_OPTIONS
#define LANG_HOOKS_INIT_OPTIONS cil_init_options
#undef LANG_HOOKS_HANDLE_OPTION
#define LANG_HOOKS_HANDLE_OPTION cil_handle_option

const struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;

/* Language hooks that are not part of lang_hooks.  */


/* Create an expression whose value is that of EXPR,
   converted to type TYPE.  The TREE_TYPE of the value
   is always TYPE.  This function implements all reasonable
   conversions; callers should filter out those that are
   not permitted by the language being compiled.

   Adapted from c/convert.c
*/
tree
convert (tree type, tree expr)
{
/* Change of width--truncation and extension of integers or reals--
   is represented with NOP_EXPR.  Proper functioning of many things
   assumes that no other conversions can be NOP_EXPRs.

   Conversion between integer and pointer is represented with CONVERT_EXPR.
   Converting integer to real uses FLOAT_EXPR
   and real to integer uses FIX_TRUNC_EXPR.

   Here is a list of all the functions that assume that widening and
   narrowing is always done with a NOP_EXPR:
     In convert.c, convert_to_integer.
     In c-typeck.c, build_binary_op (boolean ops), and
        c_common_truthvalue_conversion.
     In expr.c: expand_expr, for operands of a MULT_EXPR.
     In fold-const.c: fold.
     In tree.c: get_narrower and get_unwidened.  */
  tree e = expr;
  enum tree_code code = TREE_CODE (type);
  const char *invalid_conv_diag;

  if (type == error_mark_node
      || expr == error_mark_node
      || TREE_TYPE (expr) == error_mark_node)
    return error_mark_node;

  if ((invalid_conv_diag = targetm.invalid_conversion (TREE_TYPE (expr), type)))
    {
      error (invalid_conv_diag);
      return error_mark_node;
    }

  if (type == TREE_TYPE (expr))
    return expr;

  if (TYPE_MAIN_VARIANT (type) == TYPE_MAIN_VARIANT (TREE_TYPE (expr)))
    return fold_build1 (NOP_EXPR, type, expr);
  if (TREE_CODE (TREE_TYPE (expr)) == ERROR_MARK)
    return error_mark_node;
  if (TREE_CODE (TREE_TYPE (expr)) == VOID_TYPE)
    {
      error ("void value not ignored as it ought to be");
      return error_mark_node;
    }
  if (code == VOID_TYPE)
    return build1 (CONVERT_EXPR, type, e);
  if (code == INTEGER_TYPE || code == ENUMERAL_TYPE)
    return fold (convert_to_integer (type, e));
  if (code == BOOLEAN_TYPE)
    return build1 (CONVERT_EXPR, type, e);
  if (code == POINTER_TYPE || code == REFERENCE_TYPE)
    return fold (convert_to_pointer (type, e));
  if (code == REAL_TYPE)
    return fold (convert_to_real (type, e));
  if (code == COMPLEX_TYPE)
    return fold (convert_to_complex (type, e));
  if (code == VECTOR_TYPE)
    return fold (convert_to_vector (type, e));
  if ((code == RECORD_TYPE || code == UNION_TYPE)
      && lang_hooks.types_compatible_p (type, TREE_TYPE (expr)))
      return e;

  error ("conversion not supported");
  return error_mark_node;
}

#include "debug.h" /* for debug_hooks, needed by gt-cil-treetree.h */
#include "gt-cil-cil-lang.h"

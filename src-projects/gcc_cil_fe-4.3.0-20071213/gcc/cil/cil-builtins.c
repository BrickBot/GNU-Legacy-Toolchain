/*
    Copyright (C) 1992-2007 Free Software Foundation, Inc.

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

bool flag_no_cbuiltin;

static tree handle_const_attribute (tree *, tree, tree, int, bool *);
static tree handle_pure_attribute (tree *, tree, tree, int, bool *);

const struct attribute_spec cil_common_attribute_table[] =
{
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler } */
  /* CIL Specific Attributes */
  { "OpenSystem.C.InitializerAttribute", 0, 0,  false, false, false, NULL },

  /* Attributes borrowed from C to be able to support builtins */
  { "packed",                 0, 0, false, false, false, NULL/*handle_packed_attribute*/ },
  { "nocommon",               0, 0,  true, false, false, NULL/*handle_nocommon_attribute*/ },
  { "common",                 0, 0,  true, false, false, NULL/*handle_common_attribute*/ },
  /* FIXME: logically, noreturn attributes should be listed as
     "false, true, true" and apply to function types.  But implementing this
     would require all the places in the compiler that use TREE_THIS_VOLATILE
     on a decl to identify non-returning functions to be located and fixed
     to check the function type instead.  */
  { "noreturn",               0, 0,  true, false, false, NULL/*handle_noreturn_attribute*/ },
  { "volatile",               0, 0,  true, false, false, NULL/*handle_noreturn_attribute*/ },
  { "noinline",               0, 0,  true, false, false, NULL/*handle_noinline_attribute*/ },
  { "always_inline",          0, 0,  true, false, false, NULL/*handle_always_inline_attribute*/ },
  { "flatten",                0, 0,  true, false, false, NULL/*handle_flatten_attribute*/ },
  { "used",                   0, 0,  true, false, false, NULL/*handle_used_attribute*/ },
  { "unused",                 0, 0, false, false, false, NULL/*handle_unused_attribute*/ },
  { "externally_visible",     0, 0,  true, false, false, NULL/*handle_externally_visible_attribute*/ },
  /* The same comments as for noreturn attributes apply to const ones.  */
  { "const",                  0, 0,  true, false, false, handle_const_attribute },
  { "transparent_union",      0, 0, false, false, false, NULL/*handle_transparent_union_attribute*/ },
  { "constructor",            0, 0,  true, false, false, NULL/*handle_constructor_attribute*/ },
  { "destructor",             0, 0,  true, false, false, NULL/*handle_destructor_attribute*/ },
  { "mode",                   1, 1, false,  true, false, NULL/*handle_mode_attribute*/ },
  { "section",                1, 1,  true, false, false, NULL/*handle_section_attribute*/ },
  { "aligned",                0, 1, false, false, false, NULL/*handle_aligned_attribute*/ },
  { "weak",                   0, 0,  true, false, false, NULL/*handle_weak_attribute*/ },
  { "alias",                  1, 1,  true, false, false, NULL/*handle_alias_attribute*/ },
  { "weakref",                0, 1,  true, false, false, NULL/*handle_weakref_attribute*/ },
  { "no_instrument_function", 0, 0,  true, false, false, NULL/*handle_no_instrument_function_attribute*/ },
  { "malloc",                 0, 0,  true, false, false, NULL/*handle_malloc_attribute*/ },
  { "returns_twice",          0, 0,  true, false, false, NULL/*handle_returns_twice_attribute*/ },
  { "no_stack_limit",         0, 0,  true, false, false, NULL/*handle_no_limit_stack_attribute*/ },
  { "pure",                   0, 0,  true, false, false, handle_pure_attribute },
  /* For internal use (marking of builtins) only.  The name contains space
     to prevent its usage in source code.  */
  { "no vops",                0, 0,  true, false, false, NULL/*handle_novops_attribute*/ },
  { "deprecated",             0, 0, false, false, false, NULL/*handle_deprecated_attribute*/ },
  { "vector_size",            1, 1, false,  true, false, NULL/*handle_vector_size_attribute*/ },
  { "visibility",             1, 1, false, false, false, NULL/*handle_visibility_attribute*/ },
  { "tls_model",              1, 1,  true, false, false, NULL/*handle_tls_model_attribute*/ },
  { "nonnull",                0, -1, false, true,  true, NULL/*handle_nonnull_attribute*/ },
  { "nothrow",                0, 0,  true, false, false, NULL/*handle_nothrow_attribute*/ },
  { "may_alias",              0, 0, false,  true, false, NULL },
  { "cleanup",                1, 1,  true, false, false, NULL/*handle_cleanup_attribute*/ },
  { "warn_unused_result",     0, 0, false,  true,  true, NULL/*handle_warn_unused_result_attribute*/ },
  { "sentinel",               0, 1, false,  true,  true, NULL/*handle_sentinel_attribute*/ },
  { NULL,                     0, 0, false, false, false, NULL }
};

const struct attribute_spec cil_common_format_attribute_table[] =
{
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler } */
  { "format",                 3, 3, false,  true,  true, NULL/*handle_format_attribute*/ },
  { "format_arg",             1, 1, false,  true,  true, NULL/*handle_format_arg_attribute*/ },
  { NULL,                     0, 0, false, false, false, NULL }
};

static tree
handle_const_attribute (tree *node, tree ARG_UNUSED (name), tree ARG_UNUSED (args),
                        int ARG_UNUSED (flags), bool *no_add_attrs)
{
  tree type = TREE_TYPE (*node);

  /* See FIXME comment on noreturn in c_common_attribute_table.  */
  if (TREE_CODE (*node) == FUNCTION_DECL)
    TREE_READONLY (*node) = 1;
  else if (TREE_CODE (type) == POINTER_TYPE
           && TREE_CODE (TREE_TYPE (type)) == FUNCTION_TYPE)
    TREE_TYPE (*node)
      = build_pointer_type
        (build_type_variant (TREE_TYPE (type), 1,
                             TREE_THIS_VOLATILE (TREE_TYPE (type))));
  else
    {
      warning (OPT_Wattributes, "const attribute ignored");
      *no_add_attrs = true;
    }

  return NULL_TREE;
}

static tree
handle_pure_attribute (tree *node, tree ARG_UNUSED (name), tree ARG_UNUSED (args),
                       int ARG_UNUSED (flags), bool *no_add_attrs)
{
  if (TREE_CODE (*node) == FUNCTION_DECL)
    DECL_IS_PURE (*node) = 1;
  /* ??? TODO: Support types.  */
  else
    {
      warning (OPT_Wattributes, "pure attribute ignored");
      *no_add_attrs = true;
    }

  return NULL_TREE;
}

int flag_isoc94 = 1;
int flag_isoc99 = 1;

static GTY(()) tree signed_size_type_node;
static GTY(()) tree wint_type_node;
static GTY(()) tree intmax_type_node;
static GTY(()) tree uintmax_type_node;
static GTY(()) tree string_type_node;
static GTY(()) tree const_string_type_node;
static GTY(()) tree va_list_ref_type_node;
static GTY(()) tree va_list_arg_type_node;

/* Handle C builtins.  */

static void init_builtin_attributes (void);

static void init_builtin_types (void);

static void init_builtins (void);


enum built_in_attribute
{
#define DEF_ATTR_NULL_TREE(ENUM, ...) ENUM,
#define DEF_ATTR_INT(ENUM, ...)       ENUM,
#define DEF_ATTR_IDENT(ENUM, ...)     ENUM,
#define DEF_ATTR_TREE_LIST(ENUM, ...) ENUM,
#  include "builtin-attrs.def"
#undef DEF_ATTR_NULL_TREE
#undef DEF_ATTR_INT
#undef DEF_ATTR_IDENT
#undef DEF_ATTR_TREE_LIST
  ATTR_LAST
};

static GTY(()) tree built_in_attributes[(int) ATTR_LAST];

enum c_builtin_type
{
#define DEF_PRIMITIVE_TYPE(NAME, VALUE)    NAME,
#define DEF_FUNCTION_TYPE_0(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_1(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_2(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_3(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_4(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_5(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_6(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_7(NAME, ...)     NAME,
#define DEF_FUNCTION_TYPE_VAR_0(NAME, ...) NAME,
#define DEF_FUNCTION_TYPE_VAR_1(NAME, ...) NAME,
#define DEF_FUNCTION_TYPE_VAR_2(NAME, ...) NAME,
#define DEF_FUNCTION_TYPE_VAR_3(NAME, ...) NAME,
#define DEF_FUNCTION_TYPE_VAR_4(NAME, ...) NAME,
#define DEF_FUNCTION_TYPE_VAR_5(NAME, ...) NAME,
#define DEF_POINTER_TYPE(NAME, TYPE)       NAME,
#  include "builtin-types.def"
#undef DEF_PRIMITIVE_TYPE
#undef DEF_FUNCTION_TYPE_0
#undef DEF_FUNCTION_TYPE_1
#undef DEF_FUNCTION_TYPE_2
#undef DEF_FUNCTION_TYPE_3
#undef DEF_FUNCTION_TYPE_4
#undef DEF_FUNCTION_TYPE_5
#undef DEF_FUNCTION_TYPE_6
#undef DEF_FUNCTION_TYPE_7
#undef DEF_FUNCTION_TYPE_VAR_0
#undef DEF_FUNCTION_TYPE_VAR_1
#undef DEF_FUNCTION_TYPE_VAR_2
#undef DEF_FUNCTION_TYPE_VAR_3
#undef DEF_FUNCTION_TYPE_VAR_4
#undef DEF_FUNCTION_TYPE_VAR_5
#undef DEF_POINTER_TYPE
  BT_LAST
};

typedef enum c_builtin_type builtin_type;

/* A temporary array for c_common_nodes_and_builtins.  Used in communication with def_fn_type.  */
static GTY(()) tree builtin_types[(int) BT_LAST + 1];



static void
init_builtin_attributes (void)
{
#define DEF_ATTR_NULL_TREE(ENUM)     built_in_attributes[(int) ENUM] = NULL_TREE;
#define DEF_ATTR_INT(ENUM, VALUE)    built_in_attributes[(int) ENUM] = build_int_cst (NULL_TREE, VALUE);
#define DEF_ATTR_IDENT(ENUM, STRING) built_in_attributes[(int) ENUM] = get_identifier (STRING);
#define DEF_ATTR_TREE_LIST(ENUM, PURPOSE, VALUE, CHAIN) \
  built_in_attributes[(int) ENUM]                       \
    = tree_cons (built_in_attributes[(int) PURPOSE],    \
                 built_in_attributes[(int) VALUE],      \
                 built_in_attributes[(int) CHAIN]);
#  include "builtin-attrs.def"
#undef DEF_ATTR_NULL_TREE
#undef DEF_ATTR_INT
#undef DEF_ATTR_IDENT
#undef DEF_ATTR_TREE_LIST
}

static tree
builtin_type_for_size (int size, bool unsignedp)
{
  tree type = lang_hooks.types.type_for_size (size, unsignedp);
  return type ? type : error_mark_node;
}

static void
def_fn_type (builtin_type def, builtin_type ret, bool var, int n, ...)
{
  tree args = NULL;
  tree t;
  va_list va;
  int i;

  va_start (va, n);
  for (i = 0; i < n; ++i)
    {
      builtin_type a = va_arg (va, builtin_type);
      t = builtin_types[a];
      if (t == error_mark_node)
        goto egress;
      args = tree_cons (NULL_TREE, t, args);
    }
  va_end (va);

  args = nreverse (args);
  if (!var)
    args = chainon (args, void_list_node);

  t = builtin_types[ret];
  if (t == error_mark_node)
    goto egress;
  t = build_function_type (t, args);

 egress:
  builtin_types[def] = t;
}

static void
init_builtin_types (void)
{
#define DEF_PRIMITIVE_TYPE(ENUM, VALUE) builtin_types[ENUM] = VALUE;
#define DEF_FUNCTION_TYPE_0(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 0);
#define DEF_FUNCTION_TYPE_1(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 1, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_2(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 2, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_3(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 3, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_4(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 4, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_5(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 5, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_6(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 6, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_7(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 0, 7, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_VAR_0(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 1, 0);
#define DEF_FUNCTION_TYPE_VAR_1(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 1, 1, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_VAR_2(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 1, 2, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_VAR_3(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 1, 3, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_VAR_4(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 1, 4, __VA_ARGS__);
#define DEF_FUNCTION_TYPE_VAR_5(ENUM, RETURN, ...) def_fn_type (ENUM, RETURN, 1, 5, __VA_ARGS__);
#define DEF_POINTER_TYPE(ENUM, TYPE) \
    builtin_types[(int) ENUM] = build_pointer_type (builtin_types[(int) TYPE]);

#  include "builtin-types.def"

#undef DEF_PRIMITIVE_TYPE
#undef DEF_FUNCTION_TYPE_1
#undef DEF_FUNCTION_TYPE_2
#undef DEF_FUNCTION_TYPE_3
#undef DEF_FUNCTION_TYPE_4
#undef DEF_FUNCTION_TYPE_5
#undef DEF_FUNCTION_TYPE_6
#undef DEF_FUNCTION_TYPE_VAR_0
#undef DEF_FUNCTION_TYPE_VAR_1
#undef DEF_FUNCTION_TYPE_VAR_2
#undef DEF_FUNCTION_TYPE_VAR_3
#undef DEF_FUNCTION_TYPE_VAR_4
#undef DEF_FUNCTION_TYPE_VAR_5
#undef DEF_POINTER_TYPE
  builtin_types[(int) BT_LAST] = NULL_TREE;
}

static void
def_builtin_1 (enum built_in_function fncode,
               const char *name,
               enum built_in_class fnclass,
               tree fntype, tree libtype,
               tree fnattrs, bool implicit_p)
{
  tree decl;
  const char *libname;

  if (fntype == error_mark_node)
    return;

  libname = name + strlen ("__builtin_");

  decl = add_builtin_function (libname, libtype, fncode, fnclass, NULL, fnattrs);
  built_in_decls[(int) fncode] = decl;
  if (implicit_p)
    implicit_built_in_decls[(int) fncode] = decl;

}

static void
init_builtins (void)
{
#define DEF_BUILTIN(ENUM, NAME, CLASS, TYPE, LIBTYPE, BOTH_P, FALLBACK_P, NONANSI_P, ATTRS, IMPLICIT, COND) \
      if (NAME && COND && BOTH_P)                         \
        def_builtin_1 (ENUM, NAME, CLASS,                 \
                       builtin_types[(int) TYPE],         \
                       builtin_types[(int) LIBTYPE],      \
                       built_in_attributes[(int) ATTRS],  \
                       IMPLICIT);
#  include "builtins.def"
#undef DEF_BUILTIN
}

void
cil_build_builtins (void)
{
  signed_size_type_node  = make_signed_type (POINTER_SIZE);
  wint_type_node         = short_unsigned_type_node;
  intmax_type_node       = long_long_integer_type_node;
  uintmax_type_node      = long_long_unsigned_type_node;
  string_type_node       = build_pointer_type (char_type_node);
  const_string_type_node = build_pointer_type (build_qualified_type (char_type_node, TYPE_QUAL_CONST));
  pid_type_node          = long_integer_type_node;
  va_list_arg_type_node  = va_list_type_node;
  va_list_ref_type_node  = build_reference_type (va_list_type_node);

  init_builtin_attributes ();
  init_builtin_types ();
  if (!flag_no_cbuiltin)
      init_builtins ();

  memset (builtin_types, 0, sizeof (builtin_types));
}

#include "gt-cil-cil-builtins.h"

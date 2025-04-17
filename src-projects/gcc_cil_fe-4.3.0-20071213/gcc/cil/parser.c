/* CIL Compiler parser and tree emitter.

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

   Author:
     Ricardo Fernandez Pascual <ricardof@um.es>
     Andrea C. Ornstein <andrea.ornstein@st.com>

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>
*/

/* Mono headers must be included before GCC ones, because they use
   identifiers poisoned in GCC's system.h */

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/opcodes.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "flags.h"
#include "toplev.h"
#include "version.h"

#include "tm.h"
#include "ggc.h"
#include "tree.h"
#include "cgraph.h"
#include "diagnostic.h"
#include "opts.h"
#include "options.h"
#include "real.h"

#include "tree-gimple.h"
#include "tree-dump.h"

#include "cil.h"
#include "cil-tree.h"
#include "parser.h"
#include "bindings.h"
#include "stack.h"

/* parse only the methods which are reachable from the entry point */
bool flag_parse_only_reachable = FALSE;

/* what to do when trying to compile a method that uses some unsupported feature */
UnsupportedMethodBehavior flag_unsupported_method_behavior = UMB_WARNING;

/* Auxiliary types and variables for compilation */

// typedef GHashTable CILLabelsMap; /* maps ip to label */
#define CILLabelsMap GHashTable

static CILLabelsMap *
cil_labels_set_new (void)
{
  return g_hash_table_new (g_direct_hash, g_direct_equal);
}

static tree
cil_labels_set_get_label (CILLabelsMap *labels, const unsigned char *ip)
{
  tree label_decl_tree = g_hash_table_lookup (labels, ip);
  if (!label_decl_tree)
    {
      char label_name[128];
      snprintf (label_name, sizeof(label_name), "IL%p", ip);
      label_decl_tree = build_decl (LABEL_DECL, get_identifier (label_name), void_type_node);
      DECL_CONTEXT (label_decl_tree) = current_function_decl;
      g_hash_table_insert (labels, (void *) ip, label_decl_tree);
    }
  return label_decl_tree;
}

static tree
cil_labels_get_label (CILLabelsMap *labels, const unsigned char *ip)
{
  return g_hash_table_lookup (labels, ip);
}

static void
cil_labels_set_free (CILLabelsMap *labels)
{
  g_hash_table_destroy (labels);
}

/* CIL opcode prefix information */
typedef struct
{
  bool constrained;
  guint32 constrained_token;
  bool no_typecheck;
  bool no_rangecheck;
  bool no_nullcheck;
  bool readonly;
  bool tail;
  bool unaligned;
  int unaligned_value;
  bool bolatile;
}
CILOpcodePrefix;
static const CILOpcodePrefix cil_opcode_prefix_empty = { false, 0, false, false, false, false, false, false, 0, false };
static CILOpcodePrefix parser_current_prefix;

/* Set of classes already parsed or being parsed (identity map of MonoClass*) */
static GHashTable *parsed_classes;

/* Maps from MONO metadata objects to trees (declarations) */
static GHashTable *parsed_classes_records; /* map from MonoClass* to tree, struct like declaration for the class */
static GHashTable *parsed_classes_unions; /* map from MonoClass* to tree, record plus byte array for types with explicit size */
static GHashTable *parsed_classes_static_records; /* map from MonoClass* to tree, struct for static fields */
static GHashTable *parsed_classes_static_storages; /* map from MonoClass* to tree, variable to hold static fields for a class */
static GHashTable *parsed_methods_decl; /* map from MonoMethod* to tree, declarations of all methods */
static GHashTable *parsed_methods_impl; /* map from MonoMethod* to tree, declarations of those  methods actually compiled */

/* Work queues */
static GSList *methods_to_parse_impl; /* of MonoMethod* */
static GSList *static_fields_to_init; /* of MonoClassField* */
static GSList *pinvoke_methods_to_init; /* of MonoMethod* */

/* the images that we are compiling */
static GSList *images_that_we_are_compiling; /* of MonoImage* */

static tree
parser_get_class_record_tree (MonoClass *klass)
{
  tree ret = g_hash_table_lookup (parsed_classes_records, klass);
  gcc_assert (ret && TREE_CODE (ret) == RECORD_TYPE);
  return ret;
}

static tree
parser_get_class_record_or_union_tree (MonoClass *klass)
{
  tree ret = g_hash_table_lookup (parsed_classes_unions, klass);
  if (!ret)
    {
      ret = g_hash_table_lookup (parsed_classes_records, klass);
    }
  else
    {
      gcc_assert (TREE_CODE (ret) == UNION_TYPE);
    }
  gcc_assert (ret);
  return ret;
}

static tree
parser_get_class_static_record_tree (MonoClass *klass)
{
  tree ret = g_hash_table_lookup (parsed_classes_static_records, klass);
  gcc_assert (ret && TREE_CODE (ret) == RECORD_TYPE);
  return ret;
}

static tree
parser_get_class_static_storage_tree (MonoClass *klass)
{
  tree ret = g_hash_table_lookup (parsed_classes_static_storages, klass);
  gcc_assert (ret && TREE_CODE (ret) == VAR_DECL);
  return ret;
}

static tree
parser_get_method_tree (MonoMethod *method)
{
  tree ret = g_hash_table_lookup (parsed_methods_decl, method);
  gcc_assert (ret && (TREE_CODE (ret) == FUNCTION_DECL || TREE_CODE (ret) == VAR_DECL));
  return ret;
}

static tree
parser_get_type_tree (MonoType *type)
{
  tree ret = NULL_TREE;
  int typetype = mono_type_get_type (type);
  switch (typetype)
    {
    case MONO_TYPE_VOID:
      ret = void_type_node;
      break;
    case MONO_TYPE_BOOLEAN:
      ret = cil_type_for_size (8, true); /* TODO: should this be signed or unsigned? */
      break;
    case MONO_TYPE_CHAR:
      ret = cil_type_for_size (16, true); /* TODO: should this be signed or unsigned? */
      break;
    case MONO_TYPE_I1:
      ret = cil_type_for_size (8, false);
      break;
    case MONO_TYPE_U1:
      ret = cil_type_for_size (8, true);
      break;
    case MONO_TYPE_I2:
      ret = cil_type_for_size (16, false);
      break;
    case MONO_TYPE_U2:
      ret = cil_type_for_size (16, true);
      break;
    case MONO_TYPE_I4:
      ret = cil_type_for_size (32, false);
      break;
    case MONO_TYPE_U4:
      ret = cil_type_for_size (32, true);
      break;
    case MONO_TYPE_I8:
      ret = cil_type_for_size (64, false);
      break;
    case MONO_TYPE_U8:
      ret = cil_type_for_size (64, true);
      break;
    case MONO_TYPE_R4:
      ret = float_type_node;
      break;
    case MONO_TYPE_R8:
      ret = double_type_node;
      break;
    case MONO_TYPE_CLASS:
      {
        MonoClass *klass = mono_type_get_class (type);
        ret = build_pointer_type (parser_get_class_record_tree (klass));
      }
      break;
    case MONO_TYPE_VALUETYPE:
      {
        MonoClass *klass = mono_type_get_class (type);
        if (mono_class_is_enum (klass))
          {
            MonoType *enum_basetype = mono_class_enum_basetype (klass);
            ret = parser_get_type_tree (enum_basetype);
          }
        else
          {
            ret = parser_get_class_record_or_union_tree (klass);
          }
      }
      break;
    case MONO_TYPE_STRING:
      ret = build_pointer_type (parser_get_class_record_tree (mono_get_string_class ()));
      break;
    case MONO_TYPE_PTR:
      {
        MonoType *points_to_type = type->data.type;
        tree points_to_type_tree = parser_get_type_tree (points_to_type);
        ret = build_pointer_type (points_to_type_tree);
      }
      break;
    case MONO_TYPE_TYPEDBYREF:
      /* TODO FIXME */
      ret = build_pointer_type (void_type_node);
      break;
    case MONO_TYPE_I:
      ret = integer_type_node;
      break;
    case MONO_TYPE_U:
      ret = unsigned_type_node;
      break;
    case MONO_TYPE_FNPTR:
      /* TODO FIXME (but this works anyway) */
      ret = build_pointer_type (void_type_node);
      break;
    case MONO_TYPE_OBJECT:
      ret = build_pointer_type(parser_get_class_record_tree (mono_get_object_class ()));
      break;
    case MONO_TYPE_SZARRAY:
      /* TODO FIXME once arrays are supported*/
      ret = build_pointer_type (void_type_node);
      break;
    default:
      fprintf (stderr, "Unhandled type type: 0x%x %s\n", typetype, mono_type_get_name (type));
      gcc_unreachable ();
    }
  gcc_assert (ret);
  return ret;
}

typedef enum
  {
    GCC_CIL_METHOD_MODE_COMPILE,
    GCC_CIL_METHOD_MODE_EXTERNAL,
    GCC_CIL_METHOD_MODE_EXTERNAL_STDLIB,
    GCC_CIL_METHOD_MODE_PINVOKE
  }
GCCCilMethodMode;

static gboolean
gcc_cil_method_mode_is_external (const GCCCilMethodMode m)
{
  return m == GCC_CIL_METHOD_MODE_EXTERNAL || m == GCC_CIL_METHOD_MODE_EXTERNAL_STDLIB;
}

static bool
parser_get_method_is_gcc4netstdlib (MonoMethod *method)
{
  MonoClass *method_klass = mono_method_get_class (method);
  MonoImage *method_image = mono_class_get_image(method_klass);
  const char *image_name = mono_image_get_name(method_image);
  if (strcmp ("libstd",image_name) == 0)
    {
      /* Even though these two methods are in libstd, they have no
         counterpart in a standard libc.  */
      const char *method_name = mono_method_get_name(method);
      if ( (strcmp (".init", method_name) == 0) ||
           (strcmp (".fini", method_name) == 0))
        {
          return 0;
        }
      return 1;
    }
  else
    {
      return 0;
    }
}

static GCCCilMethodMode
parser_get_method_mode (MonoMethod *method)
{
  MonoClass *klass = mono_method_get_class (method);
  MonoImage *image = mono_class_get_image (klass);
  if (g_slist_find (images_that_we_are_compiling, image))
    {
      if (mono_method_get_flags (method, NULL) & MONO_METHOD_ATTR_PINVOKE_IMPL)
        {
          return GCC_CIL_METHOD_MODE_PINVOKE;
        }
      else
        {
          return GCC_CIL_METHOD_MODE_COMPILE;
        }
    }
  else
    {
      if (parser_get_method_is_gcc4netstdlib (method))
        {
          return GCC_CIL_METHOD_MODE_EXTERNAL_STDLIB;
        }
      else
        {
          return GCC_CIL_METHOD_MODE_EXTERNAL;
        }
    }
}

typedef enum
  {
    GCC_CIL_CLASS_MODE_COMPILE,
    GCC_CIL_CLASS_MODE_EXTERNAL
  }
GCCCilClassMode;

static GCCCilMethodMode
parser_get_class_mode (MonoClass *klass)
{
  MonoImage *image = mono_class_get_image (klass);
  if (g_slist_find (images_that_we_are_compiling, image))
    {
      return GCC_CIL_CLASS_MODE_COMPILE;
    }
  else
    {
      return GCC_CIL_CLASS_MODE_EXTERNAL;
    }
}

static void
parser_emit_ldc_i4 (gint32 i)
{
  tree exp = build_int_cst (cil_type_for_size (32, false), i);
  cil_stack_push (exp, CIL_STYPE_INT32);
}

static void
parser_emit_ldc_i8 (gint64 i)
{
  tree exp = build_int_cst (cil_type_for_size (64, false), i);
  cil_stack_push (exp, CIL_STYPE_INT64);
}

static void
parser_emit_ldc_r4 (gint32 f)
{
  REAL_VALUE_TYPE real_value;
  const long *pf = (void *) &f;
  real_from_target_fmt (&real_value, pf, &ieee_single_format);
  tree exp = build_real (double_type_node, real_value);
  cil_stack_push (exp, CIL_STYPE_F);
}

static void
parser_emit_ldc_r8 (gint64 f)
{
  REAL_VALUE_TYPE real_value;
  const long *pf = (void *) &f;
  real_from_target_fmt (&real_value, pf, &ieee_double_format);
  tree exp = build_real (double_type_node, real_value);
  cil_stack_push (exp, CIL_STYPE_F);
}

/********* binary numeric operations ****************/

/*
 * BIN_OP:
 *  binop_code, binop_tag, tree_code_int, tree_code_f, handler_for_pointers
 *
 * handler_for_pointers is of type: tree (*) (tree,tree)
 */

#define CIL_BINARY_NUMERIC_OPERATIONS					\
  BIN_OP(add, ADD, PLUS_EXPR,      PLUS_EXPR,  parser_add_op_pointer_handler)	\
  BIN_OP(sub, SUB, MINUS_EXPR,     MINUS_EXPR, parser_sub_op_pointer_handler) \
  BIN_OP(mul, MUL, MULT_EXPR,      MULT_EXPR,  NULL)		\
  BIN_OP(div, DIV, TRUNC_DIV_EXPR, RDIV_EXPR,  NULL)		\
  BIN_OP(rem, REM, TRUNC_MOD_EXPR, ERROR_MARK, NULL)


enum cil_bin_op {
#define BIN_OP(binop_code, binop_tag, tree_code_int, tree_code_f, handler_for_pointers) \
  BINARY_NUMERIC_OPERATION_ ## binop_tag ,
  CIL_BINARY_NUMERIC_OPERATIONS
  BINARY_NUMERIC_OPERATION_NUM_OPS
#undef BIN_OP
};


static CilStackType
parser_get_binary_numeric_operations_type (enum cil_bin_op bin_op, CilStackType opA_type, CilStackType opB_type)
{
  if (opA_type == CIL_STYPE_ERROR || opB_type == CIL_STYPE_ERROR)
    {
      return CIL_STYPE_ERROR;
    }
  else
    {
      gcc_assert (opA_type < CIL_STYPE_ERROR);
      gcc_assert (opB_type < CIL_STYPE_ERROR);
      const CilStackType table[6][6] = {
        { CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_MP,    CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_INT64, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_MP,    CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_F,     CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_MP,    CIL_STYPE_ERROR, CIL_STYPE_MP,    CIL_STYPE_ERROR, CIL_STYPE_NINT,  CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
      };
      CilStackType ret = table[opA_type][opB_type];
      if (ret == CIL_STYPE_ERROR)
	{
	  switch (bin_op)
	    {
	    case BINARY_NUMERIC_OPERATION_SUB:
	      if (   (opA_type == CIL_STYPE_MP)
                  && (opB_type == CIL_STYPE_MP))
		ret = CIL_STYPE_NINT;

	      /* ... fall through ... */

	    case BINARY_NUMERIC_OPERATION_ADD:
	      if (   (opA_type == CIL_STYPE_MP)
                  || (opB_type == CIL_STYPE_MP))
		ret = CIL_STYPE_MP;
	      break;
	    default:
	      break;
	    }
	}
      return ret;
    }
}


static tree
parser_add_op_pointer_handler(tree opA, tree opB)
{
  tree exp = NULL_TREE;
  enum tree_code opcode;
  int pA = POINTER_TYPE_P (TREE_TYPE(opA));
  int pB = POINTER_TYPE_P (TREE_TYPE(opB));

  gcc_assert (pA || pB);
  if  (pA && !pB)
    {
      opcode = POINTER_PLUS_EXPR;
      opB = convert (unsigned_type_node, opB);
      exp = build2 (opcode, TREE_TYPE (opA), opA, opB);
    }
  else if (!pA && pB)
    {
      opcode = POINTER_PLUS_EXPR;
      opB = convert(unsigned_type_node, opA);
      exp = build2 (opcode,  TREE_TYPE (opB), opB, opA);
    }
  else
    {
      gcc_unreachable ();
    }

  return exp;
}


static tree
parser_sub_op_pointer_handler(tree opA, tree opB)
{
  tree exp = NULL_TREE;
  enum tree_code opcode;
  int pA = POINTER_TYPE_P (TREE_TYPE(opA));
  int pB = POINTER_TYPE_P (TREE_TYPE(opB));

  gcc_assert (pA || pB);
  if  (pA && !pB)
    {
      opcode = POINTER_PLUS_EXPR;
      opB = convert(unsigned_type_node, opB);
      opB = build1(NEGATE_EXPR, TREE_TYPE(opB), opB);
      exp = build2 (opcode, TREE_TYPE (opA), opA, opB);
    }
  else if (!pA && pB)
    {
      opcode = POINTER_PLUS_EXPR;
      opB = convert(unsigned_type_node, opA);
      opA = build1(NEGATE_EXPR, TREE_TYPE(opA), opA);
      exp = build2 (opcode,  TREE_TYPE (opB), opB, opA);
    }
  else
    {
      gcc_assert (TREE_TYPE (opA) == TREE_TYPE (opB));
      opcode = MINUS_EXPR;
      tree elem_t = TREE_TYPE( TREE_TYPE (opA)); /* FIXME arrays ?*/
      opA = convert (integer_type_node, opA);
      opB = convert (integer_type_node, opB);
      exp = build2 (opcode, TREE_TYPE (opA), opA, opB);
      exp = build2 (EXACT_DIV_EXPR, elem_t, exp, TYPE_SIZE_UNIT (integer_type_node));
    }

  return exp;
}


static void
parser_emit_binary_numeric_operation(enum cil_bin_op bin_op,
				     const char* opname,
				     enum tree_code opcode_i,
				     enum tree_code opcode_f,
				     tree (*pointers_handler)(tree,tree))
{
  CilStackType opB_type;
  tree opB = cil_stack_pop (&opB_type);
  CilStackType opA_type;
  tree opA = cil_stack_pop (&opA_type);
  CilStackType result_stack_type = parser_get_binary_numeric_operations_type (bin_op, opA_type, opB_type);
  if (result_stack_type == CIL_STYPE_ERROR)
    {
      error ("Wrong operand types for %s\n", opname);
    }
  int pA = POINTER_TYPE_P (TREE_TYPE(opA));
  int pB = POINTER_TYPE_P (TREE_TYPE(opB));
  tree exp = NULL_TREE;
  switch (result_stack_type)
    {
    case CIL_STYPE_INT64:
      /* ... FALL THROUGH ...*/

    case CIL_STYPE_INT32:
      gcc_assert( !pA && !pB);
      exp = build2 (opcode_i,  TREE_TYPE (opA), opA, opB);
      break;

    case CIL_STYPE_F:
      gcc_assert( !pA && !pB);
      exp = build2 (opcode_f,  TREE_TYPE (opA), opA, opB);
      break;

    case CIL_STYPE_NINT:
      if (!pA && !pB)
	{
	  exp = build2 (opcode_i,  TREE_TYPE (opA), opA, opB);
	  break;
	}

      /* ... FALL THROUGH ...*/

    case CIL_STYPE_MP:
      if (!pA && !pB)
	{
	  gcc_unreachable ();
	  break;
	}

      /* case of pointers */
      if (!pointers_handler)
	{
	  gcc_unreachable();
	}
      exp = (*pointers_handler)(opA, opB);

      break;
    default:
      gcc_unreachable ();
    }
  cil_stack_push (exp, result_stack_type);
}


/* emit the functions here */
#define BIN_OP(binop_code, binop_tag, tree_code_int, tree_code_f, handler_for_pointers) \
static void \
parser_emit_ ## binop_code (void) \
{ \
  parser_emit_binary_numeric_operation(BINARY_NUMERIC_OPERATION_ ## binop_tag, #binop_code, tree_code_int, tree_code_f, handler_for_pointers); \
}
CIL_BINARY_NUMERIC_OPERATIONS
#undef BIN_OP


#undef CIL_BINARY_NUMERIC_OPERATIONS
/********* END binary numeric operations ****************/

static void
parser_emit_min (void)
{
  CilStackType opB_type;
  tree opB = cil_stack_pop (&opB_type);
  CilStackType opA_type;
  tree opA = cil_stack_pop (&opA_type);
  tree exp = build2 (MIN_EXPR, TREE_TYPE (opB), opA, opB);
  cil_stack_push (exp, opB_type);
}

static void
parser_emit_max (void)
{
  CilStackType opB_type;
  tree opB = cil_stack_pop (&opB_type);
  CilStackType opA_type;
  tree opA = cil_stack_pop (&opA_type);
  tree exp = build2 (MAX_EXPR, TREE_TYPE (opB), opA, opB);
  cil_stack_push (exp, opB_type);
}

static void
parser_emit_abs (void)
{
  CilStackType op_type;
  tree op = cil_stack_pop (&op_type);
  tree exp = build1 (ABS_EXPR, TREE_TYPE (op), op);
  cil_stack_push (exp, op_type);
}


/********* integer operations ****************/

#define CIL_INTEGER_OPERATIONS			\
  INT_OP(div_un, UNSIGNED, TRUNC_DIV_EXPR)	\
  INT_OP(rem_un, UNSIGNED, TRUNC_MOD_EXPR)	\
  INT_OP(and,    SIGNED,   BIT_AND_EXPR)	\
  INT_OP(or,     SIGNED,   BIT_IOR_EXPR)	\
  INT_OP(xor,    SIGNED,   BIT_XOR_EXPR)



static CilStackType
parser_get_integer_operations_type_tree (CilStackType opA_type, CilStackType opB_type)
{
  if (opA_type == CIL_STYPE_ERROR || opB_type == CIL_STYPE_ERROR)
    {
      return CIL_STYPE_ERROR;
    }
  else
    {
      static CilStackType table[6][6] = {
	{ CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
	{ CIL_STYPE_ERROR, CIL_STYPE_INT64, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
	{ CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
	{ CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
	{ CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
	{ CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
      };
      gcc_assert (opA_type < CIL_STYPE_ERROR);
      gcc_assert (opB_type < CIL_STYPE_ERROR);
      return table[opA_type][opB_type];
    }
}

/* create 'emit'fonctions */
#define SIGNED(t)
#define UNSIGNED(t) t = convert (cil_unsigned_or_pointer_type (TREE_TYPE (t)), (t));

#define INT_OP(name, CONVERT, OPCODE) \
static void \
parser_emit_##name (void) \
{ \
  CilStackType opB_type; \
  tree opB = cil_stack_pop (&opB_type); \
  CilStackType opA_type; \
  tree opA = cil_stack_pop (&opA_type); \
  CilStackType result_type = parser_get_integer_operations_type_tree (opA_type, opB_type); \
  if (result_type == CIL_STYPE_ERROR) \
    { \
      error ("Wrong operand types for "#name); \
    } \
  CONVERT (opA) \
  CONVERT (opB) \
  if (result_type == CIL_STYPE_NINT) \
    { \
      int pA = POINTER_TYPE_P (TREE_TYPE(opA)); \
      int pB = POINTER_TYPE_P (TREE_TYPE(opB)); \
      if (pA) opA = convert (unsigned_type_node, opA);	\
      if (pB) opB = convert (unsigned_type_node, opB);	\
    } \
  tree exp = build2 (OPCODE, TREE_TYPE (opA), opA, opB);	\
  cil_stack_push (exp, result_type); \
}

/* the code is emitted here: */
CIL_INTEGER_OPERATIONS;

#undef INT_OP
#undef SIGNED
#undef UNSIGNED
#undef CIL_INTEGER_OPERATIONS



/********* end of integer operations ****************/


static void
parser_emit_neg (void)
{
  CilStackType op_type;
  tree op = cil_stack_pop (&op_type);
  tree exp = build1 (NEGATE_EXPR, TREE_TYPE (op), op);
  cil_stack_push (exp, op_type);
}

static void
parser_emit_not (void)
{
  CilStackType op_type;
  tree op = cil_stack_pop (&op_type);
  tree exp = build1 (BIT_NOT_EXPR, TREE_TYPE (op), op);
  cil_stack_push (exp, op_type);
}

static void
parser_emit_ret (MonoMethod *method)
{
  MonoMethodSignature *signature = mono_method_signature (method);
  MonoType *return_type = mono_signature_get_return_type (signature);
  if (mono_type_get_type (return_type) == MONO_TYPE_VOID)
    {
      tree exp = build1 (RETURN_EXPR, void_type_node, NULL_TREE);
      cil_bindings_output_statements (exp);
    }
  else
    {
      tree return_type_tree = parser_get_type_tree (return_type);
      gcc_assert (TREE_TYPE (TREE_TYPE (current_function_decl)) == return_type_tree);
      gcc_assert (TREE_TYPE (DECL_RESULT (current_function_decl)) == return_type_tree);
      tree op = cil_stack_pop (NULL);
      tree converted_op = convert (return_type_tree, op);
      tree setret = build2 (MODIFY_EXPR, return_type_tree, DECL_RESULT (current_function_decl), converted_op);
      TREE_SIDE_EFFECTS (setret) = 1;
      TREE_USED (setret) = 1;
      tree exp = build1 (RETURN_EXPR, return_type_tree, setret);
      cil_bindings_output_statements (exp);
    }
  if (!cil_stack_is_empty ())
    {
      error ("CIL evaluation stack is not empty after ret.");
      cil_stack_debug_dump ();
    }
}

static void
parser_emit_stloc (guint16 local)
{
  tree local_decl = cil_bindings_get_local (local);
  tree value = cil_stack_pop (NULL);
  tree converted_value = convert (TREE_TYPE (local_decl), value);
  tree setexp = fold_build2 (MODIFY_EXPR, TREE_TYPE (local_decl), local_decl, converted_value);
  TREE_SIDE_EFFECTS (setexp) = 1;
  TREE_USED (setexp) = 1;
  cil_bindings_output_statements (setexp);
}

static void
parser_emit_ldloc (guint16 local)
{
  tree local_decl = cil_bindings_get_local (local);
  cil_stack_push_infer_type (cil_bindings_output_statements_and_create_temp (local_decl));
}

static void
parser_emit_ldloca (guint16 local)
{
  tree local_decl = cil_bindings_get_local (local);
  tree exp_addr = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (local_decl)), local_decl);
  cil_stack_push (cil_bindings_output_statements_and_create_temp (exp_addr), CIL_STYPE_MP);
}

static void
parser_emit_ldarg (guint16 arg)
{
  tree arg_decl = cil_bindings_get_arg (arg);
  cil_stack_push_infer_type (cil_bindings_output_statements_and_create_temp (arg_decl));
}

static void
parser_emit_ldarga (guint16 arg)
{
  tree arg_decl = cil_bindings_get_arg (arg);
  tree exp_addr = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (arg_decl)), arg_decl);
  cil_stack_push (cil_bindings_output_statements_and_create_temp (exp_addr), CIL_STYPE_MP);
}

static void
parser_emit_starg (guint16 arg)
{
  tree local_decl = cil_bindings_get_arg (arg);
  tree value = cil_stack_pop (NULL);
  tree converted_value = convert (TREE_TYPE (local_decl), value);
  tree setexp = fold_build2 (MODIFY_EXPR, TREE_TYPE (local_decl), local_decl, converted_value);
  TREE_SIDE_EFFECTS (setexp) = 1;
  TREE_USED (setexp) = 1;
  cil_bindings_output_statements (setexp);
}

static tree
parser_get_signature_args_type_tree_list (MonoMethodSignature *signature, MonoClass *this_class)
{
  tree args_type_list = NULL_TREE;
  if (mono_signature_is_instance (signature) && !mono_signature_explicit_this (signature))
    {
      tree param_type_tree = this_class ? parser_get_type_tree (mono_class_get_type (this_class)) : parser_get_type_tree (mono_class_get_type (mono_get_object_class ()));
      args_type_list = tree_cons (NULL_TREE, param_type_tree, args_type_list);
    }
  gpointer iter = NULL;
  MonoType *param_type;
  while ((param_type = mono_signature_get_params (signature, &iter)) != NULL)
    {
      tree param_type_tree = parser_get_type_tree (param_type);
      args_type_list = tree_cons (NULL_TREE, param_type_tree, args_type_list);
    }
  args_type_list = nreverse (args_type_list);
  return args_type_list;
}

static void
parser_emit_call (MonoMethod *caller, guint32 token)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoMethod *called = mono_get_method (image, token, NULL);
  MonoMethodSignature *signature = mono_method_get_signature_full (called, image, token, NULL);

  MonoClass *called_klass = mono_method_get_class (called);
  if (   strcmp ("Crt", mono_class_get_name (called_klass)) == 0
      && strcmp ("gcc4net", mono_class_get_namespace (called_klass)) == 0)
    {
      const char *called_name = mono_method_get_name (called);

      if (strncmp ("__abs", called_name, 5) == 0)
        {
          parser_emit_abs();
          return;
        }
      else
        if (   strncmp ("__min", called_name, 5) == 0
            || strncmp ("__umin", called_name, 6) == 0)
          {
            parser_emit_min();
            return;
          }
        else
        if (   strncmp ("__max", called_name, 5) == 0
            || strncmp ("__umax", called_name, 6) == 0)
          {
            parser_emit_max();
            return;
          }
    }

  tree called_tree = parser_get_method_tree (called);
  tree arglist = NULL_TREE;
  /* TODO: check types (to show errors) */
  tree args_type_list = nreverse (parser_get_signature_args_type_tree_list (signature, mono_method_get_class (called)));
  guint32 i;
  for (i = mono_signature_get_param_count (signature); i > 0; --i)
    {
      tree arg_type_tree = TREE_VALUE (args_type_list);
      tree arg = convert (arg_type_tree, cil_stack_pop (NULL));
      arglist = tree_cons (NULL_TREE, arg, arglist);
      args_type_list = TREE_CHAIN (args_type_list);
    }
  if (mono_signature_is_instance (signature) && !mono_signature_explicit_this (signature))
    {
      tree arg_type_tree = TREE_VALUE (args_type_list);
      tree arg = convert (arg_type_tree, cil_stack_pop (NULL));
      arglist = tree_cons (NULL_TREE, arg, arglist);
      args_type_list = TREE_CHAIN (args_type_list);
    }
  gcc_assert (args_type_list == NULL_TREE);
  tree exp;
  if (parser_get_method_mode (called) == GCC_CIL_METHOD_MODE_PINVOKE)
    {
      /* the tree is a variable declaration of a pointer to the function */
      exp = build_call_list (TREE_TYPE (TREE_TYPE (TREE_TYPE (called_tree))), called_tree, arglist);
    }
  else
    {
      /* the tree is the function declaration */
      exp = build_function_call_expr (called_tree, arglist);
    }
  if (mono_type_get_type (mono_signature_get_return_type (signature)) == MONO_TYPE_VOID)
    {
      cil_bindings_output_statements (exp);
    }
  else
    {
      cil_stack_push_infer_type (cil_bindings_output_statements_and_create_temp (exp));
    }
}

static tree
parser_signature_tree (MonoMethodSignature *signature, MonoClass *this_class)
{
  tree args_type_list = parser_get_signature_args_type_tree_list (signature, this_class);
  MonoType *ret_type = mono_signature_get_return_type (signature);
  tree ret_type_tree =  parser_get_type_tree (ret_type);
  return build_function_type (ret_type_tree, args_type_list);
}

static void
parser_emit_calli (MonoMethod *caller, guint32 token)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoMethodSignature *signature = mono_metadata_parse_signature (image, token);

  tree ftn_tree = cil_stack_pop (NULL);
  tree signature_type_tree = parser_signature_tree (signature, NULL);
  tree converted_ftn_tree = convert (build_pointer_type (signature_type_tree), ftn_tree);

  tree arglist = NULL_TREE;
  /* TODO: check types (to show errors) */
  tree args_type_list = nreverse (parser_get_signature_args_type_tree_list (signature, NULL));
  guint32 i;
  for (i = mono_signature_get_param_count (signature); i > 0; --i)
    {
      tree arg_type_tree = TREE_VALUE (args_type_list);
      tree arg = convert (arg_type_tree, cil_stack_pop (NULL));
      arglist = tree_cons (NULL_TREE, arg, arglist);
      args_type_list = TREE_CHAIN (args_type_list);
    }
  if (mono_signature_is_instance (signature) && !mono_signature_explicit_this (signature))
    {
      tree arg_type_tree = TREE_VALUE (args_type_list);
      tree arg = convert (arg_type_tree, cil_stack_pop (NULL));
      arglist = tree_cons (NULL_TREE, arg, arglist);
      args_type_list = TREE_CHAIN (args_type_list);
    }
  gcc_assert (args_type_list == NULL_TREE);
  tree exp = build_call_list (parser_get_type_tree (mono_signature_get_return_type (signature)), converted_ftn_tree, arglist);

  if (mono_type_get_type (mono_signature_get_return_type (signature)) == MONO_TYPE_VOID)
    {
      cil_bindings_output_statements (exp);
    }
  else
    {
      cil_stack_push_infer_type (cil_bindings_output_statements_and_create_temp (exp));
    }
}

static void
parser_emit_ldftn (MonoMethod *caller, guint32 token)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoMethod *called = mono_get_method (image, token, NULL);
  tree called_addr_tree;
  if (parser_get_method_mode (called) == GCC_CIL_METHOD_MODE_PINVOKE)
    {
      called_addr_tree = parser_get_method_tree (called);
    }
  else
    {
      tree called_tree = parser_get_method_tree (called);
      called_addr_tree = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (called_tree)), called_tree);
    }
  cil_stack_push (cil_bindings_output_statements_and_create_temp (called_addr_tree), CIL_STYPE_NINT);
}

static tree
parser_find_field_in_record (tree record, tree field_name)
{
  tree field_decl;
  for (field_decl = TYPE_FIELDS (record); field_decl; field_decl = TREE_CHAIN (field_decl))
    {
      if (DECL_NAME (field_decl) == field_name)
        {
          return field_decl;
        }
    }
  gcc_unreachable ();
}

static tree
parser_get_field_decl_tree (MonoClassField *field)
{
  tree field_name_tree = get_identifier (mono_field_get_name (field));
  MonoClass *klass = mono_field_get_parent (field);
  if (mono_field_get_flags (field) & MONO_FIELD_ATTR_STATIC)
    {
      tree storage_record = parser_get_class_static_record_tree (klass);
      tree field_decl = parser_find_field_in_record (storage_record, field_name_tree);
      return field_decl;
    }
  else
    {
      tree storage_record = parser_get_class_record_tree (klass);
      tree field_decl = parser_find_field_in_record (storage_record, field_name_tree);
      return field_decl;
    }
}

static tree
parser_build_volatile_reference_tree (tree exp)
{
  tree exp_type = TREE_TYPE (exp);
  tree v_type = build_qualified_type (exp_type, TYPE_QUALS (exp_type) | TYPE_QUAL_VOLATILE);
  tree addr = build_fold_addr_expr (exp);
  tree ptr_v_type = build_pointer_type (v_type);
  addr = fold_convert (ptr_v_type, addr);
  return build_fold_indirect_ref (addr);
}

static tree
parser_build_volatile_pointer_tree (tree ptr)
{
  gcc_assert (POINTER_TYPE_P (TREE_TYPE (ptr)));
  tree exp_type = TREE_TYPE (TREE_TYPE (ptr));
  tree v_type = build_qualified_type (exp_type, TYPE_QUALS (exp_type) | TYPE_QUAL_VOLATILE);
  tree ptr_v_type = build_pointer_type (v_type);
  return fold_convert (ptr_v_type, ptr);
}

static tree
parser_build_static_field_ref_tree (MonoClassField *field)
{
  gcc_assert (mono_field_get_flags (field) & MONO_FIELD_ATTR_STATIC);
  tree field_decl = parser_get_field_decl_tree (field);
  MonoClass *klass = mono_field_get_parent (field);
  tree storage_tree = parser_get_class_static_storage_tree (klass);
  tree field_ref = build3 (COMPONENT_REF, TREE_TYPE (field_decl), storage_tree, field_decl, NULL_TREE);
  return field_ref;
}

static void
parser_emit_ldsfld (MonoMethod *caller, guint32 token)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoClass *klass = NULL;
  MonoClassField *field = mono_field_from_token (image, token, &klass, NULL);
  tree exp = parser_build_static_field_ref_tree (field);
  if (parser_current_prefix.bolatile)
    {
      exp = parser_build_volatile_reference_tree (exp);
    }
  cil_stack_push_infer_type (cil_bindings_output_statements_and_create_temp (exp));
}

static void
parser_emit_stsfld (MonoMethod *caller, guint32 token)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoClass *klass = NULL;
  MonoClassField *field = mono_field_from_token (image, token, &klass, NULL);
  tree field_ref = parser_build_static_field_ref_tree (field);
  if (parser_current_prefix.bolatile)
    {
      field_ref = parser_build_volatile_reference_tree (field_ref);
    }
  tree value_tree = cil_stack_pop (NULL);
  tree converted_value_tree = convert (TREE_TYPE (field_ref), value_tree);
  tree setexp = build2 (MODIFY_EXPR, TREE_TYPE (field_ref), field_ref, converted_value_tree);
  TREE_SIDE_EFFECTS (setexp) = 1;
  TREE_USED (setexp) = 1;
  cil_bindings_output_statements (setexp);
}

static tree
parser_build_static_field_address_tree (MonoClassField *field)
{
  tree field_ref = parser_build_static_field_ref_tree (field);
  tree exp_addr = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (field_ref)), field_ref);
  return exp_addr;
}

static void
parser_emit_ldsflda (MonoMethod *caller, guint32 token)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoClass *klass = NULL;
  MonoClassField *field = mono_field_from_token (image, token, &klass, NULL);
  tree exp_addr = parser_build_static_field_address_tree (field);
  /* FIXME: the type should be CIL_STYPE_NINT if the object memory is unmanaged */
  cil_stack_push (exp_addr, CIL_STYPE_MP);
}

static tree
parser_build_field_ref_tree (MonoClassField *field, tree obj_ptr_tree)
{
  MonoClass *klass = mono_field_get_parent (field);
  tree obj_type_tree = parser_get_class_record_tree (klass);
  tree converted_obj_ptr_tree = convert (build_pointer_type (obj_type_tree), obj_ptr_tree);
  tree obj_tree = build1 (INDIRECT_REF, obj_type_tree, converted_obj_ptr_tree);
  tree field_decl = parser_get_field_decl_tree (field);
  tree field_ref = build3 (COMPONENT_REF, TREE_TYPE (field_decl), obj_tree, field_decl, NULL_TREE);
  return field_ref;
}

static void
parser_emit_stfld (MonoMethod *caller, guint32 token)
{
  gcc_assert (!parser_current_prefix.unaligned);  /* TODO: handle unaligned prefix  */
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoClass *klass = NULL;
  MonoClassField *field = mono_field_from_token (image, token, &klass, NULL);
  tree value_tree = cil_stack_pop (NULL);
  tree dst_obj_ptr_tree = cil_stack_pop (NULL);
  tree field_ref = parser_build_field_ref_tree (field, dst_obj_ptr_tree);
  if (parser_current_prefix.bolatile)
    {
      field_ref = parser_build_volatile_reference_tree (field_ref);
    }
  tree converted_value_tree = convert (TREE_TYPE (field_ref), value_tree);
  tree setexp = build2 (MODIFY_EXPR, TREE_TYPE (field_ref), field_ref, converted_value_tree);
  TREE_SIDE_EFFECTS (setexp) = 1;
  TREE_USED (setexp) = 1;
  cil_bindings_output_statements (setexp);
}

static void
parser_emit_ldfld (MonoMethod *caller, guint32 token)
{
  gcc_assert (!parser_current_prefix.unaligned);  /* TODO: handle unaligned prefix  */
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoClass *klass = NULL;
  MonoClassField *field = mono_field_from_token (image, token, &klass, NULL);
  tree src_obj_ptr_tree = cil_stack_pop (NULL);
  tree field_ref = parser_build_field_ref_tree (field, src_obj_ptr_tree);
  if (parser_current_prefix.bolatile)
    {
      field_ref = parser_build_volatile_reference_tree (field_ref);
    }
  cil_stack_push_infer_type (cil_bindings_output_statements_and_create_temp (field_ref));
}

static void
parser_emit_ldflda (MonoMethod *caller, guint32 token)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (caller));
  MonoClass *klass = NULL;
  MonoClassField *field = mono_field_from_token (image, token, &klass, NULL);
  CilStackType src_obj_ptr_type;
  tree src_obj_ptr_tree = cil_stack_pop (&src_obj_ptr_type);
  tree field_ref = parser_build_field_ref_tree (field, src_obj_ptr_tree);
  tree field_ref_addr = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (field_ref)), field_ref);
  cil_stack_push (cil_bindings_output_statements_and_create_temp (field_ref_addr),
                  src_obj_ptr_type == CIL_STYPE_NINT ? CIL_STYPE_NINT : CIL_STYPE_MP);
}

static void
parser_emit_ldobj (MonoMethod *method, guint32 token)
{
  gcc_assert (!parser_current_prefix.unaligned);  /* TODO: handle unaligned prefix  */
  MonoImage *image = mono_class_get_image (mono_method_get_class (method));
  MonoClass *klass = mono_class_get (image, token);
  MonoType *type = mono_class_get_type (klass);
  tree type_tree = parser_get_type_tree (type);
  tree src_tree = cil_stack_pop (NULL);
  tree converted_src_tree = convert (build_pointer_type (type_tree), src_tree);
  if (mono_type_get_type (type) == MONO_TYPE_VALUETYPE)
    {
      tree value_tree = build1 (INDIRECT_REF, type_tree, converted_src_tree);
      if (parser_current_prefix.bolatile)
        {
          value_tree = parser_build_volatile_reference_tree (value_tree);
        }
      cil_stack_push (cil_bindings_output_statements_and_create_temp (value_tree), CIL_STYPE_OBJECT);
    }
  else
    {
      /* TODO: not yet implemented */
      gcc_unreachable ();
    }
}

static void
parser_emit_stobj (MonoMethod *method, guint32 token)
{
  gcc_assert (!parser_current_prefix.unaligned);  /* TODO: handle unaligned prefix  */
  MonoImage *image = mono_class_get_image (mono_method_get_class (method));
  MonoClass *klass = mono_class_get (image, token);
  MonoType *type = mono_class_get_type (klass);
  tree type_tree = parser_get_type_tree (type);
  tree value_tree = cil_stack_pop (NULL);
  tree dest_ptr_tree = cil_stack_pop (NULL);
  tree converted_dest_ptr_tree = convert (build_pointer_type (type_tree), dest_ptr_tree);
  if (mono_type_get_type (type) == MONO_TYPE_VALUETYPE)
    {
      tree dest_tree = build1 (INDIRECT_REF, type_tree, converted_dest_ptr_tree);
      if (parser_current_prefix.bolatile)
        {
          dest_tree = parser_build_volatile_reference_tree (dest_tree);
        }
      tree setexp = fold_build2 (MODIFY_EXPR, type_tree, dest_tree, value_tree);
      TREE_SIDE_EFFECTS (setexp) = 1;
      TREE_USED (setexp) = 1;
      cil_bindings_output_statements (setexp);
    }
  else
    {
      /* TODO: not yet implemented */
      gcc_unreachable ();
    }
}

#define TEMPLATE_PARSER_EMIT_CONV(name, type_tree, stack_type) \
static void \
parser_emit_conv_##name (void) \
{ \
  tree exp = cil_stack_pop (NULL); \
  tree converted_exp = convert (type_tree, exp); \
  cil_stack_push (converted_exp, stack_type); \
}
TEMPLATE_PARSER_EMIT_CONV (i, integer_type_node, CIL_STYPE_NINT)
TEMPLATE_PARSER_EMIT_CONV (i1, cil_type_for_size (8, false), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_CONV (u1, cil_type_for_size (8, true), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_CONV (i2, cil_type_for_size (16, false), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_CONV (u2, cil_type_for_size (16, true), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_CONV (i4, cil_type_for_size (32, false), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_CONV (u4, cil_type_for_size (32, true), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_CONV (i8, cil_type_for_size (64, false), CIL_STYPE_INT64)
TEMPLATE_PARSER_EMIT_CONV (u8, cil_type_for_size (64, true), CIL_STYPE_INT64)
TEMPLATE_PARSER_EMIT_CONV (r4, float_type_node, CIL_STYPE_F)
TEMPLATE_PARSER_EMIT_CONV (r8, double_type_node, CIL_STYPE_F)
#undef TEMPLATE_PARSER_EMIT_CONV

static void
parser_emit_conv_r_un (void)
{
  tree exp = cil_stack_pop (NULL);
  tree exp_un = convert (cil_unsigned_or_pointer_type (TREE_TYPE (exp)), exp);
  tree converted_exp = convert (double_type_node, exp_un);
  cil_stack_push (converted_exp, CIL_STYPE_F);
}

#define TEMPLATE_PARSER_EMIT_LDIND(name, elem_type_tree, stack_type) \
static void \
parser_emit_ldind_##name (void) \
{ \
  gcc_assert (!parser_current_prefix.unaligned);  /* TODO: handle unaligned prefix  */ \
  tree ptr_type_tree = build_pointer_type (elem_type_tree); \
  tree ptr_tree = convert (ptr_type_tree, cil_stack_pop (NULL)); \
  tree value_tree = build1 (INDIRECT_REF, elem_type_tree, ptr_tree); \
  if (parser_current_prefix.bolatile) \
    { \
      value_tree = parser_build_volatile_reference_tree (value_tree); \
    } \
  tree stack_type_tree = cil_stack_get_tree_type_for_cil_stack_type (stack_type); \
  tree converted_value_tree = convert (stack_type_tree, value_tree); \
  cil_stack_push (cil_bindings_output_statements_and_create_temp (converted_value_tree), stack_type); /* TODO: unnecesary? */\
}
TEMPLATE_PARSER_EMIT_LDIND (i, integer_type_node, CIL_STYPE_NINT)
TEMPLATE_PARSER_EMIT_LDIND (i1, cil_type_for_size (8, false), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_LDIND (u1, cil_type_for_size (8, true), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_LDIND (i2, cil_type_for_size (16, false), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_LDIND (u2, cil_type_for_size (16, true), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_LDIND (i4, cil_type_for_size (32, false), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_LDIND (u4, cil_type_for_size (32, true), CIL_STYPE_INT32)
TEMPLATE_PARSER_EMIT_LDIND (r4, float_type_node, CIL_STYPE_F)
TEMPLATE_PARSER_EMIT_LDIND (r8, double_type_node, CIL_STYPE_F)
#undef TEMPLATE_PARSER_EMIT_LDIND

#define TEMPLATE_PARSER_EMIT_STIND(name, value_type_tree) \
static void \
parser_emit_stind_##name (void) \
{ \
  gcc_assert (!parser_current_prefix.unaligned);  /* TODO: handle unaligned prefix  */ \
  tree value_tree = convert (value_type_tree, cil_stack_pop (NULL)); \
  tree ptr_type_tree = build_pointer_type (value_type_tree); \
  tree ptr_tree = convert (ptr_type_tree, cil_stack_pop (NULL)); \
  tree dest_tree = build1 (INDIRECT_REF, value_type_tree, ptr_tree); \
  if (parser_current_prefix.bolatile) \
    { \
      dest_tree = parser_build_volatile_reference_tree (dest_tree); \
    } \
  tree expr = build2 (MODIFY_EXPR, TREE_TYPE (dest_tree), dest_tree, value_tree); \
  cil_bindings_output_statements (expr); \
}
TEMPLATE_PARSER_EMIT_STIND (i, integer_type_node)
TEMPLATE_PARSER_EMIT_STIND (i1, cil_type_for_size (8, false))
TEMPLATE_PARSER_EMIT_STIND (i2, cil_type_for_size (16, false))
TEMPLATE_PARSER_EMIT_STIND (i4, cil_type_for_size (32, false))
TEMPLATE_PARSER_EMIT_STIND (r4, float_type_node)
TEMPLATE_PARSER_EMIT_STIND (r8, double_type_node)
#undef TEMPLATE_PARSER_EMIT_STIND

static void
parser_emit_pop (void)
{
  cil_stack_pop (NULL);
}

static void
parser_emit_dup (void)
{
  CilStackType type;
  tree value = cil_stack_pop (&type);
  cil_stack_push (value, type);
  cil_stack_push (value, type);
}

static CilStackType
parser_get_binary_numeric_comparisons_type (CilStackType opA_type, CilStackType opB_type)
{
  if (opA_type == CIL_STYPE_ERROR || opB_type == CIL_STYPE_ERROR)
    {
      return CIL_STYPE_ERROR;
    }
  else
    {
      gcc_assert (opA_type < CIL_STYPE_ERROR);
      gcc_assert (opB_type < CIL_STYPE_ERROR);
      const CilStackType table[6][6] = {
        { CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_INT32 },
      };
      return table[opA_type][opB_type];
    }
}

#define TEMPLATE_PARSER_EMIT_BINARY_NUMERIC_COMPARISON(name, tree_code, tree_code_float, unsignedp) \
static void \
parser_emit_##name (void) \
{ \
  CilStackType opB_type; \
  tree opB = cil_stack_pop (&opB_type); \
  CilStackType opA_type; \
  tree opA = cil_stack_pop (&opA_type); \
  CilStackType result_type = parser_get_binary_numeric_comparisons_type (opA_type, opB_type); \
  if (result_type == CIL_STYPE_ERROR) \
    { \
      error ("Wrong operand types for "#name); \
    } \
  tree result_type_tree = cil_stack_get_tree_type_for_cil_stack_type (result_type); \
  if (TREE_TYPE (opA) != TREE_TYPE (opB)) \
    { \
      int iA = (TREE_CODE (TREE_TYPE (opA)) == INTEGER_TYPE);	\
      int iB = (TREE_CODE (TREE_TYPE (opB)) == INTEGER_TYPE);	\
      if (!iA && iB) opA = convert(TREE_TYPE (opB), opA); \
      opB = convert(TREE_TYPE (opA), opB); \
    } \
  if (opA_type == CIL_STYPE_F) \
    { \
      gcc_assert (opB_type == CIL_STYPE_F); \
      tree exp = build2 (tree_code_float, result_type_tree, opA, opB); \
      cil_stack_push (exp, CIL_STYPE_INT32); \
    } \
  else \
    { \
      tree opA_converted = unsignedp ? convert (cil_unsigned_or_pointer_type (TREE_TYPE (opA)), opA) : opA; \
      tree opB_converted = unsignedp ? convert (cil_unsigned_or_pointer_type (TREE_TYPE (opB)), opB) : opB; \
      tree exp = build2 (tree_code, result_type_tree, opA_converted, opB_converted); \
      cil_stack_push (exp, CIL_STYPE_INT32); \
    } \
}
TEMPLATE_PARSER_EMIT_BINARY_NUMERIC_COMPARISON(clt_un, LT_EXPR, UNLT_EXPR, true)
TEMPLATE_PARSER_EMIT_BINARY_NUMERIC_COMPARISON(cgt_un, GT_EXPR, UNGT_EXPR, true)
TEMPLATE_PARSER_EMIT_BINARY_NUMERIC_COMPARISON(cgt, GT_EXPR, GT_EXPR, false)
TEMPLATE_PARSER_EMIT_BINARY_NUMERIC_COMPARISON(clt, LT_EXPR, LT_EXPR, false)
TEMPLATE_PARSER_EMIT_BINARY_NUMERIC_COMPARISON(ceq, EQ_EXPR, EQ_EXPR, false)
#undef TEMPLATE_PARSER_EMIT_BINARY_NUMERIC_COMPARISON

static void
parser_emit_br (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  const unsigned char *target_ip = ip + offset;
  tree label_decl_tree = cil_labels_set_get_label (labels, target_ip);
  tree goto_expr_tree = build1 (GOTO_EXPR, void_type_node, label_decl_tree);
  cil_bindings_output_statements (goto_expr_tree);
}

static void
parser_emit_brfalse (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  const unsigned char *target_ip = ip + offset;
  tree expr_tree = cil_stack_pop (NULL);
  tree condition_value_tree = build2 (EQ_EXPR, integer_type_node, expr_tree, convert (TREE_TYPE (expr_tree), integer_zero_node));
  tree goto_expr_tree = build1 (GOTO_EXPR, void_type_node, cil_labels_set_get_label (labels, target_ip));
  tree cond_expr_tree = build3 (COND_EXPR, void_type_node, condition_value_tree, goto_expr_tree, build_empty_stmt ());
  cil_bindings_output_statements (cond_expr_tree);
}

static void
parser_emit_brtrue (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  const unsigned char *target_ip = ip + offset;
  tree expr_tree = cil_stack_pop (NULL);
  tree condition_value_tree = build2 (NE_EXPR, integer_type_node, expr_tree, convert (TREE_TYPE (expr_tree), integer_zero_node));
  tree goto_expr_tree = build1 (GOTO_EXPR, void_type_node, cil_labels_set_get_label (labels, target_ip));
  tree cond_expr_tree = build3 (COND_EXPR, void_type_node, condition_value_tree, goto_expr_tree, build_empty_stmt ());
  cil_bindings_output_statements (cond_expr_tree);
}

static void
parser_emit_switch (const unsigned char *ip, guint32 n_offsets, gint32 offsets[], CILLabelsMap *labels)
{
  tree value_tree = convert (integer_type_node, cil_stack_pop (NULL));
  tree labels_tree = make_tree_vec (n_offsets + 1);
  guint32 i;
  for (i = 0; i < n_offsets; ++i)
    {
      const unsigned char *target_ip = ip + offsets[i];
      tree label_tree = cil_labels_set_get_label (labels, target_ip);
      tree case_label_tree = build3 (CASE_LABEL_EXPR, void_type_node, build_int_cst (integer_type_node, i), NULL_TREE, label_tree);
      TREE_VEC_ELT (labels_tree, i) = case_label_tree;
    }
  tree default_label_tree = cil_labels_set_get_label (labels, ip);
  tree default_case_label_tree = build3 (CASE_LABEL_EXPR, void_type_node, NULL_TREE, NULL_TREE, default_label_tree);
  TREE_VEC_ELT (labels_tree, n_offsets) = default_case_label_tree;
  tree switch_tree = build3 (SWITCH_EXPR, void_type_node, value_tree, NULL_TREE, labels_tree);
  cil_bindings_output_statements (switch_tree);
}

static void
parser_emit_bge (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  CilStackType opB_type;
  cil_stack_peek (0, &opB_type);
  CilStackType opA_type;
  cil_stack_peek (1, &opA_type);
  if (opA_type == CIL_STYPE_F)
    {
      gcc_assert (opB_type == CIL_STYPE_F);
      parser_emit_clt_un ();
      parser_emit_brfalse (ip, offset, labels);
    }
  else
    {
      parser_emit_clt ();
      parser_emit_brfalse (ip, offset, labels);
    }
}

static void
parser_emit_bge_un (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  CilStackType opB_type;
  cil_stack_peek (0, &opB_type);
  CilStackType opA_type;
  cil_stack_peek (1, &opA_type);
  if (opA_type == CIL_STYPE_F)
    {
      gcc_assert (opB_type == CIL_STYPE_F);
      parser_emit_clt ();
      parser_emit_brfalse (ip, offset, labels);
    }
  else
    {
      parser_emit_clt_un ();
      parser_emit_brfalse (ip, offset, labels);
    }
}

static void
parser_emit_ble (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  CilStackType opB_type;
  cil_stack_peek (0, &opB_type);
  CilStackType opA_type;
  cil_stack_peek (1, &opA_type);
  if (opA_type == CIL_STYPE_F)
    {
      gcc_assert (opB_type == CIL_STYPE_F);
      parser_emit_cgt_un ();
      parser_emit_brfalse (ip, offset, labels);
    }
  else
    {
      parser_emit_cgt ();
      parser_emit_brfalse (ip, offset, labels);
    }
}

static void
parser_emit_ble_un (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  CilStackType opB_type;
  cil_stack_peek (0, &opB_type);
  CilStackType opA_type;
  cil_stack_peek (1, &opA_type);
  if (opA_type == CIL_STYPE_F)
    {
      gcc_assert (opB_type == CIL_STYPE_F);
      parser_emit_cgt ();
      parser_emit_brfalse (ip, offset, labels);
    }
  else
    {
      parser_emit_cgt_un ();
      parser_emit_brfalse (ip, offset, labels);
    }
}

static void
parser_emit_blt (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  parser_emit_clt ();
  parser_emit_brtrue (ip, offset, labels);
}

static void
parser_emit_blt_un (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  parser_emit_clt_un ();
  parser_emit_brtrue (ip, offset, labels);
}

static void
parser_emit_bgt (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  parser_emit_cgt ();
  parser_emit_brtrue (ip, offset, labels);
}

static void
parser_emit_bgt_un (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  parser_emit_cgt_un ();
  parser_emit_brtrue (ip, offset, labels);
}

static void
parser_emit_bne_un (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  parser_emit_ceq ();
  parser_emit_brfalse (ip, offset, labels);
}

static void
parser_emit_beq (const unsigned char *ip, gint32 offset, CILLabelsMap *labels)
{
  parser_emit_ceq ();
  parser_emit_brtrue (ip, offset, labels);
}

static void
parser_emit_initblk (void)
{
  /* TODO: no advantage is taken of the fact that the address is aligned (unless the unaligned prefix has been used) */
  tree size_tree = convert (cil_type_for_size (32, true), cil_stack_pop (NULL));
  tree value_tree = convert (cil_type_for_size (8, true), cil_stack_pop (NULL));
  tree addr_tree = convert (build_pointer_type (void_type_node), cil_stack_pop (NULL));

  if (parser_current_prefix.bolatile)
    {
      addr_tree = parser_build_volatile_pointer_tree (addr_tree);
    }

  tree memset_arglist = tree_cons (NULL_TREE, size_tree, NULL_TREE);
  memset_arglist = tree_cons (NULL_TREE, value_tree, memset_arglist);
  memset_arglist = tree_cons (NULL_TREE, addr_tree, memset_arglist);
  tree call_memset_tree = build_function_call_expr (built_in_decls[BUILT_IN_MEMSET], memset_arglist);
  cil_bindings_output_statements (call_memset_tree);
}

static void
parser_emit_cpblk (void)
{
  /* TODO: no advantage is taken of the fact that the address is aligned (unless the unaligned prefix has been used) */
  tree size_tree = convert (cil_type_for_size (32, true), cil_stack_pop (NULL));
  tree srcaddr_tree = convert (build_pointer_type (void_type_node), cil_stack_pop (NULL));
  tree destaddr_tree = convert (build_pointer_type (void_type_node), cil_stack_pop (NULL));

  if (parser_current_prefix.bolatile)
    {
      srcaddr_tree = parser_build_volatile_pointer_tree (srcaddr_tree);
      destaddr_tree = parser_build_volatile_pointer_tree (destaddr_tree);
    }

  tree memcpy_arglist = tree_cons (NULL_TREE, size_tree, NULL_TREE);
  memcpy_arglist = tree_cons (NULL_TREE, srcaddr_tree, memcpy_arglist);
  memcpy_arglist = tree_cons (NULL_TREE, destaddr_tree, memcpy_arglist);
  tree call_memcpy_tree = build_function_call_expr (built_in_decls[BUILT_IN_MEMCPY], memcpy_arglist);
  cil_bindings_output_statements (call_memcpy_tree);
}

static void
parser_emit_localloc (void)
{
  tree size_tree = convert (cil_type_for_size (32, true), cil_stack_pop (NULL));
  tree alloca_arglist = tree_cons (NULL_TREE, size_tree, NULL_TREE);
  tree call_alloca_tree = build_function_call_expr (built_in_decls[BUILT_IN_ALLOCA], alloca_arglist);
  cil_stack_push (cil_bindings_output_statements_and_create_temp (call_alloca_tree), CIL_STYPE_NINT);
}

static CilStackType
parser_get_shift_operations_type (CilStackType op_type, CilStackType shift_type)
{
  if (op_type == CIL_STYPE_ERROR || shift_type == CIL_STYPE_ERROR)
    {
      return CIL_STYPE_ERROR;
    }
  else
    {
      gcc_assert (op_type < CIL_STYPE_ERROR);
      gcc_assert (shift_type < CIL_STYPE_ERROR);
      const CilStackType table[6][6] = {
        { CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_INT32, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_INT64, CIL_STYPE_ERROR, CIL_STYPE_INT64, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_NINT,  CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
        { CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR, CIL_STYPE_ERROR },
      };
      return table[op_type][shift_type];
    }
}

#define TEMPLATE_PARSER_EMIT_SHIFT_OPERATION(name,tree_code,unsignedp) \
static void \
parser_emit_##name (void) \
{ \
  CilStackType shiftamount_type; \
  tree shiftamount_tree = cil_stack_pop (&shiftamount_type); \
  CilStackType value_type; \
  tree value_tree = cil_stack_pop (&value_type); \
  CilStackType result_type = parser_get_shift_operations_type (value_type, shiftamount_type); \
  if (result_type == CIL_STYPE_ERROR) \
    { \
      error ("Wrong operand types for "#name); \
    } \
  tree result_type_tree = cil_stack_get_tree_type_for_cil_stack_type (result_type); \
  tree op_type_tree = unsignedp ? cil_unsigned_type (TREE_TYPE (value_tree)) : cil_signed_type (TREE_TYPE (value_tree)); \
  tree converted_value_tree = convert (op_type_tree, value_tree); \
  tree exp_tree = build2 (tree_code, op_type_tree, converted_value_tree, shiftamount_tree); \
  tree converted_exp_tree = convert (result_type_tree, exp_tree); \
  cil_stack_push (converted_exp_tree, result_type); \
}
TEMPLATE_PARSER_EMIT_SHIFT_OPERATION(shl, LSHIFT_EXPR, true)
TEMPLATE_PARSER_EMIT_SHIFT_OPERATION(shr_un, RSHIFT_EXPR, true)
TEMPLATE_PARSER_EMIT_SHIFT_OPERATION(shr, RSHIFT_EXPR, false)

static void
parser_set_location_from_ip (MonoMethod *m, const unsigned char *ip)
{
  unsigned int offset = ip - mono_method_header_get_code (mono_method_get_header (m), NULL, NULL);
  guint32 token = mono_method_get_token (m);
  unsigned int fake_line = (token << 16) + (offset & 0xffff);
#ifdef USE_MAPPED_LOCATION
  input_location = fake_line;
#else
  input_line = fake_line;
#endif
}


static CILLabelsMap *
preparse_method_labels(unsigned const char *code, unsigned const char *code_end, CILLabelsMap *labels)
{
  unsigned const char *ip = code;
  while (ip < code_end)
    {
      MonoOpcodeEnum opcode = mono_opcode_value (&ip, code_end);
      ++ip;

      gint64 arg_int64;
      gint32 arg_int32;
      gint16 arg_int16;
      gint8 arg_int8;
      guint32 arg_token = 0;
#define read_arg(a) a = *((typeof (a) *) ip); ip += sizeof (a);
      switch (mono_opcodes [opcode].argument)
        {
        case MonoInlineNone:
          break;
        case MonoInlineType:
        case MonoInlineField:
        case MonoInlineMethod:
        case MonoInlineTok:
        case MonoInlineSig:
          read_arg (arg_token);
          break;
        case MonoInlineString:
          /* TODO */
          ip += 4;
          break;
        case MonoInlineVar:
          read_arg (arg_int16);
          break;
        case MonoShortInlineVar:
          read_arg (arg_int8);
          break;
        case MonoInlineBrTarget:
          read_arg (arg_int32);
	  /* set label */
	  cil_labels_set_get_label (labels, ip + arg_int32);
          break;
        case MonoShortInlineBrTarget:
          read_arg (arg_int8);
	  /* set label */
	  cil_labels_set_get_label (labels, ip + ((gint32) arg_int8));
          break;
        case MonoInlineSwitch:
	  {
	    read_arg (arg_int32);
	    const gint32 sval = arg_int32;
	    gint32 i;
	    for (i = 0; i < sval; ++i)
	      {
		read_arg (arg_int32);
		/* set the label */
		cil_labels_set_get_label (labels, ip + arg_int32);
	      }
	    break;
	  }
        case MonoInlineR:
          read_arg (arg_int64);
          break;
        case MonoShortInlineR:
          read_arg (arg_int32);
          break;
        case MonoInlineI:
          read_arg (arg_int32);
          break;
        case MonoShortInlineI:
          read_arg (arg_int8);
          break;
        case MonoInlineI8:
          read_arg (arg_int64);
          break;

        default:
          g_assert_not_reached ();
	}
    }

  return labels;
}

static void
parse_method_code (MonoMethod *method)
{
  MonoMethodHeader *header = mono_method_get_header (method);
  guint32 code_size;
  guint32 max_stack;
  const unsigned char *const code = mono_method_header_get_code (header, &code_size, &max_stack);

  gcc_assert (current_function_decl);

  gcc_assert (mono_method_header_get_num_clauses (header) == 0);

  cil_stack_init ();

  parser_current_prefix = cil_opcode_prefix_empty;
  const unsigned char *ip = code;
  const unsigned char *const code_end = code + code_size;

  CILLabelsMap *labels = cil_labels_set_new ();
  preparse_method_labels (code, code_end, labels);

  while (ip < code_end)
    {
      parser_set_location_from_ip (method, ip);

      tree label_decl_tree = cil_labels_get_label (labels, ip);
      if (label_decl_tree)
	{
	  tree label_expr_tree = build1 (LABEL_EXPR, void_type_node, label_decl_tree);
	  cil_bindings_output_statements (label_expr_tree);
	}

      MonoOpcodeEnum opcode = mono_opcode_value (&ip, code_end);
      ++ip;

      gint64 arg_int64;
      gint32 arg_int32;
      guint32 arg_uint32;
      guint16 arg_uint16;
      gint8 arg_int8;
      guint8 arg_uint8;
      guint32 arg_token;
#define read_arg(a) a = *((typeof (a) *) ip); ip += sizeof (a); /* TODO: endiannes */
      switch (opcode) {
      case MONO_CEE_UNALIGNED_:
        read_arg (arg_uint8);
        parser_current_prefix.unaligned = true;
        parser_current_prefix.unaligned_value = arg_uint8;
        break;
      case MONO_CEE_VOLATILE_:
        parser_current_prefix.bolatile = true;
        break;
      case MONO_CEE_LDC_I4:
        read_arg (arg_int32);
        parser_emit_ldc_i4 (arg_int32);
        break;
      case MONO_CEE_LDC_I4_S:
        read_arg (arg_int8);
        parser_emit_ldc_i4 (arg_int8);
        break;
      case MONO_CEE_LDC_I4_M1:
        parser_emit_ldc_i4 (-1);
        break;
      case MONO_CEE_LDC_I4_0:
        parser_emit_ldc_i4 (0);
        break;
      case MONO_CEE_LDC_I4_1:
        parser_emit_ldc_i4 (1);
        break;
      case MONO_CEE_LDC_I4_2:
        parser_emit_ldc_i4 (2);
        break;
      case MONO_CEE_LDC_I4_3:
        parser_emit_ldc_i4 (3);
        break;
      case MONO_CEE_LDC_I4_4:
        parser_emit_ldc_i4 (4);
        break;
      case MONO_CEE_LDC_I4_5:
        parser_emit_ldc_i4 (5);
        break;
      case MONO_CEE_LDC_I4_6:
        parser_emit_ldc_i4 (6);
        break;
      case MONO_CEE_LDC_I4_7:
        parser_emit_ldc_i4 (7);
        break;
      case MONO_CEE_LDC_I4_8:
        parser_emit_ldc_i4 (8);
        break;
      case MONO_CEE_LDC_R4:
        read_arg (arg_int32);
        parser_emit_ldc_r4 (arg_int32);
        break;
      case MONO_CEE_LDC_R8:
        read_arg (arg_int64);
        parser_emit_ldc_r8 (arg_int64);
        break;
      case MONO_CEE_LDC_I8:
        read_arg (arg_int64);
        parser_emit_ldc_i8 (arg_int64);
        break;
      case MONO_CEE_ADD:
        parser_emit_add ();
        break;
      case MONO_CEE_SUB:
        parser_emit_sub ();
        break;
      case MONO_CEE_MUL:
        parser_emit_mul ();
        break;
      case MONO_CEE_DIV:
        parser_emit_div ();
        break;
      case MONO_CEE_DIV_UN:
        parser_emit_div_un ();
        break;
      case MONO_CEE_REM:
        parser_emit_rem ();
        break;
      case MONO_CEE_REM_UN:
        parser_emit_rem_un ();
        break;
      case MONO_CEE_RET:
        parser_emit_ret (method);
        break;
      case MONO_CEE_STLOC_0:
        parser_emit_stloc (0);
        break;
      case MONO_CEE_STLOC_1:
        parser_emit_stloc (1);
        break;
      case MONO_CEE_STLOC_2:
        parser_emit_stloc (2);
        break;
      case MONO_CEE_STLOC_3:
        parser_emit_stloc (3);
        break;
      case MONO_CEE_STLOC_S:
        read_arg (arg_uint8);
        parser_emit_stloc (arg_uint8);
        break;
      case MONO_CEE_STLOC:
        read_arg (arg_uint16);
        parser_emit_stloc (arg_uint16);
        break;
      case MONO_CEE_LDLOC_0:
        parser_emit_ldloc (0);
        break;
      case MONO_CEE_LDLOC_1:
        parser_emit_ldloc (1);
        break;
      case MONO_CEE_LDLOC_2:
        parser_emit_ldloc (2);
        break;
      case MONO_CEE_LDLOC_3:
        parser_emit_ldloc (3);
        break;
      case MONO_CEE_LDLOC_S:
        read_arg (arg_uint8);
        parser_emit_ldloc (arg_uint8);
        break;
      case MONO_CEE_LDLOC:
        read_arg (arg_uint16);
        parser_emit_ldloc (arg_uint16);
        break;
      case MONO_CEE_LDLOCA_S:
        read_arg (arg_uint8);
        parser_emit_ldloca (arg_uint8);
        break;
      case MONO_CEE_LDLOCA:
        read_arg (arg_uint16);
        parser_emit_ldloca (arg_uint16);
        break;
      case MONO_CEE_LDFTN:
        read_arg (arg_token);
        parser_emit_ldftn (method, arg_token);
        break;
      case MONO_CEE_CALL:
        read_arg (arg_token);
        parser_emit_call (method, arg_token);
        break;
      case MONO_CEE_CALLI:
        read_arg (arg_token);
        parser_emit_calli (method, arg_token);
        break;
      case MONO_CEE_STARG:
        read_arg (arg_uint16);
        parser_emit_starg (arg_uint16);
        break;
      case MONO_CEE_STARG_S:
        read_arg (arg_uint8);
        parser_emit_starg (arg_uint8);
        break;
      case MONO_CEE_LDARG:
        read_arg (arg_uint16);
        parser_emit_ldarg (arg_uint16);
        break;
      case MONO_CEE_LDARG_S:
        read_arg (arg_uint8);
        parser_emit_ldarg (arg_uint8);
        break;
      case MONO_CEE_LDARG_0:
        parser_emit_ldarg (0);
        break;
      case MONO_CEE_LDARG_1:
        parser_emit_ldarg (1);
        break;
      case MONO_CEE_LDARG_2:
        parser_emit_ldarg (2);
        break;
      case MONO_CEE_LDARG_3:
        parser_emit_ldarg (3);
        break;
      case MONO_CEE_LDARGA:
        read_arg (arg_uint16);
        parser_emit_ldarga (arg_uint16);
        break;
      case MONO_CEE_LDARGA_S:
        read_arg (arg_uint8);
        parser_emit_ldarga (arg_uint8);
        break;
      case MONO_CEE_LDSFLD:
        read_arg (arg_token);
        parser_emit_ldsfld (method, arg_token);
        break;
      case MONO_CEE_LDSFLDA:
        read_arg (arg_token);
        parser_emit_ldsflda (method, arg_token);
        break;
      case MONO_CEE_STSFLD:
        read_arg (arg_token);
        parser_emit_stsfld (method, arg_token);
        break;
      case MONO_CEE_STFLD:
        read_arg (arg_token);
        parser_emit_stfld (method, arg_token);
        break;
      case MONO_CEE_LDFLD:
        read_arg (arg_token);
        parser_emit_ldfld (method, arg_token);
        break;
      case MONO_CEE_LDFLDA:
        read_arg (arg_token);
        parser_emit_ldflda (method, arg_token);
        break;
      case MONO_CEE_LDOBJ:
        read_arg (arg_token);
        parser_emit_ldobj (method, arg_token);
        break;
      case MONO_CEE_STOBJ:
        read_arg (arg_token);
        parser_emit_stobj (method, arg_token);
        break;
      case MONO_CEE_CONV_I:
        parser_emit_conv_i ();
        break;
      case MONO_CEE_CONV_I1:
        parser_emit_conv_i1 ();
        break;
      case MONO_CEE_CONV_U1:
        parser_emit_conv_u1 ();
        break;
      case MONO_CEE_CONV_I2:
        parser_emit_conv_i2 ();
        break;
      case MONO_CEE_CONV_U2:
        parser_emit_conv_u2 ();
        break;
      case MONO_CEE_CONV_I4:
        parser_emit_conv_i4 ();
        break;
      case MONO_CEE_CONV_U4:
        parser_emit_conv_u4 ();
        break;
      case MONO_CEE_CONV_U8:
        parser_emit_conv_u8 ();
        break;
      case MONO_CEE_CONV_I8:
        parser_emit_conv_i8 ();
        break;
      case MONO_CEE_CONV_R8:
        parser_emit_conv_r8 ();
        break;
      case MONO_CEE_CONV_R4:
        parser_emit_conv_r4 ();
        break;
      case MONO_CEE_CONV_R_UN:
        parser_emit_conv_r_un ();
        break;
      case MONO_CEE_LDIND_I1:
        parser_emit_ldind_i1 ();
        break;
      case MONO_CEE_LDIND_U1:
        parser_emit_ldind_u1 ();
        break;
      case MONO_CEE_LDIND_I2:
        parser_emit_ldind_i2 ();
        break;
      case MONO_CEE_LDIND_U2:
        parser_emit_ldind_u2 ();
        break;
      case MONO_CEE_LDIND_I4:
        parser_emit_ldind_i4 ();
        break;
      case MONO_CEE_LDIND_U4:
        parser_emit_ldind_u4 ();
        break;
      case MONO_CEE_LDIND_I:
        parser_emit_ldind_i ();
        break;
      case MONO_CEE_LDIND_R4:
        parser_emit_ldind_r4 ();
        break;
      case MONO_CEE_LDIND_R8:
        parser_emit_ldind_r8 ();
        break;
      case MONO_CEE_STIND_I4:
        parser_emit_stind_i4 ();
        break;
      case MONO_CEE_STIND_I2:
        parser_emit_stind_i2 ();
        break;
      case MONO_CEE_STIND_I1:
        parser_emit_stind_i1 ();
        break;
      case MONO_CEE_STIND_I:
        parser_emit_stind_i ();
        break;
      case MONO_CEE_STIND_R8:
        parser_emit_stind_r8 ();
        break;
      case MONO_CEE_STIND_R4:
        parser_emit_stind_r4 ();
        break;
      case MONO_CEE_POP:
        parser_emit_pop ();
        break;
      case MONO_CEE_DUP:
        parser_emit_dup ();
        break;
      case MONO_CEE_CLT:
        parser_emit_clt ();
        break;
      case MONO_CEE_CLT_UN:
        parser_emit_clt_un ();
        break;
      case MONO_CEE_CGT:
        parser_emit_cgt ();
        break;
      case MONO_CEE_CGT_UN:
        parser_emit_cgt_un ();
        break;
      case MONO_CEE_CEQ:
        parser_emit_ceq ();
        break;
      case MONO_CEE_BR:
        read_arg (arg_int32);
        parser_emit_br (ip, arg_int32, labels);
        break;
      case MONO_CEE_BR_S:
        read_arg (arg_int8);
        parser_emit_br (ip, arg_int8, labels);
        break;
      case MONO_CEE_BGE_S:
        read_arg (arg_int8);
        parser_emit_bge (ip, arg_int8, labels);
        break;
      case MONO_CEE_BGE:
        read_arg (arg_int32);
        parser_emit_bge (ip, arg_int32, labels);
        break;
      case MONO_CEE_BGE_UN:
        read_arg (arg_int32);
        parser_emit_bge_un (ip, arg_int32, labels);
        break;
      case MONO_CEE_BGE_UN_S:
        read_arg (arg_int8);
        parser_emit_bge_un (ip, arg_int8, labels);
        break;
      case MONO_CEE_BLE_S:
        read_arg (arg_int8);
        parser_emit_ble (ip, arg_int8, labels);
        break;
      case MONO_CEE_BLE:
        read_arg (arg_int32);
        parser_emit_ble (ip, arg_int32, labels);
        break;
      case MONO_CEE_BLE_UN_S:
        read_arg (arg_int8);
        parser_emit_ble_un (ip, arg_int8, labels);
        break;
      case MONO_CEE_BLE_UN:
        read_arg (arg_int32);
        parser_emit_ble_un (ip, arg_int32, labels);
        break;
      case MONO_CEE_BLT_S:
        read_arg (arg_int8);
        parser_emit_blt (ip, arg_int8, labels);
        break;
      case MONO_CEE_BLT:
        read_arg (arg_int32);
        parser_emit_blt (ip, arg_int32, labels);
        break;
      case MONO_CEE_BLT_UN_S:
        read_arg (arg_int8);
        parser_emit_blt_un (ip, arg_int8, labels);
        break;
      case MONO_CEE_BLT_UN:
        read_arg (arg_int32);
        parser_emit_blt_un (ip, arg_int32, labels);
        break;
      case MONO_CEE_BGT_S:
        read_arg (arg_int8);
        parser_emit_bgt (ip, arg_int8, labels);
        break;
      case MONO_CEE_BGT:
        read_arg (arg_int32);
        parser_emit_bgt (ip, arg_int32, labels);
        break;
      case MONO_CEE_BGT_UN_S:
        read_arg (arg_int8);
        parser_emit_bgt_un (ip, arg_int8, labels);
        break;
      case MONO_CEE_BGT_UN:
        read_arg (arg_int32);
        parser_emit_bgt_un (ip, arg_int32, labels);
        break;
      case MONO_CEE_BNE_UN_S:
        read_arg (arg_int8);
        parser_emit_bne_un (ip, arg_int8, labels);
        break;
      case MONO_CEE_BNE_UN:
        read_arg (arg_int32);
        parser_emit_bne_un (ip, arg_int32, labels);
        break;
      case MONO_CEE_BEQ_S:
        read_arg (arg_int8);
        parser_emit_beq (ip, arg_int8, labels);
        break;
      case MONO_CEE_BEQ:
        read_arg (arg_int32);
        parser_emit_beq (ip, arg_int32, labels);
        break;
      case MONO_CEE_BRFALSE:
        read_arg (arg_int32);
        parser_emit_brfalse (ip, arg_int32, labels);
        break;
      case MONO_CEE_BRFALSE_S:
        read_arg (arg_int8);
        parser_emit_brfalse (ip, arg_int8, labels);
        break;
      case MONO_CEE_BRTRUE:
        read_arg (arg_int32);
        parser_emit_brtrue (ip, arg_int32, labels);
        break;
      case MONO_CEE_BRTRUE_S:
        read_arg (arg_int8);
        parser_emit_brtrue (ip, arg_int8, labels);
        break;
      case MONO_CEE_INITBLK:
        parser_emit_initblk ();
        break;
      case MONO_CEE_CPBLK:
        parser_emit_cpblk ();
        break;
      case MONO_CEE_LOCALLOC:
        parser_emit_localloc ();
        break;
      case MONO_CEE_SHL:
        parser_emit_shl ();
        break;
      case MONO_CEE_SHR_UN:
        parser_emit_shr_un ();
        break;
      case MONO_CEE_SHR:
        parser_emit_shr ();
        break;
      case MONO_CEE_AND:
        parser_emit_and ();
        break;
      case MONO_CEE_OR:
        parser_emit_or ();
        break;
      case MONO_CEE_XOR:
        parser_emit_xor ();
        break;
      case MONO_CEE_NEG:
        parser_emit_neg ();
        break;
      case MONO_CEE_NOT:
        parser_emit_not ();
        break;
      case MONO_CEE_SWITCH: {
        read_arg (arg_uint32);
        const guint32 n_offsets = arg_uint32;
        gint32 offsets[n_offsets];
        guint32 i;
        for (i = 0; i < n_offsets; ++i) {
          read_arg (arg_int32);
          offsets[i] = arg_int32;
        }
        parser_emit_switch (ip, n_offsets, offsets, labels);
      }
        break;
      default:
        gcc_unreachable ();
      }
#undef read_arg

      /* reset the prefix unless current opcode is a prefix itself */
      switch (opcode) {
      case MONO_CEE_UNALIGNED_:
      case MONO_CEE_VOLATILE_:
      case MONO_CEE_TAIL_:
      case MONO_CEE_CONSTRAINED_:
      case MONO_CEE_NO_:
      case MONO_CEE_READONLY_:
        break;
      default:
        parser_current_prefix = cil_opcode_prefix_empty;
      }
    }
  cil_labels_set_free (labels);
}

static GSList *
parser_get_refertenced_types_in_signature (MonoMethodSignature *signature)
{
  GSList *ret = NULL;
  gpointer iter = NULL;
  MonoType *param_type;
  while ((param_type = mono_signature_get_params (signature, &iter)) != NULL)
    {
      ret = g_slist_prepend (ret, param_type);
    }
  MonoType *ret_type = mono_signature_get_return_type (signature);
  ret = g_slist_prepend (ret, ret_type);
  return ret;
}

/* Preparses a method, checking whether it uses any unsupported
   feature and building lists for called methods and referenced
   types. Returns true if the method can be compiled. */
static bool
parser_preparse_method (MonoMethod *method, GSList **called_methods, GSList **referenced_types, MonoOpcodeEnum *reason)
{
  MonoMethodSignature *signature = mono_method_signature (method);
  if (mono_signature_is_instance (signature))
    {
      *called_methods = NULL;
      *referenced_types = NULL;
      return false;
    }

  *called_methods = NULL;
  *referenced_types = parser_get_refertenced_types_in_signature (signature);

  if (parser_get_method_mode (method) != GCC_CIL_METHOD_MODE_COMPILE)
    {
      return true;
    }

  MonoImage *image = mono_class_get_image (mono_method_get_class (method));
  MonoMethodHeader *header = mono_method_get_header (method);
  if (!header)
    {
      /* native method or something */
      gcc_unreachable ();
    }
  guint32 code_size;
  guint32 max_stack;
  const unsigned char *code = mono_method_header_get_code (header, &code_size, &max_stack);
  unsigned const char *const code_end = code + code_size;

  /* locals */
  guint32 num_locals;
  gboolean init_locals;
  MonoType **locals = mono_method_header_get_locals (header, &num_locals, &init_locals);
  guint32 i;
  for (i = 0; i < num_locals; ++i)
    {
      *referenced_types = g_slist_prepend (*referenced_types, locals[i]);
    }

  unsigned const char *ip = code;
  while (ip < code_end)
    {
      MonoOpcodeEnum opcode = mono_opcode_value (&ip, code_end);
      ++ip;

      gint64 arg_int64;
      gint32 arg_int32;
      gint16 arg_int16;
      gint8 arg_int8;
      guint32 arg_token = 0;
#define read_arg(a) a = *((typeof (a) *) ip); ip += sizeof (a);
      switch (mono_opcodes [opcode].argument)
        {
        case MonoInlineNone:
          break;
        case MonoInlineType:
        case MonoInlineField:
        case MonoInlineMethod:
        case MonoInlineTok:
        case MonoInlineSig:
          read_arg (arg_token);
          break;
        case MonoInlineString:
          /* TODO */
          ip += 4;
          break;
        case MonoInlineVar:
          read_arg (arg_int16);
          break;
        case MonoShortInlineVar:
          read_arg (arg_int8);
          break;
        case MonoInlineBrTarget:
          read_arg (arg_int32);
          break;
        case MonoShortInlineBrTarget:
          read_arg (arg_int8);
          break;
        case MonoInlineSwitch: {
          read_arg (arg_int32);
          const gint32 sval = arg_int32;
          gint32 i;
          for (i = 0; i < sval; ++i) {
            read_arg (arg_int32);
          }
          break;
        }
        case MonoInlineR:
          read_arg (arg_int64);
          break;
        case MonoShortInlineR:
          read_arg (arg_int32);
          break;
        case MonoInlineI:
          read_arg (arg_int32);
          break;
        case MonoShortInlineI:
          read_arg (arg_int8);
          break;
        case MonoInlineI8:
          read_arg (arg_int64);
          break;
        default:
          g_assert_not_reached ();
        }
#undef read_arg

      switch (opcode) {
      case MONO_CEE_UNALIGNED_:
      case MONO_CEE_VOLATILE_:
      case MONO_CEE_LDC_I4:
      case MONO_CEE_LDC_I4_S:
      case MONO_CEE_LDC_I4_M1:
      case MONO_CEE_LDC_I4_0:
      case MONO_CEE_LDC_I4_1:
      case MONO_CEE_LDC_I4_2:
      case MONO_CEE_LDC_I4_3:
      case MONO_CEE_LDC_I4_4:
      case MONO_CEE_LDC_I4_5:
      case MONO_CEE_LDC_I4_6:
      case MONO_CEE_LDC_I4_7:
      case MONO_CEE_LDC_I4_8:
      case MONO_CEE_LDC_I8:
      case MONO_CEE_LDC_R4:
      case MONO_CEE_LDC_R8:
      case MONO_CEE_ADD:
      case MONO_CEE_SUB:
      case MONO_CEE_MUL:
      case MONO_CEE_DIV:
      case MONO_CEE_DIV_UN:
      case MONO_CEE_REM:
      case MONO_CEE_REM_UN:
      case MONO_CEE_RET:
      case MONO_CEE_STLOC:
      case MONO_CEE_STLOC_S:
      case MONO_CEE_STLOC_0:
      case MONO_CEE_STLOC_1:
      case MONO_CEE_STLOC_2:
      case MONO_CEE_STLOC_3:
      case MONO_CEE_LDLOC:
      case MONO_CEE_LDLOC_S:
      case MONO_CEE_LDLOC_0:
      case MONO_CEE_LDLOC_1:
      case MONO_CEE_LDLOC_2:
      case MONO_CEE_LDLOC_3:
      case MONO_CEE_LDLOCA:
      case MONO_CEE_LDLOCA_S:
      case MONO_CEE_LDARG:
      case MONO_CEE_LDARG_S:
      case MONO_CEE_LDARG_0:
      case MONO_CEE_LDARG_1:
      case MONO_CEE_LDARG_2:
      case MONO_CEE_LDARG_3:
      case MONO_CEE_LDARGA:
      case MONO_CEE_LDARGA_S:
      case MONO_CEE_STARG:
      case MONO_CEE_STARG_S:
      case MONO_CEE_CONV_I:
      case MONO_CEE_CONV_I1:
      case MONO_CEE_CONV_U1:
      case MONO_CEE_CONV_I2:
      case MONO_CEE_CONV_U2:
      case MONO_CEE_CONV_I4:
      case MONO_CEE_CONV_U4:
      case MONO_CEE_CONV_I8:
      case MONO_CEE_CONV_U8:
      case MONO_CEE_CONV_R4:
      case MONO_CEE_CONV_R8:
      case MONO_CEE_CONV_R_UN:
      case MONO_CEE_LDIND_I1:
      case MONO_CEE_LDIND_U1:
      case MONO_CEE_LDIND_I2:
      case MONO_CEE_LDIND_U2:
      case MONO_CEE_LDIND_I4:
      case MONO_CEE_LDIND_U4:
      case MONO_CEE_LDIND_I:
      case MONO_CEE_LDIND_R4:
      case MONO_CEE_LDIND_R8:
      case MONO_CEE_STIND_I4:
      case MONO_CEE_STIND_I2:
      case MONO_CEE_STIND_I1:
      case MONO_CEE_STIND_I:
      case MONO_CEE_STIND_R4:
      case MONO_CEE_STIND_R8:
      case MONO_CEE_POP:
      case MONO_CEE_DUP:
      case MONO_CEE_CLT:
      case MONO_CEE_CLT_UN:
      case MONO_CEE_CGT:
      case MONO_CEE_CGT_UN:
      case MONO_CEE_CEQ:
      case MONO_CEE_BR:
      case MONO_CEE_BR_S:
      case MONO_CEE_BRFALSE:
      case MONO_CEE_BRFALSE_S:
      case MONO_CEE_BRTRUE:
      case MONO_CEE_BRTRUE_S:
      case MONO_CEE_BGE:
      case MONO_CEE_BGE_S:
      case MONO_CEE_BGE_UN:
      case MONO_CEE_BGE_UN_S:
      case MONO_CEE_BLE:
      case MONO_CEE_BLE_S:
      case MONO_CEE_BLE_UN:
      case MONO_CEE_BLE_UN_S:
      case MONO_CEE_BLT:
      case MONO_CEE_BLT_S:
      case MONO_CEE_BLT_UN:
      case MONO_CEE_BLT_UN_S:
      case MONO_CEE_BGT:
      case MONO_CEE_BGT_S:
      case MONO_CEE_BGT_UN:
      case MONO_CEE_BGT_UN_S:
      case MONO_CEE_BNE_UN:
      case MONO_CEE_BNE_UN_S:
      case MONO_CEE_BEQ:
      case MONO_CEE_BEQ_S:
      case MONO_CEE_INITBLK:
      case MONO_CEE_CPBLK:
      case MONO_CEE_LOCALLOC:
      case MONO_CEE_SHL:
      case MONO_CEE_SHR_UN:
      case MONO_CEE_SHR:
      case MONO_CEE_AND:
      case MONO_CEE_OR:
      case MONO_CEE_XOR:
      case MONO_CEE_NEG:
      case MONO_CEE_NOT:
      case MONO_CEE_SWITCH:
        break;
      case MONO_CEE_CALLI: {
        MonoMethodSignature *signature = mono_metadata_parse_signature (image, arg_token);
        GSList *l = parser_get_refertenced_types_in_signature (signature);
        *referenced_types = g_slist_concat (l, *referenced_types);
      }
        break;
      case MONO_CEE_LDFTN:
      case MONO_CEE_CALL: {
        MonoMethod *called = mono_get_method (image, arg_token, NULL);
        *called_methods = g_slist_prepend (*called_methods, called);
        MonoMethodSignature *signature = mono_method_signature (called);
        GSList *l = parser_get_refertenced_types_in_signature (signature);
        *referenced_types = g_slist_concat (l, *referenced_types);
      }
        break;
      case MONO_CEE_LDFLD:
      case MONO_CEE_LDFLDA:
      case MONO_CEE_STFLD:
      case MONO_CEE_LDSFLD:
      case MONO_CEE_STSFLD:
      case MONO_CEE_LDSFLDA: {
        MonoClass *klass = NULL;
        MonoClassField *field = mono_field_from_token (image, arg_token, &klass, NULL);
        *referenced_types = g_slist_prepend (*referenced_types, mono_class_get_type (klass));
        *referenced_types = g_slist_prepend (*referenced_types, mono_field_get_type (field));
      }
        break;
      case MONO_CEE_STOBJ:
      case MONO_CEE_LDOBJ: {
        MonoClass *klass = mono_class_get (image, arg_token);
        *referenced_types = g_slist_prepend (*referenced_types, mono_class_get_type (klass));
      }
        break;
      default:
#if 0
        fprintf (stderr, "Unhandled opcode: %s\n", mono_opcode_name (opcode));
        fprintf (stderr, "Method: %s\n", mono_method_full_name (method, true));
        fprintf (stderr, "%s", mono_disasm_code (NULL, method, code, code_end));
#endif
        g_slist_free (*called_methods);
        *called_methods = NULL;
        g_slist_free (*referenced_types);
        *referenced_types = NULL;
        if (reason)
          {
            *reason = opcode;
          }
        return false;
      }
    }
  return true;
}

static void
parser_cleanup_asm_identifier (char *s)
{
  /* replace problematic characters with _ */
  char *p;
  for (p = s; *p; ++p)
    {
      if (strchr ("<>[]*.!?=+-/*&%$^()@#", *p))
        {
          *p = '_';
        }
    }
}

static char *
parser_get_class_mangled_name (MonoClass *klass, const char *suffix)
{
  /* FIXME: this is a stub implementation */
  GString *ret = g_string_new ("__");
  ret = g_string_append (ret, mono_class_get_namespace (klass));
  ret = g_string_append_c (ret, '_');
  ret = g_string_append (ret, mono_class_get_name (klass));

  parser_cleanup_asm_identifier (ret->str);

  ret = g_string_append (ret, suffix);

  return g_string_free (ret, FALSE);
}

static char *
parser_get_class_static_storage_mangled_name (MonoClass *klass)
{
  return parser_get_class_mangled_name (klass, "_static_data");
}

static char *
parser_get_method_mangled_name (MonoMethod *method)
{
  /* FIXME: this is a stub implementation */
  MonoClass *klass = mono_method_get_class (method);
  GString *ret = g_string_new (mono_class_get_namespace (klass));
  ret = g_string_append_c (ret, '_');
  ret = g_string_append (ret, mono_class_get_name (klass));
  ret = g_string_append_c (ret, '_');
  ret = g_string_append (ret, mono_method_get_name (method));
  MonoMethodSignature *signature = mono_method_signature (method);
  gpointer iter = NULL;
  MonoType *param_type;
  while ((param_type = mono_signature_get_params (signature, &iter)) != NULL)
    {
      char *param_type_name = mono_type_get_name (param_type);
      ret = g_string_append_c (ret, '_');
      ret = g_string_append (ret, param_type_name);
      free (param_type_name);
    }
  ret = g_string_append_c (ret, '_');
  MonoType *ret_type = mono_signature_get_return_type (signature);
  char *ret_type_name = mono_type_get_name (ret_type);
  ret = g_string_append (ret, ret_type_name);
  free (ret_type_name);

  parser_cleanup_asm_identifier (ret->str);

  return g_string_free (ret, FALSE);
}

static void parser_parse_type (MonoType *type);

static void
parser_parse_signature_types (MonoMethodSignature *signature)
{
  gpointer iter = NULL;
  MonoType *param_type;
  while ((param_type = mono_signature_get_params (signature, &iter)) != NULL)
    {
      parser_parse_type (param_type);
    }
  MonoType *ret_type = mono_signature_get_return_type (signature);
  parser_parse_type (ret_type);
}

static void
parse_method_decl (MonoMethod *method)
{
  gcc_assert (g_hash_table_lookup (parsed_methods_decl, method) == NULL);
  const GCCCilMethodMode method_mode = parser_get_method_mode (method);

  MonoMethodSignature *signature = mono_method_signature (method);
  parser_parse_signature_types (signature);
  tree method_type_tree = parser_signature_tree (signature, mono_method_get_class (method));

  tree identifier;
  tree method_decl = NULL_TREE;
  if (method_mode == GCC_CIL_METHOD_MODE_EXTERNAL_STDLIB)
    {
      int i;
      const char *fun_name = mono_method_get_name (method);

      if (fun_name[0] != '_' && fun_name[0] != '?')
        {
          for (i=0; i < END_BUILTINS && method_decl == NULL_TREE; ++i)
            {
              if (built_in_decls[i])
                {
                  const char *builtin_fun_name = IDENTIFIER_POINTER (DECL_NAME (built_in_decls[i]));
                  if (   builtin_fun_name
                      && builtin_fun_name[0] != '_'
                      && !strcmp(fun_name, builtin_fun_name))
                    method_decl = built_in_decls[i];
                }
            }
        }
      if (method_decl != NULL_TREE)
        {
          gcc_assert (!g_hash_table_lookup (parsed_methods_decl, method));
          gcc_assert (!g_hash_table_lookup (parsed_methods_impl, method));
          g_hash_table_insert (parsed_methods_decl, method, method_decl);
          return;
        }
      else
        {
          identifier = get_identifier (fun_name);
        }
    }
  else if (method_mode == GCC_CIL_METHOD_MODE_PINVOKE)
    {
      char *method_mangled_name = parser_get_method_mangled_name (method);
      char *method_pointer_mangled_name = g_strdup_printf ("__pinvoke_ptr_%s", method_mangled_name);
      identifier = get_identifier (method_pointer_mangled_name);
      free (method_pointer_mangled_name);
      free (method_mangled_name);
      method_decl = build_decl (VAR_DECL, identifier, build_pointer_type (method_type_tree));
      TREE_STATIC (method_decl) = 1; /* TODO: check if it is really necessary here, or could be moved to parse_method_impl */
    }
  else
    {
      char *method_mangled_name = parser_get_method_mangled_name (method);
      identifier = get_identifier (method_mangled_name);
      free (method_mangled_name);
    }

  if (method_decl == NULL_TREE)
    {
      method_decl = build_decl (FUNCTION_DECL, identifier, method_type_tree);
      DECL_CONTEXT (method_decl) = NULL_TREE; /* not nested */
      if (gcc_cil_method_mode_is_external (method_mode))
        {
          gcc_assert (method_mode == GCC_CIL_METHOD_MODE_EXTERNAL || method_mode == GCC_CIL_METHOD_MODE_EXTERNAL_STDLIB);
          DECL_EXTERNAL (method_decl) = 1;
          TREE_PUBLIC (method_decl) = 1;
        }
      else
        {
          gcc_assert (method_mode == GCC_CIL_METHOD_MODE_COMPILE);
          DECL_EXTERNAL (method_decl) = 0;
          TREE_PUBLIC (method_decl) = 0;
        }
      TREE_STATIC (method_decl) = 0; /* not yet defined */

      /* Process declaration of function defined elsewhere.  */
      rest_of_decl_compilation (method_decl, 1, 0);
    }

  gcc_assert (!g_hash_table_lookup (parsed_methods_decl, method));
  gcc_assert (!g_hash_table_lookup (parsed_methods_impl, method));
  g_hash_table_insert (parsed_methods_decl, method, method_decl);

  /* This has something (what?) to do with forcing output also. (I love these comments from treelang, which is suposed to be an example of how to do things) */
  /* Ensures that the method decl is not garbage collected */
  cil_bindings_push_decl (method_decl);
}

/* tree node from the blob value */
static tree
get_from_blob(guchar* ptr, MonoType *monotype, guchar** rptr)
{
  tree node;
  int typetype = mono_type_get_type (monotype);

  gint val_int;
  guint val_uint;
  gint64  val_int64;
  guint64 val_uint64;
  gint32  val_int32;
  guint32 val_uint32;
  gint16  val_int16;
  guint16 val_uint16;
  gint8   val_int8;
  guint8  val_uint8;
  float   val_float;
  double  val_double;
  REAL_VALUE_TYPE real_value;
  long *pf;
  size_t len;
#define read_val(val) val = *((typeof(val) *) ptr) /* FIXME big endian */
  switch(typetype)
    {
    case MONO_TYPE_U1:
      read_val(val_uint8);
      node = build_int_cst(cil_type_for_size(8, true), (HOST_WIDE_INT) val_uint8);
      ptr += sizeof(val_uint8);
      break;
    case MONO_TYPE_I1:
      read_val(val_int8);
      node = build_int_cst(cil_type_for_size(8, false), (HOST_WIDE_INT) val_int8);
      ptr += sizeof(val_int8);
      break;
    case MONO_TYPE_U2:
      read_val(val_uint16);
      node = build_int_cst(cil_type_for_size(16, true), (HOST_WIDE_INT) val_uint16);
      ptr += sizeof(val_uint16);
      break;
    case MONO_TYPE_I2:
      read_val(val_int16);
      node = build_int_cst(cil_type_for_size(16, false), (HOST_WIDE_INT) val_int16);
      ptr += sizeof(val_int16);
      break;
    case MONO_TYPE_U4:
      read_val(val_uint32);
      node = build_int_cst(cil_type_for_size(32, true), (HOST_WIDE_INT) val_uint32);
      ptr += sizeof(val_uint32);
      break;
    case MONO_TYPE_I4:
      read_val(val_int32);
      node = build_int_cst(cil_type_for_size(32, false), (HOST_WIDE_INT) val_int32);
      ptr += sizeof(val_int32);
      break;
    case MONO_TYPE_U8:
      read_val(val_uint64);
      node = build_int_cst(cil_type_for_size(64, true), (HOST_WIDE_INT) val_uint64);
      ptr += sizeof(val_int64);
      break;
    case MONO_TYPE_I8:
      read_val(val_int64);
      node = build_int_cst(cil_type_for_size(64, false), (HOST_WIDE_INT) val_int64);
      ptr += sizeof(val_int64);
      break;

    case MONO_TYPE_CHAR:
      read_val(val_int16); /* utf-8, FIXME convert to char ? */
      node = build_int_cst(cil_type_for_size(16, true), (HOST_WIDE_INT) val_int16);
      ptr += sizeof(val_int16);
      break;
    case MONO_TYPE_U:
      read_val(val_uint);
      node = build_int_cst(unsigned_type_node, (HOST_WIDE_INT) val_uint);
      ptr += sizeof(val_uint);
      break;
    case MONO_TYPE_I:
      read_val(val_int);
      node = build_int_cst(integer_type_node, (HOST_WIDE_INT) val_int);
      ptr += sizeof(val_int);
      break;


    case MONO_TYPE_R4:
      read_val(val_float);
      pf = (void *) &val_float;
      real_from_target_fmt(&real_value, pf, &ieee_single_format);
      node = build_real (double_type_node, real_value);
      ptr += sizeof(val_float);
      break;
    case MONO_TYPE_R8:
      read_val(val_double);
      pf = (void *) &val_double;
      real_from_target_fmt(&real_value, pf, &ieee_double_format);
      node = build_real (double_type_node, real_value);
      ptr += sizeof(val_double);
      break;


    case MONO_TYPE_STRING:
      read_val(val_uint8);
      ptr += sizeof(val_uint8);
      len = (int) val_uint8;
      char *val_str = g_strnfill(len+1, '\0');
      memcpy(val_str, ptr, len);
      node = build_string(len, val_str);
      free(val_str);
      ptr += len;
      break;

    case MONO_TYPE_SZARRAY:
      read_val(val_uint32);
      ptr += sizeof(val_uint32);
      len = (typeof(len)) val_uint32;
      MonoType *elt_type = mono_class_get_type(monotype->data.klass);
      tree elts = NULL_TREE;
      guint i;
      for(i = 0; i<len; i++)
	{
	  elts = tree_cons(NULL_TREE, get_from_blob(ptr, elt_type, rptr), elts);
	  ptr = *rptr;
	}
/*       tree index = build_index_type (build_int_cst(sizetype, len-1)); */
/*       tree array = build_array_type (TREE_TYPE(TREE_VALUE(elts)), index); */
      node = build_vector(TREE_TYPE(TREE_VALUE(elts)), nreverse(elts));
      break;

    default:
      fprintf (stderr, "Unhandled type type: 0x%x %s\n", typetype, mono_type_get_name (monotype));
      gcc_unreachable();
    }

  *rptr = ptr;
  return node;

}

static tree
convert_cil_attributes(MonoCustomAttrInfo *custom_attr)
{
  tree attributes = NULL_TREE;
  if (custom_attr)
    {
      int i;
      for (i = 0; i < custom_attr->num_attrs; ++i)
	{
	  /* get purpose */
	  GString * purpose = g_string_new("");
	  const MonoCustomAttrEntry *attr = &custom_attr->attrs[i];
	  MonoClass *klass = mono_method_get_class (attr->ctor);
	  const char *klass_ns = mono_class_get_namespace (klass);
	  const char *klass_name = mono_class_get_name (klass);
	  if( strcmp("",klass_ns) != 0)
	    {
	      g_string_append(purpose, klass_ns);
	      g_string_append(purpose,".");
	    }
	  g_string_append(purpose, klass_name);

	  if (!strcmp (klass_ns, "gcc4net.C_Attributes"))
	    {
	      attributes = tree_cons(get_identifier(purpose->str), NULL_TREE , attributes);
            }
	  else if (!strcmp (klass_ns, "OpenSystem.C"))
	    {
              /* TODO: should check the class assembly and method arguments? */
	      attributes = tree_cons(get_identifier(purpose->str), NULL_TREE , attributes);
            }
#if 0
	  else if (!strcmp (klass_ns, "gcc4net.JitCompilationHints"))
	    {
	      /* get value */
	      MonoMethodSignature *sig = mono_method_signature(attr->ctor);
	      tree value = NULL_TREE;
	      if( mono_signature_get_param_count(sig) > 0)
		{
		  MonoType *t;
		  gpointer iter = NULL;
		  guchar *data = (guchar*) attr->data;
		  guchar *rdata = NULL;
		  short prolog = *((short *) data);
		  data += 2;
		  if (prolog != 1)
		    {
		      fprintf(stderr,"error parsing attribute blob\n");
		      break;
		    }
		  while((t = mono_signature_get_params(sig, &iter)) != NULL)
		    {
		      char *tname = mono_type_get_name(t);
		      tree blobval = get_from_blob(data, t, &rdata);
		      value = tree_cons(get_identifier(tname), blobval, value);
		      data = rdata;
		    }
		  short num_named_params = *((short *) data);
		  data += 2;
		  while(num_named_params > 0)
		    {
		      /* TODO: named paramed */
		      num_named_params--;
		    }
		}
	      attributes = tree_cons(get_identifier(purpose->str), nreverse(value) , attributes);
	    }
#endif
	  g_string_free(purpose,FALSE);
	}
    }
  return attributes;
}

static void
parse_method_impl (MonoMethod *method)
{
  tree method_decl = parser_get_method_tree (method);
  gcc_assert (!g_hash_table_lookup (parsed_methods_impl, method));
  g_hash_table_insert (parsed_methods_impl, method, method_decl);

  /* Output message if not -quiet.  */
  /* announce_function (method_decl); */

  if (gcc_cil_method_mode_is_external (parser_get_method_mode (method)))
    {
      return;
    }
  else if (parser_get_method_mode (method) == GCC_CIL_METHOD_MODE_PINVOKE)
    {
      pinvoke_methods_to_init = g_slist_prepend (pinvoke_methods_to_init, method);
      return;
    }
  gcc_assert (parser_get_method_mode (method) == GCC_CIL_METHOD_MODE_COMPILE);

  MonoMethodHeader *header = mono_method_get_header (method);
  if (header)
    {
      MonoMethodSignature *signature = mono_method_signature (method);

      /* Set current function for error msgs, etc.  */
      current_function_decl = method_decl;
      DECL_INITIAL (method_decl) = error_mark_node; /* TODO: debug */

      /* Create a new level at the start of the function.  */
      cil_bindings_push_level ();

      /* build argument variable decls. (Note: treelang does this before calling rest_of_decl_compilation, and does not call push_decl)  */
      tree args_decl_list = NULL_TREE;
      int argnum = 0;
      if (mono_signature_is_instance (signature) && !mono_signature_explicit_this (signature))
        {
          char argname[128];
          snprintf (argname, sizeof (argname), "arg%d", argnum);
          tree param_type_tree = parser_get_type_tree (mono_class_get_type (mono_method_get_class (method)));
          tree parm_decl = build_decl (PARM_DECL, get_identifier (argname), param_type_tree);
          DECL_ARG_TYPE (parm_decl) = TREE_TYPE (parm_decl); /* Some languages have different nominal and real types.  */
          DECL_CONTEXT (parm_decl) = method_decl;
          cil_bindings_push_decl_arg (argnum, parm_decl);
          args_decl_list = chainon (parm_decl, args_decl_list);
          ++argnum;
        }
      gpointer iter = NULL;
      MonoType *param_type;
      while ((param_type = mono_signature_get_params (signature, &iter)) != NULL)
        {
          char argname[128];
          snprintf (argname, sizeof (argname), "arg%d", argnum);
          tree param_type_tree = parser_get_type_tree (param_type);
          tree parm_decl = build_decl (PARM_DECL, get_identifier (argname), param_type_tree);
          DECL_ARG_TYPE (parm_decl) = TREE_TYPE (parm_decl); /* Some languages have different nominal and real types.  */
          DECL_CONTEXT (parm_decl) = method_decl;
          cil_bindings_push_decl_arg (argnum, parm_decl);
          args_decl_list = chainon (parm_decl, args_decl_list);
          ++argnum;
        }
      args_decl_list = nreverse (args_decl_list);
      DECL_ARGUMENTS (method_decl) = args_decl_list;

      /* Create a DECL for the functions result. (FIXME: is this needed for ->void functions?) */
      tree result_decl = build_decl (RESULT_DECL, NULL_TREE, TREE_TYPE (TREE_TYPE (method_decl)));
      DECL_CONTEXT (result_decl) = method_decl;
      DECL_ARTIFICIAL (result_decl) = 1;
      DECL_IGNORED_P (result_decl) = 1;
      DECL_RESULT (method_decl) = result_decl;

      /* attributes */
      MonoCustomAttrInfo *attributes = mono_custom_attrs_from_method(method);
      if (attributes)
	{
	  tree attrs = convert_cil_attributes(attributes);
	  decl_attributes(&method_decl, attrs, 0 /*FIXME*/);
	  mono_custom_attrs_free (attributes);
	}

      /* declare locals */
      guint32 num_locals;
      gboolean init_locals;
      MonoType **locals = mono_method_header_get_locals (header, &num_locals, &init_locals);
      guint32 i;
      for (i = 0; i < num_locals; ++i)
        {
          char localname[128];
          snprintf (localname, sizeof (localname), "local%d", i);
          tree local_type_tree = parser_get_type_tree (locals[i]);
          tree local_decl = build_decl (VAR_DECL, get_identifier (localname), local_type_tree);
          DECL_CONTEXT (local_decl) = method_decl;
          if (init_locals)
            {
              switch (TREE_CODE (TREE_TYPE (local_decl)))
                {
                case INTEGER_TYPE:
                case ENUMERAL_TYPE:
                case POINTER_TYPE:
                case REFERENCE_TYPE:
                  DECL_INITIAL (local_decl) = convert (TREE_TYPE (local_decl), integer_zero_node);
                  break;
                default:
                  warning (0, "Should have initialized local %d, but didn't", i);
                }
            }
          cil_bindings_push_decl_local (i, local_decl);
          ++argnum;
        }

      TREE_STATIC (method_decl) = 1; /* means the function is defined */
      parse_method_code (method);

      DECL_SAVED_TREE (method_decl) = cil_bindings_pop_level ();

      allocate_struct_function (method_decl);

      /* Dump the original tree to a file. */
      dump_function (TDI_original, method_decl);

      /* Convert current function to GIMPLE for the middle end.  */
      gimplify_function_tree (method_decl);
      dump_function (TDI_generic, method_decl);

      /* We are not inside of any scope now.  */
      current_function_decl = NULL_TREE;
      cfun = NULL;

      /* Pass the current function off to the middle end.  */
      cgraph_finalize_function (method_decl, false);
    }
  else
    {
      /* native method or something */
      gcc_unreachable ();
    }
}

static void
parse_class_methods_decls (MonoClass *klass)
{
  /* Create only the declarations, the implementetions are created
     later */
  gpointer iter = NULL;
  MonoMethod *method;
  while ((method = mono_class_get_methods (klass, &iter)) != NULL)
    {
      gcc_assert (mono_method_get_class (method) == klass);
      parse_method_decl (method);
    }
}

static void parse_class_instance_fields (MonoClass *klass);

static void
parse_instance_field (MonoClassField *field)
{
  gcc_assert ((mono_field_get_flags (field) & MONO_FIELD_ATTR_STATIC) == 0);
  gcc_assert ((mono_field_get_flags (field) & MONO_FIELD_ATTR_LITERAL) == 0);
  MonoType *type = mono_field_get_type (field);
  MonoClass *klass = mono_field_get_parent (field);

  tree field_type_tree = NULL;
  switch (mono_type_get_type (type))
    {
    case MONO_TYPE_OBJECT:
    case MONO_TYPE_STRING:
    case MONO_TYPE_CLASS:
      /* We don't need the exact type, actually any pointer is enough */
      /* Using the exact type would be nice, but complicates things due to possible cycles */
      field_type_tree = build_pointer_type (parser_get_class_record_tree (mono_get_object_class ()));
      break;
    case MONO_TYPE_PTR:
      /* same as above */
      field_type_tree = build_pointer_type (void_type_node);
      break;
    case MONO_TYPE_VALUETYPE:
      parse_class_instance_fields (mono_type_get_class (type));
      field_type_tree = parser_get_type_tree (type);
      break;
    default:
      /* TODO: probably this is not strictly necessary, but it seems harmless */
      parser_parse_type (type);
      field_type_tree = parser_get_type_tree (type);
      break;
    }

  tree field_decl_tree = build_decl (FIELD_DECL, get_identifier (mono_field_get_name (field)), field_type_tree);
  tree record_type_tree = parser_get_class_record_tree (klass);

  bool explicit_layout = (mono_class_get_flags (klass) & MONO_TYPE_ATTR_LAYOUT_MASK) == MONO_TYPE_ATTR_EXPLICIT_LAYOUT;
  /* TODO: consider sequential layout */
  if (explicit_layout)
    {
      int idx = mono_metadata_token_index (mono_class_get_field_token (field)) - 1;
      guint32 offset;
      mono_metadata_field_info (mono_class_get_image (klass), idx, &offset, NULL, NULL);
      tree offset_tree = bitsize_int (offset * BITS_PER_UNIT);

      /* We need to pass in the alignment the DECL is known to have.
         This is the lowest-order bit set in OFFSET, but no more than the
         alignment of the record, if one is specified.  Note that an
         alignment of 0 is taken as infinite.  (from GNAT)*/
      unsigned int known_align = offset & -offset;
      if (TYPE_ALIGN (record_type_tree) && (known_align == 0 || known_align > TYPE_ALIGN (record_type_tree)))
        {
          known_align = TYPE_ALIGN (record_type_tree);
        }

      layout_decl (field_decl_tree, known_align);
      SET_DECL_OFFSET_ALIGN (field_decl_tree, host_integerp (offset_tree, 1) ? BIGGEST_ALIGNMENT : BITS_PER_UNIT);
      pos_from_bit (&DECL_FIELD_OFFSET (field_decl_tree), &DECL_FIELD_BIT_OFFSET (field_decl_tree), DECL_OFFSET_ALIGN (field_decl_tree), offset_tree);
    }

  DECL_CONTEXT (field_decl_tree) = record_type_tree;
  DECL_FCONTEXT (field_decl_tree) = record_type_tree;
  TYPE_FIELDS (record_type_tree) = chainon (TYPE_FIELDS (record_type_tree), field_decl_tree);
}

/* Utility function of above to merge last_size, the previous size of
   a record, with FIRST_BIT and SIZE that describe a field. Returns an
   expression for the size.  */
static tree
merge_sizes (tree last_size, tree first_bit, tree size)
{
  tree new_size_tree = size_binop (MAX_EXPR, last_size, size_binop (PLUS_EXPR, first_bit, size));
  /* We don't need any NON_VALUE_EXPRs and they can confuse us into
     thinking that a constant size is not constant.  */
  while (TREE_CODE (new_size_tree) == NON_LVALUE_EXPR)
    {
      new_size_tree = TREE_OPERAND (new_size_tree, 0);
    }
  return new_size_tree;
}

/* Based on code from the Ada frontend, simplified a bit */
static void
finish_record_type (MonoClass *klass)
{
  tree class_record_tree = parser_get_class_record_tree (klass);
  bool explicit_layout = (mono_class_get_flags (klass) & MONO_TYPE_ATTR_LAYOUT_MASK) == MONO_TYPE_ATTR_EXPLICIT_LAYOUT;
  /* TODO: consider sequential layout */

  gcc_assert (TREE_CODE (class_record_tree) == RECORD_TYPE);
  tree size = bitsize_zero_node;
  bool had_size = TYPE_SIZE (class_record_tree) != NULL_TREE;
  bool had_size_unit = TYPE_SIZE_UNIT (class_record_tree) != NULL_TREE;

  /* Globally initialize the record first.  If explicit layout that
     just means some initializations; otherwise, layout the
     record.  */
  if (explicit_layout)
    {
      TYPE_ALIGN (class_record_tree) = MAX (BITS_PER_UNIT, TYPE_ALIGN (class_record_tree));
      TYPE_MODE (class_record_tree) = BLKmode;

      if (!had_size_unit)
        {
          TYPE_SIZE_UNIT (class_record_tree) = size_zero_node;
        }
      if (!had_size)
        {
          TYPE_SIZE (class_record_tree) = bitsize_zero_node;
        }
    }
  else
    {
      gcc_assert (TYPE_SIZE (class_record_tree) == NULL_TREE);
      layout_type (class_record_tree);
    }

  /* At this point, the position and size of each field is known.  It
     was either set before entry (in parse_instance_field), or by
     laying out the type above.

     We now run a pass over the fields to compute the CIL size; the
     GCC size and alignment and the mode.

     We also clear the DECL_BIT_FIELD indication for the cases we know
     have not been handled yet, and adjust DECL_NONADDRESSABLE_P
     accordingly.  */
  tree field_tree;
  for (field_tree = TYPE_FIELDS (class_record_tree); field_tree; field_tree = TREE_CHAIN (field_tree))
    {
      tree pos = bit_position (field_tree);
      tree this_size = DECL_SIZE (field_tree);

      gcc_assert (TREE_CODE (this_size) == INTEGER_CST);
      gcc_assert (!DECL_BIT_FIELD (field_tree));

      if (explicit_layout)
        {
          TYPE_ALIGN (class_record_tree) = MAX (TYPE_ALIGN (class_record_tree), DECL_ALIGN (field_tree));
        }

      size = merge_sizes (size, pos, this_size);
    }

  guint32 packing_size;
  guint32 real_size;
  gboolean explicit_size = mono_metadata_packing_from_typedef (mono_class_get_image (klass), mono_class_get_type_token (klass), &packing_size, &real_size);
  if (explicit_size)
    {
      /* Note, using sizeof is actually incorrect */
      int extra_for_headers = mono_class_is_valuetype (klass) ? 0 : 2 * sizeof (void *);
      tree requested_size_bytes = build_int_cst (integer_type_node, real_size + extra_for_headers);
      tree requested_size = size_binop (MULT_EXPR, fold_convert (bitsizetype, requested_size_bytes), bitsize_unit_node);
      size = size_binop (MAX_EXPR, size, requested_size);
    }

  /* Now set any of the values we've just computed.  */
  if (explicit_layout || explicit_size)
    {
      tree size_unit = (had_size_unit ? TYPE_SIZE_UNIT (class_record_tree)
                        : convert (sizetype, size_binop (CEIL_DIV_EXPR, size,
                                            bitsize_unit_node)));
      TYPE_SIZE (class_record_tree) = variable_size (round_up (size, TYPE_ALIGN (class_record_tree)));
      TYPE_SIZE_UNIT (class_record_tree) = variable_size (round_up (size_unit, TYPE_ALIGN (class_record_tree) / BITS_PER_UNIT));
      compute_record_mode (class_record_tree);
    }
}

static void
parse_class_instance_fields (MonoClass *klass)
{
  if (g_hash_table_lookup (parsed_classes_records, klass) != NULL)
    {
      return;
    }

  MonoClass *parent = mono_class_get_parent (klass);
  if (parent)
    {
      parse_class_instance_fields (parent);
      gcc_assert (g_hash_table_lookup (parsed_classes_records, klass) == NULL);
      gcc_assert (g_hash_table_lookup (parsed_classes_records, parent) != NULL);
      gcc_assert (TYPE_SIZE ((tree) g_hash_table_lookup (parsed_classes_records, parent)) != NULL_TREE);
    }

  tree class_record_tree = make_node (RECORD_TYPE);
  g_hash_table_insert (parsed_classes_records, klass, class_record_tree);

  if (!mono_class_is_valuetype (klass))
    {
      if (parent)
        {
          /* emit parent fields, includes grandparents' and object headers */
          tree parent_record_tree = parser_get_class_record_tree (parent);
          tree parent_field_tree = build_decl (FIELD_DECL, get_identifier ("___parent"), parent_record_tree);
          DECL_CONTEXT (parent_field_tree) = class_record_tree;
          TYPE_FIELDS (class_record_tree) = chainon (TYPE_FIELDS (class_record_tree), parent_field_tree);
        }
      else
        {
          /* emit object header */
          tree vtable_tree = build_decl (FIELD_DECL, get_identifier ("___vtable"), build_pointer_type (void_type_node));
          DECL_CONTEXT (vtable_tree) = class_record_tree;
          TYPE_FIELDS (class_record_tree) = chainon (TYPE_FIELDS (class_record_tree), vtable_tree);
          tree sync_tree = build_decl (FIELD_DECL, get_identifier ("___synchronisation"), build_pointer_type (void_type_node));
          DECL_CONTEXT (sync_tree) = class_record_tree;
          TYPE_FIELDS (class_record_tree) = chainon (TYPE_FIELDS (class_record_tree), sync_tree);
        }
    }
  else
    {
      /* ValueTypes don't need a header, and they must inherit directly from System.ValueType or System.Enum, which don't have fields */
    }

  gpointer iter = NULL;
  MonoClassField *field;
  while ((field = mono_class_get_fields (klass, &iter)) != NULL)
    {
      gcc_assert (mono_field_get_parent (field) == klass);
      if ((mono_field_get_flags (field) & MONO_FIELD_ATTR_STATIC) == 0)
        {
          gcc_assert ((mono_field_get_flags (field) & MONO_FIELD_ATTR_LITERAL) == 0);
          parse_instance_field (field);
        }
    }
  finish_record_type (klass);
  gcc_assert (TYPE_SIZE (class_record_tree) != NULL_TREE);

  char *record_type_name = parser_get_class_mangled_name (klass, "_record");
  cil_bindings_push_type_decl (record_type_name, class_record_tree); /* ensures that the type is not garbage collected */
  free (record_type_name);

  /* create an union with the record and a byte array if the class has explicit size (otherwise some optimizations generate wrong code, because they don't expect records with a size greater than its elements size) */
  guint32 real_size;
  gboolean explicit_size = mono_metadata_packing_from_typedef (mono_class_get_image (klass), mono_class_get_type_token (klass), NULL, &real_size);
  if (explicit_size) /* TODO: consider parent size too */
    {
      gcc_assert (mono_class_is_valuetype (klass)); /* this can be removed once the parent size is considered too */

      tree union_type_tree = make_node (UNION_TYPE);
      tree record_decl_tree  = build_decl (FIELD_DECL, get_identifier ("__class_record"), class_record_tree);
      DECL_CONTEXT (record_decl_tree) = union_type_tree;
      TYPE_FIELDS (union_type_tree) = chainon (TYPE_FIELDS (union_type_tree), record_decl_tree);

      tree index_type_tree = build_index_type (build_int_cst (sizetype, real_size - 1));
      tree array_type_tree = build_array_type (cil_type_for_size (8, true), index_type_tree);
      tree array_field_decl_tree  = build_decl (FIELD_DECL, get_identifier ("__padding_array"), array_type_tree);
      DECL_CONTEXT (array_field_decl_tree) = union_type_tree;
      TYPE_FIELDS (union_type_tree) = chainon (TYPE_FIELDS (union_type_tree), array_field_decl_tree);

      layout_type (union_type_tree);
      gcc_assert (TYPE_SIZE (union_type_tree) != NULL_TREE);
      g_hash_table_insert (parsed_classes_unions, klass, union_type_tree);

      char *union_type_name = parser_get_class_mangled_name (klass, "_union");
      cil_bindings_push_type_decl (union_type_name, union_type_tree); /* ensures that the type is not garbage collected */
      free (union_type_name);
    }
}

static void
parse_class_static_fields (MonoClass *klass)
{
  gcc_assert (g_hash_table_lookup (parsed_classes_static_records, klass) == NULL);
  gcc_assert (g_hash_table_lookup (parsed_classes_static_storages, klass) == NULL);

  GCCCilClassMode class_mode = parser_get_class_mode (klass);

  tree class_static_record = make_node (RECORD_TYPE);
  g_hash_table_insert (parsed_classes_static_records, klass, class_static_record);

  gboolean emit_static_storage = false;
  MonoClassField *field;
  gpointer iter = NULL;
  while ((field = mono_class_get_fields (klass, &iter)) != NULL)
    {
      guint32 field_flags = mono_field_get_flags (field);
      if (field_flags & MONO_FIELD_ATTR_STATIC)
        {
          MonoType *type = mono_field_get_type (field);
          parser_parse_type (type);

          if (!(field_flags & MONO_FIELD_ATTR_LITERAL))
            {
              tree field_name_tree = get_identifier (mono_field_get_name (field));
              tree field_type_tree = parser_get_type_tree (type);
              tree field_tree = build_decl (FIELD_DECL, field_name_tree, field_type_tree);

              DECL_CONTEXT (field_tree) = class_static_record;
              DECL_FCONTEXT (field_tree) = class_static_record;
              TYPE_FIELDS (class_static_record) = chainon (TYPE_FIELDS (class_static_record), field_tree);

              emit_static_storage = true;
            }

          if (class_mode == GCC_CIL_CLASS_MODE_COMPILE && (field_flags & MONO_FIELD_ATTR_HAS_RVA))
            {
              static_fields_to_init = g_slist_prepend (static_fields_to_init, field);
            }
        }
    }
  layout_type (class_static_record);

  if (emit_static_storage && class_mode == GCC_CIL_CLASS_MODE_COMPILE)
    {
      char *ss_identifier = parser_get_class_static_storage_mangled_name (klass);
      tree ss_decl = build_decl (VAR_DECL, get_identifier (ss_identifier), class_static_record);
      free (ss_identifier);
      TREE_STATIC (ss_decl) = 1;
      cil_bindings_push_decl (ss_decl);  /* Ensures that the declaration is not GCed */
      g_hash_table_insert (parsed_classes_static_storages, klass, ss_decl);
    }
}

static void
parse_class_decl (MonoClass *klass)
{
  if (g_hash_table_lookup (parsed_classes, klass) != NULL)
    {
      return;
    }

  g_hash_table_insert (parsed_classes, klass, klass);

  if (mono_class_is_enum (klass))
    {
      /* Enums should not need to be parsed, since they are treated as they underlying type */
      MonoType *enum_basetype = mono_class_enum_basetype (klass);
      parser_parse_type (enum_basetype);
    }

#if 0
  /* attributes */
  MonoCustomAttrInfo *attributes = mono_custom_attrs_from_class(klass);
  if (attributes)
    {
      tree attrs = convert_cil_attributes(attributes);
      mono_custom_attrs_free (attributes);
    }
#endif

  parse_class_instance_fields (klass);
  gcc_assert (g_hash_table_lookup (parsed_classes_records, klass) != NULL);
  gcc_assert (TYPE_SIZE ((tree) g_hash_table_lookup (parsed_classes_records, klass)) != NULL_TREE);
  parse_class_static_fields (klass);
  parse_class_methods_decls (klass);

  /* We parse the parent class after this class because it makes easier to avoid cycles.
     The layout (class record) may have been parsed already it it was really necessary (for valuetypes).
     Arguably, it is not really needed at all. */
  MonoClass *parent = mono_class_get_parent (klass);
  if (parent)
    {
      parse_class_decl (parent);
    }

#if 0 /* TODO: probably not really needed at all */
  MonoClass *nesting = mono_class_get_nesting_type (klass);
  if (nesting)
    {
      parse_class_decl (nesting);
    }
#endif
}

static void
parser_parse_type (MonoType *type)
{
  int typetype = mono_type_get_type (type);
  switch (typetype)
    {
    case MONO_TYPE_VOID:
    case MONO_TYPE_BOOLEAN:
    case MONO_TYPE_CHAR:
    case MONO_TYPE_I1:
    case MONO_TYPE_U1:
    case MONO_TYPE_I2:
    case MONO_TYPE_U2:
    case MONO_TYPE_I4:
    case MONO_TYPE_U4:
    case MONO_TYPE_I8:
    case MONO_TYPE_U8:
    case MONO_TYPE_R4:
    case MONO_TYPE_R8:
      break;
    case MONO_TYPE_CLASS:
    case MONO_TYPE_VALUETYPE:
      {
        MonoClass *klass = mono_type_get_class (type);
        parse_class_decl (klass);
      }
      break;
    case MONO_TYPE_STRING:
      parse_class_decl (mono_get_string_class ());
      break;
    case MONO_TYPE_PTR:
      {
        MonoType *points_to_type = type->data.type;
        parser_parse_type (points_to_type);
      }
      break;
    case MONO_TYPE_TYPEDBYREF:
      /* TODO FIXME */
      break;
    case MONO_TYPE_I:
    case MONO_TYPE_U:
    case MONO_TYPE_FNPTR:
      /* TODO FIXME */
      break;
    case MONO_TYPE_OBJECT:
      parse_class_decl (mono_get_object_class ());
      break;
    case MONO_TYPE_SZARRAY:
      /* TODO FIXME */
      break;
    default:
      fprintf (stderr, "Unhandled type type: 0x%x %s\n", typetype, mono_type_get_name (type));
      gcc_unreachable ();
    }
}

static void
parser_init (const char *filename)
{
  mono_jit_init (filename);

  parsed_classes = g_hash_table_new (g_direct_hash, g_direct_equal);
  parsed_classes_records = g_hash_table_new (g_direct_hash, g_direct_equal);
  parsed_classes_unions = g_hash_table_new (g_direct_hash, g_direct_equal);
  parsed_classes_static_records = g_hash_table_new (g_direct_hash, g_direct_equal);
  parsed_classes_static_storages = g_hash_table_new (g_direct_hash, g_direct_equal);
  parsed_methods_decl = g_hash_table_new (g_direct_hash, g_direct_equal);
  parsed_methods_impl = g_hash_table_new (g_direct_hash, g_direct_equal);
  methods_to_parse_impl = NULL;
  static_fields_to_init = NULL;
  pinvoke_methods_to_init = NULL;
  images_that_we_are_compiling = NULL;
}

static bool
parser_get_is_initialization_method (MonoMethod *method)
{
  MonoCustomAttrInfo *attributes = mono_custom_attrs_from_method (method);
  if (attributes)
    {
      tree attrs = convert_cil_attributes(attributes);
      mono_custom_attrs_free (attributes);

      return lookup_attribute ("OpenSystem.C.InitializerAttribute", attrs) != NULL;
    }
  return false;
}

static GSList *
parser_get_image_initialization_methods (MonoImage *img)
{
  GSList *ret = NULL;
  const MonoTableInfo *methods_table = mono_image_get_table_info (img, MONO_TABLE_METHOD);
  int i;
  for (i = 0; i < mono_table_info_get_rows (methods_table); ++i)
    {
      guint32 token = MONO_TOKEN_METHOD_DEF | (i + 1);
      MonoMethod *method = mono_get_method (img, token, NULL);
      if (parser_get_is_initialization_method (method))
        {
          ret = g_slist_prepend (ret, method);
        }
    }
  return ret;
}

static void
parser_get_pinvoke_method_info (MonoMethod *method, const char **libname, const char **functionname, guint16 *piflags)
{
  MonoImage *image = mono_class_get_image (mono_method_get_class (method));

  g_assert (mono_method_get_flags (method, NULL) & MONO_METHOD_ATTR_PINVOKE_IMPL);

  guint32 token = mono_method_get_token (method);
  int idx = mono_metadata_token_index (token);
  int implmap_idx = mono_metadata_implmap_from_method (image, idx - 1);
  guint32 im_cols[MONO_IMPLMAP_SIZE];
  mono_metadata_decode_row (mono_image_get_table_info (image, MONO_TABLE_IMPLMAP), implmap_idx - 1, im_cols, MONO_IMPLMAP_SIZE);
  if (piflags) *piflags = im_cols[MONO_IMPLMAP_FLAGS];
  if (functionname) *functionname = mono_metadata_string_heap (image, im_cols[MONO_IMPLMAP_NAME]);
  guint32 scope_token = mono_metadata_decode_row_col (mono_image_get_table_info (image, MONO_TABLE_MODULEREF), im_cols[MONO_IMPLMAP_SCOPE] - 1, MONO_MODULEREF_NAME);
  if (libname) *libname = mono_metadata_string_heap (image, scope_token);
}

static tree
parser_build_string_literal (const char *str)
{
  size_t len = strlen (str);
  tree t = build_string (len, str);
  tree elem = build_type_variant (char_type_node, 1, 0);
  tree index = build_index_type (build_int_cst (NULL_TREE, len));
  tree type = build_array_type (elem, index);
  TREE_TYPE (t) = type;
  TREE_CONSTANT (t) = 1;
  TREE_INVARIANT (t) = 1;
  TREE_READONLY (t) = 1;
  TREE_STATIC (t) = 1;

  type = build_pointer_type (type);
  t = build1 (ADDR_EXPR, type, t);

  type = build_pointer_type (elem);
  t = build1 (NOP_EXPR, type, t);
  return t;
}

static GTY(()) tree lookup_pinvoke_builtin_tree = NULL;

static tree
parser_get_lookup_pinvoke_builtin_tree (void)
{
  if (!lookup_pinvoke_builtin_tree)
    {
      tree args_type_list = tree_cons (NULL_TREE, build_pointer_type (char_type_node), NULL_TREE);
      args_type_list = tree_cons (NULL_TREE, build_pointer_type (char_type_node), args_type_list);
      tree type_tree = build_function_type (build_pointer_type (void_type_node), args_type_list);
      lookup_pinvoke_builtin_tree = build_decl (FUNCTION_DECL, get_identifier ("___lookup_pinvoke"), type_tree);
    }
  return lookup_pinvoke_builtin_tree;
}

static void
parser_emit_pinvoke_initialization (void)
{
  tree body = NULL_TREE;
  GSList *li;
  for (li = pinvoke_methods_to_init; li; li = li->next)
    {
      MonoMethod *m = li->data;
      gcc_assert (mono_method_get_flags (m, NULL) & MONO_METHOD_ATTR_PINVOKE_IMPL);
      const char *libname;
      const char *functionname;
      guint16 piflags;
      parser_get_pinvoke_method_info (m, &libname, &functionname, &piflags);
      tree libname_str_tree = parser_build_string_literal (libname);
      tree functionname_str_tree = parser_build_string_literal (functionname);
      tree arglist = tree_cons (NULL_TREE, functionname_str_tree, NULL_TREE);
      arglist = tree_cons (NULL_TREE, libname_str_tree, arglist);
      tree lookup_call_tree = build_function_call_expr (parser_get_lookup_pinvoke_builtin_tree (), arglist);
      tree modify_expr_tree = build2 (MODIFY_EXPR, void_type_node, parser_get_method_tree (m), lookup_call_tree);
      append_to_statement_list_force (modify_expr_tree, &body);
    }
  g_slist_free (pinvoke_methods_to_init);
  pinvoke_methods_to_init = NULL;

  if (body != NULL_TREE)
    {
      cgraph_build_static_cdtor ('I', body, DEFAULT_INIT_PRIORITY);
    }
}

static void
parser_emit_initialization (MonoImage *img)
{
  tree body = NULL_TREE;

  /* static field initialization. TODO: could be improved. */
  GSList *li;
  for (li = static_fields_to_init; li; li = li->next)
    {
      MonoClassField *field = li->data;
      MonoClass *fclass = mono_class_from_mono_type (mono_field_get_type (field));
      if (mono_class_is_valuetype (fclass))
        {
          const gint32 size = mono_class_value_size (fclass, NULL); /* FIXME: broken! */
          tree byte_type_tree = cil_type_for_size (8, true);
          tree array_type = build_array_type (byte_type_tree, build_index_type (size_int (size)));
          char *ss_identifier = parser_get_class_static_storage_mangled_name (mono_field_get_parent (field));
          GString *var_name = g_string_new (ss_identifier);
          var_name = g_string_append (var_name, "__init_bytes__");
          var_name = g_string_append (var_name, mono_field_get_name (field));
          parser_cleanup_asm_identifier (var_name->str);
          tree bytes_var_decl = build_decl (VAR_DECL, get_identifier (var_name->str), array_type);
          g_string_free (var_name, true);
          free (ss_identifier);
          TREE_STATIC (bytes_var_decl) = 1;
          tree constr_list = NULL_TREE;
          gint32 i;
          for (i = 0; i < size; ++i)
            {
              constr_list = tree_cons (build_int_cst (integer_type_node, i), build_int_cst (byte_type_tree, mono_field_get_data (field)[i]), constr_list);
            }
          tree constructor = build_constructor_from_list (array_type, nreverse (constr_list));
          DECL_INITIAL (bytes_var_decl) = constructor;
          cil_bindings_push_decl (bytes_var_decl);
          tree memcpy_arglist = tree_cons (NULL_TREE, build_int_cst (integer_type_node, size), NULL_TREE);
          memcpy_arglist = tree_cons (NULL_TREE, build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (bytes_var_decl)), bytes_var_decl), memcpy_arglist);
          memcpy_arglist = tree_cons (NULL_TREE, parser_build_static_field_address_tree (field), memcpy_arglist);
          tree call_memcpy = build_function_call_expr (built_in_decls[BUILT_IN_MEMCPY], memcpy_arglist);
          append_to_statement_list_force (call_memcpy, &body);
        }
      else
        {
#if 0 /* TODO */
        /* it's a pointer type: add check */
          g_assert ((fklass->byval_arg.type == MONO_TYPE_PTR) || (fklass->byval_arg.type == MONO_TYPE_FNPTR));
          *t = *(char *)field->data;
#endif
          gcc_unreachable ();
        }
    }

  GSList *init_methods = parser_get_image_initialization_methods (img);
  for (li = init_methods; li; li = li->next)
    {
      MonoMethod *method = li->data;
      methods_to_parse_impl = g_slist_prepend (methods_to_parse_impl, method);
      parse_class_decl (mono_method_get_class (method));
      tree method_tree = parser_get_method_tree (method);
      /* TODO: arguments? these methods should not have them */
      tree call_tree = build_function_call_expr (method_tree, NULL_TREE);
      append_to_statement_list_force (call_tree, &body);
    }
  g_slist_free (init_methods);

  if (body != NULL_TREE)
    {
      cgraph_build_static_cdtor ('I', body, DEFAULT_INIT_PRIORITY);
    }
}

static void
parser_emit_main_function (MonoMethod *entry_point_method)
{
  /* emit __gcc4net_argc and __gcc4net_argv */
  tree __gcc4net_argc_decl = build_decl (VAR_DECL, get_identifier ("__gcc4net_argc"), integer_type_node);
  TREE_STATIC (__gcc4net_argc_decl) = 1;
  TREE_PUBLIC (__gcc4net_argc_decl) = 1;
  cil_bindings_push_decl (__gcc4net_argc_decl);
  tree __gcc4net_argv_decl = build_decl (VAR_DECL, get_identifier ("__gcc4net_argv"), build_pointer_type (build_pointer_type (char_type_node)));
  TREE_STATIC (__gcc4net_argv_decl) = 1;
  TREE_PUBLIC (__gcc4net_argv_decl) = 1;
  cil_bindings_push_decl (__gcc4net_argv_decl);

  /* Emit main function  */

  /* build argument type list (using cons, so in reverse order), and then reverse it */
  tree args_type_list = NULL_TREE;
  /* argc */
  args_type_list = tree_cons (NULL_TREE, build_pointer_type (build_pointer_type (char_type_node)), args_type_list);
  args_type_list = tree_cons (NULL_TREE, integer_type_node, args_type_list);
  /* Last parm if void indicates fixed length list (as opposed to va_* list). */
  args_type_list = tree_cons (NULL_TREE, void_type_node, args_type_list);
  args_type_list = nreverse (args_type_list);

  tree fn_type = build_function_type (integer_type_node, args_type_list);

  tree identifier = get_identifier ("main");
  tree fn_decl = build_decl (FUNCTION_DECL, identifier, fn_type);
  DECL_CONTEXT (fn_decl) = NULL_TREE; /* not nested */
  TREE_PUBLIC (fn_decl) = 1;
  DECL_EXTERNAL (fn_decl) = 0;
  TREE_STATIC (fn_decl) = 0;

  /* build argument variable decls.  */
  tree args_decl_list = NULL_TREE;
  /* argc */
  tree argc_parm_decl = build_decl (PARM_DECL, get_identifier ("argc"), integer_type_node);
  DECL_ARG_TYPE (argc_parm_decl) = TREE_TYPE (argc_parm_decl); /* Some languages have different nominal and real types.  */
  DECL_CONTEXT (argc_parm_decl) = fn_decl;
  args_decl_list = chainon (argc_parm_decl, args_decl_list);
  /* argv */
  tree argv_parm_decl = build_decl (PARM_DECL, get_identifier ("argv"), build_pointer_type (build_pointer_type (char_type_node)));
  DECL_ARG_TYPE (argv_parm_decl) = TREE_TYPE (argv_parm_decl); /* Some languages have different nominal and real types.  */
  DECL_CONTEXT (argv_parm_decl) = fn_decl;
  args_decl_list = chainon (argv_parm_decl, args_decl_list);

  args_decl_list = nreverse (args_decl_list);
  DECL_ARGUMENTS (fn_decl) = args_decl_list;

  /* Process declaration of function defined elsewhere.  */
  rest_of_decl_compilation (fn_decl, 1, 0);

  /* Output message if not -quiet.  */
  announce_function (fn_decl);

  /* This has something to do with forcing output also.  */
  cil_bindings_push_decl (fn_decl);

  /* Set current function for error msgs, etc.  */
  current_function_decl = fn_decl;

  DECL_INITIAL (fn_decl) = error_mark_node; /* TODO: debug */

  /* Create a DECL for the functions result.  */
  tree resultdecl = build_decl (RESULT_DECL, NULL_TREE, TREE_TYPE (TREE_TYPE (fn_decl)));
  DECL_CONTEXT (resultdecl) = fn_decl;
  DECL_ARTIFICIAL (resultdecl) = 1;
  DECL_IGNORED_P (resultdecl) = 1;
  DECL_RESULT (fn_decl) = resultdecl;

  /* Create a new level at the start of the function.  */
  cil_bindings_push_level ();

  TREE_STATIC (fn_decl) = 1; /* means the functions is defined */

  {
    /* emit: copy argc and argv to __gcc4net_argc and __gcc4net_argv */
    tree copy_argc = build2 (MODIFY_EXPR, TREE_TYPE (__gcc4net_argc_decl), __gcc4net_argc_decl, argc_parm_decl);
    cil_bindings_output_statements (copy_argc);
    tree copy_argv = build2 (MODIFY_EXPR, TREE_TYPE (__gcc4net_argv_decl), __gcc4net_argv_decl, argv_parm_decl);
    cil_bindings_output_statements (copy_argv);
  }
  {
    /* emit: call entry point */
    gcc_assert (current_function_decl == fn_decl);
    tree entry_point_method_tree = parser_get_method_tree (entry_point_method);
    tree exp = build_function_call_expr (entry_point_method_tree, NULL_TREE); /* FIXME: argslist */
    if (VOID_TYPE_P (TREE_TYPE (exp)))
      {
        cil_bindings_output_statements (exp);
      }
    else
      {
        tree setret = fold_build2 (MODIFY_EXPR, integer_type_node, DECL_RESULT (current_function_decl), exp);
        TREE_SIDE_EFFECTS (setret) = 1;
        TREE_USED (setret) = 1;
        setret = build1 (RETURN_EXPR, integer_type_node, setret);
        cil_bindings_output_statements (setret);
      }
  }

  DECL_SAVED_TREE (fn_decl) = cil_bindings_pop_level ();

  allocate_struct_function (fn_decl);

  /* Dump the original tree to a file. */
  dump_function (TDI_original, fn_decl);

  /* Convert current function to GIMPLE for the middle end.  */
  gimplify_function_tree (fn_decl);
  dump_function (TDI_generic, fn_decl);

  /* We are not inside of any scope now.  */
  current_function_decl = NULL_TREE;
  cfun = NULL;

  /* Pass the current function off to the middle end.  */
  cgraph_finalize_function (fn_decl, false);
}

static void
parser_queue_all_methods (MonoImage *img)
{
  const MonoTableInfo *methods_table = mono_image_get_table_info (img, MONO_TABLE_METHOD);
  int i;
  for (i = 0; i < mono_table_info_get_rows (methods_table); ++i)
    {
      guint32 token = MONO_TOKEN_METHOD_DEF | (i + 1);
      MonoMethod *method = mono_get_method (img, token, NULL);
      methods_to_parse_impl = g_slist_prepend (methods_to_parse_impl, method);
    }
}

void
parser_parse_file (const char *filename)
{
  parser_init (filename);
  MonoImageOpenStatus status;
  /* I would have thought that it is enough to open the image, but it doesn't work */
#if 0
  MonoImage *img = mono_image_open (filename, &status);
#else
  MonoAssembly *assembly = mono_assembly_open (filename, &status);
  MonoImage *img = mono_assembly_get_image (assembly);
#endif
  gcc_assert (img);
  images_that_we_are_compiling = g_slist_prepend (images_that_we_are_compiling, img);

  guint32 entry_point_token = mono_image_get_entry_point (img);
  MonoMethod *entry_point = NULL;
  if (entry_point_token)
    {
      entry_point = mono_get_method (img, entry_point_token, NULL);
      MonoClass *entry_class = mono_method_get_class (entry_point);
      parse_class_decl (entry_class);
      methods_to_parse_impl = g_slist_prepend (methods_to_parse_impl, entry_point);
    }

  if (!entry_point || !flag_parse_only_reachable)
    {
      parser_queue_all_methods (img);
    }

  parser_emit_initialization (img);

  while (methods_to_parse_impl)
    {
      MonoMethod *m = methods_to_parse_impl->data;
      methods_to_parse_impl = g_slist_remove (methods_to_parse_impl, m);
      if (g_hash_table_lookup (parsed_methods_impl, m) == NULL)
        {
          MonoOpcodeEnum reason;
          GSList *called_methods = NULL;
          GSList *referenced_types = NULL;
          parse_class_decl (mono_method_get_class (m));
          bool can_be_compiled = parser_preparse_method (m, &called_methods, &referenced_types, &reason);
          if (can_be_compiled)
            {
              GSList *li;
              for (li = called_methods; li; li = li->next)
                {
                  MonoMethod *cm = li->data;
                  parse_class_decl (mono_method_get_class (cm));
                  methods_to_parse_impl = g_slist_prepend (methods_to_parse_impl, cm);
                }
              for (li = referenced_types; li; li = li ->next)
                {
                  MonoType *t = li->data;
                  parser_parse_type (t);
                }
              g_slist_free (called_methods);
              g_slist_free (referenced_types);
              parse_method_impl (m);
              gcc_assert (g_hash_table_lookup (parsed_methods_impl, m) != NULL);
            }
          else
            {
              char *method_name = mono_method_full_name (m, true);
              switch (flag_unsupported_method_behavior)
                {
                case UMB_WARNING:
                  warning (0, "method '%s' not compiled, unsupported feature '%s'.", method_name, mono_opcode_name (reason));
                  break;
                case UMB_ERROR:
                  error ("method '%s' not compiled, unsupported feature '%s'.", method_name, mono_opcode_name (reason));
                  break;
                default:
                  gcc_unreachable ();
                }
              free (method_name);
            }
        }
    }
  parser_emit_pinvoke_initialization ();
  if (entry_point)
    {
      parser_emit_main_function (entry_point);
    }
}

#include "debug.h" /* for debug_hooks */
#include "gt-cil-parser.h"


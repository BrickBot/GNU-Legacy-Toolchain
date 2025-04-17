/* Name bindings

   Copyright (C) 1986-2007 Free Software Foundation, Inc.

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

   Authors:
     Ricardo Fernandez Pascual <ricardof@um.es>
     Andrea C. Ornstein <andrea.ornstein@st.com>

   Contact information at STMicroelectronics:
     Andrea C. Ornstein <andrea.ornstein@st.com>
     Erven Rohou        <erven.rohou@st.com>
*/

#include "config.h"
#include "bindings.h"
#include "tree-gimple.h"
#include "tree-iterator.h"

/* NOTE: This contains some code adapted from Treelang, wich I think
   it adapted from the ADA frontend. It could be simplified a bit for
   the CIL frontend. */

#define MAX_LOCALS 65535
#define MAX_ARGS 65535

struct binding_level GTY(())
{
  /* A chain of ..._DECL nodes for all variables, constants, functions,
     parameters and type declarations.  These ..._DECL nodes are chained
     through the TREE_CHAIN field. Note that these ..._DECL nodes are stored
     in the reverse of the order supplied to be compatible with the
     back-end.  */
  tree names;

  /* For each level (except the global one), a chain of BLOCK nodes for all
     the levels that were entered and exited one level down from this one.  */
  tree blocks;

  tree staments;

  int num_locals;
  int num_args;
  tree locals[MAX_LOCALS]; /* FIXME: should use somehing else instead of an array */
  tree args[MAX_ARGS];

  /* The binding level containing this one (the enclosing binding level). */
  struct binding_level *parent;
};

/* The binding level currently in effect.  */
static GTY(()) struct binding_level *current_binding_level = NULL;

/* The outermost binding level. This binding level is created when the
   compiler is started and it will exist through the entire compilation.  */
static GTY(()) struct binding_level *global_binding_level = NULL;

void
cil_bindings_init (void)
{
  current_function_decl = NULL;
  cil_bindings_push_level ();
  global_binding_level = current_binding_level;
}

/* Enter a new binding level. The input parameter is ignored, but has to be
   specified for back-end compatibility.  */
void
cil_bindings_push_level (void)
{
  struct binding_level *newlevel = ggc_alloc (sizeof (struct binding_level));
  memset (newlevel, 0, sizeof (struct binding_level));

  /* Add this level to the front of the chain (stack) of levels that are
     active.  */
  newlevel->parent = current_binding_level;
  current_binding_level = newlevel;
  current_binding_level->staments = alloc_stmt_list ();
}

/* Exit a binding level.  Pop the level off, and restore the state of
   the identifier-decl mappings that were in effect when this level
   was entered. Returns a function body tree. */
tree
cil_bindings_pop_level (void)
{
  /* This is the BLOCK node constructed for the binding level that we
     are about to exit. */
  tree block_node = build_block (current_binding_level->names, current_binding_level->blocks, 0, 0);

  /* Record the BLOCK node just built as the subblock its enclosing scope.  */
  tree subblock_node;
  for (subblock_node = current_binding_level->blocks; subblock_node; subblock_node = TREE_CHAIN (subblock_node))
    {
      BLOCK_SUPERCONTEXT (subblock_node) = block_node;
    }

  /* Clear out the meanings of the local variables of this level.  */
  for (subblock_node = current_binding_level->names; subblock_node; subblock_node = TREE_CHAIN (subblock_node))
    {
      if (DECL_NAME (subblock_node) != 0)
        {
          /* If the identifier was used or addressed via a local extern decl, don't forget that fact.   */
          if (DECL_EXTERNAL (subblock_node))
            {
              if (TREE_USED (subblock_node))
                {
                  TREE_USED (DECL_NAME (subblock_node)) = 1;
                }
            }
        }
    }

  /* Pop the current level.  */
  tree statements = current_binding_level->staments;
  current_binding_level = current_binding_level->parent;

  /* This is the top level block of the current function.  */
  DECL_INITIAL (current_function_decl) = block_node;

  if (block_node)
    {
      TREE_USED (block_node) = 1;
    }

  return build3 (BIND_EXPR, void_type_node, BLOCK_VARS (block_node), statements, block_node);
}

/* Records a ..._DECL node DECL as belonging to the current lexical scope.
   Returns the ..._DECL node. */
tree
cil_bindings_push_decl (tree decl)
{
  /* External objects aren't nested, other objects may be.  */
  if ((DECL_EXTERNAL (decl)) || (decl==current_function_decl))
    {
      DECL_CONTEXT (decl) = 0;
    }
  else
    {
      DECL_CONTEXT (decl) = current_function_decl;
    }

  /* Put the declaration on the list.  The list of declarations is in reverse
     order. The list will be reversed later if necessary.  This needs to be
     this way for compatibility with the back-end.  */
  TREE_CHAIN (decl) = current_binding_level->names;
  current_binding_level->names = decl;

  /* For the declaration of a type, set its name if it is not already set. */
  if (TREE_CODE (decl) == TYPE_DECL && TYPE_NAME (TREE_TYPE (decl)) == 0)
    {
      TYPE_NAME (TREE_TYPE (decl)) = DECL_NAME (decl);
    }

  /* Put automatic variables into the intermediate representation.  */
  if (TREE_CODE (decl) == VAR_DECL && !DECL_EXTERNAL (decl)
      && !TREE_STATIC (decl) && !TREE_PUBLIC (decl))
    {
      cil_bindings_output_statements (build1 (DECL_EXPR, void_type_node, decl));
    }
  return decl;
}

tree
cil_bindings_push_decl_local (int local_num, tree decl)
{
  gcc_assert (local_num == current_binding_level->num_locals);
  gcc_assert (local_num < MAX_LOCALS);
  gcc_assert (!current_binding_level->locals[local_num]);
  gcc_assert (decl);
  current_binding_level->locals[local_num] = decl;
  ++current_binding_level->num_locals;
  return cil_bindings_push_decl (decl);
}

tree
cil_bindings_push_decl_arg (int arg_num, tree decl)
{
  gcc_assert (arg_num == current_binding_level->num_args);
  gcc_assert (arg_num < MAX_ARGS);
  gcc_assert (!current_binding_level->args[arg_num]);
  gcc_assert (decl);
  current_binding_level->args[arg_num] = decl;
  ++current_binding_level->num_args;
  return decl; /* don't call cil_bindings_push_decl for function arguments */
}

tree
cil_bindings_get_local (int local_num)
{
  gcc_assert (local_num < current_binding_level->num_locals);
  return current_binding_level->locals[local_num];
}

tree cil_bindings_get_arg (int arg_num)
{
  gcc_assert (arg_num < current_binding_level->num_args);
  return current_binding_level->args[arg_num];
}

void
cil_bindings_push_type_decl (const char *name, tree type_node)
{
  tree id = (name==NULL) ? NULL_TREE : get_identifier (name);
  tree decl = build_decl (TYPE_DECL, id, type_node);
  TYPE_NAME (type_node) = id;
  cil_bindings_push_decl (decl);
}

/* Return the list of declarations in the current level. Note that this list
   is in reverse order (it has to be so for back-end compatibility).  */
tree
cil_bindings_get_decls (void)
{
  return current_binding_level->names;
}

void
cil_bindings_output_statements (tree staments)
{
  if (EXPRESSION_CLASS_P (staments))
    {
      SET_EXPR_LOCATION (staments, input_location);
    }
  tree_stmt_iterator tsi = tsi_last (current_binding_level->staments);
  tsi_link_after (&tsi, staments, TSI_CONTINUE_LINKING);
}

/* Outputs the statements and creates a temporary variable holding the result */
tree
cil_bindings_output_statements_and_create_temp (tree expresion)
{
  if (EXPRESSION_CLASS_P (expresion))
    {
      SET_EXPR_LOCATION (expresion, input_location);
    }
  static unsigned int tempcounter = 0;
  char localname[128];
  snprintf (localname, sizeof (localname), "temp%u", tempcounter);
  ++tempcounter;
  tree local_decl = build_decl (VAR_DECL, get_identifier (localname), TREE_TYPE (expresion));
  DECL_CONTEXT (local_decl) = current_function_decl;
  cil_bindings_push_decl (local_decl);
  tree stetement =   fold_build2 (MODIFY_EXPR, TREE_TYPE (local_decl), local_decl, expresion);
  cil_bindings_output_statements (stetement);
  return local_decl;
}

#include "debug.h" /* for debug_hooks, needed by gt-cil-treetree.h */
#include "gt-cil-bindings.h"

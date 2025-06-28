/* Type information for cil/bindings.c.
   Copyright (C) 2004, 2007, 2009 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* This file is machine generated.  Do not edit.  */

void
gt_ggc_mx_binding_level (void *x_p)
{
  struct binding_level * const x = (struct binding_level *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).names);
      gt_ggc_m_9tree_node ((*x).blocks);
      gt_ggc_m_9tree_node ((*x).staments);
      {
        size_t i0;
        size_t l0 = (size_t)(MAX_LOCALS);
        for (i0 = 0; i0 != l0; i0++) {
          gt_ggc_m_9tree_node ((*x).locals[i0]);
        }
      }
      {
        size_t i1;
        size_t l1 = (size_t)(MAX_ARGS);
        for (i1 = 0; i1 != l1; i1++) {
          gt_ggc_m_9tree_node ((*x).args[i1]);
        }
      }
      gt_ggc_m_13binding_level ((*x).parent);
    }
}

void
gt_pch_nx_binding_level (void *x_p)
{
  struct binding_level * const x = (struct binding_level *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_13binding_level, gt_ggc_e_13binding_level))
    {
      gt_pch_n_9tree_node ((*x).names);
      gt_pch_n_9tree_node ((*x).blocks);
      gt_pch_n_9tree_node ((*x).staments);
      {
        size_t i0;
        size_t l0 = (size_t)(MAX_LOCALS);
        for (i0 = 0; i0 != l0; i0++) {
          gt_pch_n_9tree_node ((*x).locals[i0]);
        }
      }
      {
        size_t i1;
        size_t l1 = (size_t)(MAX_ARGS);
        for (i1 = 0; i1 != l1; i1++) {
          gt_pch_n_9tree_node ((*x).args[i1]);
        }
      }
      gt_pch_n_13binding_level ((*x).parent);
    }
}

void
gt_pch_p_13binding_level (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct binding_level * const x ATTRIBUTE_UNUSED = (struct binding_level *)x_p;
  if ((void *)(x) == this_obj)
    op (&((*x).names), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).blocks), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).staments), cookie);
  {
    size_t i0;
    size_t l0 = (size_t)(MAX_LOCALS);
    for (i0 = 0; i0 != l0; i0++) {
      if ((void *)(x) == this_obj)
        op (&((*x).locals[i0]), cookie);
    }
  }
  {
    size_t i1;
    size_t l1 = (size_t)(MAX_ARGS);
    for (i1 = 0; i1 != l1; i1++) {
      if ((void *)(x) == this_obj)
        op (&((*x).args[i1]), cookie);
    }
  }
  if ((void *)(x) == this_obj)
    op (&((*x).parent), cookie);
}

/* GC roots.  */

EXPORTED_CONST struct ggc_root_tab gt_ggc_r_gt_cil_bindings_h[] = {
  {
    &global_binding_level,
    1,
    sizeof (global_binding_level),
    &gt_ggc_mx_binding_level,
    &gt_pch_nx_binding_level
  },
  {
    &current_binding_level,
    1,
    sizeof (current_binding_level),
    &gt_ggc_mx_binding_level,
    &gt_pch_nx_binding_level
  },
  LAST_GGC_ROOT_TAB
};


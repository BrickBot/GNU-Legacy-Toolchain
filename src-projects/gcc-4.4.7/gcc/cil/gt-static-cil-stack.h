/* Type information for cil/stack.c.
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

/* This file is a static copy of a machine-generated file.  */

void
gt_pch_p_10stack_node (ATTRIBUTE_UNUSED void *this_obj,
	void *x_p,
	ATTRIBUTE_UNUSED gt_pointer_operator op,
	ATTRIBUTE_UNUSED void *cookie)
{
  struct stack_node * const x ATTRIBUTE_UNUSED = (struct stack_node *)x_p;
  if ((void *)(x) == this_obj)
    op (&((*x).prev), cookie);
  if ((void *)(x) == this_obj)
    op (&((*x).element), cookie);
}

void
gt_ggc_mx_stack_node (void *x_p)
{
  struct stack_node * const x = (struct stack_node *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_10stack_node ((*x).prev);
      gt_ggc_m_9tree_node ((*x).element);
    }
}

void
gt_pch_nx_stack_node (void *x_p)
{
  struct stack_node * const x = (struct stack_node *)x_p;
  if (gt_pch_note_object (x, x, gt_pch_p_10stack_node, gt_ggc_e_10stack_node))
    {
      gt_pch_n_10stack_node ((*x).prev);
      gt_pch_n_9tree_node ((*x).element);
    }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_cil_stack_h[] = {
  {
    &stack_top,
    1,
    sizeof (stack_top),
    &gt_ggc_mx_stack_node,
    &gt_pch_nx_stack_node
  },
  LAST_GGC_ROOT_TAB
};


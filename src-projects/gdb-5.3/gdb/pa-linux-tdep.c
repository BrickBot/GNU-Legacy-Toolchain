/* Functions specific to gdb targetted to HPPA running Linux.
   Copyright 2000, 2001 Free Software Foundation, Inc.

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

#include "defs.h"
#include "value.h"
#include "inferior.h"
#include "gdbcore.h"
#include "symfile.h"
#include "objfiles.h"
#include "arch-utils.h"
#include "regcache.h"
#include "tm.h"
#include "elf/common.h"

static CORE_ADDR
pa_read_pc (ptid_t ptid)
{
  return (CORE_ADDR) read_register_pid (PA_PCOQ_HEAD_REGNUM, ptid) & ~3;
}

static void
pa_write_pc (CORE_ADDR pc, ptid_t ptid)
{
  write_register_pid (PA_PCOQ_HEAD_REGNUM, pc, ptid);
  write_register_pid (PA_PCOQ_TAIL_REGNUM, pc + 4, ptid);
}

static CORE_ADDR
pa_read_fp (void)
{
  return read_register (PA_GR3_REGNUM);
}

static void
pa_write_fp (CORE_ADDR val)
{
  write_register (PA_GR3_REGNUM, val);
}

static CORE_ADDR
pa_read_sp (void)
{
  return read_register (PA_GR30_REGNUM);
}

static void
pa_write_sp (CORE_ADDR val)
{
  write_register (PA_GR30_REGNUM, val);
}

/* These functions deal with saving and restoring register state
   around a function call in the inferior. They keep the stack
   double-word aligned; eventually, on an hp700, the stack will have
   to be aligned to a 64-byte boundary.  */

static void
pa_push_dummy_frame (void)
{
  CORE_ADDR sp, pc, pcspace;
  register int regnum;
  char reg_buffer[8];
  LONGEST int_buffer;
  int reg_size = REGISTER_SIZE;

  pc = TARGET_READ_PC (inferior_ptid);
  pcspace = read_register (PA_PCSQ_HEAD_REGNUM);

  /* Space for "arguments"; the RP goes in here. */
  sp = read_register (PA_GR30_REGNUM) + 48;
  read_register_gen (PA_GR2_REGNUM, reg_buffer);

  /* The 32bit and 64bit ABIs save the return pointer into different
     stack slots.  */
  if (reg_size == 8)
    write_memory (sp - 16, reg_buffer, 8);
  else
    write_memory (sp - 20, reg_buffer, 4);

  int_buffer = TARGET_READ_FP ();
  write_register (PA_GR3_REGNUM, sp);

  sp = push_word (sp, int_buffer);
  sp += reg_size;

  for (regnum = PA_GR1_REGNUM; regnum <= PA_GR31_REGNUM; regnum++)
    if (regnum != PA_GR2_REGNUM && regnum != PA_GR3_REGNUM)
      {
	read_register_gen (regnum, reg_buffer);
	sp = push_bytes (sp, reg_buffer, reg_size);
      }

  /* This is not necessary for the 64bit ABI.  In fact it is dangerous.  */
  if (reg_size != 8)
    sp += reg_size;

  for (regnum = PA_FR0_REGNUM; regnum < NUM_REGS; regnum++)
    {
      read_register_gen (regnum, reg_buffer);
      sp = push_bytes (sp, reg_buffer, 4);
    }
  read_register_gen (PA_IPSW_REGNUM, reg_buffer);
  sp = push_bytes (sp, reg_buffer, reg_size);
  read_register_gen (PA_SAR_REGNUM, reg_buffer);
  sp = push_bytes (sp, reg_buffer, reg_size);
  sp = push_word (sp, pc);
  sp = push_word (sp, pcspace);
  sp = push_word (sp, pc + 4);
  sp = push_word (sp, pcspace);
  write_register (PA_GR30_REGNUM, sp);
}

/* Called to determine if PC is in an interrupt handler of some
   kind.  */
static int
pa_linux_in_interrupt_handler (CORE_ADDR pc)
{
  /* gdb won't get control in a kernel ISR, so no need to worry here.  */
  return 0;
}

static CORE_ADDR
pa_linux_frame_saved_pc_in_interrupt (const struct frame_info *frame)
{
  return 0;
}

static CORE_ADDR
pa_hpux_frame_base_before_interrupt (const struct frame_info *frame)
{
  /* return r30 from the interrupt save state.  */
  return 0;
}

static void
pa_linux_frame_find_saved_regs_in_interrupt (struct frame_info *frame,
					     CORE_ADDR *saved_regs)
{
}

#define LINUX_GATEWAY_ADDR	0x100
#define END_LINUX_GATEWAY_ADDR	0x1000

/* Called to determine if the register state indicates we are in
   a syscall.  */ 
static int
pa_linux_in_syscall (const CORE_ADDR *saved_regs)
{
  int flags;
  CORE_ADDR pc;

  if (saved_regs && saved_regs[PA_IPSW_REGNUM])
    flags = read_memory_integer (saved_regs[PA_IPSW_REGNUM],
				 REGISTER_SIZE);
  else
    flags = read_register (PA_IPSW_REGNUM);

  if ((flags & PSW_C) == 0)
    return 1;

  if (saved_regs && saved_regs[PA_PCOQ_HEAD_REGNUM])
    pc = read_memory_integer (saved_regs[PA_PCOQ_HEAD_REGNUM],
			      REGISTER_SIZE);
  else
    pc = read_register (PA_PCOQ_HEAD_REGNUM);
  pc &= ~3;

  if (pc >= LINUX_GATEWAY_ADDR && pc < END_LINUX_GATEWAY_ADDR)
    return 1;

  return 0;
}

static const struct stub_struc pa_linux_sigtramp[] =
  {
    { 0x34190000, 0xfffffffd, 0 },	/* ldi  x,%r25 ; x=!!in_syscall   */
    { 0x3414015a, 0xffffffff, 4 },	/* ldi  __NR_rt_sigreturn,%r20    */
    { 0xe4008200, 0xffffffff, 8 },	/* be,l 0x100(%sr2,%r0),%sr0,%r31 */
    { 0x08000240, 0xffffffff, 12},	/* nop */

    { (unsigned) -1, 0, -999 }		/* sentinel */
  };

int
pa_linux_in_sigtramp (CORE_ADDR pc, const char *name)
{
  if (is_pa_stub (pc, pa_linux_sigtramp, NULL))
    return 1;

  if (pc >= LINUX_GATEWAY_ADDR && pc < END_LINUX_GATEWAY_ADDR
      && read_register (PA_GR20_REGNUM) == 0xad) /* __NR_rt_sigreturn */
    return 1;

  return 0;
}

/* Where to find the start of the register save area in a signal frame.  */
#define PA_LINUX_SIGCONTEXT(REGSIZE) \
 (((4 * 4	/* tramp */				\
    + 128	/* struct siginfo */			\
    + ((2	/* struct ucontext.uc_flags,uc_link */	\
        + 3)	/* struct ucontext.uc_stack */		\
       * (REGSIZE))) + 7) & -8)

#define PA_LINUX_SIGCONTEXT_GR(REGSIZE) \
  (PA_LINUX_SIGCONTEXT (REGSIZE) + (REGSIZE))

#define PA_LINUX_SIGCONTEXT_FR(REGSIZE) \
  ((PA_LINUX_SIGCONTEXT_GR (REGSIZE) + 32 * (REGSIZE) + 7) & -8)

#define PA_LINUX_SIGCONTEXT_PCSQ(REGSIZE) \
  (PA_LINUX_SIGCONTEXT_FR(REGSIZE) + 32 * 8)

#define PA_LINUX_SIGCONTEXT_PCOQ(REGSIZE) \
  (PA_LINUX_SIGCONTEXT_PCSQ(REGSIZE) + 2 * (REGSIZE))

#define PA_LINUX_SIGCONTEXT_SAR(REGSIZE) \
  (PA_LINUX_SIGCONTEXT_PCOQ(REGSIZE) + 2 * (REGSIZE))

static CORE_ADDR
pa_linux_frame_saved_pc_in_sigtramp (const struct frame_info *frame)
{
  int regsize = REGISTER_SIZE;
  CORE_ADDR addr;

  /* read pcoqh in sigcontext structure */
  addr = frame->frame + PA_LINUX_SIGCONTEXT_PCOQ (regsize);
  return read_memory_integer (addr, regsize) & ~3;
}

static CORE_ADDR
pa_linux_frame_base_before_sigtramp (struct frame_info *frame)
{
  CORE_ADDR start_pc;

  if (is_pa_stub (frame->pc, pa_linux_sigtramp, &start_pc))
    {
      /* Fudge alert: fix up frame->frame here as pa_frame_chain gets
	 it wrong.  The problem being that linux doesn't set r3 on
	 entry to signal handlers, and find_proc_framesize returns -1
	 for the signal handler trampoline as there is no unwind info.
	 It would be possible to make a special find_proc_framesize.
	 For now, this seems to work.  */
      frame->frame = start_pc;

#if 0
      {
	CORE_ADDR addr;
	int regsize = REGISTER_SIZE;

	/* read r30 in sigcontext structure */
	addr = start_pc + PA_LINUX_SIGCONTEXT_GR (regsize) + 30 * regsize;
	return read_memory_integer (addr, regsize);
      }
#else
      /* We may as well just return the start pc, as it's the same as
	 our saved r30 anyway.  */
      return start_pc;
#endif
    }
  return 0;
}

static void
pa_linux_frame_find_saved_regs_in_sigtramp (struct frame_info *frame,
					    CORE_ADDR *saved_regs)
{
  int i;
  int regsize = REGISTER_SIZE;
  CORE_ADDR addr;

  addr = frame->frame + PA_LINUX_SIGCONTEXT_GR (regsize);

  for (i = PA_GR0_REGNUM; i <= PA_GR31_REGNUM; i++)
    {
      if (i == PA_GR30_REGNUM)
	saved_regs[i] = read_memory_integer (addr, regsize);
      else
	saved_regs[i] = addr;
      addr += regsize;
    }
  addr = (addr + 7) & -8;
  for (i = PA_FR0_REGNUM; i < NUM_REGS; i++)
    {
      saved_regs[i] = addr;
      addr += 4;
    }
  saved_regs[PA_PCSQ_HEAD_REGNUM] = addr; addr += regsize;
  saved_regs[PA_PCSQ_TAIL_REGNUM] = addr; addr += regsize;
  saved_regs[PA_PCOQ_HEAD_REGNUM] = addr; addr += regsize;
  saved_regs[PA_PCOQ_TAIL_REGNUM] = addr; addr += regsize;
  saved_regs[PA_SAR_REGNUM] = addr;
}

/* Attempt to find (and return) the global pointer for the given file.

   This code searchs for the .dynamic section in OBJFILE.  Once it finds
   the addresses at which the .dynamic section lives in the child process,
   it scans the Elf64_Dyn or Elf32_Dyn entries for a DT_PLTGOT tag.  If it
   finds one of these, the corresponding d_un.d_ptr value is the global
   pointer.  */

static CORE_ADDR
generic_elf_find_global_pointer (struct objfile *objfile)
{
  struct obj_section *osect;

  ALL_OBJFILE_OSECTIONS (objfile, osect)
    if (strcmp (osect->the_bfd_section->name, ".dynamic") == 0)
      {
	CORE_ADDR addr;
	int dtag_size = REGISTER_SIZE;

	addr = osect->addr;
	while (addr < osect->endaddr)
	  {
	    int status;
	    LONGEST tag;
	    char buf[8];

	    status = target_read_memory (addr, buf, dtag_size);
	    if (status != 0)
	      break;
	    tag = extract_signed_integer (buf, dtag_size);

	    if (tag == DT_PLTGOT)
	      {
		CORE_ADDR global_pointer;

		status = target_read_memory (addr + dtag_size, buf, dtag_size);
		if (status != 0)
		  break;

		global_pointer = extract_address (buf, dtag_size);

		/* The payoff... */
		return global_pointer;
	      }

	    if (tag == DT_NULL)
	      break;

	    addr += 2 * dtag_size;
	  }
	break;
      }
  return 0;
}

/* This the pa-linux64 call dummy

   Call stack frame has already been built by gdb. Since we could be
   calling a varargs function, and we do not have the benefit of a stub to
   put things in the right place, we load the first 8 word of arguments
   into both the general and fp registers.

   fldd -64(0,%r29),%fr4
   fldd -56(0,%r29),%fr5
   fldd -48(0,%r29),%fr6
   fldd -40(0,%r29),%fr7
   fldd -32(0,%r29),%fr8
   fldd -24(0,%r29),%fr9
   fldd -16(0,%r29),%fr10
   fldd -8(0,%r29),%fr11
   ldd -64(%r29), %r26
   ldd -56(%r29), %r25
   ldd -48(%r29), %r24
   ldd -40(%r29), %r23
   ldd -32(%r29), %r22
   ldd -24(%r29), %r21
   ldd -16(%r29), %r20
   bve,l (%r1),%r2
   ldd -8(%r29), %r19
   mtsp %r21, %sr0		; Code used when popping frame
   ble 0(%sr0, %r22)
   nop
*/

/* Call dummys are sized and written out in word sized hunks.  So we have
   to pack the instructions into words.  */

static const LONGEST pa_linux64_dummy[] =
  {
    0x53a43f8353a53f93LL, 0x53a63fa353a73fb3LL,
    0x53a83fc353a93fd3LL, 0x2fa1100a2fb1100bLL,
    0x53ba3f8153b93f91LL, 0x53b83fa153b73fb1LL,
    0x53b63fc153b53fd1LL, 0x0fa110d4e820f000LL,
    0x0fb110d300151820LL, 0xe6c0000008000240LL
  };

/* Insert the specified number of args and function address
   into a dummy call sequence, DUMMY, stored at PC.  */

static CORE_ADDR
pa64_fix_call_dummy (char *dummy, CORE_ADDR pc, CORE_ADDR fun, int nargs,
		     struct value *args, struct type *type, int gcc_p)
{
  CORE_ADDR pcoqh, pcoqt;
  struct target_waitstatus w;
  char buf[8];
  int status;
  struct objfile *objfile;
  static const char stub[8] =
  {
    0xe8, 0x20, 0xd0, 0x00,	/* BVE (r1) */
    0x08, 0x00, 0x02, 0x40	/* NOP */
  };

  /* We can not modify the instruction address queues directly, so we start
     up the inferior and execute a couple of instructions to set them so
     that they point to the call dummy in the stack.  */
  pcoqh = read_register (PA_PCOQ_HEAD_REGNUM);
  pcoqt = read_register (PA_PCOQ_TAIL_REGNUM);

  if (target_read_memory (pcoqh, buf, 4) != 0)
    error ("Couldn't modify instruction address queue\n");

  if (target_read_memory (pcoqt, buf + 4, 4) != 0)
    error ("Couldn't modify instruction address queue\n");

  if (target_write_memory (pcoqh, stub, 4) != 0)
    error ("Couldn't modify instruction address queue\n");

  if (target_write_memory (pcoqt, stub + 4, 4) != 0)
    {
      target_write_memory (pcoqh, buf, 4);
      error ("Couldn't modify instruction address queue\n");
    }

  write_register (PA_GR1_REGNUM, pc);

  /* Single step twice, the BVE instruction will set the instruction
     address queue such that it points to the PC value written immediately
     above (ie the call dummy).  */
  resume (1, 0);
  target_wait (inferior_ptid, &w);
  resume (1, 0);
  target_wait (inferior_ptid, &w);

  /* Restore the two instructions at the old PC locations.  */
  target_write_memory (pcoqh, buf, 4);
  target_write_memory (pcoqt, buf + 4, 4);

  /* The call dummy wants the ultimate destination address in
     register %r1.  */
  write_register (PA_GR1_REGNUM, fun);

  /* We need to see if this objfile has a different DP value than our
     own (it could be a shared library for example).  */
  ALL_OBJFILES (objfile)
    {
      struct obj_section *s;
      obj_private_data_t *obj_private;

      /* See if FUN is in any section within this shared library.  */
      for (s = objfile->sections; s < objfile->sections_end; s++)
	if (s->addr <= fun && fun < s->endaddr)
	  break;

      if (s >= objfile->sections_end)
	continue;

      obj_private = (obj_private_data_t *) objfile->obj_private;
	
      /* The DP value may be different for each objfile.  But within an
	 objfile each function uses the same dp value.  Thus we do not need
	 to grope around the opd section looking for dp values.  */

      if (obj_private->dp == 0)
	{
	  obj_private->dp = generic_elf_find_global_pointer (objfile);
	  if (obj_private->dp == 0)
	    obj_private->dp = -1;
	}
      if (obj_private->dp != -1)
	write_register (PA_GR27_REGNUM, obj_private->dp);
      break;
    }
  return pc;
}

/* This is the pa-linux32 call dummy.  The function address is in r22.

   Call stack frame has already been built by gdb. Since we could be
   calling a varargs function, and we do not have the benefit of a stub to
   put things in the right place, we load the first 4 word of arguments
   into both the general and fp registers.

   ldo -36(%sp), %r1
   ldw -36(%sp), %arg0
   ldw -40(%sp), %arg1
   ldw -44(%sp), %arg2
   ldw -48(%sp), %arg3
   fldws 0(%r1), %fr4
   fldds -4(%r1), %fr5
   fldws -8(%r1), %fr6
   fldds -12(%r1), %fr7
   ble 0(%sr3, %r22)
   copy %r31, %r2
   mtsp %r21, %sr0	; Code used when popping a dummy frame.
   ble,n 0(%sr0, %r22)
   nop  */

static const LONGEST pa_linux32_dummy[] =
  {
    0x37c13fb9, 0x4bda3fb9, 0x4bd93fb1, 0x4bd83fa9,
    0x4bd73fa1, 0x24201004, 0x2c391005, 0x24311006,
    0x2c291007, 0xe6c0c000, 0x081f0242, 0x00151820,
    0xe6c00002, 0x08000240
  };

/* Insert the specified number of args and function address
   into a dummy call sequence, DUMMY, stored at PC.  */

static CORE_ADDR
pa_fix_call_dummy (char *dummy, CORE_ADDR pc, CORE_ADDR fun, int nargs,
		     struct value *args, struct type *type, int gcc_p)
{
  CORE_ADDR pcoqh, pcoqt;
  struct target_waitstatus w;
  char buf[8];
  int status;
  struct objfile *objfile;
  static const char stub[8] =
  {
    0xe4, 0x20, 0xc0, 0x00,	/* ble 0(%sr3,%r1)) */
    0x08, 0x00, 0x02, 0x40	/* nop */
  };

  /* We can not modify the instruction address queues directly, so we start
     up the inferior and execute a couple of instructions to set them so
     that they point to the call dummy in the stack.  */
  pcoqh = read_register (PA_PCOQ_HEAD_REGNUM);
  pcoqt = read_register (PA_PCOQ_TAIL_REGNUM);

  if (target_read_memory (pcoqh, buf, 4) != 0)
    error ("Couldn't modify instruction address queue\n");

  if (target_read_memory (pcoqt, buf + 4, 4) != 0)
    error ("Couldn't modify instruction address queue\n");

  if (target_write_memory (pcoqh, stub, 4) != 0)
    error ("Couldn't modify instruction address queue\n");

  if (target_write_memory (pcoqt, stub + 4, 4) != 0)
    {
      target_write_memory (pcoqh, buf, 4);
      error ("Couldn't modify instruction address queue\n");
    }

  write_register (PA_GR1_REGNUM, pc);

  /* Single step twice, the BLE instruction will set the instruction
     address queue such that it points to the PC value written immediately
     above (ie the call dummy).  */
  resume (1, 0);
  target_wait (inferior_ptid, &w);
  resume (1, 0);
  target_wait (inferior_ptid, &w);

  /* Restore the two instructions at the old PC locations.  */
  target_write_memory (pcoqh, buf, 4);
  target_write_memory (pcoqt, buf + 4, 4);

  /* The call dummy wants the ultimate destination address in
     register %r22.  */
  if (fun & 2)
    {
      /* It's a plabel.  */
      int gp;

      fun &= ~3;
      gp = read_memory_integer (fun + 4, 4);
      write_register (PA_GR19_REGNUM, gp);
      fun = read_memory_integer (fun, 4);
      write_register (PA_GR22_REGNUM, fun);
    }
  else
    {
      write_register (PA_GR22_REGNUM, fun);

      /* We need to see if this objfile has a different DP value than our
	 own (it could be a shared library for example).  */
      ALL_OBJFILES (objfile)
	{
	  struct obj_section *s;
	  obj_private_data_t *obj_private;

	  /* See if FUN is in any section within this shared library.  */
	  for (s = objfile->sections; s < objfile->sections_end; s++)
	    if (s->addr <= fun && fun < s->endaddr)
	      break;

	  if (s >= objfile->sections_end)
	    continue;

	  obj_private = (obj_private_data_t *) objfile->obj_private;
	
	  /* The DP value may be different for each objfile.  But within an
	     objfile each function uses the same dp value.  */

	  if (obj_private->dp == 0)
	    {
	      obj_private->dp = generic_elf_find_global_pointer (objfile);
	      if (obj_private->dp == 0)
		obj_private->dp = -1;
	    }
	  if (obj_private->dp != -1)
	    write_register (PA_GR19_REGNUM, obj_private->dp);
	  break;
	}
    }
  return pc;
}

#ifdef PA_XM_LINUX_H
/* For native compiles, check that our defines match the kernel.  */
#include <asm/ucontext.h>
#include <asm/rt_sigframe.h>
#endif

void
pa_linux_initialize_tdep (struct gdbarch *gdbarch, int is_elf64)
{
#ifdef PA_XM_LINUX_H
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((char *) &((TYPE *)0)->MEMBER - (char *)0)
#endif
  if (offsetof (struct rt_sigframe, uc.uc_mcontext)
      != PA_LINUX_SIGCONTEXT (sizeof (unsigned long))
      || (offsetof (struct rt_sigframe, uc.uc_mcontext.sc_gr)
	  != PA_LINUX_SIGCONTEXT_GR (sizeof (unsigned long)))
      || (offsetof (struct rt_sigframe, uc.uc_mcontext.sc_fr)
	  != PA_LINUX_SIGCONTEXT_FR (sizeof (unsigned long)))
      || (offsetof (struct rt_sigframe, uc.uc_mcontext.sc_iasq)
	  != PA_LINUX_SIGCONTEXT_PCSQ (sizeof (unsigned long)))
      || (offsetof (struct rt_sigframe, uc.uc_mcontext.sc_iaoq)
	  != PA_LINUX_SIGCONTEXT_PCOQ (sizeof (unsigned long)))
      || (offsetof (struct rt_sigframe, uc.uc_mcontext.sc_sar)
	  != PA_LINUX_SIGCONTEXT_SAR (sizeof (unsigned long))))
    internal_error (__FILE__, __LINE__,
		    "kernel struct rt_sigframe has changed");
#endif

  set_gdbarch_read_pc (gdbarch, pa_read_pc);
  set_gdbarch_write_pc (gdbarch, pa_write_pc);
  set_gdbarch_read_fp (gdbarch, pa_read_fp);
  set_gdbarch_write_fp (gdbarch, pa_write_fp);
  set_gdbarch_read_sp (gdbarch, pa_read_sp);
  set_gdbarch_write_sp (gdbarch, pa_write_sp);
  set_gdbarch_push_dummy_frame (gdbarch, pa_push_dummy_frame);

  gdbarch_tdep (gdbarch)->in_interrupt_handler = pa_linux_in_interrupt_handler;
  gdbarch_tdep (gdbarch)->frame_saved_pc_in_interrupt
    = pa_linux_frame_saved_pc_in_interrupt;
  gdbarch_tdep (gdbarch)->in_syscall = pa_linux_in_syscall;
  gdbarch_tdep (gdbarch)->in_sigtramp = pa_linux_in_sigtramp;
  gdbarch_tdep (gdbarch)->frame_saved_pc_in_sigtramp
    = pa_linux_frame_saved_pc_in_sigtramp;
  gdbarch_tdep (gdbarch)->frame_base_before_sigtramp
    = pa_linux_frame_base_before_sigtramp;
  gdbarch_tdep (gdbarch)->frame_find_saved_regs_in_sigtramp
    = pa_linux_frame_find_saved_regs_in_sigtramp;

  set_gdbarch_call_dummy_location (gdbarch, ON_STACK);
  /* call_dummy_address only needed for AT_ENTRY_POINT call dummies.  */
  set_gdbarch_call_dummy_start_offset (gdbarch, 0);
  set_gdbarch_call_dummy_breakpoint_offset (gdbarch, is_elf64 ? 0x44 : 0x2c);
  set_gdbarch_call_dummy_breakpoint_offset_p (gdbarch, 1);
  set_gdbarch_call_dummy_length (gdbarch, is_elf64 ? 0x50 : 0x38);
  set_gdbarch_pc_in_call_dummy (gdbarch, pc_in_call_dummy_on_stack);
  set_gdbarch_call_dummy_p (gdbarch, 1);
  set_gdbarch_call_dummy_words (gdbarch, (is_elf64
					  ? pa_linux64_dummy
					  : pa_linux32_dummy));
  set_gdbarch_sizeof_call_dummy_words (gdbarch, (is_elf64
						 ? sizeof (pa_linux64_dummy)
						 : sizeof (pa_linux32_dummy)));
  /* call_dummy_stack_adjust unused.  */
  set_gdbarch_call_dummy_stack_adjust_p  (gdbarch, 0);
  set_gdbarch_fix_call_dummy (gdbarch, (is_elf64
					? pa64_fix_call_dummy
					: pa_fix_call_dummy));
}


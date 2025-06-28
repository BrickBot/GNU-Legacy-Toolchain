/* Functions specific to running gdb native on HPPA running Linux.
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
#include "inferior.h"
#include "target.h"
#include "gdbcore.h"
#include "regcache.h"

#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_REG_H
#include <sys/reg.h>
#endif
#include <sys/user.h>

#include <asm/offset.h>
#include <sys/procfs.h>

/* Prototypes for supply_gregset etc. */
#include "gregset.h"

/* These must match the order of the register names.

   Some sort of lookup table is needed because the offsets associated
   with the registers are all over the board.  */

static const int u_offsets[NUM_REGS] =
  {
    /* general registers */
    -1,
    PT_GR1,
    PT_GR2,
    PT_GR3,
    PT_GR4,
    PT_GR5,
    PT_GR6,
    PT_GR7,
    PT_GR8,
    PT_GR9,
    PT_GR10,
    PT_GR11,
    PT_GR12,
    PT_GR13,
    PT_GR14,
    PT_GR15,
    PT_GR16,
    PT_GR17,
    PT_GR18,
    PT_GR19,
    PT_GR20,
    PT_GR21,
    PT_GR22,
    PT_GR23,
    PT_GR24,
    PT_GR25,
    PT_GR26,
    PT_GR27,
    PT_GR28,
    PT_GR29,
    PT_GR30,
    PT_GR31,

    PT_SAR,
    PT_IAOQ0,
    PT_IASQ0,
    PT_IAOQ1,
    PT_IASQ1,
    -1, /* eiem */
    PT_IIR,
    PT_ISR,
    PT_IOR,
    PT_PSW,
    -1, /* goto */

    PT_SR4,
    PT_SR0,
    PT_SR1,
    PT_SR2,
    PT_SR3,
    PT_SR5,
    PT_SR6,
    PT_SR7,

    -1, /* cr0 */
    -1, /* pid0 */
    -1, /* pid1 */
    -1, /* ccr */
    -1, /* pid2 */
    -1, /* pid3 */
    -1, /* cr24 */
    -1, /* cr25 */
    -1, /* cr26 */
    PT_CR27,
    -1, /* cr28 */
    -1, /* cr29 */
    -1, /* cr30 */

    /* Floating point regs.  */
    PT_FR0,  PT_FR0 + 4,
    PT_FR1,  PT_FR1 + 4,
    PT_FR2,  PT_FR2 + 4,
    PT_FR3,  PT_FR3 + 4,
    PT_FR4,  PT_FR4 + 4,
    PT_FR5,  PT_FR5 + 4,
    PT_FR6,  PT_FR6 + 4,
    PT_FR7,  PT_FR7 + 4,
    PT_FR8,  PT_FR8 + 4,
    PT_FR9,  PT_FR9 + 4,
    PT_FR10, PT_FR10 + 4,
    PT_FR11, PT_FR11 + 4,
    PT_FR12, PT_FR12 + 4,
    PT_FR13, PT_FR13 + 4,
    PT_FR14, PT_FR14 + 4,
    PT_FR15, PT_FR15 + 4,
    PT_FR16, PT_FR16 + 4,
    PT_FR17, PT_FR17 + 4,
    PT_FR18, PT_FR18 + 4,
    PT_FR19, PT_FR19 + 4,
    PT_FR20, PT_FR20 + 4,
    PT_FR21, PT_FR21 + 4,
    PT_FR22, PT_FR22 + 4,
    PT_FR23, PT_FR23 + 4,
    PT_FR24, PT_FR24 + 4,
    PT_FR25, PT_FR25 + 4,
    PT_FR26, PT_FR26 + 4,
    PT_FR27, PT_FR27 + 4,
    PT_FR28, PT_FR28 + 4,
    PT_FR29, PT_FR29 + 4,
    PT_FR30, PT_FR30 + 4,
    PT_FR31, PT_FR31 + 4,
  };

CORE_ADDR
register_addr (int regno, CORE_ADDR blockend)
{
  CORE_ADDR addr;

  if ((unsigned) regno >= NUM_REGS)
    error ("Invalid register number %d.", regno);

  if (u_offsets[regno] == -1)
    addr = 0;
  else
    {
      addr = (CORE_ADDR) u_offsets[regno];
      /* If this is a 64 bit kernel, but we are debugging a 32 bit
	 task, then we want to pick up the low word of the register.  */
      if (PT_GR2 - PT_GR1 == 8 && regno < PA_FR0_REGNUM)
	addr += (PT_GR2 - PT_GR1) - REGISTER_RAW_SIZE (regno);
    }

  return addr;
}

int pa_cannot_fetch_register (regno)
     int regno;
{
  return (unsigned int) regno >= NUM_REGS || u_offsets[regno] == -1;
}

int pa_cannot_store_register (regno)
     int regno;
{
  return ((unsigned int) regno >= NUM_REGS
	  || regno == PA_GR0_REGNUM
	  || regno == PA_PCSQ_HEAD_REGNUM
	  || (regno >= PA_PCSQ_TAIL_REGNUM && regno < PA_IPSW_REGNUM)
	  || (regno > PA_IPSW_REGNUM && regno < PA_FR4_REGNUM));
}

static const int greg_map[] =
  {
    PA_GR0_REGNUM,
    PA_GR1_REGNUM,
    PA_GR2_REGNUM,
    PA_GR3_REGNUM,
    PA_GR4_REGNUM,
    PA_GR5_REGNUM,
    PA_GR6_REGNUM,
    PA_GR7_REGNUM,
    PA_GR8_REGNUM,
    PA_GR9_REGNUM,
    PA_GR10_REGNUM,
    PA_GR11_REGNUM,
    PA_GR12_REGNUM,
    PA_GR13_REGNUM,
    PA_GR14_REGNUM,
    PA_GR15_REGNUM,
    PA_GR16_REGNUM,
    PA_GR17_REGNUM,
    PA_GR18_REGNUM,
    PA_GR19_REGNUM,
    PA_GR20_REGNUM,
    PA_GR21_REGNUM,
    PA_GR22_REGNUM,
    PA_GR23_REGNUM,
    PA_GR24_REGNUM,
    PA_GR25_REGNUM,
    PA_GR26_REGNUM,
    PA_GR27_REGNUM,
    PA_GR28_REGNUM,
    PA_GR29_REGNUM,
    PA_GR30_REGNUM,
    PA_GR31_REGNUM,
    PA_SR0_REGNUM,
    PA_SR1_REGNUM,
    PA_SR2_REGNUM,
    PA_SR3_REGNUM,
    PA_SR4_REGNUM,
    PA_SR5_REGNUM,
    PA_SR6_REGNUM,
    PA_SR7_REGNUM,
    PA_PCOQ_HEAD_REGNUM,
    PA_PCOQ_TAIL_REGNUM,
    PA_PCSQ_HEAD_REGNUM,
    PA_PCSQ_TAIL_REGNUM,
    PA_CR11_REGNUM,
    PA_CR19_REGNUM,
    PA_CR20_REGNUM,
    PA_CR21_REGNUM,
    PA_CR22_REGNUM,
    PA_CR0_REGNUM,
    PA_CR24_REGNUM,
    PA_CR25_REGNUM,
    PA_CR26_REGNUM,
    PA_CR27_REGNUM,
    PA_CR28_REGNUM,
    PA_CR29_REGNUM,
    PA_CR30_REGNUM,
    PA_CR31_REGNUM,
    PA_CR8_REGNUM,
    PA_CR9_REGNUM,
    PA_CR12_REGNUM,
    PA_CR13_REGNUM,
    PA_CR10_REGNUM,
    PA_CR15_REGNUM
  };

void
supply_gregset (gdb_gregset_t *gregsetp)
{
  int i;
  greg_t *regp = (greg_t *) gregsetp;

  for (i = 0; i < sizeof (greg_map) / sizeof (greg_map[0]); i++, regp++)
    {
      int regno = greg_map[i];
      /* When running a 64 bit kernel, a greg_t may be larger than the
	 actual register, so just pick off the LS bits of big-endian word.  */
      supply_register (regno,
		       ((char *) (regp + 1)) - REGISTER_RAW_SIZE (regno));
    }
}

void
fill_gregset (gdb_gregset_t *gregsetp, int regno)
{
  int i;
  greg_t *regp = (greg_t *) gregsetp;

  memset (gregsetp, 0, sizeof (*gregsetp));
  for (i = 0; i < sizeof (greg_map) / sizeof (greg_map[0]); i++, regp++)
    {
      int regi = greg_map[i];

      if (regno == -1 || regi == regno)
	{
	  int rawsize = REGISTER_RAW_SIZE (regi);
	  memcpy (((char *) (regp + 1)) - rawsize,
		  registers + REGISTER_BYTE (regi),
		  rawsize);
	}
    }
}

/*  Given a pointer to a floating point register set in /proc format
   (fpregset_t *), unpack the register contents and supply them as gdb's
   idea of the current floating point register values. */

void
supply_fpregset (gdb_fpregset_t *fpregsetp)
{
  register int regi;
  char *from;

  for (regi = 0; regi <= 31; regi++)
    {
      from = (char *) &((*fpregsetp)[regi]);
      supply_register (2*regi + PA_FR0_REGNUM, from);
      supply_register (2*regi + PA_FR0_REGNUM + 1, from + 4);
    }
}

/*  Given a pointer to a floating point register set in /proc format
   (fpregset_t *), update the register specified by REGNO from gdb's idea
   of the current floating point register set.  If REGNO is -1, update
   them all. */

void
fill_fpregset (gdb_fpregset_t *fpregsetp, int regno)
{
  if (regno == -1)
    memcpy (fpregsetp,
	    &registers[REGISTER_BYTE (PA_FR0_REGNUM)],
	    32 * 2 * REGISTER_RAW_SIZE (PA_FR0_REGNUM));
  else
    {
      /* Gross.  fpregset_t is double, registers[x] has single
	 precision reg.  */
      char *from = (char *) &registers[REGISTER_BYTE (regno)];
      char *to = (char *) &((*fpregsetp)[(regno - PA_FR0_REGNUM) / 2]);
      if ((regno - PA_FR0_REGNUM) & 1)
	to += 4;
      memcpy (to, from, REGISTER_RAW_SIZE (regno));
    }
}

int
pa_linux_insert_watchpoint (int pid, CORE_ADDR addr, int len, int rw)
{
  return -1;
}

int
pa_linux_remove_watchpoint (int pid, CORE_ADDR addr, int len)
{
  return -1;
}

CORE_ADDR
pa_linux_stopped_by_watchpoint (int pid)
{
  return 0;
}


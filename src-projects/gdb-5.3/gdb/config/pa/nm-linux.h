/* Native support for GNU/Linux, for GDB, the GNU debugger.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.

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

#ifndef PA_NM_LINUX_H
#define PA_NM_LINUX_H

#include "nm-linux.h"

#define CANNOT_FETCH_REGISTER(regno) pa_cannot_fetch_register(regno)
extern int pa_cannot_fetch_register (int regno);

#define CANNOT_STORE_REGISTER(regno) pa_cannot_store_register(regno)
extern int pa_cannot_store_register (int regno);

#ifdef GDBSERVER
#define REGISTER_U_ADDR(addr, blockend, regno) \
	(addr) = pa_register_u_addr ((blockend),(regno));

extern int pa_register_u_addr(int, int);
#endif /* GDBSERVER */

#define U_REGS_OFFSET 0

#define PTRACE_ARG3_TYPE long
#define PTRACE_XFER_TYPE long

/* Hardware watchpoints */

#define TARGET_HAS_HARDWARE_WATCHPOINTS

#define TARGET_CAN_USE_HARDWARE_WATCHPOINT(type, cnt, ot) \
  (type == bp_hardware_watchpoint) 

#define HAVE_STEPPABLE_WATCHPOINT 1

#define STOPPED_BY_WATCHPOINT(W) \
  pa_linux_stopped_by_watchpoint (PIDGET(inferior_ptid))
extern CORE_ADDR pa_linux_stopped_by_watchpoint (int);

#define target_insert_watchpoint(addr, len, type) \
  pa_linux_insert_watchpoint (PIDGET(inferior_ptid), addr, len, type)
extern int pa_linux_insert_watchpoint (int pid, CORE_ADDR addr,
                                         int len, int rw);

#define target_remove_watchpoint(addr, len, type) \
  pa_linux_remove_watchpoint (PIDGET(inferior_ptid), addr, len)
extern int pa_linux_remove_watchpoint (int pid, CORE_ADDR addr, int len);

#endif

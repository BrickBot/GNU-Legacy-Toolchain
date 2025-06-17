/* Definitions specific to 32 bit Hewlett-Packard PA-RISC machines.
   Copyright 1986, 1987, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996,
   1998, 1999, 2000, 2001 Free Software Foundation, Inc. 

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

#if defined(GDBSERVER)

/* Say how long (ordinary) registers are.  This is a piece of bogosity
   used in push_word and a few other places; REGISTER_RAW_SIZE is the
   real way to know how big a register is.  */

#define REGISTER_SIZE 4

/* Total amount of space needed to store our copies of the machine's
   register state, the array `registers'.  */
#define REGISTER_BYTES (NUM_REGS * 4)

/* Number of bytes of storage in the actual machine representation
   for register N.  On the PA-RISC, all regs are 4 bytes, including
   the FP registers (they're accessed as two 4 byte halves).  */

#define REGISTER_RAW_SIZE(N) 4

/* Index within `registers' of the first byte of the space for
   register N.  */

#define REGISTER_BYTE(N) (N) * 4

/* Largest value REGISTER_RAW_SIZE can have.  */

#define MAX_REGISTER_RAW_SIZE 4

#endif

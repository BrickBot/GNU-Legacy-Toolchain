/* Target-dependent code for the HP PA architecture, for GDB.
   Copyright 1986, 1987, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996,
   1998, 1999, 2000, 2001 Free Software Foundation, Inc.

   Contributed by the Center for Software Science at the
   University of Utah (pa-gdb-bugs@cs.utah.edu).

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
#include "frame.h"
#include "inferior.h"
#include "value.h"

/* For argument passing to the inferior */
#include "symtab.h"

#include "gdb_stat.h"
#include "gdb_wait.h"

#include "gdbcore.h"
#include "gdbcmd.h"
#include "target.h"
#include "symfile.h"
#include "objfiles.h"
#include "arch-utils.h"
#include "floatformat.h"
#include "regcache.h"

#include "elf-bfd.h"
#include "elf/common.h"
#include "tm.h"

#if DEBUG
static int framedebug = 0;
static int grokdebug = 0;
#ifndef DEBUG_PA_FRAME
#define DEBUG_PA_FRAME framedebug
#endif
#ifndef DEBUG_PA_GROK
#define DEBUG_PA_GROK grokdebug
#endif
#else
#ifndef DEBUG_PA_FRAME
#define DEBUG_PA_FRAME 0
#endif
#ifndef DEBUG_PA_GROK
#define DEBUG_PA_GROK 0
#endif
#endif

/* To support detection of the pseudo-initial frame that threads have.  */
#define THREAD_INITIAL_FRAME_SYMBOL  "__pthread_exit"
#define THREAD_INITIAL_FRAME_SYM_LEN  sizeof(THREAD_INITIAL_FRAME_SYMBOL)

/* Array of register names.  These should match register numbers
   defined in tm-pa.h.  The peculiar layout is to match HPUX, BSD and OSF
   32-bit interrupt frame register save state.  */
static const char *pa_register_names[NUM_REGS] =
  {
    "flags", "r1",    "rp",    "r3",    "r4",    "r5",    "r6",    "r7",
    "r8",    "r9",    "r10",   "r11",   "r12",   "r13",   "r14",   "r15",
    "r16",   "r17",   "r18",   "r19",   "r20",   "r21",   "r22",   "r23",
    "r24",   "r25",   "r26",   "dp",    "ret0",  "ret1",  "sp",    "r31",

    "sar",   "pcoqh", "pcsqh", "pcoqt", "pcsqt", "eiem",  "iir",   "isr",
    /* cr11   cr18     cr17     cr18     cr17     cr15     cr19     cr20 */

    "ior",   "ipsw",  "cr31",  "sr4",   "sr0",   "sr1",   "sr2",   "sr3",
    /* cr21   cr22   thandler? */

    "sr5",   "sr6",   "sr7",   "rcnt",  "ptid1", "ptid2", "ccr",   "ptid3",
    /*                          cr0      cr8      cr9      cr10     cr12 */

    "ptid4", "cr24",  "cr25",  "cr26",  "cr27",  "cr28",  "cr29",  "cr30",
    /* cr13                         mpsfu_high mpsfu_low mpsfu_ovflo */

    "fpsr",  "fpe1",  "fpe2",  "fpe3",  "fpe4",  "fpe5",  "fpe6",  "fpe7",
    "fr4",   "fr4R",  "fr5",   "fr5R",  "fr6",   "fr6R",  "fr7",   "fr7R",
    "fr8",   "fr8R",  "fr9",   "fr9R",  "fr10",  "fr10R", "fr11",  "fr11R",
    "fr12",  "fr12R", "fr13",  "fr13R", "fr14",  "fr14R", "fr15",  "fr15R",
    "fr16",  "fr16R", "fr17",  "fr17R", "fr18",  "fr18R", "fr19",  "fr19R",
    "fr20",  "fr20R", "fr21",  "fr21R", "fr22",  "fr22R", "fr23",  "fr23R",
    "fr24",  "fr24R", "fr25",  "fr25R", "fr26",  "fr26R", "fr27",  "fr27R",
    "fr28",  "fr28R", "fr29",  "fr29R", "fr30",  "fr30R", "fr31",  "fr31R",
  };


static char *
pa_register_name (int reg)
{
  return (char *) pa_register_names[reg];
}

int
pa_register_byte (int reg)
{
  return 4 * reg;
}

int
pa64_register_byte (int reg)
{
  return reg < PA_FR0_REGNUM ? 8 * reg : 4 * PA_FR0_REGNUM + 4 * reg;
}

int
pa_register_raw_size (int reg)
{
  return 4;
}

int
pa64_register_raw_size (int reg)
{
  return reg < PA_FR0_REGNUM ? 8 : 4;
}

struct type *
pa_register_virtual_type (int reg)
{
  if (reg >= PA_FR4_REGNUM)
    return builtin_type_float;
  else
    return builtin_type_long;
}

struct type *
pa64_register_virtual_type (int reg)
{
  if (reg < PA_FR0_REGNUM)
    return builtin_type_unsigned_long_long;
  else if (reg >= PA_FR4_REGNUM)
    return builtin_type_float;
  else
    return builtin_type_long;
}


/* Routines to extract various sized constants out of hppa 
   instructions.  */

static inline int
sign_extend (int x, int len)
{
  int signbit = (1 << (len - 1));
  int mask = (signbit << 1) - 1;
  return ((x & mask) ^ signbit) - signbit;
}

/* For many immediate values the sign bit is the low bit!  */

static inline int
low_sign_extend (int x, int len)
{
  int mask = (1 << (len - 1)) - 1;
  return ((x >> 1) & mask) - ((x & 1) << (len - 1));
}

/* This macro gets bit fields using HP's numbering (MSB = 0) */
#ifndef GET_FIELD
#define GET_FIELD(X, FROM, TO) \
  ((X) >> (31 - (TO)) & ((1 << ((TO) - (FROM) + 1)) - 1))
#endif

/* extract a 21 bit constant */

static int
extract_21 (int word)
{
  int val;

  val = GET_FIELD (word, 20 + 11, 20 + 11);
  val <<= 11;
  val |= GET_FIELD (word, 9 + 11, 19 + 11);
  val <<= 2;
  val |= GET_FIELD (word, 5 + 11, 6 + 11);
  val <<= 5;
  val |= GET_FIELD (word, 0 + 11, 4 + 11);
  val <<= 2;
  val |= GET_FIELD (word, 7 + 11, 8 + 11);
  return sign_extend (val, 21) << 11;
}

/* extract a 17 bit constant from branch instructions, returning the
   19 bit signed value. */

static int
extract_17 (unsigned word)
{
  return sign_extend (GET_FIELD (word, 19, 28) |
		      GET_FIELD (word, 29, 29) << 10 |
		      GET_FIELD (word, 11, 15) << 11 |
		      (word & 0x1) << 16, 17) << 2;
}

/* extract a 14 bit immediate field */

static int
extract_14 (unsigned word)
{
  return low_sign_extend (word, 14);
}

/* extract a 16 bit immediate field */

static int
extract_16a (unsigned word)
{
  int low = low_sign_extend (word & 0x3ff9, 14);
  return low ^ (word & 0xc000);
}

/* Compare the start address for two unwind entries returning 1 if 
   the first address is larger than the second, -1 if the second is
   larger than the first, and zero if they are equal.  */

static int
compare_unwind_entries (const void *arg1, const void *arg2)
{
  const struct unwind_table_entry *a = arg1;
  const struct unwind_table_entry *b = arg2;

  if (a->region_start > b->region_start)
    return 1;
  else if (a->region_start < b->region_start)
    return -1;
  else
    return 0;
}

static void
record_text_segment_lowaddr (bfd *abfd, asection *section, PTR info)
{
  if ((section->flags & (SEC_ALLOC | SEC_LOAD | SEC_READONLY))
      == (SEC_ALLOC | SEC_LOAD | SEC_READONLY))
    {
      bfd_vma value = section->vma - section->filepos;
      CORE_ADDR *low_text_segment_address = (CORE_ADDR *) info;

      if (value < *low_text_segment_address)
	*low_text_segment_address = value;
    }
}

static void
internalize_unwinds (struct objfile *objfile, struct unwind_table_entry *table,
		     asection *section, unsigned int entries, unsigned int size,
		     CORE_ADDR text_offset)
{
  /* We will read the unwind entries into temporary memory, then
     fill in the actual unwind table.  */
  if (size > 0)
    {
      unsigned long tmp;
      unsigned i;
      char *buf = alloca (size);

      /* If ELF, then unwinds are supposed to be segment relative
	 offsets instead of absolute addresses.

	 Note that when loading a shared library (text_offset != 0) the
	 unwinds are already relative to the text_offset that will be
	 passed in.  */
      if (text_offset == 0 && gdbarch_tdep (current_gdbarch)->is_elf)
	{
	  CORE_ADDR low_text_segment_address;

	  low_text_segment_address = -1;
	  bfd_map_over_sections (objfile->obfd,
				 record_text_segment_lowaddr,
				 (PTR) &low_text_segment_address);

	  text_offset += low_text_segment_address;
	}

      bfd_get_section_contents (objfile->obfd, section, buf, 0, size);

      /* Now internalize the information being careful to handle host/target
         endian issues.  */
      for (i = 0; i < entries; i++)
	{
	  table[i].region_start = bfd_get_32 (objfile->obfd, (bfd_byte *) buf);
	  table[i].region_start += text_offset;
	  buf += 4;
	  table[i].region_end = bfd_get_32 (objfile->obfd, (bfd_byte *) buf);
	  table[i].region_end += text_offset;
	  buf += 4;
	  tmp = bfd_get_32 (objfile->obfd, (bfd_byte *) buf);
	  buf += 4;
	  table[i].Cannot_unwind = (tmp >> 31) & 0x1;
	  table[i].Millicode = (tmp >> 30) & 0x1;
	  table[i].Millicode_save_sr0 = (tmp >> 29) & 0x1;
	  table[i].Region_description = (tmp >> 27) & 0x3;
	  table[i].reserved1 = (tmp >> 26) & 0x1;
	  table[i].Entry_SR = (tmp >> 25) & 0x1;
	  table[i].Entry_FR = (tmp >> 21) & 0xf;
	  table[i].Entry_GR = (tmp >> 16) & 0x1f;
	  table[i].Args_stored = (tmp >> 15) & 0x1;
	  table[i].Variable_Frame = (tmp >> 14) & 0x1;
	  table[i].Separate_Package_Body = (tmp >> 13) & 0x1;
	  table[i].Frame_Extension_Millicode = (tmp >> 12) & 0x1;
	  table[i].Stack_Overflow_Check = (tmp >> 11) & 0x1;
	  table[i].Two_Instruction_SP_Increment = (tmp >> 10) & 0x1;
	  table[i].Ada_Region = (tmp >> 9) & 0x1;
	  table[i].cxx_info = (tmp >> 8) & 0x1;
	  table[i].cxx_try_catch = (tmp >> 7) & 0x1;
	  table[i].sched_entry_seq = (tmp >> 6) & 0x1;
	  table[i].reserved2 = (tmp >> 5) & 0x1;
	  table[i].Save_SP = (tmp >> 4) & 0x1;
	  table[i].Save_RP = (tmp >> 3) & 0x1;
	  table[i].Save_MRP_in_frame = (tmp >> 2) & 0x1;
	  table[i].extn_ptr_defined = (tmp >> 1) & 0x1;
	  table[i].Cleanup_defined = tmp & 0x1;
	  tmp = bfd_get_32 (objfile->obfd, (bfd_byte *) buf);
	  buf += 4;
	  table[i].MPE_XL_interrupt_marker = (tmp >> 31) & 0x1;
	  table[i].HP_UX_interrupt_marker = (tmp >> 30) & 0x1;
	  table[i].Large_frame = (tmp >> 29) & 0x1;
	  table[i].Pseudo_SP_Set = (tmp >> 28) & 0x1;
	  table[i].reserved4 = (tmp >> 27) & 0x1;
	  table[i].Total_frame_size = tmp & 0x7ffffff;

	  /* Stub unwinds are handled elsewhere. */
	  table[i].stub_unwind.stub_type = 0;
	  table[i].stub_unwind.padding = 0;
	}
    }
}

obj_private_data_t *
pa_obj_private_alloc (struct objfile *objfile)
{
  obj_private_data_t *obj_private;
  obj_private = (obj_private_data_t *)
    obstack_alloc (&objfile->psymbol_obstack, sizeof (obj_private_data_t));

  memset (obj_private, 0, sizeof (obj_private));
  obj_private->unwind_info = NULL;
  obj_private->so_info = NULL;
  objfile->obj_private = (PTR) obj_private;
  return obj_private;
}

/* Read in the backtrace information stored in the unwind section of
   the object file.  This info is used mainly by find_unwind_entry to find
   out the stack frame size and frame pointer used by procedures.  We put
   everything on the psymbol obstack in the objfile so that it automatically
   gets freed when the objfile is destroyed.  */

static void
read_unwind_info (struct objfile *objfile)
{
  asection *unwind_sec, *stub_unwind_sec;
  unsigned unwind_size, stub_unwind_size, total_size;
  unsigned index, unwind_entries;
  unsigned stub_entries, total_entries;
  CORE_ADDR text_offset;
  struct obj_unwind_info *ui;
  obj_private_data_t *obj_private;

  text_offset = ANOFFSET (objfile->section_offsets, 0);
  ui = (struct obj_unwind_info *) obstack_alloc (&objfile->psymbol_obstack,
					   sizeof (struct obj_unwind_info));

  ui->table = NULL;
  ui->cache = NULL;
  ui->last = -1;

  /* For reasons unknown the HP PA64 tools generate multiple unwinder
     sections in a single executable.  So we just iterate over every
     section in the BFD looking for unwinder sections intead of trying
     to do a lookup with bfd_get_section_by_name. 

     First determine the total size of the unwind tables so that we
     can allocate memory in a nice big hunk.  */
  total_entries = 0;
  for (unwind_sec = objfile->obfd->sections;
       unwind_sec;
       unwind_sec = unwind_sec->next)
    {
      if (strcmp (unwind_sec->name, "$UNWIND_START$") == 0
	  || strcmp (unwind_sec->name, ".PARISC.unwind") == 0)
	{
	  unwind_size = bfd_section_size (objfile->obfd, unwind_sec);
	  unwind_entries = unwind_size / UNWIND_ENTRY_SIZE;

	  total_entries += unwind_entries;
	}
    }

  /* Now compute the size of the stub unwinds.  Note the ELF tools do not
     use stub unwinds at the current time.  */
  stub_unwind_size = 0;
  stub_entries = 0;
  stub_unwind_sec = bfd_get_section_by_name (objfile->obfd, "$UNWIND_END$");

  if (stub_unwind_sec)
    {
      stub_unwind_size = bfd_section_size (objfile->obfd, stub_unwind_sec);
      stub_entries = stub_unwind_size / STUB_UNWIND_ENTRY_SIZE;
    }

  /* Compute total number of unwind entries and their total size.  */
  total_entries += stub_entries;
  total_size = total_entries * sizeof (struct unwind_table_entry);

  /* Allocate memory for the unwind table.  */
  ui->table = (struct unwind_table_entry *)
    obstack_alloc (&objfile->psymbol_obstack, total_size);
  ui->last = total_entries - 1;

  /* Now read in each unwind section and internalize the standard unwind
     entries.  */
  index = 0;
  for (unwind_sec = objfile->obfd->sections;
       unwind_sec;
       unwind_sec = unwind_sec->next)
    {
      if (strcmp (unwind_sec->name, "$UNWIND_START$") == 0
	  || strcmp (unwind_sec->name, ".PARISC.unwind") == 0)
	{
	  unwind_size = bfd_section_size (objfile->obfd, unwind_sec);
	  unwind_entries = unwind_size / UNWIND_ENTRY_SIZE;

	  internalize_unwinds (objfile, &ui->table[index], unwind_sec,
			       unwind_entries, unwind_size, text_offset);
	  index += unwind_entries;
	}
    }

  /* Now read in and internalize the stub unwind entries.  */
  if (stub_unwind_size > 0)
    {
      unsigned int i;
      char *buf = alloca (stub_unwind_size);

      /* Read in the stub unwind entries.  */
      bfd_get_section_contents (objfile->obfd, stub_unwind_sec, buf,
				0, stub_unwind_size);

      /* Now convert them into regular unwind entries.  */
      for (i = 0; i < stub_entries; i++, index++)
	{
	  /* Clear out the next unwind entry.  */
	  memset (&ui->table[index], 0, sizeof (struct unwind_table_entry));

	  /* Convert offset & size into region_start and region_end.  
	     Stuff away the stub type into "reserved" fields.  */
	  ui->table[index].region_start = bfd_get_32 (objfile->obfd,
						      (bfd_byte *) buf);
	  ui->table[index].region_start += text_offset;
	  buf += 4;
	  ui->table[index].stub_unwind.stub_type = bfd_get_8 (objfile->obfd,
							  (bfd_byte *) buf);
	  buf += 2;
	  ui->table[index].region_end
	    = ui->table[index].region_start + 4 *
	    (bfd_get_16 (objfile->obfd, (bfd_byte *) buf) - 1);
	  buf += 2;
	}

    }

  /* Unwind table needs to be kept sorted.  */
  qsort (ui->table, total_entries, sizeof (struct unwind_table_entry),
	 compare_unwind_entries);

  /* Keep a pointer to the unwind information.  */
  obj_private = (obj_private_data_t *) objfile->obj_private;
  if (obj_private == NULL)
    obj_private = (PTR) pa_obj_private_alloc (objfile);

  obj_private->unwind_info = ui;
}

/* Lookup the unwind (stack backtrace) info for the given PC.  We search all
   of the objfiles seeking the unwind table entry for this PC.  Each objfile
   contains a sorted list of struct unwind_table_entry.  Since we do a binary
   search of the unwind tables, we depend upon them to be sorted.  */

struct unwind_table_entry *
find_unwind_entry (CORE_ADDR pc)
{
  int first, middle, last;
  struct objfile *objfile;

  /* A function at address 0?  Not in HP-UX!  Or linux, for that matter.  */
  if (pc == (CORE_ADDR) 0)
    return NULL;

  ALL_OBJFILES (objfile)
  {
    struct obj_unwind_info *ui;
    ui = NULL;
    if (objfile->obj_private)
      ui = ((obj_private_data_t *) (objfile->obj_private))->unwind_info;

    if (!ui)
      {
	read_unwind_info (objfile);
	if (objfile->obj_private == NULL)
	  error ("Internal error reading unwind information.");
	ui = ((obj_private_data_t *) (objfile->obj_private))->unwind_info;
      }

    /* First, check the cache */

    if (ui->cache
	&& pc >= ui->cache->region_start
	&& pc <= ui->cache->region_end)
      return ui->cache;

    /* Not in the cache, do a binary search */

    first = 0;
    last = ui->last;

    while (first <= last)
      {
	middle = (first + last) / 2;
	if (pc >= ui->table[middle].region_start
	    && pc <= ui->table[middle].region_end)
	  {
	    ui->cache = &ui->table[middle];
	    return &ui->table[middle];
	  }

	if (pc < ui->table[middle].region_start)
	  last = middle - 1;
	else
	  first = middle + 1;
      }
  }				/* ALL_OBJFILES() */
  return NULL;
}

static const struct stub_struc pa32_stubs[] =
  {
    /* Long branch stub.  */
    { 0x20200000, 0xffe00000, 0 },	/* ldil  LR'XXX,%r1		 */
    { 0xe0202002, 0xffe0e002, 4 },	/* be,n  RR'XXX(%sr4,%r1)	 */

    /* PIC long branch stub.  */
    { 0xe8200000, 0xffffffff,		/* b,l   .+8,%r1		 */
      -pa_stub_long_branch },		/* type of previous stub */
    { 0x28200000, 0xffe00000, 4 },	/* addil LR'XXX,%r1,%r1		 */
    { 0xe0202002, 0xffe0e002, 8 },	/* be,n  RR'XXX(%sr4,%r1)	 */

    /* Import stub.  */
    { 0x2b600000, 0xffe00000,		/* addil LR'XXX,%dp,%r1		 */
      -pa_stub_long_branch_shared },
    { 0x48350000, 0xffffc000, 4 },	/* ldw   RR'XXX(%sr0,%r1),%r21	 */
    { 0xeaa0c000, 0xffffffff, 8 },	/* bv    %r0(%r21)		 */
    { 0x48330000, 0xffffc000, 12 },	/* ldw   RR'XXX+4(%sr0,%r1),%r19 */

    /* Import stub for shared lib.  */
    { 0x2a600000, 0xffe00000,		/* addil LR'XXX,%r19,%r1	 */
      -pa_stub_import },
    { 0x48350000, 0xffffc000, 4 },	/* ldw   RR'XXX(%sr0,%r1),%r21	 */
    { 0xeaa0c000, 0xffffffff, 8 },	/* bv    %r0(%r21)		 */
    { 0x48330000, 0xffffc000, 12 },	/* ldw   RR'XXX+4(%sr0,%r1),%r19 */

    /* Multiple sub-space import stub.  */
    { 0x2b600000, 0xffe00000,		/* addil LR'XXX,%dp		 */
      -pa_stub_import_shared },
    { 0x48350000, 0xffffc000, 4 },	/* ldw   RR'XXX(%r1),%r21	 */
    { 0x48330000, 0xffffc000, 8 },	/* ldw   RR'XXX+4(%r1),%r19	 */
    { 0x02a010a1, 0xffffffff, 12 },	/* ldsid (%r21),%r1		 */
    { 0x00011820, 0xffffffff, 16 },	/* mtsp  %r1,%sr0		 */
    { 0xe2a00000, 0xffffffff, 20 },	/* be    0(%sr0,%r21)		 */
    { 0x6bc23fd1, 0xffffffff, 24 },	/* stw   %rp,-24(%sp)		 */

    /* Multiple sub-space import stub for shared lib.  */
    { 0x2a600000, 0xffe00000,		/* addil LR'XXX,%r19		 */
      -pa_stub_import_multi },
    { 0x48350000, 0xffffc000, 4 },	/* ldw   RR'XXX(%r1),%r21	 */
    { 0x48330000, 0xffffc000, 8 },	/* ldw   RR'XXX+4(%r1),%r19	 */
    { 0x02a010a1, 0xffffffff, 12 },	/* ldsid (%r21),%r1		 */
    { 0x00011820, 0xffffffff, 16 },	/* mtsp  %r1,%sr0		 */
    { 0xe2a00000, 0xffffffff, 20 },	/* be    0(%sr0,%r21)		 */
    { 0x6bc23fd1, 0xffffffff, 24 },	/* stw   %rp,-24(%sp)		 */

    /* ELF32 .plt lazy linking stub.  */
    { 0x0e801096, 0xffffffff,		/* 1: ldw  0(%r20),%r22		 */
      -pa_stub_import_multi_shared },
    { 0xeac0c000, 0xffffffff, 4 },	/*    bv   %r0(%r22)		 */
    { 0x0e881095, 0xffffffff, 8 },	/*    ldw  4(%r20),%r21		 */
    { 0xea9f1fdd, 0xffffffff, 12 },	/*    b,l  1b,%r20		 */
    { 0xd6801c1e, 0xffffffff, 16 },	/*    depi 0,31,2,%r20		 */

#define EXPORT_STUB_ENT 32
    /* Export stub.  */
    { 0xe8400002, 0xffe0e002,		/* bl,n  X,%rp			 */
      -pa_stub_lazy_link },
    { 0x08000240, 0xffffffff, 4 },	/* nop				 */
    { 0x4bc23fd1, 0xffffffff, 8 },	/* ldw   -24(%sp),%rp		 */
    { 0x004010a1, 0xffffffff, 12 },	/* ldsid (%rp),%r1		 */
    { 0x00011820, 0xffffffff, 16 },	/* mtsp  %r1,%sr0		 */
    { 0xe0400002, 0xffffffff, 20 },	/* be,n  0(%sr0,%rp)		 */

    { (unsigned) -1, 0,			/* sentinel */
      -pa_stub_export }
  };

static const struct stub_struc pa64_stubs[] =
  {
    { 0x53610000, 0xffffc00e, 0 },	/* ldd	pltoff(%r27),%r1	 */
    { 0xe820d000, 0xffffffff, 4 },	/* bve	(%r1)			 */
    { 0x537b0000, 0xffffc00e, 8 },	/* ldd	pltoff+8(%r27),%r27	 */

    { (unsigned) -1, 0,			/* sentinel */
      -pa64_stub_import }
  };

enum stub_type
is_pa_stub (CORE_ADDR pc, const struct stub_struc *srch, CORE_ADDR *start)
{
  unsigned int insn;

  insn = read_memory_integer (pc, 4);
  if (DEBUG_PA_GROK)
    printf ("is_pa_stub (%#lx, %p), insn=%#x = ", (long) pc, srch, insn);

  for (; srch->mask != 0; srch++)
    {
      if ((insn & srch->mask) == srch->insn)
	{
	  /* Possible match, check the whole stub.  */
	  int i;
	  CORE_ADDR addr;
	  const struct stub_struc *stub;

	  i = srch->offset;
	  if (i < 0)
	    i = 0;
	  stub = ((const struct stub_struc *)
		  ((const char *) srch - i * (sizeof (pa32_stubs[0]) / 4)));
	  addr = pc - i;
	  if (start)
	    *start = addr;
	  do
	    {
	      unsigned int x = read_memory_integer (addr, 4);
	      if ((x & stub->mask) != stub->insn)
		goto no_match;
	      stub++;
	      addr += 4;
	    }
	  while (stub->offset > 0);
	  if (DEBUG_PA_GROK)
	    printf ("matches %d\n", -stub->offset);
	  return -stub->offset;
	}
    no_match: ;
    }
  if (DEBUG_PA_GROK)
    printf ("no\n");
  return pa_stub_none;
}

/* Called when no unwind descriptor was found for PC.  Returns 1 if it
   appears that PC is in a linker stub.  */

static enum stub_type
pc_in_linker_stub (CORE_ADDR pc, CORE_ADDR *start)
{
  const struct stub_struc *srch = pa32_stubs;

  if (gdbarch_tdep (current_gdbarch)->is_elf64)
    srch = pa64_stubs;

  return is_pa_stub (pc, srch, start);
}

/* Called when no unwind descriptor was found for PC.  Returns 1 if it
   appears that PC is in a linker export stub.  */

static int
pc_in_linker_call_stub (CORE_ADDR pc)
{
  if (gdbarch_tdep (current_gdbarch)->is_elf)
    return 0;
  return is_pa_stub (pc, pa32_stubs + EXPORT_STUB_ENT, NULL) != pa_stub_none;
}

static int
find_return_regnum (CORE_ADDR pc)
{
  struct unwind_table_entry *u;

  u = find_unwind_entry (pc);

  if (u && u->Millicode)
    return PA_GR31_REGNUM;

  return PA_GR2_REGNUM;
}

/* Return size of frame, or -1 if we should use a frame pointer.  */
static int
find_proc_framesize (CORE_ADDR pc)
{
  struct unwind_table_entry *u;
  int ret = -1;

  if (DEBUG_PA_FRAME)
    printf ("find_proc_framesize (%#lx)\n", (long) pc);

  /* This may indicate a bug in our callers... */
  if (pc == (CORE_ADDR) 0)
    return ret;

  u = find_unwind_entry (pc);
  if (!u)
    {
      /* Linker stubs have a zero size frame.  */
      if (pc_in_linker_stub (pc, NULL) != pa_stub_none)
	ret = 0;
    }
  /* If Save_SP is set, and we're not in an interrupt or signal caller,
     then we have a frame pointer.  Use it.  Otherwise find the size
     from unwind info.  */
  else if (!u->Save_SP
	   || PA_IN_INTERRUPT_HANDLER (pc)
	   || IN_SIGTRAMP (pc, SYMBOL_NAME (lookup_minimal_symbol_by_pc (pc))))
    {
      ret = u->Total_frame_size << 3;
    }
  if (DEBUG_PA_FRAME)
    printf ("  unwind = %p, returning %#x\n", u, ret);

  return ret;
}

/* Return offset from sp at which rp is saved, or 0 if not saved.  */
static int
rp_saved (CORE_ADDR pc)
{
  struct unwind_table_entry *u;

  /* A function at, and thus a return PC from, address 0?  Not in HP-UX! */
  if (pc == (CORE_ADDR) 0)
    return 0;

  u = find_unwind_entry (pc);

  if (!u)
    {
      if (pc_in_linker_call_stub (pc))
	/* This is the so-called RP'.  */
	return -24;
      else
	return 0;
    }

  if (u->Save_RP)
    return (REGISTER_SIZE == 8 ? -16 : -20);

  switch (u->stub_unwind.stub_type)
    {
    case EXPORT:
    case IMPORT:
      return -24;
    case PARAMETER_RELOCATION:
      return -8;
    }
  return 0;
}

/* Returns non-zero if the function invocation represented by FI does not
   have a frame on the stack associated with it.  */
static int pa_frameless_function_invocation (struct frame_info *fi)
{
  int ret;

  if (pc_in_linker_stub (fi->pc, NULL) != pa_stub_none)
    ret = 1;
  else
    {
      struct unwind_table_entry *u;

      u = find_unwind_entry (fi->pc);
      ret = (u != 0 && u->Total_frame_size == 0
	     && u->stub_unwind.stub_type == 0);
    }

  if (DEBUG_PA_FRAME)
    printf ("frameless_func (%p) = %#x\n", fi, ret);
  return ret;
}

static CORE_ADDR
pa_saved_pc_after_call (struct frame_info *frame)
{
  int ret_regnum;
  CORE_ADDR pc;
  struct unwind_table_entry *u;

  if (DEBUG_PA_FRAME)
    printf ("saved_pc_after_call (%p)\n", frame);

  ret_regnum = find_return_regnum (frame->pc);
  pc = read_register (ret_regnum) & ~3;

  /* If PC is in a linker stub, then we need to dig the address
     the stub will return to out of the stack.  */
  u = find_unwind_entry (pc);
  if (u && u->stub_unwind.stub_type != 0)
    pc = FRAME_SAVED_PC (frame);

  return pc;
}

static CORE_ADDR
pa_frame_saved_pc (struct frame_info *frame)
{
  CORE_ADDR pc = frame->pc;
  struct unwind_table_entry *u;
  CORE_ADDR old_pc = 0;
  int spun_around_loop = 0;
  int rp_offset = 0;

  if (DEBUG_PA_FRAME)
    {
      printf ("frame_saved_pc (%p)\n", frame);
      printf ("  frame->prev = %p, frame->next = %p\n",
	      frame->prev, frame->next);
      printf ("  frame->frame = %#lx, frame->pc = %#lx, frame->extra = %p\n",
	      (long) frame->frame, (long) frame->pc, frame->extra_info);
    }

  if (PA_IN_INTERRUPT_HANDLER (pc))
    {
      CORE_ADDR rp;
      rp = gdbarch_tdep (current_gdbarch)->frame_saved_pc_in_interrupt (frame);
      if (DEBUG_PA_FRAME)
	printf ("  in interrupt, returning %#lx\n", (long) rp & ~3);
      return rp & ~3;
    }

  /* Deal with signal handler caller frames too.  */
  if (frame->signal_handler_caller
      && gdbarch_tdep (current_gdbarch)->frame_saved_pc_in_sigtramp)
    {
      CORE_ADDR rp;
      rp = gdbarch_tdep (current_gdbarch)->frame_saved_pc_in_sigtramp (frame);
      if (DEBUG_PA_FRAME)
	printf ("  in signal handler, returning %#lx\n", (long) rp & ~3);
      return rp & ~3;
    }

  /* Are we in a call dummy?  */
  if (frame->pc >= frame->frame
      && frame->pc < (frame->frame
		      + CALL_DUMMY_LENGTH
		      /* Account for register saves.  See 
			 find_dummy_frame_regs and push_dummy_frame.  */
		      + ((32 + 6) * REGISTER_SIZE)
		      + (NUM_REGS - PA_FR0_REGNUM) * 4))
    {
      CORE_ADDR rp;
      rp = read_memory_integer (frame->frame - (REGISTER_SIZE == 8 ? 16 : 20),
				REGISTER_SIZE);
      if (DEBUG_PA_FRAME)
	printf ("  in dummy, returning %#lx\n", (long) rp & ~3);
      return rp & ~3;
    }

  if (FRAMELESS_FUNCTION_INVOCATION (frame))
    {
      int ret_regnum;

      ret_regnum = find_return_regnum (pc);

      if (DEBUG_PA_FRAME > 1)
	printf ("  frameless, ret reg = %#x\n", ret_regnum);

      /* If the next frame is an interrupt frame or a signal
         handler caller, then we need to look in the saved
         register area to get the return pointer (the values
         in the registers may not correspond to anything useful).  */
      if (frame->next
	  && (frame->next->signal_handler_caller
	      || PA_IN_INTERRUPT_HANDLER (frame->next->pc)))
	{
	  if (!frame->next->saved_regs)
	    FRAME_INIT_SAVED_REGS (frame->next);
	  if (PA_IN_SYSCALL (frame->next->saved_regs))
	    {
	      pc = read_memory_integer (frame->next->saved_regs[PA_GR31_REGNUM],
					REGISTER_SIZE) & ~3;

	      /* Syscalls are really two frames.  The syscall stub itself
	         with a return pointer in %rp and the kernel call with
	         a return pointer in %r31.  We return the %rp variant
	         if %r31 is the same as frame->pc.  */
	      if (pc == frame->pc)
		pc = read_memory_integer (frame->next->saved_regs[PA_GR2_REGNUM],
					  REGISTER_SIZE) & ~3;
	    }
	  else
	    pc = read_memory_integer (frame->next->saved_regs[PA_GR2_REGNUM],
				      REGISTER_SIZE) & ~3;
	}
      else
	pc = read_register (ret_regnum) & ~3;
    }
  else
    {
      spun_around_loop = 0;
      old_pc = pc;

      rp_offset = 0;
      if (frame->next
	  || !frame->extra_info
	  || pc > frame->extra_info->rp_save_insn)
	{
	restart:
	  rp_offset = rp_saved (pc);
	}

      if (DEBUG_PA_FRAME > 1)
	printf ("  framed, rp_offset = %#x\n", rp_offset);

      /* Similar to code in frameless function case.  If the next
         frame is a signal or interrupt handler, then dig the right
         information out of the saved register info.  */
      if (rp_offset == 0
	  && frame->next
	  && (frame->next->signal_handler_caller
	      || PA_IN_INTERRUPT_HANDLER (frame->next->pc)))
	{
	  if (!frame->next->saved_regs)
	    FRAME_INIT_SAVED_REGS (frame->next);
	  if (PA_IN_SYSCALL (frame->next->saved_regs))
	    {
	      pc = read_memory_integer (frame->next->saved_regs[PA_GR31_REGNUM],
					REGISTER_SIZE) & ~3;

	      /* Syscalls are really two frames.  The syscall stub
		 itself with a return pointer in %rp and the kernel call
		 with a return pointer in %r31.  We return the %rp
		 variant if %r31 is the same as frame->pc.  */
	      if (pc == frame->pc)
		pc = read_memory_integer (frame->next->saved_regs[PA_GR2_REGNUM],
					  REGISTER_SIZE) & ~3;
	    }
	  else
	    pc = read_memory_integer (frame->next->saved_regs[PA_GR2_REGNUM],
				      REGISTER_SIZE) & ~3;
	}
      else if (rp_offset == 0 && frame->next)
	{
	  /* Can't find pc save position, but frame->next set implies
	     r2 has been trashed.  I suppose we could dig r2 out of
	     saved regs, but this point here is typically reached on
	     _start, so it's reasonable to say we don't have a saved
	     pc.  */
	  pc = 0;
	}
      else if (rp_offset == 0)
	{
	  old_pc = pc;
	  pc = read_register (PA_GR2_REGNUM) & ~3;
	}
      else
	{
	  old_pc = pc;
	  pc = read_memory_integer (frame->frame + rp_offset,
				    REGISTER_SIZE) & ~3;
	}
    }

  /* If PC is inside a linker stub, then dig out the address the stub
     will return to. 

     Don't do this for long branch stubs.  Why?  For some unknown reason
     _start is marked as a long branch stub in hpux10.  */
  if (pc)
    {
      u = find_unwind_entry (pc);
      if (u && u->stub_unwind.stub_type != 0
	  && u->stub_unwind.stub_type != LONG_BRANCH)
	{
	  unsigned int insn;

	  /* If this is a dynamic executable, and we're in a signal
	     handler, then the call chain will eventually point us
	     into the stub for _sigreturn.  Unlike most cases, we'll
	     be pointed to the branch to the real sigreturn rather
	     than the code after the real branch!.  Else, try to dig
	     the address the stub will return to in the normal
	     fashion.  */
	  insn = read_memory_integer (pc, 4);
	  if (DEBUG_PA_FRAME > 1)
	    printf ("  stub, insn = %#x\n", insn);

	  if ((insn & 0xfc00e000) == 0xe8000000)
	    return (pc + extract_17 (insn) + 8) & ~3;
	  else
	    {
	      if (old_pc == pc)
		spun_around_loop++;

	      if (spun_around_loop > 1)
		{
		  /* We're just about to go around the loop again with
		     no more hope of success.  Die. */
		  error ("Unable to find return pc for this frame");
		}
	      else
		goto restart;
	    }
	}
    }

  if (DEBUG_PA_FRAME)
    printf ("  returning %#lx\n", (long) pc);

  return pc;
}

/* For the given instruction (INST), return any adjustment it makes
   to the stack pointer or zero for no adjustment. 

   This only handles instructions commonly found in prologues.  */

static int
prologue_inst_adjust_sp (unsigned int inst, unsigned int high21)
{
  /* The most common way to perform a stack adjustment ldo X(sp),sp */
  if ((inst & 0xffffc000) == 0x37de0000)
    return extract_14 (inst);

  /* stwm X,D(sp) */
  if ((inst & 0xffe00000) == 0x6fc00000)
    return extract_14 (inst);

  /* std,ma X,D(sp) */
  if ((inst & 0xffe00008) == 0x73c00008)
    return ((inst & 0x1ff8) >> 1) - ((inst & 1) << 13);

  /* Second insn of "addil high21,%r30; ldo low11,(%r1),%r30" pair.  */
  if ((inst & 0xffff0000) == 0x343e0000)
    return high21 + extract_14 (inst);

  /* fstws as used by the HP compilers.  */
  if ((inst & 0xffffffe0) == 0x2fd01220)
    return low_sign_extend (inst >> 16, 5);

  /* No adjustment.  */
  return 0;
}

/* Return nonzero if INST is a branch of some kind, else return zero.  */

static inline int
is_branch (unsigned int inst)
{
  /* Return true on opcodes
     0x20,0x21,0x22,0x23, 0x28,0x29,0x2a,0x2b
     0x30,0x31,0x32,0x33, 0x38,0x39,0x3a,0x3b.  */
  return ((inst & (0x24 << 26)) == (0x20 << 26)
	  /* Or on opcodes 0x27, 0x2f.  */
	  || (inst & (0x37 << 26)) == (0x27 << 26));
}

/* Return the register number for a GR which is saved by INST or
   zero if INST does not save a GR.  */

static int
inst_saves_gr (unsigned int inst)
{
  if (
      /* Does it look like a non-indexed stb, sth, stw, stwm?
	 ie. opcodes 0x18, 0x19, 0x1a, 0x1b.  */
      (inst & (0x3c << 26)) == (0x18 << 26)

      /* How about a non-indexed std?  */
      || (inst & ((0x3f << 26) | 2)) == ((0x1c << 26) | 0)

      /* or a pa2 stwm?  */
      || (inst & ((0x3f << 26) | 6)) == ((0x1f << 26) | 4)

      /* Or possibly an indexed stb, stw, sth or std?  */
      || ((inst & ((0x3f << 26) | (1 << 12) | (3 << 8)))
	  == ((0x03 << 26) | (1 << 12) | (2 << 8)))
      )
    return (inst >> 16) & 0x1f;

  return 0;
}

/* Return the register number for a FR which is saved by INST or
   zero if INST does not save a FR.  We don't distinguish between left
   and right float reg saves here.  */

static int
inst_saves_fr (unsigned int inst)
{
  if (
      /* Is this a long displacement fstd?  */
      (inst & ((0x3f << 26) | 2)) == ((0x1c << 26) | 2)

      /* or a long displacement fstw?  */
      || (inst & ((0x3f << 26) | 4)) == ((0x1f << 26) | 0)
      || (inst & (0x3f << 26)) == (0x1e << 26)
      )
    return (inst >> 16) & 0x1f;

  /* How about a short or indexed fstd or fstw?  */
  if ((inst & ((0x3d << 26) | (1 << 9))) == ((0x09 << 26) | (1 << 9)))
    return inst & 0x1f;

  return 0;
}

/* Run through the prologue instructions for the function given by PC,
   storing information about register saves in FRAME_INFO.  Return the pc
   at the end of the prologue.  */
CORE_ADDR
pa_grok_prologue (CORE_ADDR pc, struct frame_info *frame_info)
{
  char buf[4];
  CORE_ADDR orig_pc = pc;
  CORE_ADDR *fsr = 0;
  unsigned long inst, stack_remaining, save_gr, save_fr, save_rp, save_sp;
  unsigned long args_stored, status, i, restart_gr, restart_fr;
  struct unwind_table_entry *u;
  unsigned int r1_val, r1_base;
  int final_iteration;

  if (frame_info)
    fsr = frame_info->saved_regs;

  if (DEBUG_PA_GROK)
    printf ("grok_prologue (%#lx, %p), fsr = %p, pc = %#lx\n",
	    (long) pc, frame_info, fsr,
	    (long) (frame_info ? frame_info->pc : 0));

  u = find_unwind_entry (pc);
  if (!u)
    {
      if (DEBUG_PA_GROK)
	printf ("  no unwind, returning %#lx\n", (long) pc);
      return pc;
    }

  /* If we are not at the beginning of a function, then return now. */
  if ((pc & ~3) != u->region_start)
    {
      if (DEBUG_PA_GROK)
	printf ("  not at region_start, returning %#lx\n", (long) pc);
      return pc;
    }

  restart_gr = 0;
  restart_fr = 0;

restart:
  r1_val = 0;
  r1_base = 0;

  /* This is how much of a frame adjustment we need to account for.  */
  stack_remaining = u->Total_frame_size << 3;

  /* Magic register saves we want to know about.  */
  save_rp = u->Save_RP;
  save_sp = u->Save_SP;

  /* An indication that args may be stored into the stack.  Unfortunately
     the HPUX compilers tend to set this in cases where no args were
     stored too!.  Heh, and gcc tends to *not* set it when args *are*
     stored.  So for the case where we care about arg store, ie. when
     looking for the end of the prologue, assume there are args
     stored.  */
  args_stored = u->Args_stored || frame_info == 0;

  /* Turn the Entry_GR field into a bitmask.  */
  save_gr = 1 << (u->Entry_GR + 3);
  if (u->Save_SP)
    {
      /* Frame pointer (GR3) gets saved into a special location.  */
      save_gr -= 1 << 4;
      if (u->Entry_GR == 0)
	save_gr = 0;
    }
  else
    {
      save_gr -= 1 << 3;
    }
  save_gr &= ~restart_gr;

  /* Turn the Entry_FR field into a bitmask too.  */
  save_fr = (1 << (u->Entry_FR + 12)) - (1 << 12);
  save_fr &= ~restart_fr;

  /* The frame always represents the value of %sp at entry to the
     current function (and is thus equivalent to the "saved" stack
     pointer.  */
  if (fsr)
    fsr[PA_GR30_REGNUM] = frame_info->frame;

  if (DEBUG_PA_GROK > 1)
    printf ("  save_gr=%#lx, save_fr=%#lx, save_rp=%#lx, save_sp=%#lx, stack=%#lx, args=%#lx\n",
	    save_gr, save_fr, save_rp, save_sp, stack_remaining, args_stored);

  /* Loop until we find everything of interest or hit a branch.

     For unoptimized GCC code and for any HP CC code this will never ever
     examine any user instructions.

     For optimized GCC code we're faced with problems.  GCC will schedule
     its prologue and make prologue instructions available for delay slot
     filling.  The end result is user code gets mixed in with the prologue
     and a prologue instruction may be in the delay slot of the first branch
     or call.

     Some unexpected things are expected with debugging optimized code, so
     we allow this routine to walk past user instructions in optimized
     GCC code.  */
  
  for (final_iteration = 0;
       ((!fsr || pc <= frame_info->pc + 4)
	&& ((save_gr || save_fr || save_rp || save_sp
	     || stack_remaining > 0 || args_stored)));
       pc += 4)
    {
      unsigned int gr_num, fr_num;
      unsigned long old_stack_remaining, old_save_gr, old_save_fr;
      unsigned long old_save_rp, old_save_sp, next_inst;

      /* Save copies of all the triggers so we can compare them later
         (only for HPC).  */
      old_save_gr = save_gr;
      old_save_fr = save_fr;
      old_save_rp = save_rp;
      old_save_sp = save_sp;
      old_stack_remaining = stack_remaining;

      status = target_read_memory (pc, buf, 4);

      /* Yow! */
      if (status != 0)
	return pc;

      inst = extract_unsigned_integer (buf, 4);

      /* Note the interesting effects of this instruction.  */
      /* Save r1 value from addil high21,%r30 or addil high21,%r3.  */
      if ((inst & 0xffe00000) == ((0x0a << 26) | (30 << 21))
	  || (inst & 0xffe00000) == ((0x0a << 26) | (3 << 21)))
	{
	  r1_val = extract_21 (inst);
	  r1_base = (inst >> 21) & 0x1f;
	  continue;
	}
      /* ldo offset(%r30),%r1 or ldo offset(%r3),%r1  */
      else if ((inst & 0xffff0000) == ((0x0d << 26) | (30 << 21) | (1 << 16))
	       || (inst & 0xffff0000) == ((0x0d << 26) | (3 << 21) | (1 << 16)))
	{
	  r1_val = extract_14 (inst);
	  r1_base = (inst >> 21) & 0x1f;
	  continue;
	}
      /* ldo offset(%r1),%r1  */
      else if ((inst & 0xffff0000) == ((0x0d << 26) | (1 << 21) | (1 << 16)))
	{
	  r1_val += extract_14 (inst);
	  continue;
	}
      else
	{
	  int stack_adj = prologue_inst_adjust_sp (inst, r1_val);

	  stack_remaining -= stack_adj;
	  if (stack_adj && frame_info && frame_info->extra_info)
	    {
	      frame_info->extra_info->sp_adjust_insn = pc;
	      if (DEBUG_PA_GROK > 1)
		printf ("  found sp_adjust_insn = %#lx\n", (long) pc);
	    }
	  /* copy %sp,%r3  */
	  if (inst == 0x081e0243 && frame_info && frame_info->extra_info)
	    {
	      frame_info->extra_info->fp_adjust_insn = pc;
	      if (DEBUG_PA_GROK > 1)
		printf ("  found fp_adjust_insn = %#lx\n", (long) pc);
	    }
	}

      /* There are limited ways to store the return pointer into the
	 stack.  */
      if (inst == 0x6bc23fd9) /* stw rp,-0x14(sr0,sp) */
	{
	  save_rp = 0;
	  if (fsr)
	    fsr[PA_GR2_REGNUM] = frame_info->frame - 20;
	  if (frame_info && frame_info->extra_info)
	    frame_info->extra_info->rp_save_insn = pc;
	  if (DEBUG_PA_GROK > 1)
	    printf ("  found rp_save_insn = %#lx\n", (long) pc);
	  continue;
	}
      else if (inst == 0x0fc212c1) /* std rp,-0x10(sr0,sp) */
	{
	  save_rp = 0;
	  if (fsr)
	    fsr[PA_GR2_REGNUM] = frame_info->frame - 16;
	  if (frame_info && frame_info->extra_info)
	    frame_info->extra_info->rp_save_insn = pc;
	  if (DEBUG_PA_GROK > 1)
	    printf ("  found rp_save_insn = %#lx\n", (long) pc);
	  continue;
	}

      /* Note if we saved SP into the stack.  This also happens to indicate
	 the location of the saved frame pointer.  At this time the HP
	 compilers never bother to save SP into the stack.  */
      if ((inst & 0xffffc000) == 0x6fc10000  /* stw,ma r1,N(sr0,sp) */
          || (inst & 0xffffc00c) == 0x73c10008) /* std,ma r1,N(sr0,sp) */
	{
	  if (fsr)
	    fsr[PA_GR3_REGNUM] = frame_info->frame;
	  save_sp = 0;
	  continue;
	}

      /* Are we loading some register with an offset from the argument
         pointer?  */
      if ((inst & 0xffe00000) == 0x37a00000 /* ldo offs(ret1),rn */
	  || (inst & 0xffffffe0) == 0x081d0240 /* copy ret1,rn */)
	continue;

      /* Account for general and floating-point register saves.  */
      gr_num = inst_saves_gr (inst);
      save_gr &= ~(1 << gr_num);
      if (fsr
	  && (gr_num > 3 || (gr_num == 3 && !u->Save_SP))
	  && gr_num < u->Entry_GR + 3)
	{
	  /* stwm with a positive displacement is a post modify.  */
	  if ((inst & ((0x3f << 26) | 1)) == ((0x1b << 26) | 0))
	    fsr[gr_num] = frame_info->frame;
	  /* A std has explicit post_modify forms.  */
	  else if ((inst & 0xfc0000c0) == ((0x1c << 26) | 8))
	    fsr[gr_num] = frame_info->frame;
	  else
	    {
	      CORE_ADDR offset;
	      int base_reg;

	      if ((inst & (0x3f << 26)) == 0x1c << 26)
		offset = ((inst >> 1) & 0x1ff8) - ((inst & 1) << 13);
	      else if ((inst & (0x3f << 26)) == 0x03 << 26)
		offset = low_sign_extend (inst, 5);
	      else
		offset = extract_14 (inst);

	      base_reg = (inst >> 21) & 0x1f;

	      /* Handle code with and without frame pointers.  */
	      if (base_reg == 1)
		offset += r1_val;
	      if ((base_reg == 1 && r1_base == 30)
		  || base_reg == 30)
		offset += u->Total_frame_size << 3;

	      fsr[gr_num] = frame_info->frame + offset;
	    }
	}

      fr_num = inst_saves_fr (inst);
      save_fr &= ~(1 << fr_num);

      /* GCC handles callee saved FP regs a little differently.  

         It emits an instruction to put the value of the start of
         the FP store area into %r1.  It then uses fstds,ma with
         a basereg of %r1 for the stores.

         HP CC emits them at the current stack pointer modifying
         the stack pointer as it stores each register.  */

      if (fsr
	  && fr_num >= 12 && fr_num <= 21)
	{
	  int base_reg = (inst >> 21) & 0x1f;

	  if (base_reg == 30)
	    {
	      /* HP CC FP register store.  */
	      fsr[2 * fr_num + PA_FR0_REGNUM]
		= (frame_info->frame
		   + (u->Total_frame_size << 3) - old_stack_remaining);
	    }
	  else if (base_reg == 1)
	    {
	      int offset = r1_val;

	      if (r1_base == 30)
		offset += u->Total_frame_size << 3;

	      fsr[2 * fr_num + PA_FR0_REGNUM]
		= frame_info->frame + offset;

	      /* I suppose we should really check the offset in the
		 instruction, but it will always be eight.  */
	      r1_val += 8;
	    }
	}

      /* Quit if we hit any kind of branch the previous iteration.  */
      if (final_iteration)
	{
	  pc -= 4;
	  break;
	}

      /* We want to look precisely one instruction beyond the branch
	 if we have not found everything yet.  */
      if (is_branch (inst))
	{
	  /* If the branch delay slot is nullified, don't look at it.  */
	  if ((inst & 2) != 0
	      && inst >= (0x38 << 26) && inst < (0x3b << 26))
	    break;
	  final_iteration = 1;
	}

      /* Ugh.  Also account for argument stores into the stack.
         Unfortunately args_stored only tells us that some arguments
         where stored into the stack.  Not how many or what kind!

         This is a kludge as on the HP compiler sets this bit and it
         never does prologue scheduling.  So once we see one, skip past
         all of them.  */
      if (args_stored
	  && ((gr_num >= (REGISTER_SIZE == 8 ? 19 : 23) && gr_num <= 26)
	      || (fr_num >= 4 && fr_num <= (REGISTER_SIZE == 8 ? 11 : 7))))
	{
	  do
	    {
	      pc += 4;
	      status = target_read_memory (pc, buf, 4);
	      if (status != 0)
		return pc;
	      inst = extract_unsigned_integer (buf, 4);
	      gr_num = inst_saves_gr (inst);
	      fr_num = inst_saves_fr (inst);
	    }
	  while ((gr_num >= (REGISTER_SIZE == 8 ? 19 : 23) && gr_num <= 26)
		 || (fr_num >= 4 && fr_num <= (REGISTER_SIZE == 8 ? 11 : 7))
		 || (inst & 0xfc000000) == (0x0d << 26) /* ldo */);

	  args_stored = 0;
	  pc -= 4;
	  continue;
	}

      /* What a crock.  The HP compilers set args_stored even if no
         arguments were stored into the stack (boo hiss).  This could
         cause this code to then skip a bunch of user insns (up to the
         first branch).

         To combat this we try to identify when args_stored was bogusly
         set and clear it.   We only do this when args_stored is nonzero,
         all other resources are accounted for, and nothing changed on
         this pass.  */
      if (args_stored
	  && !(save_gr || save_fr || save_rp || save_sp || stack_remaining > 0)
	  && old_save_gr == save_gr && old_save_fr == save_fr
	  && old_save_rp == save_rp && old_save_sp == save_sp
	  && old_stack_remaining == stack_remaining)
	{
	  if (DEBUG_PA_GROK > 1)
	    printf ("  toodleoo");
	  break;
	}
    }

  if (DEBUG_PA_GROK > 2 && fsr)
    {
      int i;
      for (i = PA_GR0_REGNUM; i <= PA_GR31_REGNUM; i++)
	printf (" [r%d]=%#lx", i - PA_GR0_REGNUM, fsr[i]);
      for (i = PA_FR0_REGNUM; i <= PA_FR31_REGNUM; i += 2)
	printf (" [fr%d]=%#lx,%#lx", (i-PA_FR0_REGNUM) / 2, fsr[i], fsr[i+1]);
      printf ("\n");
    }
  if (DEBUG_PA_GROK)
    printf ("  pc=%#lx\n", (long) pc);

  /* We've got a tenative location for the end of the prologue.  However
     because of limitations in the unwind descriptor mechanism we may
     have gone too far into user code looking for the save of a register
     that does not exist.  So, if there were registers we expected to be
     saved but they never were, mask them out and restart.

     This should only happen in optimized code, and should be very rare.  */
  if (frame_info == NULL
      && (save_gr || save_fr) && !(restart_fr || restart_gr))
    {
      pc = orig_pc;
      restart_gr = save_gr;
      restart_fr = save_fr;
      goto restart;
    }

  return pc;
}

static void
pa_init_extra_frame_info (int fromleaf, struct frame_info *frame)
{
  int framesize;

  if (DEBUG_PA_FRAME)
    {
      printf ("init_extra_frame_info (%d, %p)\n", fromleaf, frame);
      printf ("  frame->prev = %p, frame->next = %p\n",
	      frame->prev, frame->next);
      printf ("  frame->frame = %#lx, frame->pc = %#lx\n",
	      (long) frame->frame, (long) frame->pc);
    }

  frame->extra_info = frame_obstack_alloc (sizeof (*frame->extra_info));
  memset (frame->extra_info, 0, sizeof (*frame->extra_info));

  /* If the next frame represents a frameless function invocation
     then we have to do some adjustments that are normally done by
     FRAME_CHAIN.  (FRAME_CHAIN is not called in this case.)

     One might think frameless innermost frames should have
     a frame->frame that is the same as the parent's frame->frame.
     That is wrong; frame->frame in that case should be the *high*
     address of the parent's frame.  It's complicated as hell to
     explain, but the parent *always* creates some stack space for
     the child.  So the child actually does have a frame of some
     sorts, and its base is the high address in its parent's frame.  */
  if (fromleaf)
    {
      framesize = find_proc_framesize (FRAME_SAVED_PC (frame->next));

      /* Now adjust our base frame accordingly.  If we have a frame pointer
         use it, else subtract the size of this frame from the current
         frame.  (we always want frame->frame to point at the lowest address
         in the frame).  */
      if (framesize == -1)
	frame->frame = TARGET_READ_FP ();
      else
	frame->frame -= framesize;

      if (DEBUG_PA_FRAME)
	printf ("  framesize = %#x, frame->frame = %#lx\n",
		framesize, (long) frame->frame);
      return;
    }

  /* Adjustments for innermost frames.  */
  if (!frame->next)
    {
      if (pc_in_linker_stub (frame->pc, NULL))
	{
	  /* stubs always have frame->frame == sp.  */
	  framesize = 0;
	}
      else
	{
	  CORE_ADDR start_pc;

	  /* gcc schedules the function prologue when optimizing,
	     which can lead to code from the function body moving
	     before the stack adjustment.  That means we may have a
	     breakpoint set before the stack has been adjusted.  In
	     any case, we can stepi anywhere.  Thus, for innermost
	     frames we need to rummage through the prologue to see
	     exactly where we are.  For other frames in the call
	     chain, we don't need to worry as no function will be
	     called before the prologue has fully executed.  */
	  start_pc = get_pc_function_start (frame->pc);
	  pa_grok_prologue (start_pc, frame);
	  framesize = find_proc_framesize (frame->pc);
	}

      if (DEBUG_PA_FRAME> 1)
	printf ("  sp_adjust_insn = %#lx, fp_adjust_insn = %#lx, rp_save_insn = %#lx\n",
		(long) frame->extra_info->sp_adjust_insn,
		(long) frame->extra_info->fp_adjust_insn,
		(long) frame->extra_info->rp_save_insn);

      if (framesize == -1)
	{
	  if (frame->pc > frame->extra_info->fp_adjust_insn)
	    frame->frame = TARGET_READ_FP ();
	  else
	    frame->frame = read_register (PA_GR30_REGNUM);
	}
      else
	{
	  frame->frame = read_register (PA_GR30_REGNUM);
	  if (frame->pc > frame->extra_info->sp_adjust_insn)
	    frame->frame -= framesize;
	}

      if (DEBUG_PA_FRAME)
	printf ("  framesize = %#x, frame->frame = %#lx\n",
		framesize,
		(long) frame->frame);
    }
}

/* We always have some sort of frame.  See above.  */
static int
pa_prologue_frameless_p (CORE_ADDR ip)
{
  return 0;
}

/* Given a GDB frame, determine the address of the calling function's
   frame.  This will be used to create a new GDB frame struct, and
   then INIT_FRAME_PC_FIRST and INIT_EXTRA_FRAME_INFO will be called
   for the new frame.

   This may involve searching through prologues for several functions
   at boundaries where GCC calls HP C code, or where code which has
   a frame pointer calls code without a frame pointer.

   In the case of the PA-RISC, the frame's nominal address is the address
   of a 4-byte word containing the calling frame's address (previous FP).  */

static CORE_ADDR
pa_frame_chain (struct frame_info *frame)
{
  int my_framesize, caller_framesize;
  struct unwind_table_entry *u;
  CORE_ADDR frame_base;
  struct frame_info *tmp_frame;
  CORE_ADDR caller_pc;
  struct minimal_symbol *min_frame_symbol;
  struct symbol *frame_symbol;
  char *frame_symbol_name;
  CORE_ADDR ret;

  if (DEBUG_PA_FRAME)
    {
      printf ("frame_chain (%p)\n", frame);
      printf ("  frame->frame = %#lx, frame->pc = %#lx\n",
	      (long) frame->frame, (long) frame->pc);
    }

  /* If this is a threaded application, and we see the
     routine "__pthread_exit", treat it as the stack root
     for this thread. */
  min_frame_symbol = lookup_minimal_symbol_by_pc (frame->pc);
  frame_symbol = find_pc_function (frame->pc);

  if ((min_frame_symbol != 0) /* && (frame_symbol == 0) */ )
    {
      /* The test above for "no user function name" would defend
         against the slim likelihood that a user might define a
         routine named "__pthread_exit" and then try to debug it.

         If it weren't commented out, and you tried to debug the
         pthread library itself, you'd get errors.

         So for today, we don't make that check. */
      frame_symbol_name = SYMBOL_NAME (min_frame_symbol);
      if (frame_symbol_name != 0)
	{
	  if (0 == strncmp (frame_symbol_name,
			    THREAD_INITIAL_FRAME_SYMBOL,
			    THREAD_INITIAL_FRAME_SYM_LEN))
	    {
	      /* Pretend we've reached the bottom of the stack. */
	      return (CORE_ADDR) 0;
	    }
	}
    }				/* End of hacky code for threads. */

  if (PA_IN_INTERRUPT_HANDLER (frame->pc))
    frame_base = gdbarch_tdep (current_gdbarch)->frame_base_before_interrupt (frame);
  else if (frame->signal_handler_caller
	   && gdbarch_tdep (current_gdbarch)->frame_base_before_sigtramp)
    frame_base = gdbarch_tdep (current_gdbarch)->frame_base_before_sigtramp (frame);
  else
    frame_base = frame->frame;

  /* Get frame sizes for the current frame and the frame of the 
     caller.  */
  my_framesize = find_proc_framesize (frame->pc);
  caller_pc = FRAME_SAVED_PC (frame);

  if (DEBUG_PA_FRAME > 1)
    printf ("  frame_base = %#lx, my_framesize = %#x, caller_pc = %#lx\n",
	    (long) frame_base, my_framesize, (long) caller_pc);

  /* If we can't determine the caller's PC, then it's not likely we can
     really determine anything meaningful about its frame.  We'll consider
     this to be stack bottom. */
  if (caller_pc == (CORE_ADDR) 0)
    return (CORE_ADDR) 0;

  caller_framesize = find_proc_framesize (caller_pc);

  if (DEBUG_PA_FRAME > 1)
    printf ("  caller_framesize = %#x\n", caller_framesize);

  /* If caller does not have a frame pointer, then its frame
     can be found at current_frame - caller_framesize.  */
  if (caller_framesize != -1)
    {
      ret = frame_base - caller_framesize;
      if (DEBUG_PA_FRAME)
	printf ("  returning %#lx\n", (long) ret);
      return ret;
    }

  /* Both caller and callee have frame pointers.
     The previous frame pointer is found at the top of the current frame.  */
  if (caller_framesize == -1 && my_framesize == -1)
    {
      ret = read_memory_integer (frame_base, REGISTER_SIZE);
      if (DEBUG_PA_FRAME)
	printf ("  returning %#lx\n", (long) ret);
      return ret;
    }

  /* Caller has a frame pointer, but callee does not.  This is a little
     more difficult as GCC and HP C lay out locals and callee register save
     areas very differently.

     The previous frame pointer could be in a register, or in one of 
     several areas on the stack.

     Walk from the current frame to the innermost frame examining 
     unwind descriptors to determine if %r3 ever gets saved into the
     stack.  If so return whatever value got saved into the stack.
     If it was never saved in the stack, then the value in %r3 is still
     valid, so use it. 

     We use information from unwind descriptors to determine if %r3
     is saved into the stack (Entry_GR field has this information).  */

  for (tmp_frame = frame; tmp_frame; tmp_frame = tmp_frame->next)
    {
      u = find_unwind_entry (tmp_frame->pc);

      if (!u)
	{
	  /* We could find this information by examining prologues.  I don't
	     think anyone has actually written any tools (not even "strip")
	     which leave them out of an executable, so maybe this is a moot
	     point.  */
	  /* ??rehrauer: Actually, it's quite possible to stepi your way into
	     code that doesn't have unwind entries.  For example, stepping into
	     the dynamic linker will give you a PC that has none.  Thus, I've
	     disabled this warning. */
#if 0
	  warning ("Unable to find unwind for PC 0x%x -- Help!", tmp_frame->pc);
#endif
	  ret = 0;
	  if (DEBUG_PA_FRAME)
	    printf ("  returning %#lx\n", (long) ret);
	  return ret;
	}

      if (u->Save_SP
	  || tmp_frame->signal_handler_caller
	  || PA_IN_INTERRUPT_HANDLER (tmp_frame->pc))
	break;

      /* Entry_GR specifies the number of callee-saved general registers
         saved in the stack.  It starts at %r3, so %r3 would be 1.  */
      if (u->Entry_GR >= 1)
	{
	  /* The unwind entry claims that r3 is saved here.  However,
	     in optimized code, GCC often doesn't actually save r3.
	     We'll discover this if we look at the prologue.  */
	  if (!tmp_frame->saved_regs)
	    FRAME_INIT_SAVED_REGS (tmp_frame);
	  if (tmp_frame->saved_regs[PA_GR3_REGNUM])
	    break;
	}
    }

  if (tmp_frame)
    {
      /* We may have walked down the chain into a function with a frame
         pointer.  */
      if (DEBUG_PA_FRAME > 1)
	printf ("  reading fp from frame %p", tmp_frame);
      if (u->Save_SP
	  && !tmp_frame->signal_handler_caller
	  && !PA_IN_INTERRUPT_HANDLER (tmp_frame->pc))
	{
	  if (DEBUG_PA_FRAME > 1)
	    printf ("->frame\n");
	  ret = read_memory_integer (tmp_frame->frame, REGISTER_SIZE);
	}
      /* %r3 was saved somewhere in the stack.  Dig it out.  */
      else
	{
	  /* Sick.

	     For optimization purposes many kernels don't save the
	     callee saved registers into the save_state structure upon
	     entry into the kernel for a syscall; the optimization
	     is usually turned off if the process is being traced so
	     that the debugger can get full register state for the
	     process.

	     This scheme works well except for two cases:

	     * Attaching to a process when the process is in the
	     kernel performing a system call (debugger can't get
	     full register state for the inferior process since
	     the process wasn't being traced when it entered the
	     system call).

	     * Register state is not complete if the system call
	     causes the process to core dump.

	     The following heinous code is an attempt to deal with
	     the lack of register state in a core dump.  It will
	     fail miserably if the function which performs the
	     system call has a variable sized stack frame.  */

	  /* Abominable hack.  */
	  if (current_target.to_has_execution == 0
	      && PA_IN_SYSCALL (tmp_frame->saved_regs)
	      && (u = find_unwind_entry (FRAME_SAVED_PC (frame))) != NULL)
	    {
	      ret = frame_base - (u->Total_frame_size << 3);
	    }
	  else
	    {
	      if (DEBUG_PA_FRAME > 1)
		printf ("->saved_regs\n");
	      ret = read_memory_integer (tmp_frame->saved_regs[PA_GR3_REGNUM],
					 REGISTER_SIZE);
	    }
	}
    }
  else
    {
      /* Get the innermost frame.  */
      tmp_frame = frame;
      while (tmp_frame->next != NULL)
	tmp_frame = tmp_frame->next;

      /* Abominable hack.  See above.  */
      if (current_target.to_has_execution == 0
	  && PA_IN_SYSCALL (tmp_frame->saved_regs))
	{
	  u = find_unwind_entry (FRAME_SAVED_PC (frame));
	  if (u)
	    ret = frame_base - (u->Total_frame_size << 3);
	  else if (tmp_frame->saved_regs[PA_GR3_REGNUM])
	    ret = read_memory_integer (tmp_frame->saved_regs[PA_GR3_REGNUM],
				       REGISTER_SIZE);
	  else
	    ret = TARGET_READ_FP ();
	}
      else
	{
	  /* The value in %r3 was never saved into the stack (thus %r3
	     still holds the value of the previous frame pointer).  */
	  if (DEBUG_PA_FRAME > 1)
	    printf ("  reading fp reg\n");
	  ret = TARGET_READ_FP ();
	}
    }
  if (DEBUG_PA_FRAME)
    printf ("  returning %#lx\n", (long) ret);
  return ret;
}

/* Return the base address used when calculating arg symbol offsets.  This
   doesn't have anything to do with the actual start of the args.  */
static CORE_ADDR
pa_frame_args_address (struct frame_info *fi)
{
  return fi->frame;
}

/* As for above, but for local vars.  */
static CORE_ADDR
pa_frame_locals_address (struct frame_info *fi)
{
  return fi->frame;
}

/* Stack grows upward.  */
static int
pa_inner_than (CORE_ADDR lhs, CORE_ADDR rhs)
{
  return lhs > rhs;
}

/* On hppa the stack must always be kept 64-bit aligned.  */
static CORE_ADDR
pa_stack_align (CORE_ADDR sp)
{
  return (sp+7) & -8;
}

/* On hppa64 the stack must always be kept 128-bit aligned.  */
static CORE_ADDR
pa64_stack_align (CORE_ADDR sp)
{
  return (sp+15) & -16;
}

/* On the PA, any pass-by-value structure > 8 bytes is actually
   passed via a pointer regardless of its type or the compiler
   used.  */
static int
pa_reg_struct_has_addr (int gcc_p, struct type *type)
{
  return TYPE_LENGTH (type) > 8;
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

void
pa_extract_return_value (struct type *type, char *regbuf, char *valbuf)
{
  /* Extract from an array REGBUF containing the (raw) register state
     a function return value of type TYPE, and copy that, in virtual
     format, into VALBUF.  */

    if (TYPE_CODE (type) == TYPE_CODE_FLT && !SOFT_FLOAT)
      memcpy (valbuf,
	      regbuf + REGISTER_BYTE (PA_FR4_REGNUM),
	      TYPE_LENGTH (type));
    else
      memcpy (valbuf,
	      (regbuf + REGISTER_BYTE (PA_GR28_REGNUM)
	       + 3 - ((TYPE_LENGTH (type) - 1) & 3)),
	      TYPE_LENGTH (type));
}

void
pa64_extract_return_value (struct type *type, char *regbuf, char *valbuf)
{
  /* Floats are returned in FR4R, doubles in FR4
     integral values are in r28, padded on the left
     aggregates less that 65 bits are in r28, right padded
     aggregates up to 128 bits are in r28 and r29, right padded.  */ 
  if (TYPE_CODE (type) == TYPE_CODE_FLT && !SOFT_FLOAT)
    memcpy (valbuf,
	    regbuf + REGISTER_BYTE (PA_FR4_REGNUM) + 8 - TYPE_LENGTH (type),
	    TYPE_LENGTH (type));
  else if  (is_integral_type (type) || SOFT_FLOAT)
    memcpy (valbuf,
	    regbuf + REGISTER_BYTE (PA_GR28_REGNUM) + 8 - TYPE_LENGTH (type),
	    TYPE_LENGTH (type));
  else if (TYPE_LENGTH (type) <= 16)
    memcpy (valbuf,
	    regbuf + REGISTER_BYTE (PA_GR28_REGNUM),
	    TYPE_LENGTH (type));
}

static void
find_dummy_frame_regs (struct frame_info *frame, CORE_ADDR *frame_saved_regs)
{
  CORE_ADDR fp = frame->frame;
  int i;
  int reg_size = REGISTER_SIZE;

  /* The 32bit and 64bit ABIs save RP into different locations.  */
  if (reg_size == 8)
    frame_saved_regs[PA_GR2_REGNUM] = fp - 16;
  else
    frame_saved_regs[PA_GR2_REGNUM] = fp - 20;

  frame_saved_regs[PA_GR3_REGNUM] = fp;

  fp += 2 * reg_size;

  for (i = PA_GR1_REGNUM; i <= PA_GR31_REGNUM; i++)
    if (i != PA_GR2_REGNUM && i != PA_GR3_REGNUM)
      {
	frame_saved_regs[i] = fp;
	fp += reg_size;
      }

  /* This is not necessary or desirable for the 64bit ABI.  */
  if (reg_size != 8)
    fp += reg_size;

  for (i = PA_FR0_REGNUM; i < NUM_REGS; i++, fp += 4)
    frame_saved_regs[i] = fp;

  frame_saved_regs[PA_IPSW_REGNUM] = fp; fp += reg_size;
  frame_saved_regs[PA_SAR_REGNUM] = fp; fp += reg_size;
  frame_saved_regs[PA_PCOQ_HEAD_REGNUM] = fp; fp += reg_size;
  frame_saved_regs[PA_PCSQ_HEAD_REGNUM] = fp; fp += reg_size;
  frame_saved_regs[PA_PCOQ_TAIL_REGNUM] = fp; fp += reg_size;
  frame_saved_regs[PA_PCSQ_TAIL_REGNUM] = fp;
}

/* After returning to a dummy on the stack, restore the instruction
   queue space registers. */

static int
restore_pc_queue (CORE_ADDR *fsr)
{
  CORE_ADDR pc = read_pc ();
  CORE_ADDR new_pc = read_memory_integer (fsr[PA_PCOQ_HEAD_REGNUM],
					  REGISTER_SIZE);
  struct target_waitstatus w;
  int insn_count;

  /* HPUX doesn't let us set the space registers or the space
     registers of the PC queue through ptrace. Boo, hiss.

     But the call dummy has this sequence of instructions at the break:
     mtsp r21, sr0
     ble,n 0(sr0, r22)
     nop

     So, load up the registers and single step until we are in the
     right place. */

  write_register (PA_GR21_REGNUM,
		  read_memory_integer (fsr[PA_PCSQ_HEAD_REGNUM],
				       REGISTER_SIZE));
  write_register (PA_GR22_REGNUM, new_pc);

  for (insn_count = 0; insn_count < 3; insn_count++)
    {
      /* FIXME: What if the inferior gets a signal right now?  Want to
         merge this into wait_for_inferior (as a special kind of
         watchpoint?  By setting a breakpoint at the end?  Is there
         any other choice?  Is there *any* way to do this stuff with
         ptrace() or some equivalent?).  */
      resume (1, 0);
      target_wait (inferior_ptid, &w);

      if (w.kind == TARGET_WAITKIND_SIGNALLED)
	{
	  stop_signal = w.value.sig;
	  terminal_ours_for_output ();
	  printf_unfiltered ("\nProgram terminated with signal %s, %s.\n",
			     target_signal_to_name (stop_signal),
			     target_signal_to_string (stop_signal));
	  gdb_flush (gdb_stdout);
	  return 0;
	}
    }
  target_terminal_ours ();
  target_fetch_registers (-1);
  return 1;
}

void
pa_pop_frame (void)
{
  struct frame_info *frame = get_current_frame ();
  CORE_ADDR fp, npc, target_pc;
  int regnum;
  CORE_ADDR *fsr;
  char reg_buffer[8];
  int reg_size = REGISTER_SIZE;

  fp = FRAME_FP (frame);
  fsr = frame->saved_regs;

#ifndef NO_PC_SPACE_QUEUE_RESTORE
  if (fsr[PA_IPSW_REGNUM])	/* Restoring a call dummy frame */
    restore_pc_queue (fsr);
#endif

  for (regnum = PA_GR31_REGNUM; regnum > PA_GR0_REGNUM; regnum--)
    if (fsr[regnum])
      {
	read_memory (fsr[regnum], reg_buffer, reg_size);
	write_register_gen (regnum, reg_buffer);
      }

  for (regnum = NUM_REGS - 1; regnum >= PA_FR0_REGNUM; regnum--)
    if (fsr[regnum])
      {
	read_memory (fsr[regnum], reg_buffer, 4);
	write_register_gen (regnum, reg_buffer);
      }

  if (fsr[PA_IPSW_REGNUM])
    {
      read_memory (fsr[PA_IPSW_REGNUM], reg_buffer, reg_size);
      write_register_gen (PA_IPSW_REGNUM, reg_buffer);
    }

  if (fsr[PA_SAR_REGNUM])
    {
      read_memory (fsr[PA_SAR_REGNUM], reg_buffer, reg_size);
      write_register_gen (PA_SAR_REGNUM, reg_buffer);
    }

  /* If the PC was explicitly saved, then just restore it.  */
  if (fsr[PA_PCOQ_TAIL_REGNUM])
    {
      npc = read_memory_integer (fsr[PA_PCOQ_TAIL_REGNUM], reg_size);
      write_register (PA_PCOQ_TAIL_REGNUM, npc);
      npc = read_memory_integer (fsr[PA_PCOQ_HEAD_REGNUM], reg_size);
      write_register (PA_PCOQ_HEAD_REGNUM, npc);
    }
  /* Else use the value in %rp to set the new PC.  */
  else
    {
      npc = read_register (PA_GR2_REGNUM);
      write_pc (npc);
    }

  read_memory (fp, reg_buffer, reg_size);
  write_register_gen (PA_GR3_REGNUM, reg_buffer);

  if (fsr[PA_IPSW_REGNUM])	/* call dummy */
    write_register (PA_GR30_REGNUM, fp - 48);
  else
    write_register (PA_GR30_REGNUM, fp);

  /* The PC we just restored may be inside a return trampoline.  If so
     we want to restart the inferior and run it through the trampoline.

     Do this by setting a momentary breakpoint at the location the
     trampoline returns to. 

     Don't skip through the trampoline if we're popping a dummy frame.  */
  target_pc = SKIP_TRAMPOLINE_CODE (npc & ~3) & ~3;
  if (target_pc && !fsr[PA_IPSW_REGNUM])
    {
      struct symtab_and_line sal;
      struct breakpoint *breakpoint;
      struct cleanup *old_chain;

      /* Set up our breakpoint.   Set it to be silent as the MI code
         for "return_command" will print the frame we returned to.  */
      sal = find_pc_line (target_pc, 0);
      sal.pc = target_pc;
      breakpoint = set_momentary_breakpoint (sal, NULL, bp_finish);
      breakpoint->silent = 1;

      /* So we can clean things up.  */
      old_chain = make_cleanup_delete_breakpoint (breakpoint);

      /* Start up the inferior.  */
      clear_proceed_status ();
      proceed_to_finish = 1;
      proceed ((CORE_ADDR) -1, TARGET_SIGNAL_DEFAULT, 0);

      /* Perform our cleanups.  */
      do_cleanups (old_chain);
    }
  flush_cached_frames ();
}

/* This function pushes a stack frame with arguments as part of the
   inferior function calling mechanism.

   This is the version for the PA64, in which later arguments appear
   at higher addresses.  (The stack always grows towards higher
   addresses.)

   We simply allocate the appropriate amount of stack space and put
   arguments into their proper slots.  The call dummy code will copy
   arguments into registers as needed by the ABI.

   This ABI also requires that the caller provide an argument pointer
   to the callee, so we do that too.  */

#define PA64_REGISTER_SIZE 8
#define REG_PARM_STACK_SPACE64 (8 * 8)

CORE_ADDR
pa64_push_arguments (int nargs, struct value **args, CORE_ADDR sp, int
		     struct_return, CORE_ADDR struct_addr)
{
  /* array of arguments' offsets */
  int *offset = (int *) alloca (nargs * sizeof (int));

  /* array of arguments' lengths: real lengths in bytes, not aligned to
     word size */
  int *lengths = (int *) alloca (nargs * sizeof (int));

  /* The value of SP as it was passed into this function after
     aligning.  */
  CORE_ADDR orig_sp = STACK_ALIGN (sp);

  /* The number of stack bytes occupied by the current argument.  */
  int bytes_reserved;

  /* The total number of bytes reserved for the arguments.  */
  int cum_bytes_reserved = 0;

  /* Similarly, but aligned.  */
  int cum_bytes_aligned = 0;
  int i;

  /* Iterate over each argument provided by the user.  */
  for (i = 0; i < nargs; i++)
    {
      struct type *arg_type = VALUE_TYPE (args[i]);

      /* Integral scalar values smaller than a register are padded on
         the left.  We do this by promoting them to full-width,
         although the ABI says to pad them with garbage.  */
      if (is_integral_type (arg_type)
	  && TYPE_LENGTH (arg_type) < PA64_REGISTER_SIZE)
	{
	  args[i] = value_cast ((TYPE_UNSIGNED (arg_type)
				 ? builtin_type_unsigned_long
				 : builtin_type_long),
				args[i]);
	  arg_type = VALUE_TYPE (args[i]);
	}

      lengths[i] = TYPE_LENGTH (arg_type);

      /* Align the size of the argument to the word size for this
	 target.  */
      bytes_reserved = ((lengths[i] + PA64_REGISTER_SIZE - 1)
			& -PA64_REGISTER_SIZE);

      offset[i] = cum_bytes_reserved;

      /* Aggregates larger than eight bytes (the only types larger
         than eight bytes we have) are aligned on a 16-byte boundary,
         possibly padded on the right with garbage.  This may leave an
         empty word on the stack, and thus an unused register, as per
         the ABI.  */
      if (bytes_reserved > 8)
	{
	  /* Round up the offset to a multiple of two slots.  */
	  int new_offset = ((offset[i] + 2*PA64_REGISTER_SIZE-1)
			    & -(2*PA64_REGISTER_SIZE));

	  /* Note the space we've wasted, if any.  */
	  bytes_reserved += new_offset - offset[i];
	  offset[i] = new_offset;
	}

      cum_bytes_reserved += bytes_reserved;
    }

  /* CUM_BYTES_RESERVED already accounts for all the arguments
     passed by the user.  However, the ABIs mandate minimum stack space
     allocations for outgoing arguments.

     The ABIs also mandate minimum stack alignments which we must
     preserve.  */
  cum_bytes_aligned = STACK_ALIGN (cum_bytes_reserved);
  sp += max (cum_bytes_aligned, REG_PARM_STACK_SPACE64);

  /* Now write each of the args at the proper offset down the stack.  */
  for (i = 0; i < nargs; i++)
    write_memory (orig_sp + offset[i], VALUE_CONTENTS (args[i]), lengths[i]);

  /* For the PA64 we must pass a pointer to the outgoing argument list.
     The ABI mandates that the pointer should point to the first byte of
     storage beyond the register flushback area.  */
  write_register (PA_GR29_REGNUM, orig_sp + REG_PARM_STACK_SPACE64);

  /* The stack will have 64 bytes of additional space for a frame marker.  */
  return sp + 64;
}

/* This function pushes a stack frame with arguments as part of the
   inferior function calling mechanism.

   This is the version of the function for the 32-bit PA machines, in
   which later arguments appear at lower addresses.  (The stack always
   grows towards higher addresses.)

   We simply allocate the appropriate amount of stack space and put
   arguments into their proper slots.  The call dummy code will copy
   arguments into registers as needed by the ABI. */
   
#define PA32_REGISTER_SIZE 4
#define REG_PARM_STACK_SPACE32 (4 * 4)

CORE_ADDR
pa_push_arguments (int nargs, struct value **args, CORE_ADDR sp, int
		   struct_return, CORE_ADDR struct_addr)
{
  /* array of arguments' offsets */
  int *offset = (int *) alloca (nargs * sizeof (int));

  /* array of arguments' lengths: real lengths in bytes, not aligned to
     word size */
  int *lengths = (int *) alloca (nargs * sizeof (int));

  /* The number of stack bytes occupied by the current argument.  */
  int bytes_reserved;

  /* The total number of bytes reserved for the arguments.  */
  int cum_bytes_reserved = 0;

  /* Similarly, but aligned.  */
  int cum_bytes_aligned = 0;
  int i;

  /* Iterate over each argument provided by the user.  */
  for (i = 0; i < nargs; i++)
    {
      lengths[i] = TYPE_LENGTH (VALUE_TYPE (args[i]));

      /* Align the size of the argument to the word size for this
	 target.  */
      bytes_reserved = ((lengths[i] + PA32_REGISTER_SIZE - 1)
			& -PA32_REGISTER_SIZE);

      offset[i] = cum_bytes_reserved + lengths[i];

      /* If the argument is a double word argument, then it needs to be
	 double word aligned.  */
      if ((bytes_reserved == 2 * PA32_REGISTER_SIZE)
	  && (offset[i] % 2 * PA32_REGISTER_SIZE))
	{
	  int new_offset = 0;
	  /* BYTES_RESERVED is already aligned to the word, so we put
	     the argument at one word more down the stack.

	     This will leave one empty word on the stack, and one unused
	     register as mandated by the ABI.  */
	  new_offset = ((offset[i] + 2 * PA32_REGISTER_SIZE - 1)
			& -(2 * PA32_REGISTER_SIZE));

	  if ((new_offset - offset[i]) >= 2 * PA32_REGISTER_SIZE)
	    {
	      bytes_reserved += PA32_REGISTER_SIZE;
	      offset[i] += PA32_REGISTER_SIZE;
	    }
	}

      cum_bytes_reserved += bytes_reserved;
    }

  /* CUM_BYTES_RESERVED already accounts for all the arguments passed
     by the user.  However, the ABI mandates minimum stack space
     allocations for outgoing arguments.

     The ABI also mandates minimum stack alignments which we must
     preserve.  */
  cum_bytes_aligned = STACK_ALIGN (cum_bytes_reserved);
  sp += max (cum_bytes_aligned, REG_PARM_STACK_SPACE32);

  /* Now write each of the args at the proper offset down the stack.
     ?!? We need to promote values to a full register instead of skipping
     words in the stack.  */
  for (i = 0; i < nargs; i++)
    write_memory (sp - offset[i], VALUE_CONTENTS (args[i]), lengths[i]);

  /* The stack will have 32 bytes of additional space for a frame marker.  */
  return sp + 32;
}

/* We don't need to push the return address as we use a call dummy.  */
CORE_ADDR
pa_push_return_address (CORE_ADDR pc, CORE_ADDR sp)
{
  return sp;
}

/* elz: This function returns a value which is built looking at the given
   address.  It is called from call_function_by_hand, in case we need to
   return a value which is larger than 64 bits, and it is stored in the
   stack rather than in the registers r28 and r29 or fr4.  This function
   does the same stuff as value_being_returned in values.c, but gets the
   value from the stack rather than from the buffer where all the registers
   were saved when the function called completed.  */
struct value *
pa_value_returned_from_stack (struct type *valtype, CORE_ADDR addr)
{
  struct value *val;

  val = allocate_value (valtype);
  CHECK_TYPEDEF (valtype);
  target_read_memory (addr, VALUE_CONTENTS_RAW (val), TYPE_LENGTH (valtype));

  return val;
}

/* Store the address of the place in which to copy the structure the
   subroutine will return.  This is called from call_function. */

static void
pa_store_struct_return (CORE_ADDR addr, CORE_ADDR sp)
{
  write_register (PA_GR28_REGNUM, addr);
}

/* Write into appropriate registers a function return value
   of type TYPE, given in virtual format.

   For software floating point the return value goes into the integer
   registers.  But we don't have any flag to key this on, so we always
   store the value into the integer registers, and if it's a float value,
   then we put it in the float registers too.  */

static void
pa_store_return_value (struct type *type, char *valbuf)
{
  write_register_bytes (REGISTER_BYTE (PA_GR28_REGNUM),
			valbuf, TYPE_LENGTH (type));
  if (!SOFT_FLOAT)
    write_register_bytes ((TYPE_CODE (type) == TYPE_CODE_FLT
			   ? REGISTER_BYTE (PA_FR4_REGNUM)
			   : REGISTER_BYTE (PA_GR28_REGNUM)),
			  valbuf, TYPE_LENGTH (type));
}

/* Extract from an array REGBUF containing the (raw) register state
   the address in which a function should return its structure value,
   as a CORE_ADDR.  */

static CORE_ADDR
pa_extract_struct_value_address (char *regbuf)
{
  return extract_unsigned_integer (regbuf + REGISTER_BYTE (PA_GR28_REGNUM),
				   REGISTER_RAW_SIZE (PA_GR28_REGNUM));
}

 /* Decide whether the function returning a value of type VALUE_TYPE
    will put it on the stack or in the registers.  */
static int
pa_use_struct_convention (int gcc_p, struct type *value_type)
{
  return TYPE_LENGTH (value_type) > 2 * REGISTER_SIZE;
}

/* Determines the address of all registers in the current stack frame
   storing each in frame->saved_regs.  This includes special registers
   such as pc and fp saved in special ways in the stack frame.  sp is
   even more special: the address we return for it IS the sp for the
   next frame.  */

static void
pa_frame_init_saved_regs (struct frame_info *frame_info)
{
  CORE_ADDR pc;
  struct unwind_table_entry *u;
  unsigned long inst, stack_remaining, save_gr, save_fr, save_rp, save_sp;
  int status, i, reg;
  char buf[4];
  int fp_loc = -1;
  int final_iteration;

  if (DEBUG_PA_FRAME)
    printf ("frame_init_saved_regs (%p), pc = %#lx : ",
	    frame_info, frame_info->pc);

  if (frame_info->saved_regs)
    {
      if (DEBUG_PA_FRAME)
	printf ("already done\n");
      return;
    }

  frame_saved_regs_zalloc (frame_info);

  /* Interrupt handlers are special.  */
  if (PA_IN_INTERRUPT_HANDLER (frame_info->pc))
    {
      if (DEBUG_PA_FRAME)
	printf ("in interrupt\n");
      gdbarch_tdep (current_gdbarch)->frame_find_saved_regs_in_interrupt
	(frame_info, frame_info->saved_regs);
      return;
    }

  /* So are signal handler callers.  */
  if (frame_info->signal_handler_caller
      && gdbarch_tdep (current_gdbarch)->frame_find_saved_regs_in_sigtramp)
    {
      if (DEBUG_PA_FRAME)
	printf ("in sigtramp\n");
      gdbarch_tdep (current_gdbarch)->frame_find_saved_regs_in_sigtramp
	(frame_info, frame_info->saved_regs);
      return;
    }

  /* Call dummy frames always look the same, so there's no need to
     examine the dummy code to determine locations of saved registers;
     instead, let find_dummy_frame_regs fill in the correct offsets
     for the saved registers.  */
  if (frame_info->pc >= frame_info->frame
      && frame_info->pc <= (frame_info->frame
			    + CALL_DUMMY_LENGTH
			    /* Account for register saves.  */
			    + ((32 + 6) * REGISTER_SIZE)
			    + (NUM_REGS - PA_FR0_REGNUM) * 4))
    {
      if (DEBUG_PA_FRAME)
	printf ("in dummy\n");
      find_dummy_frame_regs (frame_info, frame_info->saved_regs);
      return;
    }

  if (DEBUG_PA_FRAME)
    printf ("grok prologue\n");

  /* Get the starting address of the function referred to by the PC
     saved in frame.  */
  pc = get_pc_function_start (frame_info->pc);

  /* Now rummage through the function machine instructions to find out
     where registers are saved.  */
  pa_grok_prologue (pc, frame_info);
}

static void
pa_print_fp_reg (int i)
{
  char buf[8];

  /* Get 32bits of data.  */
  read_relative_register_raw_bytes (i, buf);

  printf_filtered ("%-8s(single precision)     ", REGISTER_NAME (i));
  val_print (REGISTER_VIRTUAL_TYPE (i), buf, 0, 0, gdb_stdout, 0,
	     1, 0, Val_pretty_default);
  printf_filtered ("\n");

  /* If "i" is even, then this register can also be a double-precision
     FP register.  Dump it out as such.  */
  if ((i % 2) == 0)
    {
      /* Get the data in raw format for the 2nd half.  */
      read_relative_register_raw_bytes (i + 1, buf + 4);

      /* Dump it as a double.  */
      printf_filtered ("%-8s(double precision)     ", REGISTER_NAME (i));
      val_print (builtin_type_double, buf, 0, 0, gdb_stdout, 0,
		 1, 0, Val_pretty_default);
      printf_filtered ("\n");
    }
}

/* "Info all-reg" command */

static void
pa_print_registers (int fpregs)
{
  int i, j;
  int columns = 2;
  int rows = PA_FR4_REGNUM / columns;

  for (i = 0; i < rows; i++)
    {
      for (j = 0; j < columns; j++)
	{
	  /* We display registers in column-major order.  */
	  int regnum = i + j * rows;
	  char buf[8];
	  ULONGEST reg_val;

	  read_relative_register_raw_bytes (regnum, buf);
	  reg_val = extract_unsigned_integer (buf,
					      REGISTER_RAW_SIZE (regnum));

	  /* Even fancier % formats to prevent leading zeros
	     and still maintain the output in columns. */

	  printf_unfiltered ("%10s: %-19s", REGISTER_NAME (regnum),
			     phex_nz (reg_val, sizeof (ULONGEST)));
	}
      printf_unfiltered ("\n");
    }

  if (fpregs)
    for (i = PA_FR4_REGNUM; i < NUM_REGS; i++)
      pa_print_fp_reg (i);
}

/* Print the register regnum, or all registers if regnum is -1 */

static void
pa_do_registers_info (int regnum, int fpregs)
{
  if (regnum == -1)
    pa_print_registers (fpregs);
  else if (regnum < PA_FR4_REGNUM)
    {
      char buf[sizeof (ULONGEST)];
      ULONGEST reg_val;

      read_relative_register_raw_bytes (regnum, buf);
      reg_val = extract_unsigned_integer (buf,
					  REGISTER_RAW_SIZE (regnum));

      printf_unfiltered ("%s %s\n", REGISTER_NAME (regnum),
			 phex_nz (reg_val, sizeof (ULONGEST)));
    }
  else
    /* Note that real floating point values only start at
       PA_FR4_REGNUM.  FP0 and up are just status and error
       registers, which have integral (bit) values. */
    pa_print_fp_reg (regnum);
}

/* Finds the addresses of the dynamic call routines for this object
   file.  If PC isn't in any of the object files, return NULL.  */

static obj_private_data_t *
pa_find_dyn (CORE_ADDR pc)
{
  struct obj_section *pc_sec;
  struct objfile *pc_obj;
  obj_private_data_t *obj_private;
  CORE_ADDR *addr;
  static const char *syms[] = {
    "_sr4export", "$$dyncall", "$$dyncall_external"
  };
  const char **sym;

  pc_sec = find_pc_section (pc);
  if (!pc_sec)
    return NULL;

  pc_obj = pc_sec->objfile;
  obj_private = (obj_private_data_t *) pc_obj->obj_private;
  if (obj_private == NULL)
    obj_private = pa_obj_private_alloc (pc_obj);

  for (addr = obj_private->dyn, sym = syms;
       sym < syms + sizeof (syms) / sizeof (syms[0]);
       addr++, sym++)
    {
      if (*addr == 0)
	{
	  struct minimal_symbol *minsym;
	  CORE_ADDR val;

	  /* First look in this object file, but if we didn't find it,
	     look globally.  */
	  minsym = lookup_minimal_symbol (*sym, NULL, pc_obj);
	  if (!minsym)
	    minsym = lookup_minimal_symbol (*sym, NULL, NULL);
	  if (minsym)
	    val = SYMBOL_VALUE_ADDRESS (minsym);
	  else
	    val = -1;
	  *addr = val;
	}
    }
  return obj_private;
}

/* Return one if PC is in the call path of a trampoline, else return zero.

   Note we return one for *any* call trampoline (long-call, arg-reloc), not
   just shared library trampolines (import, export).  */

int
pa_in_solib_call_trampoline (CORE_ADDR pc, char *name)
{
  obj_private_data_t *obj_private;
  struct minimal_symbol *minsym;
  struct unwind_table_entry *u;

  if (DEBUG_PA_GROK)
    printf ("in_solib_call_tramp (%#lx, %s)\n", pc, name);

  if (gdbarch_tdep (current_gdbarch)->is_elf)
    {
      /* PA64 has a completely different stub/trampoline scheme.  Is
	 it better?  Maybe.  It's certainly harder to determine with
	 any certainty that we are in a stub because we can not refer
	 to the unwinders to help. 

	 The heuristic is simple.  Try to lookup the current PC value
	 in the minimal symbol table.  If that fails, then assume we
	 are not in a stub and return.

	 Then see if the PC value falls within the section bounds for
	 the section containing the minimal symbol we found in the
	 first step.  If it does, then assume we are not in a stub and
	 return.

	 Finally peek at the instructions to see if they look like a
	 stub.  */
#if 0
      /* I think this is bogus.  Stub sections get merged into the
	 .text section, so the sec->vma test will not differentiate
	 between stub/non-stub code.  ADM  */
      asection *sec;

      minsym = lookup_minimal_symbol_by_pc (pc);
      if (! minsym)
	return 0;

      sec = SYMBOL_BFD_SECTION (minsym);

      if (sec->vma <= pc
	  && sec->vma + sec->_cooked_size < pc)
	return 0;
#endif

      /* Are we in the plt stub that supports lazy dynamic linking?  */
      if (in_plt_section (pc, name))
	{
	  if (DEBUG_PA_GROK)
	    printf ("  was in plt\n");
	  return 1;
	}

      /* We might be in a stub.  */
      if (pc_in_linker_stub (pc, NULL) != pa_stub_none)
	{
	  if (DEBUG_PA_GROK)
	    printf ("  was in stub\n");
	  return 1;
	}
    }

  obj_private = pa_find_dyn (pc);

  if (obj_private != NULL
      && (pc == obj_private->dyn[dyncall]
	  || pc == obj_private->dyn[sr4export]))
    {
      if (DEBUG_PA_GROK)
	printf ("  was dyn_call\n");
      return 1;
    }

  minsym = lookup_minimal_symbol_by_pc (pc);
  if (minsym)
    {
      CORE_ADDR addr = SYMBOL_VALUE_ADDRESS (minsym);
      if (obj_private != NULL
	  && (addr == obj_private->dyn[dyncall]
	      || addr == obj_private->dyn[sr4export]))
	{
	  if (DEBUG_PA_GROK)
	    printf ("  was in dyn_call\n");
	  return 1;
	}

      if (strcmp (SYMBOL_NAME (minsym), ".stub") == 0)
	{
	  if (DEBUG_PA_GROK)
	    printf ("  was stub sym\n");
	  return 1;
	}
    }

  if (gdbarch_tdep (current_gdbarch)->is_elf)
    {
      if (DEBUG_PA_GROK)
	printf ("  nup\n");
      return 0;
    }

  /* Get the unwind descriptor corresponding to PC, return zero
     if no unwind was found.  */
  u = find_unwind_entry (pc);
  if (!u)
    return 0;

  /* If this isn't a linker stub, then return now.  */
  if (u->stub_unwind.stub_type == 0)
    return 0;

  /* By definition a long-branch stub is a call stub.  */
  if (u->stub_unwind.stub_type == LONG_BRANCH)
    return 1;

  /* The call and return path execute the same instructions within
     an IMPORT stub!  So an IMPORT stub is both a call and return
     trampoline.  */
  if (u->stub_unwind.stub_type == IMPORT)
    return 1;

  /* Parameter relocation stubs always have a call path and may have a
     return path.  */
  if (u->stub_unwind.stub_type == PARAMETER_RELOCATION
      || u->stub_unwind.stub_type == EXPORT)
    {
      CORE_ADDR addr;

      /* Search forward from the current PC until we hit a branch
         or the end of the stub.  */
      for (addr = pc; addr <= u->region_end; addr += 4)
	{
	  unsigned long insn;

	  insn = read_memory_integer (addr, 4);

	  /* Does it look like a bl?  If so then it's the call path, if
	     we find a bv or be first, then we're on the return path.  */
	  if ((insn & 0xfc00e000) == 0xe8000000)
	    return 1;
	  else if ((insn & 0xfc00e001) == 0xe800c000
		   || (insn & 0xfc000000) == 0xe0000000)
	    return 0;
	}

      /* Should never happen.  */
      warning ("Unable to find branch in parameter relocation stub.\n");
      return 0;
    }

  /* Unknown stub type.  For now, just return zero.  */
  return 0;
}

/* Return one if PC is in the return path of a trampoline, else return zero.

   Note we return one for *any* call trampoline (long-call, arg-reloc), not
   just shared library trampolines (import, export).  */

int
pa_in_solib_return_trampoline (CORE_ADDR pc, char *name)
{
  struct unwind_table_entry *u;

  /* Get the unwind descriptor corresponding to PC, return zero
     if no unwind was found.  */
  u = find_unwind_entry (pc);
  if (!u)
    {
      return 0;
    }

  /* If this isn't a linker stub or it's just a long branch stub, then
     return zero.  */
  if (u->stub_unwind.stub_type == 0 || u->stub_unwind.stub_type == LONG_BRANCH)
    return 0;

  /* The call and return path execute the same instructions within
     an IMPORT stub!  So an IMPORT stub is both a call and return
     trampoline.  */
  if (u->stub_unwind.stub_type == IMPORT)
    return 1;

  /* Parameter relocation stubs always have a call path and may have a
     return path.  */
  if (u->stub_unwind.stub_type == PARAMETER_RELOCATION
      || u->stub_unwind.stub_type == EXPORT)
    {
      CORE_ADDR addr;

      /* Search forward from the current PC until we hit a branch
         or the end of the stub.  */
      for (addr = pc; addr <= u->region_end; addr += 4)
	{
	  unsigned long insn;

	  insn = read_memory_integer (addr, 4);

	  /* Does it look like a bl?  If so then it's the call path, if
	     we find a bv or be first, then we're on the return path.  */
	  if ((insn & 0xfc00e000) == 0xe8000000)
	    return 0;
	  else if ((insn & 0xfc00e001) == 0xe800c000
		   || (insn & 0xfc000000) == 0xe0000000)
	    return 1;
	}

      /* Should never happen.  */
      warning ("Unable to find branch in parameter relocation stub.\n");
      return 0;
    }

  /* Unknown stub type.  For now, just return zero.  */
  return 0;
}

/* Figure out if PC is in a trampoline, and if so find out where
   the trampoline will jump to.  If not in a trampoline, return zero.

   Simple code examination probably is not a good idea since the code
   sequences in trampolines can also appear in user code.

   We use unwinds and information from the minimal symbol table to
   determine when we're in a trampoline.  This won't work for ELF
   (yet) since it doesn't create stub unwind entries.  Whether or
   not ELF will create stub unwinds or normal unwinds for linker
   stubs is still being debated.

   This should handle simple calls through dyncall or sr4export,
   long calls, argument relocation stubs, and dyncall/sr4export
   calling an argument relocation stub.  It even handles some stubs
   used in dynamic executables.  */

CORE_ADDR
pa_skip_trampoline_code (CORE_ADDR pc, char *name)
{
  obj_private_data_t *obj_private;
  CORE_ADDR orig_pc = pc;
  int prev_inst, curr_inst;
  CORE_ADDR loc;
  struct minimal_symbol *msym;
  struct unwind_table_entry *u;

  if (DEBUG_PA_GROK)
    printf ("skip_trampoline (%#lx, %s)\n", pc, name);

  obj_private = pa_find_dyn (pc);

  /* Addresses passed to dyncall may *NOT* be the actual address
     of the function.  So we may have to do something special.  */
  if (obj_private != NULL)
    {
      if (pc == obj_private->dyn[dyncall])
	{
	  pc = (CORE_ADDR) read_register (PA_GR22_REGNUM);

	  /* If bit 30 (counting from the left) is on, then pc is the
	     address of the PLT entry for this function, not the
	     address of the function itself.  Bit 31 has meaning too,
	     but only for MPE.  */
	  if (pc & 0x2)
	    pc = (CORE_ADDR) read_memory_integer (pc & ~3, REGISTER_SIZE);
	}
      else if (pc == obj_private->dyn[dyncall_external])
	{
	  pc = (CORE_ADDR) read_register (PA_GR22_REGNUM);
	  pc = (CORE_ADDR) read_memory_integer (pc & ~3, REGISTER_SIZE);
	}
      else if (pc == obj_private->dyn[sr4export])
	{
	  pc = (CORE_ADDR) (read_register (PA_GR22_REGNUM));
	}
    }

  /* Get the unwind descriptor corresponding to PC, return zero
     if no unwind was found.  */
  u = find_unwind_entry (pc);
  if (!u)
    {
      CORE_ADDR stub_start, addr = 0;
      enum stub_type stubt = pc_in_linker_stub (pc, &stub_start);

      switch (stubt)
	{
	default:
	  break;

	case pa_stub_long_branch_shared:
	  addr = stub_start + 8;
	  stub_start += 4;
	  /* Fall thru */

	case pa_stub_long_branch:
	  addr += extract_21 (read_memory_integer (stub_start, 4));
	  addr += extract_17 (read_memory_integer (stub_start + 4, 4));
	  break;

	case pa_stub_import:
	case pa_stub_import_multi:
	  addr = read_register (PA_GR27_REGNUM);
	  goto handle_stub_import;

	case pa_stub_import_shared:
	case pa_stub_import_multi_shared:
	  addr = read_register (PA_GR19_REGNUM);
	handle_stub_import:
	  addr += extract_21 (read_memory_integer (stub_start, 4));
	  addr += extract_14 (read_memory_integer (stub_start + 4, 4));
	  addr = read_memory_integer (addr, 4);
	  if (pc_in_linker_stub (addr, &stub_start) != pa_stub_lazy_link)
	    break;
	  /* Fall thru */

	case pa_stub_lazy_link:
	  addr = read_memory_integer (stub_start + 20, 4);
	  break;

	case pa_stub_export:
	  addr = stub_start + 8;
	  if (pc < addr)
	    addr += extract_17 (read_memory_integer (stub_start, 4));
	  else
	    addr = read_memory_integer (read_register (PA_GR30_REGNUM) - 24, 4);
	  break;

	case pa64_stub_import:
	  addr = read_register (PA_GR27_REGNUM);
	  addr += extract_16a (read_memory_integer (stub_start, 4));
	  addr = read_memory_integer (addr, 8);
	  break;
	}

      if (DEBUG_PA_GROK)
	printf ("  type = %d, returning %#lx\n", stubt, addr & ~3);

      return addr & ~3;
    }

  /* If this isn't a linker stub, then return now.  */
  /* elz: attention here! (FIXME) because of a compiler/linker 
     error, some stubs which should have a non zero stub_unwind.stub_type 
     have unfortunately a value of zero. So this function would return here
     as if we were not in a trampoline. To fix this, we go look at the partial
     symbol information, which reports this guy as a stub.
     (FIXME): Unfortunately, we are not that lucky: it turns out that the 
     partial symbol information is also wrong sometimes. This is because 
     when it is entered (somread.c::som_symtab_read()) it can happen that
     if the type of the symbol (from the som) is Entry, and the symbol is
     in a shared library, then it can also be a trampoline.  This would
     be OK, except that I believe the way they decide if we are ina shared library
     does not work. SOOOO..., even if we have a regular function w/o trampolines
     its minimal symbol can be assigned type mst_solib_trampoline.
     Also, if we find that the symbol is a real stub, then we fix the unwind
     descriptor, and define the stub type to be EXPORT.
     Hopefully this is correct most of the times. */
  if (u->stub_unwind.stub_type == 0)
    {

/* elz: NOTE (FIXME!) once the problem with the unwind information is fixed
   we can delete all the code which appears between the lines */
/*--------------------------------------------------------------------------*/
      msym = lookup_minimal_symbol_by_pc (pc);

      if (msym == NULL || MSYMBOL_TYPE (msym) != mst_solib_trampoline)
	return orig_pc == pc ? 0 : pc & ~3;

      else if (msym != NULL && MSYMBOL_TYPE (msym) == mst_solib_trampoline)
	{
	  struct objfile *objfile;
	  struct minimal_symbol *msymbol;
	  int function_found = 0;

	  /* go look if there is another minimal symbol with the same name as 
	     this one, but with type mst_text. This would happen if the msym
	     is an actual trampoline, in which case there would be another
	     symbol with the same name corresponding to the real function */

	  ALL_MSYMBOLS (objfile, msymbol)
	  {
	    if (MSYMBOL_TYPE (msymbol) == mst_text
		&& STREQ (SYMBOL_NAME (msymbol), SYMBOL_NAME (msym)))
	      {
		function_found = 1;
		break;
	      }
	  }

	  if (function_found)
	    /* the type of msym is correct (mst_solib_trampoline), but
	       the unwind info is wrong, so set it to the correct value */
	    u->stub_unwind.stub_type = EXPORT;
	  else
	    /* the stub type info in the unwind is correct (this is not a
	       trampoline), but the msym type information is wrong, it
	       should be mst_text. So we need to fix the msym, and also
	       get out of this function */
	    {
	      MSYMBOL_TYPE (msym) = mst_text;
	      return orig_pc == pc ? 0 : pc & ~3;
	    }
	}

/*--------------------------------------------------------------------------*/
    }

  /* It's a stub.  Search for a branch and figure out where it goes.
     Note we have to handle multi insn branch sequences like ldil;ble.
     Most (all?) other branches can be determined by examining the contents
     of certain registers and the stack.  */

  loc = pc;
  curr_inst = 0;
  prev_inst = 0;
  while (1)
    {
      /* Make sure we haven't walked outside the range of this stub.  */
      if (u != find_unwind_entry (loc))
	{
	  warning ("Unable to find branch in linker stub");
	  return orig_pc == pc ? 0 : pc & ~3;
	}

      prev_inst = curr_inst;
      curr_inst = read_memory_integer (loc, 4);

      /* Does it look like a branch external using %r1?  Then it's the
         branch from the stub to the actual function.  */
      if ((curr_inst & 0xffe0e000) == 0xe0202000)
	{
	  /* Yup.  See if the previous instruction loaded
	     a value into %r1.  If so compute and return the jump address.  */
	  if ((prev_inst & 0xffe00000) == 0x20200000)
	    return (extract_21 (prev_inst) + extract_17 (curr_inst)) & ~3;
	  else
	    {
	      warning ("Unable to find ldil X,%%r1 before ble Y(%%sr4,%%r1).");
	      return orig_pc == pc ? 0 : pc & ~3;
	    }
	}

      /* Does it look like a be 0(sr0,%r21)? OR 
         Does it look like a be, n 0(sr0,%r21)? OR 
         Does it look like a bve (r21)? (this is on PA2.0)
         Does it look like a bve, n(r21)? (this is also on PA2.0)
         That's the branch from an
         import stub to an export stub.

         It is impossible to determine the target of the branch via
         simple examination of instructions and/or data (consider
         that the address in the plabel may be the address of the
         bind-on-reference routine in the dynamic loader).

         So we have try an alternative approach.

         Get the name of the symbol at our current location; it should
         be a stub symbol with the same name as the symbol in the
         shared library.

         Then lookup a minimal symbol with the same name; we should
         get the minimal symbol for the target routine in the shared
         library as those take precedence of import/export stubs.  */
      if ((curr_inst == 0xe2a00000) ||
	  (curr_inst == 0xe2a00002) ||
	  (curr_inst == 0xeaa0d000) ||
	  (curr_inst == 0xeaa0d002))
	{
	  struct minimal_symbol *stubsym, *libsym;

	  stubsym = lookup_minimal_symbol_by_pc (loc);
	  if (stubsym == NULL)
	    {
	      warning ("Unable to find symbol for 0x%lx", (long) loc);
	      return orig_pc == pc ? 0 : pc & ~3;
	    }

	  libsym = lookup_minimal_symbol (SYMBOL_NAME (stubsym), NULL, NULL);
	  if (libsym == NULL)
	    {
	      warning ("Unable to find library symbol for %s\n",
		       SYMBOL_NAME (stubsym));
	      return orig_pc == pc ? 0 : pc & ~3;
	    }

	  return SYMBOL_VALUE (libsym);
	}

      /* Does it look like bl X,%rp or bl X,%r0?  Another way to do a
         branch from the stub to the actual function.  */
      /*elz */
      else if ((curr_inst & 0xffe0e000) == 0xe8400000
	       || (curr_inst & 0xffe0e000) == 0xe8000000
	       || (curr_inst & 0xffe0e000) == 0xe800A000)
	return (loc + extract_17 (curr_inst) + 8) & ~3;

      /* Does it look like bv (rp)?   Note this depends on the
         current stack pointer being the same as the stack
         pointer in the stub itself!  This is a branch on from the
         stub back to the original caller.  */
      /*else if ((curr_inst & 0xffe0e000) == 0xe840c000) */
      else if ((curr_inst & 0xffe0f000) == 0xe840c000)
	{
	  /* Yup.  See if the previous instruction loaded
	     rp from sp - 8.  */
	  if (prev_inst == 0x4bc23ff1)
	    return (read_memory_integer
		    (read_register (PA_GR30_REGNUM) - 8, 4)) & ~3;
	  else
	    {
	      warning ("Unable to find restore of %%rp before bv (%%rp).");
	      return orig_pc == pc ? 0 : pc & ~3;
	    }
	}

      /* elz: added this case to capture the new instruction
         at the end of the return part of an export stub used by
         the PA2.0: BVE, n (rp) */
      else if ((curr_inst & 0xffe0f000) == 0xe840d000)
	{
	  return (read_memory_integer
		  (read_register (PA_GR30_REGNUM) - 24, REGISTER_SIZE)) & ~3;
	}

      /* What about be,n 0(sr0,%rp)?  It's just another way we return to
         the original caller from the stub.  Used in dynamic executables.  */
      else if (curr_inst == 0xe0400002)
	{
	  /* The value we jump to is sitting in sp - 24.  But that's
	     loaded several instructions before the be instruction.
	     I guess we could check for the previous instruction being
	     mtsp %r1,%sr0 if we want to do sanity checking.  */
	  return (read_memory_integer
		  (read_register (PA_GR30_REGNUM) - 24, REGISTER_SIZE)) & ~3;
	}

      /* Haven't found the branch yet, but we're still in the stub.
         Keep looking.  */
      loc += 4;
    }
}

/* Return the address of the PC after the last prologue instruction if
   we can determine it from the debug symbols.  Else return zero.  */

static CORE_ADDR
after_prologue (CORE_ADDR pc)
{
  struct symtab_and_line sal;
  CORE_ADDR func_addr, func_end;
  struct symbol *f;
  asection *sect;

  if (DEBUG_PA_GROK)
    printf ("after_prologue (%#lx)\n", pc);

  sect = find_pc_overlay (pc);

  /* If we can not find the symbol in the partial symbol table, then
     there is no hope we can determine the function's start address
     with this code.  */
  if (!find_pc_sect_partial_function (pc, sect, NULL, &func_addr, &func_end))
    return 0;

  if (DEBUG_PA_GROK)
    printf ("  func_addr = %#lx, func_end = %#lx\n", func_addr, func_end);

  /* Get the line associated with FUNC_ADDR.  */
  sal = find_pc_sect_line (func_addr, sect, 0);

  /* There are only two cases to consider.  First, the end of the source line
     is within the function bounds.  In that case we return the end of the
     source line.  Second is the end of the source line extends beyond the
     bounds of the current function.  We need to use the slow code to
     examine instructions in that case. 

     Anything else is simply a bug elsewhere.  Fixing it here is absolutely
     the wrong thing to do.  In fact, it should be entirely possible for this
     function to always return zero since the slow instruction scanning code
     is supposed to *always* work.  If it does not, then it is a bug.  */
  if (sal.end > func_addr && sal.end < func_end)
    return sal.end;
  return 0;
}

/* Advance PC across any function entry prologue instructions
   to reach some "real" code.
   Returns either PC itself if the code at PC does not look like a
   function prologue; otherwise returns an address that (if we're
   lucky) follows the prologue.  */

static CORE_ADDR
pa_skip_prologue (CORE_ADDR pc)
{
  CORE_ADDR post_prologue_pc;

  if (DEBUG_PA_GROK)
    printf ("skip_prologue (%#lx)\n", pc);

  /* See if we can determine the end of the prologue via the symbol table.
     If so, then return either PC, or the PC after the prologue, whichever
     is greater.  */

  post_prologue_pc = after_prologue (pc);

  if (DEBUG_PA_GROK > 1)
    printf ("  post_prologue_pc = %#lx\n", post_prologue_pc);

  /* If after_prologue returned a useful address, then use it.  Else
     fall back on the instruction skipping code.

     Some folks have claimed this causes problems because the breakpoint
     may be the first instruction of the prologue.  If that happens, then
     the instruction skipping code has a bug that needs to be fixed.  */
  if (post_prologue_pc != 0)
    return pc >= post_prologue_pc ? pc : post_prologue_pc;

  return pa_grok_prologue (pc, NULL);
}

/* Sequence of bytes for breakpoint instruction.  */
static unsigned char *
pa_breakpoint_from_pc (CORE_ADDR *pcptr, int *lenptr)
{
  static unsigned char breakpoint[4] = {0x00, 0x01, 0x00, 0x04};
  *lenptr = sizeof (breakpoint);
  return breakpoint;
}

static void
pa_remote_translate_xfer_address (CORE_ADDR memaddr, int nr_bytes,
				  CORE_ADDR *targ_addr, int *targ_len)
{
  *targ_addr = memaddr;
  *targ_len  = nr_bytes;
}

void
pa_skip_permanent_breakpoint (void)
{
  /* To step over a breakpoint instruction on the PA takes some
     fiddling with the instruction address queue.

     When we stop at a breakpoint, the IA queue front (the instruction
     we're executing now) points at the breakpoint instruction, and
     the IA queue back (the next instruction to execute) points to
     whatever instruction we would execute after the breakpoint, if it
     were an ordinary instruction.  This is the case even if the
     breakpoint is in the delay slot of a branch instruction.

     Clearly, to step past the breakpoint, we need to set the queue
     front to the back.  But what do we put in the back?  What
     instruction comes after that one?  Because of the branch delay
     slot, the next insn is always at the back + 4.  */
  int tail;

  tail = read_register (PA_PCOQ_TAIL_REGNUM);
  write_register (PA_PCOQ_HEAD_REGNUM, tail);
  write_register (PA_PCSQ_HEAD_REGNUM, read_register (PA_PCSQ_TAIL_REGNUM));

  write_register (PA_PCOQ_TAIL_REGNUM, tail + 4);
  /* We can leave the tail's space the same, since there's no jump.  */
}

static void
unwind_command (char *exp, int from_tty)
{
  CORE_ADDR address;
  struct unwind_table_entry *u;

  /* If we have an expression, evaluate it and use it as the address.  */

  if (exp != 0 && *exp != 0)
    address = parse_and_eval_address (exp);
  else
    return;

  u = find_unwind_entry (address);

  if (!u)
    {
      printf_unfiltered ("Can't find unwind table entry for %s\n", exp);
      return;
    }

  printf_unfiltered ("unwind_table_entry (%p):\n", u);

  printf_unfiltered ("\tregion_start = ");
  print_address (u->region_start, gdb_stdout);

  printf_unfiltered ("\n\tregion_end = ");
  print_address (u->region_end, gdb_stdout);

#ifdef __STDC__
#define pif(FLD) if (u->FLD) printf_unfiltered (" "#FLD);
#else
#define pif(FLD) if (u->FLD) printf_unfiltered (" FLD");
#endif

  printf_unfiltered ("\n\tflags =");
  pif (Cannot_unwind);
  pif (Millicode);
  pif (Millicode_save_sr0);
  pif (Entry_SR);
  pif (Args_stored);
  pif (Variable_Frame);
  pif (Separate_Package_Body);
  pif (Frame_Extension_Millicode);
  pif (Stack_Overflow_Check);
  pif (Two_Instruction_SP_Increment);
  pif (Ada_Region);
  pif (Save_SP);
  pif (Save_RP);
  pif (Save_MRP_in_frame);
  pif (extn_ptr_defined);
  pif (Cleanup_defined);
  pif (MPE_XL_interrupt_marker);
  pif (HP_UX_interrupt_marker);
  pif (Large_frame);

  putchar_unfiltered ('\n');

#ifdef __STDC__
#define pin(FLD) printf_unfiltered ("\t"#FLD" = 0x%x\n", u->FLD);
#else
#define pin(FLD) printf_unfiltered ("\tFLD = 0x%x\n", u->FLD);
#endif

  pin (Region_description);
  pin (Entry_FR);
  pin (Entry_GR);
  pin (Total_frame_size);
}

void pa_dump_tdep (struct gdbarch *gdbarch, struct ui_file *file)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: OS_IDENT = %#x\n",
		      tdep->os_ident);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: IS_ELF = %d\n",
		      tdep->is_elf);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: IS_ELF64 = %d\n",
		      tdep->is_elf64);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: IN_SYSCALL = %p\n",
		      tdep->in_syscall);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: IN_INTERRUPT_HANDLER = %p\n",
		      tdep->in_interrupt_handler);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: IN_SIGTRAMP = %p\n",
		      tdep->in_sigtramp);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: FRAME_SAVED_PC_IN_INTERRUPT = %p\n",
		      tdep->frame_saved_pc_in_interrupt);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: FRAME_BASE_BEFORE_INTERRUPT = %p\n",
		      tdep->frame_base_before_interrupt);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: FRAME_FIND_SAVED_REGS_IN_INTERRUPT = %p\n",
		      tdep->frame_find_saved_regs_in_interrupt);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: FRAME_SAVED_PC_IN_SIGTRAMP = %p\n",
		      tdep->frame_saved_pc_in_sigtramp);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: FRAME_BASE_BEFORE_SIGTRAMP = %p\n",
		      tdep->frame_base_before_sigtramp);
  fprintf_unfiltered (file,
                      "gdbarch_dump_tdep: FRAME_FIND_SAVED_REGS_IN_SIGTRAMP = %p\n",
		      tdep->frame_find_saved_regs_in_sigtramp);
}

static struct gdbarch *
pa_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *tdep;
  int os_ident = -1;
  unsigned int is_elf = 0;
  unsigned int is_elf64 = 0;

  if (info.abfd != NULL
      && bfd_get_flavour (info.abfd) == bfd_target_elf_flavour)
    {
      is_elf = 1;
      is_elf64 = elf_elfheader (info.abfd)->e_ident[EI_CLASS] == ELFCLASS64;
      os_ident = elf_elfheader (info.abfd)->e_ident[EI_OSABI];
    }

  for (arches = gdbarch_list_lookup_by_info (arches, &info);
       arches != NULL;
       arches = gdbarch_list_lookup_by_info (arches->next, &info))
    {
      if (gdbarch_tdep (current_gdbarch)->os_ident == os_ident)
	return arches->gdbarch;
    }

  tdep = xmalloc (sizeof (struct gdbarch_tdep));
  gdbarch = gdbarch_alloc (&info, tdep);
  tdep->os_ident = os_ident;
  tdep->is_elf = is_elf;
  tdep->is_elf64 = is_elf64;

  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 32);
  set_gdbarch_long_bit (gdbarch, is_elf64 ? 64: 32);
  set_gdbarch_long_long_bit (gdbarch, 64);
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 64);
  set_gdbarch_long_double_bit (gdbarch, 64);
  set_gdbarch_ptr_bit (gdbarch, is_elf64 ? 64 : 32);
  /* default addr_bit, bfd_vma_bit.  */
  /* set_gdbarch_ieee_float (gdbarch, 1); */

  set_gdbarch_num_regs (gdbarch, NUM_REGS);
  /* default num_pseudo_regs.  */
  set_gdbarch_sp_regnum (gdbarch, PA_GR31_REGNUM);
  set_gdbarch_fp_regnum (gdbarch, PA_GR3_REGNUM);
  set_gdbarch_pc_regnum (gdbarch, PA_PCOQ_HEAD_REGNUM);
  set_gdbarch_npc_regnum (gdbarch, PA_PCOQ_TAIL_REGNUM);
  set_gdbarch_fp0_regnum (gdbarch, PA_FR0_REGNUM);

  set_gdbarch_register_name (gdbarch, pa_register_name);
  set_gdbarch_register_size (gdbarch, is_elf64 ? 8 : 4);
  set_gdbarch_register_bytes (gdbarch, (is_elf64
					? (NUM_REGS * 8
					   - (NUM_REGS - PA_FR0_REGNUM) * 4)
					: NUM_REGS * 4));
  set_gdbarch_register_byte (gdbarch, (is_elf64
				       ? pa64_register_byte
				       : pa_register_byte));
  set_gdbarch_register_raw_size (gdbarch, (is_elf64
					   ? pa64_register_raw_size
					   : pa_register_raw_size));
  set_gdbarch_max_register_raw_size (gdbarch, is_elf64 ? 8 : 4);
  set_gdbarch_register_virtual_size (gdbarch, (is_elf64
					       ? pa64_register_raw_size
					       : pa_register_raw_size));
  set_gdbarch_max_register_virtual_size (gdbarch, is_elf64 ? 8 : 4);
  set_gdbarch_register_virtual_type (gdbarch, (is_elf64
					       ? pa64_register_virtual_type
					       : pa_register_virtual_type));
  set_gdbarch_do_registers_info (gdbarch, pa_do_registers_info);

  /* default register_sim_regno.  */
  set_gdbarch_use_generic_dummy_frames (gdbarch, 0);

  set_gdbarch_believe_pcc_promotion (gdbarch, 1);
  /* default believe_pcc_promotion_type.  */

  set_gdbarch_coerce_float_to_double (gdbarch, standard_coerce_float_to_double);
  set_gdbarch_get_saved_register  (gdbarch, generic_get_saved_register);
  /* default register_convertible.  */
  /* default register_convert_to_virtual.  */
  /* default register_convert_to_raw.  */
  /* default fetch_pseudo_register.  */
  /* default store_pseudo_register.  */
  /* default pointer_to_address.  */
  /* default address_to_pointer.  */
  /* default return_value_on_stack.  */
  set_gdbarch_extract_return_value (gdbarch, (is_elf64
					      ? pa64_extract_return_value
					      : pa_extract_return_value));
  set_gdbarch_push_arguments (gdbarch, (is_elf64
					? pa64_push_arguments
					: pa_push_arguments));
  set_gdbarch_push_return_address (gdbarch, pa_push_return_address);
  set_gdbarch_pop_frame (gdbarch, pa_pop_frame);
  set_gdbarch_store_struct_return (gdbarch, pa_store_struct_return);
  set_gdbarch_store_return_value (gdbarch, pa_store_return_value);
  set_gdbarch_extract_struct_value_address (gdbarch,
					    pa_extract_struct_value_address);
  set_gdbarch_use_struct_convention (gdbarch, pa_use_struct_convention);
  set_gdbarch_frame_init_saved_regs (gdbarch, pa_frame_init_saved_regs);
  set_gdbarch_init_extra_frame_info (gdbarch, pa_init_extra_frame_info);
  set_gdbarch_skip_prologue (gdbarch, pa_skip_prologue);
  set_gdbarch_prologue_frameless_p (gdbarch, pa_prologue_frameless_p);
  set_gdbarch_inner_than (gdbarch, pa_inner_than);
  set_gdbarch_breakpoint_from_pc (gdbarch, pa_breakpoint_from_pc);
  /* default memory_insert_breakpoint.  */
  /* default memory_remove_breakpoint.  */
  set_gdbarch_decr_pc_after_break (gdbarch, 0);
  set_gdbarch_function_start_offset (gdbarch, 0);
  set_gdbarch_remote_translate_xfer_address (gdbarch,
					     pa_remote_translate_xfer_address);
  set_gdbarch_frame_args_skip (gdbarch, 0);
  set_gdbarch_frameless_function_invocation (gdbarch,
					     pa_frameless_function_invocation);
  set_gdbarch_frame_chain (gdbarch, pa_frame_chain);
  set_gdbarch_frame_chain_valid (gdbarch, generic_func_frame_chain_valid);
  set_gdbarch_frame_saved_pc (gdbarch, pa_frame_saved_pc);
  set_gdbarch_frame_args_address (gdbarch, pa_frame_args_address);
  set_gdbarch_frame_locals_address (gdbarch, pa_frame_locals_address);
  set_gdbarch_saved_pc_after_call (gdbarch, pa_saved_pc_after_call);
  set_gdbarch_frame_num_args (gdbarch, frame_num_args_unknown);
  set_gdbarch_stack_align (gdbarch, (is_elf64
				     ? pa64_stack_align
				     : pa_stack_align));
  /* Default extra_stack_alignment_needed.  */
  set_gdbarch_reg_struct_has_addr (gdbarch, pa_reg_struct_has_addr);
  /* Default save_dummy_frame_tos */
  set_gdbarch_float_format (gdbarch, &floatformat_ieee_single_big);
  set_gdbarch_double_format (gdbarch, &floatformat_ieee_double_big);
  set_gdbarch_long_double_format (gdbarch, &floatformat_ieee_double_big);
  /* Default convert_from_func_ptr_addr.  */

#if 0
  if (os_ident == ELFOSABI_LINUX)
#endif
    pa_linux_initialize_tdep (gdbarch, is_elf64);
#if 0
  /* Disable hpux support as som stuff only compiles native.  Sheesh.  */
  else
    pa_hpux_initialize_tdep (gdbarch, is_elf64);
#endif

  return gdbarch;
}

void
_initialize_pa_tdep (void)
{
  gdbarch_register (bfd_arch_hppa, pa_gdbarch_init, pa_dump_tdep);

  tm_print_insn = print_insn_hppa;
  tm_print_insn_info.bytes_per_line = 4;

  add_cmd ("unwind", class_maintenance, unwind_command,
	   "Print unwind table entry at given address.",
	   &maintenanceprintlist);
#if DEBUG
  add_show_from_set (add_set_cmd ("frame", class_maintenance, var_zinteger,
				  (char *) &framedebug,
				  "Set frame debugging.\n\
When non-zero, frame debugging is enabled.",
				  &setdebuglist),
		     &showdebuglist);
  add_show_from_set (add_set_cmd ("grok", class_maintenance, var_zinteger,
				  (char *) &grokdebug,
				  "Set prologue debugging.\n\
When non-zero, prologue debugging is enabled.",
				  &setdebuglist),
		     &showdebuglist);
#endif
}

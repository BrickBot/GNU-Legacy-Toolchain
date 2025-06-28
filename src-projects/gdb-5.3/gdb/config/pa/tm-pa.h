/* Definitions to target GDB to any Hewlett-Packard PA-RISC machine.
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

#ifndef PA_TM_PA_H
#define PA_TM_PA_H


#if !defined(GDBSERVER)

#define GDB_MULTI_ARCH 1

#else /* Defines needed for GDBSERVER.  */

/* Target system byte order.  */

#define	TARGET_BYTE_ORDER	BIG_ENDIAN

/* Some pseudo register numbers.  */

#define PC_REGNUM	PA_PCOQ_HEAD_REGNUM
#define NPC_REGNUM	PA_PCOQ_TAIL_REGNUM
#define SP_REGNUM	PA_GR31_REGNUM
#define FP_REGNUM	PA_GR3_REGNUM

#endif

/* Number of machine registers.  Well, sort of.  It really just
   specifies the range of register numbers known to gdb.  */

#define NUM_REGS 128

/* Register numbers of various registers,  */

/* General registers.  */
#define PA_GR0_REGNUM		0
#define PA_GR1_REGNUM		(PA_GR0_REGNUM+1)
#define PA_GR2_REGNUM		(PA_GR0_REGNUM+2)
#define PA_GR3_REGNUM		(PA_GR0_REGNUM+3)
#define PA_GR4_REGNUM		(PA_GR0_REGNUM+4)
#define PA_GR5_REGNUM		(PA_GR0_REGNUM+5)
#define PA_GR6_REGNUM		(PA_GR0_REGNUM+6)
#define PA_GR7_REGNUM		(PA_GR0_REGNUM+7)
#define PA_GR8_REGNUM		(PA_GR0_REGNUM+8)
#define PA_GR9_REGNUM		(PA_GR0_REGNUM+9)
#define PA_GR10_REGNUM		(PA_GR0_REGNUM+10)
#define PA_GR11_REGNUM		(PA_GR0_REGNUM+11)
#define PA_GR12_REGNUM		(PA_GR0_REGNUM+12)
#define PA_GR13_REGNUM		(PA_GR0_REGNUM+13)
#define PA_GR14_REGNUM		(PA_GR0_REGNUM+14)
#define PA_GR15_REGNUM		(PA_GR0_REGNUM+15)
#define PA_GR16_REGNUM		(PA_GR0_REGNUM+16)
#define PA_GR17_REGNUM		(PA_GR0_REGNUM+17)
#define PA_GR18_REGNUM		(PA_GR0_REGNUM+18)
#define PA_GR19_REGNUM		(PA_GR0_REGNUM+19)
#define PA_GR20_REGNUM		(PA_GR0_REGNUM+20)
#define PA_GR21_REGNUM		(PA_GR0_REGNUM+21)
#define PA_GR22_REGNUM		(PA_GR0_REGNUM+22)
#define PA_GR23_REGNUM		(PA_GR0_REGNUM+23)
#define PA_GR24_REGNUM		(PA_GR0_REGNUM+24)
#define PA_GR25_REGNUM		(PA_GR0_REGNUM+25)
#define PA_GR26_REGNUM		(PA_GR0_REGNUM+26)
#define PA_GR27_REGNUM		(PA_GR0_REGNUM+27)
#define PA_GR28_REGNUM		(PA_GR0_REGNUM+28)
#define PA_GR29_REGNUM		(PA_GR0_REGNUM+29)
#define PA_GR30_REGNUM		(PA_GR0_REGNUM+30)
#define PA_GR31_REGNUM		(PA_GR0_REGNUM+31)

/* Control registers.  The peculiar layout is to match HPUX interrupt save
   state.  */
#define PA_CR11_REGNUM		32
#define PA_PCOQ_HEAD_REGNUM	33	/* CR18 */
#define PA_PCSQ_HEAD_REGNUM	34	/* CR17 */
#define PA_PCOQ_TAIL_REGNUM	35	/* CR18 */
#define PA_PCSQ_TAIL_REGNUM	36	/* CR17 */
#define PA_CR15_REGNUM		37
#define PA_CR19_REGNUM		38
#define PA_CR20_REGNUM		39
#define PA_CR21_REGNUM		40
#define PA_CR22_REGNUM		41
#define PA_CR31_REGNUM		42

/* Space registers.  */
#define PA_SR4_REGNUM		43
#define PA_SR0_REGNUM		44
#define PA_SR1_REGNUM		45
#define PA_SR2_REGNUM		46
#define PA_SR3_REGNUM		47
#define PA_SR5_REGNUM		48
#define PA_SR6_REGNUM		49
#define PA_SR7_REGNUM		50

/* More control regs.  */
#define PA_CR0_REGNUM		51
#define PA_CR8_REGNUM		52
#define PA_CR9_REGNUM		53
#define PA_CR10_REGNUM		54
#define PA_CR12_REGNUM		55
#define PA_CR13_REGNUM		56
#define PA_CR24_REGNUM		57
#define PA_CR25_REGNUM		58
#define PA_CR26_REGNUM		59
#define PA_CR27_REGNUM		60
#define PA_CR28_REGNUM		61
#define PA_CR29_REGNUM		62
#define PA_CR30_REGNUM		63

/* Floating point registers.  */
#define PA_FR0_REGNUM		64
#define PA_FR1_REGNUM		(PA_FR0_REGNUM+2)
#define PA_FR2_REGNUM		(PA_FR0_REGNUM+4)
#define PA_FR3_REGNUM		(PA_FR0_REGNUM+6)
#define PA_FR4_REGNUM		(PA_FR0_REGNUM+8)
#define PA_FR5_REGNUM		(PA_FR0_REGNUM+10)
#define PA_FR6_REGNUM		(PA_FR0_REGNUM+12)
#define PA_FR7_REGNUM		(PA_FR0_REGNUM+14)
#define PA_FR31_REGNUM		(PA_FR0_REGNUM+62)

/* Some aliases.  */
#define PA_FLAGS_REGNUM		PA_GR0_REGNUM
#define PA_SAR_REGNUM		PA_CR11_REGNUM
#define PA_IPSW_REGNUM		PA_CR22_REGNUM

/*
 * Processor Status Word Masks
 */

#define PSW_T   0x01000000	/* Taken Branch Trap Enable */
#define PSW_H   0x00800000	/* Higher-Privilege Transfer Trap Enable */
#define PSW_L   0x00400000	/* Lower-Privilege Transfer Trap Enable */
#define PSW_N   0x00200000	/* PC Queue Front Instruction Nullified */
#define PSW_X   0x00100000	/* Data Memory Break Disable */
#define PSW_B   0x00080000	/* Taken Branch in Previous Cycle */
#define PSW_C   0x00040000	/* Code Address Translation Enable */
#define PSW_V   0x00020000	/* Divide Step Correction */
#define PSW_M   0x00010000	/* High-Priority Machine Check Disable */
#define PSW_CB  0x0000ff00	/* Carry/Borrow Bits */
#define PSW_R   0x00000010	/* Recovery Counter Enable */
#define PSW_Q   0x00000008	/* Interruption State Collection Enable */
#define PSW_P   0x00000004	/* Protection ID Validation Enable */
#define PSW_D   0x00000002	/* Data Address Translation Enable */
#define PSW_I   0x00000001	/* External, Power Failure, Low-Priority */
				/* Machine Check Interruption Enable */

/* By default assume we don't have to worry about software floating point.  */
#ifndef SOFT_FLOAT
#define SOFT_FLOAT 0
#endif

struct gdbarch_tdep
  {
    int os_ident;	/* From the ELF header, one of the ELFOSABI_
                           constants: ELFOSABI_LINUX, ELFOSABI_HPUX,
			   etc.  */
    unsigned int is_elf:1;
    unsigned int is_elf64:1;
    int (*in_syscall) (const CORE_ADDR *);
    int (*in_interrupt_handler) (CORE_ADDR);
    int (*in_sigtramp) (CORE_ADDR, const char *);
    CORE_ADDR (*frame_saved_pc_in_interrupt) (const struct frame_info *);
    CORE_ADDR (*frame_base_before_interrupt) (const struct frame_info *);
    void (*frame_find_saved_regs_in_interrupt) (struct frame_info *,
						CORE_ADDR *);
    CORE_ADDR (*frame_saved_pc_in_sigtramp) (const struct frame_info *);
    CORE_ADDR (*frame_base_before_sigtramp) (struct frame_info *);
    void (*frame_find_saved_regs_in_sigtramp) (struct frame_info *,
					       CORE_ADDR *);
  };

#define PA_IN_SYSCALL(flags) \
  (gdbarch_tdep (current_gdbarch)->in_syscall (flags))
#define PA_IN_INTERRUPT_HANDLER(pc) \
  (gdbarch_tdep (current_gdbarch)->in_interrupt_handler (pc))
#define IN_SIGTRAMP(pc, func_name) \
  (gdbarch_tdep (current_gdbarch)->in_sigtramp (pc, func_name))


/*
 * Unwind table and descriptor.
 */

struct unwind_table_entry
  {
    CORE_ADDR region_start;
    CORE_ADDR region_end;

    unsigned int Cannot_unwind:1;	/* 0 */
    unsigned int Millicode:1;	/* 1 */
    unsigned int Millicode_save_sr0:1;	/* 2 */
    unsigned int Region_description:2;	/* 3..4 */
    unsigned int reserved1:1;	/* 5 */
    unsigned int Entry_SR:1;	/* 6 */
    unsigned int Entry_FR:4;	/* number saved *//* 7..10 */
    unsigned int Entry_GR:5;	/* number saved *//* 11..15 */
    unsigned int Args_stored:1;	/* 16 */
    unsigned int Variable_Frame:1;	/* 17 */
    unsigned int Separate_Package_Body:1;	/* 18 */
    unsigned int Frame_Extension_Millicode:1;	/* 19 */
    unsigned int Stack_Overflow_Check:1;	/* 20 */
    unsigned int Two_Instruction_SP_Increment:1;	/* 21 */
    unsigned int Ada_Region:1;	/* 22 */
    unsigned int cxx_info:1;	/* 23 */
    unsigned int cxx_try_catch:1;	/* 24 */
    unsigned int sched_entry_seq:1;	/* 25 */
    unsigned int reserved2:1;	/* 26 */
    unsigned int Save_SP:1;	/* 27 */
    unsigned int Save_RP:1;	/* 28 */
    unsigned int Save_MRP_in_frame:1;	/* 29 */
    unsigned int extn_ptr_defined:1;	/* 30 */
    unsigned int Cleanup_defined:1;	/* 31 */

    unsigned int MPE_XL_interrupt_marker:1;	/* 0 */
    unsigned int HP_UX_interrupt_marker:1;	/* 1 */
    unsigned int Large_frame:1;	/* 2 */
    unsigned int Pseudo_SP_Set:1;	/* 3 */
    unsigned int reserved4:1;	/* 4 */
    unsigned int Total_frame_size:27;	/* 5..31 */

    /* This is *NOT* part of an actual unwind_descriptor in an object
       file.  It is *ONLY* part of the "internalized" descriptors that
       we create from those in a file.
     */
    struct
      {
	unsigned int stub_type:4;	/* 0..3 */
	unsigned int padding:28;	/* 4..31 */
      }
    stub_unwind;
  };

/* HP linkers also generate unwinds for various linker-generated stubs.
   GDB reads in the stubs from the $UNWIND_END$ subspace, then 
   "converts" them into normal unwind entries using some of the reserved
   fields to store the stub type.  */

struct stub_unwind_entry
  {
    /* The offset within the executable for the associated stub.  */
    unsigned stub_offset;

    /* The type of stub this unwind entry describes.  */
    char type;

    /* Unknown.  Not needed by GDB at this time.  */
    char prs_info;

    /* Length (in instructions) of the associated stub.  */
    short stub_length;
  };

/* Sizes (in bytes) of the native unwind entries.  */
#define UNWIND_ENTRY_SIZE 16
#define STUB_UNWIND_ENTRY_SIZE 8

/* The gaps represent linker stubs used in MPE and space for future
   expansion.  */
enum unwind_stub_types
  {
    LONG_BRANCH = 1,
    PARAMETER_RELOCATION = 2,
    EXPORT = 10,
    IMPORT = 11,
    IMPORT_SHLIB = 12,
  };

struct unwind_table_entry *find_unwind_entry (CORE_ADDR);

/* We use the objfile->obj_private pointer for two things:

 * 1.  An unwind table;
 *
 * 2.  A pointer to any associated shared library object.
 *
 * #defines are used to help refer to these objects.
 */

/* Info about the unwind table associated with an object file.

 * This is hung off of the "objfile->obj_private" pointer, and
 * is allocated in the objfile's psymbol obstack.  This allows
 * us to have unique unwind info for each executable and shared
 * library that we are debugging.
 */
struct obj_unwind_info
  {
    struct unwind_table_entry *table;	/* Pointer to unwind info */
    struct unwind_table_entry *cache;	/* Pointer to last entry we found */
    int last;				/* Index of last entry */
  };

enum dyncall_enum
  {
    sr4export = 0, dyncall, dyncall_external, last_dyncall_enum
  };

typedef struct obj_private_struct
  {
    struct obj_unwind_info *unwind_info;
    struct so_list *so_info;
    CORE_ADDR dp;
    CORE_ADDR dyn[last_dyncall_enum];
  }
obj_private_data_t;

#define OBJ_PRIVATE_ALLOC pa_obj_private_alloc
struct objfile;
obj_private_data_t *pa_obj_private_alloc (struct objfile *);


/* Used to match stub code sequences.  */
struct stub_struc
  {
    unsigned int insn;
    unsigned int mask;
    int offset;
  };

enum stub_type {
  pa_stub_none,
  pa_stub_long_branch,
  pa_stub_long_branch_shared,
  pa_stub_import,
  pa_stub_import_shared,
  pa_stub_import_multi,
  pa_stub_import_multi_shared,
  pa_stub_lazy_link,
  pa_stub_export,
  pa64_stub_import
};

enum stub_type is_pa_stub (CORE_ADDR, const struct stub_struc *, CORE_ADDR *);

/* INIT_EXTRA_FRAME_INFO needs the PC.  */
#define	INIT_FRAME_PC(FROMLEAF, PREV)	/* nothing */
#define INIT_FRAME_PC_FIRST(FROMLEAF, PREV) \
  (PREV)->pc = ((FROMLEAF) ? SAVED_PC_AFTER_CALL ((PREV)->next)		     \
		: (PREV)->next ? FRAME_SAVED_PC ((PREV)->next)		     \
		: PA_IN_SYSCALL (NULL) ? read_register (PA_GR31_REGNUM) & ~3 \
		: read_pc ())

struct frame_extra_info
  {
    CORE_ADDR sp_adjust_insn;
    CORE_ADDR fp_adjust_insn;
    CORE_ADDR rp_save_insn;
  };


/* If PC is in some function-call trampoline code, return the PC
   where the function itself actually starts.  If not, return NULL.  */

#undef SKIP_TRAMPOLINE_CODE
#define	SKIP_TRAMPOLINE_CODE(pc) pa_skip_trampoline_code (pc, NULL)
extern CORE_ADDR pa_skip_trampoline_code (CORE_ADDR, char *);

/* Return non-zero if we are in an appropriate trampoline. */
#undef IN_SOLIB_CALL_TRAMPOLINE
#define IN_SOLIB_CALL_TRAMPOLINE(pc, name) \
   pa_in_solib_call_trampoline (pc, name)
extern int pa_in_solib_call_trampoline (CORE_ADDR, char *);

#define IN_SOLIB_RETURN_TRAMPOLINE(pc, name) \
  pa_in_solib_return_trampoline (pc, name)
extern int pa_in_solib_return_trampoline (CORE_ADDR, char *);

/* elz: Return a large value, which is stored on the stack at addr.
   This is defined only for the hppa, at this moment. 
   EXTRACT_STRUCT_VALUE_ADDRESS is not called anymore, because it assumes
   that on exit from a called function which returns a large structure on
   the stack, the address of the ret structure is still in register 28.
   Unfortunately this register is usually overwritten by the called
   function itself, on hppa.  This is specified in the calling convention
   doc. As far as I know, the only way to get the return value is to have
   the caller tell us where it told the callee to put it, rather than have
   the callee tell us.  */
#define VALUE_RETURNED_FROM_STACK(valtype,addr) \
  pa_value_returned_from_stack (valtype, addr)
extern struct value *pa_value_returned_from_stack (struct type *, CORE_ADDR);

/* Sometimes we may pluck out a minimal symbol that has a negative
   address.

   An example of this occurs when an a.out is linked against a foo.sl.
   The foo.sl defines a global bar(), and the a.out declares a signature
   for bar().  However, the a.out doesn't directly call bar(), but passes
   its address in another call.

   If you have this scenario and attempt to "break bar" before running,
   gdb will find a minimal symbol for bar() in the a.out.  But that
   symbol's address will be negative.  What this appears to denote is
   an index backwards from the base of the procedure linkage table (PLT)
   into the data linkage table (DLT), the end of which is contiguous
   with the start of the PLT.  This is clearly not a valid address for
   us to set a breakpoint on.

   Note that one must be careful in how one checks for a negative address.
   0xc0000000 is a legitimate address of something in a shared text
   segment, for example.  Since I don't know what the possible range
   is of these "really, truly negative" addresses that come from the
   minimal symbols, I'm resorting to the gross hack of checking the
   top byte of the address for all 1's.  Sigh.
 */
#define PC_REQUIRES_RUN_BEFORE_USE(pc) \
  (! target_has_stack && (pc & 0xFF000000))

/* When fetching register values from an inferior or a core file,
   clean them up using this macro.  BUF is a char pointer to
   the raw value of the register in the registers[] array.  */

#define	CLEAN_UP_REGISTER_VALUE(regno, buf) \
  do {	\
    if ((regno) == PA_PCOQ_HEAD_REGNUM || (regno) == PA_PCOQ_TAIL_REGNUM) \
      (buf)[sizeof(CORE_ADDR) -1] &= ~0x3; \
  } while (0)

/* PA specific macro to see if the current instruction is nullified. */
#ifndef INSTRUCTION_NULLIFIED
#define INSTRUCTION_NULLIFIED \
  (((int) read_register (PA_IPSW_REGNUM) & PSW_N) && ! PA_IN_SYSCALL (NULL))
#endif

/* The low two bits of the PC on the PA contain the privilege level.  Some
   genius implementing a (non-GCC) compiler apparently decided this means
   that "addresses" in a text section therefore include a privilege level,
   and thus symbol tables should contain these bits.  This seems like a
   bonehead thing to do--anyway, it seems to work for our purposes to just
   ignore those bits.  */
#define SMASH_TEXT_ADDRESS(addr) ((addr) &= ~0x3)

/* For a number of horrible reasons we may have to adjust the location
   of variables on the stack.  Ugh.  */
#define HPREAD_ADJUST_STACK_ADDRESS(ADDR) hpread_adjust_stack_address(ADDR)

extern int hpread_adjust_stack_address (CORE_ADDR);

/* Here's how to step off a permanent breakpoint.  */
#define SKIP_PERMANENT_BREAKPOINT pa_skip_permanent_breakpoint
extern void pa_skip_permanent_breakpoint (void);

/* On HP-UX, certain system routines (millicode) have names beginning
   with $ or $$, e.g. $$dyncall, which handles inter-space procedure
   calls on PA-RISC.  Tell the expression parser to check for those
   when parsing tokens that begin with "$".  */
#define SYMBOLS_CAN_START_WITH_DOLLAR 1

#endif

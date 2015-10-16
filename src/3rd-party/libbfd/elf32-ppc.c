/* PowerPC-specific support for 32-bit ELF
   Copyright (C) 1994-2014 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */


/* This file is based on a preliminary PowerPC ELF ABI.  The
   information may not match the final PowerPC ELF ABI.  It includes
   suggestions from the in-progress Embedded PowerPC ABI, and that
   information may also not match.  */

#include "sysdep.h"
#include <stdarg.h>
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/ppc.h"
#include "elf32-ppc.h"
#include "elf-vxworks.h"
#include "dwarf2.h"
#include "elf-linux-psinfo.h"

typedef enum split16_format_type
{
  split16a_type = 0,
  split16d_type
}
split16_format_type;

/* RELA relocations are used here.  */

static bfd_reloc_status_type ppc_elf_addr16_ha_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc_elf_unhandled_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);

/* Branch prediction bit for branch taken relocs.  */
#define BRANCH_PREDICT_BIT 0x200000
/* Mask to set RA in memory instructions.  */
#define RA_REGISTER_MASK 0x001f0000
/* Value to shift register by to insert RA.  */
#define RA_REGISTER_SHIFT 16

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER "/usr/lib/ld.so.1"

/* For old-style PLT.  */
/* The number of single-slot PLT entries (the rest use two slots).  */
#define PLT_NUM_SINGLE_ENTRIES 8192

/* For new-style .glink and .plt.  */
#define GLINK_PLTRESOLVE 16*4
#define GLINK_ENTRY_SIZE 4*4
#define TLS_GET_ADDR_GLINK_SIZE 12*4

/* VxWorks uses its own plt layout, filled in by the static linker.  */

/* The standard VxWorks PLT entry.  */
#define VXWORKS_PLT_ENTRY_SIZE 32
static const bfd_vma ppc_elf_vxworks_plt_entry
    [VXWORKS_PLT_ENTRY_SIZE / 4] =
  {
    0x3d800000, /* lis     r12,0                 */
    0x818c0000, /* lwz     r12,0(r12)            */
    0x7d8903a6, /* mtctr   r12                   */
    0x4e800420, /* bctr                          */
    0x39600000, /* li      r11,0                 */
    0x48000000, /* b       14 <.PLT0resolve+0x4> */
    0x60000000, /* nop                           */
    0x60000000, /* nop                           */
  };
static const bfd_vma ppc_elf_vxworks_pic_plt_entry
    [VXWORKS_PLT_ENTRY_SIZE / 4] =
  {
    0x3d9e0000, /* addis r12,r30,0 */
    0x818c0000, /* lwz	 r12,0(r12) */
    0x7d8903a6, /* mtctr r12 */
    0x4e800420, /* bctr */
    0x39600000, /* li	 r11,0 */
    0x48000000, /* b	 14 <.PLT0resolve+0x4> 14: R_PPC_REL24 .PLTresolve */
    0x60000000, /* nop */
    0x60000000, /* nop */
  };

/* The initial VxWorks PLT entry.  */
#define VXWORKS_PLT_INITIAL_ENTRY_SIZE 32
static const bfd_vma ppc_elf_vxworks_plt0_entry
    [VXWORKS_PLT_INITIAL_ENTRY_SIZE / 4] =
  {
    0x3d800000, /* lis     r12,0        */
    0x398c0000, /* addi    r12,r12,0    */
    0x800c0008, /* lwz     r0,8(r12)    */
    0x7c0903a6, /* mtctr   r0           */
    0x818c0004, /* lwz     r12,4(r12)   */
    0x4e800420, /* bctr                 */
    0x60000000, /* nop                  */
    0x60000000, /* nop                  */
  };
static const bfd_vma ppc_elf_vxworks_pic_plt0_entry
    [VXWORKS_PLT_INITIAL_ENTRY_SIZE / 4] =
  {
    0x819e0008, /* lwz	 r12,8(r30) */
    0x7d8903a6, /* mtctr r12        */
    0x819e0004, /* lwz	 r12,4(r30) */
    0x4e800420, /* bctr             */
    0x60000000, /* nop              */
    0x60000000, /* nop              */
    0x60000000, /* nop              */
    0x60000000, /* nop              */
  };

/* For executables, we have some additional relocations in
   .rela.plt.unloaded, for the kernel loader.  */

/* The number of non-JMP_SLOT relocations per PLT0 slot. */
#define VXWORKS_PLT_NON_JMP_SLOT_RELOCS 3
/* The number of relocations in the PLTResolve slot. */
#define VXWORKS_PLTRESOLVE_RELOCS 2
/* The number of relocations in the PLTResolve slot when when creating
   a shared library. */
#define VXWORKS_PLTRESOLVE_RELOCS_SHLIB 0

/* Some instructions.  */
#define ADDIS_11_11	0x3d6b0000
#define ADDIS_11_30	0x3d7e0000
#define ADDIS_12_12	0x3d8c0000
#define ADDI_11_11	0x396b0000
#define ADD_0_11_11	0x7c0b5a14
#define ADD_3_12_2	0x7c6c1214
#define ADD_11_0_11	0x7d605a14
#define B		0x48000000
#define BA		0x48000002
#define BCL_20_31	0x429f0005
#define BCTR		0x4e800420
#define BEQLR		0x4d820020
#define CMPWI_11_0	0x2c0b0000
#define LIS_11		0x3d600000
#define LIS_12		0x3d800000
#define LWZU_0_12	0x840c0000
#define LWZ_0_12	0x800c0000
#define LWZ_11_3	0x81630000
#define LWZ_11_11	0x816b0000
#define LWZ_11_30	0x817e0000
#define LWZ_12_3	0x81830000
#define LWZ_12_12	0x818c0000
#define MR_0_3		0x7c601b78
#define MR_3_0		0x7c030378
#define MFLR_0		0x7c0802a6
#define MFLR_12		0x7d8802a6
#define MTCTR_0		0x7c0903a6
#define MTCTR_11	0x7d6903a6
#define MTLR_0		0x7c0803a6
#define NOP		0x60000000
#define SUB_11_11_12	0x7d6c5850

/* Offset of tp and dtp pointers from start of TLS block.  */
#define TP_OFFSET	0x7000
#define DTP_OFFSET	0x8000

/* The value of a defined global symbol.  */
#define SYM_VAL(SYM) \
  ((SYM)->root.u.def.section->output_section->vma	\
   + (SYM)->root.u.def.section->output_offset		\
   + (SYM)->root.u.def.value)

static reloc_howto_type *ppc_elf_howto_table[R_PPC_max];

static reloc_howto_type ppc_elf_howto_raw[] = {
  /* This reloc does nothing.  */
  HOWTO (R_PPC_NONE,		/* type */
	 0,			/* rightshift */
	 3,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_NONE",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A standard 32 bit relocation.  */
  HOWTO (R_PPC_ADDR32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* An absolute 26 bit branch; the lower two bits must be zero.
     FIXME: we don't check that, we just clear them.  */
  HOWTO (R_PPC_ADDR24,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 26,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR24",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x3fffffc,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A standard 16 bit relocation.  */
  HOWTO (R_PPC_ADDR16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit relocation without overflow.  */
  HOWTO (R_PPC_ADDR16_LO,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR16_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The high order 16 bits of an address.  */
  HOWTO (R_PPC_ADDR16_HI,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR16_HI",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The high order 16 bits of an address, plus 1 if the contents of
     the low 16 bits, treated as a signed number, is negative.  */
  HOWTO (R_PPC_ADDR16_HA,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_addr16_ha_reloc, /* special_function */
	 "R_PPC_ADDR16_HA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* An absolute 16 bit branch; the lower two bits must be zero.
     FIXME: we don't check that, we just clear them.  */
  HOWTO (R_PPC_ADDR14,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR14",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xfffc,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* An absolute 16 bit branch, for which bit 10 should be set to
     indicate that the branch is expected to be taken.	The lower two
     bits must be zero.  */
  HOWTO (R_PPC_ADDR14_BRTAKEN,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR14_BRTAKEN",/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xfffc,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* An absolute 16 bit branch, for which bit 10 should be set to
     indicate that the branch is not expected to be taken.  The lower
     two bits must be zero.  */
  HOWTO (R_PPC_ADDR14_BRNTAKEN, /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_ADDR14_BRNTAKEN",/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xfffc,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A relative 26 bit branch; the lower two bits must be zero.  */
  HOWTO (R_PPC_REL24,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 26,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL24",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x3fffffc,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A relative 16 bit branch; the lower two bits must be zero.  */
  HOWTO (R_PPC_REL14,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL14",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xfffc,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A relative 16 bit branch.  Bit 10 should be set to indicate that
     the branch is expected to be taken.  The lower two bits must be
     zero.  */
  HOWTO (R_PPC_REL14_BRTAKEN,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL14_BRTAKEN",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xfffc,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A relative 16 bit branch.  Bit 10 should be set to indicate that
     the branch is not expected to be taken.  The lower two bits must
     be zero.  */
  HOWTO (R_PPC_REL14_BRNTAKEN,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL14_BRNTAKEN",/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xfffc,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* Like R_PPC_ADDR16, but referring to the GOT table entry for the
     symbol.  */
  HOWTO (R_PPC_GOT16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_GOT16",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_ADDR16_LO, but referring to the GOT table entry for
     the symbol.  */
  HOWTO (R_PPC_GOT16_LO,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_GOT16_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_ADDR16_HI, but referring to the GOT table entry for
     the symbol.  */
  HOWTO (R_PPC_GOT16_HI,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_GOT16_HI",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		 /* pcrel_offset */

  /* Like R_PPC_ADDR16_HA, but referring to the GOT table entry for
     the symbol.  */
  HOWTO (R_PPC_GOT16_HA,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_addr16_ha_reloc, /* special_function */
	 "R_PPC_GOT16_HA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_REL24, but referring to the procedure linkage table
     entry for the symbol.  */
  HOWTO (R_PPC_PLTREL24,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 26,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_PLTREL24",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x3fffffc,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* This is used only by the dynamic linker.  The symbol should exist
     both in the object being run and in some shared library.  The
     dynamic linker copies the data addressed by the symbol from the
     shared library into the object, because the object being
     run has to have the data at some particular address.  */
  HOWTO (R_PPC_COPY,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_COPY",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_ADDR32, but used when setting global offset table
     entries.  */
  HOWTO (R_PPC_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_GLOB_DAT",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Marks a procedure linkage table entry for a symbol.  */
  HOWTO (R_PPC_JMP_SLOT,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_JMP_SLOT",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Used only by the dynamic linker.  When the object is run, this
     longword is set to the load address of the object, plus the
     addend.  */
  HOWTO (R_PPC_RELATIVE,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_RELATIVE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_REL24, but uses the value of the symbol within the
     object rather than the final value.  Normally used for
     _GLOBAL_OFFSET_TABLE_.  */
  HOWTO (R_PPC_LOCAL24PC,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 26,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_LOCAL24PC",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x3fffffc,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* Like R_PPC_ADDR32, but may be unaligned.  */
  HOWTO (R_PPC_UADDR32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_UADDR32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_ADDR16, but may be unaligned.  */
  HOWTO (R_PPC_UADDR16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_UADDR16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit PC relative */
  HOWTO (R_PPC_REL32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL32",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* 32-bit relocation to the symbol's procedure linkage table.
     FIXME: not supported.  */
  HOWTO (R_PPC_PLT32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_PLT32",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit PC relative relocation to the symbol's procedure linkage table.
     FIXME: not supported.  */
  HOWTO (R_PPC_PLTREL32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_PLTREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* Like R_PPC_ADDR16_LO, but referring to the PLT table entry for
     the symbol.  */
  HOWTO (R_PPC_PLT16_LO,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_PLT16_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_ADDR16_HI, but referring to the PLT table entry for
     the symbol.  */
  HOWTO (R_PPC_PLT16_HI,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_PLT16_HI",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		 /* pcrel_offset */

  /* Like R_PPC_ADDR16_HA, but referring to the PLT table entry for
     the symbol.  */
  HOWTO (R_PPC_PLT16_HA,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_addr16_ha_reloc, /* special_function */
	 "R_PPC_PLT16_HA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A sign-extended 16 bit value relative to _SDA_BASE_, for use with
     small data items.  */
  HOWTO (R_PPC_SDAREL16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_SDAREL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16-bit section relative relocation.  */
  HOWTO (R_PPC_SECTOFF,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_SECTOFF",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16-bit lower half section relative relocation.  */
  HOWTO (R_PPC_SECTOFF_LO,	  /* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_SECTOFF_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16-bit upper half section relative relocation.  */
  HOWTO (R_PPC_SECTOFF_HI,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_SECTOFF_HI",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		 /* pcrel_offset */

  /* 16-bit upper half adjusted section relative relocation.  */
  HOWTO (R_PPC_SECTOFF_HA,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_addr16_ha_reloc, /* special_function */
	 "R_PPC_SECTOFF_HA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Marker relocs for TLS.  */
  HOWTO (R_PPC_TLS,
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_TLS",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_PPC_TLSGD,
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_PPC_TLSGD",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_PPC_TLSLD,
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_PPC_TLSLD",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Computes the load module index of the load module that contains the
     definition of its TLS sym.  */
  HOWTO (R_PPC_DTPMOD32,
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_DTPMOD32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Computes a dtv-relative displacement, the difference between the value
     of sym+add and the base address of the thread-local storage block that
     contains the definition of sym, minus 0x8000.  */
  HOWTO (R_PPC_DTPREL32,
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_DTPREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit dtprel reloc.  */
  HOWTO (R_PPC_DTPREL16,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_DTPREL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like DTPREL16, but no overflow.  */
  HOWTO (R_PPC_DTPREL16_LO,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_DTPREL16_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like DTPREL16_LO, but next higher group of 16 bits.  */
  HOWTO (R_PPC_DTPREL16_HI,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_DTPREL16_HI",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like DTPREL16_HI, but adjust for low 16 bits.  */
  HOWTO (R_PPC_DTPREL16_HA,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_DTPREL16_HA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Computes a tp-relative displacement, the difference between the value of
     sym+add and the value of the thread pointer (r13).  */
  HOWTO (R_PPC_TPREL32,
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_TPREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit tprel reloc.  */
  HOWTO (R_PPC_TPREL16,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_TPREL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like TPREL16, but no overflow.  */
  HOWTO (R_PPC_TPREL16_LO,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_TPREL16_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like TPREL16_LO, but next higher group of 16 bits.  */
  HOWTO (R_PPC_TPREL16_HI,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_TPREL16_HI",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like TPREL16_HI, but adjust for low 16 bits.  */
  HOWTO (R_PPC_TPREL16_HA,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_TPREL16_HA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Allocates two contiguous entries in the GOT to hold a tls_index structure,
     with values (sym+add)@dtpmod and (sym+add)@dtprel, and computes the offset
     to the first entry.  */
  HOWTO (R_PPC_GOT_TLSGD16,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSGD16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TLSGD16, but no overflow.  */
  HOWTO (R_PPC_GOT_TLSGD16_LO,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSGD16_LO", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TLSGD16_LO, but next higher group of 16 bits.  */
  HOWTO (R_PPC_GOT_TLSGD16_HI,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSGD16_HI", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TLSGD16_HI, but adjust for low 16 bits.  */
  HOWTO (R_PPC_GOT_TLSGD16_HA,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSGD16_HA", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Allocates two contiguous entries in the GOT to hold a tls_index structure,
     with values (sym+add)@dtpmod and zero, and computes the offset to the
     first entry.  */
  HOWTO (R_PPC_GOT_TLSLD16,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSLD16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TLSLD16, but no overflow.  */
  HOWTO (R_PPC_GOT_TLSLD16_LO,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSLD16_LO", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TLSLD16_LO, but next higher group of 16 bits.  */
  HOWTO (R_PPC_GOT_TLSLD16_HI,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSLD16_HI", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TLSLD16_HI, but adjust for low 16 bits.  */
  HOWTO (R_PPC_GOT_TLSLD16_HA,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TLSLD16_HA", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Allocates an entry in the GOT with value (sym+add)@dtprel, and computes
     the offset to the entry.  */
  HOWTO (R_PPC_GOT_DTPREL16,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_DTPREL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_DTPREL16, but no overflow.  */
  HOWTO (R_PPC_GOT_DTPREL16_LO,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_DTPREL16_LO", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_DTPREL16_LO, but next higher group of 16 bits.  */
  HOWTO (R_PPC_GOT_DTPREL16_HI,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_DTPREL16_HI", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_DTPREL16_HI, but adjust for low 16 bits.  */
  HOWTO (R_PPC_GOT_DTPREL16_HA,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_DTPREL16_HA", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Allocates an entry in the GOT with value (sym+add)@tprel, and computes the
     offset to the entry.  */
  HOWTO (R_PPC_GOT_TPREL16,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TPREL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TPREL16, but no overflow.  */
  HOWTO (R_PPC_GOT_TPREL16_LO,
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TPREL16_LO", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TPREL16_LO, but next higher group of 16 bits.  */
  HOWTO (R_PPC_GOT_TPREL16_HI,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TPREL16_HI", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like GOT_TPREL16_HI, but adjust for low 16 bits.  */
  HOWTO (R_PPC_GOT_TPREL16_HA,
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_unhandled_reloc, /* special_function */
	 "R_PPC_GOT_TPREL16_HA", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The remaining relocs are from the Embedded ELF ABI, and are not
     in the SVR4 ELF ABI.  */

  /* 32 bit value resulting from the addend minus the symbol.  */
  HOWTO (R_PPC_EMB_NADDR32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_NADDR32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16 bit value resulting from the addend minus the symbol.  */
  HOWTO (R_PPC_EMB_NADDR16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_NADDR16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16 bit value resulting from the addend minus the symbol.  */
  HOWTO (R_PPC_EMB_NADDR16_LO,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_ADDR16_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The high order 16 bits of the addend minus the symbol.  */
  HOWTO (R_PPC_EMB_NADDR16_HI,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_NADDR16_HI", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The high order 16 bits of the result of the addend minus the address,
     plus 1 if the contents of the low 16 bits, treated as a signed number,
     is negative.  */
  HOWTO (R_PPC_EMB_NADDR16_HA,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_addr16_ha_reloc, /* special_function */
	 "R_PPC_EMB_NADDR16_HA", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16 bit value resulting from allocating a 4 byte word to hold an
     address in the .sdata section, and returning the offset from
     _SDA_BASE_ for that relocation.  */
  HOWTO (R_PPC_EMB_SDAI16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_SDAI16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16 bit value resulting from allocating a 4 byte word to hold an
     address in the .sdata2 section, and returning the offset from
     _SDA2_BASE_ for that relocation.  */
  HOWTO (R_PPC_EMB_SDA2I16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_SDA2I16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A sign-extended 16 bit value relative to _SDA2_BASE_, for use with
     small data items.	 */
  HOWTO (R_PPC_EMB_SDA2REL,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_SDA2REL",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Relocate against either _SDA_BASE_ or _SDA2_BASE_, filling in the 16 bit
     signed offset from the appropriate base, and filling in the register
     field with the appropriate register (0, 2, or 13).  */
  HOWTO (R_PPC_EMB_SDA21,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_SDA21",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Relocation not handled: R_PPC_EMB_MRKREF */
  /* Relocation not handled: R_PPC_EMB_RELSEC16 */
  /* Relocation not handled: R_PPC_EMB_RELST_LO */
  /* Relocation not handled: R_PPC_EMB_RELST_HI */
  /* Relocation not handled: R_PPC_EMB_RELST_HA */
  /* Relocation not handled: R_PPC_EMB_BIT_FLD */

  /* PC relative relocation against either _SDA_BASE_ or _SDA2_BASE_, filling
     in the 16 bit signed offset from the appropriate base, and filling in the
     register field with the appropriate register (0, 2, or 13).  */
  HOWTO (R_PPC_EMB_RELSDA,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_EMB_RELSDA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A relative 8 bit branch.  */
  HOWTO (R_PPC_VLE_REL8,	/* type */
	 1,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_VLE_REL8",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A relative 15 bit branch.  */
  HOWTO (R_PPC_VLE_REL15,	/* type */
	 1,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 15,			/* bitsize */
	 TRUE,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_VLE_REL15",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xfe,			/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A relative 24 bit branch.  */
  HOWTO (R_PPC_VLE_REL24,	/* type */
	 1,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 24,			/* bitsize */
	 TRUE,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_VLE_REL24",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1fffffe,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* The 16 LSBS in split16a format.  */
  HOWTO (R_PPC_VLE_LO16A,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_LO16A",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f007ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The 16 LSBS in split16d format.  */
  HOWTO (R_PPC_VLE_LO16D,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_LO16D",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f07ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 split16a format.  */
  HOWTO (R_PPC_VLE_HI16A,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_HI16A",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f007ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 split16d format.  */
  HOWTO (R_PPC_VLE_HI16D,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_HI16D",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f07ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 (High Adjusted) in split16a format.  */
  HOWTO (R_PPC_VLE_HA16A,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_HA16A",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f007ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 (High Adjusted) in split16d format.  */
  HOWTO (R_PPC_VLE_HA16D,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_HA16D",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f07ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* This reloc is like R_PPC_EMB_SDA21 but only applies to e_add16i
     instructions.  If the register base is 0 then the linker changes
     the e_add16i to an e_li instruction.  */
  HOWTO (R_PPC_VLE_SDA21,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_VLE_SDA21",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Like R_PPC_VLE_SDA21 but ignore overflow.  */
  HOWTO (R_PPC_VLE_SDA21_LO,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_VLE_SDA21_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The 16 LSBS relative to _SDA_BASE_ in split16a format.  */
  HOWTO (R_PPC_VLE_SDAREL_LO16A,/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_SDAREL_LO16A",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f007ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* The 16 LSBS relative to _SDA_BASE_ in split16d format.  */
  HOWTO (R_PPC_VLE_SDAREL_LO16D, /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_SDAREL_LO16D",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f07ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 relative to _SDA_BASE_ in split16a format.  */
  HOWTO (R_PPC_VLE_SDAREL_HI16A,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_SDAREL_HI16A",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f007ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 relative to _SDA_BASE_ in split16d format.  */
  HOWTO (R_PPC_VLE_SDAREL_HI16D,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_SDAREL_HI16D",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f07ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 (HA) relative to _SDA_BASE split16a format.  */
  HOWTO (R_PPC_VLE_SDAREL_HA16A,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_SDAREL_HA16A",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f007ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Bits 16-31 (HA) relative to _SDA_BASE split16d format.  */
  HOWTO (R_PPC_VLE_SDAREL_HA16D,	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_VLE_SDAREL_HA16D",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x1f07ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_PPC_IRELATIVE,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	 /* special_function */
	 "R_PPC_IRELATIVE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit relative relocation.  */
  HOWTO (R_PPC_REL16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL16",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A 16 bit relative relocation without overflow.  */
  HOWTO (R_PPC_REL16_LO,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL16_LO",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* The high order 16 bits of a relative address.  */
  HOWTO (R_PPC_REL16_HI,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_REL16_HI",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* The high order 16 bits of a relative address, plus 1 if the contents of
     the low 16 bits, treated as a signed number, is negative.  */
  HOWTO (R_PPC_REL16_HA,	/* type */
	 16,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 ppc_elf_addr16_ha_reloc, /* special_function */
	 "R_PPC_REL16_HA",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_PPC_GNU_VTINHERIT,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_PPC_GNU_VTINHERIT",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_PPC_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_PPC_GNU_VTENTRY",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Phony reloc to handle AIX style TOC entries.  */
  HOWTO (R_PPC_TOC16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PPC_TOC16",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */
};

/* External 32-bit PPC structure for PRPSINFO.  This structure is
   ABI-defined, thus we choose to use char arrays here in order to
   avoid dealing with different types in different architectures.

   The PPC 32-bit structure uses int for `pr_uid' and `pr_gid' while
   most non-PPC architectures use `short int'.

   This structure will ultimately be written in the corefile's note
   section, as the PRPSINFO.  */

struct elf_external_ppc_linux_prpsinfo32
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    char pr_flag[4];			/* Flags.  */
    char pr_uid[4];
    char pr_gid[4];
    char pr_pid[4];
    char pr_ppid[4];
    char pr_pgrp[4];
    char pr_sid[4];
    char pr_fname[16];			/* Filename of executable.  */
    char pr_psargs[80];			/* Initial part of arg list.  */
  };

/* Helper macro to swap (properly handling endianess) things from the
   `elf_internal_prpsinfo' structure to the `elf_external_ppc_prpsinfo32'
   structure.

   Note that FROM should be a pointer, and TO should be the explicit type.  */

#define PPC_LINUX_PRPSINFO32_SWAP_FIELDS(abfd, from, to)	      \
  do								      \
    {								      \
      H_PUT_8 (abfd, from->pr_state, &to.pr_state);		      \
      H_PUT_8 (abfd, from->pr_sname, &to.pr_sname);		      \
      H_PUT_8 (abfd, from->pr_zomb, &to.pr_zomb);		      \
      H_PUT_8 (abfd, from->pr_nice, &to.pr_nice);		      \
      H_PUT_32 (abfd, from->pr_flag, to.pr_flag);		      \
      H_PUT_32 (abfd, from->pr_uid, to.pr_uid);			      \
      H_PUT_32 (abfd, from->pr_gid, to.pr_gid);			      \
      H_PUT_32 (abfd, from->pr_pid, to.pr_pid);			      \
      H_PUT_32 (abfd, from->pr_ppid, to.pr_ppid);		      \
      H_PUT_32 (abfd, from->pr_pgrp, to.pr_pgrp);		      \
      H_PUT_32 (abfd, from->pr_sid, to.pr_sid);			      \
      strncpy (to.pr_fname, from->pr_fname, sizeof (to.pr_fname));    \
      strncpy (to.pr_psargs, from->pr_psargs, sizeof (to.pr_psargs)); \
    } while (0)


/* Initialize the ppc_elf_howto_table, so that linear accesses can be done.  */

static void
ppc_elf_howto_init (void)
{
  unsigned int i, type;

  for (i = 0;
       i < sizeof (ppc_elf_howto_raw) / sizeof (ppc_elf_howto_raw[0]);
       i++)
    {
      type = ppc_elf_howto_raw[i].type;
      if (type >= (sizeof (ppc_elf_howto_table)
		   / sizeof (ppc_elf_howto_table[0])))
	abort ();
      ppc_elf_howto_table[type] = &ppc_elf_howto_raw[i];
    }
}

static reloc_howto_type *
ppc_elf_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			   bfd_reloc_code_real_type code)
{
  enum elf_ppc_reloc_type r;

  /* Initialize howto table if not already done.  */
  if (!ppc_elf_howto_table[R_PPC_ADDR32])
    ppc_elf_howto_init ();

  switch (code)
    {
    default:
      return NULL;

    case BFD_RELOC_NONE:		r = R_PPC_NONE;			break;
    case BFD_RELOC_32:			r = R_PPC_ADDR32;		break;
    case BFD_RELOC_PPC_BA26:		r = R_PPC_ADDR24;		break;
    case BFD_RELOC_PPC64_ADDR16_DS:
    case BFD_RELOC_16:			r = R_PPC_ADDR16;		break;
    case BFD_RELOC_PPC64_ADDR16_LO_DS:
    case BFD_RELOC_LO16:		r = R_PPC_ADDR16_LO;		break;
    case BFD_RELOC_HI16:		r = R_PPC_ADDR16_HI;		break;
    case BFD_RELOC_HI16_S:		r = R_PPC_ADDR16_HA;		break;
    case BFD_RELOC_PPC_BA16:		r = R_PPC_ADDR14;		break;
    case BFD_RELOC_PPC_BA16_BRTAKEN:	r = R_PPC_ADDR14_BRTAKEN;	break;
    case BFD_RELOC_PPC_BA16_BRNTAKEN:	r = R_PPC_ADDR14_BRNTAKEN;	break;
    case BFD_RELOC_PPC_B26:		r = R_PPC_REL24;		break;
    case BFD_RELOC_PPC_B16:		r = R_PPC_REL14;		break;
    case BFD_RELOC_PPC_B16_BRTAKEN:	r = R_PPC_REL14_BRTAKEN;	break;
    case BFD_RELOC_PPC_B16_BRNTAKEN:	r = R_PPC_REL14_BRNTAKEN;	break;
    case BFD_RELOC_PPC64_GOT16_DS:
    case BFD_RELOC_16_GOTOFF:		r = R_PPC_GOT16;		break;
    case BFD_RELOC_PPC64_GOT16_LO_DS:
    case BFD_RELOC_LO16_GOTOFF:		r = R_PPC_GOT16_LO;		break;
    case BFD_RELOC_HI16_GOTOFF:		r = R_PPC_GOT16_HI;		break;
    case BFD_RELOC_HI16_S_GOTOFF:	r = R_PPC_GOT16_HA;		break;
    case BFD_RELOC_24_PLT_PCREL:	r = R_PPC_PLTREL24;		break;
    case BFD_RELOC_PPC_COPY:		r = R_PPC_COPY;			break;
    case BFD_RELOC_PPC_GLOB_DAT:	r = R_PPC_GLOB_DAT;		break;
    case BFD_RELOC_PPC_LOCAL24PC:	r = R_PPC_LOCAL24PC;		break;
    case BFD_RELOC_32_PCREL:		r = R_PPC_REL32;		break;
    case BFD_RELOC_32_PLTOFF:		r = R_PPC_PLT32;		break;
    case BFD_RELOC_32_PLT_PCREL:	r = R_PPC_PLTREL32;		break;
    case BFD_RELOC_PPC64_PLT16_LO_DS:
    case BFD_RELOC_LO16_PLTOFF:		r = R_PPC_PLT16_LO;		break;
    case BFD_RELOC_HI16_PLTOFF:		r = R_PPC_PLT16_HI;		break;
    case BFD_RELOC_HI16_S_PLTOFF:	r = R_PPC_PLT16_HA;		break;
    case BFD_RELOC_GPREL16:		r = R_PPC_SDAREL16;		break;
    case BFD_RELOC_PPC64_SECTOFF_DS:
    case BFD_RELOC_16_BASEREL:		r = R_PPC_SECTOFF;		break;
    case BFD_RELOC_PPC64_SECTOFF_LO_DS:
    case BFD_RELOC_LO16_BASEREL:	r = R_PPC_SECTOFF_LO;		break;
    case BFD_RELOC_HI16_BASEREL:	r = R_PPC_SECTOFF_HI;		break;
    case BFD_RELOC_HI16_S_BASEREL:	r = R_PPC_SECTOFF_HA;		break;
    case BFD_RELOC_CTOR:		r = R_PPC_ADDR32;		break;
    case BFD_RELOC_PPC64_TOC16_DS:
    case BFD_RELOC_PPC_TOC16:		r = R_PPC_TOC16;		break;
    case BFD_RELOC_PPC_TLS:		r = R_PPC_TLS;			break;
    case BFD_RELOC_PPC_TLSGD:		r = R_PPC_TLSGD;		break;
    case BFD_RELOC_PPC_TLSLD:		r = R_PPC_TLSLD;		break;
    case BFD_RELOC_PPC_DTPMOD:		r = R_PPC_DTPMOD32;		break;
    case BFD_RELOC_PPC64_TPREL16_DS:
    case BFD_RELOC_PPC_TPREL16:		r = R_PPC_TPREL16;		break;
    case BFD_RELOC_PPC64_TPREL16_LO_DS:
    case BFD_RELOC_PPC_TPREL16_LO:	r = R_PPC_TPREL16_LO;		break;
    case BFD_RELOC_PPC_TPREL16_HI:	r = R_PPC_TPREL16_HI;		break;
    case BFD_RELOC_PPC_TPREL16_HA:	r = R_PPC_TPREL16_HA;		break;
    case BFD_RELOC_PPC_TPREL:		r = R_PPC_TPREL32;		break;
    case BFD_RELOC_PPC64_DTPREL16_DS:
    case BFD_RELOC_PPC_DTPREL16:	r = R_PPC_DTPREL16;		break;
    case BFD_RELOC_PPC64_DTPREL16_LO_DS:
    case BFD_RELOC_PPC_DTPREL16_LO:	r = R_PPC_DTPREL16_LO;		break;
    case BFD_RELOC_PPC_DTPREL16_HI:	r = R_PPC_DTPREL16_HI;		break;
    case BFD_RELOC_PPC_DTPREL16_HA:	r = R_PPC_DTPREL16_HA;		break;
    case BFD_RELOC_PPC_DTPREL:		r = R_PPC_DTPREL32;		break;
    case BFD_RELOC_PPC_GOT_TLSGD16:	r = R_PPC_GOT_TLSGD16;		break;
    case BFD_RELOC_PPC_GOT_TLSGD16_LO:	r = R_PPC_GOT_TLSGD16_LO;	break;
    case BFD_RELOC_PPC_GOT_TLSGD16_HI:	r = R_PPC_GOT_TLSGD16_HI;	break;
    case BFD_RELOC_PPC_GOT_TLSGD16_HA:	r = R_PPC_GOT_TLSGD16_HA;	break;
    case BFD_RELOC_PPC_GOT_TLSLD16:	r = R_PPC_GOT_TLSLD16;		break;
    case BFD_RELOC_PPC_GOT_TLSLD16_LO:	r = R_PPC_GOT_TLSLD16_LO;	break;
    case BFD_RELOC_PPC_GOT_TLSLD16_HI:	r = R_PPC_GOT_TLSLD16_HI;	break;
    case BFD_RELOC_PPC_GOT_TLSLD16_HA:	r = R_PPC_GOT_TLSLD16_HA;	break;
    case BFD_RELOC_PPC_GOT_TPREL16:	r = R_PPC_GOT_TPREL16;		break;
    case BFD_RELOC_PPC_GOT_TPREL16_LO:	r = R_PPC_GOT_TPREL16_LO;	break;
    case BFD_RELOC_PPC_GOT_TPREL16_HI:	r = R_PPC_GOT_TPREL16_HI;	break;
    case BFD_RELOC_PPC_GOT_TPREL16_HA:	r = R_PPC_GOT_TPREL16_HA;	break;
    case BFD_RELOC_PPC_GOT_DTPREL16:	r = R_PPC_GOT_DTPREL16;		break;
    case BFD_RELOC_PPC_GOT_DTPREL16_LO:	r = R_PPC_GOT_DTPREL16_LO;	break;
    case BFD_RELOC_PPC_GOT_DTPREL16_HI:	r = R_PPC_GOT_DTPREL16_HI;	break;
    case BFD_RELOC_PPC_GOT_DTPREL16_HA:	r = R_PPC_GOT_DTPREL16_HA;	break;
    case BFD_RELOC_PPC_EMB_NADDR32:	r = R_PPC_EMB_NADDR32;		break;
    case BFD_RELOC_PPC_EMB_NADDR16:	r = R_PPC_EMB_NADDR16;		break;
    case BFD_RELOC_PPC_EMB_NADDR16_LO:	r = R_PPC_EMB_NADDR16_LO;	break;
    case BFD_RELOC_PPC_EMB_NADDR16_HI:	r = R_PPC_EMB_NADDR16_HI;	break;
    case BFD_RELOC_PPC_EMB_NADDR16_HA:	r = R_PPC_EMB_NADDR16_HA;	break;
    case BFD_RELOC_PPC_EMB_SDAI16:	r = R_PPC_EMB_SDAI16;		break;
    case BFD_RELOC_PPC_EMB_SDA2I16:	r = R_PPC_EMB_SDA2I16;		break;
    case BFD_RELOC_PPC_EMB_SDA2REL:	r = R_PPC_EMB_SDA2REL;		break;
    case BFD_RELOC_PPC_EMB_SDA21:	r = R_PPC_EMB_SDA21;		break;
    case BFD_RELOC_PPC_EMB_MRKREF:	r = R_PPC_EMB_MRKREF;		break;
    case BFD_RELOC_PPC_EMB_RELSEC16:	r = R_PPC_EMB_RELSEC16;		break;
    case BFD_RELOC_PPC_EMB_RELST_LO:	r = R_PPC_EMB_RELST_LO;		break;
    case BFD_RELOC_PPC_EMB_RELST_HI:	r = R_PPC_EMB_RELST_HI;		break;
    case BFD_RELOC_PPC_EMB_RELST_HA:	r = R_PPC_EMB_RELST_HA;		break;
    case BFD_RELOC_PPC_EMB_BIT_FLD:	r = R_PPC_EMB_BIT_FLD;		break;
    case BFD_RELOC_PPC_EMB_RELSDA:	r = R_PPC_EMB_RELSDA;		break;
    case BFD_RELOC_PPC_VLE_REL8:	r = R_PPC_VLE_REL8;		break;
    case BFD_RELOC_PPC_VLE_REL15:	r = R_PPC_VLE_REL15;		break;
    case BFD_RELOC_PPC_VLE_REL24:	r = R_PPC_VLE_REL24;		break;
    case BFD_RELOC_PPC_VLE_LO16A:	r = R_PPC_VLE_LO16A;		break;
    case BFD_RELOC_PPC_VLE_LO16D:	r = R_PPC_VLE_LO16D;		break;
    case BFD_RELOC_PPC_VLE_HI16A:	r = R_PPC_VLE_HI16A;		break;
    case BFD_RELOC_PPC_VLE_HI16D:	r = R_PPC_VLE_HI16D;		break;
    case BFD_RELOC_PPC_VLE_HA16A:	r = R_PPC_VLE_HA16A;		break;
    case BFD_RELOC_PPC_VLE_HA16D:	r = R_PPC_VLE_HA16D;		break;
    case BFD_RELOC_PPC_VLE_SDA21:	r = R_PPC_VLE_SDA21;		break;
    case BFD_RELOC_PPC_VLE_SDA21_LO:	r = R_PPC_VLE_SDA21_LO;		break;
    case BFD_RELOC_PPC_VLE_SDAREL_LO16A:
      r = R_PPC_VLE_SDAREL_LO16A;
      break;
    case BFD_RELOC_PPC_VLE_SDAREL_LO16D:
      r = R_PPC_VLE_SDAREL_LO16D;
      break;
    case BFD_RELOC_PPC_VLE_SDAREL_HI16A:
      r = R_PPC_VLE_SDAREL_HI16A;
      break;
    case BFD_RELOC_PPC_VLE_SDAREL_HI16D:
      r = R_PPC_VLE_SDAREL_HI16D;
      break;
    case BFD_RELOC_PPC_VLE_SDAREL_HA16A:
      r = R_PPC_VLE_SDAREL_HA16A;
      break;
    case BFD_RELOC_PPC_VLE_SDAREL_HA16D:
      r = R_PPC_VLE_SDAREL_HA16D;
      break;
    case BFD_RELOC_16_PCREL:		r = R_PPC_REL16;		break;
    case BFD_RELOC_LO16_PCREL:		r = R_PPC_REL16_LO;		break;
    case BFD_RELOC_HI16_PCREL:		r = R_PPC_REL16_HI;		break;
    case BFD_RELOC_HI16_S_PCREL:	r = R_PPC_REL16_HA;		break;
    case BFD_RELOC_VTABLE_INHERIT:	r = R_PPC_GNU_VTINHERIT;	break;
    case BFD_RELOC_VTABLE_ENTRY:	r = R_PPC_GNU_VTENTRY;		break;
    }

  return ppc_elf_howto_table[r];
};

static reloc_howto_type *
ppc_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			   const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (ppc_elf_howto_raw) / sizeof (ppc_elf_howto_raw[0]);
       i++)
    if (ppc_elf_howto_raw[i].name != NULL
	&& strcasecmp (ppc_elf_howto_raw[i].name, r_name) == 0)
      return &ppc_elf_howto_raw[i];

  return NULL;
}

/* Set the howto pointer for a PowerPC ELF reloc.  */

static void
ppc_elf_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
		       arelent *cache_ptr,
		       Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  /* Initialize howto table if not already done.  */
  if (!ppc_elf_howto_table[R_PPC_ADDR32])
    ppc_elf_howto_init ();

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= R_PPC_max)
    {
      (*_bfd_error_handler) (_("%B: unrecognised PPC reloc number: %d"),
			     abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      r_type = R_PPC_NONE;
    }
  cache_ptr->howto = ppc_elf_howto_table[r_type];

  /* Just because the above assert didn't trigger doesn't mean that
     ELF32_R_TYPE (dst->r_info) is necessarily a valid relocation.  */
  if (!cache_ptr->howto)
    {
      (*_bfd_error_handler) (_("%B: invalid relocation type %d"),
                             abfd, r_type);
      bfd_set_error (bfd_error_bad_value);

      cache_ptr->howto = ppc_elf_howto_table[R_PPC_NONE];
    }
}

/* Handle the R_PPC_ADDR16_HA and R_PPC_REL16_HA relocs.  */

static bfd_reloc_status_type
ppc_elf_addr16_ha_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			 arelent *reloc_entry,
			 asymbol *symbol,
			 void *data ATTRIBUTE_UNUSED,
			 asection *input_section,
			 bfd *output_bfd,
			 char **error_message ATTRIBUTE_UNUSED)
{
  bfd_vma relocation;

  if (output_bfd != NULL)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  relocation += symbol->section->output_section->vma;
  relocation += symbol->section->output_offset;
  relocation += reloc_entry->addend;
  if (reloc_entry->howto->pc_relative)
    relocation -= reloc_entry->address;

  reloc_entry->addend += (relocation & 0x8000) << 1;

  return bfd_reloc_continue;
}

static bfd_reloc_status_type
ppc_elf_unhandled_reloc (bfd *abfd,
			 arelent *reloc_entry,
			 asymbol *symbol,
			 void *data,
			 asection *input_section,
			 bfd *output_bfd,
			 char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  if (error_message != NULL)
    {
      static char buf[60];
      sprintf (buf, _("generic linker can't handle %s"),
	       reloc_entry->howto->name);
      *error_message = buf;
    }
  return bfd_reloc_dangerous;
}

/* Sections created by the linker.  */

typedef struct elf_linker_section
{
  /* Pointer to the bfd section.  */
  asection *section;
  /* Section name.  */
  const char *name;
  /* Associated bss section name.  */
  const char *bss_name;
  /* Associated symbol name.  */
  const char *sym_name;
  /* Associated symbol.  */
  struct elf_link_hash_entry *sym;
} elf_linker_section_t;

/* Linked list of allocated pointer entries.  This hangs off of the
   symbol lists, and provides allows us to return different pointers,
   based on different addend's.  */

typedef struct elf_linker_section_pointers
{
  /* next allocated pointer for this symbol */
  struct elf_linker_section_pointers *next;
  /* offset of pointer from beginning of section */
  bfd_vma offset;
  /* addend used */
  bfd_vma addend;
  /* which linker section this is */
  elf_linker_section_t *lsect;
} elf_linker_section_pointers_t;

struct ppc_elf_obj_tdata
{
  struct elf_obj_tdata elf;

  /* A mapping from local symbols to offsets into the various linker
     sections added.  This is index by the symbol index.  */
  elf_linker_section_pointers_t **linker_section_pointers;

  /* Flags used to auto-detect plt type.  */
  unsigned int makes_plt_call : 1;
  unsigned int has_rel16 : 1;
};

#define ppc_elf_tdata(bfd) \
  ((struct ppc_elf_obj_tdata *) (bfd)->tdata.any)

#define elf_local_ptr_offsets(bfd) \
  (ppc_elf_tdata (bfd)->linker_section_pointers)

#define is_ppc_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_object_id (bfd) == PPC32_ELF_DATA)

/* Override the generic function because we store some extras.  */

static bfd_boolean
ppc_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct ppc_elf_obj_tdata),
				  PPC32_ELF_DATA);
}

/* Fix bad default arch selected for a 32 bit input bfd when the
   default is 64 bit.  */

static bfd_boolean
ppc_elf_object_p (bfd *abfd)
{
  if (abfd->arch_info->the_default && abfd->arch_info->bits_per_word == 64)
    {
      Elf_Internal_Ehdr *i_ehdr = elf_elfheader (abfd);

      if (i_ehdr->e_ident[EI_CLASS] == ELFCLASS32)
	{
	  /* Relies on arch after 64 bit default being 32 bit default.  */
	  abfd->arch_info = abfd->arch_info->next;
	  BFD_ASSERT (abfd->arch_info->bits_per_word == 32);
	}
    }
  return TRUE;
}

/* Function to set whether a module needs the -mrelocatable bit set.  */

static bfd_boolean
ppc_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = TRUE;
  return TRUE;
}

/* Support for core dump NOTE sections.  */

static bfd_boolean
ppc_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  unsigned int size;

  switch (note->descsz)
    {
    default:
      return FALSE;

    case 268:		/* Linux/PPC.  */
      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 72;
      size = 192;

      break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bfd_boolean
ppc_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return FALSE;

    case 128:		/* Linux/PPC elf_prpsinfo.  */
      elf_tdata (abfd)->core->pid
	= bfd_get_32 (abfd, note->descdata + 16);
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 32, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 48, 80);
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */

  {
    char *command = elf_tdata (abfd)->core->command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return TRUE;
}

char *
elfcore_write_ppc_linux_prpsinfo32 (bfd *abfd, char *buf, int *bufsiz,
				      const struct elf_internal_linux_prpsinfo *prpsinfo)
{
  struct elf_external_ppc_linux_prpsinfo32 data;

  memset (&data, 0, sizeof (data));
  PPC_LINUX_PRPSINFO32_SWAP_FIELDS (abfd, prpsinfo, data);

  return elfcore_write_note (abfd, buf, bufsiz,
			     "CORE", NT_PRPSINFO, &data, sizeof (data));
}

static char *
ppc_elf_write_core_note (bfd *abfd, char *buf, int *bufsiz, int note_type, ...)
{
  switch (note_type)
    {
    default:
      return NULL;

    case NT_PRPSINFO:
      {
	char data[128];
	va_list ap;

	va_start (ap, note_type);
	memset (data, 0, sizeof (data));
	strncpy (data + 32, va_arg (ap, const char *), 16);
	strncpy (data + 48, va_arg (ap, const char *), 80);
	va_end (ap);
	return elfcore_write_note (abfd, buf, bufsiz,
				   "CORE", note_type, data, sizeof (data));
      }

    case NT_PRSTATUS:
      {
	char data[268];
	va_list ap;
	long pid;
	int cursig;
	const void *greg;

	va_start (ap, note_type);
	memset (data, 0, 72);
	pid = va_arg (ap, long);
	bfd_put_32 (abfd, pid, data + 24);
	cursig = va_arg (ap, int);
	bfd_put_16 (abfd, cursig, data + 12);
	greg = va_arg (ap, const void *);
	memcpy (data + 72, greg, 192);
	memset (data + 264, 0, 4);
	va_end (ap);
	return elfcore_write_note (abfd, buf, bufsiz,
				   "CORE", note_type, data, sizeof (data));
      }
    }
}

static flagword
ppc_elf_lookup_section_flags (char *flag_name)
{

  if (!strcmp (flag_name, "SHF_PPC_VLE"))
    return SHF_PPC_VLE;

  return 0;
}

/* Add the VLE flag if required.  */

bfd_boolean
ppc_elf_section_processing (bfd *abfd, Elf_Internal_Shdr *shdr)
{
  if (bfd_get_mach (abfd) == bfd_mach_ppc_vle
      && (shdr->sh_flags & SHF_EXECINSTR) != 0)
    shdr->sh_flags |= SHF_PPC_VLE;

  return TRUE;
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
ppc_elf_plt_sym_val (bfd_vma i ATTRIBUTE_UNUSED,
		     const asection *plt ATTRIBUTE_UNUSED,
		     const arelent *rel)
{
  return rel->address;
}

/* Handle a PowerPC specific section when reading an object file.  This
   is called when bfd_section_from_shdr finds a section with an unknown
   type.  */

static bfd_boolean
ppc_elf_section_from_shdr (bfd *abfd,
			   Elf_Internal_Shdr *hdr,
			   const char *name,
			   int shindex)
{
  asection *newsect;
  flagword flags;

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return FALSE;

  newsect = hdr->bfd_section;
  flags = bfd_get_section_flags (abfd, newsect);
  if (hdr->sh_flags & SHF_EXCLUDE)
    flags |= SEC_EXCLUDE;

  if (hdr->sh_type == SHT_ORDERED)
    flags |= SEC_SORT_ENTRIES;

  bfd_set_section_flags (abfd, newsect, flags);
  return TRUE;
}

/* Set up any other section flags and such that may be necessary.  */

static bfd_boolean
ppc_elf_fake_sections (bfd *abfd ATTRIBUTE_UNUSED,
		       Elf_Internal_Shdr *shdr,
		       asection *asect)
{
  if ((asect->flags & SEC_SORT_ENTRIES) != 0)
    shdr->sh_type = SHT_ORDERED;

  return TRUE;
}

/* If we have .sbss2 or .PPC.EMB.sbss0 output sections, we
   need to bump up the number of section headers.  */

static int
ppc_elf_additional_program_headers (bfd *abfd,
				    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *s;
  int ret = 0;

  s = bfd_get_section_by_name (abfd, ".sbss2");
  if (s != NULL && (s->flags & SEC_ALLOC) != 0)
    ++ret;

  s = bfd_get_section_by_name (abfd, ".PPC.EMB.sbss0");
  if (s != NULL && (s->flags & SEC_ALLOC) != 0)
    ++ret;

  return ret;
}

/* Modify the segment map for VLE executables.  */

bfd_boolean
ppc_elf_modify_segment_map (bfd *abfd,
			    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  struct elf_segment_map *m, *n;
  bfd_size_type amt;
  unsigned int j, k;
  bfd_boolean sect0_vle, sectj_vle;

  /* At this point in the link, output sections have already been sorted by
     LMA and assigned to segments.  All that is left to do is to ensure
     there is no mixing of VLE & non-VLE sections in a text segment.
     If we find that case, we split the segment.
     We maintain the original output section order.  */

  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
    {
      if (m->count == 0)
	continue;

      sect0_vle = (elf_section_flags (m->sections[0]) & SHF_PPC_VLE) != 0;
      for (j = 1; j < m->count; ++j)
	{
	  sectj_vle = (elf_section_flags (m->sections[j]) & SHF_PPC_VLE) != 0;

	  if (sectj_vle != sect0_vle)
	    break;
        }
      if (j >= m->count)
	continue;

      /* sections 0..j-1 stay in this (current) segment,
	 the remainder are put in a new segment.
	 The scan resumes with the new segment.  */

      /* Fix the new segment.  */
      amt = sizeof (struct elf_segment_map);
      amt += (m->count - j - 1) * sizeof (asection *);
      n = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
      if (n == NULL)
        return FALSE;

      n->p_type = PT_LOAD;
      n->p_flags = PF_X | PF_R;
      if (sectj_vle)
        n->p_flags |= PF_PPC_VLE;
      n->count = m->count - j;
      for (k = 0; k < n->count; ++k)
        {
          n->sections[k] = m->sections[j+k];
          m->sections[j+k] = NULL;
	}
      n->next = m->next;
      m->next = n;

      /* Fix the current segment  */
      m->count = j;
    }

  return TRUE;
}

/* Add extra PPC sections -- Note, for now, make .sbss2 and
   .PPC.EMB.sbss0 a normal section, and not a bss section so
   that the linker doesn't crater when trying to make more than
   2 sections.  */

static const struct bfd_elf_special_section ppc_elf_special_sections[] =
{
  { STRING_COMMA_LEN (".plt"),             0, SHT_NOBITS,   SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".sbss"),           -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".sbss2"),          -2, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".sdata"),          -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".sdata2"),         -2, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".tags"),            0, SHT_ORDERED,  SHF_ALLOC },
  { STRING_COMMA_LEN (".PPC.EMB.apuinfo"), 0, SHT_NOTE,     0 },
  { STRING_COMMA_LEN (".PPC.EMB.sbss0"),   0, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".PPC.EMB.sdata0"),  0, SHT_PROGBITS, SHF_ALLOC },
  { NULL,                              0,  0, 0,            0 }
};

/* This is what we want for new plt/got.  */
static struct bfd_elf_special_section ppc_alt_plt =
  { STRING_COMMA_LEN (".plt"),             0, SHT_PROGBITS, SHF_ALLOC };

static const struct bfd_elf_special_section *
ppc_elf_get_sec_type_attr (bfd *abfd ATTRIBUTE_UNUSED, asection *sec)
{
  const struct bfd_elf_special_section *ssect;

  /* See if this is one of the special sections.  */
  if (sec->name == NULL)
    return NULL;

  ssect = _bfd_elf_get_special_section (sec->name, ppc_elf_special_sections,
					sec->use_rela_p);
  if (ssect != NULL)
    {
      if (ssect == ppc_elf_special_sections && (sec->flags & SEC_LOAD) != 0)
	ssect = &ppc_alt_plt;
      return ssect;
    }

  return _bfd_elf_get_sec_type_attr (abfd, sec);
}

/* Very simple linked list structure for recording apuinfo values.  */
typedef struct apuinfo_list
{
  struct apuinfo_list *next;
  unsigned long value;
}
apuinfo_list;

static apuinfo_list *head;
static bfd_boolean apuinfo_set;

static void
apuinfo_list_init (void)
{
  head = NULL;
  apuinfo_set = FALSE;
}

static void
apuinfo_list_add (unsigned long value)
{
  apuinfo_list *entry = head;

  while (entry != NULL)
    {
      if (entry->value == value)
	return;
      entry = entry->next;
    }

  entry = bfd_malloc (sizeof (* entry));
  if (entry == NULL)
    return;

  entry->value = value;
  entry->next  = head;
  head = entry;
}

static unsigned
apuinfo_list_length (void)
{
  apuinfo_list *entry;
  unsigned long count;

  for (entry = head, count = 0;
       entry;
       entry = entry->next)
    ++ count;

  return count;
}

static inline unsigned long
apuinfo_list_element (unsigned long number)
{
  apuinfo_list * entry;

  for (entry = head;
       entry && number --;
       entry = entry->next)
    ;

  return entry ? entry->value : 0;
}

static void
apuinfo_list_finish (void)
{
  apuinfo_list *entry;

  for (entry = head; entry;)
    {
      apuinfo_list *next = entry->next;
      free (entry);
      entry = next;
    }

  head = NULL;
}

#define APUINFO_SECTION_NAME	".PPC.EMB.apuinfo"
#define APUINFO_LABEL		"APUinfo"

/* Scan the input BFDs and create a linked list of
   the APUinfo values that will need to be emitted.  */

static void
ppc_elf_begin_write_processing (bfd *abfd, struct bfd_link_info *link_info)
{
  bfd *ibfd;
  asection *asec;
  char *buffer = NULL;
  bfd_size_type largest_input_size = 0;
  unsigned i;
  unsigned long length;
  const char *error_message = NULL;

  if (link_info == NULL)
    return;

  apuinfo_list_init ();

  /* Read in the input sections contents.  */
  for (ibfd = link_info->input_bfds; ibfd; ibfd = ibfd->link.next)
    {
      unsigned long datum;

      asec = bfd_get_section_by_name (ibfd, APUINFO_SECTION_NAME);
      if (asec == NULL)
	continue;

      error_message = _("corrupt %s section in %B");
      length = asec->size;
      if (length < 20)
	goto fail;

      apuinfo_set = TRUE;
      if (largest_input_size < asec->size)
	{
	  if (buffer)
	    free (buffer);
	  largest_input_size = asec->size;
	  buffer = bfd_malloc (largest_input_size);
	  if (!buffer)
	    return;
	}

      if (bfd_seek (ibfd, asec->filepos, SEEK_SET) != 0
	  || (bfd_bread (buffer, length, ibfd) != length))
	{
	  error_message = _("unable to read in %s section from %B");
	  goto fail;
	}

      /* Verify the contents of the header.  Note - we have to
	 extract the values this way in order to allow for a
	 host whose endian-ness is different from the target.  */
      datum = bfd_get_32 (ibfd, buffer);
      if (datum != sizeof APUINFO_LABEL)
	goto fail;

      datum = bfd_get_32 (ibfd, buffer + 8);
      if (datum != 0x2)
	goto fail;

      if (strcmp (buffer + 12, APUINFO_LABEL) != 0)
	goto fail;

      /* Get the number of bytes used for apuinfo entries.  */
      datum = bfd_get_32 (ibfd, buffer + 4);
      if (datum + 20 != length)
	goto fail;

      /* Scan the apuinfo section, building a list of apuinfo numbers.  */
      for (i = 0; i < datum; i += 4)
	apuinfo_list_add (bfd_get_32 (ibfd, buffer + 20 + i));
    }

  error_message = NULL;

  if (apuinfo_set)
    {
      /* Compute the size of the output section.  */
      unsigned num_entries = apuinfo_list_length ();

      /* Set the output section size, if it exists.  */
      asec = bfd_get_section_by_name (abfd, APUINFO_SECTION_NAME);

      if (asec && ! bfd_set_section_size (abfd, asec, 20 + num_entries * 4))
	{
	  ibfd = abfd;
	  error_message = _("warning: unable to set size of %s section in %B");
	}
    }

 fail:
  if (buffer)
    free (buffer);

  if (error_message)
    (*_bfd_error_handler) (error_message, ibfd, APUINFO_SECTION_NAME);
}

/* Prevent the output section from accumulating the input sections'
   contents.  We have already stored this in our linked list structure.  */

static bfd_boolean
ppc_elf_write_section (bfd *abfd ATTRIBUTE_UNUSED,
		       struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
		       asection *asec,
		       bfd_byte *contents ATTRIBUTE_UNUSED)
{
  return apuinfo_set && strcmp (asec->name, APUINFO_SECTION_NAME) == 0;
}

/* Finally we can generate the output section.  */

static void
ppc_elf_final_write_processing (bfd *abfd, bfd_boolean linker ATTRIBUTE_UNUSED)
{
  bfd_byte *buffer;
  asection *asec;
  unsigned i;
  unsigned num_entries;
  bfd_size_type length;

  asec = bfd_get_section_by_name (abfd, APUINFO_SECTION_NAME);
  if (asec == NULL)
    return;

  if (!apuinfo_set)
    return;

  length = asec->size;
  if (length < 20)
    return;

  buffer = bfd_malloc (length);
  if (buffer == NULL)
    {
      (*_bfd_error_handler)
	(_("failed to allocate space for new APUinfo section."));
      return;
    }

  /* Create the apuinfo header.  */
  num_entries = apuinfo_list_length ();
  bfd_put_32 (abfd, sizeof APUINFO_LABEL, buffer);
  bfd_put_32 (abfd, num_entries * 4, buffer + 4);
  bfd_put_32 (abfd, 0x2, buffer + 8);
  strcpy ((char *) buffer + 12, APUINFO_LABEL);

  length = 20;
  for (i = 0; i < num_entries; i++)
    {
      bfd_put_32 (abfd, apuinfo_list_element (i), buffer + length);
      length += 4;
    }

  if (length != asec->size)
    (*_bfd_error_handler) (_("failed to compute new APUinfo section."));

  if (! bfd_set_section_contents (abfd, asec, buffer, (file_ptr) 0, length))
    (*_bfd_error_handler) (_("failed to install new APUinfo section."));

  free (buffer);

  apuinfo_list_finish ();
}

static bfd_boolean
is_nonpic_glink_stub (bfd *abfd, asection *glink, bfd_vma off)
{
  bfd_byte buf[GLINK_ENTRY_SIZE];

  if (!bfd_get_section_contents (abfd, glink, buf, off, GLINK_ENTRY_SIZE))
    return FALSE;

  return ((bfd_get_32 (abfd, buf + 0) & 0xffff0000) == LIS_11
	  && (bfd_get_32 (abfd, buf + 4) & 0xffff0000) == LWZ_11_11
	  && bfd_get_32 (abfd, buf + 8) == MTCTR_11
	  && bfd_get_32 (abfd, buf + 12) == BCTR);
}

static bfd_boolean
section_covers_vma (bfd *abfd ATTRIBUTE_UNUSED, asection *section, void *ptr)
{
  bfd_vma vma = *(bfd_vma *) ptr;
  return ((section->flags & SEC_ALLOC) != 0
	  && section->vma <= vma
	  && vma < section->vma + section->size);
}

static long
ppc_elf_get_synthetic_symtab (bfd *abfd, long symcount, asymbol **syms,
			      long dynsymcount, asymbol **dynsyms,
			      asymbol **ret)
{
  bfd_boolean (*slurp_relocs) (bfd *, asection *, asymbol **, bfd_boolean);
  asection *plt, *relplt, *dynamic, *glink;
  bfd_vma glink_vma = 0;
  bfd_vma resolv_vma = 0;
  bfd_vma stub_vma;
  asymbol *s;
  arelent *p;
  long count, i;
  size_t size;
  char *names;
  bfd_byte buf[4];

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0)
    return 0;

  if (dynsymcount <= 0)
    return 0;

  relplt = bfd_get_section_by_name (abfd, ".rela.plt");
  if (relplt == NULL)
    return 0;

  plt = bfd_get_section_by_name (abfd, ".plt");
  if (plt == NULL)
    return 0;

  /* Call common code to handle old-style executable PLTs.  */
  if (elf_section_flags (plt) & SHF_EXECINSTR)
    return _bfd_elf_get_synthetic_symtab (abfd, symcount, syms,
					  dynsymcount, dynsyms, ret);

  /* If this object was prelinked, the prelinker stored the address
     of .glink at got[1].  If it wasn't prelinked, got[1] will be zero.  */
  dynamic = bfd_get_section_by_name (abfd, ".dynamic");
  if (dynamic != NULL)
    {
      bfd_byte *dynbuf, *extdyn, *extdynend;
      size_t extdynsize;
      void (*swap_dyn_in) (bfd *, const void *, Elf_Internal_Dyn *);

      if (!bfd_malloc_and_get_section (abfd, dynamic, &dynbuf))
	return -1;

      extdynsize = get_elf_backend_data (abfd)->s->sizeof_dyn;
      swap_dyn_in = get_elf_backend_data (abfd)->s->swap_dyn_in;

      extdyn = dynbuf;
      extdynend = extdyn + dynamic->size;
      for (; extdyn < extdynend; extdyn += extdynsize)
	{
	  Elf_Internal_Dyn dyn;
	  (*swap_dyn_in) (abfd, extdyn, &dyn);

	  if (dyn.d_tag == DT_NULL)
	    break;

	  if (dyn.d_tag == DT_PPC_GOT)
	    {
	      unsigned int g_o_t = dyn.d_un.d_val;
	      asection *got = bfd_get_section_by_name (abfd, ".got");
	      if (got != NULL
		  && bfd_get_section_contents (abfd, got, buf,
					       g_o_t - got->vma + 4, 4))
		glink_vma = bfd_get_32 (abfd, buf);
	      break;
	    }
	}
      free (dynbuf);
    }

  /* Otherwise we read the first plt entry.  */
  if (glink_vma == 0)
    {
      if (bfd_get_section_contents (abfd, plt, buf, 0, 4))
	glink_vma = bfd_get_32 (abfd, buf);
    }

  if (glink_vma == 0)
    return 0;

  /* The .glink section usually does not survive the final
     link; search for the section (usually .text) where the
     glink stubs now reside.  */
  glink = bfd_sections_find_if (abfd, section_covers_vma, &glink_vma);
  if (glink == NULL)
    return 0;

  /* Determine glink PLT resolver by reading the relative branch
     from the first glink stub.  */
  if (bfd_get_section_contents (abfd, glink, buf,
				glink_vma - glink->vma, 4))
    {
      unsigned int insn = bfd_get_32 (abfd, buf);

      /* The first glink stub may either branch to the resolver ...  */
      insn ^= B;
      if ((insn & ~0x3fffffc) == 0)
	resolv_vma = glink_vma + (insn ^ 0x2000000) - 0x2000000;

      /* ... or fall through a bunch of NOPs.  */
      else if ((insn ^ B ^ NOP) == 0)
	for (i = 4;
	     bfd_get_section_contents (abfd, glink, buf,
				       glink_vma - glink->vma + i, 4);
	     i += 4)
	  if (bfd_get_32 (abfd, buf) != NOP)
	    {
	      resolv_vma = glink_vma + i;
	      break;
	    }
    }

  count = relplt->size / sizeof (Elf32_External_Rela);
  /* If the stubs are those for -shared/-pie then we might have
     multiple stubs for each plt entry.  If that is the case then
     there is no way to associate stubs with their plt entries short
     of figuring out the GOT pointer value used in the stub.  */
  if (!is_nonpic_glink_stub (abfd, glink,
			     glink_vma - GLINK_ENTRY_SIZE - glink->vma))
    return 0;

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  if (! (*slurp_relocs) (abfd, relplt, dynsyms, TRUE))
    return -1;

  size = count * sizeof (asymbol);
  p = relplt->relocation;
  for (i = 0; i < count; i++, p++)
    {
      size += strlen ((*p->sym_ptr_ptr)->name) + sizeof ("@plt");
      if (p->addend != 0)
	size += sizeof ("+0x") - 1 + 8;
    }

  size += sizeof (asymbol) + sizeof ("__glink");

  if (resolv_vma)
    size += sizeof (asymbol) + sizeof ("__glink_PLTresolve");

  s = *ret = bfd_malloc (size);
  if (s == NULL)
    return -1;

  stub_vma = glink_vma;
  names = (char *) (s + count + 1 + (resolv_vma != 0));
  p = relplt->relocation + count - 1;
  for (i = 0; i < count; i++)
    {
      size_t len;

      *s = **p->sym_ptr_ptr;
      /* Undefined syms won't have BSF_LOCAL or BSF_GLOBAL set.  Since
	 we are defining a symbol, ensure one of them is set.  */
      if ((s->flags & BSF_LOCAL) == 0)
	s->flags |= BSF_GLOBAL;
      s->flags |= BSF_SYNTHETIC;
      s->section = glink;
      stub_vma -= 16;
      if (strcmp ((*p->sym_ptr_ptr)->name, "__tls_get_addr_opt") == 0)
	stub_vma -= 32;
      s->value = stub_vma - glink->vma;
      s->name = names;
      s->udata.p = NULL;
      len = strlen ((*p->sym_ptr_ptr)->name);
      memcpy (names, (*p->sym_ptr_ptr)->name, len);
      names += len;
      if (p->addend != 0)
	{
	  memcpy (names, "+0x", sizeof ("+0x") - 1);
	  names += sizeof ("+0x") - 1;
	  bfd_sprintf_vma (abfd, names, p->addend);
	  names += strlen (names);
	}
      memcpy (names, "@plt", sizeof ("@plt"));
      names += sizeof ("@plt");
      ++s;
      --p;
    }

  /* Add a symbol at the start of the glink branch table.  */
  memset (s, 0, sizeof *s);
  s->the_bfd = abfd;
  s->flags = BSF_GLOBAL | BSF_SYNTHETIC;
  s->section = glink;
  s->value = glink_vma - glink->vma;
  s->name = names;
  memcpy (names, "__glink", sizeof ("__glink"));
  names += sizeof ("__glink");
  s++;
  count++;

  if (resolv_vma)
    {
      /* Add a symbol for the glink PLT resolver.  */
      memset (s, 0, sizeof *s);
      s->the_bfd = abfd;
      s->flags = BSF_GLOBAL | BSF_SYNTHETIC;
      s->section = glink;
      s->value = resolv_vma - glink->vma;
      s->name = names;
      memcpy (names, "__glink_PLTresolve", sizeof ("__glink_PLTresolve"));
      names += sizeof ("__glink_PLTresolve");
      s++;
      count++;
    }

  return count;
}

/* The following functions are specific to the ELF linker, while
   functions above are used generally.  They appear in this file more
   or less in the order in which they are called.  eg.
   ppc_elf_check_relocs is called early in the link process,
   ppc_elf_finish_dynamic_sections is one of the last functions
   called.  */

/* Track PLT entries needed for a given symbol.  We might need more
   than one glink entry per symbol when generating a pic binary.  */
struct plt_entry
{
  struct plt_entry *next;

  /* -fPIC uses multiple GOT sections, one per file, called ".got2".
     This field stores the offset into .got2 used to initialise the
     GOT pointer reg.  It will always be at least 32768.  (Current
     gcc always uses an offset of 32768, but ld -r will pack .got2
     sections together resulting in larger offsets).  */
  bfd_vma addend;

  /* The .got2 section.  */
  asection *sec;

  /* PLT refcount or offset.  */
  union
    {
      bfd_signed_vma refcount;
      bfd_vma offset;
    } plt;

  /* .glink stub offset.  */
  bfd_vma glink_offset;
};

/* Of those relocs that might be copied as dynamic relocs, this function
   selects those that must be copied when linking a shared library,
   even when the symbol is local.  */

static int
must_be_dyn_reloc (struct bfd_link_info *info,
		   enum elf_ppc_reloc_type r_type)
{
  switch (r_type)
    {
    default:
      return 1;

    case R_PPC_REL24:
    case R_PPC_REL14:
    case R_PPC_REL14_BRTAKEN:
    case R_PPC_REL14_BRNTAKEN:
    case R_PPC_REL32:
      return 0;

    case R_PPC_TPREL32:
    case R_PPC_TPREL16:
    case R_PPC_TPREL16_LO:
    case R_PPC_TPREL16_HI:
    case R_PPC_TPREL16_HA:
      return !info->executable;
    }
}

/* If ELIMINATE_COPY_RELOCS is non-zero, the linker will try to avoid
   copying dynamic variables from a shared lib into an app's dynbss
   section, and instead use a dynamic relocation to point into the
   shared lib.  */
#define ELIMINATE_COPY_RELOCS 1

/* Used to track dynamic relocations for local symbols.  */
struct ppc_dyn_relocs
{
  struct ppc_dyn_relocs *next;

  /* The input section of the reloc.  */
  asection *sec;

  /* Total number of relocs copied for the input section.  */
  unsigned int count : 31;

  /* Whether this entry is for STT_GNU_IFUNC symbols.  */
  unsigned int ifunc : 1;
};

/* PPC ELF linker hash entry.  */

struct ppc_elf_link_hash_entry
{
  struct elf_link_hash_entry elf;

  /* If this symbol is used in the linker created sections, the processor
     specific backend uses this field to map the field into the offset
     from the beginning of the section.  */
  elf_linker_section_pointers_t *linker_section_pointer;

  /* Track dynamic relocs copied for this symbol.  */
  struct elf_dyn_relocs *dyn_relocs;

  /* Contexts in which symbol is used in the GOT (or TOC).
     TLS_GD .. TLS_TLS bits are or'd into the mask as the
     corresponding relocs are encountered during check_relocs.
     tls_optimize clears TLS_GD .. TLS_TPREL when optimizing to
     indicate the corresponding GOT entry type is not needed.  */
#define TLS_GD		 1	/* GD reloc. */
#define TLS_LD		 2	/* LD reloc. */
#define TLS_TPREL	 4	/* TPREL reloc, => IE. */
#define TLS_DTPREL	 8	/* DTPREL reloc, => LD. */
#define TLS_TLS		16	/* Any TLS reloc.  */
#define TLS_TPRELGD	32	/* TPREL reloc resulting from GD->IE. */
#define PLT_IFUNC	64	/* STT_GNU_IFUNC.  */
  char tls_mask;

  /* Nonzero if we have seen a small data relocation referring to this
     symbol.  */
  unsigned char has_sda_refs;
};

#define ppc_elf_hash_entry(ent) ((struct ppc_elf_link_hash_entry *) (ent))

/* PPC ELF linker hash table.  */

struct ppc_elf_link_hash_table
{
  struct elf_link_hash_table elf;

  /* Various options passed from the linker.  */
  struct ppc_elf_params *params;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *got;
  asection *relgot;
  asection *glink;
  asection *plt;
  asection *relplt;
  asection *iplt;
  asection *reliplt;
  asection *dynbss;
  asection *relbss;
  asection *dynsbss;
  asection *relsbss;
  elf_linker_section_t sdata[2];
  asection *sbss;
  asection *glink_eh_frame;

  /* The (unloaded but important) .rela.plt.unloaded on VxWorks.  */
  asection *srelplt2;

  /* The .got.plt section (VxWorks only)*/
  asection *sgotplt;

  /* Shortcut to __tls_get_addr.  */
  struct elf_link_hash_entry *tls_get_addr;

  /* The bfd that forced an old-style PLT.  */
  bfd *old_bfd;

  /* TLS local dynamic got entry handling.  */
  union {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tlsld_got;

  /* Offset of branch table to PltResolve function in glink.  */
  bfd_vma glink_pltresolve;

  /* Size of reserved GOT entries.  */
  unsigned int got_header_size;
  /* Non-zero if allocating the header left a gap.  */
  unsigned int got_gap;

  /* The type of PLT we have chosen to use.  */
  enum ppc_elf_plt_type plt_type;

  /* True if the target system is VxWorks.  */
  unsigned int is_vxworks:1;

  /* The size of PLT entries.  */
  int plt_entry_size;
  /* The distance between adjacent PLT slots.  */
  int plt_slot_size;
  /* The size of the first PLT entry.  */
  int plt_initial_entry_size;

  /* Small local sym cache.  */
  struct sym_cache sym_cache;
};

/* Rename some of the generic section flags to better document how they
   are used for ppc32.  The flags are only valid for ppc32 elf objects.  */

/* Nonzero if this section has TLS related relocations.  */
#define has_tls_reloc sec_flg0

/* Nonzero if this section has a call to __tls_get_addr.  */
#define has_tls_get_addr_call sec_flg1

/* Get the PPC ELF linker hash table from a link_info structure.  */

#define ppc_elf_hash_table(p) \
  (elf_hash_table_id ((struct elf_link_hash_table *) ((p)->hash)) \
  == PPC32_ELF_DATA ? ((struct ppc_elf_link_hash_table *) ((p)->hash)) : NULL)

/* Create an entry in a PPC ELF linker hash table.  */

static struct bfd_hash_entry *
ppc_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
			   struct bfd_hash_table *table,
			   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct ppc_elf_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      ppc_elf_hash_entry (entry)->linker_section_pointer = NULL;
      ppc_elf_hash_entry (entry)->dyn_relocs = NULL;
      ppc_elf_hash_entry (entry)->tls_mask = 0;
      ppc_elf_hash_entry (entry)->has_sda_refs = 0;
    }

  return entry;
}

/* Create a PPC ELF linker hash table.  */

static struct bfd_link_hash_table *
ppc_elf_link_hash_table_create (bfd *abfd)
{
  struct ppc_elf_link_hash_table *ret;
  static struct ppc_elf_params default_params = { PLT_OLD, 0, 1, 0, 0, 12 };

  ret = bfd_zmalloc (sizeof (struct ppc_elf_link_hash_table));
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd,
				      ppc_elf_link_hash_newfunc,
				      sizeof (struct ppc_elf_link_hash_entry),
				      PPC32_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->elf.init_plt_refcount.refcount = 0;
  ret->elf.init_plt_refcount.glist = NULL;
  ret->elf.init_plt_offset.offset = 0;
  ret->elf.init_plt_offset.glist = NULL;

  ret->params = &default_params;

  ret->sdata[0].name = ".sdata";
  ret->sdata[0].sym_name = "_SDA_BASE_";
  ret->sdata[0].bss_name = ".sbss";

  ret->sdata[1].name = ".sdata2";
  ret->sdata[1].sym_name = "_SDA2_BASE_";
  ret->sdata[1].bss_name = ".sbss2";

  ret->plt_entry_size = 12;
  ret->plt_slot_size = 8;
  ret->plt_initial_entry_size = 72;

  return &ret->elf.root;
}

/* Hook linker params into hash table.  */

void
ppc_elf_link_params (struct bfd_link_info *info, struct ppc_elf_params *params)
{
  struct ppc_elf_link_hash_table *htab = ppc_elf_hash_table (info);

  if (htab)
    htab->params = params;
}

/* Create .got and the related sections.  */

static bfd_boolean
ppc_elf_create_got (bfd *abfd, struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab;
  asection *s;
  flagword flags;

  if (!_bfd_elf_create_got_section (abfd, info))
    return FALSE;

  htab = ppc_elf_hash_table (info);
  htab->got = s = bfd_get_linker_section (abfd, ".got");
  if (s == NULL)
    abort ();

  if (htab->is_vxworks)
    {
      htab->sgotplt = bfd_get_linker_section (abfd, ".got.plt");
      if (!htab->sgotplt)
	abort ();
    }
  else
    {
      /* The powerpc .got has a blrl instruction in it.  Mark it
	 executable.  */
      flags = (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS
	       | SEC_IN_MEMORY | SEC_LINKER_CREATED);
      if (!bfd_set_section_flags (abfd, s, flags))
	return FALSE;
    }

  htab->relgot = bfd_get_linker_section (abfd, ".rela.got");
  if (!htab->relgot)
    abort ();

  return TRUE;
}

/* Create a special linker section, used for R_PPC_EMB_SDAI16 and
   R_PPC_EMB_SDA2I16 pointers.  These sections become part of .sdata
   and .sdata2.  Create _SDA_BASE_ and _SDA2_BASE too.  */

static bfd_boolean
ppc_elf_create_linker_section (bfd *abfd,
			       struct bfd_link_info *info,
			       flagword flags,
			       elf_linker_section_t *lsect)
{
  asection *s;

  flags |= (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	    | SEC_LINKER_CREATED);

  s = bfd_make_section_anyway_with_flags (abfd, lsect->name, flags);
  if (s == NULL)
    return FALSE;
  lsect->section = s;

  /* Define the sym on the first section of this name.  */
  s = bfd_get_section_by_name (abfd, lsect->name);

  lsect->sym = _bfd_elf_define_linkage_sym (abfd, info, s, lsect->sym_name);
  if (lsect->sym == NULL)
    return FALSE;
  lsect->sym->root.u.def.value = 0x8000;
  return TRUE;
}

static bfd_boolean
ppc_elf_create_glink (bfd *abfd, struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab = ppc_elf_hash_table (info);
  asection *s;
  flagword flags;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_READONLY | SEC_HAS_CONTENTS
	   | SEC_IN_MEMORY | SEC_LINKER_CREATED);
  s = bfd_make_section_anyway_with_flags (abfd, ".glink", flags);
  htab->glink = s;
  if (s == NULL
      || !bfd_set_section_alignment (abfd, s,
				     htab->params->ppc476_workaround ? 6 : 4))
    return FALSE;

  if (!info->no_ld_generated_unwind_info)
    {
      flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_HAS_CONTENTS
	       | SEC_IN_MEMORY | SEC_LINKER_CREATED);
      s = bfd_make_section_anyway_with_flags (abfd, ".eh_frame", flags);
      htab->glink_eh_frame = s;
      if (s == NULL
	  || !bfd_set_section_alignment (abfd, s, 2))
	return FALSE;
    }

  flags = SEC_ALLOC | SEC_LINKER_CREATED;
  s = bfd_make_section_anyway_with_flags (abfd, ".iplt", flags);
  htab->iplt = s;
  if (s == NULL
      || !bfd_set_section_alignment (abfd, s, 4))
    return FALSE;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_HAS_CONTENTS
	   | SEC_IN_MEMORY | SEC_LINKER_CREATED);
  s = bfd_make_section_anyway_with_flags (abfd, ".rela.iplt", flags);
  htab->reliplt = s;
  if (s == NULL
      || ! bfd_set_section_alignment (abfd, s, 2))
    return FALSE;

  if (!ppc_elf_create_linker_section (abfd, info, 0,
				      &htab->sdata[0]))
    return FALSE;

  if (!ppc_elf_create_linker_section (abfd, info, SEC_READONLY,
				      &htab->sdata[1]))
    return FALSE;

  return TRUE;
}

/* We have to create .dynsbss and .rela.sbss here so that they get mapped
   to output sections (just like _bfd_elf_create_dynamic_sections has
   to create .dynbss and .rela.bss).  */

static bfd_boolean
ppc_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab;
  asection *s;
  flagword flags;

  htab = ppc_elf_hash_table (info);

  if (htab->got == NULL
      && !ppc_elf_create_got (abfd, info))
    return FALSE;

  if (!_bfd_elf_create_dynamic_sections (abfd, info))
    return FALSE;

  if (htab->glink == NULL
      && !ppc_elf_create_glink (abfd, info))
    return FALSE;

  htab->dynbss = bfd_get_linker_section (abfd, ".dynbss");
  s = bfd_make_section_anyway_with_flags (abfd, ".dynsbss",
					  SEC_ALLOC | SEC_LINKER_CREATED);
  htab->dynsbss = s;
  if (s == NULL)
    return FALSE;

  if (! info->shared)
    {
      htab->relbss = bfd_get_linker_section (abfd, ".rela.bss");
      flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_HAS_CONTENTS
	       | SEC_IN_MEMORY | SEC_LINKER_CREATED);
      s = bfd_make_section_anyway_with_flags (abfd, ".rela.sbss", flags);
      htab->relsbss = s;
      if (s == NULL
	  || ! bfd_set_section_alignment (abfd, s, 2))
	return FALSE;
    }

  if (htab->is_vxworks
      && !elf_vxworks_create_dynamic_sections (abfd, info, &htab->srelplt2))
    return FALSE;

  htab->relplt = bfd_get_linker_section (abfd, ".rela.plt");
  htab->plt = s = bfd_get_linker_section (abfd, ".plt");
  if (s == NULL)
    abort ();

  flags = SEC_ALLOC | SEC_CODE | SEC_LINKER_CREATED;
  if (htab->plt_type == PLT_VXWORKS)
    /* The VxWorks PLT is a loaded section with contents.  */
    flags |= SEC_HAS_CONTENTS | SEC_LOAD | SEC_READONLY;
  return bfd_set_section_flags (abfd, s, flags);
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
ppc_elf_copy_indirect_symbol (struct bfd_link_info *info,
			      struct elf_link_hash_entry *dir,
			      struct elf_link_hash_entry *ind)
{
  struct ppc_elf_link_hash_entry *edir, *eind;

  edir = (struct ppc_elf_link_hash_entry *) dir;
  eind = (struct ppc_elf_link_hash_entry *) ind;

  edir->tls_mask |= eind->tls_mask;
  edir->has_sda_refs |= eind->has_sda_refs;

  /* If called to transfer flags for a weakdef during processing
     of elf_adjust_dynamic_symbol, don't copy non_got_ref.
     We clear it ourselves for ELIMINATE_COPY_RELOCS.  */
  if (!(ELIMINATE_COPY_RELOCS
	&& eind->elf.root.type != bfd_link_hash_indirect
	&& edir->elf.dynamic_adjusted))
    edir->elf.non_got_ref |= eind->elf.non_got_ref;

  edir->elf.ref_dynamic |= eind->elf.ref_dynamic;
  edir->elf.ref_regular |= eind->elf.ref_regular;
  edir->elf.ref_regular_nonweak |= eind->elf.ref_regular_nonweak;
  edir->elf.needs_plt |= eind->elf.needs_plt;
  edir->elf.pointer_equality_needed |= eind->elf.pointer_equality_needed;

  if (eind->dyn_relocs != NULL)
    {
      if (edir->dyn_relocs != NULL)
	{
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  /* Add reloc counts against the indirect sym to the direct sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &eind->dyn_relocs; (p = *pp) != NULL; )
	    {
	      struct elf_dyn_relocs *q;

	      for (q = edir->dyn_relocs; q != NULL; q = q->next)
		if (q->sec == p->sec)
		  {
		    q->pc_count += p->pc_count;
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = edir->dyn_relocs;
	}

      edir->dyn_relocs = eind->dyn_relocs;
      eind->dyn_relocs = NULL;
    }

  /* If we were called to copy over info for a weak sym, that's all.
     You might think dyn_relocs need not be copied over;  After all,
     both syms will be dynamic or both non-dynamic so we're just
     moving reloc accounting around.  However, ELIMINATE_COPY_RELOCS
     code in ppc_elf_adjust_dynamic_symbol needs to check for
     dyn_relocs in read-only sections, and it does so on what is the
     DIR sym here.  */
  if (eind->elf.root.type != bfd_link_hash_indirect)
    return;

  /* Copy over the GOT refcount entries that we may have already seen to
     the symbol which just became indirect.  */
  edir->elf.got.refcount += eind->elf.got.refcount;
  eind->elf.got.refcount = 0;

  /* And plt entries.  */
  if (eind->elf.plt.plist != NULL)
    {
      if (edir->elf.plt.plist != NULL)
	{
	  struct plt_entry **entp;
	  struct plt_entry *ent;

	  for (entp = &eind->elf.plt.plist; (ent = *entp) != NULL; )
	    {
	      struct plt_entry *dent;

	      for (dent = edir->elf.plt.plist; dent != NULL; dent = dent->next)
		if (dent->sec == ent->sec && dent->addend == ent->addend)
		  {
		    dent->plt.refcount += ent->plt.refcount;
		    *entp = ent->next;
		    break;
		  }
	      if (dent == NULL)
		entp = &ent->next;
	    }
	  *entp = edir->elf.plt.plist;
	}

      edir->elf.plt.plist = eind->elf.plt.plist;
      eind->elf.plt.plist = NULL;
    }

  if (eind->elf.dynindx != -1)
    {
      if (edir->elf.dynindx != -1)
	_bfd_elf_strtab_delref (elf_hash_table (info)->dynstr,
				edir->elf.dynstr_index);
      edir->elf.dynindx = eind->elf.dynindx;
      edir->elf.dynstr_index = eind->elf.dynstr_index;
      eind->elf.dynindx = -1;
      eind->elf.dynstr_index = 0;
    }
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */

static bfd_boolean
ppc_elf_add_symbol_hook (bfd *abfd,
			 struct bfd_link_info *info,
			 Elf_Internal_Sym *sym,
			 const char **namep ATTRIBUTE_UNUSED,
			 flagword *flagsp ATTRIBUTE_UNUSED,
			 asection **secp,
			 bfd_vma *valp)
{
  if (sym->st_shndx == SHN_COMMON
      && !info->relocatable
      && is_ppc_elf (info->output_bfd)
      && sym->st_size <= elf_gp_size (abfd))
    {
      /* Common symbols less than or equal to -G nn bytes are automatically
	 put into .sbss.  */
      struct ppc_elf_link_hash_table *htab;

      htab = ppc_elf_hash_table (info);
      if (htab->sbss == NULL)
	{
	  flagword flags = SEC_IS_COMMON | SEC_LINKER_CREATED;

	  if (!htab->elf.dynobj)
	    htab->elf.dynobj = abfd;

	  htab->sbss = bfd_make_section_anyway_with_flags (htab->elf.dynobj,
							   ".sbss",
							   flags);
	  if (htab->sbss == NULL)
	    return FALSE;
	}

      *secp = htab->sbss;
      *valp = sym->st_size;
    }

  if ((ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC
       || ELF_ST_BIND (sym->st_info) == STB_GNU_UNIQUE)
      && (abfd->flags & DYNAMIC) == 0
      && bfd_get_flavour (info->output_bfd) == bfd_target_elf_flavour)
    elf_tdata (info->output_bfd)->has_gnu_symbols = TRUE;

  return TRUE;
}

/* Find a linker generated pointer with a given addend and type.  */

static elf_linker_section_pointers_t *
elf_find_pointer_linker_section
  (elf_linker_section_pointers_t *linker_pointers,
   bfd_vma addend,
   elf_linker_section_t *lsect)
{
  for ( ; linker_pointers != NULL; linker_pointers = linker_pointers->next)
    if (lsect == linker_pointers->lsect && addend == linker_pointers->addend)
      return linker_pointers;

  return NULL;
}

/* Allocate a pointer to live in a linker created section.  */

static bfd_boolean
elf_allocate_pointer_linker_section (bfd *abfd,
				     elf_linker_section_t *lsect,
				     struct elf_link_hash_entry *h,
				     const Elf_Internal_Rela *rel)
{
  elf_linker_section_pointers_t **ptr_linker_section_ptr = NULL;
  elf_linker_section_pointers_t *linker_section_ptr;
  unsigned long r_symndx = ELF32_R_SYM (rel->r_info);
  bfd_size_type amt;

  BFD_ASSERT (lsect != NULL);

  /* Is this a global symbol?  */
  if (h != NULL)
    {
      struct ppc_elf_link_hash_entry *eh;

      /* Has this symbol already been allocated?  If so, our work is done.  */
      eh = (struct ppc_elf_link_hash_entry *) h;
      if (elf_find_pointer_linker_section (eh->linker_section_pointer,
					   rel->r_addend,
					   lsect))
	return TRUE;

      ptr_linker_section_ptr = &eh->linker_section_pointer;
    }
  else
    {
      BFD_ASSERT (is_ppc_elf (abfd));

      /* Allocation of a pointer to a local symbol.  */
      elf_linker_section_pointers_t **ptr = elf_local_ptr_offsets (abfd);

      /* Allocate a table to hold the local symbols if first time.  */
      if (!ptr)
	{
	  unsigned int num_symbols = elf_symtab_hdr (abfd).sh_info;

	  amt = num_symbols;
	  amt *= sizeof (elf_linker_section_pointers_t *);
	  ptr = bfd_zalloc (abfd, amt);

	  if (!ptr)
	    return FALSE;

	  elf_local_ptr_offsets (abfd) = ptr;
	}

      /* Has this symbol already been allocated?  If so, our work is done.  */
      if (elf_find_pointer_linker_section (ptr[r_symndx],
					   rel->r_addend,
					   lsect))
	return TRUE;

      ptr_linker_section_ptr = &ptr[r_symndx];
    }

  /* Allocate space for a pointer in the linker section, and allocate
     a new pointer record from internal memory.  */
  BFD_ASSERT (ptr_linker_section_ptr != NULL);
  amt = sizeof (elf_linker_section_pointers_t);
  linker_section_ptr = bfd_alloc (abfd, amt);

  if (!linker_section_ptr)
    return FALSE;

  linker_section_ptr->next = *ptr_linker_section_ptr;
  linker_section_ptr->addend = rel->r_addend;
  linker_section_ptr->lsect = lsect;
  *ptr_linker_section_ptr = linker_section_ptr;

  if (!bfd_set_section_alignment (lsect->section->owner, lsect->section, 2))
    return FALSE;
  linker_section_ptr->offset = lsect->section->size;
  lsect->section->size += 4;

#ifdef DEBUG
  fprintf (stderr,
	   "Create pointer in linker section %s, offset = %ld, section size = %ld\n",
	   lsect->name, (long) linker_section_ptr->offset,
	   (long) lsect->section->size);
#endif

  return TRUE;
}

static struct plt_entry **
update_local_sym_info (bfd *abfd,
		       Elf_Internal_Shdr *symtab_hdr,
		       unsigned long r_symndx,
		       int tls_type)
{
  bfd_signed_vma *local_got_refcounts = elf_local_got_refcounts (abfd);
  struct plt_entry **local_plt;
  char *local_got_tls_masks;

  if (local_got_refcounts == NULL)
    {
      bfd_size_type size = symtab_hdr->sh_info;

      size *= (sizeof (*local_got_refcounts)
	       + sizeof (*local_plt)
	       + sizeof (*local_got_tls_masks));
      local_got_refcounts = bfd_zalloc (abfd, size);
      if (local_got_refcounts == NULL)
	return NULL;
      elf_local_got_refcounts (abfd) = local_got_refcounts;
    }

  local_plt = (struct plt_entry **) (local_got_refcounts + symtab_hdr->sh_info);
  local_got_tls_masks = (char *) (local_plt + symtab_hdr->sh_info);
  local_got_tls_masks[r_symndx] |= tls_type;
  if (tls_type != PLT_IFUNC)
    local_got_refcounts[r_symndx] += 1;
  return local_plt + r_symndx;
}

static bfd_boolean
update_plt_info (bfd *abfd, struct plt_entry **plist,
		 asection *sec, bfd_vma addend)
{
  struct plt_entry *ent;

  if (addend < 32768)
    sec = NULL;
  for (ent = *plist; ent != NULL; ent = ent->next)
    if (ent->sec == sec && ent->addend == addend)
      break;
  if (ent == NULL)
    {
      bfd_size_type amt = sizeof (*ent);
      ent = bfd_alloc (abfd, amt);
      if (ent == NULL)
	return FALSE;
      ent->next = *plist;
      ent->sec = sec;
      ent->addend = addend;
      ent->plt.refcount = 0;
      *plist = ent;
    }
  ent->plt.refcount += 1;
  return TRUE;
}

static struct plt_entry *
find_plt_ent (struct plt_entry **plist, asection *sec, bfd_vma addend)
{
  struct plt_entry *ent;

  if (addend < 32768)
    sec = NULL;
  for (ent = *plist; ent != NULL; ent = ent->next)
    if (ent->sec == sec && ent->addend == addend)
      break;
  return ent;
}

static bfd_boolean
is_branch_reloc (enum elf_ppc_reloc_type r_type)
{
  return (r_type == R_PPC_PLTREL24
	  || r_type == R_PPC_LOCAL24PC
	  || r_type == R_PPC_REL24
	  || r_type == R_PPC_REL14
	  || r_type == R_PPC_REL14_BRTAKEN
	  || r_type == R_PPC_REL14_BRNTAKEN
	  || r_type == R_PPC_ADDR24
	  || r_type == R_PPC_ADDR14
	  || r_type == R_PPC_ADDR14_BRTAKEN
	  || r_type == R_PPC_ADDR14_BRNTAKEN);
}

static void
bad_shared_reloc (bfd *abfd, enum elf_ppc_reloc_type r_type)
{
  (*_bfd_error_handler)
    (_("%B: relocation %s cannot be used when making a shared object"),
     abfd,
     ppc_elf_howto_table[r_type]->name);
  bfd_set_error (bfd_error_bad_value);
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

static bfd_boolean
ppc_elf_check_relocs (bfd *abfd,
		      struct bfd_link_info *info,
		      asection *sec,
		      const Elf_Internal_Rela *relocs)
{
  struct ppc_elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *got2, *sreloc;
  struct elf_link_hash_entry *tga;

  if (info->relocatable)
    return TRUE;

  /* Don't do anything special with non-loaded, non-alloced sections.
     In particular, any relocs in such sections should not affect GOT
     and PLT reference counting (ie. we don't allow them to create GOT
     or PLT entries), there's no possibility or desire to optimize TLS
     relocs, and there's not much point in propagating relocs to shared
     libs that the dynamic linker won't relocate.  */
  if ((sec->flags & SEC_ALLOC) == 0)
    return TRUE;

#ifdef DEBUG
  _bfd_error_handler ("ppc_elf_check_relocs called for section %A in %B",
		      sec, abfd);
#endif

  BFD_ASSERT (is_ppc_elf (abfd));

  /* Initialize howto table if not already done.  */
  if (!ppc_elf_howto_table[R_PPC_ADDR32])
    ppc_elf_howto_init ();

  htab = ppc_elf_hash_table (info);
  if (htab->glink == NULL)
    {
      if (htab->elf.dynobj == NULL)
	htab->elf.dynobj = abfd;
      if (!ppc_elf_create_glink (htab->elf.dynobj, info))
	return FALSE;
    }
  tga = elf_link_hash_lookup (&htab->elf, "__tls_get_addr",
			      FALSE, FALSE, TRUE);
  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);
  got2 = bfd_get_section_by_name (abfd, ".got2");
  sreloc = NULL;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned long r_symndx;
      enum elf_ppc_reloc_type r_type;
      struct elf_link_hash_entry *h;
      int tls_type;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  /* PR15323, ref flags aren't set for references in the same
	     object.  */
	  h->root.non_ir_ref = 1;
	}

      /* If a relocation refers to _GLOBAL_OFFSET_TABLE_, create the .got.
	 This shows up in particular in an R_PPC_ADDR32 in the eabi
	 startup code.  */
      if (h != NULL
	  && htab->got == NULL
	  && strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
	{
	  if (htab->elf.dynobj == NULL)
	    htab->elf.dynobj = abfd;
	  if (!ppc_elf_create_got (htab->elf.dynobj, info))
	    return FALSE;
	  BFD_ASSERT (h == htab->elf.hgot);
	}

      tls_type = 0;
      r_type = ELF32_R_TYPE (rel->r_info);
      if (h == NULL && !htab->is_vxworks)
	{
	  Elf_Internal_Sym *isym = bfd_sym_from_r_symndx (&htab->sym_cache,
							  abfd, r_symndx);
	  if (isym == NULL)
	    return FALSE;

	  if (ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      struct plt_entry **ifunc;

	      /* Set PLT_IFUNC flag for this sym, no GOT entry yet.  */
	      ifunc = update_local_sym_info (abfd, symtab_hdr, r_symndx,
					     PLT_IFUNC);
	      if (ifunc == NULL)
		return FALSE;

	      /* STT_GNU_IFUNC symbols must have a PLT entry;
		 In a non-pie executable even when there are
		 no plt calls.  */
	      if (!info->shared
		  || is_branch_reloc (r_type))
		{
		  bfd_vma addend = 0;
		  if (r_type == R_PPC_PLTREL24)
		    {
		      ppc_elf_tdata (abfd)->makes_plt_call = 1;
		      if (info->shared)
			addend = rel->r_addend;
		    }
		  if (!update_plt_info (abfd, ifunc, got2, addend))
		    return FALSE;
		}
	    }
	}

      if (!htab->is_vxworks
	  && is_branch_reloc (r_type)
	  && h != NULL
	  && h == tga)
	{
	  if (rel != relocs
	      && (ELF32_R_TYPE (rel[-1].r_info) == R_PPC_TLSGD
		  || ELF32_R_TYPE (rel[-1].r_info) == R_PPC_TLSLD))
	    /* We have a new-style __tls_get_addr call with a marker
	       reloc.  */
	    ;
	  else
	    /* Mark this section as having an old-style call.  */
	    sec->has_tls_get_addr_call = 1;
	}

      switch (r_type)
	{
	case R_PPC_TLSGD:
	case R_PPC_TLSLD:
	  /* These special tls relocs tie a call to __tls_get_addr with
	     its parameter symbol.  */
	  break;

	case R_PPC_GOT_TLSLD16:
	case R_PPC_GOT_TLSLD16_LO:
	case R_PPC_GOT_TLSLD16_HI:
	case R_PPC_GOT_TLSLD16_HA:
	  tls_type = TLS_TLS | TLS_LD;
	  goto dogottls;

	case R_PPC_GOT_TLSGD16:
	case R_PPC_GOT_TLSGD16_LO:
	case R_PPC_GOT_TLSGD16_HI:
	case R_PPC_GOT_TLSGD16_HA:
	  tls_type = TLS_TLS | TLS_GD;
	  goto dogottls;

	case R_PPC_GOT_TPREL16:
	case R_PPC_GOT_TPREL16_LO:
	case R_PPC_GOT_TPREL16_HI:
	case R_PPC_GOT_TPREL16_HA:
	  if (info->shared)
	    info->flags |= DF_STATIC_TLS;
	  tls_type = TLS_TLS | TLS_TPREL;
	  goto dogottls;

	case R_PPC_GOT_DTPREL16:
	case R_PPC_GOT_DTPREL16_LO:
	case R_PPC_GOT_DTPREL16_HI:
	case R_PPC_GOT_DTPREL16_HA:
	  tls_type = TLS_TLS | TLS_DTPREL;
	dogottls:
	  sec->has_tls_reloc = 1;
	  /* Fall thru */

	  /* GOT16 relocations */
	case R_PPC_GOT16:
	case R_PPC_GOT16_LO:
	case R_PPC_GOT16_HI:
	case R_PPC_GOT16_HA:
	  /* This symbol requires a global offset table entry.  */
	  if (htab->got == NULL)
	    {
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;
	      if (!ppc_elf_create_got (htab->elf.dynobj, info))
		return FALSE;
	    }
	  if (h != NULL)
	    {
	      h->got.refcount += 1;
	      ppc_elf_hash_entry (h)->tls_mask |= tls_type;
	    }
	  else
	    /* This is a global offset table entry for a local symbol.  */
	    if (!update_local_sym_info (abfd, symtab_hdr, r_symndx, tls_type))
	      return FALSE;

	  /* We may also need a plt entry if the symbol turns out to be
	     an ifunc.  */
	  if (h != NULL && !info->shared)
	    {
	      if (!update_plt_info (abfd, &h->plt.plist, NULL, 0))
		return FALSE;
	    }
	  break;

	  /* Indirect .sdata relocation.  */
	case R_PPC_EMB_SDAI16:
	  if (info->shared)
	    {
	      bad_shared_reloc (abfd, r_type);
	      return FALSE;
	    }
	  htab->sdata[0].sym->ref_regular = 1;
	  if (!elf_allocate_pointer_linker_section (abfd, &htab->sdata[0],
						    h, rel))
	    return FALSE;
	  if (h != NULL)
	    {
	      ppc_elf_hash_entry (h)->has_sda_refs = TRUE;
	      h->non_got_ref = TRUE;
	    }
	  break;

	  /* Indirect .sdata2 relocation.  */
	case R_PPC_EMB_SDA2I16:
	  if (info->shared)
	    {
	      bad_shared_reloc (abfd, r_type);
	      return FALSE;
	    }
	  htab->sdata[1].sym->ref_regular = 1;
	  if (!elf_allocate_pointer_linker_section (abfd, &htab->sdata[1],
						    h, rel))
	    return FALSE;
	  if (h != NULL)
	    {
	      ppc_elf_hash_entry (h)->has_sda_refs = TRUE;
	      h->non_got_ref = TRUE;
	    }
	  break;

	case R_PPC_SDAREL16:
	  htab->sdata[0].sym->ref_regular = 1;
	  /* Fall thru */

	case R_PPC_VLE_SDAREL_LO16A:
	case R_PPC_VLE_SDAREL_LO16D:
	case R_PPC_VLE_SDAREL_HI16A:
	case R_PPC_VLE_SDAREL_HI16D:
	case R_PPC_VLE_SDAREL_HA16A:
	case R_PPC_VLE_SDAREL_HA16D:
	  if (h != NULL)
	    {
	      ppc_elf_hash_entry (h)->has_sda_refs = TRUE;
	      h->non_got_ref = TRUE;
	    }
	  break;

	case R_PPC_VLE_REL8:
	case R_PPC_VLE_REL15:
	case R_PPC_VLE_REL24:
	case R_PPC_VLE_LO16A:
	case R_PPC_VLE_LO16D:
	case R_PPC_VLE_HI16A:
	case R_PPC_VLE_HI16D:
	case R_PPC_VLE_HA16A:
	case R_PPC_VLE_HA16D:
	  break;

	case R_PPC_EMB_SDA2REL:
	  if (info->shared)
	    {
	      bad_shared_reloc (abfd, r_type);
	      return FALSE;
	    }
	  htab->sdata[1].sym->ref_regular = 1;
	  if (h != NULL)
	    {
	      ppc_elf_hash_entry (h)->has_sda_refs = TRUE;
	      h->non_got_ref = TRUE;
	    }
	  break;

	case R_PPC_VLE_SDA21_LO:
	case R_PPC_VLE_SDA21:
	case R_PPC_EMB_SDA21:
	case R_PPC_EMB_RELSDA:
	  if (info->shared)
	    {
	      bad_shared_reloc (abfd, r_type);
	      return FALSE;
	    }
	  if (h != NULL)
	    {
	      ppc_elf_hash_entry (h)->has_sda_refs = TRUE;
	      h->non_got_ref = TRUE;
	    }
	  break;

	case R_PPC_EMB_NADDR32:
	case R_PPC_EMB_NADDR16:
	case R_PPC_EMB_NADDR16_LO:
	case R_PPC_EMB_NADDR16_HI:
	case R_PPC_EMB_NADDR16_HA:
	  if (info->shared)
	    {
	      bad_shared_reloc (abfd, r_type);
	      return FALSE;
	    }
	  if (h != NULL)
	    h->non_got_ref = TRUE;
	  break;

	case R_PPC_PLTREL24:
	  if (h == NULL)
	    break;
	  /* Fall through */
	case R_PPC_PLT32:
	case R_PPC_PLTREL32:
	case R_PPC_PLT16_LO:
	case R_PPC_PLT16_HI:
	case R_PPC_PLT16_HA:
#ifdef DEBUG
	  fprintf (stderr, "Reloc requires a PLT entry\n");
#endif
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in finish_dynamic_symbol,
	     because this might be a case of linking PIC code without
	     linking in any dynamic objects, in which case we don't
	     need to generate a procedure linkage table after all.  */

	  if (h == NULL)
	    {
	      /* It does not make sense to have a procedure linkage
		 table entry for a local symbol.  */
	      info->callbacks->einfo (_("%P: %H: %s reloc against local symbol\n"),
				      abfd, sec, rel->r_offset,
				      ppc_elf_howto_table[r_type]->name);
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }
	  else
	    {
	      bfd_vma addend = 0;

	      if (r_type == R_PPC_PLTREL24)
		{
		  ppc_elf_tdata (abfd)->makes_plt_call = 1;
		  if (info->shared)
		    addend = rel->r_addend;
		}
	      h->needs_plt = 1;
	      if (!update_plt_info (abfd, &h->plt.plist, got2, addend))
		return FALSE;
	    }
	  break;

	  /* The following relocations don't need to propagate the
	     relocation if linking a shared object since they are
	     section relative.  */
	case R_PPC_SECTOFF:
	case R_PPC_SECTOFF_LO:
	case R_PPC_SECTOFF_HI:
	case R_PPC_SECTOFF_HA:
	case R_PPC_DTPREL16:
	case R_PPC_DTPREL16_LO:
	case R_PPC_DTPREL16_HI:
	case R_PPC_DTPREL16_HA:
	case R_PPC_TOC16:
	  break;

	case R_PPC_REL16:
	case R_PPC_REL16_LO:
	case R_PPC_REL16_HI:
	case R_PPC_REL16_HA:
	  ppc_elf_tdata (abfd)->has_rel16 = 1;
	  break;

	  /* These are just markers.  */
	case R_PPC_TLS:
	case R_PPC_EMB_MRKREF:
	case R_PPC_NONE:
	case R_PPC_max:
	case R_PPC_RELAX:
	case R_PPC_RELAX_PLT:
	case R_PPC_RELAX_PLTREL24:
	  break;

	  /* These should only appear in dynamic objects.  */
	case R_PPC_COPY:
	case R_PPC_GLOB_DAT:
	case R_PPC_JMP_SLOT:
	case R_PPC_RELATIVE:
	case R_PPC_IRELATIVE:
	  break;

	  /* These aren't handled yet.  We'll report an error later.  */
	case R_PPC_ADDR30:
	case R_PPC_EMB_RELSEC16:
	case R_PPC_EMB_RELST_LO:
	case R_PPC_EMB_RELST_HI:
	case R_PPC_EMB_RELST_HA:
	case R_PPC_EMB_BIT_FLD:
	  break;

	  /* This refers only to functions defined in the shared library.  */
	case R_PPC_LOCAL24PC:
	  if (h != NULL && h == htab->elf.hgot && htab->plt_type == PLT_UNSET)
	    {
	      htab->plt_type = PLT_OLD;
	      htab->old_bfd = abfd;
	    }
	  if (h != NULL && h->type == STT_GNU_IFUNC)
	    {
	      if (info->shared)
		{
		  info->callbacks->einfo (_("%P: %H: @local call to ifunc %s\n"),
					  abfd, sec, rel->r_offset,
					  h->root.root.string);
		  bfd_set_error (bfd_error_bad_value);
		  return FALSE;
		}
	      h->needs_plt = 1;
	      if (!update_plt_info (abfd, &h->plt.plist, NULL, 0))
		return FALSE;
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_PPC_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return FALSE;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_PPC_GNU_VTENTRY:
	  BFD_ASSERT (h != NULL);
	  if (h != NULL
	      && !bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return FALSE;
	  break;

	  /* We shouldn't really be seeing these.  */
	case R_PPC_TPREL32:
	case R_PPC_TPREL16:
	case R_PPC_TPREL16_LO:
	case R_PPC_TPREL16_HI:
	case R_PPC_TPREL16_HA:
	  if (info->shared)
	    info->flags |= DF_STATIC_TLS;
	  goto dodyn;

	  /* Nor these.  */
	case R_PPC_DTPMOD32:
	case R_PPC_DTPREL32:
	  goto dodyn;

	case R_PPC_REL32:
	  if (h == NULL
	      && got2 != NULL
	      && (sec->flags & SEC_CODE) != 0
	      && info->shared
	      && htab->plt_type == PLT_UNSET)
	    {
	      /* Old -fPIC gcc code has .long LCTOC1-LCFx just before
		 the start of a function, which assembles to a REL32
		 reference to .got2.  If we detect one of these, then
		 force the old PLT layout because the linker cannot
		 reliably deduce the GOT pointer value needed for
		 PLT call stubs.  */
	      asection *s;
	      Elf_Internal_Sym *isym;

	      isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					    abfd, r_symndx);
	      if (isym == NULL)
		return FALSE;

	      s = bfd_section_from_elf_index (abfd, isym->st_shndx);
	      if (s == got2)
		{
		  htab->plt_type = PLT_OLD;
		  htab->old_bfd = abfd;
		}
	    }
	  if (h == NULL || h == htab->elf.hgot)
	    break;
	  /* fall through */

	case R_PPC_ADDR32:
	case R_PPC_ADDR16:
	case R_PPC_ADDR16_LO:
	case R_PPC_ADDR16_HI:
	case R_PPC_ADDR16_HA:
	case R_PPC_UADDR32:
	case R_PPC_UADDR16:
	  if (h != NULL && !info->shared)
	    {
	      /* We may need a plt entry if the symbol turns out to be
		 a function defined in a dynamic object.  */
	      if (!update_plt_info (abfd, &h->plt.plist, NULL, 0))
		return FALSE;

	      /* We may need a copy reloc too.  */
	      h->non_got_ref = 1;
	      h->pointer_equality_needed = 1;
	    }
	  goto dodyn;

	case R_PPC_REL24:
	case R_PPC_REL14:
	case R_PPC_REL14_BRTAKEN:
	case R_PPC_REL14_BRNTAKEN:
	  if (h == NULL)
	    break;
	  if (h == htab->elf.hgot)
	    {
	      if (htab->plt_type == PLT_UNSET)
		{
		  htab->plt_type = PLT_OLD;
		  htab->old_bfd = abfd;
		}
	      break;
	    }
	  /* fall through */

	case R_PPC_ADDR24:
	case R_PPC_ADDR14:
	case R_PPC_ADDR14_BRTAKEN:
	case R_PPC_ADDR14_BRNTAKEN:
	  if (h != NULL && !info->shared)
	    {
	      /* We may need a plt entry if the symbol turns out to be
		 a function defined in a dynamic object.  */
	      h->needs_plt = 1;
	      if (!update_plt_info (abfd, &h->plt.plist, NULL, 0))
		return FALSE;
	      break;
	    }

	dodyn:
	  /* If we are creating a shared library, and this is a reloc
	     against a global symbol, or a non PC relative reloc
	     against a local symbol, then we need to copy the reloc
	     into the shared library.  However, if we are linking with
	     -Bsymbolic, we do not need to copy a reloc against a
	     global symbol which is defined in an object we are
	     including in the link (i.e., DEF_REGULAR is set).  At
	     this point we have not seen all the input files, so it is
	     possible that DEF_REGULAR is not set now but will be set
	     later (it is never cleared).  In case of a weak definition,
	     DEF_REGULAR may be cleared later by a strong definition in
	     a shared library.  We account for that possibility below by
	     storing information in the dyn_relocs field of the hash
	     table entry.  A similar situation occurs when creating
	     shared libraries and symbol visibility changes render the
	     symbol local.

	     If on the other hand, we are creating an executable, we
	     may need to keep relocations for symbols satisfied by a
	     dynamic library if we manage to avoid copy relocs for the
	     symbol.  */
	  if ((info->shared
	       && (must_be_dyn_reloc (info, r_type)
		   || (h != NULL
		       && (!SYMBOLIC_BIND (info, h)
			   || h->root.type == bfd_link_hash_defweak
			   || !h->def_regular))))
	      || (ELIMINATE_COPY_RELOCS
		  && !info->shared
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak
		      || !h->def_regular)))
	    {
#ifdef DEBUG
	      fprintf (stderr,
		       "ppc_elf_check_relocs needs to "
		       "create relocation for %s\n",
		       (h && h->root.root.string
			? h->root.root.string : "<unknown>"));
#endif
	      if (sreloc == NULL)
		{
		  if (htab->elf.dynobj == NULL)
		    htab->elf.dynobj = abfd;

		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->elf.dynobj, 2, abfd, /*rela?*/ TRUE);

		  if (sreloc == NULL)
		    return FALSE;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		{
		  struct elf_dyn_relocs *p;
		  struct elf_dyn_relocs **rel_head;

		  rel_head = &ppc_elf_hash_entry (h)->dyn_relocs;
		  p = *rel_head;
		  if (p == NULL || p->sec != sec)
		    {
		      p = bfd_alloc (htab->elf.dynobj, sizeof *p);
		      if (p == NULL)
			return FALSE;
		      p->next = *rel_head;
		      *rel_head = p;
		      p->sec = sec;
		      p->count = 0;
		      p->pc_count = 0;
		    }
		  p->count += 1;
		  if (!must_be_dyn_reloc (info, r_type))
		    p->pc_count += 1;
		}
	      else
		{
		  /* Track dynamic relocs needed for local syms too.
		     We really need local syms available to do this
		     easily.  Oh well.  */
		  struct ppc_dyn_relocs *p;
		  struct ppc_dyn_relocs **rel_head;
		  bfd_boolean is_ifunc;
		  asection *s;
		  void *vpp;
		  Elf_Internal_Sym *isym;

		  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
						abfd, r_symndx);
		  if (isym == NULL)
		    return FALSE;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    s = sec;

		  vpp = &elf_section_data (s)->local_dynrel;
		  rel_head = (struct ppc_dyn_relocs **) vpp;
		  is_ifunc = ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC;
		  p = *rel_head;
		  if (p != NULL && p->sec == sec && p->ifunc != is_ifunc)
		    p = p->next;
		  if (p == NULL || p->sec != sec || p->ifunc != is_ifunc)
		    {
		      p = bfd_alloc (htab->elf.dynobj, sizeof *p);
		      if (p == NULL)
			return FALSE;
		      p->next = *rel_head;
		      *rel_head = p;
		      p->sec = sec;
		      p->ifunc = is_ifunc;
		      p->count = 0;
		    }
		  p->count += 1;
		}
	    }

	  break;
	}
    }

  return TRUE;
}


/* Merge object attributes from IBFD into OBFD.  Raise an error if
   there are conflicting attributes.  */
static bfd_boolean
ppc_elf_merge_obj_attributes (bfd *ibfd, bfd *obfd)
{
  obj_attribute *in_attr, *in_attrs;
  obj_attribute *out_attr, *out_attrs;

  if (!elf_known_obj_attributes_proc (obfd)[0].i)
    {
      /* This is the first object.  Copy the attributes.  */
      _bfd_elf_copy_obj_attributes (ibfd, obfd);

      /* Use the Tag_null value to indicate the attributes have been
	 initialized.  */
      elf_known_obj_attributes_proc (obfd)[0].i = 1;

      return TRUE;
    }

  in_attrs = elf_known_obj_attributes (ibfd)[OBJ_ATTR_GNU];
  out_attrs = elf_known_obj_attributes (obfd)[OBJ_ATTR_GNU];

  /* Check for conflicting Tag_GNU_Power_ABI_FP attributes and merge
     non-conflicting ones.  */
  in_attr = &in_attrs[Tag_GNU_Power_ABI_FP];
  out_attr = &out_attrs[Tag_GNU_Power_ABI_FP];
  if (in_attr->i != out_attr->i)
    {
      out_attr->type = 1;
      if (out_attr->i == 0)
	out_attr->i = in_attr->i;
      else if (in_attr->i == 0)
	;
      else if (out_attr->i == 1 && in_attr->i == 2)
	_bfd_error_handler
	  (_("Warning: %B uses hard float, %B uses soft float"), obfd, ibfd);
      else if (out_attr->i == 1 && in_attr->i == 3)
	_bfd_error_handler
	  (_("Warning: %B uses double-precision hard float, %B uses single-precision hard float"),
	  obfd, ibfd);
      else if (out_attr->i == 3 && in_attr->i == 1)
	_bfd_error_handler
	  (_("Warning: %B uses double-precision hard float, %B uses single-precision hard float"),
	  ibfd, obfd);
      else if (out_attr->i == 3 && in_attr->i == 2)
	_bfd_error_handler
	  (_("Warning: %B uses soft float, %B uses single-precision hard float"),
	  ibfd, obfd);
      else if (out_attr->i == 2 && (in_attr->i == 1 || in_attr->i == 3))
	_bfd_error_handler
	  (_("Warning: %B uses hard float, %B uses soft float"), ibfd, obfd);
      else if (in_attr->i > 3)
	_bfd_error_handler
	  (_("Warning: %B uses unknown floating point ABI %d"), ibfd,
	   in_attr->i);
      else
	_bfd_error_handler
	  (_("Warning: %B uses unknown floating point ABI %d"), obfd,
	   out_attr->i);
    }

  /* Check for conflicting Tag_GNU_Power_ABI_Vector attributes and
     merge non-conflicting ones.  */
  in_attr = &in_attrs[Tag_GNU_Power_ABI_Vector];
  out_attr = &out_attrs[Tag_GNU_Power_ABI_Vector];
  if (in_attr->i != out_attr->i)
    {
      const char *in_abi = NULL, *out_abi = NULL;

      switch (in_attr->i)
	{
	case 1: in_abi = "generic"; break;
	case 2: in_abi = "AltiVec"; break;
	case 3: in_abi = "SPE"; break;
	}

      switch (out_attr->i)
	{
	case 1: out_abi = "generic"; break;
	case 2: out_abi = "AltiVec"; break;
	case 3: out_abi = "SPE"; break;
	}

      out_attr->type = 1;
      if (out_attr->i == 0)
	out_attr->i = in_attr->i;
      else if (in_attr->i == 0)
	;
      /* For now, allow generic to transition to AltiVec or SPE
	 without a warning.  If GCC marked files with their stack
	 alignment and used don't-care markings for files which are
	 not affected by the vector ABI, we could warn about this
	 case too.  */
      else if (out_attr->i == 1)
	out_attr->i = in_attr->i;
      else if (in_attr->i == 1)
	;
      else if (in_abi == NULL)
	_bfd_error_handler
	  (_("Warning: %B uses unknown vector ABI %d"), ibfd,
	   in_attr->i);
      else if (out_abi == NULL)
	_bfd_error_handler
	  (_("Warning: %B uses unknown vector ABI %d"), obfd,
	   in_attr->i);
      else
	_bfd_error_handler
	  (_("Warning: %B uses vector ABI \"%s\", %B uses \"%s\""),
	   ibfd, obfd, in_abi, out_abi);
    }

  /* Check for conflicting Tag_GNU_Power_ABI_Struct_Return attributes
     and merge non-conflicting ones.  */
  in_attr = &in_attrs[Tag_GNU_Power_ABI_Struct_Return];
  out_attr = &out_attrs[Tag_GNU_Power_ABI_Struct_Return];
  if (in_attr->i != out_attr->i)
    {
      out_attr->type = 1;
      if (out_attr->i == 0)
       out_attr->i = in_attr->i;
      else if (in_attr->i == 0)
       ;
      else if (out_attr->i == 1 && in_attr->i == 2)
       _bfd_error_handler
         (_("Warning: %B uses r3/r4 for small structure returns, %B uses memory"), obfd, ibfd);
      else if (out_attr->i == 2 && in_attr->i == 1)
       _bfd_error_handler
         (_("Warning: %B uses r3/r4 for small structure returns, %B uses memory"), ibfd, obfd);
      else if (in_attr->i > 2)
       _bfd_error_handler
         (_("Warning: %B uses unknown small structure return convention %d"), ibfd,
          in_attr->i);
      else
       _bfd_error_handler
         (_("Warning: %B uses unknown small structure return convention %d"), obfd,
          out_attr->i);
    }

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  _bfd_elf_merge_object_attributes (ibfd, obfd);

  return TRUE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bfd_boolean
ppc_elf_merge_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  flagword old_flags;
  flagword new_flags;
  bfd_boolean error;

  if (!is_ppc_elf (ibfd) || !is_ppc_elf (obfd))
    return TRUE;

  /* Check if we have the same endianness.  */
  if (! _bfd_generic_verify_endian_match (ibfd, obfd))
    return FALSE;

  if (!ppc_elf_merge_obj_attributes (ibfd, obfd))
    return FALSE;

  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;
  if (!elf_flags_init (obfd))
    {
      /* First call, no flags set.  */
      elf_flags_init (obfd) = TRUE;
      elf_elfheader (obfd)->e_flags = new_flags;
    }

  /* Compatible flags are ok.  */
  else if (new_flags == old_flags)
    ;

  /* Incompatible flags.  */
  else
    {
      /* Warn about -mrelocatable mismatch.  Allow -mrelocatable-lib
	 to be linked with either.  */
      error = FALSE;
      if ((new_flags & EF_PPC_RELOCATABLE) != 0
	  && (old_flags & (EF_PPC_RELOCATABLE | EF_PPC_RELOCATABLE_LIB)) == 0)
	{
	  error = TRUE;
	  (*_bfd_error_handler)
	    (_("%B: compiled with -mrelocatable and linked with "
	       "modules compiled normally"), ibfd);
	}
      else if ((new_flags & (EF_PPC_RELOCATABLE | EF_PPC_RELOCATABLE_LIB)) == 0
	       && (old_flags & EF_PPC_RELOCATABLE) != 0)
	{
	  error = TRUE;
	  (*_bfd_error_handler)
	    (_("%B: compiled normally and linked with "
	       "modules compiled with -mrelocatable"), ibfd);
	}

      /* The output is -mrelocatable-lib iff both the input files are.  */
      if (! (new_flags & EF_PPC_RELOCATABLE_LIB))
	elf_elfheader (obfd)->e_flags &= ~EF_PPC_RELOCATABLE_LIB;

      /* The output is -mrelocatable iff it can't be -mrelocatable-lib,
	 but each input file is either -mrelocatable or -mrelocatable-lib.  */
      if (! (elf_elfheader (obfd)->e_flags & EF_PPC_RELOCATABLE_LIB)
	  && (new_flags & (EF_PPC_RELOCATABLE_LIB | EF_PPC_RELOCATABLE))
	  && (old_flags & (EF_PPC_RELOCATABLE_LIB | EF_PPC_RELOCATABLE)))
	elf_elfheader (obfd)->e_flags |= EF_PPC_RELOCATABLE;

      /* Do not warn about eabi vs. V.4 mismatch, just or in the bit if
	 any module uses it.  */
      elf_elfheader (obfd)->e_flags |= (new_flags & EF_PPC_EMB);

      new_flags &= ~(EF_PPC_RELOCATABLE | EF_PPC_RELOCATABLE_LIB | EF_PPC_EMB);
      old_flags &= ~(EF_PPC_RELOCATABLE | EF_PPC_RELOCATABLE_LIB | EF_PPC_EMB);

      /* Warn about any other mismatches.  */
      if (new_flags != old_flags)
	{
	  error = TRUE;
	  (*_bfd_error_handler)
	    (_("%B: uses different e_flags (0x%lx) fields "
	       "than previous modules (0x%lx)"),
	     ibfd, (long) new_flags, (long) old_flags);
	}

      if (error)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}
    }

  return TRUE;
}

static void
ppc_elf_vle_split16 (bfd *output_bfd, bfd_byte *loc,
		     bfd_vma value,
		     split16_format_type split16_format)

{
  unsigned int insn, top5;

  insn = bfd_get_32 (output_bfd, loc);
  top5 = value & 0xf800;
  top5 = top5 << (split16_format == split16a_type ? 9 : 5);
  insn |= top5;
  insn |= value & 0x7ff;
  bfd_put_32 (output_bfd, insn, loc);
}


/* Choose which PLT scheme to use, and set .plt flags appropriately.
   Returns -1 on error, 0 for old PLT, 1 for new PLT.  */
int
ppc_elf_select_plt_layout (bfd *output_bfd ATTRIBUTE_UNUSED,
			   struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab;
  flagword flags;

  htab = ppc_elf_hash_table (info);

  if (htab->plt_type == PLT_UNSET)
    {
      struct elf_link_hash_entry *h;

      if (htab->params->plt_style == PLT_OLD)
	htab->plt_type = PLT_OLD;
      else if (info->shared
	       && htab->elf.dynamic_sections_created
	       && (h = elf_link_hash_lookup (&htab->elf, "_mcount",
					     FALSE, FALSE, TRUE)) != NULL
	       && (h->type == STT_FUNC
		   || h->needs_plt)
	       && h->ref_regular
	       && !(SYMBOL_CALLS_LOCAL (info, h)
		    || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
			&& h->root.type == bfd_link_hash_undefweak)))
	{
	  /* Profiling of shared libs (and pies) is not supported with
	     secure plt, because ppc32 does profiling before a
	     function prologue and a secure plt pic call stubs needs
	     r30 to be set up.  */
	  htab->plt_type = PLT_OLD;
	}
      else
	{
	  bfd *ibfd;
	  enum ppc_elf_plt_type plt_type = htab->params->plt_style;

	  /* Look through the reloc flags left by ppc_elf_check_relocs.
	     Use the old style bss plt if a file makes plt calls
	     without using the new relocs, and if ld isn't given
	     --secure-plt and we never see REL16 relocs.  */
	  if (plt_type == PLT_UNSET)
	    plt_type = PLT_OLD;
	  for (ibfd = info->input_bfds; ibfd; ibfd = ibfd->link.next)
	    if (is_ppc_elf (ibfd))
	      {
		if (ppc_elf_tdata (ibfd)->has_rel16)
		  plt_type = PLT_NEW;
		else if (ppc_elf_tdata (ibfd)->makes_plt_call)
		  {
		    plt_type = PLT_OLD;
		    htab->old_bfd = ibfd;
		    break;
		  }
	      }
	  htab->plt_type = plt_type;
	}
    }
  if (htab->plt_type == PLT_OLD && htab->params->plt_style == PLT_NEW)
    {
      if (htab->old_bfd != NULL)
	info->callbacks->einfo (_("%P: bss-plt forced due to %B\n"),
				htab->old_bfd);
      else
	info->callbacks->einfo (_("%P: bss-plt forced by profiling\n"));
    }

  BFD_ASSERT (htab->plt_type != PLT_VXWORKS);

  if (htab->plt_type == PLT_NEW)
    {
      flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
	       | SEC_IN_MEMORY | SEC_LINKER_CREATED);

      /* The new PLT is a loaded section.  */
      if (htab->plt != NULL
	  && !bfd_set_section_flags (htab->elf.dynobj, htab->plt, flags))
	return -1;

      /* The new GOT is not executable.  */
      if (htab->got != NULL
	  && !bfd_set_section_flags (htab->elf.dynobj, htab->got, flags))
	return -1;
    }
  else
    {
      /* Stop an unused .glink section from affecting .text alignment.  */
      if (htab->glink != NULL
	  && !bfd_set_section_alignment (htab->elf.dynobj, htab->glink, 0))
	return -1;
    }
  return htab->plt_type == PLT_NEW;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
ppc_elf_gc_mark_hook (asection *sec,
		      struct bfd_link_info *info,
		      Elf_Internal_Rela *rel,
		      struct elf_link_hash_entry *h,
		      Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_PPC_GNU_VTINHERIT:
      case R_PPC_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Update the got, plt and dynamic reloc reference counts for the
   section being removed.  */

static bfd_boolean
ppc_elf_gc_sweep_hook (bfd *abfd,
		       struct bfd_link_info *info,
		       asection *sec,
		       const Elf_Internal_Rela *relocs)
{
  struct ppc_elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_signed_vma *local_got_refcounts;
  const Elf_Internal_Rela *rel, *relend;
  asection *got2;

  if (info->relocatable)
    return TRUE;

  if ((sec->flags & SEC_ALLOC) == 0)
    return TRUE;

  elf_section_data (sec)->local_dynrel = NULL;

  htab = ppc_elf_hash_table (info);
  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);
  got2 = bfd_get_section_by_name (abfd, ".got2");

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      unsigned long r_symndx;
      enum elf_ppc_reloc_type r_type;
      struct elf_link_hash_entry *h = NULL;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx >= symtab_hdr->sh_info)
	{
	  struct elf_dyn_relocs **pp, *p;
	  struct ppc_elf_link_hash_entry *eh;

	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	  eh = (struct ppc_elf_link_hash_entry *) h;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; pp = &p->next)
	    if (p->sec == sec)
	      {
		/* Everything must go for SEC.  */
		*pp = p->next;
		break;
	      }
	}

      r_type = ELF32_R_TYPE (rel->r_info);
      if (!htab->is_vxworks
	  && h == NULL
	  && local_got_refcounts != NULL
	  && (!info->shared
	      || is_branch_reloc (r_type)))
	{
	  struct plt_entry **local_plt = (struct plt_entry **)
	    (local_got_refcounts + symtab_hdr->sh_info);
	  char *local_got_tls_masks = (char *)
	    (local_plt + symtab_hdr->sh_info);
	  if ((local_got_tls_masks[r_symndx] & PLT_IFUNC) != 0)
	    {
	      struct plt_entry **ifunc = local_plt + r_symndx;
	      bfd_vma addend = 0;
	      struct plt_entry *ent;

	      if (r_type == R_PPC_PLTREL24 && info->shared)
		addend = rel->r_addend;
	      ent = find_plt_ent (ifunc, got2, addend);
	      if (ent->plt.refcount > 0)
		ent->plt.refcount -= 1;
	      continue;
	    }
	}

      switch (r_type)
	{
	case R_PPC_GOT_TLSLD16:
	case R_PPC_GOT_TLSLD16_LO:
	case R_PPC_GOT_TLSLD16_HI:
	case R_PPC_GOT_TLSLD16_HA:
	case R_PPC_GOT_TLSGD16:
	case R_PPC_GOT_TLSGD16_LO:
	case R_PPC_GOT_TLSGD16_HI:
	case R_PPC_GOT_TLSGD16_HA:
	case R_PPC_GOT_TPREL16:
	case R_PPC_GOT_TPREL16_LO:
	case R_PPC_GOT_TPREL16_HI:
	case R_PPC_GOT_TPREL16_HA:
	case R_PPC_GOT_DTPREL16:
	case R_PPC_GOT_DTPREL16_LO:
	case R_PPC_GOT_DTPREL16_HI:
	case R_PPC_GOT_DTPREL16_HA:
	case R_PPC_GOT16:
	case R_PPC_GOT16_LO:
	case R_PPC_GOT16_HI:
	case R_PPC_GOT16_HA:
	  if (h != NULL)
	    {
	      if (h->got.refcount > 0)
		h->got.refcount--;
	      if (!info->shared)
		{
		  struct plt_entry *ent;

		  ent = find_plt_ent (&h->plt.plist, NULL, 0);
		  if (ent != NULL && ent->plt.refcount > 0)
		    ent->plt.refcount -= 1;
		}
	    }
	  else if (local_got_refcounts != NULL)
	    {
	      if (local_got_refcounts[r_symndx] > 0)
		local_got_refcounts[r_symndx]--;
	    }
	  break;

	case R_PPC_REL24:
	case R_PPC_REL14:
	case R_PPC_REL14_BRTAKEN:
	case R_PPC_REL14_BRNTAKEN:
	case R_PPC_REL32:
	  if (h == NULL || h == htab->elf.hgot)
	    break;
	  /* Fall thru */

	case R_PPC_ADDR32:
	case R_PPC_ADDR24:
	case R_PPC_ADDR16:
	case R_PPC_ADDR16_LO:
	case R_PPC_ADDR16_HI:
	case R_PPC_ADDR16_HA:
	case R_PPC_ADDR14:
	case R_PPC_ADDR14_BRTAKEN:
	case R_PPC_ADDR14_BRNTAKEN:
	case R_PPC_UADDR32:
	case R_PPC_UADDR16:
	  if (info->shared)
	    break;

	case R_PPC_PLT32:
	case R_PPC_PLTREL24:
	case R_PPC_PLTREL32:
	case R_PPC_PLT16_LO:
	case R_PPC_PLT16_HI:
	case R_PPC_PLT16_HA:
	  if (h != NULL)
	    {
	      bfd_vma addend = 0;
	      struct plt_entry *ent;

	      if (r_type == R_PPC_PLTREL24 && info->shared)
		addend = rel->r_addend;
	      ent = find_plt_ent (&h->plt.plist, got2, addend);
	      if (ent != NULL && ent->plt.refcount > 0)
		ent->plt.refcount -= 1;
	    }
	  break;

	default:
	  break;
	}
    }
  return TRUE;
}

/* Set plt output section type, htab->tls_get_addr, and call the
   generic ELF tls_setup function.  */

asection *
ppc_elf_tls_setup (bfd *obfd, struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab;

  htab = ppc_elf_hash_table (info);
  htab->tls_get_addr = elf_link_hash_lookup (&htab->elf, "__tls_get_addr",
					     FALSE, FALSE, TRUE);
  if (htab->plt_type != PLT_NEW)
    htab->params->no_tls_get_addr_opt = TRUE;

  if (!htab->params->no_tls_get_addr_opt)
    {
      struct elf_link_hash_entry *opt, *tga;
      opt = elf_link_hash_lookup (&htab->elf, "__tls_get_addr_opt",
				  FALSE, FALSE, TRUE);
      if (opt != NULL
	  && (opt->root.type == bfd_link_hash_defined
	      || opt->root.type == bfd_link_hash_defweak))
	{
	  /* If glibc supports an optimized __tls_get_addr call stub,
	     signalled by the presence of __tls_get_addr_opt, and we'll
	     be calling __tls_get_addr via a plt call stub, then
	     make __tls_get_addr point to __tls_get_addr_opt.  */
	  tga = htab->tls_get_addr;
	  if (htab->elf.dynamic_sections_created
	      && tga != NULL
	      && (tga->type == STT_FUNC
		  || tga->needs_plt)
	      && !(SYMBOL_CALLS_LOCAL (info, tga)
		   || (ELF_ST_VISIBILITY (tga->other) != STV_DEFAULT
		       && tga->root.type == bfd_link_hash_undefweak)))
	    {
	      struct plt_entry *ent;
	      for (ent = tga->plt.plist; ent != NULL; ent = ent->next)
		if (ent->plt.refcount > 0)
		  break;
	      if (ent != NULL)
		{
		  tga->root.type = bfd_link_hash_indirect;
		  tga->root.u.i.link = &opt->root;
		  ppc_elf_copy_indirect_symbol (info, opt, tga);
		  if (opt->dynindx != -1)
		    {
		      /* Use __tls_get_addr_opt in dynamic relocations.  */
		      opt->dynindx = -1;
		      _bfd_elf_strtab_delref (elf_hash_table (info)->dynstr,
					      opt->dynstr_index);
		      if (!bfd_elf_link_record_dynamic_symbol (info, opt))
			return FALSE;
		    }
		  htab->tls_get_addr = opt;
		}
	    }
	}
      else
	htab->params->no_tls_get_addr_opt = TRUE;
    }
  if (htab->plt_type == PLT_NEW
      && htab->plt != NULL
      && htab->plt->output_section != NULL)
    {
      elf_section_type (htab->plt->output_section) = SHT_PROGBITS;
      elf_section_flags (htab->plt->output_section) = SHF_ALLOC + SHF_WRITE;
    }

  return _bfd_elf_tls_setup (obfd, info);
}

/* Return TRUE iff REL is a branch reloc with a global symbol matching
   HASH.  */

static bfd_boolean
branch_reloc_hash_match (const bfd *ibfd,
			 const Elf_Internal_Rela *rel,
			 const struct elf_link_hash_entry *hash)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (ibfd);
  enum elf_ppc_reloc_type r_type = ELF32_R_TYPE (rel->r_info);
  unsigned int r_symndx = ELF32_R_SYM (rel->r_info);

  if (r_symndx >= symtab_hdr->sh_info && is_branch_reloc (r_type))
    {
      struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (ibfd);
      struct elf_link_hash_entry *h;

      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;
      if (h == hash)
	return TRUE;
    }
  return FALSE;
}

/* Run through all the TLS relocs looking for optimization
   opportunities.  */

bfd_boolean
ppc_elf_tls_optimize (bfd *obfd ATTRIBUTE_UNUSED,
		      struct bfd_link_info *info)
{
  bfd *ibfd;
  asection *sec;
  struct ppc_elf_link_hash_table *htab;
  int pass;

  if (info->relocatable || !info->executable)
    return TRUE;

  htab = ppc_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  /* Make two passes through the relocs.  First time check that tls
     relocs involved in setting up a tls_get_addr call are indeed
     followed by such a call.  If they are not, don't do any tls
     optimization.  On the second pass twiddle tls_mask flags to
     notify relocate_section that optimization can be done, and
     adjust got and plt refcounts.  */
  for (pass = 0; pass < 2; ++pass)
    for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
      {
	Elf_Internal_Sym *locsyms = NULL;
	Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (ibfd);
	asection *got2 = bfd_get_section_by_name (ibfd, ".got2");

	for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	  if (sec->has_tls_reloc && !bfd_is_abs_section (sec->output_section))
	    {
	      Elf_Internal_Rela *relstart, *rel, *relend;
	      int expecting_tls_get_addr = 0;

	      /* Read the relocations.  */
	      relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL,
						    info->keep_memory);
	      if (relstart == NULL)
		return FALSE;

	      relend = relstart + sec->reloc_count;
	      for (rel = relstart; rel < relend; rel++)
		{
		  enum elf_ppc_reloc_type r_type;
		  unsigned long r_symndx;
		  struct elf_link_hash_entry *h = NULL;
		  char *tls_mask;
		  char tls_set, tls_clear;
		  bfd_boolean is_local;
		  bfd_signed_vma *got_count;

		  r_symndx = ELF32_R_SYM (rel->r_info);
		  if (r_symndx >= symtab_hdr->sh_info)
		    {
		      struct elf_link_hash_entry **sym_hashes;

		      sym_hashes = elf_sym_hashes (ibfd);
		      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
		      while (h->root.type == bfd_link_hash_indirect
			     || h->root.type == bfd_link_hash_warning)
			h = (struct elf_link_hash_entry *) h->root.u.i.link;
		    }

		  is_local = FALSE;
		  if (h == NULL
		      || !h->def_dynamic)
		    is_local = TRUE;

		  r_type = ELF32_R_TYPE (rel->r_info);
		  /* If this section has old-style __tls_get_addr calls
		     without marker relocs, then check that each
		     __tls_get_addr call reloc is preceded by a reloc
		     that conceivably belongs to the __tls_get_addr arg
		     setup insn.  If we don't find matching arg setup
		     relocs, don't do any tls optimization.  */
		  if (pass == 0
		      && sec->has_tls_get_addr_call
		      && h != NULL
		      && h == htab->tls_get_addr
		      && !expecting_tls_get_addr
		      && is_branch_reloc (r_type))
		    {
		      info->callbacks->minfo ("%H __tls_get_addr lost arg, "
					      "TLS optimization disabled\n",
					      ibfd, sec, rel->r_offset);
		      if (elf_section_data (sec)->relocs != relstart)
			free (relstart);
		      return TRUE;
		    }

		  expecting_tls_get_addr = 0;
		  switch (r_type)
		    {
		    case R_PPC_GOT_TLSLD16:
		    case R_PPC_GOT_TLSLD16_LO:
		      expecting_tls_get_addr = 1;
		      /* Fall thru */

		    case R_PPC_GOT_TLSLD16_HI:
		    case R_PPC_GOT_TLSLD16_HA:
		      /* These relocs should never be against a symbol
			 defined in a shared lib.  Leave them alone if
			 that turns out to be the case.  */
		      if (!is_local)
			continue;

		      /* LD -> LE */
		      tls_set = 0;
		      tls_clear = TLS_LD;
		      break;

		    case R_PPC_GOT_TLSGD16:
		    case R_PPC_GOT_TLSGD16_LO:
		      expecting_tls_get_addr = 1;
		      /* Fall thru */

		    case R_PPC_GOT_TLSGD16_HI:
		    case R_PPC_GOT_TLSGD16_HA:
		      if (is_local)
			/* GD -> LE */
			tls_set = 0;
		      else
			/* GD -> IE */
			tls_set = TLS_TLS | TLS_TPRELGD;
		      tls_clear = TLS_GD;
		      break;

		    case R_PPC_GOT_TPREL16:
		    case R_PPC_GOT_TPREL16_LO:
		    case R_PPC_GOT_TPREL16_HI:
		    case R_PPC_GOT_TPREL16_HA:
		      if (is_local)
			{
			  /* IE -> LE */
			  tls_set = 0;
			  tls_clear = TLS_TPREL;
			  break;
			}
		      else
			continue;

		    case R_PPC_TLSGD:
		    case R_PPC_TLSLD:
		      expecting_tls_get_addr = 2;
		      tls_set = 0;
		      tls_clear = 0;
		      break;

		    default:
		      continue;
		    }

		  if (pass == 0)
		    {
		      if (!expecting_tls_get_addr
			  || (expecting_tls_get_addr == 1
			      && !sec->has_tls_get_addr_call))
			continue;

		      if (rel + 1 < relend
			  && branch_reloc_hash_match (ibfd, rel + 1,
						      htab->tls_get_addr))
			continue;

		      /* Uh oh, we didn't find the expected call.  We
			 could just mark this symbol to exclude it
			 from tls optimization but it's safer to skip
			 the entire optimization.  */
		      info->callbacks->minfo (_("%H arg lost __tls_get_addr, "
						"TLS optimization disabled\n"),
					      ibfd, sec, rel->r_offset);
		      if (elf_section_data (sec)->relocs != relstart)
			free (relstart);
		      return TRUE;
		    }

		  if (expecting_tls_get_addr)
		    {
		      struct plt_entry *ent;
		      bfd_vma addend = 0;

		      if (info->shared
			  && ELF32_R_TYPE (rel[1].r_info) == R_PPC_PLTREL24)
			addend = rel[1].r_addend;
		      ent = find_plt_ent (&htab->tls_get_addr->plt.plist,
					  got2, addend);
		      if (ent != NULL && ent->plt.refcount > 0)
			ent->plt.refcount -= 1;

		      if (expecting_tls_get_addr == 2)
			continue;
		    }

		  if (h != NULL)
		    {
		      tls_mask = &ppc_elf_hash_entry (h)->tls_mask;
		      got_count = &h->got.refcount;
		    }
		  else
		    {
		      bfd_signed_vma *lgot_refs;
		      struct plt_entry **local_plt;
		      char *lgot_masks;

		      if (locsyms == NULL)
			{
			  locsyms = (Elf_Internal_Sym *) symtab_hdr->contents;
			  if (locsyms == NULL)
			    locsyms = bfd_elf_get_elf_syms (ibfd, symtab_hdr,
							    symtab_hdr->sh_info,
							    0, NULL, NULL, NULL);
			  if (locsyms == NULL)
			    {
			      if (elf_section_data (sec)->relocs != relstart)
				free (relstart);
			      return FALSE;
			    }
			}
		      lgot_refs = elf_local_got_refcounts (ibfd);
		      if (lgot_refs == NULL)
			abort ();
		      local_plt = (struct plt_entry **)
			(lgot_refs + symtab_hdr->sh_info);
		      lgot_masks = (char *) (local_plt + symtab_hdr->sh_info);
		      tls_mask = &lgot_masks[r_symndx];
		      got_count = &lgot_refs[r_symndx];
		    }

		  if (tls_set == 0)
		    {
		      /* We managed to get rid of a got entry.  */
		      if (*got_count > 0)
			*got_count -= 1;
		    }

		  *tls_mask |= tls_set;
		  *tls_mask &= ~tls_clear;
		}

	      if (elf_section_data (sec)->relocs != relstart)
		free (relstart);
	    }

	if (locsyms != NULL
	    && (symtab_hdr->contents != (unsigned char *) locsyms))
	  {
	    if (!info->keep_memory)
	      free (locsyms);
	    else
	      symtab_hdr->contents = (unsigned char *) locsyms;
	  }
      }
  return TRUE;
}

/* Return true if we have dynamic relocs that apply to read-only sections.  */

static bfd_boolean
readonly_dynrelocs (struct elf_link_hash_entry *h)
{
  struct elf_dyn_relocs *p;

  for (p = ppc_elf_hash_entry (h)->dyn_relocs; p != NULL; p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s != NULL
	  && ((s->flags & (SEC_READONLY | SEC_ALLOC))
	      == (SEC_READONLY | SEC_ALLOC)))
	return TRUE;
    }
  return FALSE;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bfd_boolean
ppc_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
			       struct elf_link_hash_entry *h)
{
  struct ppc_elf_link_hash_table *htab;
  asection *s;

#ifdef DEBUG
  fprintf (stderr, "ppc_elf_adjust_dynamic_symbol called for %s\n",
	   h->root.root.string);
#endif

  /* Make sure we know what is going on here.  */
  htab = ppc_elf_hash_table (info);
  BFD_ASSERT (htab->elf.dynobj != NULL
	      && (h->needs_plt
		  || h->type == STT_GNU_IFUNC
		  || h->u.weakdef != NULL
		  || (h->def_dynamic
		      && h->ref_regular
		      && !h->def_regular)));

  /* Deal with function syms.  */
  if (h->type == STT_FUNC
      || h->type == STT_GNU_IFUNC
      || h->needs_plt)
    {
      /* Clear procedure linkage table information for any symbol that
	 won't need a .plt entry.  */
      struct plt_entry *ent;
      for (ent = h->plt.plist; ent != NULL; ent = ent->next)
	if (ent->plt.refcount > 0)
	  break;
      if (ent == NULL
	  || (h->type != STT_GNU_IFUNC
	      && (SYMBOL_CALLS_LOCAL (info, h)
		  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		      && h->root.type == bfd_link_hash_undefweak))))
	{
	  /* A PLT entry is not required/allowed when:

	     1. We are not using ld.so; because then the PLT entry
	     can't be set up, so we can't use one.  In this case,
	     ppc_elf_adjust_dynamic_symbol won't even be called.

	     2. GC has rendered the entry unused.

	     3. We know for certain that a call to this symbol
	     will go to this object, or will remain undefined.  */
	  h->plt.plist = NULL;
	  h->needs_plt = 0;
	  h->pointer_equality_needed = 0;
	}
      else
	{
	  /* Taking a function's address in a read/write section
	     doesn't require us to define the function symbol in the
	     executable on a plt call stub.  A dynamic reloc can
	     be used instead.  */
	  if (h->pointer_equality_needed
	      && h->type != STT_GNU_IFUNC
	      && !htab->is_vxworks
	      && !ppc_elf_hash_entry (h)->has_sda_refs
	      && !readonly_dynrelocs (h))
	    {
	      h->pointer_equality_needed = 0;
	      h->non_got_ref = 0;
	    }

	  /* After adjust_dynamic_symbol, non_got_ref set in the
	     non-shared case means that we have allocated space in
	     .dynbss for the symbol and thus dyn_relocs for this
	     symbol should be discarded.
	     If we get here we know we are making a PLT entry for this
	     symbol, and in an executable we'd normally resolve
	     relocations against this symbol to the PLT entry.  Allow
	     dynamic relocs if the reference is weak, and the dynamic
	     relocs will not cause text relocation.  */
	  else if (!h->ref_regular_nonweak
		   && h->non_got_ref
		   && h->type != STT_GNU_IFUNC
		   && !htab->is_vxworks
		   && !ppc_elf_hash_entry (h)->has_sda_refs
		   && !readonly_dynrelocs (h))
	    h->non_got_ref = 0;
	}
      return TRUE;
    }
  else
    h->plt.plist = NULL;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
		  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
      if (ELIMINATE_COPY_RELOCS)
	h->non_got_ref = h->u.weakdef->non_got_ref;
      return TRUE;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (info->shared)
    return TRUE;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return TRUE;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return TRUE;
    }

   /* If we didn't find any dynamic relocs in read-only sections, then
      we'll be keeping the dynamic relocs and avoiding the copy reloc.
      We can't do this if there are any small data relocations.  This
      doesn't work on VxWorks, where we can not have dynamic
      relocations (other than copy and jump slot relocations) in an
      executable.  */
  if (ELIMINATE_COPY_RELOCS
      && !ppc_elf_hash_entry (h)->has_sda_refs
      && !htab->is_vxworks
      && !h->def_regular
      && !readonly_dynrelocs (h))
    {
      h->non_got_ref = 0;
      return TRUE;
    }

  /* Protected variables do not work with .dynbss.  The copy in
     .dynbss won't be used by the shared library with the protected
     definition for the variable.  Text relocations are preferable
     to an incorrect program.  */
  if (h->protected_def)
    {
      h->non_got_ref = 0;
      return TRUE;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.

     Of course, if the symbol is referenced using SDAREL relocs, we
     must instead allocate it in .sbss.  */

  if (ppc_elf_hash_entry (h)->has_sda_refs)
    s = htab->dynsbss;
  else
    s = htab->dynbss;
  BFD_ASSERT (s != NULL);

  /* We must generate a R_PPC_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      asection *srel;

      if (ppc_elf_hash_entry (h)->has_sda_refs)
	srel = htab->relsbss;
      else
	srel = htab->relbss;
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Generate a symbol to mark plt call stubs.  For non-PIC code the sym is
   xxxxxxxx.plt_call32.<callee> where xxxxxxxx is a hex number, usually 0,
   specifying the addend on the plt relocation.  For -fpic code, the sym
   is xxxxxxxx.plt_pic32.<callee>, and for -fPIC
   xxxxxxxx.got2.plt_pic32.<callee>.  */

static bfd_boolean
add_stub_sym (struct plt_entry *ent,
	      struct elf_link_hash_entry *h,
	      struct bfd_link_info *info)
{
  struct elf_link_hash_entry *sh;
  size_t len1, len2, len3;
  char *name;
  const char *stub;
  struct ppc_elf_link_hash_table *htab = ppc_elf_hash_table (info);

  if (info->shared)
    stub = ".plt_pic32.";
  else
    stub = ".plt_call32.";

  len1 = strlen (h->root.root.string);
  len2 = strlen (stub);
  len3 = 0;
  if (ent->sec)
    len3 = strlen (ent->sec->name);
  name = bfd_malloc (len1 + len2 + len3 + 9);
  if (name == NULL)
    return FALSE;
  sprintf (name, "%08x", (unsigned) ent->addend & 0xffffffff);
  if (ent->sec)
    memcpy (name + 8, ent->sec->name, len3);
  memcpy (name + 8 + len3, stub, len2);
  memcpy (name + 8 + len3 + len2, h->root.root.string, len1 + 1);
  sh = elf_link_hash_lookup (&htab->elf, name, TRUE, FALSE, FALSE);
  if (sh == NULL)
    return FALSE;
  if (sh->root.type == bfd_link_hash_new)
    {
      sh->root.type = bfd_link_hash_defined;
      sh->root.u.def.section = htab->glink;
      sh->root.u.def.value = ent->glink_offset;
      sh->ref_regular = 1;
      sh->def_regular = 1;
      sh->ref_regular_nonweak = 1;
      sh->forced_local = 1;
      sh->non_elf = 0;
    }
  return TRUE;
}

/* Allocate NEED contiguous space in .got, and return the offset.
   Handles allocation of the got header when crossing 32k.  */

static bfd_vma
allocate_got (struct ppc_elf_link_hash_table *htab, unsigned int need)
{
  bfd_vma where;
  unsigned int max_before_header;

  if (htab->plt_type == PLT_VXWORKS)
    {
      where = htab->got->size;
      htab->got->size += need;
    }
  else
    {
      max_before_header = htab->plt_type == PLT_NEW ? 32768 : 32764;
      if (need <= htab->got_gap)
	{
	  where = max_before_header - htab->got_gap;
	  htab->got_gap -= need;
	}
      else
	{
	  if (htab->got->size + need > max_before_header
	      && htab->got->size <= max_before_header)
	    {
	      htab->got_gap = max_before_header - htab->got->size;
	      htab->got->size = max_before_header + htab->got_header_size;
	    }
	  where = htab->got->size;
	  htab->got->size += need;
	}
    }
  return where;
}

/* Allocate space in associated reloc sections for dynamic relocs.  */

static bfd_boolean
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info = inf;
  struct ppc_elf_link_hash_entry *eh;
  struct ppc_elf_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  htab = ppc_elf_hash_table (info);
  if (htab->elf.dynamic_sections_created
      || h->type == STT_GNU_IFUNC)
    {
      struct plt_entry *ent;
      bfd_boolean doneone = FALSE;
      bfd_vma plt_offset = 0, glink_offset = 0;
      bfd_boolean dyn;

      for (ent = h->plt.plist; ent != NULL; ent = ent->next)
	if (ent->plt.refcount > 0)
	  {
	    /* Make sure this symbol is output as a dynamic symbol.  */
	    if (h->dynindx == -1
		&& !h->forced_local
		&& !h->def_regular
		&& htab->elf.dynamic_sections_created)
	      {
		if (! bfd_elf_link_record_dynamic_symbol (info, h))
		  return FALSE;
	      }

	    dyn = htab->elf.dynamic_sections_created;
	    if (info->shared
		|| h->type == STT_GNU_IFUNC
		|| WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h))
	      {
		asection *s = htab->plt;
		if (!dyn || h->dynindx == -1)
		  s = htab->iplt;

		if (htab->plt_type == PLT_NEW || !dyn || h->dynindx == -1)
		  {
		    if (!doneone)
		      {
			plt_offset = s->size;
			s->size += 4;
		      }
		    ent->plt.offset = plt_offset;

		    s = htab->glink;
		    if (!doneone || info->shared)
		      {
			glink_offset = s->size;
			s->size += GLINK_ENTRY_SIZE;
			if (h == htab->tls_get_addr
			    && !htab->params->no_tls_get_addr_opt)
			  s->size += TLS_GET_ADDR_GLINK_SIZE - GLINK_ENTRY_SIZE;
		      }
		    if (!doneone
			&& !info->shared
			&& h->def_dynamic
			&& !h->def_regular)
		      {
			h->root.u.def.section = s;
			h->root.u.def.value = glink_offset;
		      }
		    ent->glink_offset = glink_offset;

		    if (htab->params->emit_stub_syms
			&& !add_stub_sym (ent, h, info))
		      return FALSE;
		  }
		else
		  {
		    if (!doneone)
		      {
			/* If this is the first .plt entry, make room
			   for the special first entry.  */
			if (s->size == 0)
			  s->size += htab->plt_initial_entry_size;

			/* The PowerPC PLT is actually composed of two
			   parts, the first part is 2 words (for a load
			   and a jump), and then there is a remaining
			   word available at the end.  */
			plt_offset = (htab->plt_initial_entry_size
				      + (htab->plt_slot_size
					 * ((s->size
					     - htab->plt_initial_entry_size)
					    / htab->plt_entry_size)));

			/* If this symbol is not defined in a regular
			   file, and we are not generating a shared
			   library, then set the symbol to this location
			   in the .plt.  This is to avoid text
			   relocations, and is required to make
			   function pointers compare as equal between
			   the normal executable and the shared library.  */
			if (! info->shared
			    && h->def_dynamic
			    && !h->def_regular)
			  {
			    h->root.u.def.section = s;
			    h->root.u.def.value = plt_offset;
			  }

			/* Make room for this entry.  */
			s->size += htab->plt_entry_size;
			/* After the 8192nd entry, room for two entries
			   is allocated.  */
			if (htab->plt_type == PLT_OLD
			    && (s->size - htab->plt_initial_entry_size)
				/ htab->plt_entry_size
			       > PLT_NUM_SINGLE_ENTRIES)
			  s->size += htab->plt_entry_size;
		      }
		    ent->plt.offset = plt_offset;
		  }

		/* We also need to make an entry in the .rela.plt section.  */
		if (!doneone)
		  {
		    if (!htab->elf.dynamic_sections_created
			|| h->dynindx == -1)
		      htab->reliplt->size += sizeof (Elf32_External_Rela);
		    else
		      {
			htab->relplt->size += sizeof (Elf32_External_Rela);

			if (htab->plt_type == PLT_VXWORKS)
			  {
			    /* Allocate space for the unloaded relocations.  */
			    if (!info->shared
				&& htab->elf.dynamic_sections_created)
			      {
				if (ent->plt.offset
				    == (bfd_vma) htab->plt_initial_entry_size)
				  {
				    htab->srelplt2->size
				      += (sizeof (Elf32_External_Rela)
					  * VXWORKS_PLTRESOLVE_RELOCS);
				  }

				htab->srelplt2->size
				  += (sizeof (Elf32_External_Rela)
				      * VXWORKS_PLT_NON_JMP_SLOT_RELOCS);
			      }

			    /* Every PLT entry has an associated GOT entry in
			       .got.plt.  */
			    htab->sgotplt->size += 4;
			  }
		      }
		    doneone = TRUE;
		  }
	      }
	    else
	      ent->plt.offset = (bfd_vma) -1;
	  }
	else
	  ent->plt.offset = (bfd_vma) -1;

      if (!doneone)
	{
	  h->plt.plist = NULL;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.plist = NULL;
      h->needs_plt = 0;
    }

  eh = (struct ppc_elf_link_hash_entry *) h;
  if (eh->elf.got.refcount > 0)
    {
      bfd_boolean dyn;
      unsigned int need;

      /* Make sure this symbol is output as a dynamic symbol.  */
      if (eh->elf.dynindx == -1
	  && !eh->elf.forced_local
	  && eh->elf.type != STT_GNU_IFUNC
	  && htab->elf.dynamic_sections_created)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, &eh->elf))
	    return FALSE;
	}

      need = 0;
      if ((eh->tls_mask & TLS_TLS) != 0)
	{
	  if ((eh->tls_mask & TLS_LD) != 0)
	    {
	      if (!eh->elf.def_dynamic)
		/* We'll just use htab->tlsld_got.offset.  This should
		   always be the case.  It's a little odd if we have
		   a local dynamic reloc against a non-local symbol.  */
		htab->tlsld_got.refcount += 1;
	      else
		need += 8;
	    }
	  if ((eh->tls_mask & TLS_GD) != 0)
	    need += 8;
	  if ((eh->tls_mask & (TLS_TPREL | TLS_TPRELGD)) != 0)
	    need += 4;
	  if ((eh->tls_mask & TLS_DTPREL) != 0)
	    need += 4;
	}
      else
	need += 4;
      if (need == 0)
	eh->elf.got.offset = (bfd_vma) -1;
      else
	{
	  eh->elf.got.offset = allocate_got (htab, need);
	  dyn = htab->elf.dynamic_sections_created;
	  if ((info->shared
	       || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, &eh->elf))
	      && (ELF_ST_VISIBILITY (eh->elf.other) == STV_DEFAULT
		  || eh->elf.root.type != bfd_link_hash_undefweak))
	    {
	      asection *rsec = htab->relgot;

	      if (eh->elf.type == STT_GNU_IFUNC)
		rsec = htab->reliplt;
	      /* All the entries we allocated need relocs.
		 Except LD only needs one.  */
	      if ((eh->tls_mask & TLS_LD) != 0
		  && eh->elf.def_dynamic)
		need -= 4;
	      rsec->size += need * (sizeof (Elf32_External_Rela) / 4);
	    }
	}
    }
  else
    eh->elf.got.offset = (bfd_vma) -1;

  if (eh->dyn_relocs == NULL
      || !htab->elf.dynamic_sections_created)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for relocs that have become local due to symbol visibility
     changes.  */

  if (info->shared)
    {
      /* Relocs that use pc_count are those that appear on a call insn,
	 or certain REL relocs (see must_be_dyn_reloc) that can be
	 generated via assembly.  We want calls to protected symbols to
	 resolve directly to the function rather than going via the plt.
	 If people want function pointer comparisons to work as expected
	 then they should avoid writing weird assembly.  */
      if (SYMBOL_CALLS_LOCAL (info, h))
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; )
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}

      if (htab->is_vxworks)
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; )
	    {
	      if (strcmp (p->sec->output_section->name, ".tls_vars") == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}

      /* Discard relocs on undefined symbols that must be local.  */
      if (eh->dyn_relocs != NULL
	  && h->root.type == bfd_link_hash_undefined
	  && (ELF_ST_VISIBILITY (h->other) == STV_HIDDEN
	      || ELF_ST_VISIBILITY (h->other) == STV_INTERNAL))
	eh->dyn_relocs = NULL;

      /* Also discard relocs on undefined weak syms with non-default
	 visibility.  */
      if (eh->dyn_relocs != NULL
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	    eh->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && !h->forced_local
		   && !h->def_regular)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return FALSE;
	    }
	}
    }
  else if (ELIMINATE_COPY_RELOCS)
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && !h->def_regular)
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1
	      && !h->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return FALSE;
	    }

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      eh->dyn_relocs = NULL;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      if (eh->elf.type == STT_GNU_IFUNC)
	sreloc = htab->reliplt;
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return TRUE;
}

/* Set DF_TEXTREL if we find any dynamic relocs that apply to
   read-only sections.  */

static bfd_boolean
maybe_set_textrel (struct elf_link_hash_entry *h, void *info)
{
  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (readonly_dynrelocs (h))
    {
      ((struct bfd_link_info *) info)->flags |= DF_TEXTREL;

      /* Not an error, just cut short the traversal.  */
      return FALSE;
    }
  return TRUE;
}

static const unsigned char glink_eh_frame_cie[] =
{
  0, 0, 0, 16,				/* length.  */
  0, 0, 0, 0,				/* id.  */
  1,					/* CIE version.  */
  'z', 'R', 0,				/* Augmentation string.  */
  4,					/* Code alignment.  */
  0x7c,					/* Data alignment.  */
  65,					/* RA reg.  */
  1,					/* Augmentation size.  */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4,	/* FDE encoding.  */
  DW_CFA_def_cfa, 1, 0			/* def_cfa: r1 offset 0.  */
};

/* Set the sizes of the dynamic sections.  */

static bfd_boolean
ppc_elf_size_dynamic_sections (bfd *output_bfd,
			       struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab;
  asection *s;
  bfd_boolean relocs;
  bfd *ibfd;

#ifdef DEBUG
  fprintf (stderr, "ppc_elf_size_dynamic_sections called\n");
#endif

  htab = ppc_elf_hash_table (info);
  BFD_ASSERT (htab->elf.dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
	{
	  s = bfd_get_linker_section (htab->elf.dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  if (htab->plt_type == PLT_OLD)
    htab->got_header_size = 16;
  else if (htab->plt_type == PLT_NEW)
    htab->got_header_size = 12;

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      struct plt_entry **local_plt;
      struct plt_entry **end_local_plt;
      char *lgot_masks;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;

      if (!is_ppc_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct ppc_dyn_relocs *p;

	  for (p = ((struct ppc_dyn_relocs *)
		    elf_section_data (s)->local_dynrel);
	       p != NULL;
	       p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (htab->is_vxworks
		       && strcmp (p->sec->output_section->name,
				  ".tls_vars") == 0)
		{
		  /* Relocations in vxworks .tls_vars sections are
		     handled specially by the loader.  */
		}
	      else if (p->count != 0)
		{
		  asection *sreloc = elf_section_data (p->sec)->sreloc;
		  if (p->ifunc)
		    sreloc = htab->reliplt;
		  sreloc->size += p->count * sizeof (Elf32_External_Rela);
		  if ((p->sec->output_section->flags
		       & (SEC_READONLY | SEC_ALLOC))
		      == (SEC_READONLY | SEC_ALLOC))
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      local_plt = (struct plt_entry **) end_local_got;
      end_local_plt = local_plt + locsymcount;
      lgot_masks = (char *) end_local_plt;

      for (; local_got < end_local_got; ++local_got, ++lgot_masks)
	if (*local_got > 0)
	  {
	    unsigned int need = 0;
	    if ((*lgot_masks & TLS_TLS) != 0)
	      {
		if ((*lgot_masks & TLS_GD) != 0)
		  need += 8;
		if ((*lgot_masks & TLS_LD) != 0)
		  htab->tlsld_got.refcount += 1;
		if ((*lgot_masks & (TLS_TPREL | TLS_TPRELGD)) != 0)
		  need += 4;
		if ((*lgot_masks & TLS_DTPREL) != 0)
		  need += 4;
	      }
	    else
	      need += 4;
	    if (need == 0)
	      *local_got = (bfd_vma) -1;
	    else
	      {
		*local_got = allocate_got (htab, need);
		if (info->shared)
		  {
		    asection *srel = htab->relgot;
		    if ((*lgot_masks & PLT_IFUNC) != 0)
		      srel = htab->reliplt;
		    srel->size += need * (sizeof (Elf32_External_Rela) / 4);
		  }
	      }
	  }
	else
	  *local_got = (bfd_vma) -1;

      if (htab->is_vxworks)
	continue;

      /* Allocate space for calls to local STT_GNU_IFUNC syms in .iplt.  */
      for (; local_plt < end_local_plt; ++local_plt)
	{
	  struct plt_entry *ent;
	  bfd_boolean doneone = FALSE;
	  bfd_vma plt_offset = 0, glink_offset = 0;

	  for (ent = *local_plt; ent != NULL; ent = ent->next)
	    if (ent->plt.refcount > 0)
	      {
		s = htab->iplt;

		if (!doneone)
		  {
		    plt_offset = s->size;
		    s->size += 4;
		  }
		ent->plt.offset = plt_offset;

		s = htab->glink;
		if (!doneone || info->shared)
		  {
		    glink_offset = s->size;
		    s->size += GLINK_ENTRY_SIZE;
		  }
		ent->glink_offset = glink_offset;

		if (!doneone)
		  {
		    htab->reliplt->size += sizeof (Elf32_External_Rela);
		    doneone = TRUE;
		  }
	      }
	    else
	      ent->plt.offset = (bfd_vma) -1;
	}
    }

  /* Allocate space for global sym dynamic relocs.  */
  elf_link_hash_traverse (elf_hash_table (info), allocate_dynrelocs, info);

  if (htab->tlsld_got.refcount > 0)
    {
      htab->tlsld_got.offset = allocate_got (htab, 8);
      if (info->shared)
	htab->relgot->size += sizeof (Elf32_External_Rela);
    }
  else
    htab->tlsld_got.offset = (bfd_vma) -1;

  if (htab->got != NULL && htab->plt_type != PLT_VXWORKS)
    {
      unsigned int g_o_t = 32768;

      /* If we haven't allocated the header, do so now.  When we get here,
	 for old plt/got the got size will be 0 to 32764 (not allocated),
	 or 32780 to 65536 (header allocated).  For new plt/got, the
	 corresponding ranges are 0 to 32768 and 32780 to 65536.  */
      if (htab->got->size <= 32768)
	{
	  g_o_t = htab->got->size;
	  if (htab->plt_type == PLT_OLD)
	    g_o_t += 4;
	  htab->got->size += htab->got_header_size;
	}

      htab->elf.hgot->root.u.def.value = g_o_t;
    }
  if (info->shared)
    {
      struct elf_link_hash_entry *sda = htab->sdata[0].sym;

      sda->root.u.def.section = htab->elf.hgot->root.u.def.section;
      sda->root.u.def.value = htab->elf.hgot->root.u.def.value;
    }
  if (info->emitrelocations)
    {
      struct elf_link_hash_entry *sda = htab->sdata[0].sym;

      if (sda != NULL && sda->ref_regular)
	sda->root.u.def.section->flags |= SEC_KEEP;
      sda = htab->sdata[1].sym;
      if (sda != NULL && sda->ref_regular)
	sda->root.u.def.section->flags |= SEC_KEEP;
    }

  if (htab->glink != NULL
      && htab->glink->size != 0
      && htab->elf.dynamic_sections_created)
    {
      htab->glink_pltresolve = htab->glink->size;
      /* Space for the branch table.  */
      htab->glink->size += htab->glink->size / (GLINK_ENTRY_SIZE / 4) - 4;
      /* Pad out to align the start of PLTresolve.  */
      htab->glink->size += -htab->glink->size & (htab->params->ppc476_workaround
						 ? 63 : 15);
      htab->glink->size += GLINK_PLTRESOLVE;

      if (htab->params->emit_stub_syms)
	{
	  struct elf_link_hash_entry *sh;
	  sh = elf_link_hash_lookup (&htab->elf, "__glink",
				     TRUE, FALSE, FALSE);
	  if (sh == NULL)
	    return FALSE;
	  if (sh->root.type == bfd_link_hash_new)
	    {
	      sh->root.type = bfd_link_hash_defined;
	      sh->root.u.def.section = htab->glink;
	      sh->root.u.def.value = htab->glink_pltresolve;
	      sh->ref_regular = 1;
	      sh->def_regular = 1;
	      sh->ref_regular_nonweak = 1;
	      sh->forced_local = 1;
	      sh->non_elf = 0;
	    }
	  sh = elf_link_hash_lookup (&htab->elf, "__glink_PLTresolve",
				     TRUE, FALSE, FALSE);
	  if (sh == NULL)
	    return FALSE;
	  if (sh->root.type == bfd_link_hash_new)
	    {
	      sh->root.type = bfd_link_hash_defined;
	      sh->root.u.def.section = htab->glink;
	      sh->root.u.def.value = htab->glink->size - GLINK_PLTRESOLVE;
	      sh->ref_regular = 1;
	      sh->def_regular = 1;
	      sh->ref_regular_nonweak = 1;
	      sh->forced_local = 1;
	      sh->non_elf = 0;
	    }
	}
    }

  if (htab->glink != NULL
      && htab->glink->size != 0
      && htab->glink_eh_frame != NULL
      && !bfd_is_abs_section (htab->glink_eh_frame->output_section)
      && _bfd_elf_eh_frame_present (info))
    {
      s = htab->glink_eh_frame;
      s->size = sizeof (glink_eh_frame_cie) + 20;
      if (info->shared)
	{
	  s->size += 4;
	  if (htab->glink->size - GLINK_PLTRESOLVE + 8 >= 256)
	    s->size += 4;
	}
    }

  /* We've now determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = FALSE;
  for (s = htab->elf.dynobj->sections; s != NULL; s = s->next)
    {
      bfd_boolean strip_section = TRUE;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->plt
	  || s == htab->got)
	{
	  /* We'd like to strip these sections if they aren't needed, but if
	     we've exported dynamic symbols from them we must leave them.
	     It's too late to tell BFD to get rid of the symbols.  */
	  if (htab->elf.hplt != NULL)
	    strip_section = FALSE;
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (s == htab->iplt
	       || s == htab->glink
	       || s == htab->glink_eh_frame
	       || s == htab->sgotplt
	       || s == htab->sbss
	       || s == htab->dynbss
	       || s == htab->dynsbss)
	{
	  /* Strip these too.  */
	}
      else if (s == htab->sdata[0].section
	       || s == htab->sdata[1].section)
	{
	  strip_section = (s->flags & SEC_KEEP) == 0;
	}
      else if (CONST_STRNEQ (bfd_get_section_name (htab->elf.dynobj, s),
			     ".rela"))
	{
	  if (s->size != 0)
	    {
	      /* Remember whether there are any relocation sections.  */
	      relocs = TRUE;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0 && strip_section)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rela.bss and
	     .rela.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  */
      s->contents = bfd_zalloc (htab->elf.dynobj, s->size);
      if (s->contents == NULL)
	return FALSE;
    }

  if (htab->elf.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in ppc_elf_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (info->executable)
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return FALSE;
	}

      if (htab->plt != NULL && htab->plt->size != 0)
	{
	  if (!add_dynamic_entry (DT_PLTGOT, 0)
	      || !add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return FALSE;
	}

      if (htab->plt_type == PLT_NEW
	  && htab->glink != NULL
	  && htab->glink->size != 0)
	{
	  if (!add_dynamic_entry (DT_PPC_GOT, 0))
	    return FALSE;
	  if (!htab->params->no_tls_get_addr_opt
	      && htab->tls_get_addr != NULL
	      && htab->tls_get_addr->plt.plist != NULL
	      && !add_dynamic_entry (DT_PPC_OPT, PPC_OPT_TLS))
	    return FALSE;
	}

      if (relocs)
	{
	  if (!add_dynamic_entry (DT_RELA, 0)
	      || !add_dynamic_entry (DT_RELASZ, 0)
	      || !add_dynamic_entry (DT_RELAENT, sizeof (Elf32_External_Rela)))
	    return FALSE;
	}

      /* If any dynamic relocs apply to a read-only section, then we
	 need a DT_TEXTREL entry.  */
      if ((info->flags & DF_TEXTREL) == 0)
	elf_link_hash_traverse (elf_hash_table (info), maybe_set_textrel,
				info);

      if ((info->flags & DF_TEXTREL) != 0)
	{
	  if (!add_dynamic_entry (DT_TEXTREL, 0))
	    return FALSE;
	}
      if (htab->is_vxworks
	  && !elf_vxworks_add_dynamic_entries (output_bfd, info))
	return FALSE;
   }
#undef add_dynamic_entry

  if (htab->glink_eh_frame != NULL
      && htab->glink_eh_frame->contents != NULL)
    {
      unsigned char *p = htab->glink_eh_frame->contents;
      bfd_vma val;

      memcpy (p, glink_eh_frame_cie, sizeof (glink_eh_frame_cie));
      /* CIE length (rewrite in case little-endian).  */
      bfd_put_32 (htab->elf.dynobj, sizeof (glink_eh_frame_cie) - 4, p);
      p += sizeof (glink_eh_frame_cie);
      /* FDE length.  */
      val = htab->glink_eh_frame->size - 4 - sizeof (glink_eh_frame_cie);
      bfd_put_32 (htab->elf.dynobj, val, p);
      p += 4;
      /* CIE pointer.  */
      val = p - htab->glink_eh_frame->contents;
      bfd_put_32 (htab->elf.dynobj, val, p);
      p += 4;
      /* Offset to .glink.  Set later.  */
      p += 4;
      /* .glink size.  */
      bfd_put_32 (htab->elf.dynobj, htab->glink->size, p);
      p += 4;
      /* Augmentation.  */
      p += 1;

      if (info->shared
	  && htab->elf.dynamic_sections_created)
	{
	  bfd_vma adv = (htab->glink->size - GLINK_PLTRESOLVE + 8) >> 2;
	  if (adv < 64)
	    *p++ = DW_CFA_advance_loc + adv;
	  else if (adv < 256)
	    {
	      *p++ = DW_CFA_advance_loc1;
	      *p++ = adv;
	    }
	  else if (adv < 65536)
	    {
	      *p++ = DW_CFA_advance_loc2;
	      bfd_put_16 (htab->elf.dynobj, adv, p);
	      p += 2;
	    }
	  else
	    {
	      *p++ = DW_CFA_advance_loc4;
	      bfd_put_32 (htab->elf.dynobj, adv, p);
	      p += 4;
	    }
	  *p++ = DW_CFA_register;
	  *p++ = 65;
	  p++;
	  *p++ = DW_CFA_advance_loc + 4;
	  *p++ = DW_CFA_restore_extended;
	  *p++ = 65;
	}
      BFD_ASSERT ((bfd_vma) ((p + 3 - htab->glink_eh_frame->contents) & -4)
		  == htab->glink_eh_frame->size);
    }

  return TRUE;
}

/* Arrange to have _SDA_BASE_ or _SDA2_BASE_ stripped from the output
   if it looks like nothing is using them.  */

static void
maybe_strip_sdasym (bfd *output_bfd, elf_linker_section_t *lsect)
{
  struct elf_link_hash_entry *sda = lsect->sym;

  if (sda != NULL && !sda->ref_regular && sda->dynindx == -1)
    {
      asection *s;

      s = bfd_get_section_by_name (output_bfd, lsect->name);
      if (s == NULL || bfd_section_removed_from_list (output_bfd, s))
	{
	  s = bfd_get_section_by_name (output_bfd, lsect->bss_name);
	  if (s == NULL || bfd_section_removed_from_list (output_bfd, s))
	    {
	      sda->def_regular = 0;
	      /* This is somewhat magic.  See elf_link_output_extsym.  */
	      sda->ref_dynamic = 1;
	      sda->forced_local = 0;
	    }
	}
    }
}

void
ppc_elf_maybe_strip_sdata_syms (struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab = ppc_elf_hash_table (info);

  if (htab != NULL)
    {
      maybe_strip_sdasym (info->output_bfd, &htab->sdata[0]);
      maybe_strip_sdasym (info->output_bfd, &htab->sdata[1]);
    }
}


/* Return TRUE if symbol should be hashed in the `.gnu.hash' section.  */

static bfd_boolean
ppc_elf_hash_symbol (struct elf_link_hash_entry *h)
{
  if (h->plt.plist != NULL
      && !h->def_regular
      && (!h->pointer_equality_needed
	  || !h->ref_regular_nonweak))
    return FALSE;

  return _bfd_elf_hash_symbol (h);
}

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

/* Relaxation trampolines.  r12 is available for clobbering (r11, is
   used for some functions that are allowed to break the ABI).  */
static const int shared_stub_entry[] =
  {
    0x7c0802a6, /* mflr 0 */
    0x429f0005, /* bcl 20, 31, .Lxxx */
    0x7d8802a6, /* mflr 12 */
    0x3d8c0000, /* addis 12, 12, (xxx-.Lxxx)@ha */
    0x398c0000, /* addi 12, 12, (xxx-.Lxxx)@l */
    0x7c0803a6, /* mtlr 0 */
    0x7d8903a6, /* mtctr 12 */
    0x4e800420, /* bctr */
  };

static const int stub_entry[] =
  {
    0x3d800000, /* lis 12,xxx@ha */
    0x398c0000, /* addi 12,12,xxx@l */
    0x7d8903a6, /* mtctr 12 */
    0x4e800420, /* bctr */
  };

struct ppc_elf_relax_info
{
  unsigned int workaround_size;
};

/* This function implements long branch trampolines, and the ppc476
   icache bug workaround.  Any section needing trampolines or patch
   space for the workaround has its size extended so that we can
   add trampolines at the end of the section.  */

static bfd_boolean
ppc_elf_relax_section (bfd *abfd,
		       asection *isec,
		       struct bfd_link_info *link_info,
		       bfd_boolean *again)
{
  struct one_fixup
  {
    struct one_fixup *next;
    asection *tsec;
    /* Final link, can use the symbol offset.  For a
       relocatable link we use the symbol's index.  */
    bfd_vma toff;
    bfd_vma trampoff;
  };

  Elf_Internal_Shdr *symtab_hdr;
  bfd_byte *contents = NULL;
  Elf_Internal_Sym *isymbuf = NULL;
  Elf_Internal_Rela *internal_relocs = NULL;
  Elf_Internal_Rela *irel, *irelend = NULL;
  struct one_fixup *fixups = NULL;
  struct ppc_elf_relax_info *relax_info = NULL;
  unsigned changes = 0;
  bfd_boolean workaround_change;
  struct ppc_elf_link_hash_table *htab;
  bfd_size_type trampbase, trampoff, newsize;
  asection *got2;
  bfd_boolean maybe_pasted;

  *again = FALSE;

  /* No need to do anything with non-alloc or non-code sections.  */
  if ((isec->flags & SEC_ALLOC) == 0
      || (isec->flags & SEC_CODE) == 0
      || (isec->flags & SEC_LINKER_CREATED) != 0
      || isec->size < 4)
    return TRUE;

  /* We cannot represent the required PIC relocs in the output, so don't
     do anything.  The linker doesn't support mixing -shared and -r
     anyway.  */
  if (link_info->relocatable && link_info->shared)
    return TRUE;

  htab = ppc_elf_hash_table (link_info);
  if (htab == NULL)
    return TRUE;

  isec->size = (isec->size + 3) & -4;
  if (isec->rawsize == 0)
    isec->rawsize = isec->size;
  trampbase = isec->size;

  BFD_ASSERT (isec->sec_info_type == SEC_INFO_TYPE_NONE
	      || isec->sec_info_type == SEC_INFO_TYPE_TARGET);
  isec->sec_info_type = SEC_INFO_TYPE_TARGET;

  if (htab->params->ppc476_workaround)
    {
      if (elf_section_data (isec)->sec_info == NULL)
	{
	  elf_section_data (isec)->sec_info
	    = bfd_zalloc (abfd, sizeof (struct ppc_elf_relax_info));
	  if (elf_section_data (isec)->sec_info == NULL)
	    return FALSE;
	}
      relax_info = elf_section_data (isec)->sec_info;
      trampbase -= relax_info->workaround_size;
    }

  maybe_pasted = (strcmp (isec->output_section->name, ".init") == 0
		  || strcmp (isec->output_section->name, ".fini") == 0);
  /* Space for a branch around any trampolines.  */
  trampoff = trampbase;
  if (maybe_pasted && trampbase == isec->rawsize)
    trampoff += 4;

  symtab_hdr = &elf_symtab_hdr (abfd);

  if (htab->params->branch_trampolines)
    {
      /* Get a copy of the native relocations.  */
      if (isec->reloc_count != 0)
	{
	  internal_relocs = _bfd_elf_link_read_relocs (abfd, isec, NULL, NULL,
						       link_info->keep_memory);
	  if (internal_relocs == NULL)
	    goto error_return;
	}

      got2 = bfd_get_section_by_name (abfd, ".got2");

      irelend = internal_relocs + isec->reloc_count;
      for (irel = internal_relocs; irel < irelend; irel++)
	{
	  unsigned long r_type = ELF32_R_TYPE (irel->r_info);
	  bfd_vma toff, roff;
	  asection *tsec;
	  struct one_fixup *f;
	  size_t insn_offset = 0;
	  bfd_vma max_branch_offset, val;
	  bfd_byte *hit_addr;
	  unsigned long t0;
	  struct elf_link_hash_entry *h;
	  struct plt_entry **plist;
	  unsigned char sym_type;

	  switch (r_type)
	    {
	    case R_PPC_REL24:
	    case R_PPC_LOCAL24PC:
	    case R_PPC_PLTREL24:
	      max_branch_offset = 1 << 25;
	      break;

	    case R_PPC_REL14:
	    case R_PPC_REL14_BRTAKEN:
	    case R_PPC_REL14_BRNTAKEN:
	      max_branch_offset = 1 << 15;
	      break;

	    default:
	      continue;
	    }

	  /* Get the value of the symbol referred to by the reloc.  */
	  h = NULL;
	  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	    {
	      /* A local symbol.  */
	      Elf_Internal_Sym *isym;

	      /* Read this BFD's local symbols.  */
	      if (isymbuf == NULL)
		{
		  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
		  if (isymbuf == NULL)
		    isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
						    symtab_hdr->sh_info, 0,
						    NULL, NULL, NULL);
		  if (isymbuf == 0)
		    goto error_return;
		}
	      isym = isymbuf + ELF32_R_SYM (irel->r_info);
	      if (isym->st_shndx == SHN_UNDEF)
		tsec = bfd_und_section_ptr;
	      else if (isym->st_shndx == SHN_ABS)
		tsec = bfd_abs_section_ptr;
	      else if (isym->st_shndx == SHN_COMMON)
		tsec = bfd_com_section_ptr;
	      else
		tsec = bfd_section_from_elf_index (abfd, isym->st_shndx);

	      toff = isym->st_value;
	      sym_type = ELF_ST_TYPE (isym->st_info);
	    }
	  else
	    {
	      /* Global symbol handling.  */
	      unsigned long indx;

	      indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
	      h = elf_sym_hashes (abfd)[indx];

	      while (h->root.type == bfd_link_hash_indirect
		     || h->root.type == bfd_link_hash_warning)
		h = (struct elf_link_hash_entry *) h->root.u.i.link;

	      if (h->root.type == bfd_link_hash_defined
		  || h->root.type == bfd_link_hash_defweak)
		{
		  tsec = h->root.u.def.section;
		  toff = h->root.u.def.value;
		}
	      else if (h->root.type == bfd_link_hash_undefined
		       || h->root.type == bfd_link_hash_undefweak)
		{
		  tsec = bfd_und_section_ptr;
		  toff = link_info->relocatable ? indx : 0;
		}
	      else
		continue;

	      /* If this branch is to __tls_get_addr then we may later
		 optimise away the call.  We won't be needing a long-
		 branch stub in that case.  */
	      if (link_info->executable
		  && !link_info->relocatable
		  && h == htab->tls_get_addr
		  && irel != internal_relocs)
		{
		  unsigned long t_symndx = ELF32_R_SYM (irel[-1].r_info);
		  unsigned long t_rtype = ELF32_R_TYPE (irel[-1].r_info);
		  unsigned int tls_mask = 0;

		  /* The previous reloc should be one of R_PPC_TLSGD or
		     R_PPC_TLSLD, or for older object files, a reloc
		     on the __tls_get_addr arg setup insn.  Get tls
		     mask bits from the symbol on that reloc.  */
		  if (t_symndx < symtab_hdr->sh_info)
		    {
		      bfd_vma *local_got_offsets = elf_local_got_offsets (abfd);

		      if (local_got_offsets != NULL)
			{
			  struct plt_entry **local_plt = (struct plt_entry **)
			    (local_got_offsets + symtab_hdr->sh_info);
			  char *lgot_masks = (char *)
			    (local_plt + symtab_hdr->sh_info);
			  tls_mask = lgot_masks[t_symndx];
			}
		    }
		  else
		    {
		      struct elf_link_hash_entry *th
			= elf_sym_hashes (abfd)[t_symndx - symtab_hdr->sh_info];

		      while (th->root.type == bfd_link_hash_indirect
			     || th->root.type == bfd_link_hash_warning)
			th = (struct elf_link_hash_entry *) th->root.u.i.link;

		      tls_mask
			= ((struct ppc_elf_link_hash_entry *) th)->tls_mask;
		    }

		  /* The mask bits tell us if the call will be
		     optimised away.  */
		  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0
		      && (t_rtype == R_PPC_TLSGD
			  || t_rtype == R_PPC_GOT_TLSGD16
			  || t_rtype == R_PPC_GOT_TLSGD16_LO))
		    continue;
		  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0
		      && (t_rtype == R_PPC_TLSLD
			  || t_rtype == R_PPC_GOT_TLSLD16
			  || t_rtype == R_PPC_GOT_TLSLD16_LO))
		    continue;
		}

	      sym_type = h->type;
	    }

	  /* The condition here under which we call find_plt_ent must
	     match that in relocate_section.  If we call find_plt_ent here
	     but not in relocate_section, or vice versa, then the branch
	     destination used here may be incorrect.  */
	  plist = NULL;
	  if (h != NULL)
	    {
	      /* We know is_branch_reloc (r_type) is true.  */
	      if (h->type == STT_GNU_IFUNC
		  || r_type == R_PPC_PLTREL24)
		plist = &h->plt.plist;
	    }
	  else if (sym_type == STT_GNU_IFUNC
		   && elf_local_got_offsets (abfd) != NULL)
	    {
	      bfd_vma *local_got_offsets = elf_local_got_offsets (abfd);
	      struct plt_entry **local_plt = (struct plt_entry **)
		(local_got_offsets + symtab_hdr->sh_info);
	      plist = local_plt + ELF32_R_SYM (irel->r_info);
	    }
	  if (plist != NULL)
	    {
	      bfd_vma addend = 0;
	      struct plt_entry *ent;

	      if (r_type == R_PPC_PLTREL24 && link_info->shared)
		addend = irel->r_addend;
	      ent = find_plt_ent (plist, got2, addend);
	      if (ent != NULL)
		{
		  if (htab->plt_type == PLT_NEW
		      || h == NULL
		      || !htab->elf.dynamic_sections_created
		      || h->dynindx == -1)
		    {
		      tsec = htab->glink;
		      toff = ent->glink_offset;
		    }
		  else
		    {
		      tsec = htab->plt;
		      toff = ent->plt.offset;
		    }
		}
	    }

	  /* If the branch and target are in the same section, you have
	     no hope of adding stubs.  We'll error out later should the
	     branch overflow.  */
	  if (tsec == isec)
	    continue;

	  /* There probably isn't any reason to handle symbols in
	     SEC_MERGE sections;  SEC_MERGE doesn't seem a likely
	     attribute for a code section, and we are only looking at
	     branches.  However, implement it correctly here as a
	     reference for other target relax_section functions.  */
	  if (0 && tsec->sec_info_type == SEC_INFO_TYPE_MERGE)
	    {
	      /* At this stage in linking, no SEC_MERGE symbol has been
		 adjusted, so all references to such symbols need to be
		 passed through _bfd_merged_section_offset.  (Later, in
		 relocate_section, all SEC_MERGE symbols *except* for
		 section symbols have been adjusted.)

		 gas may reduce relocations against symbols in SEC_MERGE
		 sections to a relocation against the section symbol when
		 the original addend was zero.  When the reloc is against
		 a section symbol we should include the addend in the
		 offset passed to _bfd_merged_section_offset, since the
		 location of interest is the original symbol.  On the
		 other hand, an access to "sym+addend" where "sym" is not
		 a section symbol should not include the addend;  Such an
		 access is presumed to be an offset from "sym";  The
		 location of interest is just "sym".  */
	      if (sym_type == STT_SECTION)
		toff += irel->r_addend;

	      toff
		= _bfd_merged_section_offset (abfd, &tsec,
					      elf_section_data (tsec)->sec_info,
					      toff);

	      if (sym_type != STT_SECTION)
		toff += irel->r_addend;
	    }
	  /* PLTREL24 addends are special.  */
	  else if (r_type != R_PPC_PLTREL24)
	    toff += irel->r_addend;

	  /* Attempted -shared link of non-pic code loses.  */
	  if ((!link_info->relocatable
	       && tsec == bfd_und_section_ptr)
	      || tsec->output_section == NULL
	      || (tsec->owner != NULL
		  && (tsec->owner->flags & BFD_PLUGIN) != 0))
	    continue;

	  roff = irel->r_offset;

	  /* If the branch is in range, no need to do anything.  */
	  if (tsec != bfd_und_section_ptr
	      && (!link_info->relocatable
		  /* A relocatable link may have sections moved during
		     final link, so do not presume they remain in range.  */
		  || tsec->output_section == isec->output_section))
	    {
	      bfd_vma symaddr, reladdr;

	      symaddr = tsec->output_section->vma + tsec->output_offset + toff;
	      reladdr = isec->output_section->vma + isec->output_offset + roff;
	      if (symaddr - reladdr + max_branch_offset
		  < 2 * max_branch_offset)
		continue;
	    }

	  /* Look for an existing fixup to this address.  */
	  for (f = fixups; f ; f = f->next)
	    if (f->tsec == tsec && f->toff == toff)
	      break;

	  if (f == NULL)
	    {
	      size_t size;
	      unsigned long stub_rtype;

	      val = trampoff - roff;
	      if (val >= max_branch_offset)
		/* Oh dear, we can't reach a trampoline.  Don't try to add
		   one.  We'll report an error later.  */
		continue;

	      if (link_info->shared)
		{
		  size = 4 * ARRAY_SIZE (shared_stub_entry);
		  insn_offset = 12;
		}
	      else
		{
		  size = 4 * ARRAY_SIZE (stub_entry);
		  insn_offset = 0;
		}
	      stub_rtype = R_PPC_RELAX;
	      if (tsec == htab->plt
		  || tsec == htab->glink)
		{
		  stub_rtype = R_PPC_RELAX_PLT;
		  if (r_type == R_PPC_PLTREL24)
		    stub_rtype = R_PPC_RELAX_PLTREL24;
		}

	      /* Hijack the old relocation.  Since we need two
		 relocations for this use a "composite" reloc.  */
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   stub_rtype);
	      irel->r_offset = trampoff + insn_offset;
	      if (r_type == R_PPC_PLTREL24
		  && stub_rtype != R_PPC_RELAX_PLTREL24)
		irel->r_addend = 0;

	      /* Record the fixup so we don't do it again this section.  */
	      f = bfd_malloc (sizeof (*f));
	      f->next = fixups;
	      f->tsec = tsec;
	      f->toff = toff;
	      f->trampoff = trampoff;
	      fixups = f;

	      trampoff += size;
	      changes++;
	    }
	  else
	    {
	      val = f->trampoff - roff;
	      if (val >= max_branch_offset)
		continue;

	      /* Nop out the reloc, since we're finalizing things here.  */
	      irel->r_info = ELF32_R_INFO (0, R_PPC_NONE);
	    }

	  /* Get the section contents.  */
	  if (contents == NULL)
	    {
	      /* Get cached copy if it exists.  */
	      if (elf_section_data (isec)->this_hdr.contents != NULL)
		contents = elf_section_data (isec)->this_hdr.contents;
	      /* Go get them off disk.  */
	      else if (!bfd_malloc_and_get_section (abfd, isec, &contents))
		goto error_return;
	    }

	  /* Fix up the existing branch to hit the trampoline.  */
	  hit_addr = contents + roff;
	  switch (r_type)
	    {
	    case R_PPC_REL24:
	    case R_PPC_LOCAL24PC:
	    case R_PPC_PLTREL24:
	      t0 = bfd_get_32 (abfd, hit_addr);
	      t0 &= ~0x3fffffc;
	      t0 |= val & 0x3fffffc;
	      bfd_put_32 (abfd, t0, hit_addr);
	      break;

	    case R_PPC_REL14:
	    case R_PPC_REL14_BRTAKEN:
	    case R_PPC_REL14_BRNTAKEN:
	      t0 = bfd_get_32 (abfd, hit_addr);
	      t0 &= ~0xfffc;
	      t0 |= val & 0xfffc;
	      bfd_put_32 (abfd, t0, hit_addr);
	      break;
	    }
	}

      while (fixups != NULL)
	{
	  struct one_fixup *f = fixups;
	  fixups = fixups->next;
	  free (f);
	}
    }

  workaround_change = FALSE;
  newsize = trampoff;
  if (htab->params->ppc476_workaround
      && (!link_info->relocatable
	  || isec->output_section->alignment_power >= htab->params->pagesize_p2))
    {
      bfd_vma addr, end_addr;
      unsigned int crossings;
      bfd_vma pagesize = (bfd_vma) 1 << htab->params->pagesize_p2;

      addr = isec->output_section->vma + isec->output_offset;
      end_addr = addr + trampoff;
      addr &= -pagesize;
      crossings = ((end_addr & -pagesize) - addr) >> htab->params->pagesize_p2;
      if (crossings != 0)
	{
	  /* Keep space aligned, to ensure the patch code itself does
	     not cross a page.  Don't decrease size calculated on a
	     previous pass as otherwise we might never settle on a layout.  */
	  newsize = 15 - ((end_addr - 1) & 15);
	  newsize += crossings * 16;
	  if (relax_info->workaround_size < newsize)
	    {
	      relax_info->workaround_size = newsize;
	      workaround_change = TRUE;
	    }
	  /* Ensure relocate_section is called.  */
	  isec->flags |= SEC_RELOC;
	}
      newsize = trampoff + relax_info->workaround_size;
    }

  if (changes || workaround_change)
    isec->size = newsize;

  if (isymbuf != NULL
      && symtab_hdr->contents != (unsigned char *) isymbuf)
    {
      if (! link_info->keep_memory)
	free (isymbuf);
      else
	{
	  /* Cache the symbols for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) isymbuf;
	}
    }

  if (contents != NULL
      && elf_section_data (isec)->this_hdr.contents != contents)
    {
      if (!changes && !link_info->keep_memory)
	free (contents);
      else
	{
	  /* Cache the section contents for elf_link_input_bfd.  */
	  elf_section_data (isec)->this_hdr.contents = contents;
	}
    }

  if (changes != 0)
    {
      /* Append sufficient NOP relocs so we can write out relocation
	 information for the trampolines.  */
      Elf_Internal_Shdr *rel_hdr;
      Elf_Internal_Rela *new_relocs = bfd_malloc ((changes + isec->reloc_count)
						  * sizeof (*new_relocs));
      unsigned ix;

      if (!new_relocs)
	goto error_return;
      memcpy (new_relocs, internal_relocs,
	      isec->reloc_count * sizeof (*new_relocs));
      for (ix = changes; ix--;)
	{
	  irel = new_relocs + ix + isec->reloc_count;

	  irel->r_info = ELF32_R_INFO (0, R_PPC_NONE);
	}
      if (internal_relocs != elf_section_data (isec)->relocs)
	free (internal_relocs);
      elf_section_data (isec)->relocs = new_relocs;
      isec->reloc_count += changes;
      rel_hdr = _bfd_elf_single_rel_hdr (isec);
      rel_hdr->sh_size += changes * rel_hdr->sh_entsize;
    }
  else if (internal_relocs != NULL
	   && elf_section_data (isec)->relocs != internal_relocs)
    free (internal_relocs);

  *again = changes != 0 || workaround_change;
  return TRUE;

 error_return:
  while (fixups != NULL)
    {
      struct one_fixup *f = fixups;
      fixups = fixups->next;
      free (f);
    }
  if (isymbuf != NULL && (unsigned char *) isymbuf != symtab_hdr->contents)
    free (isymbuf);
  if (contents != NULL
      && elf_section_data (isec)->this_hdr.contents != contents)
    free (contents);
  if (internal_relocs != NULL
      && elf_section_data (isec)->relocs != internal_relocs)
    free (internal_relocs);
  return FALSE;
}

/* What to do when ld finds relocations against symbols defined in
   discarded sections.  */

static unsigned int
ppc_elf_action_discarded (asection *sec)
{
  if (strcmp (".fixup", sec->name) == 0)
    return 0;

  if (strcmp (".got2", sec->name) == 0)
    return 0;

  return _bfd_elf_default_action_discarded (sec);
}

/* Fill in the address for a pointer generated in a linker section.  */

static bfd_vma
elf_finish_pointer_linker_section (bfd *input_bfd,
				   elf_linker_section_t *lsect,
				   struct elf_link_hash_entry *h,
				   bfd_vma relocation,
				   const Elf_Internal_Rela *rel)
{
  elf_linker_section_pointers_t *linker_section_ptr;

  BFD_ASSERT (lsect != NULL);

  if (h != NULL)
    {
      /* Handle global symbol.  */
      struct ppc_elf_link_hash_entry *eh;

      eh = (struct ppc_elf_link_hash_entry *) h;
      BFD_ASSERT (eh->elf.def_regular);
      linker_section_ptr = eh->linker_section_pointer;
    }
  else
    {
      /* Handle local symbol.  */
      unsigned long r_symndx = ELF32_R_SYM (rel->r_info);

      BFD_ASSERT (is_ppc_elf (input_bfd));
      BFD_ASSERT (elf_local_ptr_offsets (input_bfd) != NULL);
      linker_section_ptr = elf_local_ptr_offsets (input_bfd)[r_symndx];
    }

  linker_section_ptr = elf_find_pointer_linker_section (linker_section_ptr,
							rel->r_addend,
							lsect);
  BFD_ASSERT (linker_section_ptr != NULL);

  /* Offset will always be a multiple of four, so use the bottom bit
     as a "written" flag.  */
  if ((linker_section_ptr->offset & 1) == 0)
    {
      bfd_put_32 (lsect->section->owner,
		  relocation + linker_section_ptr->addend,
		  lsect->section->contents + linker_section_ptr->offset);
      linker_section_ptr->offset += 1;
    }

  relocation = (lsect->section->output_section->vma
		+ lsect->section->output_offset
		+ linker_section_ptr->offset - 1
		- SYM_VAL (lsect->sym));

#ifdef DEBUG
  fprintf (stderr,
	   "Finish pointer in linker section %s, offset = %ld (0x%lx)\n",
	   lsect->name, (long) relocation, (long) relocation);
#endif

  return relocation;
}

#define PPC_LO(v) ((v) & 0xffff)
#define PPC_HI(v) (((v) >> 16) & 0xffff)
#define PPC_HA(v) PPC_HI ((v) + 0x8000)

static void
write_glink_stub (struct plt_entry *ent, asection *plt_sec, unsigned char *p,
		  struct bfd_link_info *info)
{
  struct ppc_elf_link_hash_table *htab = ppc_elf_hash_table (info);
  bfd *output_bfd = info->output_bfd;
  bfd_vma plt;

  plt = ((ent->plt.offset & ~1)
	 + plt_sec->output_section->vma
	 + plt_sec->output_offset);

  if (info->shared)
    {
      bfd_vma got = 0;

      if (ent->addend >= 32768)
	got = (ent->addend
	       + ent->sec->output_section->vma
	       + ent->sec->output_offset);
      else if (htab->elf.hgot != NULL)
	got = SYM_VAL (htab->elf.hgot);

      plt -= got;

      if (plt + 0x8000 < 0x10000)
	{
	  bfd_put_32 (output_bfd, LWZ_11_30 + PPC_LO (plt), p);
	  p += 4;
	  bfd_put_32 (output_bfd, MTCTR_11, p);
	  p += 4;
	  bfd_put_32 (output_bfd, BCTR, p);
	  p += 4;
	  bfd_put_32 (output_bfd, htab->params->ppc476_workaround ? BA : NOP, p);
	  p += 4;
	}
      else
	{
	  bfd_put_32 (output_bfd, ADDIS_11_30 + PPC_HA (plt), p);
	  p += 4;
	  bfd_put_32 (output_bfd, LWZ_11_11 + PPC_LO (plt), p);
	  p += 4;
	  bfd_put_32 (output_bfd, MTCTR_11, p);
	  p += 4;
	  bfd_put_32 (output_bfd, BCTR, p);
	  p += 4;
	}
    }
  else
    {
      bfd_put_32 (output_bfd, LIS_11 + PPC_HA (plt), p);
      p += 4;
      bfd_put_32 (output_bfd, LWZ_11_11 + PPC_LO (plt), p);
      p += 4;
      bfd_put_32 (output_bfd, MTCTR_11, p);
      p += 4;
      bfd_put_32 (output_bfd, BCTR, p);
      p += 4;
    }
}

/* Return true if symbol is defined statically.  */

static bfd_boolean
is_static_defined (struct elf_link_hash_entry *h)
{
  return ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	  && h->root.u.def.section != NULL
	  && h->root.u.def.section->output_section != NULL);
}

/* If INSN is an opcode that may be used with an @tls operand, return
   the transformed insn for TLS optimisation, otherwise return 0.  If
   REG is non-zero only match an insn with RB or RA equal to REG.  */

unsigned int
_bfd_elf_ppc_at_tls_transform (unsigned int insn, unsigned int reg)
{
  unsigned int rtra;

  if ((insn & (0x3f << 26)) != 31 << 26)
    return 0;

  if (reg == 0 || ((insn >> 11) & 0x1f) == reg)
    rtra = insn & ((1 << 26) - (1 << 16));
  else if (((insn >> 16) & 0x1f) == reg)
    rtra = (insn & (0x1f << 21)) | ((insn & (0x1f << 11)) << 5);
  else
    return 0;

  if ((insn & (0x3ff << 1)) == 266 << 1)
    /* add -> addi.  */
    insn = 14 << 26;
  else if ((insn & (0x1f << 1)) == 23 << 1
	   && ((insn & (0x1f << 6)) < 14 << 6
	       || ((insn & (0x1f << 6)) >= 16 << 6
		   && (insn & (0x1f << 6)) < 24 << 6)))
    /* load and store indexed -> dform.  */
    insn = (32 | ((insn >> 6) & 0x1f)) << 26;
  else if ((insn & (((0x1a << 5) | 0x1f) << 1)) == 21 << 1)
    /* ldx, ldux, stdx, stdux -> ld, ldu, std, stdu.  */
    insn = ((58 | ((insn >> 6) & 4)) << 26) | ((insn >> 6) & 1);
  else if ((insn & (((0x1f << 5) | 0x1f) << 1)) == 341 << 1)
    /* lwax -> lwa.  */
    insn = (58 << 26) | 2;
  else
    return 0;
  insn |= rtra;
  return insn;
}

/* If INSN is an opcode that may be used with an @tprel operand, return
   the transformed insn for an undefined weak symbol, ie. with the
   thread pointer REG operand removed.  Otherwise return 0.  */

unsigned int
_bfd_elf_ppc_at_tprel_transform (unsigned int insn, unsigned int reg)
{
  if ((insn & (0x1f << 16)) == reg << 16
      && ((insn & (0x3f << 26)) == 14u << 26 /* addi */
	  || (insn & (0x3f << 26)) == 15u << 26 /* addis */
	  || (insn & (0x3f << 26)) == 32u << 26 /* lwz */
	  || (insn & (0x3f << 26)) == 34u << 26 /* lbz */
	  || (insn & (0x3f << 26)) == 36u << 26 /* stw */
	  || (insn & (0x3f << 26)) == 38u << 26 /* stb */
	  || (insn & (0x3f << 26)) == 40u << 26 /* lhz */
	  || (insn & (0x3f << 26)) == 42u << 26 /* lha */
	  || (insn & (0x3f << 26)) == 44u << 26 /* sth */
	  || (insn & (0x3f << 26)) == 46u << 26 /* lmw */
	  || (insn & (0x3f << 26)) == 47u << 26 /* stmw */
	  || (insn & (0x3f << 26)) == 48u << 26 /* lfs */
	  || (insn & (0x3f << 26)) == 50u << 26 /* lfd */
	  || (insn & (0x3f << 26)) == 52u << 26 /* stfs */
	  || (insn & (0x3f << 26)) == 54u << 26 /* stfd */
	  || ((insn & (0x3f << 26)) == 58u << 26 /* lwa,ld,lmd */
	      && (insn & 3) != 1)
	  || ((insn & (0x3f << 26)) == 62u << 26 /* std, stmd */
	      && ((insn & 3) == 0 || (insn & 3) == 3))))
    {
      insn &= ~(0x1f << 16);
    }
  else if ((insn & (0x1f << 21)) == reg << 21
	   && ((insn & (0x3e << 26)) == 24u << 26 /* ori, oris */
	       || (insn & (0x3e << 26)) == 26u << 26 /* xori,xoris */
	       || (insn & (0x3e << 26)) == 28u << 26 /* andi,andis */))
    {
      insn &= ~(0x1f << 21);
      insn |= (insn & (0x1f << 16)) << 5;
      if ((insn & (0x3e << 26)) == 26 << 26 /* xori,xoris */)
	insn -= 2 >> 26;  /* convert to ori,oris */
    }
  else
    insn = 0;
  return insn;
}

static bfd_boolean
is_insn_ds_form (unsigned int insn)
{
  return ((insn & (0x3f << 26)) == 58u << 26 /* ld,ldu,lwa */
	  || (insn & (0x3f << 26)) == 62u << 26 /* std,stdu,stq */
	  || (insn & (0x3f << 26)) == 57u << 26 /* lfdp */
	  || (insn & (0x3f << 26)) == 61u << 26 /* stfdp */);
}

static bfd_boolean
is_insn_dq_form (unsigned int insn)
{
  return (insn & (0x3f << 26)) == 56u << 26; /* lq */
}

/* The RELOCATE_SECTION function is called by the ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjust the section contents as
   necessary, and (if using Rela relocs and generating a
   relocatable output file) adjusting the reloc addend as
   necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

static bfd_boolean
ppc_elf_relocate_section (bfd *output_bfd,
			  struct bfd_link_info *info,
			  bfd *input_bfd,
			  asection *input_section,
			  bfd_byte *contents,
			  Elf_Internal_Rela *relocs,
			  Elf_Internal_Sym *local_syms,
			  asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  struct ppc_elf_link_hash_table *htab;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  Elf_Internal_Rela outrel;
  asection *got2;
  bfd_vma *local_got_offsets;
  bfd_boolean ret = TRUE;
  bfd_vma d_offset = (bfd_big_endian (output_bfd) ? 2 : 0);
  bfd_boolean is_vxworks_tls;

#ifdef DEBUG
  _bfd_error_handler ("ppc_elf_relocate_section called for %B section %A, "
		      "%ld relocations%s",
		      input_bfd, input_section,
		      (long) input_section->reloc_count,
		      (info->relocatable) ? " (relocatable)" : "");
#endif

  got2 = bfd_get_section_by_name (input_bfd, ".got2");

  /* Initialize howto table if not already done.  */
  if (!ppc_elf_howto_table[R_PPC_ADDR32])
    ppc_elf_howto_init ();

  htab = ppc_elf_hash_table (info);
  local_got_offsets = elf_local_got_offsets (input_bfd);
  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  /* We have to handle relocations in vxworks .tls_vars sections
     specially, because the dynamic loader is 'weird'.  */
  is_vxworks_tls = (htab->is_vxworks && info->shared
		    && !strcmp (input_section->output_section->name,
				".tls_vars"));
  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      enum elf_ppc_reloc_type r_type;
      bfd_vma addend;
      bfd_reloc_status_type r;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      const char *sym_name;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      bfd_vma relocation;
      bfd_vma branch_bit, from;
      bfd_boolean unresolved_reloc;
      bfd_boolean warned;
      unsigned int tls_type, tls_mask, tls_gd;
      struct plt_entry **ifunc;
      struct reloc_howto_struct alt_howto;

      r_type = ELF32_R_TYPE (rel->r_info);
      sym = NULL;
      sec = NULL;
      h = NULL;
      unresolved_reloc = FALSE;
      warned = FALSE;
      r_symndx = ELF32_R_SYM (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  sym_name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym, sec);

	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  bfd_boolean ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  sym_name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	{
	  /* For relocs against symbols from removed linkonce sections,
	     or sections discarded by a linker script, we just want the
	     section contents zeroed.  Avoid any special processing.  */
	  howto = NULL;
	  if (r_type < R_PPC_max)
	    howto = ppc_elf_howto_table[r_type];
	  RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					   rel, 1, relend, howto, 0, contents);
	}

      if (info->relocatable)
	{
	  if (got2 != NULL
	      && r_type == R_PPC_PLTREL24
	      && rel->r_addend != 0)
	    {
	      /* R_PPC_PLTREL24 is rather special.  If non-zero, the
		 addend specifies the GOT pointer offset within .got2.  */
	      rel->r_addend += got2->output_offset;
	    }
	  if (r_type != R_PPC_RELAX_PLT
	      && r_type != R_PPC_RELAX_PLTREL24
	      && r_type != R_PPC_RELAX)
	    continue;
	}

      /* TLS optimizations.  Replace instruction sequences and relocs
	 based on information we collected in tls_optimize.  We edit
	 RELOCS so that --emit-relocs will output something sensible
	 for the final instruction stream.  */
      tls_mask = 0;
      tls_gd = 0;
      if (h != NULL)
	tls_mask = ((struct ppc_elf_link_hash_entry *) h)->tls_mask;
      else if (local_got_offsets != NULL)
	{
	  struct plt_entry **local_plt;
	  char *lgot_masks;
	  local_plt
	    = (struct plt_entry **) (local_got_offsets + symtab_hdr->sh_info);
	  lgot_masks = (char *) (local_plt + symtab_hdr->sh_info);
	  tls_mask = lgot_masks[r_symndx];
	}

      /* Ensure reloc mapping code below stays sane.  */
      if ((R_PPC_GOT_TLSLD16 & 3)    != (R_PPC_GOT_TLSGD16 & 3)
	  || (R_PPC_GOT_TLSLD16_LO & 3) != (R_PPC_GOT_TLSGD16_LO & 3)
	  || (R_PPC_GOT_TLSLD16_HI & 3) != (R_PPC_GOT_TLSGD16_HI & 3)
	  || (R_PPC_GOT_TLSLD16_HA & 3) != (R_PPC_GOT_TLSGD16_HA & 3)
	  || (R_PPC_GOT_TLSLD16 & 3)    != (R_PPC_GOT_TPREL16 & 3)
	  || (R_PPC_GOT_TLSLD16_LO & 3) != (R_PPC_GOT_TPREL16_LO & 3)
	  || (R_PPC_GOT_TLSLD16_HI & 3) != (R_PPC_GOT_TPREL16_HI & 3)
	  || (R_PPC_GOT_TLSLD16_HA & 3) != (R_PPC_GOT_TPREL16_HA & 3))
	abort ();
      switch (r_type)
	{
	default:
	  break;

	case R_PPC_GOT_TPREL16:
	case R_PPC_GOT_TPREL16_LO:
	  if ((tls_mask & TLS_TLS) != 0
	      && (tls_mask & TLS_TPREL) == 0)
	    {
	      bfd_vma insn;

	      insn = bfd_get_32 (output_bfd, contents + rel->r_offset - d_offset);
	      insn &= 31 << 21;
	      insn |= 0x3c020000;	/* addis 0,2,0 */
	      bfd_put_32 (output_bfd, insn, contents + rel->r_offset - d_offset);
	      r_type = R_PPC_TPREL16_HA;
	      rel->r_info = ELF32_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC_TLS:
	  if ((tls_mask & TLS_TLS) != 0
	      && (tls_mask & TLS_TPREL) == 0)
	    {
	      bfd_vma insn;

	      insn = bfd_get_32 (output_bfd, contents + rel->r_offset);
	      insn = _bfd_elf_ppc_at_tls_transform (insn, 2);
	      if (insn == 0)
		abort ();
	      bfd_put_32 (output_bfd, insn, contents + rel->r_offset);
	      r_type = R_PPC_TPREL16_LO;
	      rel->r_info = ELF32_R_INFO (r_symndx, r_type);

	      /* Was PPC_TLS which sits on insn boundary, now
		 PPC_TPREL16_LO which is at low-order half-word.  */
	      rel->r_offset += d_offset;
	    }
	  break;

	case R_PPC_GOT_TLSGD16_HI:
	case R_PPC_GOT_TLSGD16_HA:
	  tls_gd = TLS_TPRELGD;
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0)
	    goto tls_gdld_hi;
	  break;

	case R_PPC_GOT_TLSLD16_HI:
	case R_PPC_GOT_TLSLD16_HA:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0)
	    {
	    tls_gdld_hi:
	      if ((tls_mask & tls_gd) != 0)
		r_type = (((r_type - (R_PPC_GOT_TLSGD16 & 3)) & 3)
			  + R_PPC_GOT_TPREL16);
	      else
		{
		  rel->r_offset -= d_offset;
		  bfd_put_32 (output_bfd, NOP, contents + rel->r_offset);
		  r_type = R_PPC_NONE;
		}
	      rel->r_info = ELF32_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC_GOT_TLSGD16:
	case R_PPC_GOT_TLSGD16_LO:
	  tls_gd = TLS_TPRELGD;
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0)
	    goto tls_ldgd_opt;
	  break;

	case R_PPC_GOT_TLSLD16:
	case R_PPC_GOT_TLSLD16_LO:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0)
	    {
	      unsigned int insn1, insn2;
	      bfd_vma offset;

	    tls_ldgd_opt:
	      offset = (bfd_vma) -1;
	      /* If not using the newer R_PPC_TLSGD/LD to mark
		 __tls_get_addr calls, we must trust that the call
		 stays with its arg setup insns, ie. that the next
		 reloc is the __tls_get_addr call associated with
		 the current reloc.  Edit both insns.  */
	      if (input_section->has_tls_get_addr_call
		  && rel + 1 < relend
		  && branch_reloc_hash_match (input_bfd, rel + 1,
					      htab->tls_get_addr))
		offset = rel[1].r_offset;
	      /* We read the low GOT_TLS insn because we need to keep
		 the destination reg.  It may be something other than
		 the usual r3, and moved to r3 before the call by
		 intervening code.  */
	      insn1 = bfd_get_32 (output_bfd,
				  contents + rel->r_offset - d_offset);
	      if ((tls_mask & tls_gd) != 0)
		{
		  /* IE */
		  insn1 &= (0x1f << 21) | (0x1f << 16);
		  insn1 |= 32 << 26;	/* lwz */
		  if (offset != (bfd_vma) -1)
		    {
		      rel[1].r_info = ELF32_R_INFO (STN_UNDEF, R_PPC_NONE);
		      insn2 = 0x7c631214;	/* add 3,3,2 */
		      bfd_put_32 (output_bfd, insn2, contents + offset);
		    }
		  r_type = (((r_type - (R_PPC_GOT_TLSGD16 & 3)) & 3)
			    + R_PPC_GOT_TPREL16);
		  rel->r_info = ELF32_R_INFO (r_symndx, r_type);
		}
	      else
		{
		  /* LE */
		  insn1 &= 0x1f << 21;
		  insn1 |= 0x3c020000;	/* addis r,2,0 */
		  if (tls_gd == 0)
		    {
		      /* Was an LD reloc.  */
		      for (r_symndx = 0;
			   r_symndx < symtab_hdr->sh_info;
			   r_symndx++)
			if (local_sections[r_symndx] == sec)
			  break;
		      if (r_symndx >= symtab_hdr->sh_info)
			r_symndx = STN_UNDEF;
		      rel->r_addend = htab->elf.tls_sec->vma + DTP_OFFSET;
		      if (r_symndx != STN_UNDEF)
			rel->r_addend -= (local_syms[r_symndx].st_value
					  + sec->output_offset
					  + sec->output_section->vma);
		    }
		  r_type = R_PPC_TPREL16_HA;
		  rel->r_info = ELF32_R_INFO (r_symndx, r_type);
		  if (offset != (bfd_vma) -1)
		    {
		      rel[1].r_info = ELF32_R_INFO (r_symndx, R_PPC_TPREL16_LO);
		      rel[1].r_offset = offset + d_offset;
		      rel[1].r_addend = rel->r_addend;
		      insn2 = 0x38630000;	/* addi 3,3,0 */
		      bfd_put_32 (output_bfd, insn2, contents + offset);
		    }
		}
	      bfd_put_32 (output_bfd, insn1,
			  contents + rel->r_offset - d_offset);
	      if (tls_gd == 0)
		{
		  /* We changed the symbol on an LD reloc.  Start over
		     in order to get h, sym, sec etc. right.  */
		  rel--;
		  continue;
		}
	    }
	  break;

	case R_PPC_TLSGD:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0)
	    {
	      unsigned int insn2;
	      bfd_vma offset = rel->r_offset;

	      if ((tls_mask & TLS_TPRELGD) != 0)
		{
		  /* IE */
		  r_type = R_PPC_NONE;
		  insn2 = 0x7c631214;	/* add 3,3,2 */
		}
	      else
		{
		  /* LE */
		  r_type = R_PPC_TPREL16_LO;
		  rel->r_offset += d_offset;
		  insn2 = 0x38630000;	/* addi 3,3,0 */
		}
	      rel->r_info = ELF32_R_INFO (r_symndx, r_type);
	      bfd_put_32 (output_bfd, insn2, contents + offset);
	      /* Zap the reloc on the _tls_get_addr call too.  */
	      BFD_ASSERT (offset == rel[1].r_offset);
	      rel[1].r_info = ELF32_R_INFO (STN_UNDEF, R_PPC_NONE);
	    }
	  break;

	case R_PPC_TLSLD:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0)
	    {
	      unsigned int insn2;

	      for (r_symndx = 0;
		   r_symndx < symtab_hdr->sh_info;
		   r_symndx++)
		if (local_sections[r_symndx] == sec)
		  break;
	      if (r_symndx >= symtab_hdr->sh_info)
		r_symndx = STN_UNDEF;
	      rel->r_addend = htab->elf.tls_sec->vma + DTP_OFFSET;
	      if (r_symndx != STN_UNDEF)
		rel->r_addend -= (local_syms[r_symndx].st_value
				  + sec->output_offset
				  + sec->output_section->vma);

	      rel->r_info = ELF32_R_INFO (r_symndx, R_PPC_TPREL16_LO);
	      rel->r_offset += d_offset;
	      insn2 = 0x38630000;	/* addi 3,3,0 */
	      bfd_put_32 (output_bfd, insn2,
			  contents + rel->r_offset - d_offset);
	      /* Zap the reloc on the _tls_get_addr call too.  */
	      BFD_ASSERT (rel->r_offset - d_offset == rel[1].r_offset);
	      rel[1].r_info = ELF32_R_INFO (STN_UNDEF, R_PPC_NONE);
	      rel--;
	      continue;
	    }
	  break;
	}

      /* Handle other relocations that tweak non-addend part of insn.  */
      branch_bit = 0;
      switch (r_type)
	{
	default:
	  break;

	  /* Branch taken prediction relocations.  */
	case R_PPC_ADDR14_BRTAKEN:
	case R_PPC_REL14_BRTAKEN:
	  branch_bit = BRANCH_PREDICT_BIT;
	  /* Fall thru */

	  /* Branch not taken prediction relocations.  */
	case R_PPC_ADDR14_BRNTAKEN:
	case R_PPC_REL14_BRNTAKEN:
	  {
	    bfd_vma insn;

	    insn = bfd_get_32 (output_bfd, contents + rel->r_offset);
	    insn &= ~BRANCH_PREDICT_BIT;
	    insn |= branch_bit;

	    from = (rel->r_offset
		    + input_section->output_offset
		    + input_section->output_section->vma);

	    /* Invert 'y' bit if not the default.  */
	    if ((bfd_signed_vma) (relocation + rel->r_addend - from) < 0)
	      insn ^= BRANCH_PREDICT_BIT;

	    bfd_put_32 (output_bfd, insn, contents + rel->r_offset);
	    break;
	  }
	}

      ifunc = NULL;
      if (!htab->is_vxworks)
	{
	  struct plt_entry *ent;

	  if (h != NULL)
	    {
	      if (h->type == STT_GNU_IFUNC)
		ifunc = &h->plt.plist;
	    }
	  else if (local_got_offsets != NULL
		   && ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
	    {
	      struct plt_entry **local_plt;

	      local_plt = (struct plt_entry **) (local_got_offsets
						 + symtab_hdr->sh_info);
	      ifunc = local_plt + r_symndx;
	    }

	  ent = NULL;
	  if (ifunc != NULL
	      && (!info->shared
		  || is_branch_reloc (r_type)))
	    {
	      addend = 0;
	      if (r_type == R_PPC_PLTREL24 && info->shared)
		addend = rel->r_addend;
	      ent = find_plt_ent (ifunc, got2, addend);
	    }
	  if (ent != NULL)
	    {
	      if (h == NULL && (ent->plt.offset & 1) == 0)
		{
		  Elf_Internal_Rela rela;
		  bfd_byte *loc;

		  rela.r_offset = (htab->iplt->output_section->vma
				   + htab->iplt->output_offset
				   + ent->plt.offset);
		  rela.r_info = ELF32_R_INFO (0, R_PPC_IRELATIVE);
		  rela.r_addend = relocation;
		  loc = htab->reliplt->contents;
		  loc += (htab->reliplt->reloc_count++
			  * sizeof (Elf32_External_Rela));
		  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

		  ent->plt.offset |= 1;
		}
	      if (h == NULL && (ent->glink_offset & 1) == 0)
		{
		  unsigned char *p = ((unsigned char *) htab->glink->contents
				      + ent->glink_offset);
		  write_glink_stub (ent, htab->iplt, p, info);
		  ent->glink_offset |= 1;
		}

	      unresolved_reloc = FALSE;
	      if (htab->plt_type == PLT_NEW
		  || !htab->elf.dynamic_sections_created
		  || h == NULL
		  || h->dynindx == -1)
		relocation = (htab->glink->output_section->vma
			      + htab->glink->output_offset
			      + (ent->glink_offset & ~1));
	      else
		relocation = (htab->plt->output_section->vma
			      + htab->plt->output_offset
			      + ent->plt.offset);
	    }
	}

      addend = rel->r_addend;
      tls_type = 0;
      howto = NULL;
      if (r_type < R_PPC_max)
	howto = ppc_elf_howto_table[r_type];
      switch (r_type)
	{
	default:
	  info->callbacks->einfo
	    (_("%P: %B: unknown relocation type %d for symbol %s\n"),
	     input_bfd, (int) r_type, sym_name);

	  bfd_set_error (bfd_error_bad_value);
	  ret = FALSE;
	  continue;

	case R_PPC_NONE:
	case R_PPC_TLS:
	case R_PPC_TLSGD:
	case R_PPC_TLSLD:
	case R_PPC_EMB_MRKREF:
	case R_PPC_GNU_VTINHERIT:
	case R_PPC_GNU_VTENTRY:
	  continue;

	  /* GOT16 relocations.  Like an ADDR16 using the symbol's
	     address in the GOT as relocation value instead of the
	     symbol's value itself.  Also, create a GOT entry for the
	     symbol and put the symbol value there.  */
	case R_PPC_GOT_TLSGD16:
	case R_PPC_GOT_TLSGD16_LO:
	case R_PPC_GOT_TLSGD16_HI:
	case R_PPC_GOT_TLSGD16_HA:
	  tls_type = TLS_TLS | TLS_GD;
	  goto dogot;

	case R_PPC_GOT_TLSLD16:
	case R_PPC_GOT_TLSLD16_LO:
	case R_PPC_GOT_TLSLD16_HI:
	case R_PPC_GOT_TLSLD16_HA:
	  tls_type = TLS_TLS | TLS_LD;
	  goto dogot;

	case R_PPC_GOT_TPREL16:
	case R_PPC_GOT_TPREL16_LO:
	case R_PPC_GOT_TPREL16_HI:
	case R_PPC_GOT_TPREL16_HA:
	  tls_type = TLS_TLS | TLS_TPREL;
	  goto dogot;

	case R_PPC_GOT_DTPREL16:
	case R_PPC_GOT_DTPREL16_LO:
	case R_PPC_GOT_DTPREL16_HI:
	case R_PPC_GOT_DTPREL16_HA:
	  tls_type = TLS_TLS | TLS_DTPREL;
	  goto dogot;

	case R_PPC_GOT16:
	case R_PPC_GOT16_LO:
	case R_PPC_GOT16_HI:
	case R_PPC_GOT16_HA:
	  tls_mask = 0;
	dogot:
	  {
	    /* Relocation is to the entry for this symbol in the global
	       offset table.  */
	    bfd_vma off;
	    bfd_vma *offp;
	    unsigned long indx;

	    if (htab->got == NULL)
	      abort ();

	    indx = 0;
	    if (tls_type == (TLS_TLS | TLS_LD)
		&& (h == NULL
		    || !h->def_dynamic))
	      offp = &htab->tlsld_got.offset;
	    else if (h != NULL)
	      {
		bfd_boolean dyn;
		dyn = htab->elf.dynamic_sections_created;
		if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, info->shared, h)
		    || (info->shared
			&& SYMBOL_REFERENCES_LOCAL (info, h)))
		  /* This is actually a static link, or it is a
		     -Bsymbolic link and the symbol is defined
		     locally, or the symbol was forced to be local
		     because of a version file.  */
		  ;
		else
		  {
		    BFD_ASSERT (h->dynindx != -1);
		    indx = h->dynindx;
		    unresolved_reloc = FALSE;
		  }
		offp = &h->got.offset;
	      }
	    else
	      {
		if (local_got_offsets == NULL)
		  abort ();
		offp = &local_got_offsets[r_symndx];
	      }

	    /* The offset must always be a multiple of 4.  We use the
	       least significant bit to record whether we have already
	       processed this entry.  */
	    off = *offp;
	    if ((off & 1) != 0)
	      off &= ~1;
	    else
	      {
		unsigned int tls_m = (tls_mask
				      & (TLS_LD | TLS_GD | TLS_DTPREL
					 | TLS_TPREL | TLS_TPRELGD));

		if (offp == &htab->tlsld_got.offset)
		  tls_m = TLS_LD;
		else if (h == NULL
			 || !h->def_dynamic)
		  tls_m &= ~TLS_LD;

		/* We might have multiple got entries for this sym.
		   Initialize them all.  */
		do
		  {
		    int tls_ty = 0;

		    if ((tls_m & TLS_LD) != 0)
		      {
			tls_ty = TLS_TLS | TLS_LD;
			tls_m &= ~TLS_LD;
		      }
		    else if ((tls_m & TLS_GD) != 0)
		      {
			tls_ty = TLS_TLS | TLS_GD;
			tls_m &= ~TLS_GD;
		      }
		    else if ((tls_m & TLS_DTPREL) != 0)
		      {
			tls_ty = TLS_TLS | TLS_DTPREL;
			tls_m &= ~TLS_DTPREL;
		      }
		    else if ((tls_m & (TLS_TPREL | TLS_TPRELGD)) != 0)
		      {
			tls_ty = TLS_TLS | TLS_TPREL;
			tls_m = 0;
		      }

		    /* Generate relocs for the dynamic linker.  */
		    if ((info->shared || indx != 0)
			&& (offp == &htab->tlsld_got.offset
			    || h == NULL
			    || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			    || h->root.type != bfd_link_hash_undefweak))
		      {
			asection *rsec = htab->relgot;
			bfd_byte * loc;

			if (ifunc != NULL)
			  rsec = htab->reliplt;
			outrel.r_offset = (htab->got->output_section->vma
					   + htab->got->output_offset
					   + off);
			outrel.r_addend = 0;
			if (tls_ty & (TLS_LD | TLS_GD))
			  {
			    outrel.r_info = ELF32_R_INFO (indx, R_PPC_DTPMOD32);
			    if (tls_ty == (TLS_TLS | TLS_GD))
			      {
				loc = rsec->contents;
				loc += (rsec->reloc_count++
					* sizeof (Elf32_External_Rela));
				bfd_elf32_swap_reloca_out (output_bfd,
							   &outrel, loc);
				outrel.r_offset += 4;
				outrel.r_info
				  = ELF32_R_INFO (indx, R_PPC_DTPREL32);
			      }
			  }
			else if (tls_ty == (TLS_TLS | TLS_DTPREL))
			  outrel.r_info = ELF32_R_INFO (indx, R_PPC_DTPREL32);
			else if (tls_ty == (TLS_TLS | TLS_TPREL))
			  outrel.r_info = ELF32_R_INFO (indx, R_PPC_TPREL32);
			else if (indx != 0)
			  outrel.r_info = ELF32_R_INFO (indx, R_PPC_GLOB_DAT);
			else if (ifunc != NULL)
			  outrel.r_info = ELF32_R_INFO (0, R_PPC_IRELATIVE);
			else
			  outrel.r_info = ELF32_R_INFO (0, R_PPC_RELATIVE);
			if (indx == 0 && tls_ty != (TLS_TLS | TLS_LD))
			  {
			    outrel.r_addend += relocation;
			    if (tls_ty & (TLS_GD | TLS_DTPREL | TLS_TPREL))
			      {
				if (htab->elf.tls_sec == NULL)
				  outrel.r_addend = 0;
				else
				  outrel.r_addend -= htab->elf.tls_sec->vma;
			      }
			  }
			loc = rsec->contents;
			loc += (rsec->reloc_count++
				* sizeof (Elf32_External_Rela));
			bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		      }

		    /* Init the .got section contents if we're not
		       emitting a reloc.  */
		    else
		      {
			bfd_vma value = relocation;

			if (tls_ty == (TLS_TLS | TLS_LD))
			  value = 1;
			else if (tls_ty != 0)
			  {
			    if (htab->elf.tls_sec == NULL)
			      value = 0;
			    else
			      {
				value -= htab->elf.tls_sec->vma + DTP_OFFSET;
				if (tls_ty == (TLS_TLS | TLS_TPREL))
				  value += DTP_OFFSET - TP_OFFSET;
			      }

			    if (tls_ty == (TLS_TLS | TLS_GD))
			      {
				bfd_put_32 (output_bfd, value,
					    htab->got->contents + off + 4);
				value = 1;
			      }
			  }
			bfd_put_32 (output_bfd, value,
				    htab->got->contents + off);
		      }

		    off += 4;
		    if (tls_ty & (TLS_LD | TLS_GD))
		      off += 4;
		  }
		while (tls_m != 0);

		off = *offp;
		*offp = off | 1;
	      }

	    if (off >= (bfd_vma) -2)
	      abort ();

	    if ((tls_type & TLS_TLS) != 0)
	      {
		if (tls_type != (TLS_TLS | TLS_LD))
		  {
		    if ((tls_mask & TLS_LD) != 0
			&& !(h == NULL
			     || !h->def_dynamic))
		      off += 8;
		    if (tls_type != (TLS_TLS | TLS_GD))
		      {
			if ((tls_mask & TLS_GD) != 0)
			  off += 8;
			if (tls_type != (TLS_TLS | TLS_DTPREL))
			  {
			    if ((tls_mask & TLS_DTPREL) != 0)
			      off += 4;
			  }
		      }
		  }
	      }

	    relocation = (htab->got->output_section->vma
			  + htab->got->output_offset
			  + off
			  - SYM_VAL (htab->elf.hgot));

	    /* Addends on got relocations don't make much sense.
	       x+off@got is actually x@got+off, and since the got is
	       generated by a hash table traversal, the value in the
	       got at entry m+n bears little relation to the entry m.  */
	    if (addend != 0)
	      info->callbacks->einfo
		(_("%P: %H: non-zero addend on %s reloc against `%s'\n"),
		 input_bfd, input_section, rel->r_offset,
		 howto->name,
		 sym_name);
	  }
	  break;

	  /* Relocations that need no special processing.  */
	case R_PPC_LOCAL24PC:
	  /* It makes no sense to point a local relocation
	     at a symbol not in this object.  */
	  if (unresolved_reloc)
	    {
	      if (! (*info->callbacks->undefined_symbol) (info,
							  h->root.root.string,
							  input_bfd,
							  input_section,
							  rel->r_offset,
							  TRUE))
		return FALSE;
	      continue;
	    }
	  break;

	case R_PPC_DTPREL16:
	case R_PPC_DTPREL16_LO:
	case R_PPC_DTPREL16_HI:
	case R_PPC_DTPREL16_HA:
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + DTP_OFFSET;
	  break;

	  /* Relocations that may need to be propagated if this is a shared
	     object.  */
	case R_PPC_TPREL16:
	case R_PPC_TPREL16_LO:
	case R_PPC_TPREL16_HI:
	case R_PPC_TPREL16_HA:
	  if (h != NULL
	      && h->root.type == bfd_link_hash_undefweak
	      && h->dynindx == -1)
	    {
	      /* Make this relocation against an undefined weak symbol
		 resolve to zero.  This is really just a tweak, since
		 code using weak externs ought to check that they are
		 defined before using them.  */
	      bfd_byte *p = contents + rel->r_offset - d_offset;
	      unsigned int insn = bfd_get_32 (output_bfd, p);
	      insn = _bfd_elf_ppc_at_tprel_transform (insn, 2);
	      if (insn != 0)
		bfd_put_32 (output_bfd, insn, p);
	      break;
	    }
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + TP_OFFSET;
	  /* The TPREL16 relocs shouldn't really be used in shared
	     libs as they will result in DT_TEXTREL being set, but
	     support them anyway.  */
	  goto dodyn;

	case R_PPC_TPREL32:
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + TP_OFFSET;
	  goto dodyn;

	case R_PPC_DTPREL32:
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + DTP_OFFSET;
	  goto dodyn;

	case R_PPC_DTPMOD32:
	  relocation = 1;
	  addend = 0;
	  goto dodyn;

	case R_PPC_REL16:
	case R_PPC_REL16_LO:
	case R_PPC_REL16_HI:
	case R_PPC_REL16_HA:
	  break;

	case R_PPC_REL32:
	  if (h == NULL || h == htab->elf.hgot)
	    break;
	  /* fall through */

	case R_PPC_ADDR32:
	case R_PPC_ADDR16:
	case R_PPC_ADDR16_LO:
	case R_PPC_ADDR16_HI:
	case R_PPC_ADDR16_HA:
	case R_PPC_UADDR32:
	case R_PPC_UADDR16:
	  goto dodyn;

	case R_PPC_VLE_REL8:
	case R_PPC_VLE_REL15:
	case R_PPC_VLE_REL24:
	case R_PPC_REL24:
	case R_PPC_REL14:
	case R_PPC_REL14_BRTAKEN:
	case R_PPC_REL14_BRNTAKEN:
	  /* If these relocations are not to a named symbol, they can be
	     handled right here, no need to bother the dynamic linker.  */
	  if (SYMBOL_CALLS_LOCAL (info, h)
	      || h == htab->elf.hgot)
	    break;
	  /* fall through */

	case R_PPC_ADDR24:
	case R_PPC_ADDR14:
	case R_PPC_ADDR14_BRTAKEN:
	case R_PPC_ADDR14_BRNTAKEN:
	  if (h != NULL && !info->shared)
	    break;
	  /* fall through */

	dodyn:
	  if ((input_section->flags & SEC_ALLOC) == 0
	      || is_vxworks_tls)
	    break;

	  if ((info->shared
	       && !(h != NULL
		    && ((h->root.type == bfd_link_hash_undefined
			 && (ELF_ST_VISIBILITY (h->other) == STV_HIDDEN
			     || ELF_ST_VISIBILITY (h->other) == STV_INTERNAL))
			|| (h->root.type == bfd_link_hash_undefweak
			    && ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)))
	       && (must_be_dyn_reloc (info, r_type)
		   || !SYMBOL_CALLS_LOCAL (info, h)))
	      || (ELIMINATE_COPY_RELOCS
		  && !info->shared
		  && h != NULL
		  && h->dynindx != -1
		  && !h->non_got_ref
		  && !h->def_regular))
	    {
	      int skip;
	      bfd_byte *loc;
	      asection *sreloc;
#ifdef DEBUG
	      fprintf (stderr, "ppc_elf_relocate_section needs to "
		       "create relocation for %s\n",
		       (h && h->root.root.string
			? h->root.root.string : "<unknown>"));
#endif

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */
	      sreloc = elf_section_data (input_section)->sreloc;
	      if (ifunc)
		sreloc = htab->reliplt;
	      if (sreloc == NULL)
		return FALSE;

	      skip = 0;
	      outrel.r_offset = _bfd_elf_section_offset (output_bfd, info,
							 input_section,
							 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) -1
		  || outrel.r_offset == (bfd_vma) -2)
		skip = (int) outrel.r_offset;
	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      if (skip)
		memset (&outrel, 0, sizeof outrel);
	      else if ((h != NULL
			&& (h->root.type == bfd_link_hash_undefined
			    || h->root.type == bfd_link_hash_undefweak))
		       || !SYMBOL_REFERENCES_LOCAL (info, h))
		{
		  BFD_ASSERT (h->dynindx != -1);
		  unresolved_reloc = FALSE;
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  outrel.r_addend = relocation + rel->r_addend;

		  if (r_type != R_PPC_ADDR32)
		    {
		      long indx = 0;

		      if (ifunc != NULL)
			{
			  /* If we get here when building a static
			     executable, then the libc startup function
			     responsible for applying indirect function
			     relocations is going to complain about
			     the reloc type.
			     If we get here when building a dynamic
			     executable, it will be because we have
			     a text relocation.  The dynamic loader
			     will set the text segment writable and
			     non-executable to apply text relocations.
			     So we'll segfault when trying to run the
			     indirection function to resolve the reloc.  */
			  info->callbacks->einfo
			    (_("%P: %H: relocation %s for indirect "
			       "function %s unsupported\n"),
			     input_bfd, input_section, rel->r_offset,
			     howto->name,
			     sym_name);
			  ret = FALSE;
			}
		      else if (r_symndx == STN_UNDEF || bfd_is_abs_section (sec))
			;
		      else if (sec == NULL || sec->owner == NULL)
			{
			  bfd_set_error (bfd_error_bad_value);
			  ret = FALSE;
			}
		      else
			{
			  asection *osec;

			  /* We are turning this relocation into one
			     against a section symbol.  It would be
			     proper to subtract the symbol's value,
			     osec->vma, from the emitted reloc addend,
			     but ld.so expects buggy relocs.
			     FIXME: Why not always use a zero index?  */
			  osec = sec->output_section;
			  indx = elf_section_data (osec)->dynindx;
			  if (indx == 0)
			    {
			      osec = htab->elf.text_index_section;
			      indx = elf_section_data (osec)->dynindx;
			    }
			  BFD_ASSERT (indx != 0);
#ifdef DEBUG
			  if (indx == 0)
			    printf ("indx=%ld section=%s flags=%08x name=%s\n",
				    indx, osec->name, osec->flags,
				    h->root.root.string);
#endif
			}

		      outrel.r_info = ELF32_R_INFO (indx, r_type);
		    }
		  else if (ifunc != NULL)
		    outrel.r_info = ELF32_R_INFO (0, R_PPC_IRELATIVE);
		  else
		    outrel.r_info = ELF32_R_INFO (0, R_PPC_RELATIVE);
		}

	      loc = sreloc->contents;
	      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      if (skip == -1)
		continue;

	      /* This reloc will be computed at runtime.  We clear the memory
		 so that it contains predictable value.  */
	      if (! skip
		  && ((input_section->flags & SEC_ALLOC) != 0
		      || ELF32_R_TYPE (outrel.r_info) != R_PPC_RELATIVE))
		{
		  relocation = howto->pc_relative ? outrel.r_offset : 0;
		  addend = 0;
		  break;
		}
	    }
	  break;

	case R_PPC_RELAX_PLT:
	case R_PPC_RELAX_PLTREL24:
	  if (h != NULL)
	    {
	      struct plt_entry *ent;
	      bfd_vma got2_addend = 0;

	      if (r_type == R_PPC_RELAX_PLTREL24)
		{
		  if (info->shared)
		    got2_addend = addend;
		  addend = 0;
		}
	      ent = find_plt_ent (&h->plt.plist, got2, got2_addend);
	      if (htab->plt_type == PLT_NEW)
		relocation = (htab->glink->output_section->vma
			      + htab->glink->output_offset
			      + ent->glink_offset);
	      else
		relocation = (htab->plt->output_section->vma
			      + htab->plt->output_offset
			      + ent->plt.offset);
	    }
	  /* Fall thru */

	case R_PPC_RELAX:
	  {
	    const int *stub;
	    size_t size;
	    size_t insn_offset = rel->r_offset;
	    unsigned int insn;

	    if (info->shared)
	      {
		relocation -= (input_section->output_section->vma
			       + input_section->output_offset
			       + rel->r_offset - 4);
		stub = shared_stub_entry;
		bfd_put_32 (output_bfd, stub[0], contents + insn_offset - 12);
		bfd_put_32 (output_bfd, stub[1], contents + insn_offset - 8);
		bfd_put_32 (output_bfd, stub[2], contents + insn_offset - 4);
		stub += 3;
		size = ARRAY_SIZE (shared_stub_entry) - 3;
	      }
	    else
	      {
		stub = stub_entry;
		size = ARRAY_SIZE (stub_entry);
	      }

	    relocation += addend;
	    if (info->relocatable)
	      relocation = 0;

	    /* First insn is HA, second is LO.  */
	    insn = *stub++;
	    insn |= ((relocation + 0x8000) >> 16) & 0xffff;
	    bfd_put_32 (output_bfd, insn, contents + insn_offset);
	    insn_offset += 4;

	    insn = *stub++;
	    insn |= relocation & 0xffff;
	    bfd_put_32 (output_bfd, insn, contents + insn_offset);
	    insn_offset += 4;
	    size -= 2;

	    while (size != 0)
	      {
		insn = *stub++;
		--size;
		bfd_put_32 (output_bfd, insn, contents + insn_offset);
		insn_offset += 4;
	      }

	    /* Rewrite the reloc and convert one of the trailing nop
	       relocs to describe this relocation.  */
	    BFD_ASSERT (ELF32_R_TYPE (relend[-1].r_info) == R_PPC_NONE);
	    /* The relocs are at the bottom 2 bytes */
	    rel[0].r_offset += d_offset;
	    memmove (rel + 1, rel, (relend - rel - 1) * sizeof (*rel));
	    rel[0].r_info = ELF32_R_INFO (r_symndx, R_PPC_ADDR16_HA);
	    rel[1].r_offset += 4;
	    rel[1].r_info = ELF32_R_INFO (r_symndx, R_PPC_ADDR16_LO);
	    rel++;
	  }
	  continue;

	  /* Indirect .sdata relocation.  */
	case R_PPC_EMB_SDAI16:
	  BFD_ASSERT (htab->sdata[0].section != NULL);
	  if (!is_static_defined (htab->sdata[0].sym))
	    {
	      unresolved_reloc = TRUE;
	      break;
	    }
	  relocation
	    = elf_finish_pointer_linker_section (input_bfd, &htab->sdata[0],
						 h, relocation, rel);
	  addend = 0;
	  break;

	  /* Indirect .sdata2 relocation.  */
	case R_PPC_EMB_SDA2I16:
	  BFD_ASSERT (htab->sdata[1].section != NULL);
	  if (!is_static_defined (htab->sdata[1].sym))
	    {
	      unresolved_reloc = TRUE;
	      break;
	    }
	  relocation
	    = elf_finish_pointer_linker_section (input_bfd, &htab->sdata[1],
						 h, relocation, rel);
	  addend = 0;
	  break;

	  /* Handle the TOC16 reloc.  We want to use the offset within the .got
	     section, not the actual VMA.  This is appropriate when generating
	     an embedded ELF object, for which the .got section acts like the
	     AIX .toc section.  */
	case R_PPC_TOC16:			/* phony GOT16 relocations */
	  if (sec == NULL || sec->output_section == NULL)
	    {
	      unresolved_reloc = TRUE;
	      break;
	    }
	  BFD_ASSERT (strcmp (bfd_get_section_name (sec->owner, sec),
			      ".got") == 0
		      || strcmp (bfd_get_section_name (sec->owner, sec),
				 ".cgot") == 0);

	  addend -= sec->output_section->vma + sec->output_offset + 0x8000;
	  break;

	case R_PPC_PLTREL24:
	  if (h != NULL && ifunc == NULL)
	    {
	      struct plt_entry *ent = find_plt_ent (&h->plt.plist, got2,
						    info->shared ? addend : 0);
	      if (ent == NULL
		  || htab->plt == NULL)
		{
		  /* We didn't make a PLT entry for this symbol.  This
		     happens when statically linking PIC code, or when
		     using -Bsymbolic.  */
		}
	      else
		{
		  /* Relocation is to the entry for this symbol in the
		     procedure linkage table.  */
		  unresolved_reloc = FALSE;
		  if (htab->plt_type == PLT_NEW)
		    relocation = (htab->glink->output_section->vma
				  + htab->glink->output_offset
				  + ent->glink_offset);
		  else
		    relocation = (htab->plt->output_section->vma
				  + htab->plt->output_offset
				  + ent->plt.offset);
		}
	    }

	  /* R_PPC_PLTREL24 is rather special.  If non-zero, the
	     addend specifies the GOT pointer offset within .got2.
	     Don't apply it to the relocation field.  */
	  addend = 0;
	  break;

	  /* Relocate against _SDA_BASE_.  */
	case R_PPC_SDAREL16:
	  {
	    const char *name;
	    struct elf_link_hash_entry *sda = htab->sdata[0].sym;

	    if (sec == NULL
		|| sec->output_section == NULL
		|| !is_static_defined (sda))
	      {
		unresolved_reloc = TRUE;
		break;
	      }
	    addend -= SYM_VAL (sda);

	    name = bfd_get_section_name (output_bfd, sec->output_section);
	    if (!(strcmp (name, ".sdata") == 0
		  || strcmp (name, ".sbss") == 0))
	      {
		info->callbacks->einfo
		  (_("%P: %B: the target (%s) of a %s relocation is "
		     "in the wrong output section (%s)\n"),
		   input_bfd,
		   sym_name,
		   howto->name,
		   name);
	      }
	  }
	  break;

	  /* Relocate against _SDA2_BASE_.  */
	case R_PPC_EMB_SDA2REL:
	  {
	    const char *name;
	    struct elf_link_hash_entry *sda = htab->sdata[1].sym;

	    if (sec == NULL
		|| sec->output_section == NULL
		|| !is_static_defined (sda))
	      {
		unresolved_reloc = TRUE;
		break;
	      }
	    addend -= SYM_VAL (sda);

	    name = bfd_get_section_name (output_bfd, sec->output_section);
	    if (!(strcmp (name, ".sdata2") == 0
		  || strcmp (name, ".sbss2") == 0))
	      {
		info->callbacks->einfo
		  (_("%P: %B: the target (%s) of a %s relocation is "
		     "in the wrong output section (%s)\n"),
		   input_bfd,
		   sym_name,
		   howto->name,
		   name);
	      }
	  }
	  break;

	case R_PPC_VLE_LO16A:
	  relocation = relocation + addend;
	  ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
			       relocation, split16a_type);
	  continue;

	case R_PPC_VLE_LO16D:
	  relocation = relocation + addend;
	  ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
			       relocation, split16d_type);
	  continue;

	case R_PPC_VLE_HI16A:
	  relocation = (relocation + addend) >> 16;
	  ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
			       relocation, split16a_type);
	  continue;

	case R_PPC_VLE_HI16D:
	  relocation = (relocation + addend) >> 16;
	  ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
			       relocation, split16d_type);
	  continue;

	case R_PPC_VLE_HA16A:
	  relocation = (relocation + addend + 0x8000) >> 16;
	  ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
			       relocation, split16a_type);
	  continue;

	case R_PPC_VLE_HA16D:
	  relocation = (relocation + addend + 0x8000) >> 16;
	  ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
			       relocation, split16d_type);
	  continue;

	  /* Relocate against either _SDA_BASE_, _SDA2_BASE_, or 0.  */
	case R_PPC_EMB_SDA21:
	case R_PPC_VLE_SDA21:
	case R_PPC_EMB_RELSDA:
	case R_PPC_VLE_SDA21_LO:
	  {
	    const char *name;
	    int reg;
	    unsigned int insn;
	    struct elf_link_hash_entry *sda = NULL;

	    if (sec == NULL || sec->output_section == NULL)
	      {
		unresolved_reloc = TRUE;
		break;
	      }

	    name = bfd_get_section_name (output_bfd, sec->output_section);
	    if (strcmp (name, ".sdata") == 0
		|| strcmp (name, ".sbss") == 0)
	      {
		reg = 13;
		sda = htab->sdata[0].sym;
	      }
	    else if (strcmp (name, ".sdata2") == 0
		     || strcmp (name, ".sbss2") == 0)
	      {
		reg = 2;
		sda = htab->sdata[1].sym;
	      }
	    else if (strcmp (name, ".PPC.EMB.sdata0") == 0
		     || strcmp (name, ".PPC.EMB.sbss0") == 0)
	      {
		reg = 0;
	      }
	    else
	      {
		info->callbacks->einfo
		  (_("%P: %B: the target (%s) of a %s relocation is "
		     "in the wrong output section (%s)\n"),
		   input_bfd,
		   sym_name,
		   howto->name,
		   name);

		bfd_set_error (bfd_error_bad_value);
		ret = FALSE;
		continue;
	      }

	    if (sda != NULL)
	      {
		if (!is_static_defined (sda))
		  {
		    unresolved_reloc = TRUE;
		    break;
		  }
		addend -= SYM_VAL (sda);
	      }

	    insn = bfd_get_32 (output_bfd, contents + rel->r_offset);
	    if (reg == 0
		&& (r_type == R_PPC_VLE_SDA21
		    || r_type == R_PPC_VLE_SDA21_LO))
	      {
		relocation = relocation + addend;
		addend = 0;

		/* Force e_li insn, keeping RT from original insn.  */
		insn &= 0x1f << 21;
		insn |= 28u << 26;

		/* We have an li20 field, bits 17..20, 11..15, 21..31.  */
		/* Top 4 bits of value to 17..20.  */
		insn |= (relocation & 0xf0000) >> 5;
		/* Next 5 bits of the value to 11..15.  */
		insn |= (relocation & 0xf800) << 5;
		/* And the final 11 bits of the value to bits 21 to 31.  */
		insn |= relocation & 0x7ff;

		bfd_put_32 (output_bfd, insn, contents + rel->r_offset);

		if (r_type == R_PPC_VLE_SDA21
		    && ((relocation + 0x80000) & 0xffffffff) > 0x100000)
		  goto overflow;
		continue;
	      }
	    else if (r_type == R_PPC_EMB_SDA21
		     || r_type == R_PPC_VLE_SDA21
		     || r_type == R_PPC_VLE_SDA21_LO)
	      {
		/* Fill in register field.  */
		insn = (insn & ~RA_REGISTER_MASK) | (reg << RA_REGISTER_SHIFT);
	      }
	    bfd_put_32 (output_bfd, insn, contents + rel->r_offset);
	  }
	  break;

	case R_PPC_VLE_SDAREL_LO16A:
	case R_PPC_VLE_SDAREL_LO16D:
	case R_PPC_VLE_SDAREL_HI16A:
	case R_PPC_VLE_SDAREL_HI16D:
	case R_PPC_VLE_SDAREL_HA16A:
	case R_PPC_VLE_SDAREL_HA16D:
	  {
	    bfd_vma value;
	    const char *name;
	    //int reg;
	    struct elf_link_hash_entry *sda = NULL;

	    if (sec == NULL || sec->output_section == NULL)
	      {
		unresolved_reloc = TRUE;
		break;
	      }

	    name = bfd_get_section_name (output_bfd, sec->output_section);
	    if (strcmp (name, ".sdata") == 0
		|| strcmp (name, ".sbss") == 0)
	      {
		//reg = 13;
		sda = htab->sdata[0].sym;
	      }
	    else if (strcmp (name, ".sdata2") == 0
		     || strcmp (name, ".sbss2") == 0)
	      {
		//reg = 2;
		sda = htab->sdata[1].sym;
	      }
	    else
	      {
		(*_bfd_error_handler)
		  (_("%B: the target (%s) of a %s relocation is "
		     "in the wrong output section (%s)"),
		   input_bfd,
		   sym_name,
		   howto->name,
		   name);

		bfd_set_error (bfd_error_bad_value);
		ret = FALSE;
		continue;
	      }

	    if (sda != NULL)
	      {
		if (!is_static_defined (sda))
		  {
		    unresolved_reloc = TRUE;
		    break;
		  }
	      }

	    value = (sda->root.u.def.section->output_section->vma
		     + sda->root.u.def.section->output_offset
		     + addend);

	    if (r_type == R_PPC_VLE_SDAREL_LO16A)
	      ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
				   value, split16a_type);
	    else if (r_type == R_PPC_VLE_SDAREL_LO16D)
	      ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
				   value, split16d_type);
	    else if (r_type == R_PPC_VLE_SDAREL_HI16A)
	      {
		value = value >> 16;
		ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
				     value, split16a_type);
	      }
	    else if (r_type == R_PPC_VLE_SDAREL_HI16D)
	      {
		value = value >> 16;
		ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
				     value, split16d_type);
	      }
	    else if (r_type == R_PPC_VLE_SDAREL_HA16A)
	      {
		value = (value + 0x8000) >> 16;
		ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
				     value, split16a_type);
	      }
	    else if (r_type == R_PPC_VLE_SDAREL_HA16D)
	      {
		value = (value + 0x8000) >> 16;
		ppc_elf_vle_split16 (output_bfd, contents + rel->r_offset,
				     value, split16d_type);
	      }
	  }
	  continue;

	  /* Relocate against the beginning of the section.  */
	case R_PPC_SECTOFF:
	case R_PPC_SECTOFF_LO:
	case R_PPC_SECTOFF_HI:
	case R_PPC_SECTOFF_HA:
	  if (sec == NULL || sec->output_section == NULL)
	    {
	      unresolved_reloc = TRUE;
	      break;
	    }
	  addend -= sec->output_section->vma;
	  break;

	  /* Negative relocations.  */
	case R_PPC_EMB_NADDR32:
	case R_PPC_EMB_NADDR16:
	case R_PPC_EMB_NADDR16_LO:
	case R_PPC_EMB_NADDR16_HI:
	case R_PPC_EMB_NADDR16_HA:
	  addend -= 2 * relocation;
	  break;

	case R_PPC_COPY:
	case R_PPC_GLOB_DAT:
	case R_PPC_JMP_SLOT:
	case R_PPC_RELATIVE:
	case R_PPC_IRELATIVE:
	case R_PPC_PLT32:
	case R_PPC_PLTREL32:
	case R_PPC_PLT16_LO:
	case R_PPC_PLT16_HI:
	case R_PPC_PLT16_HA:
	case R_PPC_ADDR30:
	case R_PPC_EMB_RELSEC16:
	case R_PPC_EMB_RELST_LO:
	case R_PPC_EMB_RELST_HI:
	case R_PPC_EMB_RELST_HA:
	case R_PPC_EMB_BIT_FLD:
	  info->callbacks->einfo
	    (_("%P: %B: relocation %s is not yet supported for symbol %s\n"),
	     input_bfd,
	     howto->name,
	     sym_name);

	  bfd_set_error (bfd_error_invalid_operation);
	  ret = FALSE;
	  continue;
	}

      /* Do any further special processing.  */
      switch (r_type)
	{
	default:
	  break;

	case R_PPC_ADDR16_HA:
	case R_PPC_REL16_HA:
	case R_PPC_SECTOFF_HA:
	case R_PPC_TPREL16_HA:
	case R_PPC_DTPREL16_HA:
	case R_PPC_EMB_NADDR16_HA:
	case R_PPC_EMB_RELST_HA:
	  /* It's just possible that this symbol is a weak symbol
	     that's not actually defined anywhere.  In that case,
	     'sec' would be NULL, and we should leave the symbol
	     alone (it will be set to zero elsewhere in the link).  */
	  if (sec == NULL)
	    break;
	  /* Fall thru */

	case R_PPC_PLT16_HA:
	case R_PPC_GOT16_HA:
	case R_PPC_GOT_TLSGD16_HA:
	case R_PPC_GOT_TLSLD16_HA:
	case R_PPC_GOT_TPREL16_HA:
	case R_PPC_GOT_DTPREL16_HA:
	  /* Add 0x10000 if sign bit in 0:15 is set.
	     Bits 0:15 are not used.  */
	  addend += 0x8000;
	  break;

	case R_PPC_ADDR16:
	case R_PPC_ADDR16_LO:
	case R_PPC_GOT16:
	case R_PPC_GOT16_LO:
	case R_PPC_SDAREL16:
	case R_PPC_SECTOFF:
	case R_PPC_SECTOFF_LO:
	case R_PPC_DTPREL16:
	case R_PPC_DTPREL16_LO:
	case R_PPC_TPREL16:
	case R_PPC_TPREL16_LO:
	case R_PPC_GOT_TLSGD16:
	case R_PPC_GOT_TLSGD16_LO:
	case R_PPC_GOT_TLSLD16:
	case R_PPC_GOT_TLSLD16_LO:
	case R_PPC_GOT_DTPREL16:
	case R_PPC_GOT_DTPREL16_LO:
	case R_PPC_GOT_TPREL16:
	case R_PPC_GOT_TPREL16_LO:
	  {
	    /* The 32-bit ABI lacks proper relocations to deal with
	       certain 64-bit instructions.  Prevent damage to bits
	       that make up part of the insn opcode.  */
	    unsigned int insn, mask, lobit;

	    insn = bfd_get_32 (output_bfd, contents + rel->r_offset - d_offset);
	    mask = 0;
	    if (is_insn_ds_form (insn))
	      mask = 3;
	    else if (is_insn_dq_form (insn))
	      mask = 15;
	    else
	      break;
	    lobit = mask & (relocation + addend);
	    if (lobit != 0)
	      {
		addend -= lobit;
		info->callbacks->einfo
		  (_("%P: %H: error: %s against `%s' not a multiple of %u\n"),
		   input_bfd, input_section, rel->r_offset,
		   howto->name, sym_name, mask + 1);
		bfd_set_error (bfd_error_bad_value);
		ret = FALSE;
	      }
	    addend += insn & mask;
	  }
	  break;
	}

#ifdef DEBUG
      fprintf (stderr, "\ttype = %s (%d), name = %s, symbol index = %ld, "
	       "offset = %ld, addend = %ld\n",
	       howto->name,
	       (int) r_type,
	       sym_name,
	       r_symndx,
	       (long) rel->r_offset,
	       (long) addend);
#endif

      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->def_dynamic)
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      rel->r_offset) != (bfd_vma) -1)
	{
	  info->callbacks->einfo
	    (_("%P: %H: unresolvable %s relocation against symbol `%s'\n"),
	     input_bfd, input_section, rel->r_offset,
	     howto->name,
	     sym_name);
	  ret = FALSE;
	}

      /* 16-bit fields in insns mostly have signed values, but a
	 few insns have 16-bit unsigned values.  Really, we should
	 have different reloc types.  */
      if (howto->complain_on_overflow != complain_overflow_dont
	  && howto->dst_mask == 0xffff
	  && (input_section->flags & SEC_CODE) != 0)
	{
	  enum complain_overflow complain = complain_overflow_signed;

	  if ((elf_section_flags (input_section) & SHF_PPC_VLE) == 0)
	    {
	      unsigned int insn;

	      insn = bfd_get_32 (input_bfd, contents + (rel->r_offset & ~3));
	      if ((insn & (0x3f << 26)) == 10u << 26 /* cmpli */)
		complain = complain_overflow_bitfield;
	      else if ((insn & (0x3f << 26)) == 28u << 26 /* andi */
		       || (insn & (0x3f << 26)) == 24u << 26 /* ori */
		       || (insn & (0x3f << 26)) == 26u << 26 /* xori */)
		complain = complain_overflow_unsigned;
	    }
	  if (howto->complain_on_overflow != complain)
	    {
	      alt_howto = *howto;
	      alt_howto.complain_on_overflow = complain;
	      howto = &alt_howto;
	    }
	}

      r = _bfd_final_link_relocate (howto, input_bfd, input_section, contents,
				    rel->r_offset, relocation, addend);

      if (r != bfd_reloc_ok)
	{
	  if (r == bfd_reloc_overflow)
	    {
	    overflow:
	      /* On code like "if (foo) foo();" don't report overflow
		 on a branch to zero when foo is undefined.  */
	      if (!warned
		  && !(h != NULL
		       && (h->root.type == bfd_link_hash_undefweak
			   || h->root.type == bfd_link_hash_undefined)
		       && is_branch_reloc (r_type)))
		{
		  if (!((*info->callbacks->reloc_overflow)
			(info, (h ? &h->root : NULL), sym_name,
			 howto->name, rel->r_addend,
			 input_bfd, input_section, rel->r_offset)))
		    return FALSE;
		}
	    }
	  else
	    {
	      info->callbacks->einfo
		(_("%P: %H: %s reloc against `%s': error %d\n"),
		 input_bfd, input_section, rel->r_offset,
		 howto->name, sym_name, (int) r);
	      ret = FALSE;
	    }
	}
    }

#ifdef DEBUG
  fprintf (stderr, "\n");
#endif

  if (input_section->sec_info_type == SEC_INFO_TYPE_TARGET
      && input_section->size != input_section->rawsize
      && (strcmp (input_section->output_section->name, ".init") == 0
	  || strcmp (input_section->output_section->name, ".fini") == 0))
    {
      /* Branch around the trampolines.  */
      unsigned int insn = B + input_section->size - input_section->rawsize;
      bfd_put_32 (input_bfd, insn, contents + input_section->rawsize);
    }

  if (htab->params->ppc476_workaround
      && input_section->sec_info_type == SEC_INFO_TYPE_TARGET
      && (!info->relocatable
	  || (input_section->output_section->alignment_power
	      >= htab->params->pagesize_p2)))
    {
      struct ppc_elf_relax_info *relax_info;
      bfd_vma start_addr, end_addr, addr;
      bfd_vma pagesize = (bfd_vma) 1 << htab->params->pagesize_p2;

      relax_info = elf_section_data (input_section)->sec_info;
      if (relax_info->workaround_size != 0)
	{
	  bfd_byte *p;
	  unsigned int n;
	  bfd_byte fill[4];

	  bfd_put_32 (input_bfd, BA, fill);
	  p = contents + input_section->size - relax_info->workaround_size;
	  n = relax_info->workaround_size >> 2;
	  while (n--)
	    {
	      memcpy (p, fill, 4);
	      p += 4;
	    }
	}

      /* The idea is: Replace the last instruction on a page with a
	 branch to a patch area.  Put the insn there followed by a
	 branch back to the next page.  Complicated a little by
	 needing to handle moved conditional branches, and by not
	 wanting to touch data-in-text.  */

      start_addr = (input_section->output_section->vma
		    + input_section->output_offset);
      end_addr = (start_addr + input_section->size
		  - relax_info->workaround_size);
      for (addr = ((start_addr & -pagesize) + pagesize - 4);
	   addr < end_addr;
	   addr += pagesize)
	{
	  bfd_vma offset = addr - start_addr;
	  Elf_Internal_Rela *lo, *hi;
	  bfd_boolean is_data;
	  bfd_vma patch_off, patch_addr;
	  unsigned int insn;

	  /* Do we have a data reloc at this offset?  If so, leave
	     the word alone.  */
	  is_data = FALSE;
	  lo = relocs;
	  hi = relend;
	  rel = NULL;
	  while (lo < hi)
	    {
	      rel = lo + (hi - lo) / 2;
	      if (rel->r_offset < offset)
		lo = rel + 1;
	      else if (rel->r_offset > offset + 3)
		hi = rel;
	      else
		{
		  switch (ELF32_R_TYPE (rel->r_info))
		    {
		    case R_PPC_ADDR32:
		    case R_PPC_UADDR32:
		    case R_PPC_REL32:
		    case R_PPC_ADDR30:
		      is_data = TRUE;
		      break;
		    default:
		      break;
		    }
		  break;
		}
	    }
	  if (is_data)
	    continue;

	  /* Some instructions can be left alone too.  Unconditional
	     branches, except for bcctr with BO=0x14 (bctr, bctrl),
	     avoid the icache failure.

	     The problem occurs due to prefetch across a page boundary
	     where stale instructions can be fetched from the next
	     page, and the mechanism for flushing these bad
	     instructions fails under certain circumstances.  The
	     unconditional branches:
	     1) Branch: b, bl, ba, bla,
	     2) Branch Conditional: bc, bca, bcl, bcla,
	     3) Branch Conditional to Link Register: bclr, bclrl,
	     where (2) and (3) have BO=0x14 making them unconditional,
	     prevent the bad prefetch because the prefetch itself is
	     affected by these instructions.  This happens even if the
	     instruction is not executed.

	     A bctr example:
	     .
	     .	lis 9,new_page@ha
	     .	addi 9,9,new_page@l
	     .	mtctr 9
	     .	bctr
	     .	nop
	     .	nop
	     . new_page:
	     .
	     The bctr is not predicted taken due to ctr not being
	     ready, so prefetch continues on past the bctr into the
	     new page which might have stale instructions.  If they
	     fail to be flushed, then they will be executed after the
	     bctr executes.  Either of the following modifications
	     prevent the bad prefetch from happening in the first
	     place:
	     .
	     .	lis 9,new_page@ha	 lis 9,new_page@ha	
	     .	addi 9,9,new_page@l	 addi 9,9,new_page@l	
	     .	mtctr 9			 mtctr 9			
	     .	bctr			 bctr			
	     .	nop			 b somewhere_else
	     .	b somewhere_else	 nop			
	     . new_page:		new_page:		
	     .  */
	  insn = bfd_get_32 (input_bfd, contents + offset);
	  if ((insn & (0x3f << 26)) == (18u << 26)          /* b,bl,ba,bla */
	      || ((insn & (0x3f << 26)) == (16u << 26)      /* bc,bcl,bca,bcla*/
		  && (insn & (0x14 << 21)) == (0x14 << 21)) /*   with BO=0x14 */
	      || ((insn & (0x3f << 26)) == (19u << 26)
		  && (insn & (0x3ff << 1)) == (16u << 1)    /* bclr,bclrl */
		  && (insn & (0x14 << 21)) == (0x14 << 21)))/*   with BO=0x14 */
	    continue;

	  patch_addr = (start_addr + input_section->size
			- relax_info->workaround_size);
	  patch_addr = (patch_addr + 15) & -16;
	  patch_off = patch_addr - start_addr;
	  bfd_put_32 (input_bfd, B + patch_off - offset, contents + offset);

	  if (rel != NULL
	      && rel->r_offset >= offset
	      && rel->r_offset < offset + 4)
	    {
	      asection *sreloc;

	      /* If the insn we are patching had a reloc, adjust the
		 reloc r_offset so that the reloc applies to the moved
		 location.  This matters for -r and --emit-relocs.  */
	      if (rel + 1 != relend)
		{
		  Elf_Internal_Rela tmp = *rel;

		  /* Keep the relocs sorted by r_offset.  */
		  memmove (rel, rel + 1, (relend - (rel + 1)) * sizeof (*rel));
		  relend[-1] = tmp;
		}
	      relend[-1].r_offset += patch_off - offset;

	      /* Adjust REL16 addends too.  */
	      switch (ELF32_R_TYPE (relend[-1].r_info))
		{
		case R_PPC_REL16:
		case R_PPC_REL16_LO:
		case R_PPC_REL16_HI:
		case R_PPC_REL16_HA:
		  relend[-1].r_addend += patch_off - offset;
		  break;
		default:
		  break;
		}

	      /* If we are building a PIE or shared library with
		 non-PIC objects, perhaps we had a dynamic reloc too?
		 If so, the dynamic reloc must move with the insn.  */
	      sreloc = elf_section_data (input_section)->sreloc;
	      if (sreloc != NULL)
		{
		  Elf32_External_Rela *slo, *shi, *srelend;
		  bfd_vma soffset;

		  slo = (Elf32_External_Rela *) sreloc->contents;
		  shi = srelend = slo + sreloc->reloc_count;
		  soffset = (offset + input_section->output_section->vma
			     + input_section->output_offset);
		  while (slo < shi)
		    {
		      Elf32_External_Rela *srel = slo + (shi - slo) / 2;
		      bfd_elf32_swap_reloca_in (output_bfd, (bfd_byte *) srel,
						&outrel);
		      if (outrel.r_offset < soffset)
			slo = srel + 1;
		      else if (outrel.r_offset > soffset + 3)
			shi = srel;
		      else
			{
			  if (srel + 1 != srelend)
			    {
			      memmove (srel, srel + 1,
				       (srelend - (srel + 1)) * sizeof (*srel));
			      srel = srelend - 1;
			    }
			  outrel.r_offset += patch_off - offset;
			  bfd_elf32_swap_reloca_out (output_bfd, &outrel,
						     (bfd_byte *) srel);
			  break;
			}
		    }
		}
	    }
	  else
	    rel = NULL;

	  if ((insn & (0x3f << 26)) == (16u << 26) /* bc */
	      && (insn & 2) == 0 /* relative */)
	    {
	      bfd_vma delta = ((insn & 0xfffc) ^ 0x8000) - 0x8000;

	      delta += offset - patch_off;
	      if (info->relocatable && rel != NULL)
		delta = 0;
	      if (!info->relocatable && rel != NULL)
		{
		  enum elf_ppc_reloc_type r_type;

		  r_type = ELF32_R_TYPE (relend[-1].r_info);
		  if (r_type == R_PPC_REL14_BRTAKEN)
		    insn |= BRANCH_PREDICT_BIT;
		  else if (r_type == R_PPC_REL14_BRNTAKEN)
		    insn &= ~BRANCH_PREDICT_BIT;
		  else
		    BFD_ASSERT (r_type == R_PPC_REL14);

		  if ((r_type == R_PPC_REL14_BRTAKEN
		       || r_type == R_PPC_REL14_BRNTAKEN)
		      && delta + 0x8000 < 0x10000
		      && (bfd_signed_vma) delta < 0)
		    insn ^= BRANCH_PREDICT_BIT;
		}
	      if (delta + 0x8000 < 0x10000)
		{
		  bfd_put_32 (input_bfd,
			      (insn & ~0xfffc) | (delta & 0xfffc),
			      contents + patch_off);
		  patch_off += 4;
		  bfd_put_32 (input_bfd,
			      B | ((offset + 4 - patch_off) & 0x3fffffc),
			      contents + patch_off);
		  patch_off += 4;
		}
	      else
		{
		  if (rel != NULL)
		    {
		      unsigned int r_sym = ELF32_R_SYM (relend[-1].r_info);

		      relend[-1].r_offset += 8;
		      relend[-1].r_info = ELF32_R_INFO (r_sym, R_PPC_REL24);
		    }
		  bfd_put_32 (input_bfd,
			      (insn & ~0xfffc) | 8,
			      contents + patch_off);
		  patch_off += 4;
		  bfd_put_32 (input_bfd,
			      B | ((offset + 4 - patch_off) & 0x3fffffc),
			      contents + patch_off);
		  patch_off += 4;
		  bfd_put_32 (input_bfd,
			      B | ((delta - 8) & 0x3fffffc),
			      contents + patch_off);
		  patch_off += 4;
		}
	    }
	  else
	    {
	      bfd_put_32 (input_bfd, insn, contents + patch_off);
	      patch_off += 4;
	      bfd_put_32 (input_bfd,
			  B | ((offset + 4 - patch_off) & 0x3fffffc),
			  contents + patch_off);
	      patch_off += 4;
	    }
	  BFD_ASSERT (patch_off <= input_section->size);
	  relax_info->workaround_size = input_section->size - patch_off;
	}
    }

  return ret;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
ppc_elf_finish_dynamic_symbol (bfd *output_bfd,
			       struct bfd_link_info *info,
			       struct elf_link_hash_entry *h,
			       Elf_Internal_Sym *sym)
{
  struct ppc_elf_link_hash_table *htab;
  struct plt_entry *ent;
  bfd_boolean doneone;

#ifdef DEBUG
  fprintf (stderr, "ppc_elf_finish_dynamic_symbol called for %s",
	   h->root.root.string);
#endif

  htab = ppc_elf_hash_table (info);
  BFD_ASSERT (htab->elf.dynobj != NULL);

  doneone = FALSE;
  for (ent = h->plt.plist; ent != NULL; ent = ent->next)
    if (ent->plt.offset != (bfd_vma) -1)
      {
	if (!doneone)
	  {
	    Elf_Internal_Rela rela;
	    bfd_byte *loc;
	    bfd_vma reloc_index;

	    if (htab->plt_type == PLT_NEW
		|| !htab->elf.dynamic_sections_created
		|| h->dynindx == -1)
	      reloc_index = ent->plt.offset / 4;
	    else
	      {
		reloc_index = ((ent->plt.offset - htab->plt_initial_entry_size)
			       / htab->plt_slot_size);
		if (reloc_index > PLT_NUM_SINGLE_ENTRIES
		    && htab->plt_type == PLT_OLD)
		  reloc_index -= (reloc_index - PLT_NUM_SINGLE_ENTRIES) / 2;
	      }

	    /* This symbol has an entry in the procedure linkage table.
	       Set it up.  */
	    if (htab->plt_type == PLT_VXWORKS
		&& htab->elf.dynamic_sections_created
		&& h->dynindx != -1)
	      {
		bfd_vma got_offset;
		const bfd_vma *plt_entry;

		/* The first three entries in .got.plt are reserved.  */
		got_offset = (reloc_index + 3) * 4;

		/* Use the right PLT. */
		plt_entry = info->shared ? ppc_elf_vxworks_pic_plt_entry
			    : ppc_elf_vxworks_plt_entry;

		/* Fill in the .plt on VxWorks.  */
		if (info->shared)
		  {
		    bfd_put_32 (output_bfd,
				plt_entry[0] | PPC_HA (got_offset),
				htab->plt->contents + ent->plt.offset + 0);
		    bfd_put_32 (output_bfd,
				plt_entry[1] | PPC_LO (got_offset),
				htab->plt->contents + ent->plt.offset + 4);
		  }
		else
		  {
		    bfd_vma got_loc = got_offset + SYM_VAL (htab->elf.hgot);

		    bfd_put_32 (output_bfd,
				plt_entry[0] | PPC_HA (got_loc),
				htab->plt->contents + ent->plt.offset + 0);
		    bfd_put_32 (output_bfd,
				plt_entry[1] | PPC_LO (got_loc),
				htab->plt->contents + ent->plt.offset + 4);
		  }

		bfd_put_32 (output_bfd, plt_entry[2],
			    htab->plt->contents + ent->plt.offset + 8);
		bfd_put_32 (output_bfd, plt_entry[3],
			    htab->plt->contents + ent->plt.offset + 12);

		/* This instruction is an immediate load.  The value loaded is
		   the byte offset of the R_PPC_JMP_SLOT relocation from the
		   start of the .rela.plt section.  The value is stored in the
		   low-order 16 bits of the load instruction.  */
		/* NOTE: It appears that this is now an index rather than a
		   prescaled offset.  */
		bfd_put_32 (output_bfd,
			    plt_entry[4] | reloc_index,
			    htab->plt->contents + ent->plt.offset + 16);
		/* This instruction is a PC-relative branch whose target is
		   the start of the PLT section.  The address of this branch
		   instruction is 20 bytes beyond the start of this PLT entry.
		   The address is encoded in bits 6-29, inclusive.  The value
		   stored is right-shifted by two bits, permitting a 26-bit
		   offset.  */
		bfd_put_32 (output_bfd,
			    (plt_entry[5]
			     | (-(ent->plt.offset + 20) & 0x03fffffc)),
			    htab->plt->contents + ent->plt.offset + 20);
		bfd_put_32 (output_bfd, plt_entry[6],
			    htab->plt->contents + ent->plt.offset + 24);
		bfd_put_32 (output_bfd, plt_entry[7],
			    htab->plt->contents + ent->plt.offset + 28);

		/* Fill in the GOT entry corresponding to this PLT slot with
		   the address immediately after the "bctr" instruction
		   in this PLT entry.  */
		bfd_put_32 (output_bfd, (htab->plt->output_section->vma
					 + htab->plt->output_offset
					 + ent->plt.offset + 16),
			    htab->sgotplt->contents + got_offset);

		if (!info->shared)
		  {
		    /* Fill in a couple of entries in .rela.plt.unloaded.  */
		    loc = htab->srelplt2->contents
		      + ((VXWORKS_PLTRESOLVE_RELOCS + reloc_index
			  * VXWORKS_PLT_NON_JMP_SLOT_RELOCS)
			 * sizeof (Elf32_External_Rela));

		    /* Provide the @ha relocation for the first instruction.  */
		    rela.r_offset = (htab->plt->output_section->vma
				     + htab->plt->output_offset
				     + ent->plt.offset + 2);
		    rela.r_info = ELF32_R_INFO (htab->elf.hgot->indx,
						R_PPC_ADDR16_HA);
		    rela.r_addend = got_offset;
		    bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
		    loc += sizeof (Elf32_External_Rela);

		    /* Provide the @l relocation for the second instruction.  */
		    rela.r_offset = (htab->plt->output_section->vma
				     + htab->plt->output_offset
				     + ent->plt.offset + 6);
		    rela.r_info = ELF32_R_INFO (htab->elf.hgot->indx,
						R_PPC_ADDR16_LO);
		    rela.r_addend = got_offset;
		    bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
		    loc += sizeof (Elf32_External_Rela);

		    /* Provide a relocation for the GOT entry corresponding to this
		       PLT slot.  Point it at the middle of the .plt entry.  */
		    rela.r_offset = (htab->sgotplt->output_section->vma
				     + htab->sgotplt->output_offset
				     + got_offset);
		    rela.r_info = ELF32_R_INFO (htab->elf.hplt->indx,
						R_PPC_ADDR32);
		    rela.r_addend = ent->plt.offset + 16;
		    bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
		  }

		/* VxWorks uses non-standard semantics for R_PPC_JMP_SLOT.
		   In particular, the offset for the relocation is not the
		   address of the PLT entry for this function, as specified
		   by the ABI.  Instead, the offset is set to the address of
		   the GOT slot for this function.  See EABI 4.4.4.1.  */
		rela.r_offset = (htab->sgotplt->output_section->vma
				 + htab->sgotplt->output_offset
				 + got_offset);

	      }
	    else
	      {
		asection *splt = htab->plt;
		if (!htab->elf.dynamic_sections_created
		    || h->dynindx == -1)
		  splt = htab->iplt;

		rela.r_offset = (splt->output_section->vma
				 + splt->output_offset
				 + ent->plt.offset);
		if (htab->plt_type == PLT_OLD
		    || !htab->elf.dynamic_sections_created
		    || h->dynindx == -1)
		  {
		    /* We don't need to fill in the .plt.  The ppc dynamic
		       linker will fill it in.  */
		  }
		else
		  {
		    bfd_vma val = (htab->glink_pltresolve + ent->plt.offset
				   + htab->glink->output_section->vma
				   + htab->glink->output_offset);
		    bfd_put_32 (output_bfd, val,
				splt->contents + ent->plt.offset);
		  }
	      }

	    /* Fill in the entry in the .rela.plt section.  */
	    rela.r_addend = 0;
	    if (!htab->elf.dynamic_sections_created
		|| h->dynindx == -1)
	      {
		BFD_ASSERT (h->type == STT_GNU_IFUNC
			    && h->def_regular
			    && (h->root.type == bfd_link_hash_defined
				|| h->root.type == bfd_link_hash_defweak));
		rela.r_info = ELF32_R_INFO (0, R_PPC_IRELATIVE);
		rela.r_addend = SYM_VAL (h);
	      }
	    else
	      rela.r_info = ELF32_R_INFO (h->dynindx, R_PPC_JMP_SLOT);

	    if (!htab->elf.dynamic_sections_created
		|| h->dynindx == -1)
	      loc = (htab->reliplt->contents
		     + (htab->reliplt->reloc_count++
			* sizeof (Elf32_External_Rela)));
	    else
	      loc = (htab->relplt->contents
		     + reloc_index * sizeof (Elf32_External_Rela));
	    bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

	    if (!h->def_regular)
	      {
		/* Mark the symbol as undefined, rather than as
		   defined in the .plt section.  Leave the value if
		   there were any relocations where pointer equality
		   matters (this is a clue for the dynamic linker, to
		   make function pointer comparisons work between an
		   application and shared library), otherwise set it
		   to zero.  */
		sym->st_shndx = SHN_UNDEF;
		if (!h->pointer_equality_needed)
		  sym->st_value = 0;
		else if (!h->ref_regular_nonweak)
		  {
		    /* This breaks function pointer comparisons, but
		       that is better than breaking tests for a NULL
		       function pointer.  */
		    sym->st_value = 0;
		  }
	      }
	    else if (h->type == STT_GNU_IFUNC
		     && !info->shared)
	      {
		/* Set the value of ifunc symbols in a non-pie
		   executable to the glink entry.  This is to avoid
		   text relocations.  We can't do this for ifunc in
		   allocate_dynrelocs, as we do for normal dynamic
		   function symbols with plt entries, because we need
		   to keep the original value around for the ifunc
		   relocation.  */
		sym->st_shndx = (_bfd_elf_section_from_bfd_section
				 (output_bfd, htab->glink->output_section));
		sym->st_value = (ent->glink_offset
				 + htab->glink->output_offset
				 + htab->glink->output_section->vma);
	      }
	    doneone = TRUE;
	  }

	if (htab->plt_type == PLT_NEW
	    || !htab->elf.dynamic_sections_created
	    || h->dynindx == -1)
	  {
	    unsigned char *p;
	    asection *splt = htab->plt;
	    if (!htab->elf.dynamic_sections_created
		|| h->dynindx == -1)
	      splt = htab->iplt;

	    p = (unsigned char *) htab->glink->contents + ent->glink_offset;

	    if (h == htab->tls_get_addr && !htab->params->no_tls_get_addr_opt)
	      {
		bfd_put_32 (output_bfd, LWZ_11_3, p);
		p += 4;
		bfd_put_32 (output_bfd, LWZ_12_3 + 4, p);
		p += 4;
		bfd_put_32 (output_bfd, MR_0_3, p);
		p += 4;
		bfd_put_32 (output_bfd, CMPWI_11_0, p);
		p += 4;
		bfd_put_32 (output_bfd, ADD_3_12_2, p);
		p += 4;
		bfd_put_32 (output_bfd, BEQLR, p);
		p += 4;
		bfd_put_32 (output_bfd, MR_3_0, p);
		p += 4;
		bfd_put_32 (output_bfd, NOP, p);
		p += 4;
	      }

	    write_glink_stub (ent, splt, p, info);

	    if (!info->shared)
	      /* We only need one non-PIC glink stub.  */
	      break;
	  }
	else
	  break;
      }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbols needs a copy reloc.  Set it up.  */

#ifdef DEBUG
      fprintf (stderr, ", copy");
#endif

      BFD_ASSERT (h->dynindx != -1);

      if (ppc_elf_hash_entry (h)->has_sda_refs)
	s = htab->relsbss;
      else
	s = htab->relbss;
      BFD_ASSERT (s != NULL);

      rela.r_offset = SYM_VAL (h);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_PPC_COPY);
      rela.r_addend = 0;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

#ifdef DEBUG
  fprintf (stderr, "\n");
#endif

  return TRUE;
}

static enum elf_reloc_type_class
ppc_elf_reloc_type_class (const struct bfd_link_info *info,
			  const asection *rel_sec,
			  const Elf_Internal_Rela *rela)
{
  struct ppc_elf_link_hash_table *htab = ppc_elf_hash_table (info);

  if (rel_sec == htab->reliplt)
    return reloc_class_ifunc;

  switch (ELF32_R_TYPE (rela->r_info))
    {
    case R_PPC_RELATIVE:
      return reloc_class_relative;
    case R_PPC_JMP_SLOT:
      return reloc_class_plt;
    case R_PPC_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bfd_boolean
ppc_elf_finish_dynamic_sections (bfd *output_bfd,
				 struct bfd_link_info *info)
{
  asection *sdyn;
  asection *splt;
  struct ppc_elf_link_hash_table *htab;
  bfd_vma got;
  bfd *dynobj;
  bfd_boolean ret = TRUE;

#ifdef DEBUG
  fprintf (stderr, "ppc_elf_finish_dynamic_sections called\n");
#endif

  htab = ppc_elf_hash_table (info);
  dynobj = elf_hash_table (info)->dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");
  if (htab->is_vxworks)
    splt = bfd_get_linker_section (dynobj, ".plt");
  else
    splt = NULL;

  got = 0;
  if (htab->elf.hgot != NULL)
    got = SYM_VAL (htab->elf.hgot);

  if (htab->elf.dynamic_sections_created)
    {
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (htab->plt != NULL && sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    case DT_PLTGOT:
	      if (htab->is_vxworks)
		s = htab->sgotplt;
	      else
		s = htab->plt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_PLTRELSZ:
	      dyn.d_un.d_val = htab->relplt->size;
	      break;

	    case DT_JMPREL:
	      s = htab->relplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_PPC_GOT:
	      dyn.d_un.d_ptr = got;
	      break;

	    case DT_RELASZ:
	      if (htab->is_vxworks)
		{
		  if (htab->relplt)
		    dyn.d_un.d_ptr -= htab->relplt->size;
		  break;
		}
	      continue;

	    default:
	      if (htab->is_vxworks
		  && elf_vxworks_finish_dynamic_entry (output_bfd, &dyn))
		break;
	      continue;
	    }

	  bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	}
    }

  if (htab->got != NULL)
    {
      if (htab->elf.hgot->root.u.def.section == htab->got
	  || htab->elf.hgot->root.u.def.section == htab->sgotplt)
	{
	  unsigned char *p = htab->elf.hgot->root.u.def.section->contents;

	  p += htab->elf.hgot->root.u.def.value;
	  if (htab->plt_type == PLT_OLD)
	    {
	      /* Add a blrl instruction at _GLOBAL_OFFSET_TABLE_-4
		 so that a function can easily find the address of
		 _GLOBAL_OFFSET_TABLE_.  */
	      BFD_ASSERT (htab->elf.hgot->root.u.def.value - 4
			  < htab->elf.hgot->root.u.def.section->size);
	      bfd_put_32 (output_bfd, 0x4e800021, p - 4);
	    }

	  if (sdyn != NULL)
	    {
	      bfd_vma val = sdyn->output_section->vma + sdyn->output_offset;
	      BFD_ASSERT (htab->elf.hgot->root.u.def.value
			  < htab->elf.hgot->root.u.def.section->size);
	      bfd_put_32 (output_bfd, val, p);
	    }
	}
      else
	{
	  info->callbacks->einfo (_("%P: %s not defined in linker created %s\n"),
				  htab->elf.hgot->root.root.string,
				  (htab->sgotplt != NULL
				   ? htab->sgotplt->name : htab->got->name));
	  bfd_set_error (bfd_error_bad_value);
	  ret = FALSE;
	}

      elf_section_data (htab->got->output_section)->this_hdr.sh_entsize = 4;
    }

  /* Fill in the first entry in the VxWorks procedure linkage table.  */
  if (splt && splt->size > 0)
    {
      /* Use the right PLT. */
      const bfd_vma *plt_entry = (info->shared
				  ? ppc_elf_vxworks_pic_plt0_entry
				  : ppc_elf_vxworks_plt0_entry);

      if (!info->shared)
	{
	  bfd_vma got_value = SYM_VAL (htab->elf.hgot);

	  bfd_put_32 (output_bfd, plt_entry[0] | PPC_HA (got_value),
		      splt->contents +  0);
	  bfd_put_32 (output_bfd, plt_entry[1] | PPC_LO (got_value),
		      splt->contents +  4);
	}
      else
	{
	  bfd_put_32 (output_bfd, plt_entry[0], splt->contents +  0);
	  bfd_put_32 (output_bfd, plt_entry[1], splt->contents +  4);
	}
      bfd_put_32 (output_bfd, plt_entry[2], splt->contents +  8);
      bfd_put_32 (output_bfd, plt_entry[3], splt->contents + 12);
      bfd_put_32 (output_bfd, plt_entry[4], splt->contents + 16);
      bfd_put_32 (output_bfd, plt_entry[5], splt->contents + 20);
      bfd_put_32 (output_bfd, plt_entry[6], splt->contents + 24);
      bfd_put_32 (output_bfd, plt_entry[7], splt->contents + 28);

      if (! info->shared)
	{
	  Elf_Internal_Rela rela;
	  bfd_byte *loc;

	  loc = htab->srelplt2->contents;

	  /* Output the @ha relocation for the first instruction.  */
	  rela.r_offset = (htab->plt->output_section->vma
			   + htab->plt->output_offset
			   + 2);
	  rela.r_info = ELF32_R_INFO (htab->elf.hgot->indx, R_PPC_ADDR16_HA);
	  rela.r_addend = 0;
	  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
	  loc += sizeof (Elf32_External_Rela);

	  /* Output the @l relocation for the second instruction.  */
	  rela.r_offset = (htab->plt->output_section->vma
			   + htab->plt->output_offset
			   + 6);
	  rela.r_info = ELF32_R_INFO (htab->elf.hgot->indx, R_PPC_ADDR16_LO);
	  rela.r_addend = 0;
	  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
	  loc += sizeof (Elf32_External_Rela);

	  /* Fix up the remaining relocations.  They may have the wrong
	     symbol index for _G_O_T_ or _P_L_T_ depending on the order
	     in which symbols were output.  */
	  while (loc < htab->srelplt2->contents + htab->srelplt2->size)
	    {
	      Elf_Internal_Rela rel;

	      bfd_elf32_swap_reloc_in (output_bfd, loc, &rel);
	      rel.r_info = ELF32_R_INFO (htab->elf.hgot->indx, R_PPC_ADDR16_HA);
	      bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);
	      loc += sizeof (Elf32_External_Rela);

	      bfd_elf32_swap_reloc_in (output_bfd, loc, &rel);
	      rel.r_info = ELF32_R_INFO (htab->elf.hgot->indx, R_PPC_ADDR16_LO);
	      bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);
	      loc += sizeof (Elf32_External_Rela);

	      bfd_elf32_swap_reloc_in (output_bfd, loc, &rel);
	      rel.r_info = ELF32_R_INFO (htab->elf.hplt->indx, R_PPC_ADDR32);
	      bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);
	      loc += sizeof (Elf32_External_Rela);
	    }
	}
    }

  if (htab->glink != NULL
      && htab->glink->contents != NULL
      && htab->elf.dynamic_sections_created)
    {
      unsigned char *p;
      unsigned char *endp;
      bfd_vma res0;
      unsigned int i;

      /*
       * PIC glink code is the following:
       *
       * # ith PLT code stub.
       *   addis 11,30,(plt+(i-1)*4-got)@ha
       *   lwz 11,(plt+(i-1)*4-got)@l(11)
       *   mtctr 11
       *   bctr
       *
       * # A table of branches, one for each plt entry.
       * # The idea is that the plt call stub loads ctr and r11 with these
       * # addresses, so (r11 - res_0) gives the plt index * 4.
       * res_0:	b PLTresolve
       * res_1:	b PLTresolve
       * .
       * # Some number of entries towards the end can be nops
       * res_n_m3: nop
       * res_n_m2: nop
       * res_n_m1:
       *
       * PLTresolve:
       *    addis 11,11,(1f-res_0)@ha
       *    mflr 0
       *    bcl 20,31,1f
       * 1: addi 11,11,(1b-res_0)@l
       *    mflr 12
       *    mtlr 0
       *    sub 11,11,12                # r11 = index * 4
       *    addis 12,12,(got+4-1b)@ha
       *    lwz 0,(got+4-1b)@l(12)      # got[1] address of dl_runtime_resolve
       *    lwz 12,(got+8-1b)@l(12)     # got[2] contains the map address
       *    mtctr 0
       *    add 0,11,11
       *    add 11,0,11                 # r11 = index * 12 = reloc offset.
       *    bctr
       */
      static const unsigned int pic_plt_resolve[] =
	{
	  ADDIS_11_11,
	  MFLR_0,
	  BCL_20_31,
	  ADDI_11_11,
	  MFLR_12,
	  MTLR_0,
	  SUB_11_11_12,
	  ADDIS_12_12,
	  LWZ_0_12,
	  LWZ_12_12,
	  MTCTR_0,
	  ADD_0_11_11,
	  ADD_11_0_11,
	  BCTR,
	  NOP,
	  NOP
	};

      /*
       * Non-PIC glink code is a little simpler.
       *
       * # ith PLT code stub.
       *   lis 11,(plt+(i-1)*4)@ha
       *   lwz 11,(plt+(i-1)*4)@l(11)
       *   mtctr 11
       *   bctr
       *
       * The branch table is the same, then comes
       *
       * PLTresolve:
       *    lis 12,(got+4)@ha
       *    addis 11,11,(-res_0)@ha
       *    lwz 0,(got+4)@l(12)         # got[1] address of dl_runtime_resolve
       *    addi 11,11,(-res_0)@l       # r11 = index * 4
       *    mtctr 0
       *    add 0,11,11
       *    lwz 12,(got+8)@l(12)        # got[2] contains the map address
       *    add 11,0,11                 # r11 = index * 12 = reloc offset.
       *    bctr
       */
      static const unsigned int plt_resolve[] =
	{
	  LIS_12,
	  ADDIS_11_11,
	  LWZ_0_12,
	  ADDI_11_11,
	  MTCTR_0,
	  ADD_0_11_11,
	  LWZ_12_12,
	  ADD_11_0_11,
	  BCTR,
	  NOP,
	  NOP,
	  NOP,
	  NOP,
	  NOP,
	  NOP,
	  NOP
	};

      if (ARRAY_SIZE (pic_plt_resolve) != GLINK_PLTRESOLVE / 4)
	abort ();
      if (ARRAY_SIZE (plt_resolve) != GLINK_PLTRESOLVE / 4)
	abort ();

      /* Build the branch table, one for each plt entry (less one),
	 and perhaps some padding.  */
      p = htab->glink->contents;
      p += htab->glink_pltresolve;
      endp = htab->glink->contents;
      endp += htab->glink->size - GLINK_PLTRESOLVE;
      while (p < endp - (htab->params->ppc476_workaround ? 0 : 8 * 4))
	{
	  bfd_put_32 (output_bfd, B + endp - p, p);
	  p += 4;
	}
      while (p < endp)
	{
	  bfd_put_32 (output_bfd, NOP, p);
	  p += 4;
	}

      res0 = (htab->glink_pltresolve
	      + htab->glink->output_section->vma
	      + htab->glink->output_offset);

      if (htab->params->ppc476_workaround)
	{
	  /* Ensure that a call stub at the end of a page doesn't
	     result in prefetch over the end of the page into the
	     glink branch table.  */
	  bfd_vma pagesize = (bfd_vma) 1 << htab->params->pagesize_p2;
	  bfd_vma page_addr;
	  bfd_vma glink_start = (htab->glink->output_section->vma
				 + htab->glink->output_offset);

	  for (page_addr = res0 & -pagesize;
	       page_addr > glink_start;
	       page_addr -= pagesize)
	    {
	      /* We have a plt call stub that may need fixing.  */
	      bfd_byte *loc;
	      unsigned int insn;

	      loc = htab->glink->contents + page_addr - 4 - glink_start;
	      insn = bfd_get_32 (output_bfd, loc);
	      if (insn == BCTR)
		{
		  /* By alignment, we know that there must be at least
		     one other call stub before this one.  */
		  insn = bfd_get_32 (output_bfd, loc - 16);
		  if (insn == BCTR)
		    bfd_put_32 (output_bfd, B | (-16 & 0x3fffffc), loc);
		  else
		    bfd_put_32 (output_bfd, B | (-20 & 0x3fffffc), loc);
		}
	    }
	}

      /* Last comes the PLTresolve stub.  */
      if (info->shared)
	{
	  bfd_vma bcl;

	  for (i = 0; i < ARRAY_SIZE (pic_plt_resolve); i++)
	    {
	      unsigned int insn = pic_plt_resolve[i];

	      if (htab->params->ppc476_workaround && insn == NOP)
		insn = BA + 0;
	      bfd_put_32 (output_bfd, insn, p);
	      p += 4;
	    }
	  p -= 4 * ARRAY_SIZE (pic_plt_resolve);

	  bcl = (htab->glink->size - GLINK_PLTRESOLVE + 3*4
		 + htab->glink->output_section->vma
		 + htab->glink->output_offset);

	  bfd_put_32 (output_bfd,
		      ADDIS_11_11 + PPC_HA (bcl - res0), p + 0*4);
	  bfd_put_32 (output_bfd,
		      ADDI_11_11 + PPC_LO (bcl - res0), p + 3*4);
	  bfd_put_32 (output_bfd,
		      ADDIS_12_12 + PPC_HA (got + 4 - bcl), p + 7*4);
	  if (PPC_HA (got + 4 - bcl) == PPC_HA (got + 8 - bcl))
	    {
	      bfd_put_32 (output_bfd,
			  LWZ_0_12 + PPC_LO (got + 4 - bcl), p + 8*4);
	      bfd_put_32 (output_bfd,
			  LWZ_12_12 + PPC_LO (got + 8 - bcl), p + 9*4);
	    }
	  else
	    {
	      bfd_put_32 (output_bfd,
			  LWZU_0_12 + PPC_LO (got + 4 - bcl), p + 8*4);
	      bfd_put_32 (output_bfd,
			  LWZ_12_12 + 4, p + 9*4);
	    }
	}
      else
	{
	  for (i = 0; i < ARRAY_SIZE (plt_resolve); i++)
	    {
	      unsigned int insn = plt_resolve[i];

	      if (htab->params->ppc476_workaround && insn == NOP)
		insn = BA + 0;
	      bfd_put_32 (output_bfd, insn, p);
	      p += 4;
	    }
	  p -= 4 * ARRAY_SIZE (plt_resolve);

	  bfd_put_32 (output_bfd,
		      LIS_12 + PPC_HA (got + 4), p + 0*4);
	  bfd_put_32 (output_bfd,
		      ADDIS_11_11 + PPC_HA (-res0), p + 1*4);
	  bfd_put_32 (output_bfd,
		      ADDI_11_11 + PPC_LO (-res0), p + 3*4);
	  if (PPC_HA (got + 4) == PPC_HA (got + 8))
	    {
	      bfd_put_32 (output_bfd,
			  LWZ_0_12 + PPC_LO (got + 4), p + 2*4);
	      bfd_put_32 (output_bfd,
			  LWZ_12_12 + PPC_LO (got + 8), p + 6*4);
	    }
	  else
	    {
	      bfd_put_32 (output_bfd,
			  LWZU_0_12 + PPC_LO (got + 4), p + 2*4);
	      bfd_put_32 (output_bfd,
			  LWZ_12_12 + 4, p + 6*4);
	    }
	}
    }

  if (htab->glink_eh_frame != NULL
      && htab->glink_eh_frame->contents != NULL)
    {
      unsigned char *p = htab->glink_eh_frame->contents;
      bfd_vma val;

      p += sizeof (glink_eh_frame_cie);
      /* FDE length.  */
      p += 4;
      /* CIE pointer.  */
      p += 4;
      /* Offset to .glink.  */
      val = (htab->glink->output_section->vma
	     + htab->glink->output_offset);
      val -= (htab->glink_eh_frame->output_section->vma
	      + htab->glink_eh_frame->output_offset);
      val -= p - htab->glink_eh_frame->contents;
      bfd_put_32 (htab->elf.dynobj, val, p);

      if (htab->glink_eh_frame->sec_info_type == SEC_INFO_TYPE_EH_FRAME
	  && !_bfd_elf_write_section_eh_frame (output_bfd, info,
					       htab->glink_eh_frame,
					       htab->glink_eh_frame->contents))
	return FALSE;
    }

  return ret;
}

#define TARGET_LITTLE_SYM	powerpc_elf32_le_vec
#define TARGET_LITTLE_NAME	"elf32-powerpcle"
#define TARGET_BIG_SYM		powerpc_elf32_vec
#define TARGET_BIG_NAME		"elf32-powerpc"
#define ELF_ARCH		bfd_arch_powerpc
#define ELF_TARGET_ID		PPC32_ELF_DATA
#define ELF_MACHINE_CODE	EM_PPC
#ifdef __QNXTARGET__
#define ELF_MAXPAGESIZE		0x1000
#define ELF_COMMONPAGESIZE	0x1000
#else
#define ELF_MAXPAGESIZE		0x10000
#define ELF_COMMONPAGESIZE	0x10000
#endif
#define ELF_MINPAGESIZE		0x1000
#define elf_info_to_howto	ppc_elf_info_to_howto

#ifdef  EM_CYGNUS_POWERPC
#define ELF_MACHINE_ALT1	EM_CYGNUS_POWERPC
#endif

#ifdef EM_PPC_OLD
#define ELF_MACHINE_ALT2	EM_PPC_OLD
#endif

#define elf_backend_plt_not_loaded	1
#define elf_backend_can_gc_sections	1
#define elf_backend_can_refcount	1
#define elf_backend_rela_normal		1
#define elf_backend_caches_rawsize	1

#define bfd_elf32_mkobject			ppc_elf_mkobject
#define bfd_elf32_bfd_merge_private_bfd_data	ppc_elf_merge_private_bfd_data
#define bfd_elf32_bfd_relax_section		ppc_elf_relax_section
#define bfd_elf32_bfd_reloc_type_lookup		ppc_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		ppc_elf_reloc_name_lookup
#define bfd_elf32_bfd_set_private_flags		ppc_elf_set_private_flags
#define bfd_elf32_bfd_link_hash_table_create	ppc_elf_link_hash_table_create
#define bfd_elf32_get_synthetic_symtab		ppc_elf_get_synthetic_symtab

#define elf_backend_object_p			ppc_elf_object_p
#define elf_backend_gc_mark_hook		ppc_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook		ppc_elf_gc_sweep_hook
#define elf_backend_section_from_shdr		ppc_elf_section_from_shdr
#define elf_backend_relocate_section		ppc_elf_relocate_section
#define elf_backend_create_dynamic_sections	ppc_elf_create_dynamic_sections
#define elf_backend_check_relocs		ppc_elf_check_relocs
#define elf_backend_copy_indirect_symbol	ppc_elf_copy_indirect_symbol
#define elf_backend_adjust_dynamic_symbol	ppc_elf_adjust_dynamic_symbol
#define elf_backend_add_symbol_hook		ppc_elf_add_symbol_hook
#define elf_backend_size_dynamic_sections	ppc_elf_size_dynamic_sections
#define elf_backend_hash_symbol			ppc_elf_hash_symbol
#define elf_backend_finish_dynamic_symbol	ppc_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections	ppc_elf_finish_dynamic_sections
#define elf_backend_fake_sections		ppc_elf_fake_sections
#define elf_backend_additional_program_headers	ppc_elf_additional_program_headers
#define elf_backend_modify_segment_map     	ppc_elf_modify_segment_map
#define elf_backend_grok_prstatus		ppc_elf_grok_prstatus
#define elf_backend_grok_psinfo			ppc_elf_grok_psinfo
#define elf_backend_write_core_note		ppc_elf_write_core_note
#define elf_backend_reloc_type_class		ppc_elf_reloc_type_class
#define elf_backend_begin_write_processing	ppc_elf_begin_write_processing
#define elf_backend_final_write_processing	ppc_elf_final_write_processing
#define elf_backend_write_section		ppc_elf_write_section
#define elf_backend_get_sec_type_attr		ppc_elf_get_sec_type_attr
#define elf_backend_plt_sym_val			ppc_elf_plt_sym_val
#define elf_backend_action_discarded		ppc_elf_action_discarded
#define elf_backend_init_index_section		_bfd_elf_init_1_index_section
#define elf_backend_lookup_section_flags_hook	ppc_elf_lookup_section_flags
#define elf_backend_section_processing		ppc_elf_section_processing

#include "elf32-target.h"

/* FreeBSD Target */

#undef  TARGET_LITTLE_SYM
#undef  TARGET_LITTLE_NAME

#undef  TARGET_BIG_SYM
#define TARGET_BIG_SYM  powerpc_elf32_fbsd_vec
#undef  TARGET_BIG_NAME
#define TARGET_BIG_NAME "elf32-powerpc-freebsd"

#undef  ELF_OSABI
#define ELF_OSABI	ELFOSABI_FREEBSD

#undef  elf32_bed
#define elf32_bed	elf32_powerpc_fbsd_bed

#include "elf32-target.h"

/* VxWorks Target */

#undef TARGET_LITTLE_SYM
#undef TARGET_LITTLE_NAME

#undef TARGET_BIG_SYM
#define TARGET_BIG_SYM		powerpc_elf32_vxworks_vec
#undef TARGET_BIG_NAME
#define TARGET_BIG_NAME		"elf32-powerpc-vxworks"

#undef  ELF_OSABI

/* VxWorks uses the elf default section flags for .plt.  */
static const struct bfd_elf_special_section *
ppc_elf_vxworks_get_sec_type_attr (bfd *abfd ATTRIBUTE_UNUSED, asection *sec)
{
  if (sec->name == NULL)
    return NULL;

  if (strcmp (sec->name, ".plt") == 0)
    return _bfd_elf_get_sec_type_attr (abfd, sec);

  return ppc_elf_get_sec_type_attr (abfd, sec);
}

/* Like ppc_elf_link_hash_table_create, but overrides
   appropriately for VxWorks.  */
static struct bfd_link_hash_table *
ppc_elf_vxworks_link_hash_table_create (bfd *abfd)
{
  struct bfd_link_hash_table *ret;

  ret = ppc_elf_link_hash_table_create (abfd);
  if (ret)
    {
      struct ppc_elf_link_hash_table *htab
        = (struct ppc_elf_link_hash_table *)ret;
      htab->is_vxworks = 1;
      htab->plt_type = PLT_VXWORKS;
      htab->plt_entry_size = VXWORKS_PLT_ENTRY_SIZE;
      htab->plt_slot_size = VXWORKS_PLT_ENTRY_SIZE;
      htab->plt_initial_entry_size = VXWORKS_PLT_INITIAL_ENTRY_SIZE;
    }
  return ret;
}

/* Tweak magic VxWorks symbols as they are loaded.  */
static bfd_boolean
ppc_elf_vxworks_add_symbol_hook (bfd *abfd,
				 struct bfd_link_info *info,
				 Elf_Internal_Sym *sym,
				 const char **namep ATTRIBUTE_UNUSED,
				 flagword *flagsp ATTRIBUTE_UNUSED,
				 asection **secp,
				 bfd_vma *valp)
{
  if (!elf_vxworks_add_symbol_hook(abfd, info, sym,namep, flagsp, secp,
				   valp))
    return FALSE;

  return ppc_elf_add_symbol_hook(abfd, info, sym,namep, flagsp, secp, valp);
}

static void
ppc_elf_vxworks_final_write_processing (bfd *abfd, bfd_boolean linker)
{
  ppc_elf_final_write_processing(abfd, linker);
  elf_vxworks_final_write_processing(abfd, linker);
}

/* On VxWorks, we emit relocations against _PROCEDURE_LINKAGE_TABLE_, so
   define it.  */
#undef elf_backend_want_plt_sym
#define elf_backend_want_plt_sym		1
#undef elf_backend_want_got_plt
#define elf_backend_want_got_plt		1
#undef elf_backend_got_symbol_offset
#define elf_backend_got_symbol_offset		0
#undef elf_backend_plt_not_loaded
#define elf_backend_plt_not_loaded		0
#undef elf_backend_plt_readonly
#define elf_backend_plt_readonly		1
#undef elf_backend_got_header_size
#define elf_backend_got_header_size		12

#undef bfd_elf32_get_synthetic_symtab

#undef bfd_elf32_bfd_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_create \
  ppc_elf_vxworks_link_hash_table_create
#undef elf_backend_add_symbol_hook
#define elf_backend_add_symbol_hook \
  ppc_elf_vxworks_add_symbol_hook
#undef elf_backend_link_output_symbol_hook
#define elf_backend_link_output_symbol_hook \
  elf_vxworks_link_output_symbol_hook
#undef elf_backend_final_write_processing
#define elf_backend_final_write_processing \
  ppc_elf_vxworks_final_write_processing
#undef elf_backend_get_sec_type_attr
#define elf_backend_get_sec_type_attr \
  ppc_elf_vxworks_get_sec_type_attr
#undef elf_backend_emit_relocs
#define elf_backend_emit_relocs \
  elf_vxworks_emit_relocs

#undef elf32_bed
#define elf32_bed				ppc_elf_vxworks_bed
#undef elf_backend_post_process_headers

#include "elf32-target.h"

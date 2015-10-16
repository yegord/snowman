/* Or1k-specific support for 32-bit ELF.
   Copyright 2001-2014 Free Software Foundation, Inc.
   Contributed for OR32 by Johan Rydberg, jrydberg@opencores.org

   PIC parts added by Stefan Kristiansson, stefan.kristiansson@saunalahti.fi,
   largely based on elf32-m32r.c and elf32-microblaze.c.

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
   along with this program; if not, see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/or1k.h"
#include "libiberty.h"

#define PLT_ENTRY_SIZE 20

#define PLT0_ENTRY_WORD0 0x19800000 /* l.movhi r12, 0 <- hi(.got+4) */
#define PLT0_ENTRY_WORD1 0xa98c0000 /* l.ori r12, r12, 0 <- lo(.got+4) */
#define PLT0_ENTRY_WORD2 0x85ec0004 /* l.lwz r15, 4(r12) <- *(.got+8)*/
#define PLT0_ENTRY_WORD3 0x44007800 /* l.jr r15 */
#define PLT0_ENTRY_WORD4 0x858c0000 /* l.lwz r12, 0(r12) */

#define PLT0_PIC_ENTRY_WORD0 0x85900004 /* l.lwz r12, 4(r16) */
#define PLT0_PIC_ENTRY_WORD1 0x85f00008 /* l.lwz r15, 8(r16) */
#define PLT0_PIC_ENTRY_WORD2 0x44007800 /* l.jr r15 */
#define PLT0_PIC_ENTRY_WORD3 0x15000000 /* l.nop */
#define PLT0_PIC_ENTRY_WORD4 0x15000000 /* l.nop */

#define PLT_ENTRY_WORD0 0x19800000 /* l.movhi r12, 0 <- hi(got idx addr) */
#define PLT_ENTRY_WORD1 0xa98c0000 /* l.ori r12, r12, 0 <- lo(got idx addr) */
#define PLT_ENTRY_WORD2 0x858c0000 /* l.lwz r12, 0(r12) */
#define PLT_ENTRY_WORD3 0x44006000 /* l.jr r12 */
#define PLT_ENTRY_WORD4 0xa9600000 /* l.ori r11, r0, 0 <- reloc offset */

#define PLT_PIC_ENTRY_WORD0 0x85900000 /* l.lwz r12, 0(r16) <- index in got */
#define PLT_PIC_ENTRY_WORD1 0xa9600000 /* l.ori r11, r0, 0 <- reloc offset */
#define PLT_PIC_ENTRY_WORD2 0x44006000 /* l.jr r12 */
#define PLT_PIC_ENTRY_WORD3 0x15000000 /* l.nop */
#define PLT_PIC_ENTRY_WORD4 0x15000000 /* l.nop */

#define ELF_DYNAMIC_INTERPRETER "/usr/lib/ld.so.1"

static reloc_howto_type or1k_elf_howto_table[] =
{
  /* This reloc does nothing.  */
  HOWTO (R_OR1K_NONE,           /* type */
         0,                     /* rightshift */
         3,                     /* size (0 = byte, 1 = short, 2 = long) */
         0,                     /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_NONE",         /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0,                     /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_32,
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         32,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_unsigned, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_32",           /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0xffffffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_16,
         0,                     /* rightshift */
         1,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_unsigned, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_16",           /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_8,
         0,                     /* rightshift */
         0,                     /* size (0 = byte, 1 = short, 2 = long) */
         8,                     /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_unsigned, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_8",            /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0xff,                  /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_LO_16_IN_INSN, /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_LO_16_IN_INSN", /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0x0000ffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_HI_16_IN_INSN, /* type */
         16,                    /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_HI_16_IN_INSN", /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0x0000ffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  /* A PC relative 26 bit relocation, right shifted by 2.  */
  HOWTO (R_OR1K_INSN_REL_26, /* type */
         2,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         26,                    /* bitsize */
         TRUE,                  /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_signed, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_INSN_REL_26", /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0x03ffffff,            /* dst_mask */
         TRUE),                 /* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_OR1K_GNU_VTINHERIT, /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         0,                     /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         NULL,                  /* special_function */
         "R_OR1K_GNU_VTINHERIT", /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0,                     /* dst_mask */
         FALSE),                /* pcrel_offset */

  /* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_OR1K_GNU_VTENTRY, /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         0,                     /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         _bfd_elf_rel_vtable_reloc_fn, /* special_function */
         "R_OR1K_GNU_VTENTRY", /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0,                     /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_32_PCREL,
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         32,                    /* bitsize */
         TRUE,                  /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_signed, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_32_PCREL",     /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0xffffffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_16_PCREL,
         0,                     /* rightshift */
         1,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         TRUE,                  /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_signed, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_16_PCREL",     /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_8_PCREL,
         0,                     /* rightshift */
         0,                     /* size (0 = byte, 1 = short, 2 = long) */
         8,                     /* bitsize */
         TRUE,                  /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_signed, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_8_PCREL",      /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0xff,                  /* dst_mask */
         FALSE),                /* pcrel_offset */

   HOWTO (R_OR1K_GOTPC_HI16,    /* Type.  */
         16,                    /* Rightshift.  */
         2,                     /* Size (0 = byte, 1 = short, 2 = long).  */
         16,                    /* Bitsize.  */
         TRUE,                  /* PC_relative.  */
         0,                     /* Bitpos.  */
         complain_overflow_dont, /* Complain on overflow.  */
         bfd_elf_generic_reloc, /* Special Function.  */
         "R_OR1K_GOTPC_HI16",   /* Name.  */
         FALSE,         /* Partial Inplace.  */
         0,                     /* Source Mask.  */
         0xffff,                /* Dest Mask.  */
         TRUE),                 /* PC relative offset?  */

   HOWTO (R_OR1K_GOTPC_LO16,    /* Type.  */
         0,                     /* Rightshift.  */
         2,                     /* Size (0 = byte, 1 = short, 2 = long).  */
         16,                    /* Bitsize.  */
         TRUE,                  /* PC_relative.  */
         0,                     /* Bitpos.  */
         complain_overflow_dont, /* Complain on overflow.  */
         bfd_elf_generic_reloc, /* Special Function.  */
         "R_OR1K_GOTPC_LO16",   /* Name.  */
         FALSE,         /* Partial Inplace.  */
         0,                     /* Source Mask.  */
         0xffff,                /* Dest Mask.  */
         TRUE),                 /* PC relative offset?  */

  HOWTO (R_OR1K_GOT16,          /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_signed, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_GOT16",        /* name */
         FALSE,                 /* partial_inplace */
         0,                     /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  /* A 26 bit PLT relocation.  Shifted by 2.  */
  HOWTO (R_OR1K_PLT26,  /* Type.  */
         2,                     /* Rightshift.  */
         2,                     /* Size (0 = byte, 1 = short, 2 = long).  */
         26,                    /* Bitsize.  */
         TRUE,                  /* PC_relative.  */
         0,                     /* Bitpos.  */
         complain_overflow_dont, /* Complain on overflow.  */
         bfd_elf_generic_reloc,/* Special Function.  */
         "R_OR1K_PLT26",        /* Name.  */
         FALSE,         /* Partial Inplace.  */
         0,                     /* Source Mask.  */
         0x03ffffff,            /* Dest Mask.  */
         TRUE),                 /* PC relative offset?  */

  HOWTO (R_OR1K_GOTOFF_HI16,    /* type */
         16,                    /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_GOTOFF_HI16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_GOTOFF_LO16,    /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_GOTOFF_LO16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_COPY,           /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         32,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_bitfield, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_COPY",         /* name */
         FALSE,                 /* partial_inplace */
         0xffffffff,            /* src_mask */
         0xffffffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_GLOB_DAT,       /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         32,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_bitfield, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_GLOB_DAT",     /* name */
         FALSE,                 /* partial_inplace */
         0xffffffff,            /* src_mask */
         0xffffffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_JMP_SLOT,       /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         32,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_bitfield, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_JMP_SLOT",     /* name */
         FALSE,                 /* partial_inplace */
         0xffffffff,            /* src_mask */
         0xffffffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_RELATIVE,       /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         32,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_bitfield, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_RELATIVE",     /* name */
         FALSE,                 /* partial_inplace */
         0xffffffff,            /* src_mask */
         0xffffffff,            /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_GD_HI16,    /* type */
         16,                    /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_GD_HI16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_GD_LO16,    /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_GD_LO16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDM_HI16,   /* type */
         16,                    /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_LDM_HI16", /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDM_LO16,   /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_LDM_LO16", /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDO_HI16,   /* type */
         16,                    /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_LDO_HI16", /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_LDO_LO16,   /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_LDO_LO16", /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_IE_HI16,    /* type */
         16,                    /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_IE_HI16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_IE_LO16,    /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_IE_LO16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_LE_HI16,    /* type */
         16,                    /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_LE_HI16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

  HOWTO (R_OR1K_TLS_LE_LO16,    /* type */
         0,                     /* rightshift */
         2,                     /* size (0 = byte, 1 = short, 2 = long) */
         16,                    /* bitsize */
         FALSE,                 /* pc_relative */
         0,                     /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc, /* special_function */
         "R_OR1K_TLS_LE_LO16",  /* name */
         FALSE,                 /* partial_inplace */
         0x0,                   /* src_mask */
         0xffff,                /* dst_mask */
         FALSE),                /* pcrel_offset */

};

/* Map BFD reloc types to Or1k ELF reloc types.  */

struct or1k_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int or1k_reloc_val;
};

static const struct or1k_reloc_map or1k_reloc_map[] =
{
  { BFD_RELOC_NONE,             R_OR1K_NONE },
  { BFD_RELOC_32,               R_OR1K_32 },
  { BFD_RELOC_16,               R_OR1K_16 },
  { BFD_RELOC_8,                R_OR1K_8 },
  { BFD_RELOC_LO16,             R_OR1K_LO_16_IN_INSN },
  { BFD_RELOC_HI16,             R_OR1K_HI_16_IN_INSN },
  { BFD_RELOC_OR1K_REL_26,      R_OR1K_INSN_REL_26 },
  { BFD_RELOC_VTABLE_ENTRY,     R_OR1K_GNU_VTENTRY },
  { BFD_RELOC_VTABLE_INHERIT,   R_OR1K_GNU_VTINHERIT },
  { BFD_RELOC_32_PCREL,         R_OR1K_32_PCREL },
  { BFD_RELOC_16_PCREL,         R_OR1K_16_PCREL },
  { BFD_RELOC_8_PCREL,          R_OR1K_8_PCREL },
  { BFD_RELOC_OR1K_GOTPC_HI16,  R_OR1K_GOTPC_HI16 },
  { BFD_RELOC_OR1K_GOTPC_LO16,  R_OR1K_GOTPC_LO16 },
  { BFD_RELOC_OR1K_GOT16,       R_OR1K_GOT16 },
  { BFD_RELOC_OR1K_PLT26,       R_OR1K_PLT26 },
  { BFD_RELOC_OR1K_GOTOFF_HI16, R_OR1K_GOTOFF_HI16 },
  { BFD_RELOC_OR1K_GOTOFF_LO16, R_OR1K_GOTOFF_LO16 },
  { BFD_RELOC_OR1K_GLOB_DAT,    R_OR1K_GLOB_DAT },
  { BFD_RELOC_OR1K_COPY,        R_OR1K_COPY },
  { BFD_RELOC_OR1K_JMP_SLOT,    R_OR1K_JMP_SLOT },
  { BFD_RELOC_OR1K_RELATIVE,    R_OR1K_RELATIVE },
  { BFD_RELOC_OR1K_TLS_GD_HI16, R_OR1K_TLS_GD_HI16 },
  { BFD_RELOC_OR1K_TLS_GD_LO16, R_OR1K_TLS_GD_LO16 },
  { BFD_RELOC_OR1K_TLS_LDM_HI16,        R_OR1K_TLS_LDM_HI16 },
  { BFD_RELOC_OR1K_TLS_LDM_LO16,        R_OR1K_TLS_LDM_LO16 },
  { BFD_RELOC_OR1K_TLS_LDO_HI16,        R_OR1K_TLS_LDO_HI16 },
  { BFD_RELOC_OR1K_TLS_LDO_LO16,        R_OR1K_TLS_LDO_LO16 },
  { BFD_RELOC_OR1K_TLS_IE_HI16, R_OR1K_TLS_IE_HI16 },
  { BFD_RELOC_OR1K_TLS_IE_LO16, R_OR1K_TLS_IE_LO16 },
  { BFD_RELOC_OR1K_TLS_LE_HI16, R_OR1K_TLS_LE_HI16 },
  { BFD_RELOC_OR1K_TLS_LE_LO16, R_OR1K_TLS_LE_LO16 },
};

/* The linker needs to keep track of the number of relocs that it
   decides to copy as dynamic relocs in check_relocs for each symbol.
   This is so that it can later discard them if they are found to be
   unnecessary.  We store the information in a field extending the
   regular ELF linker hash table.  */

struct elf_or1k_dyn_relocs
{
  struct elf_or1k_dyn_relocs *next;

  /* The input section of the reloc.  */
  asection *sec;

  /* Total number of relocs copied for the input section.  */
  bfd_size_type count;

  /* Number of pc-relative relocs copied for the input section.  */
  bfd_size_type pc_count;
};

#define TLS_UNKNOWN    0
#define TLS_NONE       1
#define TLS_GD         2
#define TLS_LD         3
#define TLS_IE         4
#define TLS_LE         5

/* ELF linker hash entry.  */
struct elf_or1k_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* Track dynamic relocs copied for this symbol.  */
  struct elf_or1k_dyn_relocs *dyn_relocs;

  /* Track type of TLS access.  */
  unsigned char tls_type;
};

/* ELF object data.  */
struct elf_or1k_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  unsigned char *local_tls_type;
};

#define elf_or1k_tdata(abfd) \
  ((struct elf_or1k_obj_tdata *) (abfd)->tdata.any)

#define elf_or1k_local_tls_type(abfd) \
  (elf_or1k_tdata (abfd)->local_tls_type)

/* ELF linker hash table.  */
struct elf_or1k_link_hash_table
{
  struct elf_link_hash_table root;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sgot;
  asection *sgotplt;
  asection *srelgot;
  asection *splt;
  asection *srelplt;
  asection *sdynbss;
  asection *srelbss;

  /* Small local sym to section mapping cache.  */
  struct sym_cache sym_sec;
};

/* Get the ELF linker hash table from a link_info structure.  */
#define or1k_elf_hash_table(p) \
  (elf_hash_table_id ((struct elf_link_hash_table *) ((p)->hash)) \
   == OR1K_ELF_DATA ? ((struct elf_or1k_link_hash_table *) ((p)->hash)) : NULL)

static bfd_boolean
elf_or1k_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_or1k_obj_tdata),
                                  OR1K_ELF_DATA);
}

/* Create an entry in an or1k ELF linker hash table.  */

static struct bfd_hash_entry *
or1k_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
                            struct bfd_hash_table *table,
                            const char *string)
{
  struct elf_or1k_link_hash_entry *ret =
    (struct elf_or1k_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table,
                             sizeof (struct elf_or1k_link_hash_entry));
  if (ret == NULL)
    return NULL;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_or1k_link_hash_entry *)
         _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
                                     table, string));
  if (ret != NULL)
    {
      struct elf_or1k_link_hash_entry *eh;

      eh = (struct elf_or1k_link_hash_entry *) ret;
      eh->dyn_relocs = NULL;
      eh->tls_type = TLS_UNKNOWN;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create an or1k ELF linker hash table.  */

static struct bfd_link_hash_table *
or1k_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_or1k_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct elf_or1k_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
                                      or1k_elf_link_hash_newfunc,
                                      sizeof (struct elf_or1k_link_hash_entry),
                                      OR1K_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->root.root;
}

static reloc_howto_type *
or1k_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
			bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = ARRAY_SIZE (or1k_reloc_map); i--;)
    if (or1k_reloc_map[i].bfd_reloc_val == code)
      return & or1k_elf_howto_table[or1k_reloc_map[i].or1k_reloc_val];

  return NULL;
}

static reloc_howto_type *
or1k_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (or1k_elf_howto_table)
            / sizeof (or1k_elf_howto_table[0]));
       i++)
    if (or1k_elf_howto_table[i].name != NULL
        && strcasecmp (or1k_elf_howto_table[i].name, r_name) == 0)
      return &or1k_elf_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an Or1k ELF reloc.  */

static void
or1k_info_to_howto_rela (bfd * abfd ATTRIBUTE_UNUSED,
			 arelent * cache_ptr,
			 Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_OR1K_max)
    {
      _bfd_error_handler (_("%B: invalid OR1K reloc number: %d"), abfd, r_type);
      r_type = 0;
    }
  cache_ptr->howto = & or1k_elf_howto_table[r_type];
}


/* Return the relocation value for @tpoff relocations..  */
static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;

  /* The thread pointer on or1k stores the address after the TCB where
     the data is, just compute the difference. No need to compensate
     for the size of TCB.  */
  return (address - elf_hash_table (info)->tls_sec->vma);
}

/* Relocate an Or1k ELF section.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjusting the section contents as
   necessary, and (if using Rela relocs and generating a relocatable
   output file) adjusting the reloc addend as necessary.

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
or1k_elf_relocate_section (bfd *output_bfd,
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
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  struct elf_or1k_link_hash_table *htab = or1k_elf_hash_table (info);
  bfd *dynobj;
  asection *sreloc;
  bfd_vma *local_got_offsets;
  asection *sgot;

  if (htab == NULL)
    return FALSE;

  dynobj = htab->root.dynobj;
  local_got_offsets = elf_local_got_offsets (input_bfd);

  sreloc = elf_section_data (input_section)->sreloc;

  sgot = htab->sgot;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name = NULL;
      int r_type;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      if (r_type == R_OR1K_GNU_VTINHERIT
          || r_type == R_OR1K_GNU_VTENTRY)
        continue;

      if (r_type < 0 || r_type >= (int) R_OR1K_max)
        {
          bfd_set_error (bfd_error_bad_value);
          return FALSE;
        }

      howto = or1k_elf_howto_table + ELF32_R_TYPE (rel->r_info);
      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
        {
          sym = local_syms + r_symndx;
          sec = local_sections[r_symndx];
          relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

          name = bfd_elf_string_from_elf_section
            (input_bfd, symtab_hdr->sh_link, sym->st_name);
          name = (name == NULL) ? bfd_section_name (input_bfd, sec) : name;
        }
      else
        {
          bfd_boolean unresolved_reloc, warned, ignored;

          RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
                                   r_symndx, symtab_hdr, sym_hashes,
                                   h, sec, relocation,
                                   unresolved_reloc, warned, ignored);
        }

      if (sec != NULL && discarded_section (sec))
        RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
                                         rel, 1, relend, howto, 0, contents);

      if (info->relocatable)
        continue;

      switch (howto->type)
        {
        case R_OR1K_PLT26:
          {
            if (htab->splt != NULL && h != NULL
                && h->plt.offset != (bfd_vma) -1)
              {
                relocation = (htab->splt->output_section->vma
                              + htab->splt->output_offset
                              + h->plt.offset);
              }
            break;
          }

        case R_OR1K_GOT16:
          /* Relocation is to the entry for this symbol in the global
             offset table.  */
          BFD_ASSERT (sgot != NULL);
          if (h != NULL)
            {
              bfd_boolean dyn;
              bfd_vma off;

              off = h->got.offset;
              BFD_ASSERT (off != (bfd_vma) -1);

              dyn = htab->root.dynamic_sections_created;
              if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, info->shared, h)
                  || (info->shared
                      && SYMBOL_REFERENCES_LOCAL (info, h)))
                {
                  /* This is actually a static link, or it is a
                     -Bsymbolic link and the symbol is defined
                     locally, or the symbol was forced to be local
                     because of a version file.  We must initialize
                     this entry in the global offset table.  Since the
                     offset must always be a multiple of 4, we use the
                     least significant bit to record whether we have
                     initialized it already.

                     When doing a dynamic link, we create a .rela.got
                     relocation entry to initialize the value.  This
                     is done in the finish_dynamic_symbol routine.  */
                  if ((off & 1) != 0)
                    off &= ~1;
                  else
                    {
                      /* Write entry in GOT.  */
                      bfd_put_32 (output_bfd, relocation,
                                  sgot->contents + off);
                      /* Mark GOT entry as having been written.  */
                      h->got.offset |= 1;
                    }
                }

              relocation = sgot->output_offset + off;
            }
          else
            {
              bfd_vma off;
              bfd_byte *loc;

              BFD_ASSERT (local_got_offsets != NULL
                          && local_got_offsets[r_symndx] != (bfd_vma) -1);

              /* Get offset into GOT table.  */
              off = local_got_offsets[r_symndx];

              /* The offset must always be a multiple of 4.  We use
                 the least significant bit to record whether we have
                 already processed this entry.  */
              if ((off & 1) != 0)
                off &= ~1;
              else
                {
                  /* Write entry in GOT.  */
                  bfd_put_32 (output_bfd, relocation, sgot->contents + off);
                  if (info->shared)
                    {
                      asection *srelgot;
                      Elf_Internal_Rela outrel;

                      /* We need to generate a R_OR1K_RELATIVE reloc
                         for the dynamic linker.  */
                      srelgot = bfd_get_section_by_name (dynobj, ".rela.got");
                      BFD_ASSERT (srelgot != NULL);

                      outrel.r_offset = (sgot->output_section->vma
                                         + sgot->output_offset
                                         + off);
                      outrel.r_info = ELF32_R_INFO (0, R_OR1K_RELATIVE);
                      outrel.r_addend = relocation;
                      loc = srelgot->contents;
                      loc += srelgot->reloc_count * sizeof (Elf32_External_Rela);
                      bfd_elf32_swap_reloca_out (output_bfd, &outrel,loc);
                      ++srelgot->reloc_count;
                    }

                  local_got_offsets[r_symndx] |= 1;
                }
              relocation = sgot->output_offset + off;
            }

          /* Addend should be zero.  */
          if (rel->r_addend != 0)
            (*_bfd_error_handler)
              (_("internal error: addend should be zero for R_OR1K_GOT16"));

          break;

        case R_OR1K_GOTOFF_LO16:
        case R_OR1K_GOTOFF_HI16:
          /* Relocation is offset from GOT.  */
          BFD_ASSERT (sgot != NULL);
          relocation -= sgot->output_section->vma;
          break;

        case R_OR1K_INSN_REL_26:
        case R_OR1K_HI_16_IN_INSN:
        case R_OR1K_LO_16_IN_INSN:
        case R_OR1K_32:
          /* R_OR1K_16? */
          {
            /* r_symndx will be STN_UNDEF (zero) only for relocs against symbols
               from removed linkonce sections, or sections discarded by
               a linker script.  */
            if (r_symndx == STN_UNDEF
                || (input_section->flags & SEC_ALLOC) == 0)
              break;

            if ((info->shared
                 && (h == NULL
                     || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
                     || h->root.type != bfd_link_hash_undefweak)
		 && (howto->type != R_OR1K_INSN_REL_26
		     || !SYMBOL_CALLS_LOCAL (info, h)))
                || (!info->shared
                    && h != NULL
                    && h->dynindx != -1
                    && !h->non_got_ref
                    && ((h->def_dynamic
                         && !h->def_regular)
                        || h->root.type == bfd_link_hash_undefweak
                        || h->root.type == bfd_link_hash_undefined)))
              {
                Elf_Internal_Rela outrel;
                bfd_byte *loc;
                bfd_boolean skip;

                /* When generating a shared object, these relocations
                   are copied into the output file to be resolved at run
                   time.  */

                BFD_ASSERT (sreloc != NULL);

                skip = FALSE;

                outrel.r_offset =
                  _bfd_elf_section_offset (output_bfd, info, input_section,
                                           rel->r_offset);
                if (outrel.r_offset == (bfd_vma) -1)
                  skip = TRUE;
                else if (outrel.r_offset == (bfd_vma) -2)
                  skip = TRUE;
                outrel.r_offset += (input_section->output_section->vma
                                    + input_section->output_offset);

                if (skip)
                  memset (&outrel, 0, sizeof outrel);
                /* h->dynindx may be -1 if the symbol was marked to
                   become local.  */
                else if (h != NULL
                         && ((! info->symbolic && h->dynindx != -1)
                             || !h->def_regular))
                  {
                    BFD_ASSERT (h->dynindx != -1);
                    outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
                    outrel.r_addend = rel->r_addend;
                  }
                else
                  {
                    if (r_type == R_OR1K_32)
                      {
                        outrel.r_info = ELF32_R_INFO (0, R_OR1K_RELATIVE);
                        outrel.r_addend = relocation + rel->r_addend;
                      }
                    else
                      {
                        BFD_FAIL ();
                        (*_bfd_error_handler)
                          (_("%B: probably compiled without -fPIC?"),
                           input_bfd);
                        bfd_set_error (bfd_error_bad_value);
                        return FALSE;
                      }
                  }

                loc = sreloc->contents;
                loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
                bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
                break;
              }
            break;
          }

        case R_OR1K_TLS_LDM_HI16:
        case R_OR1K_TLS_LDM_LO16:
        case R_OR1K_TLS_LDO_HI16:
        case R_OR1K_TLS_LDO_LO16:
          /* TODO: implement support for local dynamic.  */
          BFD_FAIL ();
          (*_bfd_error_handler)
            (_("%B: support for local dynamic not implemented"),
             input_bfd);
          bfd_set_error (bfd_error_bad_value);
          return FALSE;


        case R_OR1K_TLS_GD_HI16:
        case R_OR1K_TLS_GD_LO16:
        case R_OR1K_TLS_IE_HI16:
        case R_OR1K_TLS_IE_LO16:
          {
            bfd_vma gotoff;
            Elf_Internal_Rela rela;
            bfd_byte *loc;
            int dynamic;

            sreloc = bfd_get_section_by_name (dynobj, ".rela.got");

            /* Mark as TLS related GOT entry by setting
               bit 2 as well as bit 1.  */
            if (h != NULL)
              {
                gotoff = h->got.offset;
                h->got.offset |= 3;
              }
            else
              {
                gotoff = local_got_offsets[r_symndx];
                local_got_offsets[r_symndx] |= 3;
              }

            /* Only process the relocation once.  */
            if (gotoff & 1)
              {
                relocation = sgot->output_offset + (gotoff  & ~3);
                break;
              }

            BFD_ASSERT (elf_hash_table (info)->hgot == NULL
			|| elf_hash_table (info)->hgot->root.u.def.value == 0);

            /* Dynamic entries will require relocations. if we do not need
               them we will just use the default R_OR1K_NONE and
               not set anything.  */
            dynamic = info->shared
	      || (sec && (sec->flags & SEC_ALLOC) != 0
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak || !h->def_regular));

            /* Shared GD.  */
            if (dynamic && (howto->type == R_OR1K_TLS_GD_HI16
			    || howto->type == R_OR1K_TLS_GD_LO16))
              {
                int i;

                /* Add DTPMOD and DTPOFF GOT and rela entries.  */
                for (i = 0; i < 2; ++i)
                  {
                    rela.r_offset = sgot->output_section->vma +
                      sgot->output_offset + gotoff + i*4;
                    if (h != NULL && h->dynindx != -1)
                      {
                        rela.r_info = ELF32_R_INFO (h->dynindx,
                            (i == 0 ? R_OR1K_TLS_DTPMOD : R_OR1K_TLS_DTPOFF));
                        rela.r_addend = 0;
                      }
                    else
                      {
                        rela.r_info = ELF32_R_INFO (0,
                            (i == 0 ? R_OR1K_TLS_DTPMOD : R_OR1K_TLS_DTPOFF));
                        rela.r_addend = tpoff (info, relocation);
                      }

                    loc = sreloc->contents;
                    loc += sreloc->reloc_count++ *
                      sizeof (Elf32_External_Rela);

                    bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
                    bfd_put_32 (output_bfd, 0, sgot->contents + gotoff + i*4);
                  }
              }
            /* Static GD.  */
            else if (howto->type == R_OR1K_TLS_GD_HI16
                     || howto->type == R_OR1K_TLS_GD_LO16)
              {
                bfd_put_32 (output_bfd, 1, sgot->contents + gotoff);
                bfd_put_32 (output_bfd, tpoff (info, relocation),
                    sgot->contents + gotoff + 4);
              }
            /* Shared IE.  */
            else if (dynamic)
              {
                /* Add TPOFF GOT and rela entries.  */
                rela.r_offset = sgot->output_section->vma +
                  sgot->output_offset + gotoff;
                if (h != NULL && h->dynindx != -1)
                  {
                    rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_TLS_TPOFF);
                    rela.r_addend = 0;
                  }
                else
                  {
                    rela.r_info = ELF32_R_INFO (0, R_OR1K_TLS_TPOFF);
                    rela.r_addend = tpoff (info, relocation);
                  }

                loc = sreloc->contents;
                loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);

                bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
                bfd_put_32 (output_bfd, 0, sgot->contents + gotoff);
              }
            /* Static IE.  */
            else
              {
                bfd_put_32 (output_bfd, tpoff (info, relocation),
			    sgot->contents + gotoff);
              }
            relocation = sgot->output_offset + gotoff;
            break;
          }
        case R_OR1K_TLS_LE_HI16:
        case R_OR1K_TLS_LE_LO16:

          /* Relocation is offset from TP.  */
          relocation = tpoff (info, relocation);
          break;

        case R_OR1K_TLS_DTPMOD:
        case R_OR1K_TLS_DTPOFF:
        case R_OR1K_TLS_TPOFF:
          /* These are resolved dynamically on load and shouldn't
             be used as linker input.  */
          BFD_FAIL ();
          (*_bfd_error_handler)
            (_("%B: will not resolve runtime TLS relocation"),
             input_bfd);
          bfd_set_error (bfd_error_bad_value);
          return FALSE;

        default:
          break;
        }
      r = _bfd_final_link_relocate (howto, input_bfd, input_section, contents,
                                    rel->r_offset, relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
        {
          const char *msg = NULL;

          switch (r)
            {
            case bfd_reloc_overflow:
              r = info->callbacks->reloc_overflow
                (info, (h ? &h->root : NULL), name, howto->name,
                 (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
              break;

            case bfd_reloc_undefined:
              r = info->callbacks->undefined_symbol
                (info, name, input_bfd, input_section, rel->r_offset, TRUE);
              break;

            case bfd_reloc_outofrange:
              msg = _("internal error: out of range error");
              break;

            case bfd_reloc_notsupported:
              msg = _("internal error: unsupported relocation error");
              break;

            case bfd_reloc_dangerous:
              msg = _("internal error: dangerous relocation");
              break;

            default:
              msg = _("internal error: unknown error");
              break;
            }

          if (msg)
            r = info->callbacks->warning
              (info, msg, name, input_bfd, input_section, rel->r_offset);

          if (!r)
            return FALSE;
        }
    }

  return TRUE;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
or1k_elf_gc_mark_hook (asection *sec,
		       struct bfd_link_info *info,
		       Elf_Internal_Rela *rel,
		       struct elf_link_hash_entry *h,
		       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_OR1K_GNU_VTINHERIT:
      case R_OR1K_GNU_VTENTRY:
        return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

static bfd_boolean
or1k_elf_gc_sweep_hook (bfd *abfd,
                        struct bfd_link_info *info ATTRIBUTE_UNUSED,
                        asection *sec,
                        const Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED)
{
  /* Update the got entry reference counts for the section being removed.  */
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_signed_vma *local_got_refcounts;
  const Elf_Internal_Rela *rel, *relend;

  elf_section_data (sec)->local_dynrel = NULL;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      unsigned long r_symndx;
      struct elf_link_hash_entry *h = NULL;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx >= symtab_hdr->sh_info)
        {
          h = sym_hashes[r_symndx - symtab_hdr->sh_info];
          while (h->root.type == bfd_link_hash_indirect
                 || h->root.type == bfd_link_hash_warning)
            h = (struct elf_link_hash_entry *) h->root.u.i.link;
        }

      switch (ELF32_R_TYPE (rel->r_info))
        {
        case R_OR1K_GOT16:
          if (h != NULL)
            {
              if (h->got.refcount > 0)
                h->got.refcount--;
            }
          else
            {
              if (local_got_refcounts && local_got_refcounts[r_symndx] > 0)
                local_got_refcounts[r_symndx]--;
            }
          break;

        default:
          break;
        }
    }
  return TRUE;
}

/* Create .got, .gotplt, and .rela.got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */

static bfd_boolean
create_got_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf_or1k_link_hash_table *htab;
  asection *s;

  /* This function may be called more than once.  */
  s = bfd_get_section_by_name (dynobj, ".got");
  if (s != NULL && (s->flags & SEC_LINKER_CREATED) != 0)
    return TRUE;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (! _bfd_elf_create_got_section (dynobj, info))
    return FALSE;

  htab->sgot = bfd_get_section_by_name (dynobj, ".got");
  htab->sgotplt = bfd_get_section_by_name (dynobj, ".got.plt");
  htab->srelgot = bfd_get_section_by_name (dynobj, ".rela.got");

  if (! htab->sgot || ! htab->sgotplt || ! htab->srelgot)
    abort ();

  if (! bfd_set_section_flags (dynobj, htab->srelgot, SEC_ALLOC
                               | SEC_LOAD
                               | SEC_HAS_CONTENTS
                               | SEC_IN_MEMORY
                               | SEC_LINKER_CREATED
                               | SEC_READONLY)
      || ! bfd_set_section_alignment (dynobj, htab->srelgot, 2))
    return FALSE;

  return TRUE;
}

/* Look through the relocs for a section during the first phase.  */

static bfd_boolean
or1k_elf_check_relocs (bfd *abfd,
		       struct bfd_link_info *info,
		       asection *sec,
		       const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;

  const Elf_Internal_Rela *rel_end;
  struct elf_or1k_link_hash_table *htab;
  bfd *dynobj;
  asection *sreloc = NULL;

  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  dynobj = htab->root.dynobj;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      unsigned char tls_type;

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

      switch (ELF32_R_TYPE (rel->r_info))
        {
        case R_OR1K_TLS_GD_HI16:
        case R_OR1K_TLS_GD_LO16:
          tls_type = TLS_GD;
          break;
        case R_OR1K_TLS_LDM_HI16:
        case R_OR1K_TLS_LDM_LO16:
        case R_OR1K_TLS_LDO_HI16:
        case R_OR1K_TLS_LDO_LO16:
          tls_type = TLS_LD;
          break;
        case R_OR1K_TLS_IE_HI16:
        case R_OR1K_TLS_IE_LO16:
          tls_type = TLS_IE;
          break;
        case R_OR1K_TLS_LE_HI16:
        case R_OR1K_TLS_LE_LO16:
          tls_type = TLS_LE;
          break;
        default:
          tls_type = TLS_NONE;
        }

      /* Record TLS type.  */
      if (h != NULL)
          ((struct elf_or1k_link_hash_entry *) h)->tls_type = tls_type;
      else
        {
          unsigned char *local_tls_type;

          /* This is a TLS type record for a local symbol.  */
          local_tls_type = (unsigned char *) elf_or1k_local_tls_type (abfd);
          if (local_tls_type == NULL)
            {
              bfd_size_type size;

              size = symtab_hdr->sh_info;
              local_tls_type = bfd_zalloc (abfd, size);
              if (local_tls_type == NULL)
                return FALSE;
              elf_or1k_local_tls_type (abfd) = local_tls_type;
            }
          local_tls_type[r_symndx] = tls_type;
        }

      switch (ELF32_R_TYPE (rel->r_info))
        {
          /* This relocation describes the C++ object vtable hierarchy.
             Reconstruct it for later use during GC.  */
        case R_OR1K_GNU_VTINHERIT:
          if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
            return FALSE;
          break;

          /* This relocation describes which C++ vtable entries are actually
             used.  Record for later use during GC.  */
        case R_OR1K_GNU_VTENTRY:
          BFD_ASSERT (h != NULL);
          if (h != NULL
              && !bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
            return FALSE;
          break;

          /* This relocation requires .plt entry.  */
        case R_OR1K_PLT26:
          if (h != NULL)
            {
              h->needs_plt = 1;
              h->plt.refcount += 1;
            }
          break;

        case R_OR1K_GOT16:
        case R_OR1K_GOTOFF_HI16:
        case R_OR1K_GOTOFF_LO16:
        case R_OR1K_TLS_GD_HI16:
        case R_OR1K_TLS_GD_LO16:
        case R_OR1K_TLS_IE_HI16:
        case R_OR1K_TLS_IE_LO16:
          if (htab->sgot == NULL)
            {
              if (dynobj == NULL)
                htab->root.dynobj = dynobj = abfd;
              if (! create_got_section (dynobj, info))
                return FALSE;
            }

          if (ELF32_R_TYPE (rel->r_info) != R_OR1K_GOTOFF_HI16 &&
              ELF32_R_TYPE (rel->r_info) != R_OR1K_GOTOFF_LO16)
            {
              if (h != NULL)
                h->got.refcount += 1;
              else
                {
                  bfd_signed_vma *local_got_refcounts;

                  /* This is a global offset table entry for a local symbol.  */
                  local_got_refcounts = elf_local_got_refcounts (abfd);
                  if (local_got_refcounts == NULL)
                    {
                      bfd_size_type size;

                      size = symtab_hdr->sh_info;
                      size *= sizeof (bfd_signed_vma);
                      local_got_refcounts = bfd_zalloc (abfd, size);
                      if (local_got_refcounts == NULL)
                        return FALSE;
                      elf_local_got_refcounts (abfd) = local_got_refcounts;
                    }
                  local_got_refcounts[r_symndx] += 1;
                }
            }
          break;

        case R_OR1K_INSN_REL_26:
        case R_OR1K_HI_16_IN_INSN:
        case R_OR1K_LO_16_IN_INSN:
        case R_OR1K_32:
          /* R_OR1K_16? */
          {
            if (h != NULL && !info->shared)
              {
                /* We may need a copy reloc.  */
                h->non_got_ref = 1;

                /* We may also need a .plt entry.  */
                h->plt.refcount += 1;
                if (ELF32_R_TYPE (rel->r_info) != R_OR1K_INSN_REL_26)
                  h->pointer_equality_needed = 1;
              }

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
               storing information in the relocs_copied field of the hash
               table entry.  A similar situation occurs when creating
               shared libraries and symbol visibility changes render the
               symbol local.

               If on the other hand, we are creating an executable, we
               may need to keep relocations for symbols satisfied by a
               dynamic library if we manage to avoid copy relocs for the
               symbol.  */

            if ((info->shared
                 && (sec->flags & SEC_ALLOC) != 0
                 && (ELF32_R_TYPE (rel->r_info) != R_OR1K_INSN_REL_26
                     || (h != NULL
                         && (!SYMBOLIC_BIND (info, h)
                             || h->root.type == bfd_link_hash_defweak
                             || !h->def_regular))))
                || (!info->shared
                    && (sec->flags & SEC_ALLOC) != 0
                    && h != NULL
                    && (h->root.type == bfd_link_hash_defweak
                        || !h->def_regular)))
              {
                struct elf_or1k_dyn_relocs *p;
                struct elf_or1k_dyn_relocs **head;

                /* When creating a shared object, we must copy these
                   relocs into the output file.  We create a reloc
                   section in dynobj and make room for the reloc.  */
                if (sreloc == NULL)
                  {
                    const char *name;
                    unsigned int strndx = elf_elfheader (abfd)->e_shstrndx;
                    unsigned int shnam = _bfd_elf_single_rel_hdr (sec)->sh_name;

                    name = bfd_elf_string_from_elf_section (abfd, strndx, shnam);
                    if (name == NULL)
                      return FALSE;

                    if (strncmp (name, ".rela", 5) != 0
                        || strcmp (bfd_get_section_name (abfd, sec),
                                   name + 5) != 0)
                      {
                        (*_bfd_error_handler)
                          (_("%B: bad relocation section name `%s\'"),
                           abfd, name);
                      }

                    if (htab->root.dynobj == NULL)
                      htab->root.dynobj = abfd;
                    dynobj = htab->root.dynobj;

                    sreloc = bfd_get_section_by_name (dynobj, name);
                    if (sreloc == NULL)
                      {
                        sreloc = _bfd_elf_make_dynamic_reloc_section
                          (sec, dynobj, 2, abfd, /*rela?*/ TRUE);

                        if (sreloc == NULL)
                          return FALSE;
                      }
                    elf_section_data (sec)->sreloc = sreloc;
                  }

                /* If this is a global symbol, we count the number of
                   relocations we need for this symbol.  */
                if (h != NULL)
                  head = &((struct elf_or1k_link_hash_entry *) h)->dyn_relocs;
                else
                  {
                    /* Track dynamic relocs needed for local syms too.
                       We really need local syms available to do this
                       easily.  Oh well.  */

                    asection *s;
                    Elf_Internal_Sym *isym;
                    void *vpp;

                    isym = bfd_sym_from_r_symndx (&htab->sym_sec,
                                                  abfd, r_symndx);
                    if (isym == NULL)
                      return FALSE;

                    s = bfd_section_from_elf_index (abfd, isym->st_shndx);
                    if (s == NULL)
                      return FALSE;

                    vpp = &elf_section_data (s)->local_dynrel;
                    head = (struct elf_or1k_dyn_relocs **) vpp;
                  }

                p = *head;
                if (p == NULL || p->sec != sec)
                  {
                    bfd_size_type amt = sizeof *p;
                    p = ((struct elf_or1k_dyn_relocs *)
                         bfd_alloc (htab->root.dynobj, amt));
                    if (p == NULL)
                      return FALSE;
                    p->next = *head;
                    *head = p;
                    p->sec = sec;
                    p->count = 0;
                    p->pc_count = 0;
                  }

                p->count += 1;
                if (ELF32_R_TYPE (rel->r_info) == R_OR1K_INSN_REL_26)
                  p->pc_count += 1;
              }
          }
          break;
        }
    }

  return TRUE;
}

/* Finish up the dynamic sections.  */

static bfd_boolean
or1k_elf_finish_dynamic_sections (bfd *output_bfd,
                                  struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn, *sgot;
  struct elf_or1k_link_hash_table *htab;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  dynobj = htab->root.dynobj;

  sgot = htab->sgotplt;
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (htab->root.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sgot != NULL && sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);

      for (; dyncon < dynconend; dyncon++)
        {
          Elf_Internal_Dyn dyn;
          asection *s;

          bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

          switch (dyn.d_tag)
            {
            default:
              continue;

            case DT_PLTGOT:
              s = htab->sgot->output_section;
              BFD_ASSERT (s != NULL);
              dyn.d_un.d_ptr = s->vma;
              break;

            case DT_JMPREL:
              s = htab->srelplt->output_section;
              BFD_ASSERT (s != NULL);
              dyn.d_un.d_ptr = s->vma;
              break;

            case DT_PLTRELSZ:
              s = htab->srelplt->output_section;
              BFD_ASSERT (s != NULL);
              dyn.d_un.d_val = s->size;
              break;

            case DT_RELASZ:
              /* My reading of the SVR4 ABI indicates that the
                 procedure linkage table relocs (DT_JMPREL) should be
                 included in the overall relocs (DT_RELA).  This is
                 what Solaris does.  However, UnixWare can not handle
                 that case.  Therefore, we override the DT_RELASZ entry
                 here to make it not include the JMPREL relocs.  Since
                 the linker script arranges for .rela.plt to follow all
                 other relocation sections, we don't have to worry
                 about changing the DT_RELA entry.  */
              if (htab->srelplt != NULL)
                {
                  /* FIXME: this calculation sometimes produces
                     wrong result, the problem is that the dyn.d_un.d_val
                     is not always correct, needs investigation why
                     that happens. In the meantime, reading the
                     ".rela.dyn" section by name seems to yield
                     correct result.

                  s = htab->srelplt->output_section;
                  dyn.d_un.d_val -= s->size;
                  */

                  s = bfd_get_section_by_name (output_bfd, ".rela.dyn");
                  dyn.d_un.d_val = s ? s->size : 0;
                }
              break;
            }
          bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
        }


      /* Fill in the first entry in the procedure linkage table.  */
      splt = htab->splt;
      if (splt && splt->size > 0)
        {
          if (info->shared)
            {
              bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD0,
                          splt->contents);
              bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD1,
                          splt->contents + 4);
              bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD2,
                          splt->contents + 8);
              bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD3,
                          splt->contents + 12);
              bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD4,
                          splt->contents + 16);
            }
          else
            {
              unsigned long addr;
              /* addr = .got + 4 */
              addr = sgot->output_section->vma + sgot->output_offset + 4;
              bfd_put_32 (output_bfd,
                          PLT0_ENTRY_WORD0 | ((addr >> 16) & 0xffff),
                          splt->contents);
              bfd_put_32 (output_bfd,
                          PLT0_ENTRY_WORD1 | (addr & 0xffff),
                          splt->contents + 4);
              bfd_put_32 (output_bfd, PLT0_ENTRY_WORD2, splt->contents + 8);
              bfd_put_32 (output_bfd, PLT0_ENTRY_WORD3, splt->contents + 12);
              bfd_put_32 (output_bfd, PLT0_ENTRY_WORD4, splt->contents + 16);
            }

          elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;
        }
    }

  /* Set the first entry in the global offset table to the address of
     the dynamic section.  */
  if (sgot && sgot->size > 0)
    {
      if (sdyn == NULL)
        bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents);
      else
        bfd_put_32 (output_bfd,
                    sdyn->output_section->vma + sdyn->output_offset,
                    sgot->contents);
      elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;
    }

  if (htab->sgot && htab->sgot->size > 0)
    elf_section_data (htab->sgot->output_section)->this_hdr.sh_entsize = 4;

  return TRUE;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
or1k_elf_finish_dynamic_symbol (bfd *output_bfd,
                                struct bfd_link_info *info,
                                struct elf_link_hash_entry *h,
                                Elf_Internal_Sym *sym)
{
  struct elf_or1k_link_hash_table *htab;
  bfd_byte *loc;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *sgot;
      asection *srela;

      bfd_vma plt_index;
      bfd_vma got_offset;
      bfd_vma got_addr;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the procedure linkage table.  Set
         it up.  */
      BFD_ASSERT (h->dynindx != -1);

      splt = htab->splt;
      sgot = htab->sgotplt;
      srela = htab->srelplt;
      BFD_ASSERT (splt != NULL && sgot != NULL && srela != NULL);

      /* Get the index in the procedure linkage table which
         corresponds to this symbol.  This is the index of this symbol
         in all the symbols for which we are making plt entries.  The
         first entry in the procedure linkage table is reserved.  */
      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;

      /* Get the offset into the .got table of the entry that
        corresponds to this function.  Each .got entry is 4 bytes.
        The first three are reserved.  */
      got_offset = (plt_index + 3) * 4;
      got_addr = got_offset;

      /* Fill in the entry in the procedure linkage table.  */
      if (! info->shared)
        {
          got_addr += htab->sgotplt->output_section->vma
            + htab->sgotplt->output_offset;
          bfd_put_32 (output_bfd, PLT_ENTRY_WORD0 | ((got_addr >> 16) & 0xffff),
                      splt->contents + h->plt.offset);
          bfd_put_32 (output_bfd, PLT_ENTRY_WORD1 | (got_addr & 0xffff),
                      splt->contents + h->plt.offset + 4);
          bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD2,
                      splt->contents + h->plt.offset + 8);
          bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD3,
                      splt->contents + h->plt.offset + 12);
          bfd_put_32 (output_bfd, PLT_ENTRY_WORD4
                      | plt_index * sizeof (Elf32_External_Rela),
                      splt->contents + h->plt.offset + 16);
        }
      else
        {
          bfd_put_32 (output_bfd, PLT_PIC_ENTRY_WORD0 | (got_addr & 0xffff),
                      splt->contents + h->plt.offset);
          bfd_put_32 (output_bfd, PLT_PIC_ENTRY_WORD1
                      | plt_index * sizeof (Elf32_External_Rela),
                      splt->contents + h->plt.offset + 4);
          bfd_put_32 (output_bfd, (bfd_vma) PLT_PIC_ENTRY_WORD2,
                      splt->contents + h->plt.offset + 8);
          bfd_put_32 (output_bfd, (bfd_vma) PLT_PIC_ENTRY_WORD3,
                      splt->contents + h->plt.offset + 12);
          bfd_put_32 (output_bfd, (bfd_vma) PLT_PIC_ENTRY_WORD4,
                      splt->contents + h->plt.offset + 16);
        }

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
                  (splt->output_section->vma
                   + splt->output_offset), /* Same offset.  */
                  sgot->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgot->output_section->vma
                       + sgot->output_offset
                       + got_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_JMP_SLOT);
      rela.r_addend = 0;
      loc = srela->contents;
      loc += plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
        {
          /* Mark the symbol as undefined, rather than as defined in
             the .plt section.  Leave the value alone.  */
          sym->st_shndx = SHN_UNDEF;
        }

    }

  if (h->got.offset != (bfd_vma) -1
      && (h->got.offset & 2) == 0) /* Homemade TLS check.  */
    {
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the global offset table.  Set it
         up.  */
      sgot = htab->sgot;
      srela = htab->srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      rela.r_offset = (sgot->output_section->vma
                       + sgot->output_offset
                       + (h->got.offset &~ 1));

      /* If this is a -Bsymbolic link, and the symbol is defined
         locally, we just want to emit a RELATIVE reloc.  Likewise if
         the symbol was forced to be local because of a version file.
         The entry in the global offset table will already have been
         initialized in the relocate_section function.  */
      if (info->shared && SYMBOL_REFERENCES_LOCAL (info, h))
        {
          rela.r_info = ELF32_R_INFO (0, R_OR1K_RELATIVE);
          rela.r_addend = (h->root.u.def.value
                           + h->root.u.def.section->output_section->vma
                           + h->root.u.def.section->output_offset);
        }
      else
        {
          BFD_ASSERT ((h->got.offset & 1) == 0);
          bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + h->got.offset);
          rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_GLOB_DAT);
          rela.r_addend = 0;
        }

      loc = srela->contents;
      loc += srela->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++srela->reloc_count;
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;

      /* This symbols needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1
                  && (h->root.type == bfd_link_hash_defined
                      || h->root.type == bfd_link_hash_defweak));

      s = bfd_get_section_by_name (h->root.u.def.section->owner,
                                   ".rela.bss");
      BFD_ASSERT (s != NULL);

      rela.r_offset = (h->root.u.def.value
                       + h->root.u.def.section->output_section->vma
                       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_OR1K_COPY);
      rela.r_addend = 0;
      loc = s->contents;
      loc += s->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++s->reloc_count;
    }

  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || h == htab->root.hgot)
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

static enum elf_reloc_type_class
or1k_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
                           const asection *rel_sec ATTRIBUTE_UNUSED,
                           const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_OR1K_RELATIVE:  return reloc_class_relative;
    case R_OR1K_JMP_SLOT:  return reloc_class_plt;
    case R_OR1K_COPY:      return reloc_class_copy;
    default:               return reloc_class_normal;
    }
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bfd_boolean
or1k_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
                                struct elf_link_hash_entry *h)
{
  struct elf_or1k_link_hash_table *htab;
  struct elf_or1k_link_hash_entry *eh;
  struct elf_or1k_dyn_relocs *p;
  bfd *dynobj;
  asection *s;

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
              && (h->needs_plt
                  || h->u.weakdef != NULL
                  || (h->def_dynamic
                      && h->ref_regular
                      && !h->def_regular)));

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      if (! info->shared
          && !h->def_dynamic
          && !h->ref_dynamic
          && h->root.type != bfd_link_hash_undefweak
          && h->root.type != bfd_link_hash_undefined)
        {
          /* This case can occur if we saw a PLT reloc in an input
             file, but the symbol was never referred to by a dynamic
             object.  In such a case, we don't actually need to build
             a procedure linkage table, and we can just do a PCREL
             reloc instead.  */
          h->plt.offset = (bfd_vma) -1;
          h->needs_plt = 0;
        }

      return TRUE;
    }
  else
    h->plt.offset = (bfd_vma) -1;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
                  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
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

  eh = (struct elf_or1k_link_hash_entry *) h;
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      s = p->sec->output_section;
      if (s != NULL && (s->flags & (SEC_READONLY | SEC_HAS_CONTENTS)) != 0)
        break;
    }

  /* If we didn't find any dynamic relocs in sections which needs the
     copy reloc, then we'll be keeping the dynamic relocs and avoiding
     the copy reloc.  */
  if (p == NULL)
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
     same memory location for the variable.  */

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  s = htab->sdynbss;
  BFD_ASSERT (s != NULL);

  /* We must generate a R_OR1K_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      asection *srel;

      srel = htab->srelbss;
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bfd_boolean
allocate_dynrelocs (struct elf_link_hash_entry *h, void * inf)
{
  struct bfd_link_info *info;
  struct elf_or1k_link_hash_table *htab;
  struct elf_or1k_link_hash_entry *eh;
  struct elf_or1k_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  info = (struct bfd_link_info *) inf;
  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  eh = (struct elf_or1k_link_hash_entry *) h;

  if (htab->root.dynamic_sections_created
      && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
          && !h->forced_local)
        {
          if (! bfd_elf_link_record_dynamic_symbol (info, h))
            return FALSE;
        }

      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, info->shared, h))
        {
          asection *s = htab->splt;

          /* If this is the first .plt entry, make room for the special
             first entry.  */
          if (s->size == 0)
            s->size = PLT_ENTRY_SIZE;

          h->plt.offset = s->size;

          /* If this symbol is not defined in a regular file, and we are
             not generating a shared library, then set the symbol to this
             location in the .plt.  This is required to make function
             pointers compare as equal between the normal executable and
             the shared library.  */
          if (! info->shared
              && !h->def_regular)
            {
              h->root.u.def.section = s;
              h->root.u.def.value = h->plt.offset;
            }

          /* Make room for this entry.  */
          s->size += PLT_ENTRY_SIZE;

          /* We also need to make an entry in the .got.plt section, which
             will be placed in the .got section by the linker script.  */
          htab->sgotplt->size += 4;

          /* We also need to make an entry in the .rel.plt section.  */
          htab->srelplt->size += sizeof (Elf32_External_Rela);
        }
      else
        {
          h->plt.offset = (bfd_vma) -1;
          h->needs_plt = 0;
        }
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
    }

  if (h->got.refcount > 0)
    {
      asection *s;
      bfd_boolean dyn;
      unsigned char tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
          && !h->forced_local)
        {
          if (! bfd_elf_link_record_dynamic_symbol (info, h))
            return FALSE;
        }

      s = htab->sgot;

      h->got.offset = s->size;

      tls_type = ((struct elf_or1k_link_hash_entry *) h)->tls_type;

      /* TLS GD requires two GOT and two relocs.  */
      if (tls_type == TLS_GD)
        s->size += 8;
      else
        s->size += 4;
      dyn = htab->root.dynamic_sections_created;
      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, info->shared, h))
        {
          if (tls_type == TLS_GD)
            htab->srelgot->size += 2 * sizeof (Elf32_External_Rela);
          else
            htab->srelgot->size += sizeof (Elf32_External_Rela);
        }
    }
  else
    h->got.offset = (bfd_vma) -1;

  if (eh->dyn_relocs == NULL)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (info->shared)
    {
      if (SYMBOL_CALLS_LOCAL (info, h))
        {
          struct elf_or1k_dyn_relocs **pp;

          for (pp = &eh->dyn_relocs; (p = *pp) != NULL;)
            {
              p->count -= p->pc_count;
              p->pc_count = 0;
              if (p->count == 0)
                *pp = p->next;
              else
                pp = &p->next;
            }
        }

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
                   && !h->forced_local)
            {
              if (! bfd_elf_link_record_dynamic_symbol (info, h))
                return FALSE;
            }
        }
    }
  else
    {
      /* For the non-shared case, discard space for relocs against
         symbols which turn out to need copy relocs or are not
         dynamic.  */

      if (!h->non_got_ref
          && ((h->def_dynamic
               && !h->def_regular)
              || (htab->root.dynamic_sections_created
                  && (h->root.type == bfd_link_hash_undefweak
                      || h->root.type == bfd_link_hash_undefined))))
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
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return TRUE;
}

/* Find any dynamic relocs that apply to read-only sections.  */

static bfd_boolean
readonly_dynrelocs (struct elf_link_hash_entry *h, void * inf)
{
  struct elf_or1k_link_hash_entry *eh;
  struct elf_or1k_dyn_relocs *p;

  eh = (struct elf_or1k_link_hash_entry *) h;
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s != NULL && (s->flags & SEC_READONLY) != 0)
        {
          struct bfd_link_info *info = (struct bfd_link_info *) inf;

          info->flags |= DF_TEXTREL;

          /* Not an error, just cut short the traversal.  */
          return FALSE;
        }
    }
  return TRUE;
}

/* Set the sizes of the dynamic sections.  */

static bfd_boolean
or1k_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
                                struct bfd_link_info *info)
{
  struct elf_or1k_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd_boolean relocs;
  bfd *ibfd;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  dynobj = htab->root.dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->root.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
        {
          s = bfd_get_section_by_name (dynobj, ".interp");
          BFD_ASSERT (s != NULL);
          s->size = sizeof ELF_DYNAMIC_INTERPRETER;
          s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
        }
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      unsigned char *local_tls_type;
      asection *srel;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
        continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
        {
          struct elf_or1k_dyn_relocs *p;

          for (p = ((struct elf_or1k_dyn_relocs *)
                    elf_section_data (s)->local_dynrel);
               p != NULL;
               p = p->next)
            {
              if (! bfd_is_abs_section (p->sec)
                  && bfd_is_abs_section (p->sec->output_section))
                {
                  /* Input section has been discarded, either because
                     it is a copy of a linkonce section or due to
                     linker script /DISCARD/, so we'll be discarding
                     the relocs too.  */
                }
              else if (p->count != 0)
                {
                  srel = elf_section_data (p->sec)->sreloc;
                  srel->size += p->count * sizeof (Elf32_External_Rela);
                  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
                    info->flags |= DF_TEXTREL;
                }
            }
        }

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
        continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      s = htab->sgot;
      srel = htab->srelgot;
      local_tls_type = (unsigned char *) elf_or1k_local_tls_type (ibfd);
      for (; local_got < end_local_got; ++local_got)
        {
          if (*local_got > 0)
            {
              *local_got = s->size;

              /* TLS GD requires two GOT and two relocs.  */
              if (local_tls_type != NULL && *local_tls_type == TLS_GD)
                s->size += 8;
              else
                s->size += 4;
              if (info->shared)
                {
                  if (local_tls_type != NULL && *local_tls_type == TLS_GD)
                    srel->size += 2 * sizeof (Elf32_External_Rela);
                  else
                    srel->size += sizeof (Elf32_External_Rela);
                }
            }
          else

            *local_got = (bfd_vma) -1;

          if (local_tls_type)
            ++local_tls_type;
        }
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, allocate_dynrelocs, info);

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = FALSE;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
        continue;

      if (s == htab->splt
          || s == htab->sgot
          || s == htab->sgotplt
          || s == htab->sdynbss)
        {
          /* Strip this section if we don't need it; see the
             comment below.  */
        }
      else if (CONST_STRNEQ (bfd_get_section_name (dynobj, s), ".rela"))
        {
          if (s->size != 0 && s != htab->srelplt)
            relocs = TRUE;

          /* We use the reloc_count field as a counter if we need
             to copy relocs into the output file.  */
          s->reloc_count = 0;
        }
      else
        /* It's not one of our sections, so don't allocate space.  */
        continue;

      if (s->size == 0)
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

      /* Allocate memory for the section contents.  We use bfd_zalloc
         here in case unused entries are not reclaimed before the
         section's contents are written out.  This should not happen,
         but this way if it does, we get a R_OR1K_NONE reloc instead
         of garbage.  */
      s->contents = bfd_zalloc (dynobj, s->size);

      if (s->contents == NULL)
        return FALSE;
    }

  if (htab->root.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
         values later, in or1k_elf_finish_dynamic_sections, but we
         must add the entries now so that we get the correct size for
         the .dynamic section.  The DT_DEBUG entry is filled in by the
         dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

     if (info->executable)
       {
         if (! add_dynamic_entry (DT_DEBUG, 0))
           return FALSE;
       }

     if (htab->splt->size != 0)
       {
         if (! add_dynamic_entry (DT_PLTGOT, 0)
             || ! add_dynamic_entry (DT_PLTRELSZ, 0)
             || ! add_dynamic_entry (DT_PLTREL, DT_RELA)
             || ! add_dynamic_entry (DT_JMPREL, 0))
           return FALSE;
        }

     if (relocs)
       {
         if (! add_dynamic_entry (DT_RELA, 0)
             || ! add_dynamic_entry (DT_RELASZ, 0)
             || ! add_dynamic_entry (DT_RELAENT,
                                     sizeof (Elf32_External_Rela)))
           return FALSE;

         /* If any dynamic relocs apply to a read-only section,
            then we need a DT_TEXTREL entry.  */
         if ((info->flags & DF_TEXTREL) == 0)
           elf_link_hash_traverse (&htab->root, readonly_dynrelocs,
                                   info);

         if ((info->flags & DF_TEXTREL) != 0)
           {
             if (! add_dynamic_entry (DT_TEXTREL, 0))
               return FALSE;
           }
       }
    }

#undef add_dynamic_entry
  return TRUE;
}

/* Create dynamic sections when linking against a dynamic object.  */

static bfd_boolean
or1k_elf_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf_or1k_link_hash_table *htab;

  htab = or1k_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (!htab->sgot && !create_got_section (dynobj, info))
    return FALSE;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return FALSE;

  htab->splt = bfd_get_section_by_name (dynobj, ".plt");
  htab->srelplt = bfd_get_section_by_name (dynobj, ".rela.plt");
  htab->sdynbss = bfd_get_section_by_name (dynobj, ".dynbss");
  if (!info->shared)
    htab->srelbss = bfd_get_section_by_name (dynobj, ".rela.bss");

  if (!htab->splt || !htab->srelplt || !htab->sdynbss
      || (!info->shared && !htab->srelbss))
    abort ();

  return TRUE;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
or1k_elf_copy_indirect_symbol (struct bfd_link_info *info,
                               struct elf_link_hash_entry *dir,
                               struct elf_link_hash_entry *ind)
{
  struct elf_or1k_link_hash_entry * edir;
  struct elf_or1k_link_hash_entry * eind;

  edir = (struct elf_or1k_link_hash_entry *) dir;
  eind = (struct elf_or1k_link_hash_entry *) ind;

  if (eind->dyn_relocs != NULL)
    {
      if (edir->dyn_relocs != NULL)
        {
          struct elf_or1k_dyn_relocs **pp;
          struct elf_or1k_dyn_relocs *p;

          /* Add reloc counts against the indirect sym to the direct sym
             list.  Merge any entries against the same section.  */
          for (pp = &eind->dyn_relocs; (p = *pp) != NULL;)
            {
              struct elf_or1k_dyn_relocs *q;

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

  if (ind->root.type == bfd_link_hash_indirect)
    {
      if (dir->got.refcount <= 0)
        {
          edir->tls_type = eind->tls_type;
          eind->tls_type = TLS_UNKNOWN;
        }
    }

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Set the right machine number.  */

static bfd_boolean
or1k_elf_object_p (bfd *abfd)
{
  unsigned long mach = bfd_mach_or1k;

  if (elf_elfheader (abfd)->e_flags & EF_OR1K_NODELAY)
    mach = bfd_mach_or1knd;

  return bfd_default_set_arch_mach (abfd, bfd_arch_or1k, mach);
}

/* Store the machine number in the flags field.  */

static void
or1k_elf_final_write_processing (bfd *abfd,
				 bfd_boolean linker ATTRIBUTE_UNUSED)
{
  switch (bfd_get_mach (abfd))
    {
    default:
    case bfd_mach_or1k:
      break;
    case bfd_mach_or1knd:
      elf_elfheader (abfd)->e_flags |= EF_OR1K_NODELAY;
      break;
    }
}

static bfd_boolean
or1k_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
              || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = TRUE;
  return TRUE;
}

/* Make sure all input files are consistent with respect to
   EF_OR1K_NODELAY flag setting.  */

static bfd_boolean
elf32_or1k_merge_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  flagword out_flags;
  flagword in_flags;

  in_flags  = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  if (!elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = TRUE;
      elf_elfheader (obfd)->e_flags = in_flags;

      return TRUE;
    }

  if (in_flags == out_flags)
    return TRUE;

  if ((in_flags & EF_OR1K_NODELAY) != (out_flags & EF_OR1K_NODELAY))
    {
      (*_bfd_error_handler)
	(_("%B: EF_OR1K_NODELAY flag mismatch with previous modules"), ibfd);

      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  return TRUE;

}

#define ELF_ARCH                        bfd_arch_or1k
#define ELF_MACHINE_CODE                EM_OR1K
#define ELF_TARGET_ID                   OR1K_ELF_DATA
#define ELF_MAXPAGESIZE                 0x2000

#define TARGET_BIG_SYM                  or1k_elf32_vec
#define TARGET_BIG_NAME                 "elf32-or1k"

#define elf_info_to_howto_rel           NULL
#define elf_info_to_howto               or1k_info_to_howto_rela
#define elf_backend_relocate_section    or1k_elf_relocate_section
#define elf_backend_gc_mark_hook        or1k_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook       or1k_elf_gc_sweep_hook
#define elf_backend_check_relocs        or1k_elf_check_relocs
#define elf_backend_reloc_type_class    or1k_elf_reloc_type_class
#define elf_backend_can_gc_sections     1
#define elf_backend_rela_normal         1

#define bfd_elf32_mkobject                   elf_or1k_mkobject

#define bfd_elf32_bfd_merge_private_bfd_data elf32_or1k_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags or1k_elf_set_private_flags
#define bfd_elf32_bfd_reloc_type_lookup or1k_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup or1k_reloc_name_lookup

#define elf_backend_object_p                or1k_elf_object_p
#define elf_backend_final_write_processing  or1k_elf_final_write_processing
#define elf_backend_can_refcount                1

#define elf_backend_plt_readonly                1
#define elf_backend_want_got_plt                1
#define elf_backend_want_plt_sym                0
#define elf_backend_got_header_size             12
#define bfd_elf32_bfd_link_hash_table_create    or1k_elf_link_hash_table_create
#define elf_backend_copy_indirect_symbol        or1k_elf_copy_indirect_symbol
#define elf_backend_create_dynamic_sections     or1k_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections     or1k_elf_finish_dynamic_sections
#define elf_backend_size_dynamic_sections       or1k_elf_size_dynamic_sections
#define elf_backend_adjust_dynamic_symbol       or1k_elf_adjust_dynamic_symbol
#define elf_backend_finish_dynamic_symbol       or1k_elf_finish_dynamic_symbol

#include "elf32-target.h"

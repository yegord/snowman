/* AArch64-specific support for NN-bit ELF.
   Copyright (C) 2009-2014 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

/* Notes on implementation:

  Thread Local Store (TLS)

  Overview:

  The implementation currently supports both traditional TLS and TLS
  descriptors, but only general dynamic (GD).

  For traditional TLS the assembler will present us with code
  fragments of the form:

  adrp x0, :tlsgd:foo
                           R_AARCH64_TLSGD_ADR_PAGE21(foo)
  add  x0, :tlsgd_lo12:foo
                           R_AARCH64_TLSGD_ADD_LO12_NC(foo)
  bl   __tls_get_addr
  nop

  For TLS descriptors the assembler will present us with code
  fragments of the form:

  adrp  x0, :tlsdesc:foo                      R_AARCH64_TLSDESC_ADR_PAGE21(foo)
  ldr   x1, [x0, #:tlsdesc_lo12:foo]          R_AARCH64_TLSDESC_LD64_LO12(foo)
  add   x0, x0, #:tlsdesc_lo12:foo            R_AARCH64_TLSDESC_ADD_LO12(foo)
  .tlsdesccall foo
  blr   x1                                    R_AARCH64_TLSDESC_CALL(foo)

  The relocations R_AARCH64_TLSGD_{ADR_PREL21,ADD_LO12_NC} against foo
  indicate that foo is thread local and should be accessed via the
  traditional TLS mechanims.

  The relocations R_AARCH64_TLSDESC_{ADR_PAGE21,LD64_LO12_NC,ADD_LO12_NC}
  against foo indicate that 'foo' is thread local and should be accessed
  via a TLS descriptor mechanism.

  The precise instruction sequence is only relevant from the
  perspective of linker relaxation which is currently not implemented.

  The static linker must detect that 'foo' is a TLS object and
  allocate a double GOT entry. The GOT entry must be created for both
  global and local TLS symbols. Note that this is different to none
  TLS local objects which do not need a GOT entry.

  In the traditional TLS mechanism, the double GOT entry is used to
  provide the tls_index structure, containing module and offset
  entries. The static linker places the relocation R_AARCH64_TLS_DTPMOD
  on the module entry. The loader will subsequently fixup this
  relocation with the module identity.

  For global traditional TLS symbols the static linker places an
  R_AARCH64_TLS_DTPREL relocation on the offset entry. The loader
  will subsequently fixup the offset. For local TLS symbols the static
  linker fixes up offset.

  In the TLS descriptor mechanism the double GOT entry is used to
  provide the descriptor. The static linker places the relocation
  R_AARCH64_TLSDESC on the first GOT slot. The loader will
  subsequently fix this up.

  Implementation:

  The handling of TLS symbols is implemented across a number of
  different backend functions. The following is a top level view of
  what processing is performed where.

  The TLS implementation maintains state information for each TLS
  symbol. The state information for local and global symbols is kept
  in different places. Global symbols use generic BFD structures while
  local symbols use backend specific structures that are allocated and
  maintained entirely by the backend.

  The flow:

  elfNN_aarch64_check_relocs()

  This function is invoked for each relocation.

  The TLS relocations R_AARCH64_TLSGD_{ADR_PREL21,ADD_LO12_NC} and
  R_AARCH64_TLSDESC_{ADR_PAGE21,LD64_LO12_NC,ADD_LO12_NC} are
  spotted. One time creation of local symbol data structures are
  created when the first local symbol is seen.

  The reference count for a symbol is incremented.  The GOT type for
  each symbol is marked as general dynamic.

  elfNN_aarch64_allocate_dynrelocs ()

  For each global with positive reference count we allocate a double
  GOT slot. For a traditional TLS symbol we allocate space for two
  relocation entries on the GOT, for a TLS descriptor symbol we
  allocate space for one relocation on the slot. Record the GOT offset
  for this symbol.

  elfNN_aarch64_size_dynamic_sections ()

  Iterate all input BFDS, look for in the local symbol data structure
  constructed earlier for local TLS symbols and allocate them double
  GOT slots along with space for a single GOT relocation. Update the
  local symbol structure to record the GOT offset allocated.

  elfNN_aarch64_relocate_section ()

  Calls elfNN_aarch64_final_link_relocate ()

  Emit the relevant TLS relocations against the GOT for each TLS
  symbol. For local TLS symbols emit the GOT offset directly. The GOT
  relocations are emitted once the first time a TLS symbol is
  encountered. The implementation uses the LSB of the GOT offset to
  flag that the relevant GOT relocations for a symbol have been
  emitted. All of the TLS code that uses the GOT offset needs to take
  care to mask out this flag bit before using the offset.

  elfNN_aarch64_final_link_relocate ()

  Fixup the R_AARCH64_TLSGD_{ADR_PREL21, ADD_LO12_NC} relocations.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "libbfd.h"
#include "bfd_stdint.h"
#include "elf-bfd.h"
#include "bfdlink.h"
#include "objalloc.h"
#include "elf/aarch64.h"
#include "elfxx-aarch64.h"

#define ARCH_SIZE	NN

#if ARCH_SIZE == 64
#define AARCH64_R(NAME)		R_AARCH64_ ## NAME
#define AARCH64_R_STR(NAME)	"R_AARCH64_" #NAME
#define HOWTO64(...)		HOWTO (__VA_ARGS__)
#define HOWTO32(...)		EMPTY_HOWTO (0)
#define LOG_FILE_ALIGN	3
#endif

#if ARCH_SIZE == 32
#define AARCH64_R(NAME)		R_AARCH64_P32_ ## NAME
#define AARCH64_R_STR(NAME)	"R_AARCH64_P32_" #NAME
#define HOWTO64(...)		EMPTY_HOWTO (0)
#define HOWTO32(...)		HOWTO (__VA_ARGS__)
#define LOG_FILE_ALIGN	2
#endif

#define IS_AARCH64_TLS_RELOC(R_TYPE)				\
  ((R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLS_DTPMOD			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLS_DTPREL			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLS_TPREL			\
   || IS_AARCH64_TLSDESC_RELOC ((R_TYPE)))

#define IS_AARCH64_TLSDESC_RELOC(R_TYPE)			\
  ((R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD_PREL19		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD64_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_OFF_G1		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LDR			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADD			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_CALL		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC)

#define ELIMINATE_COPY_RELOCS 0

/* Return size of a relocation entry.  HTAB is the bfd's
   elf_aarch64_link_hash_entry.  */
#define RELOC_SIZE(HTAB) (sizeof (ElfNN_External_Rela))

/* GOT Entry size - 8 bytes in ELF64 and 4 bytes in ELF32.  */
#define GOT_ENTRY_SIZE                  (ARCH_SIZE / 8)
#define PLT_ENTRY_SIZE                  (32)
#define PLT_SMALL_ENTRY_SIZE            (16)
#define PLT_TLSDESC_ENTRY_SIZE          (32)

/* Encoding of the nop instruction */
#define INSN_NOP 0xd503201f

#define aarch64_compute_jump_table_size(htab)		\
  (((htab)->root.srelplt == NULL) ? 0			\
   : (htab)->root.srelplt->reloc_count * GOT_ENTRY_SIZE)

/* The first entry in a procedure linkage table looks like this
   if the distance between the PLTGOT and the PLT is < 4GB use
   these PLT entries. Note that the dynamic linker gets &PLTGOT[2]
   in x16 and needs to work out PLTGOT[1] by using an address of
   [x16,#-GOT_ENTRY_SIZE].  */
static const bfd_byte elfNN_aarch64_small_plt0_entry[PLT_ENTRY_SIZE] =
{
  0xf0, 0x7b, 0xbf, 0xa9,	/* stp x16, x30, [sp, #-16]!  */
  0x10, 0x00, 0x00, 0x90,	/* adrp x16, (GOT+16)  */
#if ARCH_SIZE == 64
  0x11, 0x0A, 0x40, 0xf9,	/* ldr x17, [x16, #PLT_GOT+0x10]  */
  0x10, 0x42, 0x00, 0x91,	/* add x16, x16,#PLT_GOT+0x10   */
#else
  0x11, 0x0A, 0x40, 0xb9,	/* ldr w17, [x16, #PLT_GOT+0x8]  */
  0x10, 0x22, 0x00, 0x11,	/* add w16, w16,#PLT_GOT+0x8   */
#endif
  0x20, 0x02, 0x1f, 0xd6,	/* br x17  */
  0x1f, 0x20, 0x03, 0xd5,	/* nop */
  0x1f, 0x20, 0x03, 0xd5,	/* nop */
  0x1f, 0x20, 0x03, 0xd5,	/* nop */
};

/* Per function entry in a procedure linkage table looks like this
   if the distance between the PLTGOT and the PLT is < 4GB use
   these PLT entries.  */
static const bfd_byte elfNN_aarch64_small_plt_entry[PLT_SMALL_ENTRY_SIZE] =
{
  0x10, 0x00, 0x00, 0x90,	/* adrp x16, PLTGOT + n * 8  */
#if ARCH_SIZE == 64
  0x11, 0x02, 0x40, 0xf9,	/* ldr x17, [x16, PLTGOT + n * 8] */
  0x10, 0x02, 0x00, 0x91,	/* add x16, x16, :lo12:PLTGOT + n * 8  */
#else
  0x11, 0x02, 0x40, 0xb9,	/* ldr w17, [x16, PLTGOT + n * 4] */
  0x10, 0x02, 0x00, 0x11,	/* add w16, w16, :lo12:PLTGOT + n * 4  */
#endif
  0x20, 0x02, 0x1f, 0xd6,	/* br x17.  */
};

static const bfd_byte
elfNN_aarch64_tlsdesc_small_plt_entry[PLT_TLSDESC_ENTRY_SIZE] =
{
  0xe2, 0x0f, 0xbf, 0xa9,	/* stp x2, x3, [sp, #-16]! */
  0x02, 0x00, 0x00, 0x90,	/* adrp x2, 0 */
  0x03, 0x00, 0x00, 0x90,	/* adrp x3, 0 */
#if ARCH_SIZE == 64
  0x42, 0x00, 0x40, 0xf9,	/* ldr x2, [x2, #0] */
  0x63, 0x00, 0x00, 0x91,	/* add x3, x3, 0 */
#else
  0x42, 0x00, 0x40, 0xb9,	/* ldr w2, [x2, #0] */
  0x63, 0x00, 0x00, 0x11,	/* add w3, w3, 0 */
#endif
  0x40, 0x00, 0x1f, 0xd6,	/* br x2 */
  0x1f, 0x20, 0x03, 0xd5,	/* nop */
  0x1f, 0x20, 0x03, 0xd5,	/* nop */
};

#define elf_info_to_howto               elfNN_aarch64_info_to_howto
#define elf_info_to_howto_rel           elfNN_aarch64_info_to_howto

#define AARCH64_ELF_ABI_VERSION		0

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define ALL_ONES (~ (bfd_vma) 0)

/* Indexed by the bfd interal reloc enumerators.
   Therefore, the table needs to be synced with BFD_RELOC_AARCH64_*
   in reloc.c.   */

static reloc_howto_type elfNN_aarch64_howto_table[] =
{
  EMPTY_HOWTO (0),

  /* Basic data relocations.  */

#if ARCH_SIZE == 64
  HOWTO (R_AARCH64_NULL,	/* type */
	 0,			/* rightshift */
	 3,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_AARCH64_NULL",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */
#else
  HOWTO (R_AARCH64_NONE,	/* type */
	 0,			/* rightshift */
	 3,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_AARCH64_NONE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */
#endif

  /* .xword: (S+A) */
  HOWTO64 (AARCH64_R (ABS64),	/* type */
	 0,			/* rightshift */
	 4,			/* size (4 = long long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ABS64),	/* name */
	 FALSE,			/* partial_inplace */
	 ALL_ONES,		/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* .word: (S+A) */
  HOWTO (AARCH64_R (ABS32),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ABS32),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* .half:  (S+A) */
  HOWTO (AARCH64_R (ABS16),	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ABS16),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* .xword: (S+A-P) */
  HOWTO64 (AARCH64_R (PREL64),	/* type */
	 0,			/* rightshift */
	 4,			/* size (4 = long long) */
	 64,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (PREL64),	/* name */
	 FALSE,			/* partial_inplace */
	 ALL_ONES,		/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* .word: (S+A-P) */
  HOWTO (AARCH64_R (PREL32),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (PREL32),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* .half: (S+A-P) */
  HOWTO (AARCH64_R (PREL16),	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (PREL16),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* Group relocations to create a 16, 32, 48 or 64 bit
     unsigned data or abs address inline.  */

  /* MOVZ:   ((S+A) >>  0) & 0xffff */
  HOWTO (AARCH64_R (MOVW_UABS_G0),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_UABS_G0),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOVK:   ((S+A) >>  0) & 0xffff [no overflow check] */
  HOWTO (AARCH64_R (MOVW_UABS_G0_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_UABS_G0_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOVZ:   ((S+A) >> 16) & 0xffff */
  HOWTO (AARCH64_R (MOVW_UABS_G1),	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_UABS_G1),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOVK:   ((S+A) >> 16) & 0xffff [no overflow check] */
  HOWTO64 (AARCH64_R (MOVW_UABS_G1_NC),	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_UABS_G1_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOVZ:   ((S+A) >> 32) & 0xffff */
  HOWTO64 (AARCH64_R (MOVW_UABS_G2),	/* type */
	 32,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_UABS_G2),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOVK:   ((S+A) >> 32) & 0xffff [no overflow check] */
  HOWTO64 (AARCH64_R (MOVW_UABS_G2_NC),	/* type */
	 32,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_UABS_G2_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOVZ:   ((S+A) >> 48) & 0xffff */
  HOWTO64 (AARCH64_R (MOVW_UABS_G3),	/* type */
	 48,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_UABS_G3),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Group relocations to create high part of a 16, 32, 48 or 64 bit
     signed data or abs address inline. Will change instruction
     to MOVN or MOVZ depending on sign of calculated value.  */

  /* MOV[ZN]:   ((S+A) >>  0) & 0xffff */
  HOWTO (AARCH64_R (MOVW_SABS_G0),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_SABS_G0),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOV[ZN]:   ((S+A) >> 16) & 0xffff */
  HOWTO64 (AARCH64_R (MOVW_SABS_G1),	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_SABS_G1),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* MOV[ZN]:   ((S+A) >> 32) & 0xffff */
  HOWTO64 (AARCH64_R (MOVW_SABS_G2),	/* type */
	 32,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (MOVW_SABS_G2),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

/* Relocations to generate 19, 21 and 33 bit PC-relative load/store
   addresses: PG(x) is (x & ~0xfff).  */

  /* LD-lit: ((S+A-P) >> 2) & 0x7ffff */
  HOWTO (AARCH64_R (LD_PREL_LO19),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 19,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LD_PREL_LO19),	/* name */
	 FALSE,			/* partial_inplace */
	 0x7ffff,		/* src_mask */
	 0x7ffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* ADR:    (S+A-P) & 0x1fffff */
  HOWTO (AARCH64_R (ADR_PREL_LO21),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ADR_PREL_LO21),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* ADRP:   ((PG(S+A)-PG(P)) >> 12) & 0x1fffff */
  HOWTO (AARCH64_R (ADR_PREL_PG_HI21),	/* type */
	 12,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ADR_PREL_PG_HI21),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* ADRP:   ((PG(S+A)-PG(P)) >> 12) & 0x1fffff [no overflow check] */
  HOWTO64 (AARCH64_R (ADR_PREL_PG_HI21_NC),	/* type */
	 12,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ADR_PREL_PG_HI21_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* ADD:    (S+A) & 0xfff [no overflow check] */
  HOWTO (AARCH64_R (ADD_ABS_LO12_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 10,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ADD_ABS_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0x3ffc00,		/* src_mask */
	 0x3ffc00,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* LD/ST8:  (S+A) & 0xfff */
  HOWTO (AARCH64_R (LDST8_ABS_LO12_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LDST8_ABS_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xfff,			/* src_mask */
	 0xfff,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Relocations for control-flow instructions.  */

  /* TBZ/NZ: ((S+A-P) >> 2) & 0x3fff */
  HOWTO (AARCH64_R (TSTBR14),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 14,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TSTBR14),	/* name */
	 FALSE,			/* partial_inplace */
	 0x3fff,		/* src_mask */
	 0x3fff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* B.cond: ((S+A-P) >> 2) & 0x7ffff */
  HOWTO (AARCH64_R (CONDBR19),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 19,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (CONDBR19),	/* name */
	 FALSE,			/* partial_inplace */
	 0x7ffff,		/* src_mask */
	 0x7ffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* B:      ((S+A-P) >> 2) & 0x3ffffff */
  HOWTO (AARCH64_R (JUMP26),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 26,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (JUMP26),	/* name */
	 FALSE,			/* partial_inplace */
	 0x3ffffff,		/* src_mask */
	 0x3ffffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* BL:     ((S+A-P) >> 2) & 0x3ffffff */
  HOWTO (AARCH64_R (CALL26),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 26,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (CALL26),	/* name */
	 FALSE,			/* partial_inplace */
	 0x3ffffff,		/* src_mask */
	 0x3ffffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* LD/ST16:  (S+A) & 0xffe */
  HOWTO (AARCH64_R (LDST16_ABS_LO12_NC),	/* type */
	 1,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LDST16_ABS_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffe,			/* src_mask */
	 0xffe,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* LD/ST32:  (S+A) & 0xffc */
  HOWTO (AARCH64_R (LDST32_ABS_LO12_NC),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LDST32_ABS_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffc,			/* src_mask */
	 0xffc,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* LD/ST64:  (S+A) & 0xff8 */
  HOWTO (AARCH64_R (LDST64_ABS_LO12_NC),	/* type */
	 3,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LDST64_ABS_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xff8,			/* src_mask */
	 0xff8,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* LD/ST128:  (S+A) & 0xff0 */
  HOWTO (AARCH64_R (LDST128_ABS_LO12_NC),	/* type */
	 4,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LDST128_ABS_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xff0,			/* src_mask */
	 0xff0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Set a load-literal immediate field to bits
     0x1FFFFC of G(S)-P */
  HOWTO (AARCH64_R (GOT_LD_PREL19),	/* type */
	 2,				/* rightshift */
	 2,				/* size (0 = byte,1 = short,2 = long) */
	 19,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 AARCH64_R_STR (GOT_LD_PREL19),	/* name */
	 FALSE,				/* partial_inplace */
	 0xffffe0,			/* src_mask */
	 0xffffe0,			/* dst_mask */
	 TRUE),				/* pcrel_offset */

  /* Get to the page for the GOT entry for the symbol
     (G(S) - P) using an ADRP instruction.  */
  HOWTO (AARCH64_R (ADR_GOT_PAGE),	/* type */
	 12,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (ADR_GOT_PAGE),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* LD64: GOT offset G(S) & 0xff8  */
  HOWTO64 (AARCH64_R (LD64_GOT_LO12_NC),	/* type */
	 3,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LD64_GOT_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xff8,			/* src_mask */
	 0xff8,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* LD32: GOT offset G(S) & 0xffc  */
  HOWTO32 (AARCH64_R (LD32_GOT_LO12_NC),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (LD32_GOT_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffc,			/* src_mask */
	 0xffc,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Get to the page for the GOT entry for the symbol
     (G(S) - P) using an ADRP instruction.  */
  HOWTO (AARCH64_R (TLSGD_ADR_PAGE21),	/* type */
	 12,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSGD_ADR_PAGE21),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* ADD: GOT offset G(S) & 0xff8 [no overflow check] */
  HOWTO (AARCH64_R (TLSGD_ADD_LO12_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSGD_ADD_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xfff,			/* src_mask */
	 0xfff,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSIE_MOVW_GOTTPREL_G1),	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSIE_MOVW_GOTTPREL_G1),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSIE_MOVW_GOTTPREL_G0_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSIE_MOVW_GOTTPREL_G0_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSIE_ADR_GOTTPREL_PAGE21),	/* type */
	 12,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSIE_ADR_GOTTPREL_PAGE21),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSIE_LD64_GOTTPREL_LO12_NC),	/* type */
	 3,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSIE_LD64_GOTTPREL_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xff8,			/* src_mask */
	 0xff8,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO32 (AARCH64_R (TLSIE_LD32_GOTTPREL_LO12_NC),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSIE_LD32_GOTTPREL_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffc,			/* src_mask */
	 0xffc,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSIE_LD_GOTTPREL_PREL19),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSIE_LD_GOTTPREL_PREL19),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1ffffc,		/* src_mask */
	 0x1ffffc,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSLE_MOVW_TPREL_G2),	/* type */
	 32,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_MOVW_TPREL_G2),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSLE_MOVW_TPREL_G1),	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_MOVW_TPREL_G1),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSLE_MOVW_TPREL_G1_NC),	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_MOVW_TPREL_G1_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSLE_MOVW_TPREL_G0),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_MOVW_TPREL_G0),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSLE_MOVW_TPREL_G0_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_MOVW_TPREL_G0_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSLE_ADD_TPREL_HI12),	/* type */
	 12,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_ADD_TPREL_HI12),	/* name */
	 FALSE,			/* partial_inplace */
	 0xfff,			/* src_mask */
	 0xfff,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSLE_ADD_TPREL_LO12),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_ADD_TPREL_LO12),	/* name */
	 FALSE,			/* partial_inplace */
	 0xfff,			/* src_mask */
	 0xfff,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSLE_ADD_TPREL_LO12_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSLE_ADD_TPREL_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xfff,			/* src_mask */
	 0xfff,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSDESC_LD_PREL19),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_LD_PREL19),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1ffffc,		/* src_mask */
	 0x1ffffc,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  HOWTO (AARCH64_R (TLSDESC_ADR_PREL21),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_ADR_PREL21),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* Get to the page for the GOT entry for the symbol
     (G(S) - P) using an ADRP instruction.  */
  HOWTO (AARCH64_R (TLSDESC_ADR_PAGE21),	/* type */
	 12,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_ADR_PAGE21),	/* name */
	 FALSE,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* LD64: GOT offset G(S) & 0xff8.  */
  HOWTO64 (AARCH64_R (TLSDESC_LD64_LO12_NC),	/* type */
	 3,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_LD64_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xff8,			/* src_mask */
	 0xff8,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* LD32: GOT offset G(S) & 0xffc.  */
  HOWTO32 (AARCH64_R (TLSDESC_LD32_LO12_NC),	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_LD32_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffc,			/* src_mask */
	 0xffc,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* ADD: GOT offset G(S) & 0xfff.  */
  HOWTO (AARCH64_R (TLSDESC_ADD_LO12_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_ADD_LO12_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xfff,			/* src_mask */
	 0xfff,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSDESC_OFF_G1),	/* type */
	 16,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_OFF_G1),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSDESC_OFF_G0_NC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_OFF_G0_NC),	/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSDESC_LDR),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_LDR),	/* name */
	 FALSE,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0x0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO64 (AARCH64_R (TLSDESC_ADD),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_ADD),	/* name */
	 FALSE,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0x0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSDESC_CALL),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC_CALL),	/* name */
	 FALSE,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0x0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (COPY),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (COPY),	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (GLOB_DAT),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (GLOB_DAT),	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (JUMP_SLOT),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (JUMP_SLOT),	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (RELATIVE),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (RELATIVE),	/* name */
	 TRUE,			/* partial_inplace */
	 ALL_ONES,		/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLS_DTPMOD),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
#if ARCH_SIZE == 64
	 AARCH64_R_STR (TLS_DTPMOD64),	/* name */
#else
	 AARCH64_R_STR (TLS_DTPMOD),	/* name */
#endif
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 FALSE),		/* pc_reloffset */

  HOWTO (AARCH64_R (TLS_DTPREL),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
#if ARCH_SIZE == 64
	 AARCH64_R_STR (TLS_DTPREL64),	/* name */
#else
	 AARCH64_R_STR (TLS_DTPREL),	/* name */
#endif
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLS_TPREL),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
#if ARCH_SIZE == 64
	 AARCH64_R_STR (TLS_TPREL64),	/* name */
#else
	 AARCH64_R_STR (TLS_TPREL),	/* name */
#endif
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (TLSDESC),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (TLSDESC),	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (AARCH64_R (IRELATIVE),	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 AARCH64_R_STR (IRELATIVE),	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ALL_ONES,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (0),
};

static reloc_howto_type elfNN_aarch64_howto_none =
  HOWTO (R_AARCH64_NONE,	/* type */
	 0,			/* rightshift */
	 3,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_AARCH64_NONE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE);		/* pcrel_offset */

/* Given HOWTO, return the bfd internal relocation enumerator.  */

static bfd_reloc_code_real_type
elfNN_aarch64_bfd_reloc_from_howto (reloc_howto_type *howto)
{
  const int size
    = (int) ARRAY_SIZE (elfNN_aarch64_howto_table);
  const ptrdiff_t offset
    = howto - elfNN_aarch64_howto_table;

  if (offset > 0 && offset < size - 1)
    return BFD_RELOC_AARCH64_RELOC_START + offset;

  if (howto == &elfNN_aarch64_howto_none)
    return BFD_RELOC_AARCH64_NONE;

  return BFD_RELOC_AARCH64_RELOC_START;
}

/* Given R_TYPE, return the bfd internal relocation enumerator.  */

static bfd_reloc_code_real_type
elfNN_aarch64_bfd_reloc_from_type (unsigned int r_type)
{
  static bfd_boolean initialized_p = FALSE;
  /* Indexed by R_TYPE, values are offsets in the howto_table.  */
  static unsigned int offsets[R_AARCH64_end];

  if (initialized_p == FALSE)
    {
      unsigned int i;

      for (i = 1; i < ARRAY_SIZE (elfNN_aarch64_howto_table) - 1; ++i)
	if (elfNN_aarch64_howto_table[i].type != 0)
	  offsets[elfNN_aarch64_howto_table[i].type] = i;

      initialized_p = TRUE;
    }

  if (r_type == R_AARCH64_NONE || r_type == R_AARCH64_NULL)
    return BFD_RELOC_AARCH64_NONE;

  /* PR 17512: file: b371e70a.  */
  if (r_type >= R_AARCH64_end)
    {
      _bfd_error_handler (_("Invalid AArch64 reloc number: %d"), r_type);
      bfd_set_error (bfd_error_bad_value);
      return BFD_RELOC_AARCH64_NONE;
    }

  return BFD_RELOC_AARCH64_RELOC_START + offsets[r_type];
}

struct elf_aarch64_reloc_map
{
  bfd_reloc_code_real_type from;
  bfd_reloc_code_real_type to;
};

/* Map bfd generic reloc to AArch64-specific reloc.  */
static const struct elf_aarch64_reloc_map elf_aarch64_reloc_map[] =
{
  {BFD_RELOC_NONE, BFD_RELOC_AARCH64_NONE},

  /* Basic data relocations.  */
  {BFD_RELOC_CTOR, BFD_RELOC_AARCH64_NN},
  {BFD_RELOC_64, BFD_RELOC_AARCH64_64},
  {BFD_RELOC_32, BFD_RELOC_AARCH64_32},
  {BFD_RELOC_16, BFD_RELOC_AARCH64_16},
  {BFD_RELOC_64_PCREL, BFD_RELOC_AARCH64_64_PCREL},
  {BFD_RELOC_32_PCREL, BFD_RELOC_AARCH64_32_PCREL},
  {BFD_RELOC_16_PCREL, BFD_RELOC_AARCH64_16_PCREL},
};

/* Given the bfd internal relocation enumerator in CODE, return the
   corresponding howto entry.  */

static reloc_howto_type *
elfNN_aarch64_howto_from_bfd_reloc (bfd_reloc_code_real_type code)
{
  unsigned int i;

  /* Convert bfd generic reloc to AArch64-specific reloc.  */
  if (code < BFD_RELOC_AARCH64_RELOC_START
      || code > BFD_RELOC_AARCH64_RELOC_END)
    for (i = 0; i < ARRAY_SIZE (elf_aarch64_reloc_map); i++)
      if (elf_aarch64_reloc_map[i].from == code)
	{
	  code = elf_aarch64_reloc_map[i].to;
	  break;
	}

  if (code > BFD_RELOC_AARCH64_RELOC_START
      && code < BFD_RELOC_AARCH64_RELOC_END)
    if (elfNN_aarch64_howto_table[code - BFD_RELOC_AARCH64_RELOC_START].type)
      return &elfNN_aarch64_howto_table[code - BFD_RELOC_AARCH64_RELOC_START];

  if (code == BFD_RELOC_AARCH64_NONE)
    return &elfNN_aarch64_howto_none;

  return NULL;
}

static reloc_howto_type *
elfNN_aarch64_howto_from_type (unsigned int r_type)
{
  bfd_reloc_code_real_type val;
  reloc_howto_type *howto;

#if ARCH_SIZE == 32
  if (r_type > 256)
    {
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
#endif

  if (r_type == R_AARCH64_NONE)
    return &elfNN_aarch64_howto_none;

  val = elfNN_aarch64_bfd_reloc_from_type (r_type);
  howto = elfNN_aarch64_howto_from_bfd_reloc (val);

  if (howto != NULL)
    return howto;

  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

static void
elfNN_aarch64_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED, arelent *bfd_reloc,
			     Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type;

  r_type = ELFNN_R_TYPE (elf_reloc->r_info);
  bfd_reloc->howto = elfNN_aarch64_howto_from_type (r_type);
}

static reloc_howto_type *
elfNN_aarch64_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  reloc_howto_type *howto = elfNN_aarch64_howto_from_bfd_reloc (code);

  if (howto != NULL)
    return howto;

  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

static reloc_howto_type *
elfNN_aarch64_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 1; i < ARRAY_SIZE (elfNN_aarch64_howto_table) - 1; ++i)
    if (elfNN_aarch64_howto_table[i].name != NULL
	&& strcasecmp (elfNN_aarch64_howto_table[i].name, r_name) == 0)
      return &elfNN_aarch64_howto_table[i];

  return NULL;
}

#define TARGET_LITTLE_SYM               aarch64_elfNN_le_vec
#define TARGET_LITTLE_NAME              "elfNN-littleaarch64"
#define TARGET_BIG_SYM                  aarch64_elfNN_be_vec
#define TARGET_BIG_NAME                 "elfNN-bigaarch64"

/* The linker script knows the section names for placement.
   The entry_names are used to do simple name mangling on the stubs.
   Given a function name, and its type, the stub can be found. The
   name can be changed. The only requirement is the %s be present.  */
#define STUB_ENTRY_NAME   "__%s_veneer"

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER     "/lib/ld.so.1"

#define AARCH64_MAX_FWD_BRANCH_OFFSET \
  (((1 << 25) - 1) << 2)
#define AARCH64_MAX_BWD_BRANCH_OFFSET \
  (-((1 << 25) << 2))

#define AARCH64_MAX_ADRP_IMM ((1 << 20) - 1)
#define AARCH64_MIN_ADRP_IMM (-(1 << 20))

static int
aarch64_valid_for_adrp_p (bfd_vma value, bfd_vma place)
{
  bfd_signed_vma offset = (bfd_signed_vma) (PG (value) - PG (place)) >> 12;
  return offset <= AARCH64_MAX_ADRP_IMM && offset >= AARCH64_MIN_ADRP_IMM;
}

static int
aarch64_valid_branch_p (bfd_vma value, bfd_vma place)
{
  bfd_signed_vma offset = (bfd_signed_vma) (value - place);
  return (offset <= AARCH64_MAX_FWD_BRANCH_OFFSET
	  && offset >= AARCH64_MAX_BWD_BRANCH_OFFSET);
}

static const uint32_t aarch64_adrp_branch_stub [] =
{
  0x90000010,			/*	adrp	ip0, X */
				/*		R_AARCH64_ADR_HI21_PCREL(X) */
  0x91000210,			/*	add	ip0, ip0, :lo12:X */
				/*		R_AARCH64_ADD_ABS_LO12_NC(X) */
  0xd61f0200,			/*	br	ip0 */
};

static const uint32_t aarch64_long_branch_stub[] =
{
#if ARCH_SIZE == 64
  0x58000090,			/*	ldr   ip0, 1f */
#else
  0x18000090,			/*	ldr   wip0, 1f */
#endif
  0x10000011,			/*	adr   ip1, #0 */
  0x8b110210,			/*	add   ip0, ip0, ip1 */
  0xd61f0200,			/*	br	ip0 */
  0x00000000,			/* 1:	.xword or .word
				   R_AARCH64_PRELNN(X) + 12
				 */
  0x00000000,
};

static const uint32_t aarch64_erratum_835769_stub[] =
{
  0x00000000,    /* Placeholder for multiply accumulate.  */
  0x14000000,    /* b <label> */
};

/* Section name for stubs is the associated section name plus this
   string.  */
#define STUB_SUFFIX ".stub"

enum elf_aarch64_stub_type
{
  aarch64_stub_none,
  aarch64_stub_adrp_branch,
  aarch64_stub_long_branch,
  aarch64_stub_erratum_835769_veneer,
};

struct elf_aarch64_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump).  */
  bfd_vma target_value;
  asection *target_section;

  enum elf_aarch64_stub_type stub_type;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf_aarch64_link_hash_entry *h;

  /* Destination symbol type */
  unsigned char st_type;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;

  /* The name for the local symbol at the start of this stub.  The
     stub name in the hash table has to be unique; this does not, so
     it can be friendlier.  */
  char *output_name;

  /* The instruction which caused this stub to be generated (only valid for
     erratum 835769 workaround stubs at present).  */
  uint32_t veneered_insn;
};

/* Used to build a map of a section.  This is required for mixed-endian
   code/data.  */

typedef struct elf_elf_section_map
{
  bfd_vma vma;
  char type;
}
elf_aarch64_section_map;


typedef struct _aarch64_elf_section_data
{
  struct bfd_elf_section_data elf;
  unsigned int mapcount;
  unsigned int mapsize;
  elf_aarch64_section_map *map;
}
_aarch64_elf_section_data;

#define elf_aarch64_section_data(sec) \
  ((_aarch64_elf_section_data *) elf_section_data (sec))

/* A fix-descriptor for erratum 835769.  */
struct aarch64_erratum_835769_fix
{
  bfd *input_bfd;
  asection *section;
  bfd_vma offset;
  uint32_t veneered_insn;
  char *stub_name;
  enum elf_aarch64_stub_type stub_type;
};

/* The size of the thread control block which is defined to be two pointers.  */
#define TCB_SIZE	(ARCH_SIZE/8)*2

struct elf_aarch64_local_symbol
{
  unsigned int got_type;
  bfd_signed_vma got_refcount;
  bfd_vma got_offset;

  /* Offset of the GOTPLT entry reserved for the TLS descriptor. The
     offset is from the end of the jump table and reserved entries
     within the PLTGOT.

     The magic value (bfd_vma) -1 indicates that an offset has not be
     allocated.  */
  bfd_vma tlsdesc_got_jump_table_offset;
};

struct elf_aarch64_obj_tdata
{
  struct elf_obj_tdata root;

  /* local symbol descriptors */
  struct elf_aarch64_local_symbol *locals;

  /* Zero to warn when linking objects with incompatible enum sizes.  */
  int no_enum_size_warning;

  /* Zero to warn when linking objects with incompatible wchar_t sizes.  */
  int no_wchar_size_warning;
};

#define elf_aarch64_tdata(bfd)				\
  ((struct elf_aarch64_obj_tdata *) (bfd)->tdata.any)

#define elf_aarch64_locals(bfd) (elf_aarch64_tdata (bfd)->locals)

#define is_aarch64_elf(bfd)				\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour	\
   && elf_tdata (bfd) != NULL				\
   && elf_object_id (bfd) == AARCH64_ELF_DATA)

static bfd_boolean
elfNN_aarch64_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_aarch64_obj_tdata),
				  AARCH64_ELF_DATA);
}

#define elf_aarch64_hash_entry(ent) \
  ((struct elf_aarch64_link_hash_entry *)(ent))

#define GOT_UNKNOWN    0
#define GOT_NORMAL     1
#define GOT_TLS_GD     2
#define GOT_TLS_IE     4
#define GOT_TLSDESC_GD 8

#define GOT_TLS_GD_ANY_P(type)	((type & GOT_TLS_GD) || (type & GOT_TLSDESC_GD))

/* AArch64 ELF linker hash entry.  */
struct elf_aarch64_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* Track dynamic relocs copied for this symbol.  */
  struct elf_dyn_relocs *dyn_relocs;

  /* Since PLT entries have variable size, we need to record the
     index into .got.plt instead of recomputing it from the PLT
     offset.  */
  bfd_signed_vma plt_got_offset;

  /* Bit mask representing the type of GOT entry(s) if any required by
     this symbol.  */
  unsigned int got_type;

  /* A pointer to the most recently used stub hash entry against this
     symbol.  */
  struct elf_aarch64_stub_hash_entry *stub_cache;

  /* Offset of the GOTPLT entry reserved for the TLS descriptor.  The offset
     is from the end of the jump table and reserved entries within the PLTGOT.

     The magic value (bfd_vma) -1 indicates that an offset has not
     be allocated.  */
  bfd_vma tlsdesc_got_jump_table_offset;
};

static unsigned int
elfNN_aarch64_symbol_got_type (struct elf_link_hash_entry *h,
			       bfd *abfd,
			       unsigned long r_symndx)
{
  if (h)
    return elf_aarch64_hash_entry (h)->got_type;

  if (! elf_aarch64_locals (abfd))
    return GOT_UNKNOWN;

  return elf_aarch64_locals (abfd)[r_symndx].got_type;
}

/* Get the AArch64 elf linker hash table from a link_info structure.  */
#define elf_aarch64_hash_table(info)					\
  ((struct elf_aarch64_link_hash_table *) ((info)->hash))

#define aarch64_stub_hash_lookup(table, string, create, copy)		\
  ((struct elf_aarch64_stub_hash_entry *)				\
   bfd_hash_lookup ((table), (string), (create), (copy)))

/* AArch64 ELF linker hash table.  */
struct elf_aarch64_link_hash_table
{
  /* The main hash table.  */
  struct elf_link_hash_table root;

  /* Nonzero to force PIC branch veneers.  */
  int pic_veneer;

  /* Fix erratum 835769.  */
  int fix_erratum_835769;

  /* A table of fix locations for erratum 835769.  This holds erratum
     fix locations between elfNN_aarch64_size_stubs() and
     elfNN_aarch64_write_section().  */
  struct aarch64_erratum_835769_fix *aarch64_erratum_835769_fixes;
  unsigned int num_aarch64_erratum_835769_fixes;

  /* The number of bytes in the initial entry in the PLT.  */
  bfd_size_type plt_header_size;

  /* The number of bytes in the subsequent PLT etries.  */
  bfd_size_type plt_entry_size;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sdynbss;
  asection *srelbss;

  /* Small local sym cache.  */
  struct sym_cache sym_cache;

  /* For convenience in allocate_dynrelocs.  */
  bfd *obfd;

  /* The amount of space used by the reserved portion of the sgotplt
     section, plus whatever space is used by the jump slots.  */
  bfd_vma sgotplt_jump_table_size;

  /* The stub hash table.  */
  struct bfd_hash_table stub_hash_table;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection *(*add_stub_section) (const char *, asection *);
  void (*layout_sections_again) (void);

  /* Array to keep track of which stub sections have been created, and
     information on stub grouping.  */
  struct map_stub
  {
    /* This is the section to which stubs in the group will be
       attached.  */
    asection *link_sec;
    /* The stub section.  */
    asection *stub_sec;
  } *stub_group;

  /* Assorted information used by elfNN_aarch64_size_stubs.  */
  unsigned int bfd_count;
  int top_index;
  asection **input_list;

  /* The offset into splt of the PLT entry for the TLS descriptor
     resolver.  Special values are 0, if not necessary (or not found
     to be necessary yet), and -1 if needed but not determined
     yet.  */
  bfd_vma tlsdesc_plt;

  /* The GOT offset for the lazy trampoline.  Communicated to the
     loader via DT_TLSDESC_GOT.  The magic value (bfd_vma) -1
     indicates an offset is not allocated.  */
  bfd_vma dt_tlsdesc_got;

  /* Used by local STT_GNU_IFUNC symbols.  */
  htab_t loc_hash_table;
  void * loc_hash_memory;
};

/* Create an entry in an AArch64 ELF linker hash table.  */

static struct bfd_hash_entry *
elfNN_aarch64_link_hash_newfunc (struct bfd_hash_entry *entry,
				 struct bfd_hash_table *table,
				 const char *string)
{
  struct elf_aarch64_link_hash_entry *ret =
    (struct elf_aarch64_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table,
			     sizeof (struct elf_aarch64_link_hash_entry));
  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_aarch64_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    {
      ret->dyn_relocs = NULL;
      ret->got_type = GOT_UNKNOWN;
      ret->plt_got_offset = (bfd_vma) - 1;
      ret->stub_cache = NULL;
      ret->tlsdesc_got_jump_table_offset = (bfd_vma) - 1;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Initialize an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table, const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct
					 elf_aarch64_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_aarch64_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct elf_aarch64_stub_hash_entry *) entry;
      eh->stub_sec = NULL;
      eh->stub_offset = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
      eh->stub_type = aarch64_stub_none;
      eh->h = NULL;
      eh->id_sec = NULL;
    }

  return entry;
}

/* Compute a hash of a local hash entry.  We use elf_link_hash_entry
  for local symbol so that we can handle local STT_GNU_IFUNC symbols
  as global symbol.  We reuse indx and dynstr_index for local symbol
  hash since they aren't used by global symbols in this backend.  */

static hashval_t
elfNN_aarch64_local_htab_hash (const void *ptr)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) ptr;
  return ELF_LOCAL_SYMBOL_HASH (h->indx, h->dynstr_index);
}

/* Compare local hash entries.  */

static int
elfNN_aarch64_local_htab_eq (const void *ptr1, const void *ptr2)
{
  struct elf_link_hash_entry *h1
     = (struct elf_link_hash_entry *) ptr1;
  struct elf_link_hash_entry *h2
    = (struct elf_link_hash_entry *) ptr2;

  return h1->indx == h2->indx && h1->dynstr_index == h2->dynstr_index;
}

/* Find and/or create a hash entry for local symbol.  */

static struct elf_link_hash_entry *
elfNN_aarch64_get_local_sym_hash (struct elf_aarch64_link_hash_table *htab,
				  bfd *abfd, const Elf_Internal_Rela *rel,
				  bfd_boolean create)
{
  struct elf_aarch64_link_hash_entry e, *ret;
  asection *sec = abfd->sections;
  hashval_t h = ELF_LOCAL_SYMBOL_HASH (sec->id,
				       ELFNN_R_SYM (rel->r_info));
  void **slot;

  e.root.indx = sec->id;
  e.root.dynstr_index = ELFNN_R_SYM (rel->r_info);
  slot = htab_find_slot_with_hash (htab->loc_hash_table, &e, h,
				   create ? INSERT : NO_INSERT);

  if (!slot)
    return NULL;

  if (*slot)
    {
      ret = (struct elf_aarch64_link_hash_entry *) *slot;
      return &ret->root;
    }

  ret = (struct elf_aarch64_link_hash_entry *)
	objalloc_alloc ((struct objalloc *) htab->loc_hash_memory,
			sizeof (struct elf_aarch64_link_hash_entry));
  if (ret)
    {
      memset (ret, 0, sizeof (*ret));
      ret->root.indx = sec->id;
      ret->root.dynstr_index = ELFNN_R_SYM (rel->r_info);
      ret->root.dynindx = -1;
      *slot = ret;
    }
  return &ret->root;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
elfNN_aarch64_copy_indirect_symbol (struct bfd_link_info *info,
				    struct elf_link_hash_entry *dir,
				    struct elf_link_hash_entry *ind)
{
  struct elf_aarch64_link_hash_entry *edir, *eind;

  edir = (struct elf_aarch64_link_hash_entry *) dir;
  eind = (struct elf_aarch64_link_hash_entry *) ind;

  if (eind->dyn_relocs != NULL)
    {
      if (edir->dyn_relocs != NULL)
	{
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  /* Add reloc counts against the indirect sym to the direct sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &eind->dyn_relocs; (p = *pp) != NULL;)
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

  if (ind->root.type == bfd_link_hash_indirect)
    {
      /* Copy over PLT info.  */
      if (dir->got.refcount <= 0)
	{
	  edir->got_type = eind->got_type;
	  eind->got_type = GOT_UNKNOWN;
	}
    }

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Destroy an AArch64 elf linker hash table.  */

static void
elfNN_aarch64_link_hash_table_free (bfd *obfd)
{
  struct elf_aarch64_link_hash_table *ret
    = (struct elf_aarch64_link_hash_table *) obfd->link.hash;

  if (ret->loc_hash_table)
    htab_delete (ret->loc_hash_table);
  if (ret->loc_hash_memory)
    objalloc_free ((struct objalloc *) ret->loc_hash_memory);

  bfd_hash_table_free (&ret->stub_hash_table);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create an AArch64 elf linker hash table.  */

static struct bfd_link_hash_table *
elfNN_aarch64_link_hash_table_create (bfd *abfd)
{
  struct elf_aarch64_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct elf_aarch64_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init
      (&ret->root, abfd, elfNN_aarch64_link_hash_newfunc,
       sizeof (struct elf_aarch64_link_hash_entry), AARCH64_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->plt_header_size = PLT_ENTRY_SIZE;
  ret->plt_entry_size = PLT_SMALL_ENTRY_SIZE;
  ret->obfd = abfd;
  ret->dt_tlsdesc_got = (bfd_vma) - 1;

  if (!bfd_hash_table_init (&ret->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct elf_aarch64_stub_hash_entry)))
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }

  ret->loc_hash_table = htab_try_create (1024,
					 elfNN_aarch64_local_htab_hash,
					 elfNN_aarch64_local_htab_eq,
					 NULL);
  ret->loc_hash_memory = objalloc_create ();
  if (!ret->loc_hash_table || !ret->loc_hash_memory)
    {
      elfNN_aarch64_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.root.hash_table_free = elfNN_aarch64_link_hash_table_free;

  return &ret->root.root;
}

static bfd_boolean
aarch64_relocate (unsigned int r_type, bfd *input_bfd, asection *input_section,
		  bfd_vma offset, bfd_vma value)
{
  reloc_howto_type *howto;
  bfd_vma place;

  howto = elfNN_aarch64_howto_from_type (r_type);
  place = (input_section->output_section->vma + input_section->output_offset
	   + offset);

  r_type = elfNN_aarch64_bfd_reloc_from_type (r_type);
  value = _bfd_aarch64_elf_resolve_relocation (r_type, place, value, 0, FALSE);
  return _bfd_aarch64_elf_put_addend (input_bfd,
				      input_section->contents + offset, r_type,
				      howto, value);
}

static enum elf_aarch64_stub_type
aarch64_select_branch_stub (bfd_vma value, bfd_vma place)
{
  if (aarch64_valid_for_adrp_p (value, place))
    return aarch64_stub_adrp_branch;
  return aarch64_stub_long_branch;
}

/* Determine the type of stub needed, if any, for a call.  */

static enum elf_aarch64_stub_type
aarch64_type_of_stub (struct bfd_link_info *info,
		      asection *input_sec,
		      const Elf_Internal_Rela *rel,
		      unsigned char st_type,
		      struct elf_aarch64_link_hash_entry *hash,
		      bfd_vma destination)
{
  bfd_vma location;
  bfd_signed_vma branch_offset;
  unsigned int r_type;
  struct elf_aarch64_link_hash_table *globals;
  enum elf_aarch64_stub_type stub_type = aarch64_stub_none;
  bfd_boolean via_plt_p;

  if (st_type != STT_FUNC)
    return stub_type;

  globals = elf_aarch64_hash_table (info);
  via_plt_p = (globals->root.splt != NULL && hash != NULL
	       && hash->root.plt.offset != (bfd_vma) - 1);

  if (via_plt_p)
    return stub_type;

  /* Determine where the call point is.  */
  location = (input_sec->output_offset
	      + input_sec->output_section->vma + rel->r_offset);

  branch_offset = (bfd_signed_vma) (destination - location);

  r_type = ELFNN_R_TYPE (rel->r_info);

  /* We don't want to redirect any old unconditional jump in this way,
     only one which is being used for a sibcall, where it is
     acceptable for the IP0 and IP1 registers to be clobbered.  */
  if ((r_type == AARCH64_R (CALL26) || r_type == AARCH64_R (JUMP26))
      && (branch_offset > AARCH64_MAX_FWD_BRANCH_OFFSET
	  || branch_offset < AARCH64_MAX_BWD_BRANCH_OFFSET))
    {
      stub_type = aarch64_stub_long_branch;
    }

  return stub_type;
}

/* Build a name for an entry in the stub hash table.  */

static char *
elfNN_aarch64_stub_name (const asection *input_section,
			 const asection *sym_sec,
			 const struct elf_aarch64_link_hash_entry *hash,
			 const Elf_Internal_Rela *rel)
{
  char *stub_name;
  bfd_size_type len;

  if (hash)
    {
      len = 8 + 1 + strlen (hash->root.root.root.string) + 1 + 16 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	snprintf (stub_name, len, "%08x_%s+%" BFD_VMA_FMT "x",
		  (unsigned int) input_section->id,
		  hash->root.root.root.string,
		  rel->r_addend);
    }
  else
    {
      len = 8 + 1 + 8 + 1 + 8 + 1 + 16 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	snprintf (stub_name, len, "%08x_%x:%x+%" BFD_VMA_FMT "x",
		  (unsigned int) input_section->id,
		  (unsigned int) sym_sec->id,
		  (unsigned int) ELFNN_R_SYM (rel->r_info),
		  rel->r_addend);
    }

  return stub_name;
}

/* Look up an entry in the stub hash.  Stub entries are cached because
   creating the stub name takes a bit of time.  */

static struct elf_aarch64_stub_hash_entry *
elfNN_aarch64_get_stub_entry (const asection *input_section,
			      const asection *sym_sec,
			      struct elf_link_hash_entry *hash,
			      const Elf_Internal_Rela *rel,
			      struct elf_aarch64_link_hash_table *htab)
{
  struct elf_aarch64_stub_hash_entry *stub_entry;
  struct elf_aarch64_link_hash_entry *h =
    (struct elf_aarch64_link_hash_entry *) hash;
  const asection *id_sec;

  if ((input_section->flags & SEC_CODE) == 0)
    return NULL;

  /* If this input section is part of a group of sections sharing one
     stub section, then use the id of the first section in the group.
     Stub names need to include a section id, as there may well be
     more than one stub used to reach say, printf, and we need to
     distinguish between them.  */
  id_sec = htab->stub_group[input_section->id].link_sec;

  if (h != NULL && h->stub_cache != NULL
      && h->stub_cache->h == h && h->stub_cache->id_sec == id_sec)
    {
      stub_entry = h->stub_cache;
    }
  else
    {
      char *stub_name;

      stub_name = elfNN_aarch64_stub_name (id_sec, sym_sec, h, rel);
      if (stub_name == NULL)
	return NULL;

      stub_entry = aarch64_stub_hash_lookup (&htab->stub_hash_table,
					     stub_name, FALSE, FALSE);
      if (h != NULL)
	h->stub_cache = stub_entry;

      free (stub_name);
    }

  return stub_entry;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf_aarch64_stub_hash_entry *
elfNN_aarch64_add_stub (const char *stub_name,
			asection *section,
			struct elf_aarch64_link_hash_table *htab)
{
  asection *link_sec;
  asection *stub_sec;
  struct elf_aarch64_stub_hash_entry *stub_entry;

  link_sec = htab->stub_group[section->id].link_sec;
  stub_sec = htab->stub_group[section->id].stub_sec;
  if (stub_sec == NULL)
    {
      stub_sec = htab->stub_group[link_sec->id].stub_sec;
      if (stub_sec == NULL)
	{
	  size_t namelen;
	  bfd_size_type len;
	  char *s_name;

	  namelen = strlen (link_sec->name);
	  len = namelen + sizeof (STUB_SUFFIX);
	  s_name = bfd_alloc (htab->stub_bfd, len);
	  if (s_name == NULL)
	    return NULL;

	  memcpy (s_name, link_sec->name, namelen);
	  memcpy (s_name + namelen, STUB_SUFFIX, sizeof (STUB_SUFFIX));
	  stub_sec = (*htab->add_stub_section) (s_name, link_sec);
	  if (stub_sec == NULL)
	    return NULL;
	  htab->stub_group[link_sec->id].stub_sec = stub_sec;
	}
      htab->stub_group[section->id].stub_sec = stub_sec;
    }

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = aarch64_stub_hash_lookup (&htab->stub_hash_table, stub_name,
					 TRUE, FALSE);
  if (stub_entry == NULL)
    {
      (*_bfd_error_handler) (_("%s: cannot create stub entry %s"),
			     section->owner, stub_name);
      return NULL;
    }

  stub_entry->stub_sec = stub_sec;
  stub_entry->stub_offset = 0;
  stub_entry->id_sec = link_sec;

  return stub_entry;
}

static bfd_boolean
aarch64_build_one_stub (struct bfd_hash_entry *gen_entry,
			void *in_arg ATTRIBUTE_UNUSED)
{
  struct elf_aarch64_stub_hash_entry *stub_entry;
  asection *stub_sec;
  bfd *stub_bfd;
  bfd_byte *loc;
  bfd_vma sym_value;
  bfd_vma veneered_insn_loc;
  bfd_vma veneer_entry_loc;
  bfd_signed_vma branch_offset = 0;
  unsigned int template_size;
  const uint32_t *template;
  unsigned int i;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf_aarch64_stub_hash_entry *) gen_entry;

  stub_sec = stub_entry->stub_sec;

  /* Make a note of the offset within the stubs for this entry.  */
  stub_entry->stub_offset = stub_sec->size;
  loc = stub_sec->contents + stub_entry->stub_offset;

  stub_bfd = stub_sec->owner;

  /* This is the address of the stub destination.  */
  sym_value = (stub_entry->target_value
	       + stub_entry->target_section->output_offset
	       + stub_entry->target_section->output_section->vma);

  if (stub_entry->stub_type == aarch64_stub_long_branch)
    {
      bfd_vma place = (stub_entry->stub_offset + stub_sec->output_section->vma
		       + stub_sec->output_offset);

      /* See if we can relax the stub.  */
      if (aarch64_valid_for_adrp_p (sym_value, place))
	stub_entry->stub_type = aarch64_select_branch_stub (sym_value, place);
    }

  switch (stub_entry->stub_type)
    {
    case aarch64_stub_adrp_branch:
      template = aarch64_adrp_branch_stub;
      template_size = sizeof (aarch64_adrp_branch_stub);
      break;
    case aarch64_stub_long_branch:
      template = aarch64_long_branch_stub;
      template_size = sizeof (aarch64_long_branch_stub);
      break;
    case aarch64_stub_erratum_835769_veneer:
      template = aarch64_erratum_835769_stub;
      template_size = sizeof (aarch64_erratum_835769_stub);
      break;
    default:
      BFD_FAIL ();
      return FALSE;
    }

  for (i = 0; i < (template_size / sizeof template[0]); i++)
    {
      bfd_putl32 (template[i], loc);
      loc += 4;
    }

  template_size = (template_size + 7) & ~7;
  stub_sec->size += template_size;

  switch (stub_entry->stub_type)
    {
    case aarch64_stub_adrp_branch:
      if (aarch64_relocate (AARCH64_R (ADR_PREL_PG_HI21), stub_bfd, stub_sec,
			    stub_entry->stub_offset, sym_value))
	/* The stub would not have been relaxed if the offset was out
	   of range.  */
	BFD_FAIL ();

      if (aarch64_relocate (AARCH64_R (ADD_ABS_LO12_NC), stub_bfd, stub_sec,
			    stub_entry->stub_offset + 4, sym_value))
	BFD_FAIL ();
      break;

    case aarch64_stub_long_branch:
      /* We want the value relative to the address 12 bytes back from the
         value itself.  */
      if (aarch64_relocate (AARCH64_R (PRELNN), stub_bfd, stub_sec,
			    stub_entry->stub_offset + 16, sym_value + 12))
	BFD_FAIL ();
      break;

    case aarch64_stub_erratum_835769_veneer:
      veneered_insn_loc = stub_entry->target_section->output_section->vma
			  + stub_entry->target_section->output_offset
			  + stub_entry->target_value;
      veneer_entry_loc = stub_entry->stub_sec->output_section->vma
			  + stub_entry->stub_sec->output_offset
			  + stub_entry->stub_offset;
      branch_offset = veneered_insn_loc - veneer_entry_loc;
      branch_offset >>= 2;
      branch_offset &= 0x3ffffff;
      bfd_putl32 (stub_entry->veneered_insn,
		  stub_sec->contents + stub_entry->stub_offset);
      bfd_putl32 (template[1] | branch_offset,
		  stub_sec->contents + stub_entry->stub_offset + 4);
      break;

    default:
      break;
    }

  return TRUE;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bfd_boolean
aarch64_size_one_stub (struct bfd_hash_entry *gen_entry,
		       void *in_arg ATTRIBUTE_UNUSED)
{
  struct elf_aarch64_stub_hash_entry *stub_entry;
  int size;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf_aarch64_stub_hash_entry *) gen_entry;

  switch (stub_entry->stub_type)
    {
    case aarch64_stub_adrp_branch:
      size = sizeof (aarch64_adrp_branch_stub);
      break;
    case aarch64_stub_long_branch:
      size = sizeof (aarch64_long_branch_stub);
      break;
    case aarch64_stub_erratum_835769_veneer:
      size = sizeof (aarch64_erratum_835769_stub);
      break;
    default:
      BFD_FAIL ();
      return FALSE;
      break;
    }

  size = (size + 7) & ~7;
  stub_entry->stub_sec->size += size;
  return TRUE;
}

/* External entry points for sizing and building linker stubs.  */

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elfNN_aarch64_setup_section_lists (bfd *output_bfd,
				   struct bfd_link_info *info)
{
  bfd *input_bfd;
  unsigned int bfd_count;
  int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  bfd_size_type amt;
  struct elf_aarch64_link_hash_table *htab =
    elf_aarch64_hash_table (info);

  if (!is_elf_hash_table (htab))
    return 0;

  /* Count the number of input BFDs and find the top input section id.  */
  for (input_bfd = info->input_bfds, bfd_count = 0, top_id = 0;
       input_bfd != NULL; input_bfd = input_bfd->link.next)
    {
      bfd_count += 1;
      for (section = input_bfd->sections;
	   section != NULL; section = section->next)
	{
	  if (top_id < section->id)
	    top_id = section->id;
	}
    }
  htab->bfd_count = bfd_count;

  amt = sizeof (struct map_stub) * (top_id + 1);
  htab->stub_group = bfd_zmalloc (amt);
  if (htab->stub_group == NULL)
    return -1;

  /* We can't use output_bfd->section_count here to find the top output
     section index as some sections may have been removed, and
     _bfd_strip_section_from_output doesn't renumber the indices.  */
  for (section = output_bfd->sections, top_index = 0;
       section != NULL; section = section->next)
    {
      if (top_index < section->index)
	top_index = section->index;
    }

  htab->top_index = top_index;
  amt = sizeof (asection *) * (top_index + 1);
  input_list = bfd_malloc (amt);
  htab->input_list = input_list;
  if (input_list == NULL)
    return -1;

  /* For sections we aren't interested in, mark their entries with a
     value we can check later.  */
  list = input_list + top_index;
  do
    *list = bfd_abs_section_ptr;
  while (list-- != input_list);

  for (section = output_bfd->sections;
       section != NULL; section = section->next)
    {
      if ((section->flags & SEC_CODE) != 0)
	input_list[section->index] = NULL;
    }

  return 1;
}

/* Used by elfNN_aarch64_next_input_section and group_sections.  */
#define PREV_SEC(sec) (htab->stub_group[(sec)->id].link_sec)

/* The linker repeatedly calls this function for each input section,
   in the order that input sections are linked into output sections.
   Build lists of input sections to determine groupings between which
   we may insert linker stubs.  */

void
elfNN_aarch64_next_input_section (struct bfd_link_info *info, asection *isec)
{
  struct elf_aarch64_link_hash_table *htab =
    elf_aarch64_hash_table (info);

  if (isec->output_section->index <= htab->top_index)
    {
      asection **list = htab->input_list + isec->output_section->index;

      if (*list != bfd_abs_section_ptr)
	{
	  /* Steal the link_sec pointer for our list.  */
	  /* This happens to make the list in reverse order,
	     which is what we want.  */
	  PREV_SEC (isec) = *list;
	  *list = isec;
	}
    }
}

/* See whether we can group stub sections together.  Grouping stub
   sections may result in fewer stubs.  More importantly, we need to
   put all .init* and .fini* stubs at the beginning of the .init or
   .fini output sections respectively, because glibc splits the
   _init and _fini functions into multiple parts.  Putting a stub in
   the middle of a function is not a good idea.  */

static void
group_sections (struct elf_aarch64_link_hash_table *htab,
		bfd_size_type stub_group_size,
		bfd_boolean stubs_always_before_branch)
{
  asection **list = htab->input_list + htab->top_index;

  do
    {
      asection *tail = *list;

      if (tail == bfd_abs_section_ptr)
	continue;

      while (tail != NULL)
	{
	  asection *curr;
	  asection *prev;
	  bfd_size_type total;

	  curr = tail;
	  total = tail->size;
	  while ((prev = PREV_SEC (curr)) != NULL
		 && ((total += curr->output_offset - prev->output_offset)
		     < stub_group_size))
	    curr = prev;

	  /* OK, the size from the start of CURR to the end is less
	     than stub_group_size and thus can be handled by one stub
	     section.  (Or the tail section is itself larger than
	     stub_group_size, in which case we may be toast.)
	     We should really be keeping track of the total size of
	     stubs added here, as stubs contribute to the final output
	     section size.  */
	  do
	    {
	      prev = PREV_SEC (tail);
	      /* Set up this stub group.  */
	      htab->stub_group[tail->id].link_sec = curr;
	    }
	  while (tail != curr && (tail = prev) != NULL);

	  /* But wait, there's more!  Input sections up to stub_group_size
	     bytes before the stub section can be handled by it too.  */
	  if (!stubs_always_before_branch)
	    {
	      total = 0;
	      while (prev != NULL
		     && ((total += tail->output_offset - prev->output_offset)
			 < stub_group_size))
		{
		  tail = prev;
		  prev = PREV_SEC (tail);
		  htab->stub_group[tail->id].link_sec = curr;
		}
	    }
	  tail = prev;
	}
    }
  while (list-- != htab->input_list);

  free (htab->input_list);
}

#undef PREV_SEC

#define AARCH64_BITS(x, pos, n) (((x) >> (pos)) & ((1 << (n)) - 1))

#define AARCH64_RT(insn) AARCH64_BITS (insn, 0, 5)
#define AARCH64_RT2(insn) AARCH64_BITS (insn, 10, 5)
#define AARCH64_RA(insn) AARCH64_BITS (insn, 10, 5)
#define AARCH64_RD(insn) AARCH64_BITS (insn, 0, 5)
#define AARCH64_RN(insn) AARCH64_BITS (insn, 5, 5)
#define AARCH64_RM(insn) AARCH64_BITS (insn, 16, 5)

#define AARCH64_MAC(insn) (((insn) & 0xff000000) == 0x9b000000)
#define AARCH64_BIT(insn, n) AARCH64_BITS (insn, n, 1)
#define AARCH64_OP31(insn) AARCH64_BITS (insn, 21, 3)
#define AARCH64_ZR 0x1f

/* All ld/st ops.  See C4-182 of the ARM ARM.  The encoding space for
   LD_PCREL, LDST_RO, LDST_UI and LDST_UIMM cover prefetch ops.  */

#define AARCH64_LD(insn) (AARCH64_BIT (insn, 22) == 1)
#define AARCH64_LDST(insn) (((insn) & 0x0a000000) == 0x08000000)
#define AARCH64_LDST_EX(insn) (((insn) & 0x3f000000) == 0x08000000)
#define AARCH64_LDST_PCREL(insn) (((insn) & 0x3b000000) == 0x18000000)
#define AARCH64_LDST_NAP(insn) (((insn) & 0x3b800000) == 0x28000000)
#define AARCH64_LDSTP_PI(insn) (((insn) & 0x3b800000) == 0x28800000)
#define AARCH64_LDSTP_O(insn) (((insn) & 0x3b800000) == 0x29000000)
#define AARCH64_LDSTP_PRE(insn) (((insn) & 0x3b800000) == 0x29800000)
#define AARCH64_LDST_UI(insn) (((insn) & 0x3b200c00) == 0x38000000)
#define AARCH64_LDST_PIIMM(insn) (((insn) & 0x3b200c00) == 0x38000400)
#define AARCH64_LDST_U(insn) (((insn) & 0x3b200c00) == 0x38000800)
#define AARCH64_LDST_PREIMM(insn) (((insn) & 0x3b200c00) == 0x38000c00)
#define AARCH64_LDST_RO(insn) (((insn) & 0x3b200c00) == 0x38200800)
#define AARCH64_LDST_UIMM(insn) (((insn) & 0x3b000000) == 0x39000000)
#define AARCH64_LDST_SIMD_M(insn) (((insn) & 0xbfbf0000) == 0x0c000000)
#define AARCH64_LDST_SIMD_M_PI(insn) (((insn) & 0xbfa00000) == 0x0c800000)
#define AARCH64_LDST_SIMD_S(insn) (((insn) & 0xbf9f0000) == 0x0d000000)
#define AARCH64_LDST_SIMD_S_PI(insn) (((insn) & 0xbf800000) == 0x0d800000)

/* Classify an INSN if it is indeed a load/store.  Return TRUE if INSN
   is a load/store along with the Rt and Rtn.  Return FALSE if not a
   load/store.  */

static bfd_boolean
aarch64_mem_op_p (uint32_t insn, unsigned int *rt, unsigned int *rtn,
		  bfd_boolean *pair, bfd_boolean *load)
{
  uint32_t opcode;
  unsigned int r;
  uint32_t opc = 0;
  uint32_t v = 0;
  uint32_t opc_v = 0;

  /* Bail out quickly if INSN doesn't fall into the the load-store
     encoding space.  */
  if (!AARCH64_LDST (insn))
    return FALSE;

  *pair = FALSE;
  *load = FALSE;
  if (AARCH64_LDST_EX (insn))
    {
      *rt = AARCH64_RT (insn);
      *rtn = *rt;
      if (AARCH64_BIT (insn, 21) == 1)
        {
	  *pair = TRUE;
	  *rtn = AARCH64_RT2 (insn);
	}
      *load = AARCH64_LD (insn);
      return TRUE;
    }
  else if (AARCH64_LDST_NAP (insn)
	   || AARCH64_LDSTP_PI (insn)
	   || AARCH64_LDSTP_O (insn)
	   || AARCH64_LDSTP_PRE (insn))
    {
      *pair = TRUE;
      *rt = AARCH64_RT (insn);
      *rtn = AARCH64_RT2 (insn);
      *load = AARCH64_LD (insn);
      return TRUE;
    }
  else if (AARCH64_LDST_PCREL (insn)
	   || AARCH64_LDST_UI (insn)
	   || AARCH64_LDST_PIIMM (insn)
	   || AARCH64_LDST_U (insn)
	   || AARCH64_LDST_PREIMM (insn)
	   || AARCH64_LDST_RO (insn)
	   || AARCH64_LDST_UIMM (insn))
   {
      *rt = AARCH64_RT (insn);
      *rtn = *rt;
      if (AARCH64_LDST_PCREL (insn))
	*load = TRUE;
      opc = AARCH64_BITS (insn, 22, 2);
      v = AARCH64_BIT (insn, 26);
      opc_v = opc | (v << 2);
      *load =  (opc_v == 1 || opc_v == 2 || opc_v == 3
		|| opc_v == 5 || opc_v == 7);
      return TRUE;
   }
  else if (AARCH64_LDST_SIMD_M (insn)
	   || AARCH64_LDST_SIMD_M_PI (insn))
    {
      *rt = AARCH64_RT (insn);
      *load = AARCH64_BIT (insn, 22);
      opcode = (insn >> 12) & 0xf;
      switch (opcode)
	{
	case 0:
	case 2:
	  *rtn = *rt + 3;
	  break;

	case 4:
	case 6:
	  *rtn = *rt + 2;
	  break;

	case 7:
	  *rtn = *rt;
	  break;

	case 8:
	case 10:
	  *rtn = *rt + 1;
	  break;

	default:
	  return FALSE;
	}
      return TRUE;
    }
  else if (AARCH64_LDST_SIMD_S (insn)
	   || AARCH64_LDST_SIMD_S_PI (insn))
    {
      *rt = AARCH64_RT (insn);
      r = (insn >> 21) & 1;
      *load = AARCH64_BIT (insn, 22);
      opcode = (insn >> 13) & 0x7;
      switch (opcode)
	{
	case 0:
	case 2:
	case 4:
	  *rtn = *rt + r;
	  break;

	case 1:
	case 3:
	case 5:
	  *rtn = *rt + (r == 0 ? 2 : 3);
	  break;

	case 6:
	  *rtn = *rt + r;
	  break;

	case 7:
	  *rtn = *rt + (r == 0 ? 2 : 3);
	  break;

	default:
	  return FALSE;
	}
      return TRUE;
    }

  return FALSE;
}

/* Return TRUE if INSN is multiply-accumulate.  */

static bfd_boolean
aarch64_mlxl_p (uint32_t insn)
{
  uint32_t op31 = AARCH64_OP31 (insn);

  if (AARCH64_MAC (insn)
      && (op31 == 0 || op31 == 1 || op31 == 5)
      /* Exclude MUL instructions which are encoded as a multiple accumulate
	 with RA = XZR.  */
      && AARCH64_RA (insn) != AARCH64_ZR)
    return TRUE;

  return FALSE;
}

/* Some early revisions of the Cortex-A53 have an erratum (835769) whereby
   it is possible for a 64-bit multiply-accumulate instruction to generate an
   incorrect result.  The details are quite complex and hard to
   determine statically, since branches in the code may exist in some
   circumstances, but all cases end with a memory (load, store, or
   prefetch) instruction followed immediately by the multiply-accumulate
   operation.  We employ a linker patching technique, by moving the potentially
   affected multiply-accumulate instruction into a patch region and replacing
   the original instruction with a branch to the patch.  This function checks
   if INSN_1 is the memory operation followed by a multiply-accumulate
   operation (INSN_2).  Return TRUE if an erratum sequence is found, FALSE
   if INSN_1 and INSN_2 are safe.  */

static bfd_boolean
aarch64_erratum_sequence (uint32_t insn_1, uint32_t insn_2)
{
  uint32_t rt;
  uint32_t rtn;
  uint32_t rn;
  uint32_t rm;
  uint32_t ra;
  bfd_boolean pair;
  bfd_boolean load;

  if (aarch64_mlxl_p (insn_2)
      && aarch64_mem_op_p (insn_1, &rt, &rtn, &pair, &load))
    {
      /* Any SIMD memory op is independent of the subsequent MLA
	 by definition of the erratum.  */
      if (AARCH64_BIT (insn_1, 26))
	return TRUE;

      /* If not SIMD, check for integer memory ops and MLA relationship.  */
      rn = AARCH64_RN (insn_2);
      ra = AARCH64_RA (insn_2);
      rm = AARCH64_RM (insn_2);

      /* If this is a load and there's a true(RAW) dependency, we are safe
	 and this is not an erratum sequence.  */
      if (load &&
	  (rt == rn || rt == rm || rt == ra
	   || (pair && (rtn == rn || rtn == rm || rtn == ra))))
	return FALSE;

      /* We conservatively put out stubs for all other cases (including
	 writebacks).  */
      return TRUE;
    }

  return FALSE;
}

/* Used to order a list of mapping symbols by address.  */

static int
elf_aarch64_compare_mapping (const void *a, const void *b)
{
  const elf_aarch64_section_map *amap = (const elf_aarch64_section_map *) a;
  const elf_aarch64_section_map *bmap = (const elf_aarch64_section_map *) b;

  if (amap->vma > bmap->vma)
    return 1;
  else if (amap->vma < bmap->vma)
    return -1;
  else if (amap->type > bmap->type)
    /* Ensure results do not depend on the host qsort for objects with
       multiple mapping symbols at the same address by sorting on type
       after vma.  */
    return 1;
  else if (amap->type < bmap->type)
    return -1;
  else
    return 0;
}

static bfd_boolean
erratum_835769_scan (bfd *input_bfd,
		     struct bfd_link_info *info,
		     struct aarch64_erratum_835769_fix **fixes_p,
		     unsigned int *num_fixes_p,
		     unsigned int *fix_table_size_p)
{
  asection *section;
  struct elf_aarch64_link_hash_table *htab = elf_aarch64_hash_table (info);
  struct aarch64_erratum_835769_fix *fixes = *fixes_p;
  unsigned int num_fixes = *num_fixes_p;
  unsigned int fix_table_size = *fix_table_size_p;

  if (htab == NULL)
    return FALSE;

  for (section = input_bfd->sections;
       section != NULL;
       section = section->next)
    {
      bfd_byte *contents = NULL;
      struct _aarch64_elf_section_data *sec_data;
      unsigned int span;

      if (elf_section_type (section) != SHT_PROGBITS
	  || (elf_section_flags (section) & SHF_EXECINSTR) == 0
	  || (section->flags & SEC_EXCLUDE) != 0
	  || (section->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	  || (section->output_section == bfd_abs_section_ptr))
	continue;

      if (elf_section_data (section)->this_hdr.contents != NULL)
	contents = elf_section_data (section)->this_hdr.contents;
      else if (! bfd_malloc_and_get_section (input_bfd, section, &contents))
	return TRUE;

      sec_data = elf_aarch64_section_data (section);

      qsort (sec_data->map, sec_data->mapcount,
	     sizeof (elf_aarch64_section_map), elf_aarch64_compare_mapping);

      for (span = 0; span < sec_data->mapcount; span++)
	{
	  unsigned int span_start = sec_data->map[span].vma;
	  unsigned int span_end = ((span == sec_data->mapcount - 1)
				   ? sec_data->map[0].vma + section->size
				   : sec_data->map[span + 1].vma);
	  unsigned int i;
	  char span_type = sec_data->map[span].type;

	  if (span_type == 'd')
	    continue;

	  for (i = span_start; i + 4 < span_end; i += 4)
	    {
	      uint32_t insn_1 = bfd_getl32 (contents + i);
	      uint32_t insn_2 = bfd_getl32 (contents + i + 4);

	      if (aarch64_erratum_sequence (insn_1, insn_2))
		{
		  char *stub_name = NULL;
		  stub_name = (char *) bfd_malloc
				(strlen ("__erratum_835769_veneer_") + 16);
		  if (stub_name != NULL)
		    sprintf
		      (stub_name,"__erratum_835769_veneer_%d", num_fixes);
		  else
		    return TRUE;

		  if (num_fixes == fix_table_size)
		    {
		      fix_table_size *= 2;
		      fixes =
			(struct aarch64_erratum_835769_fix *)
			  bfd_realloc (fixes,
				       sizeof (struct aarch64_erratum_835769_fix)
					 * fix_table_size);
		      if (fixes == NULL)
			return TRUE;
		    }

		  fixes[num_fixes].input_bfd = input_bfd;
		  fixes[num_fixes].section = section;
		  fixes[num_fixes].offset = i + 4;
		  fixes[num_fixes].veneered_insn = insn_2;
		  fixes[num_fixes].stub_name = stub_name;
		  fixes[num_fixes].stub_type = aarch64_stub_erratum_835769_veneer;
		  num_fixes++;
		}
	    }
	}
      if (elf_section_data (section)->this_hdr.contents == NULL)
	free (contents);
    }

  *fixes_p = fixes;
  *num_fixes_p = num_fixes;
  *fix_table_size_p = fix_table_size;
  return FALSE;
}

/* Find or create a stub section.  Returns a pointer to the stub section, and
   the section to which the stub section will be attached (in *LINK_SEC_P).
   LINK_SEC_P may be NULL.  */

static asection *
elf_aarch64_create_or_find_stub_sec (asection **link_sec_p, asection *section,
				   struct elf_aarch64_link_hash_table *htab)
{
  asection *link_sec;
  asection *stub_sec;

  link_sec = htab->stub_group[section->id].link_sec;
  BFD_ASSERT (link_sec != NULL);
  stub_sec = htab->stub_group[section->id].stub_sec;

  if (stub_sec == NULL)
    {
      stub_sec = htab->stub_group[link_sec->id].stub_sec;
      if (stub_sec == NULL)
	{
	  size_t namelen;
	  bfd_size_type len;
	  char *s_name;

	  namelen = strlen (link_sec->name);
	  len = namelen + sizeof (STUB_SUFFIX);
	  s_name = (char *) bfd_alloc (htab->stub_bfd, len);
	  if (s_name == NULL)
	    return NULL;

	  memcpy (s_name, link_sec->name, namelen);
	  memcpy (s_name + namelen, STUB_SUFFIX, sizeof (STUB_SUFFIX));
	  stub_sec = (*htab->add_stub_section) (s_name, link_sec);

	  if (stub_sec == NULL)
	    return NULL;
	  htab->stub_group[link_sec->id].stub_sec = stub_sec;
	}
      htab->stub_group[section->id].stub_sec = stub_sec;
    }

  if (link_sec_p)
    *link_sec_p = link_sec;

  return stub_sec;
}

/* Determine and set the size of the stub section for a final link.

   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bl"
   instruction.  */

bfd_boolean
elfNN_aarch64_size_stubs (bfd *output_bfd,
			  bfd *stub_bfd,
			  struct bfd_link_info *info,
			  bfd_signed_vma group_size,
			  asection * (*add_stub_section) (const char *,
							  asection *),
			  void (*layout_sections_again) (void))
{
  bfd_size_type stub_group_size;
  bfd_boolean stubs_always_before_branch;
  bfd_boolean stub_changed = 0;
  struct elf_aarch64_link_hash_table *htab = elf_aarch64_hash_table (info);
  struct aarch64_erratum_835769_fix *erratum_835769_fixes = NULL;
  unsigned int num_erratum_835769_fixes = 0;
  unsigned int erratum_835769_fix_table_size = 10;
  unsigned int i;

  if (htab->fix_erratum_835769)
    {
      erratum_835769_fixes
	= (struct aarch64_erratum_835769_fix *)
	    bfd_zmalloc
	      (sizeof (struct aarch64_erratum_835769_fix) *
					erratum_835769_fix_table_size);
      if (erratum_835769_fixes == NULL)
	goto error_ret_free_local;
    }

  /* Propagate mach to stub bfd, because it may not have been
     finalized when we created stub_bfd.  */
  bfd_set_arch_mach (stub_bfd, bfd_get_arch (output_bfd),
		     bfd_get_mach (output_bfd));

  /* Stash our params away.  */
  htab->stub_bfd = stub_bfd;
  htab->add_stub_section = add_stub_section;
  htab->layout_sections_again = layout_sections_again;
  stubs_always_before_branch = group_size < 0;
  if (group_size < 0)
    stub_group_size = -group_size;
  else
    stub_group_size = group_size;

  if (stub_group_size == 1)
    {
      /* Default values.  */
      /* AArch64 branch range is +-128MB. The value used is 1MB less.  */
      stub_group_size = 127 * 1024 * 1024;
    }

  group_sections (htab, stub_group_size, stubs_always_before_branch);

  while (1)
    {
      bfd *input_bfd;
      unsigned int bfd_indx;
      asection *stub_sec;
      unsigned prev_num_erratum_835769_fixes = num_erratum_835769_fixes;

      num_erratum_835769_fixes = 0;
      for (input_bfd = info->input_bfds, bfd_indx = 0;
	   input_bfd != NULL; input_bfd = input_bfd->link.next, bfd_indx++)
	{
	  Elf_Internal_Shdr *symtab_hdr;
	  asection *section;
	  Elf_Internal_Sym *local_syms = NULL;

	  /* We'll need the symbol table in a second.  */
	  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
	  if (symtab_hdr->sh_info == 0)
	    continue;

	  /* Walk over each section attached to the input bfd.  */
	  for (section = input_bfd->sections;
	       section != NULL; section = section->next)
	    {
	      Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

	      /* If there aren't any relocs, then there's nothing more
		 to do.  */
	      if ((section->flags & SEC_RELOC) == 0
		  || section->reloc_count == 0
		  || (section->flags & SEC_CODE) == 0)
		continue;

	      /* If this section is a link-once section that will be
		 discarded, then don't create any stubs.  */
	      if (section->output_section == NULL
		  || section->output_section->owner != output_bfd)
		continue;

	      /* Get the relocs.  */
	      internal_relocs
		= _bfd_elf_link_read_relocs (input_bfd, section, NULL,
					     NULL, info->keep_memory);
	      if (internal_relocs == NULL)
		goto error_ret_free_local;

	      /* Now examine each relocation.  */
	      irela = internal_relocs;
	      irelaend = irela + section->reloc_count;
	      for (; irela < irelaend; irela++)
		{
		  unsigned int r_type, r_indx;
		  enum elf_aarch64_stub_type stub_type;
		  struct elf_aarch64_stub_hash_entry *stub_entry;
		  asection *sym_sec;
		  bfd_vma sym_value;
		  bfd_vma destination;
		  struct elf_aarch64_link_hash_entry *hash;
		  const char *sym_name;
		  char *stub_name;
		  const asection *id_sec;
		  unsigned char st_type;
		  bfd_size_type len;

		  r_type = ELFNN_R_TYPE (irela->r_info);
		  r_indx = ELFNN_R_SYM (irela->r_info);

		  if (r_type >= (unsigned int) R_AARCH64_end)
		    {
		      bfd_set_error (bfd_error_bad_value);
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		      goto error_ret_free_local;
		    }

		  /* Only look for stubs on unconditional branch and
		     branch and link instructions.  */
		  if (r_type != (unsigned int) AARCH64_R (CALL26)
		      && r_type != (unsigned int) AARCH64_R (JUMP26))
		    continue;

		  /* Now determine the call target, its name, value,
		     section.  */
		  sym_sec = NULL;
		  sym_value = 0;
		  destination = 0;
		  hash = NULL;
		  sym_name = NULL;
		  if (r_indx < symtab_hdr->sh_info)
		    {
		      /* It's a local symbol.  */
		      Elf_Internal_Sym *sym;
		      Elf_Internal_Shdr *hdr;

		      if (local_syms == NULL)
			{
			  local_syms
			    = (Elf_Internal_Sym *) symtab_hdr->contents;
			  if (local_syms == NULL)
			    local_syms
			      = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
						      symtab_hdr->sh_info, 0,
						      NULL, NULL, NULL);
			  if (local_syms == NULL)
			    goto error_ret_free_internal;
			}

		      sym = local_syms + r_indx;
		      hdr = elf_elfsections (input_bfd)[sym->st_shndx];
		      sym_sec = hdr->bfd_section;
		      if (!sym_sec)
			/* This is an undefined symbol.  It can never
			   be resolved.  */
			continue;

		      if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
			sym_value = sym->st_value;
		      destination = (sym_value + irela->r_addend
				     + sym_sec->output_offset
				     + sym_sec->output_section->vma);
		      st_type = ELF_ST_TYPE (sym->st_info);
		      sym_name
			= bfd_elf_string_from_elf_section (input_bfd,
							   symtab_hdr->sh_link,
							   sym->st_name);
		    }
		  else
		    {
		      int e_indx;

		      e_indx = r_indx - symtab_hdr->sh_info;
		      hash = ((struct elf_aarch64_link_hash_entry *)
			      elf_sym_hashes (input_bfd)[e_indx]);

		      while (hash->root.root.type == bfd_link_hash_indirect
			     || hash->root.root.type == bfd_link_hash_warning)
			hash = ((struct elf_aarch64_link_hash_entry *)
				hash->root.root.u.i.link);

		      if (hash->root.root.type == bfd_link_hash_defined
			  || hash->root.root.type == bfd_link_hash_defweak)
			{
			  struct elf_aarch64_link_hash_table *globals =
			    elf_aarch64_hash_table (info);
			  sym_sec = hash->root.root.u.def.section;
			  sym_value = hash->root.root.u.def.value;
			  /* For a destination in a shared library,
			     use the PLT stub as target address to
			     decide whether a branch stub is
			     needed.  */
			  if (globals->root.splt != NULL && hash != NULL
			      && hash->root.plt.offset != (bfd_vma) - 1)
			    {
			      sym_sec = globals->root.splt;
			      sym_value = hash->root.plt.offset;
			      if (sym_sec->output_section != NULL)
				destination = (sym_value
					       + sym_sec->output_offset
					       +
					       sym_sec->output_section->vma);
			    }
			  else if (sym_sec->output_section != NULL)
			    destination = (sym_value + irela->r_addend
					   + sym_sec->output_offset
					   + sym_sec->output_section->vma);
			}
		      else if (hash->root.root.type == bfd_link_hash_undefined
			       || (hash->root.root.type
				   == bfd_link_hash_undefweak))
			{
			  /* For a shared library, use the PLT stub as
			     target address to decide whether a long
			     branch stub is needed.
			     For absolute code, they cannot be handled.  */
			  struct elf_aarch64_link_hash_table *globals =
			    elf_aarch64_hash_table (info);

			  if (globals->root.splt != NULL && hash != NULL
			      && hash->root.plt.offset != (bfd_vma) - 1)
			    {
			      sym_sec = globals->root.splt;
			      sym_value = hash->root.plt.offset;
			      if (sym_sec->output_section != NULL)
				destination = (sym_value
					       + sym_sec->output_offset
					       +
					       sym_sec->output_section->vma);
			    }
			  else
			    continue;
			}
		      else
			{
			  bfd_set_error (bfd_error_bad_value);
			  goto error_ret_free_internal;
			}
		      st_type = ELF_ST_TYPE (hash->root.type);
		      sym_name = hash->root.root.root.string;
		    }

		  /* Determine what (if any) linker stub is needed.  */
		  stub_type = aarch64_type_of_stub
		    (info, section, irela, st_type, hash, destination);
		  if (stub_type == aarch64_stub_none)
		    continue;

		  /* Support for grouping stub sections.  */
		  id_sec = htab->stub_group[section->id].link_sec;

		  /* Get the name of this stub.  */
		  stub_name = elfNN_aarch64_stub_name (id_sec, sym_sec, hash,
						       irela);
		  if (!stub_name)
		    goto error_ret_free_internal;

		  stub_entry =
		    aarch64_stub_hash_lookup (&htab->stub_hash_table,
					      stub_name, FALSE, FALSE);
		  if (stub_entry != NULL)
		    {
		      /* The proper stub has already been created.  */
		      free (stub_name);
		      continue;
		    }

		  stub_entry = elfNN_aarch64_add_stub (stub_name, section,
						       htab);
		  if (stub_entry == NULL)
		    {
		      free (stub_name);
		      goto error_ret_free_internal;
		    }

		  stub_entry->target_value = sym_value;
		  stub_entry->target_section = sym_sec;
		  stub_entry->stub_type = stub_type;
		  stub_entry->h = hash;
		  stub_entry->st_type = st_type;

		  if (sym_name == NULL)
		    sym_name = "unnamed";
		  len = sizeof (STUB_ENTRY_NAME) + strlen (sym_name);
		  stub_entry->output_name = bfd_alloc (htab->stub_bfd, len);
		  if (stub_entry->output_name == NULL)
		    {
		      free (stub_name);
		      goto error_ret_free_internal;
		    }

		  snprintf (stub_entry->output_name, len, STUB_ENTRY_NAME,
			    sym_name);

		  stub_changed = TRUE;
		}

	      /* We're done with the internal relocs, free them.  */
	      if (elf_section_data (section)->relocs == NULL)
		free (internal_relocs);
	    }

	  if (htab->fix_erratum_835769)
	    {
	      /* Scan for sequences which might trigger erratum 835769.  */
	      if (erratum_835769_scan (input_bfd, info, &erratum_835769_fixes,
				       &num_erratum_835769_fixes,
				       &erratum_835769_fix_table_size)  != 0)
		goto error_ret_free_local;
	    }
	}

      if (prev_num_erratum_835769_fixes != num_erratum_835769_fixes)
	stub_changed = TRUE;

      if (!stub_changed)
	break;

      /* OK, we've added some stubs.  Find out the new size of the
         stub sections.  */
      for (stub_sec = htab->stub_bfd->sections;
	   stub_sec != NULL; stub_sec = stub_sec->next)
	{
	  /* Ignore non-stub sections.  */
	  if (!strstr (stub_sec->name, STUB_SUFFIX))
	    continue;
	  stub_sec->size = 0;
	}

      bfd_hash_traverse (&htab->stub_hash_table, aarch64_size_one_stub, htab);

      /* Add erratum 835769 veneers to stub section sizes too.  */
      if (htab->fix_erratum_835769)
	for (i = 0; i < num_erratum_835769_fixes; i++)
	  {
	    stub_sec = elf_aarch64_create_or_find_stub_sec (NULL,
			 erratum_835769_fixes[i].section, htab);

	    if (stub_sec == NULL)
	      goto error_ret_free_local;

	    stub_sec->size += 8;
	  }

      /* Ask the linker to do its stuff.  */
      (*htab->layout_sections_again) ();
      stub_changed = FALSE;
    }

  /* Add stubs for erratum 835769 fixes now.  */
  if (htab->fix_erratum_835769)
    {
      for (i = 0; i < num_erratum_835769_fixes; i++)
	{
	  struct elf_aarch64_stub_hash_entry *stub_entry;
	  char *stub_name = erratum_835769_fixes[i].stub_name;
	  asection *section = erratum_835769_fixes[i].section;
	  unsigned int section_id = erratum_835769_fixes[i].section->id;
	  asection *link_sec = htab->stub_group[section_id].link_sec;
	  asection *stub_sec = htab->stub_group[section_id].stub_sec;

	  stub_entry = aarch64_stub_hash_lookup (&htab->stub_hash_table,
						 stub_name, TRUE, FALSE);
	  if (stub_entry == NULL)
	    {
	      (*_bfd_error_handler) (_("%s: cannot create stub entry %s"),
				     section->owner,
				     stub_name);
	      return FALSE;
	    }

	  stub_entry->stub_sec = stub_sec;
	  stub_entry->stub_offset = 0;
	  stub_entry->id_sec = link_sec;
	  stub_entry->stub_type = erratum_835769_fixes[i].stub_type;
	  stub_entry->target_section = section;
	  stub_entry->target_value = erratum_835769_fixes[i].offset;
	  stub_entry->veneered_insn = erratum_835769_fixes[i].veneered_insn;
	  stub_entry->output_name = erratum_835769_fixes[i].stub_name;
	}

      /* Stash the erratum 835769 fix array for use later in
	 elfNN_aarch64_write_section().  */
      htab->aarch64_erratum_835769_fixes = erratum_835769_fixes;
      htab->num_aarch64_erratum_835769_fixes = num_erratum_835769_fixes;
    }
  else
    {
      htab->aarch64_erratum_835769_fixes = NULL;
      htab->num_aarch64_erratum_835769_fixes = 0;
    }

  return TRUE;

error_ret_free_local:
  return FALSE;
}

/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  We also set up the .plt entries for statically linked PIC
   functions here.  This function is called via aarch64_elf_finish in the
   linker.  */

bfd_boolean
elfNN_aarch64_build_stubs (struct bfd_link_info *info)
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  struct elf_aarch64_link_hash_table *htab;

  htab = elf_aarch64_hash_table (info);

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL; stub_sec = stub_sec->next)
    {
      bfd_size_type size;

      /* Ignore non-stub sections.  */
      if (!strstr (stub_sec->name, STUB_SUFFIX))
	continue;

      /* Allocate memory to hold the linker stubs.  */
      size = stub_sec->size;
      stub_sec->contents = bfd_zalloc (htab->stub_bfd, size);
      if (stub_sec->contents == NULL && size != 0)
	return FALSE;
      stub_sec->size = 0;
    }

  /* Build the stubs as directed by the stub hash table.  */
  table = &htab->stub_hash_table;
  bfd_hash_traverse (table, aarch64_build_one_stub, info);

  return TRUE;
}


/* Add an entry to the code/data map for section SEC.  */

static void
elfNN_aarch64_section_map_add (asection *sec, char type, bfd_vma vma)
{
  struct _aarch64_elf_section_data *sec_data =
    elf_aarch64_section_data (sec);
  unsigned int newidx;

  if (sec_data->map == NULL)
    {
      sec_data->map = bfd_malloc (sizeof (elf_aarch64_section_map));
      sec_data->mapcount = 0;
      sec_data->mapsize = 1;
    }

  newidx = sec_data->mapcount++;

  if (sec_data->mapcount > sec_data->mapsize)
    {
      sec_data->mapsize *= 2;
      sec_data->map = bfd_realloc_or_free
	(sec_data->map, sec_data->mapsize * sizeof (elf_aarch64_section_map));
    }

  if (sec_data->map)
    {
      sec_data->map[newidx].vma = vma;
      sec_data->map[newidx].type = type;
    }
}


/* Initialise maps of insn/data for input BFDs.  */
void
bfd_elfNN_aarch64_init_maps (bfd *abfd)
{
  Elf_Internal_Sym *isymbuf;
  Elf_Internal_Shdr *hdr;
  unsigned int i, localsyms;

  /* Make sure that we are dealing with an AArch64 elf binary.  */
  if (!is_aarch64_elf (abfd))
    return;

  if ((abfd->flags & DYNAMIC) != 0)
   return;

  hdr = &elf_symtab_hdr (abfd);
  localsyms = hdr->sh_info;

  /* Obtain a buffer full of symbols for this BFD. The hdr->sh_info field
     should contain the number of local symbols, which should come before any
     global symbols.  Mapping symbols are always local.  */
  isymbuf = bfd_elf_get_elf_syms (abfd, hdr, localsyms, 0, NULL, NULL, NULL);

  /* No internal symbols read?  Skip this BFD.  */
  if (isymbuf == NULL)
    return;

  for (i = 0; i < localsyms; i++)
    {
      Elf_Internal_Sym *isym = &isymbuf[i];
      asection *sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      const char *name;

      if (sec != NULL && ELF_ST_BIND (isym->st_info) == STB_LOCAL)
	{
	  name = bfd_elf_string_from_elf_section (abfd,
						  hdr->sh_link,
						  isym->st_name);

	  if (bfd_is_aarch64_special_symbol_name
	      (name, BFD_AARCH64_SPECIAL_SYM_TYPE_MAP))
	    elfNN_aarch64_section_map_add (sec, name[1], isym->st_value);
	}
    }
}

/* Set option values needed during linking.  */
void
bfd_elfNN_aarch64_set_options (struct bfd *output_bfd,
			       struct bfd_link_info *link_info,
			       int no_enum_warn,
			       int no_wchar_warn, int pic_veneer,
			       int fix_erratum_835769)
{
  struct elf_aarch64_link_hash_table *globals;

  globals = elf_aarch64_hash_table (link_info);
  globals->pic_veneer = pic_veneer;
  globals->fix_erratum_835769 = fix_erratum_835769;

  BFD_ASSERT (is_aarch64_elf (output_bfd));
  elf_aarch64_tdata (output_bfd)->no_enum_size_warning = no_enum_warn;
  elf_aarch64_tdata (output_bfd)->no_wchar_size_warning = no_wchar_warn;
}

static bfd_vma
aarch64_calculate_got_entry_vma (struct elf_link_hash_entry *h,
				 struct elf_aarch64_link_hash_table
				 *globals, struct bfd_link_info *info,
				 bfd_vma value, bfd *output_bfd,
				 bfd_boolean *unresolved_reloc_p)
{
  bfd_vma off = (bfd_vma) - 1;
  asection *basegot = globals->root.sgot;
  bfd_boolean dyn = globals->root.dynamic_sections_created;

  if (h != NULL)
    {
      BFD_ASSERT (basegot != NULL);
      off = h->got.offset;
      BFD_ASSERT (off != (bfd_vma) - 1);
      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, info->shared, h)
	  || (info->shared
	      && SYMBOL_REFERENCES_LOCAL (info, h))
	  || (ELF_ST_VISIBILITY (h->other)
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This is actually a static link, or it is a -Bsymbolic link
	     and the symbol is defined locally.  We must initialize this
	     entry in the global offset table.  Since the offset must
	     always be a multiple of 8 (4 in the case of ILP32), we use
	     the least significant bit to record whether we have
	     initialized it already.
	     When doing a dynamic link, we create a .rel(a).got relocation
	     entry to initialize the value.  This is done in the
	     finish_dynamic_symbol routine.  */
	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      bfd_put_NN (output_bfd, value, basegot->contents + off);
	      h->got.offset |= 1;
	    }
	}
      else
	*unresolved_reloc_p = FALSE;

      off = off + basegot->output_section->vma + basegot->output_offset;
    }

  return off;
}

/* Change R_TYPE to a more efficient access model where possible,
   return the new reloc type.  */

static bfd_reloc_code_real_type
aarch64_tls_transition_without_check (bfd_reloc_code_real_type r_type,
				      struct elf_link_hash_entry *h)
{
  bfd_boolean is_local = h == NULL;

  switch (r_type)
    {
    case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
      return (is_local
	      ? BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1
	      : BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21);

    case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_LDNN_LO12_NC:
      return (is_local
	      ? BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC
	      : BFD_RELOC_AARCH64_TLSIE_LDNN_GOTTPREL_LO12_NC);

    case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
      return is_local ? BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1 : r_type;

    case BFD_RELOC_AARCH64_TLSIE_LDNN_GOTTPREL_LO12_NC:
      return is_local ? BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC : r_type;

    case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_CALL:
      /* Instructions with these relocations will become NOPs.  */
      return BFD_RELOC_AARCH64_NONE;

    default:
      break;
    }

  return r_type;
}

static unsigned int
aarch64_reloc_got_type (bfd_reloc_code_real_type r_type)
{
  switch (r_type)
    {
    case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
    case BFD_RELOC_AARCH64_GOT_LD_PREL19:
      return GOT_NORMAL;

    case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
      return GOT_TLS_GD;

    case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSDESC_CALL:
    case BFD_RELOC_AARCH64_TLSDESC_LD64_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC:
      return GOT_TLSDESC_GD;

    case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
    case BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC:
      return GOT_TLS_IE;

    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
      return GOT_UNKNOWN;

    default:
      break;
    }
  return GOT_UNKNOWN;
}

static bfd_boolean
aarch64_can_relax_tls (bfd *input_bfd,
		       struct bfd_link_info *info,
		       bfd_reloc_code_real_type r_type,
		       struct elf_link_hash_entry *h,
		       unsigned long r_symndx)
{
  unsigned int symbol_got_type;
  unsigned int reloc_got_type;

  if (! IS_AARCH64_TLS_RELOC (r_type))
    return FALSE;

  symbol_got_type = elfNN_aarch64_symbol_got_type (h, input_bfd, r_symndx);
  reloc_got_type = aarch64_reloc_got_type (r_type);

  if (symbol_got_type == GOT_TLS_IE && GOT_TLS_GD_ANY_P (reloc_got_type))
    return TRUE;

  if (info->shared)
    return FALSE;

  if  (h && h->root.type == bfd_link_hash_undefweak)
    return FALSE;

  return TRUE;
}

/* Given the relocation code R_TYPE, return the relaxed bfd reloc
   enumerator.  */

static bfd_reloc_code_real_type
aarch64_tls_transition (bfd *input_bfd,
			struct bfd_link_info *info,
			unsigned int r_type,
			struct elf_link_hash_entry *h,
			unsigned long r_symndx)
{
  bfd_reloc_code_real_type bfd_r_type
    = elfNN_aarch64_bfd_reloc_from_type (r_type);

  if (! aarch64_can_relax_tls (input_bfd, info, bfd_r_type, h, r_symndx))
    return bfd_r_type;

  return aarch64_tls_transition_without_check (bfd_r_type, h);
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving R_AARCH64_TLS_DTPREL relocation.  */

static bfd_vma
dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  BFD_ASSERT (elf_hash_table (info)->tls_sec != NULL);
  return elf_hash_table (info)->tls_sec->vma;
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving R_AARCH64_TLS_GOTTPREL64 relocations.  */

static bfd_vma
tpoff_base (struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* If tls_sec is NULL, we should have signalled an error already.  */
  BFD_ASSERT (htab->tls_sec != NULL);

  bfd_vma base = align_power ((bfd_vma) TCB_SIZE,
			      htab->tls_sec->alignment_power);
  return htab->tls_sec->vma - base;
}

static bfd_vma *
symbol_got_offset_ref (bfd *input_bfd, struct elf_link_hash_entry *h,
		       unsigned long r_symndx)
{
  /* Calculate the address of the GOT entry for symbol
     referred to in h.  */
  if (h != NULL)
    return &h->got.offset;
  else
    {
      /* local symbol */
      struct elf_aarch64_local_symbol *l;

      l = elf_aarch64_locals (input_bfd);
      return &l[r_symndx].got_offset;
    }
}

static void
symbol_got_offset_mark (bfd *input_bfd, struct elf_link_hash_entry *h,
			unsigned long r_symndx)
{
  bfd_vma *p;
  p = symbol_got_offset_ref (input_bfd, h, r_symndx);
  *p |= 1;
}

static int
symbol_got_offset_mark_p (bfd *input_bfd, struct elf_link_hash_entry *h,
			  unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_got_offset_ref (input_bfd, h, r_symndx);
  return value & 1;
}

static bfd_vma
symbol_got_offset (bfd *input_bfd, struct elf_link_hash_entry *h,
		   unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_got_offset_ref (input_bfd, h, r_symndx);
  value &= ~1;
  return value;
}

static bfd_vma *
symbol_tlsdesc_got_offset_ref (bfd *input_bfd, struct elf_link_hash_entry *h,
			       unsigned long r_symndx)
{
  /* Calculate the address of the GOT entry for symbol
     referred to in h.  */
  if (h != NULL)
    {
      struct elf_aarch64_link_hash_entry *eh;
      eh = (struct elf_aarch64_link_hash_entry *) h;
      return &eh->tlsdesc_got_jump_table_offset;
    }
  else
    {
      /* local symbol */
      struct elf_aarch64_local_symbol *l;

      l = elf_aarch64_locals (input_bfd);
      return &l[r_symndx].tlsdesc_got_jump_table_offset;
    }
}

static void
symbol_tlsdesc_got_offset_mark (bfd *input_bfd, struct elf_link_hash_entry *h,
				unsigned long r_symndx)
{
  bfd_vma *p;
  p = symbol_tlsdesc_got_offset_ref (input_bfd, h, r_symndx);
  *p |= 1;
}

static int
symbol_tlsdesc_got_offset_mark_p (bfd *input_bfd,
				  struct elf_link_hash_entry *h,
				  unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_tlsdesc_got_offset_ref (input_bfd, h, r_symndx);
  return value & 1;
}

static bfd_vma
symbol_tlsdesc_got_offset (bfd *input_bfd, struct elf_link_hash_entry *h,
			  unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_tlsdesc_got_offset_ref (input_bfd, h, r_symndx);
  value &= ~1;
  return value;
}

/* Data for make_branch_to_erratum_835769_stub().  */

struct erratum_835769_branch_to_stub_data
{
  asection *output_section;
  bfd_byte *contents;
};

/* Helper to insert branches to erratum 835769 stubs in the right
   places for a particular section.  */

static bfd_boolean
make_branch_to_erratum_835769_stub (struct bfd_hash_entry *gen_entry,
				    void *in_arg)
{
  struct elf_aarch64_stub_hash_entry *stub_entry;
  struct erratum_835769_branch_to_stub_data *data;
  bfd_byte *contents;
  unsigned long branch_insn = 0;
  bfd_vma veneered_insn_loc, veneer_entry_loc;
  bfd_signed_vma branch_offset;
  unsigned int target;
  bfd *abfd;

  stub_entry = (struct elf_aarch64_stub_hash_entry *) gen_entry;
  data = (struct erratum_835769_branch_to_stub_data *) in_arg;

  if (stub_entry->target_section != data->output_section
      || stub_entry->stub_type != aarch64_stub_erratum_835769_veneer)
    return TRUE;

  contents = data->contents;
  veneered_insn_loc = stub_entry->target_section->output_section->vma
		      + stub_entry->target_section->output_offset
		      + stub_entry->target_value;
  veneer_entry_loc = stub_entry->stub_sec->output_section->vma
		     + stub_entry->stub_sec->output_offset
		     + stub_entry->stub_offset;
  branch_offset = veneer_entry_loc - veneered_insn_loc;

  abfd = stub_entry->target_section->owner;
  if (!aarch64_valid_branch_p (veneer_entry_loc, veneered_insn_loc))
	    (*_bfd_error_handler)
		(_("%B: error: Erratum 835769 stub out "
		   "of range (input file too large)"), abfd);

  target = stub_entry->target_value;
  branch_insn = 0x14000000;
  branch_offset >>= 2;
  branch_offset &= 0x3ffffff;
  branch_insn |= branch_offset;
  bfd_putl32 (branch_insn, &contents[target]);

  return TRUE;
}

static bfd_boolean
elfNN_aarch64_write_section (bfd *output_bfd  ATTRIBUTE_UNUSED,
			     struct bfd_link_info *link_info,
			     asection *sec,
			     bfd_byte *contents)

{
  struct elf_aarch64_link_hash_table *globals =
					elf_aarch64_hash_table (link_info);

  if (globals == NULL)
    return FALSE;

  /* Fix code to point to erratum 835769 stubs.  */
  if (globals->fix_erratum_835769)
    {
      struct erratum_835769_branch_to_stub_data data;

      data.output_section = sec;
      data.contents = contents;
      bfd_hash_traverse (&globals->stub_hash_table,
			 make_branch_to_erratum_835769_stub, &data);
    }

  return FALSE;
}

/* Perform a relocation as part of a final link.  */
static bfd_reloc_status_type
elfNN_aarch64_final_link_relocate (reloc_howto_type *howto,
				   bfd *input_bfd,
				   bfd *output_bfd,
				   asection *input_section,
				   bfd_byte *contents,
				   Elf_Internal_Rela *rel,
				   bfd_vma value,
				   struct bfd_link_info *info,
				   asection *sym_sec,
				   struct elf_link_hash_entry *h,
				   bfd_boolean *unresolved_reloc_p,
				   bfd_boolean save_addend,
				   bfd_vma *saved_addend,
				   Elf_Internal_Sym *sym)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int r_type = howto->type;
  bfd_reloc_code_real_type bfd_r_type
    = elfNN_aarch64_bfd_reloc_from_howto (howto);
  bfd_reloc_code_real_type new_bfd_r_type;
  unsigned long r_symndx;
  bfd_byte *hit_data = contents + rel->r_offset;
  bfd_vma place, off;
  bfd_signed_vma signed_addend;
  struct elf_aarch64_link_hash_table *globals;
  bfd_boolean weak_undef_p;
  asection *base_got;

  globals = elf_aarch64_hash_table (info);

  symtab_hdr = &elf_symtab_hdr (input_bfd);

  BFD_ASSERT (is_aarch64_elf (input_bfd));

  r_symndx = ELFNN_R_SYM (rel->r_info);

  /* It is possible to have linker relaxations on some TLS access
     models.  Update our information here.  */
  new_bfd_r_type = aarch64_tls_transition (input_bfd, info, r_type, h, r_symndx);
  if (new_bfd_r_type != bfd_r_type)
    {
      bfd_r_type = new_bfd_r_type;
      howto = elfNN_aarch64_howto_from_bfd_reloc (bfd_r_type);
      BFD_ASSERT (howto != NULL);
      r_type = howto->type;
    }

  place = input_section->output_section->vma
    + input_section->output_offset + rel->r_offset;

  /* Get addend, accumulating the addend for consecutive relocs
     which refer to the same offset.  */
  signed_addend = saved_addend ? *saved_addend : 0;
  signed_addend += rel->r_addend;

  weak_undef_p = (h ? h->root.type == bfd_link_hash_undefweak
		  : bfd_is_und_section (sym_sec));

  /* Since STT_GNU_IFUNC symbol must go through PLT, we handle
     it here if it is defined in a non-shared object.  */
  if (h != NULL
      && h->type == STT_GNU_IFUNC
      && h->def_regular)
    {
      asection *plt;
      const char *name;

      if ((input_section->flags & SEC_ALLOC) == 0
	  || h->plt.offset == (bfd_vma) -1)
	abort ();

      /* STT_GNU_IFUNC symbol must go through PLT.  */
      plt = globals->root.splt ? globals->root.splt : globals->root.iplt;
      value = (plt->output_section->vma + plt->output_offset + h->plt.offset);

      switch (bfd_r_type)
	{
	default:
	  if (h->root.root.string)
	    name = h->root.root.string;
	  else
	    name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
				     NULL);
	  (*_bfd_error_handler)
	    (_("%B: relocation %s against STT_GNU_IFUNC "
	       "symbol `%s' isn't handled by %s"), input_bfd,
	     howto->name, name, __FUNCTION__);
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;

	case BFD_RELOC_AARCH64_NN:
	  if (rel->r_addend != 0)
	    {
	      if (h->root.root.string)
		name = h->root.root.string;
	      else
		name = bfd_elf_sym_name (input_bfd, symtab_hdr,
					 sym, NULL);
	      (*_bfd_error_handler)
		(_("%B: relocation %s against STT_GNU_IFUNC "
		   "symbol `%s' has non-zero addend: %d"),
		 input_bfd, howto->name, name, rel->r_addend);
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }

	  /* Generate dynamic relocation only when there is a
	     non-GOT reference in a shared object.  */
	  if (info->shared && h->non_got_ref)
	    {
	      Elf_Internal_Rela outrel;
	      asection *sreloc;

	      /* Need a dynamic relocation to get the real function
		 address.  */
	      outrel.r_offset = _bfd_elf_section_offset (output_bfd,
							 info,
							 input_section,
							 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) -1
		  || outrel.r_offset == (bfd_vma) -2)
		abort ();

	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      if (h->dynindx == -1
		  || h->forced_local
		  || info->executable)
		{
		  /* This symbol is resolved locally.  */
		  outrel.r_info = ELFNN_R_INFO (0, AARCH64_R (IRELATIVE));
		  outrel.r_addend = (h->root.u.def.value
				     + h->root.u.def.section->output_section->vma
				     + h->root.u.def.section->output_offset);
		}
	      else
		{
		  outrel.r_info = ELFNN_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = 0;
		}

	      sreloc = globals->root.irelifunc;
	      elf_append_rela (output_bfd, sreloc, &outrel);

	      /* If this reloc is against an external symbol, we
		 do not want to fiddle with the addend.  Otherwise,
		 we need to include the symbol value so that it
		 becomes an addend for the dynamic reloc.  For an
		 internal symbol, we have updated addend.  */
	      return bfd_reloc_ok;
	    }
	  /* FALLTHROUGH */
	case BFD_RELOC_AARCH64_JUMP26:
	case BFD_RELOC_AARCH64_CALL26:
	  value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						       signed_addend,
						       weak_undef_p);
	  return _bfd_aarch64_elf_put_addend (input_bfd, hit_data, bfd_r_type,
					      howto, value);
	case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
	case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
	case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
	case BFD_RELOC_AARCH64_GOT_LD_PREL19:
	  base_got = globals->root.sgot;
	  off = h->got.offset;

	  if (base_got == NULL)
	    abort ();

	  if (off == (bfd_vma) -1)
	    {
	      bfd_vma plt_index;

	      /* We can't use h->got.offset here to save state, or
		 even just remember the offset, as finish_dynamic_symbol
		 would use that as offset into .got.  */

	      if (globals->root.splt != NULL)
		{
		  plt_index = ((h->plt.offset - globals->plt_header_size) /
			       globals->plt_entry_size);
		  off = (plt_index + 3) * GOT_ENTRY_SIZE;
		  base_got = globals->root.sgotplt;
		}
	      else
		{
		  plt_index = h->plt.offset / globals->plt_entry_size;
		  off = plt_index * GOT_ENTRY_SIZE;
		  base_got = globals->root.igotplt;
		}

	      if (h->dynindx == -1
		  || h->forced_local
		  || info->symbolic)
		{
		  /* This references the local definition.  We must
		     initialize this entry in the global offset table.
		     Since the offset must always be a multiple of 8,
		     we use the least significant bit to record
		     whether we have initialized it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.	 */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_NN (output_bfd, value,
				  base_got->contents + off);
		      /* Note that this is harmless as -1 | 1 still is -1.  */
		      h->got.offset |= 1;
		    }
		}
	      value = (base_got->output_section->vma
		       + base_got->output_offset + off);
	    }
	  else
	    value = aarch64_calculate_got_entry_vma (h, globals, info,
						     value, output_bfd,
						     unresolved_reloc_p);
	  value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						       0, weak_undef_p);
	  return _bfd_aarch64_elf_put_addend (input_bfd, hit_data, bfd_r_type, howto, value);
	case BFD_RELOC_AARCH64_ADR_HI21_PCREL:
	case BFD_RELOC_AARCH64_ADD_LO12:
	  break;
	}
    }

  switch (bfd_r_type)
    {
    case BFD_RELOC_AARCH64_NONE:
    case BFD_RELOC_AARCH64_TLSDESC_CALL:
      *unresolved_reloc_p = FALSE;
      return bfd_reloc_ok;

    case BFD_RELOC_AARCH64_NN:

      /* When generating a shared object or relocatable executable, these
         relocations are copied into the output file to be resolved at
         run time.  */
      if (((info->shared == TRUE) || globals->root.is_relocatable_executable)
	  && (input_section->flags & SEC_ALLOC)
	  && (h == NULL
	      || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	      || h->root.type != bfd_link_hash_undefweak))
	{
	  Elf_Internal_Rela outrel;
	  bfd_byte *loc;
	  bfd_boolean skip, relocate;
	  asection *sreloc;

	  *unresolved_reloc_p = FALSE;

	  skip = FALSE;
	  relocate = FALSE;

	  outrel.r_addend = signed_addend;
	  outrel.r_offset =
	    _bfd_elf_section_offset (output_bfd, info, input_section,
				     rel->r_offset);
	  if (outrel.r_offset == (bfd_vma) - 1)
	    skip = TRUE;
	  else if (outrel.r_offset == (bfd_vma) - 2)
	    {
	      skip = TRUE;
	      relocate = TRUE;
	    }

	  outrel.r_offset += (input_section->output_section->vma
			      + input_section->output_offset);

	  if (skip)
	    memset (&outrel, 0, sizeof outrel);
	  else if (h != NULL
		   && h->dynindx != -1
		   && (!info->shared || !SYMBOLIC_BIND (info, h) || !h->def_regular))
	    outrel.r_info = ELFNN_R_INFO (h->dynindx, r_type);
	  else
	    {
	      int symbol;

	      /* On SVR4-ish systems, the dynamic loader cannot
		 relocate the text and data segments independently,
		 so the symbol does not matter.  */
	      symbol = 0;
	      outrel.r_info = ELFNN_R_INFO (symbol, AARCH64_R (RELATIVE));
	      outrel.r_addend += value;
	    }

	  sreloc = elf_section_data (input_section)->sreloc;
	  if (sreloc == NULL || sreloc->contents == NULL)
	    return bfd_reloc_notsupported;

	  loc = sreloc->contents + sreloc->reloc_count++ * RELOC_SIZE (globals);
	  bfd_elfNN_swap_reloca_out (output_bfd, &outrel, loc);

	  if (sreloc->reloc_count * RELOC_SIZE (globals) > sreloc->size)
	    {
	      /* Sanity to check that we have previously allocated
		 sufficient space in the relocation section for the
		 number of relocations we actually want to emit.  */
	      abort ();
	    }

	  /* If this reloc is against an external symbol, we do not want to
	     fiddle with the addend.  Otherwise, we need to include the symbol
	     value so that it becomes an addend for the dynamic reloc.  */
	  if (!relocate)
	    return bfd_reloc_ok;

	  return _bfd_final_link_relocate (howto, input_bfd, input_section,
					   contents, rel->r_offset, value,
					   signed_addend);
	}
      else
	value += signed_addend;
      break;

    case BFD_RELOC_AARCH64_JUMP26:
    case BFD_RELOC_AARCH64_CALL26:
      {
	asection *splt = globals->root.splt;
	bfd_boolean via_plt_p =
	  splt != NULL && h != NULL && h->plt.offset != (bfd_vma) - 1;

	/* A call to an undefined weak symbol is converted to a jump to
	   the next instruction unless a PLT entry will be created.
	   The jump to the next instruction is optimized as a NOP.
	   Do the same for local undefined symbols.  */
	if (weak_undef_p && ! via_plt_p)
	  {
	    bfd_putl32 (INSN_NOP, hit_data);
	    return bfd_reloc_ok;
	  }

	/* If the call goes through a PLT entry, make sure to
	   check distance to the right destination address.  */
	if (via_plt_p)
	  {
	    value = (splt->output_section->vma
		     + splt->output_offset + h->plt.offset);
	    *unresolved_reloc_p = FALSE;
	  }

	/* If the target symbol is global and marked as a function the
	   relocation applies a function call or a tail call.  In this
	   situation we can veneer out of range branches.  The veneers
	   use IP0 and IP1 hence cannot be used arbitrary out of range
	   branches that occur within the body of a function.  */
	if (h && h->type == STT_FUNC)
	  {
	    /* Check if a stub has to be inserted because the destination
	       is too far away.  */
	    if (! aarch64_valid_branch_p (value, place))
	      {
		/* The target is out of reach, so redirect the branch to
		   the local stub for this function.  */
		struct elf_aarch64_stub_hash_entry *stub_entry;
		stub_entry = elfNN_aarch64_get_stub_entry (input_section,
							   sym_sec, h,
							   rel, globals);
		if (stub_entry != NULL)
		  value = (stub_entry->stub_offset
			   + stub_entry->stub_sec->output_offset
			   + stub_entry->stub_sec->output_section->vma);
	      }
	  }
      }
      value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						   signed_addend, weak_undef_p);
      break;

    case BFD_RELOC_AARCH64_ADR_LO21_PCREL:
    case BFD_RELOC_AARCH64_ADR_HI21_PCREL:
    case BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL:
    case BFD_RELOC_AARCH64_LD_LO19_PCREL:
    case BFD_RELOC_AARCH64_16_PCREL:
    case BFD_RELOC_AARCH64_32_PCREL:
    case BFD_RELOC_AARCH64_64_PCREL:
      if (info->shared
	  && (input_section->flags & SEC_ALLOC) != 0
	  && (input_section->flags & SEC_READONLY) != 0
	  && h != NULL
	  && !h->def_regular)
	{
	  int howto_index = bfd_r_type - BFD_RELOC_AARCH64_RELOC_START;

	  (*_bfd_error_handler)
	    (_("%B: relocation %s against external symbol `%s' can not be used"
	       " when making a shared object; recompile with -fPIC"),
	     input_bfd, elfNN_aarch64_howto_table[howto_index].name,
	     h->root.root.string);
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

    case BFD_RELOC_AARCH64_16:
#if ARCH_SIZE == 64
    case BFD_RELOC_AARCH64_32:
#endif
    case BFD_RELOC_AARCH64_ADD_LO12:
    case BFD_RELOC_AARCH64_BRANCH19:
    case BFD_RELOC_AARCH64_LDST8_LO12:
    case BFD_RELOC_AARCH64_LDST16_LO12:
    case BFD_RELOC_AARCH64_LDST32_LO12:
    case BFD_RELOC_AARCH64_LDST64_LO12:
    case BFD_RELOC_AARCH64_LDST128_LO12:
    case BFD_RELOC_AARCH64_MOVW_G0_S:
    case BFD_RELOC_AARCH64_MOVW_G1_S:
    case BFD_RELOC_AARCH64_MOVW_G2_S:
    case BFD_RELOC_AARCH64_MOVW_G0:
    case BFD_RELOC_AARCH64_MOVW_G0_NC:
    case BFD_RELOC_AARCH64_MOVW_G1:
    case BFD_RELOC_AARCH64_MOVW_G1_NC:
    case BFD_RELOC_AARCH64_MOVW_G2:
    case BFD_RELOC_AARCH64_MOVW_G2_NC:
    case BFD_RELOC_AARCH64_MOVW_G3:
    case BFD_RELOC_AARCH64_TSTBR14:
      value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						   signed_addend, weak_undef_p);
      break;

    case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
    case BFD_RELOC_AARCH64_GOT_LD_PREL19:
      if (globals->root.sgot == NULL)
	BFD_ASSERT (h != NULL);

      if (h != NULL)
	{
	  value = aarch64_calculate_got_entry_vma (h, globals, info, value,
						   output_bfd,
						   unresolved_reloc_p);
	  value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						       0, weak_undef_p);
	}
      else
      {
	struct elf_aarch64_local_symbol *locals
	  = elf_aarch64_locals (input_bfd);

	if (locals == NULL)
	  {
	    int howto_index = bfd_r_type - BFD_RELOC_AARCH64_RELOC_START;
	    (*_bfd_error_handler)
	      (_("%B: Local symbol descriptor table be NULL when applying "
		 "relocation %s against local symbol"),
	       input_bfd, elfNN_aarch64_howto_table[howto_index].name);
	    abort ();
	  }

	off = symbol_got_offset (input_bfd, h, r_symndx);
	base_got = globals->root.sgot;
	bfd_vma got_entry_addr = (base_got->output_section->vma
				  + base_got->output_offset + off);

	if (!symbol_got_offset_mark_p (input_bfd, h, r_symndx))
	  {
	    bfd_put_64 (output_bfd, value, base_got->contents + off);

	    if (info->shared)
	      {
		asection *s;
		Elf_Internal_Rela outrel;

		/* For local symbol, we have done absolute relocation in static
		   linking stageh. While for share library, we need to update
		   the content of GOT entry according to the share objects
		   loading base address. So we need to generate a
		   R_AARCH64_RELATIVE reloc for dynamic linker.  */
		s = globals->root.srelgot;
		if (s == NULL)
		  abort ();

		outrel.r_offset = got_entry_addr;
		outrel.r_info = ELFNN_R_INFO (0, AARCH64_R (RELATIVE));
		outrel.r_addend = value;
		elf_append_rela (output_bfd, s, &outrel);
	      }

	    symbol_got_offset_mark (input_bfd, h, r_symndx);
	  }

	/* Update the relocation value to GOT entry addr as we have transformed
	   the direct data access into indirect data access through GOT.  */
	value = got_entry_addr;
      }

      break;

    case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
    case BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC:
      if (globals->root.sgot == NULL)
	return bfd_reloc_notsupported;

      value = (symbol_got_offset (input_bfd, h, r_symndx)
	       + globals->root.sgot->output_section->vma
	       + globals->root.sgot->output_offset);

      value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						   0, weak_undef_p);
      *unresolved_reloc_p = FALSE;
      break;

    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
      value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						   signed_addend - tpoff_base (info),
						   weak_undef_p);
      *unresolved_reloc_p = FALSE;
      break;

    case BFD_RELOC_AARCH64_TLSDESC_ADD:
    case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_LD64_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_LDR:
      if (globals->root.sgot == NULL)
	return bfd_reloc_notsupported;
      value = (symbol_tlsdesc_got_offset (input_bfd, h, r_symndx)
	       + globals->root.sgotplt->output_section->vma
	       + globals->root.sgotplt->output_offset
	       + globals->sgotplt_jump_table_size);

      value = _bfd_aarch64_elf_resolve_relocation (bfd_r_type, place, value,
						   0, weak_undef_p);
      *unresolved_reloc_p = FALSE;
      break;

    default:
      return bfd_reloc_notsupported;
    }

  if (saved_addend)
    *saved_addend = value;

  /* Only apply the final relocation in a sequence.  */
  if (save_addend)
    return bfd_reloc_continue;

  return _bfd_aarch64_elf_put_addend (input_bfd, hit_data, bfd_r_type,
				      howto, value);
}

/* Handle TLS relaxations.  Relaxing is possible for symbols that use
   R_AARCH64_TLSDESC_ADR_{PAGE, LD64_LO12_NC, ADD_LO12_NC} during a static
   link.

   Return bfd_reloc_ok if we're done, bfd_reloc_continue if the caller
   is to then call final_link_relocate.  Return other values in the
   case of error.  */

static bfd_reloc_status_type
elfNN_aarch64_tls_relax (struct elf_aarch64_link_hash_table *globals,
			 bfd *input_bfd, bfd_byte *contents,
			 Elf_Internal_Rela *rel, struct elf_link_hash_entry *h)
{
  bfd_boolean is_local = h == NULL;
  unsigned int r_type = ELFNN_R_TYPE (rel->r_info);
  unsigned long insn;

  BFD_ASSERT (globals && input_bfd && contents && rel);

  switch (elfNN_aarch64_bfd_reloc_from_type (r_type))
    {
    case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
      if (is_local)
	{
	  /* GD->LE relaxation:
	     adrp x0, :tlsgd:var     =>   movz x0, :tprel_g1:var
	     or
	     adrp x0, :tlsdesc:var   =>   movz x0, :tprel_g1:var
	   */
	  bfd_putl32 (0xd2a00000, contents + rel->r_offset);
	  return bfd_reloc_continue;
	}
      else
	{
	  /* GD->IE relaxation:
	     adrp x0, :tlsgd:var     =>   adrp x0, :gottprel:var
	     or
	     adrp x0, :tlsdesc:var   =>   adrp x0, :gottprel:var
	   */
	  return bfd_reloc_continue;
	}

    case BFD_RELOC_AARCH64_TLSDESC_LDNN_LO12_NC:
      if (is_local)
	{
	  /* GD->LE relaxation:
	     ldr xd, [x0, #:tlsdesc_lo12:var]   =>   movk x0, :tprel_g0_nc:var
	   */
	  bfd_putl32 (0xf2800000, contents + rel->r_offset);
	  return bfd_reloc_continue;
	}
      else
	{
	  /* GD->IE relaxation:
	     ldr xd, [x0, #:tlsdesc_lo12:var] => ldr x0, [x0, #:gottprel_lo12:var]
	   */
	  insn = bfd_getl32 (contents + rel->r_offset);
	  insn &= 0xffffffe0;
	  bfd_putl32 (insn, contents + rel->r_offset);
	  return bfd_reloc_continue;
	}

    case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
      if (is_local)
	{
	  /* GD->LE relaxation
	     add  x0, #:tlsgd_lo12:var  => movk x0, :tprel_g0_nc:var
	     bl   __tls_get_addr        => mrs  x1, tpidr_el0
	     nop                        => add  x0, x1, x0
	   */

	  /* First kill the tls_get_addr reloc on the bl instruction.  */
	  BFD_ASSERT (rel->r_offset + 4 == rel[1].r_offset);
	  rel[1].r_info = ELFNN_R_INFO (STN_UNDEF, R_AARCH64_NONE);

	  bfd_putl32 (0xf2800000, contents + rel->r_offset);
	  bfd_putl32 (0xd53bd041, contents + rel->r_offset + 4);
	  bfd_putl32 (0x8b000020, contents + rel->r_offset + 8);
	  return bfd_reloc_continue;
	}
      else
	{
	  /* GD->IE relaxation
	     ADD  x0, #:tlsgd_lo12:var  => ldr  x0, [x0, #:gottprel_lo12:var]
	     BL   __tls_get_addr        => mrs  x1, tpidr_el0
	       R_AARCH64_CALL26
	     NOP                        => add  x0, x1, x0
	   */

	  BFD_ASSERT (ELFNN_R_TYPE (rel[1].r_info) == AARCH64_R (CALL26));

	  /* Remove the relocation on the BL instruction.  */
	  rel[1].r_info = ELFNN_R_INFO (STN_UNDEF, R_AARCH64_NONE);

	  bfd_putl32 (0xf9400000, contents + rel->r_offset);

	  /* We choose to fixup the BL and NOP instructions using the
	     offset from the second relocation to allow flexibility in
	     scheduling instructions between the ADD and BL.  */
	  bfd_putl32 (0xd53bd041, contents + rel[1].r_offset);
	  bfd_putl32 (0x8b000020, contents + rel[1].r_offset + 4);
	  return bfd_reloc_continue;
	}

    case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_CALL:
      /* GD->IE/LE relaxation:
         add x0, x0, #:tlsdesc_lo12:var   =>   nop
         blr xd                           =>   nop
       */
      bfd_putl32 (INSN_NOP, contents + rel->r_offset);
      return bfd_reloc_ok;

    case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
      /* IE->LE relaxation:
         adrp xd, :gottprel:var   =>   movz xd, :tprel_g1:var
       */
      if (is_local)
	{
	  insn = bfd_getl32 (contents + rel->r_offset);
	  bfd_putl32 (0xd2a00000 | (insn & 0x1f), contents + rel->r_offset);
	}
      return bfd_reloc_continue;

    case BFD_RELOC_AARCH64_TLSIE_LDNN_GOTTPREL_LO12_NC:
      /* IE->LE relaxation:
         ldr  xd, [xm, #:gottprel_lo12:var]   =>   movk xd, :tprel_g0_nc:var
       */
      if (is_local)
	{
	  insn = bfd_getl32 (contents + rel->r_offset);
	  bfd_putl32 (0xf2800000 | (insn & 0x1f), contents + rel->r_offset);
	}
      return bfd_reloc_continue;

    default:
      return bfd_reloc_continue;
    }

  return bfd_reloc_ok;
}

/* Relocate an AArch64 ELF section.  */

static bfd_boolean
elfNN_aarch64_relocate_section (bfd *output_bfd,
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
  const char *name;
  struct elf_aarch64_link_hash_table *globals;
  bfd_boolean save_addend = FALSE;
  bfd_vma addend = 0;

  globals = elf_aarch64_hash_table (info);

  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      unsigned int r_type;
      bfd_reloc_code_real_type bfd_r_type;
      bfd_reloc_code_real_type relaxed_bfd_r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      arelent bfd_reloc;
      char sym_type;
      bfd_boolean unresolved_reloc = FALSE;
      char *error_message = NULL;

      r_symndx = ELFNN_R_SYM (rel->r_info);
      r_type = ELFNN_R_TYPE (rel->r_info);

      bfd_reloc.howto = elfNN_aarch64_howto_from_type (r_type);
      howto = bfd_reloc.howto;

      if (howto == NULL)
	{
	  (*_bfd_error_handler)
	    (_("%B: unrecognized relocation (0x%x) in section `%A'"),
	     input_bfd, input_section, r_type);
	  return FALSE;
	}
      bfd_r_type = elfNN_aarch64_bfd_reloc_from_howto (howto);

      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sym_type = ELFNN_ST_TYPE (sym->st_info);
	  sec = local_sections[r_symndx];

	  /* An object file might have a reference to a local
	     undefined symbol.  This is a daft object file, but we
	     should at least do something about it.  */
	  if (r_type != R_AARCH64_NONE && r_type != R_AARCH64_NULL
	      && bfd_is_und_section (sec)
	      && ELF_ST_BIND (sym->st_info) != STB_WEAK)
	    {
	      if (!info->callbacks->undefined_symbol
		  (info, bfd_elf_string_from_elf_section
		   (input_bfd, symtab_hdr->sh_link, sym->st_name),
		   input_bfd, input_section, rel->r_offset, TRUE))
		return FALSE;
	    }

	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  /* Relocate against local STT_GNU_IFUNC symbol.  */
	  if (!info->relocatable
	      && ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elfNN_aarch64_get_local_sym_hash (globals, input_bfd,
						    rel, FALSE);
	      if (h == NULL)
		abort ();

	      /* Set STT_GNU_IFUNC symbol value.  */
	      h->root.u.def.value = sym->st_value;
	      h->root.u.def.section = sec;
	    }
	}
      else
	{
	  bfd_boolean warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  sym_type = h->type;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (info->relocatable)
	continue;

      if (h != NULL)
	name = h->root.root.string;
      else
	{
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd, symtab_hdr->sh_link, sym->st_name));
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (input_bfd, sec);
	}

      if (r_symndx != 0
	  && r_type != R_AARCH64_NONE
	  && r_type != R_AARCH64_NULL
	  && (h == NULL
	      || h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	  && IS_AARCH64_TLS_RELOC (bfd_r_type) != (sym_type == STT_TLS))
	{
	  (*_bfd_error_handler)
	    ((sym_type == STT_TLS
	      ? _("%B(%A+0x%lx): %s used with TLS symbol %s")
	      : _("%B(%A+0x%lx): %s used with non-TLS symbol %s")),
	     input_bfd,
	     input_section, (long) rel->r_offset, howto->name, name);
	}

      /* We relax only if we can see that there can be a valid transition
         from a reloc type to another.
         We call elfNN_aarch64_final_link_relocate unless we're completely
         done, i.e., the relaxation produced the final output we want.  */

      relaxed_bfd_r_type = aarch64_tls_transition (input_bfd, info, r_type,
						   h, r_symndx);
      if (relaxed_bfd_r_type != bfd_r_type)
	{
	  bfd_r_type = relaxed_bfd_r_type;
	  howto = elfNN_aarch64_howto_from_bfd_reloc (bfd_r_type);
	  BFD_ASSERT (howto != NULL);
	  r_type = howto->type;
	  r = elfNN_aarch64_tls_relax (globals, input_bfd, contents, rel, h);
	  unresolved_reloc = 0;
	}
      else
	r = bfd_reloc_continue;

      /* There may be multiple consecutive relocations for the
         same offset.  In that case we are supposed to treat the
         output of each relocation as the addend for the next.  */
      if (rel + 1 < relend
	  && rel->r_offset == rel[1].r_offset
	  && ELFNN_R_TYPE (rel[1].r_info) != R_AARCH64_NONE
	  && ELFNN_R_TYPE (rel[1].r_info) != R_AARCH64_NULL)
	save_addend = TRUE;
      else
	save_addend = FALSE;

      if (r == bfd_reloc_continue)
	r = elfNN_aarch64_final_link_relocate (howto, input_bfd, output_bfd,
					       input_section, contents, rel,
					       relocation, info, sec,
					       h, &unresolved_reloc,
					       save_addend, &addend, sym);

      switch (elfNN_aarch64_bfd_reloc_from_type (r_type))
	{
	case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
	case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
	  if (! symbol_got_offset_mark_p (input_bfd, h, r_symndx))
	    {
	      bfd_boolean need_relocs = FALSE;
	      bfd_byte *loc;
	      int indx;
	      bfd_vma off;

	      off = symbol_got_offset (input_bfd, h, r_symndx);
	      indx = h && h->dynindx != -1 ? h->dynindx : 0;

	      need_relocs =
		(info->shared || indx != 0) &&
		(h == NULL
		 || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		 || h->root.type != bfd_link_hash_undefweak);

	      BFD_ASSERT (globals->root.srelgot != NULL);

	      if (need_relocs)
		{
		  Elf_Internal_Rela rela;
		  rela.r_info = ELFNN_R_INFO (indx, AARCH64_R (TLS_DTPMOD));
		  rela.r_addend = 0;
		  rela.r_offset = globals->root.sgot->output_section->vma +
		    globals->root.sgot->output_offset + off;


		  loc = globals->root.srelgot->contents;
		  loc += globals->root.srelgot->reloc_count++
		    * RELOC_SIZE (htab);
		  bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);

		  if (indx == 0)
		    {
		      bfd_put_NN (output_bfd,
				  relocation - dtpoff_base (info),
				  globals->root.sgot->contents + off
				  + GOT_ENTRY_SIZE);
		    }
		  else
		    {
		      /* This TLS symbol is global. We emit a
			 relocation to fixup the tls offset at load
			 time.  */
		      rela.r_info =
			ELFNN_R_INFO (indx, AARCH64_R (TLS_DTPREL));
		      rela.r_addend = 0;
		      rela.r_offset =
			(globals->root.sgot->output_section->vma
			 + globals->root.sgot->output_offset + off
			 + GOT_ENTRY_SIZE);

		      loc = globals->root.srelgot->contents;
		      loc += globals->root.srelgot->reloc_count++
			* RELOC_SIZE (globals);
		      bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);
		      bfd_put_NN (output_bfd, (bfd_vma) 0,
				  globals->root.sgot->contents + off
				  + GOT_ENTRY_SIZE);
		    }
		}
	      else
		{
		  bfd_put_NN (output_bfd, (bfd_vma) 1,
			      globals->root.sgot->contents + off);
		  bfd_put_NN (output_bfd,
			      relocation - dtpoff_base (info),
			      globals->root.sgot->contents + off
			      + GOT_ENTRY_SIZE);
		}

	      symbol_got_offset_mark (input_bfd, h, r_symndx);
	    }
	  break;

	case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
	case BFD_RELOC_AARCH64_TLSIE_LDNN_GOTTPREL_LO12_NC:
	  if (! symbol_got_offset_mark_p (input_bfd, h, r_symndx))
	    {
	      bfd_boolean need_relocs = FALSE;
	      bfd_byte *loc;
	      int indx;
	      bfd_vma off;

	      off = symbol_got_offset (input_bfd, h, r_symndx);

	      indx = h && h->dynindx != -1 ? h->dynindx : 0;

	      need_relocs =
		(info->shared || indx != 0) &&
		(h == NULL
		 || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		 || h->root.type != bfd_link_hash_undefweak);

	      BFD_ASSERT (globals->root.srelgot != NULL);

	      if (need_relocs)
		{
		  Elf_Internal_Rela rela;

		  if (indx == 0)
		    rela.r_addend = relocation - dtpoff_base (info);
		  else
		    rela.r_addend = 0;

		  rela.r_info = ELFNN_R_INFO (indx, AARCH64_R (TLS_TPREL));
		  rela.r_offset = globals->root.sgot->output_section->vma +
		    globals->root.sgot->output_offset + off;

		  loc = globals->root.srelgot->contents;
		  loc += globals->root.srelgot->reloc_count++
		    * RELOC_SIZE (htab);

		  bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);

		  bfd_put_NN (output_bfd, rela.r_addend,
			      globals->root.sgot->contents + off);
		}
	      else
		bfd_put_NN (output_bfd, relocation - tpoff_base (info),
			    globals->root.sgot->contents + off);

	      symbol_got_offset_mark (input_bfd, h, r_symndx);
	    }
	  break;

	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
	  break;

	case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC:
	case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
	case BFD_RELOC_AARCH64_TLSDESC_LDNN_LO12_NC:
	  if (! symbol_tlsdesc_got_offset_mark_p (input_bfd, h, r_symndx))
	    {
	      bfd_boolean need_relocs = FALSE;
	      int indx = h && h->dynindx != -1 ? h->dynindx : 0;
	      bfd_vma off = symbol_tlsdesc_got_offset (input_bfd, h, r_symndx);

	      need_relocs = (h == NULL
			     || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			     || h->root.type != bfd_link_hash_undefweak);

	      BFD_ASSERT (globals->root.srelgot != NULL);
	      BFD_ASSERT (globals->root.sgot != NULL);

	      if (need_relocs)
		{
		  bfd_byte *loc;
		  Elf_Internal_Rela rela;
		  rela.r_info = ELFNN_R_INFO (indx, AARCH64_R (TLSDESC));

		  rela.r_addend = 0;
		  rela.r_offset = (globals->root.sgotplt->output_section->vma
				   + globals->root.sgotplt->output_offset
				   + off + globals->sgotplt_jump_table_size);

		  if (indx == 0)
		    rela.r_addend = relocation - dtpoff_base (info);

		  /* Allocate the next available slot in the PLT reloc
		     section to hold our R_AARCH64_TLSDESC, the next
		     available slot is determined from reloc_count,
		     which we step. But note, reloc_count was
		     artifically moved down while allocating slots for
		     real PLT relocs such that all of the PLT relocs
		     will fit above the initial reloc_count and the
		     extra stuff will fit below.  */
		  loc = globals->root.srelplt->contents;
		  loc += globals->root.srelplt->reloc_count++
		    * RELOC_SIZE (globals);

		  bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);

		  bfd_put_NN (output_bfd, (bfd_vma) 0,
			      globals->root.sgotplt->contents + off +
			      globals->sgotplt_jump_table_size);
		  bfd_put_NN (output_bfd, (bfd_vma) 0,
			      globals->root.sgotplt->contents + off +
			      globals->sgotplt_jump_table_size +
			      GOT_ENTRY_SIZE);
		}

	      symbol_tlsdesc_got_offset_mark (input_bfd, h, r_symndx);
	    }
	  break;
	default:
	  break;
	}

      if (!save_addend)
	addend = 0;


      /* Dynamic relocs are not propagated for SEC_DEBUGGING sections
         because such sections are not SEC_ALLOC and thus ld.so will
         not process them.  */
      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->def_dynamic)
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      +rel->r_offset) != (bfd_vma) - 1)
	{
	  (*_bfd_error_handler)
	    (_
	     ("%B(%A+0x%lx): unresolvable %s relocation against symbol `%s'"),
	     input_bfd, input_section, (long) rel->r_offset, howto->name,
	     h->root.root.string);
	  return FALSE;
	}

      if (r != bfd_reloc_ok && r != bfd_reloc_continue)
	{
	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      /* If the overflowing reloc was to an undefined symbol,
		 we have already printed one error message and there
		 is no point complaining again.  */
	      if ((!h ||
		   h->root.type != bfd_link_hash_undefined)
		  && (!((*info->callbacks->reloc_overflow)
			(info, (h ? &h->root : NULL), name, howto->name,
			 (bfd_vma) 0, input_bfd, input_section,
			 rel->r_offset))))
		return FALSE;
	      break;

	    case bfd_reloc_undefined:
	      if (!((*info->callbacks->undefined_symbol)
		    (info, name, input_bfd, input_section,
		     rel->r_offset, TRUE)))
		return FALSE;
	      break;

	    case bfd_reloc_outofrange:
	      error_message = _("out of range");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      error_message = _("unsupported relocation");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      /* error_message should already be set.  */
	      goto common_error;

	    default:
	      error_message = _("unknown error");
	      /* Fall through.  */

	    common_error:
	      BFD_ASSERT (error_message != NULL);
	      if (!((*info->callbacks->reloc_dangerous)
		    (info, error_message, input_bfd, input_section,
		     rel->r_offset)))
		return FALSE;
	      break;
	    }
	}
    }

  return TRUE;
}

/* Set the right machine number.  */

static bfd_boolean
elfNN_aarch64_object_p (bfd *abfd)
{
#if ARCH_SIZE == 32
  bfd_default_set_arch_mach (abfd, bfd_arch_aarch64, bfd_mach_aarch64_ilp32);
#else
  bfd_default_set_arch_mach (abfd, bfd_arch_aarch64, bfd_mach_aarch64);
#endif
  return TRUE;
}

/* Function to keep AArch64 specific flags in the ELF header.  */

static bfd_boolean
elfNN_aarch64_set_private_flags (bfd *abfd, flagword flags)
{
  if (elf_flags_init (abfd) && elf_elfheader (abfd)->e_flags != flags)
    {
    }
  else
    {
      elf_elfheader (abfd)->e_flags = flags;
      elf_flags_init (abfd) = TRUE;
    }

  return TRUE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bfd_boolean
elfNN_aarch64_merge_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  flagword out_flags;
  flagword in_flags;
  bfd_boolean flags_compatible = TRUE;
  asection *sec;

  /* Check if we have the same endianess.  */
  if (!_bfd_generic_verify_endian_match (ibfd, obfd))
    return FALSE;

  if (!is_aarch64_elf (ibfd) || !is_aarch64_elf (obfd))
    return TRUE;

  /* The input BFD must have had its flags initialised.  */
  /* The following seems bogus to me -- The flags are initialized in
     the assembler but I don't think an elf_flags_init field is
     written into the object.  */
  /* BFD_ASSERT (elf_flags_init (ibfd)); */

  in_flags = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (!elf_flags_init (obfd))
    {
      /* If the input is the default architecture and had the default
         flags then do not bother setting the flags for the output
         architecture, instead allow future merges to do this.  If no
         future merges ever set these flags then they will retain their
         uninitialised values, which surprise surprise, correspond
         to the default values.  */
      if (bfd_get_arch_info (ibfd)->the_default
	  && elf_elfheader (ibfd)->e_flags == 0)
	return TRUE;

      elf_flags_init (obfd) = TRUE;
      elf_elfheader (obfd)->e_flags = in_flags;

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && bfd_get_arch_info (obfd)->the_default)
	return bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
				  bfd_get_mach (ibfd));

      return TRUE;
    }

  /* Identical flags must be compatible.  */
  if (in_flags == out_flags)
    return TRUE;

  /* Check to see if the input BFD actually contains any sections.  If
     not, its flags may not have been initialised either, but it
     cannot actually cause any incompatiblity.  Do not short-circuit
     dynamic objects; their section list may be emptied by
     elf_link_add_object_symbols.

     Also check to see if there are no code sections in the input.
     In this case there is no need to check for code specific flags.
     XXX - do we need to worry about floating-point format compatability
     in data sections ?  */
  if (!(ibfd->flags & DYNAMIC))
    {
      bfd_boolean null_input_bfd = TRUE;
      bfd_boolean only_data_sections = TRUE;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  if ((bfd_get_section_flags (ibfd, sec)
	       & (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	      == (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	    only_data_sections = FALSE;

	  null_input_bfd = FALSE;
	  break;
	}

      if (null_input_bfd || only_data_sections)
	return TRUE;
    }

  return flags_compatible;
}

/* Display the flags field.  */

static bfd_boolean
elfNN_aarch64_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;
  unsigned long flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  /* Ignore init flag - it may not be set, despite the flags field
     containing valid data.  */

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (flags)
    fprintf (file, _("<Unrecognised flag bits set>"));

  fputc ('\n', file);

  return TRUE;
}

/* Update the got entry reference counts for the section being removed.  */

static bfd_boolean
elfNN_aarch64_gc_sweep_hook (bfd *abfd,
			     struct bfd_link_info *info,
			     asection *sec,
			     const Elf_Internal_Rela * relocs)
{
  struct elf_aarch64_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_aarch64_local_symbol *locals;
  const Elf_Internal_Rela *rel, *relend;

  if (info->relocatable)
    return TRUE;

  htab = elf_aarch64_hash_table (info);

  if (htab == NULL)
    return FALSE;

  elf_section_data (sec)->local_dynrel = NULL;

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  locals = elf_aarch64_locals (abfd);

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      unsigned long r_symndx;
      unsigned int r_type;
      struct elf_link_hash_entry *h = NULL;

      r_symndx = ELFNN_R_SYM (rel->r_info);

      if (r_symndx >= symtab_hdr->sh_info)
	{

	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
        }
      else
	{
	  Elf_Internal_Sym *isym;

	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					abfd, r_symndx);

	  /* Check relocation against local STT_GNU_IFUNC symbol.  */
	  if (isym != NULL
	      && ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elfNN_aarch64_get_local_sym_hash (htab, abfd, rel, FALSE);
	      if (h == NULL)
		abort ();
	    }
	}

      if (h)
	{
	  struct elf_aarch64_link_hash_entry *eh;
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  eh = (struct elf_aarch64_link_hash_entry *) h;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; pp = &p->next)
	    if (p->sec == sec)
	      {
		/* Everything must go for SEC.  */
		*pp = p->next;
		break;
	      }
	}

      r_type = ELFNN_R_TYPE (rel->r_info);
      switch (aarch64_tls_transition (abfd,info, r_type, h ,r_symndx))
	{
	case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
	case BFD_RELOC_AARCH64_GOT_LD_PREL19:
	case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
	case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
	case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC:
	case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
	case BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC:
	case BFD_RELOC_AARCH64_TLSDESC_LD64_LO12_NC:
	case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
	case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
	case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
	case BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC:
	case BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
	  if (h != NULL)
	    {
	      if (h->got.refcount > 0)
		h->got.refcount -= 1;

	      if (h->type == STT_GNU_IFUNC)
		{
		  if (h->plt.refcount > 0)
		    h->plt.refcount -= 1;
		}
	    }
	  else if (locals != NULL)
	    {
	      if (locals[r_symndx].got_refcount > 0)
		locals[r_symndx].got_refcount -= 1;
	    }
	  break;

	case BFD_RELOC_AARCH64_CALL26:
	case BFD_RELOC_AARCH64_JUMP26:
	  /* If this is a local symbol then we resolve it
	     directly without creating a PLT entry.  */
	  if (h == NULL)
	    continue;

	  if (h->plt.refcount > 0)
	    h->plt.refcount -= 1;
	  break;

	case BFD_RELOC_AARCH64_MOVW_G0_NC:
	case BFD_RELOC_AARCH64_MOVW_G1_NC:
	case BFD_RELOC_AARCH64_MOVW_G2_NC:
	case BFD_RELOC_AARCH64_MOVW_G3:
	case BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL:
	case BFD_RELOC_AARCH64_ADR_HI21_PCREL:
	case BFD_RELOC_AARCH64_ADR_LO21_PCREL:
	case BFD_RELOC_AARCH64_NN:
	  if (h != NULL && info->executable)
	    {
	      if (h->plt.refcount > 0)
		h->plt.refcount -= 1;
	    }
	  break;

	default:
	  break;
	}
    }

  return TRUE;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.	*/

static bfd_boolean
elfNN_aarch64_adjust_dynamic_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *h)
{
  struct elf_aarch64_link_hash_table *htab;
  asection *s;

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC || h->type == STT_GNU_IFUNC || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || (h->type != STT_GNU_IFUNC
	      && (SYMBOL_CALLS_LOCAL (info, h)
		  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		      && h->root.type == bfd_link_hash_undefweak))))
	{
	  /* This case can occur if we saw a CALL26 reloc in
	     an input file, but the symbol wasn't referred to
	     by a dynamic object or all references were
	     garbage collected. In which case we can end up
	     resolving.  */
	  h->plt.offset = (bfd_vma) - 1;
	  h->needs_plt = 0;
	}

      return TRUE;
    }
  else
    /* It's possible that we incorrectly decided a .plt reloc was
       needed for an R_X86_64_PC32 reloc to a non-function sym in
       check_relocs.  We can't decide accurately between function and
       non-function syms in check-relocs;  Objects loaded later in
       the link may change h->type.  So fix it now.  */
    h->plt.offset = (bfd_vma) - 1;


  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
		  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
      if (ELIMINATE_COPY_RELOCS || info->nocopyreloc)
	h->non_got_ref = h->u.weakdef->non_got_ref;
      return TRUE;
    }

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

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  htab = elf_aarch64_hash_table (info);

  /* We must generate a R_AARCH64_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      htab->srelbss->size += RELOC_SIZE (htab);
      h->needs_copy = 1;
    }

  s = htab->sdynbss;

  return _bfd_elf_adjust_dynamic_copy (info, h, s);

}

static bfd_boolean
elfNN_aarch64_allocate_local_symbols (bfd *abfd, unsigned number)
{
  struct elf_aarch64_local_symbol *locals;
  locals = elf_aarch64_locals (abfd);
  if (locals == NULL)
    {
      locals = (struct elf_aarch64_local_symbol *)
	bfd_zalloc (abfd, number * sizeof (struct elf_aarch64_local_symbol));
      if (locals == NULL)
	return FALSE;
      elf_aarch64_locals (abfd) = locals;
    }
  return TRUE;
}

/* Create the .got section to hold the global offset table.  */

static bfd_boolean
aarch64_elf_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  flagword flags;
  asection *s;
  struct elf_link_hash_entry *h;
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* This function may be called more than once.  */
  s = bfd_get_linker_section (abfd, ".got");
  if (s != NULL)
    return TRUE;

  flags = bed->dynamic_sec_flags;

  s = bfd_make_section_anyway_with_flags (abfd,
					  (bed->rela_plts_and_copies_p
					   ? ".rela.got" : ".rel.got"),
					  (bed->dynamic_sec_flags
					   | SEC_READONLY));
  if (s == NULL
      || ! bfd_set_section_alignment (abfd, s, bed->s->log_file_align))
    return FALSE;
  htab->srelgot = s;

  s = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  if (s == NULL
      || !bfd_set_section_alignment (abfd, s, bed->s->log_file_align))
    return FALSE;
  htab->sgot = s;
  htab->sgot->size += GOT_ENTRY_SIZE;

  if (bed->want_got_sym)
    {
      /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
	 (or .got.plt) section.  We don't do this in the linker script
	 because we don't want to define the symbol if we are not creating
	 a global offset table.  */
      h = _bfd_elf_define_linkage_sym (abfd, info, s,
				       "_GLOBAL_OFFSET_TABLE_");
      elf_hash_table (info)->hgot = h;
      if (h == NULL)
	return FALSE;
    }

  if (bed->want_got_plt)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".got.plt", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (abfd, s,
					 bed->s->log_file_align))
	return FALSE;
      htab->sgotplt = s;
    }

  /* The first bit of the global offset table is the header.  */
  s->size += bed->got_header_size;

  return TRUE;
}

/* Look through the relocs for a section during the first phase.  */

static bfd_boolean
elfNN_aarch64_check_relocs (bfd *abfd, struct bfd_link_info *info,
			    asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;

  struct elf_aarch64_link_hash_table *htab;

  if (info->relocatable)
    return TRUE;

  BFD_ASSERT (is_aarch64_elf (abfd));

  htab = elf_aarch64_hash_table (info);
  sreloc = NULL;

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      unsigned int r_type;
      bfd_reloc_code_real_type bfd_r_type;
      Elf_Internal_Sym *isym;

      r_symndx = ELFNN_R_SYM (rel->r_info);
      r_type = ELFNN_R_TYPE (rel->r_info);

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  (*_bfd_error_handler) (_("%B: bad symbol index: %d"), abfd,
				 r_symndx);
	  return FALSE;
	}

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					abfd, r_symndx);
	  if (isym == NULL)
	    return FALSE;

	  /* Check relocation against local STT_GNU_IFUNC symbol.  */
	  if (ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elfNN_aarch64_get_local_sym_hash (htab, abfd, rel,
						    TRUE);
	      if (h == NULL)
		return FALSE;

	      /* Fake a STT_GNU_IFUNC symbol.  */
	      h->type = STT_GNU_IFUNC;
	      h->def_regular = 1;
	      h->ref_regular = 1;
	      h->forced_local = 1;
	      h->root.type = bfd_link_hash_defined;
	    }
	  else
	    h = NULL;
	}
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

      /* Could be done earlier, if h were already available.  */
      bfd_r_type = aarch64_tls_transition (abfd, info, r_type, h, r_symndx);

      if (h != NULL)
	{
	  /* Create the ifunc sections for static executables.  If we
	     never see an indirect function symbol nor we are building
	     a static executable, those sections will be empty and
	     won't appear in output.  */
	  switch (bfd_r_type)
	    {
	    default:
	      break;

	    case BFD_RELOC_AARCH64_NN:
	    case BFD_RELOC_AARCH64_CALL26:
	    case BFD_RELOC_AARCH64_JUMP26:
	    case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
	    case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
	    case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
	    case BFD_RELOC_AARCH64_GOT_LD_PREL19:
	    case BFD_RELOC_AARCH64_ADR_HI21_PCREL:
	    case BFD_RELOC_AARCH64_ADD_LO12:
	      if (htab->root.dynobj == NULL)
		htab->root.dynobj = abfd;
	      if (!_bfd_elf_create_ifunc_sections (htab->root.dynobj, info))
		return FALSE;
	      break;
	    }

	  /* It is referenced by a non-shared object. */
	  h->ref_regular = 1;
	  h->root.non_ir_ref = 1;
	}

      switch (bfd_r_type)
	{
	case BFD_RELOC_AARCH64_NN:

	  /* We don't need to handle relocs into sections not going into
	     the "real" output.  */
	  if ((sec->flags & SEC_ALLOC) == 0)
	    break;

	  if (h != NULL)
	    {
	      if (!info->shared)
		h->non_got_ref = 1;

	      h->plt.refcount += 1;
	      h->pointer_equality_needed = 1;
	    }

	  /* No need to do anything if we're not creating a shared
	     object.  */
	  if (! info->shared)
	    break;

	  {
	    struct elf_dyn_relocs *p;
	    struct elf_dyn_relocs **head;

	    /* We must copy these reloc types into the output file.
	       Create a reloc section in dynobj and make room for
	       this reloc.  */
	    if (sreloc == NULL)
	      {
		if (htab->root.dynobj == NULL)
		  htab->root.dynobj = abfd;

		sreloc = _bfd_elf_make_dynamic_reloc_section
		  (sec, htab->root.dynobj, LOG_FILE_ALIGN, abfd, /*rela? */ TRUE);

		if (sreloc == NULL)
		  return FALSE;
	      }

	    /* If this is a global symbol, we count the number of
	       relocations we need for this symbol.  */
	    if (h != NULL)
	      {
		struct elf_aarch64_link_hash_entry *eh;
		eh = (struct elf_aarch64_link_hash_entry *) h;
		head = &eh->dyn_relocs;
	      }
	    else
	      {
		/* Track dynamic relocs needed for local syms too.
		   We really need local syms available to do this
		   easily.  Oh well.  */

		asection *s;
		void **vpp;

		isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					      abfd, r_symndx);
		if (isym == NULL)
		  return FALSE;

		s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		if (s == NULL)
		  s = sec;

		/* Beware of type punned pointers vs strict aliasing
		   rules.  */
		vpp = &(elf_section_data (s)->local_dynrel);
		head = (struct elf_dyn_relocs **) vpp;
	      }

	    p = *head;
	    if (p == NULL || p->sec != sec)
	      {
		bfd_size_type amt = sizeof *p;
		p = ((struct elf_dyn_relocs *)
		     bfd_zalloc (htab->root.dynobj, amt));
		if (p == NULL)
		  return FALSE;
		p->next = *head;
		*head = p;
		p->sec = sec;
	      }

	    p->count += 1;

	  }
	  break;

	  /* RR: We probably want to keep a consistency check that
	     there are no dangling GOT_PAGE relocs.  */
	case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
	case BFD_RELOC_AARCH64_GOT_LD_PREL19:
	case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
	case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
	case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12_NC:
	case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
	case BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC:
	case BFD_RELOC_AARCH64_TLSDESC_LD64_LO12_NC:
	case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
	case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
	case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
	case BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC:
	case BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12:
	case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
	case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
	  {
	    unsigned got_type;
	    unsigned old_got_type;

	    got_type = aarch64_reloc_got_type (bfd_r_type);

	    if (h)
	      {
		h->got.refcount += 1;
		old_got_type = elf_aarch64_hash_entry (h)->got_type;
	      }
	    else
	      {
		struct elf_aarch64_local_symbol *locals;

		if (!elfNN_aarch64_allocate_local_symbols
		    (abfd, symtab_hdr->sh_info))
		  return FALSE;

		locals = elf_aarch64_locals (abfd);
		BFD_ASSERT (r_symndx < symtab_hdr->sh_info);
		locals[r_symndx].got_refcount += 1;
		old_got_type = locals[r_symndx].got_type;
	      }

	    /* If a variable is accessed with both general dynamic TLS
	       methods, two slots may be created.  */
	    if (GOT_TLS_GD_ANY_P (old_got_type) && GOT_TLS_GD_ANY_P (got_type))
	      got_type |= old_got_type;

	    /* We will already have issued an error message if there
	       is a TLS/non-TLS mismatch, based on the symbol type.
	       So just combine any TLS types needed.  */
	    if (old_got_type != GOT_UNKNOWN && old_got_type != GOT_NORMAL
		&& got_type != GOT_NORMAL)
	      got_type |= old_got_type;

	    /* If the symbol is accessed by both IE and GD methods, we
	       are able to relax.  Turn off the GD flag, without
	       messing up with any other kind of TLS types that may be
	       involved.  */
	    if ((got_type & GOT_TLS_IE) && GOT_TLS_GD_ANY_P (got_type))
	      got_type &= ~ (GOT_TLSDESC_GD | GOT_TLS_GD);

	    if (old_got_type != got_type)
	      {
		if (h != NULL)
		  elf_aarch64_hash_entry (h)->got_type = got_type;
		else
		  {
		    struct elf_aarch64_local_symbol *locals;
		    locals = elf_aarch64_locals (abfd);
		    BFD_ASSERT (r_symndx < symtab_hdr->sh_info);
		    locals[r_symndx].got_type = got_type;
		  }
	      }

	    if (htab->root.dynobj == NULL)
	      htab->root.dynobj = abfd;
	    if (! aarch64_elf_create_got_section (htab->root.dynobj, info))
	      return FALSE;
	    break;
	  }

	case BFD_RELOC_AARCH64_MOVW_G0_NC:
	case BFD_RELOC_AARCH64_MOVW_G1_NC:
	case BFD_RELOC_AARCH64_MOVW_G2_NC:
	case BFD_RELOC_AARCH64_MOVW_G3:
	  if (info->shared)
	    {
	      int howto_index = bfd_r_type - BFD_RELOC_AARCH64_RELOC_START;
	      (*_bfd_error_handler)
		(_("%B: relocation %s against `%s' can not be used when making "
		   "a shared object; recompile with -fPIC"),
		 abfd, elfNN_aarch64_howto_table[howto_index].name,
		 (h) ? h->root.root.string : "a local symbol");
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }

	case BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL:
	case BFD_RELOC_AARCH64_ADR_HI21_PCREL:
	case BFD_RELOC_AARCH64_ADR_LO21_PCREL:
	  if (h != NULL && info->executable)
	    {
	      /* If this reloc is in a read-only section, we might
		 need a copy reloc.  We can't check reliably at this
		 stage whether the section is read-only, as input
		 sections have not yet been mapped to output sections.
		 Tentatively set the flag for now, and correct in
		 adjust_dynamic_symbol.  */
	      h->non_got_ref = 1;
	      h->plt.refcount += 1;
	      h->pointer_equality_needed = 1;
	    }
	  /* FIXME:: RR need to handle these in shared libraries
	     and essentially bomb out as these being non-PIC
	     relocations in shared libraries.  */
	  break;

	case BFD_RELOC_AARCH64_CALL26:
	case BFD_RELOC_AARCH64_JUMP26:
	  /* If this is a local symbol then we resolve it
	     directly without creating a PLT entry.  */
	  if (h == NULL)
	    continue;

	  h->needs_plt = 1;
	  if (h->plt.refcount <= 0)
	    h->plt.refcount = 1;
	  else
	    h->plt.refcount += 1;
	  break;

	default:
	  break;
	}
    }

  return TRUE;
}

/* Treat mapping symbols as special target symbols.  */

static bfd_boolean
elfNN_aarch64_is_target_special_symbol (bfd *abfd ATTRIBUTE_UNUSED,
					asymbol *sym)
{
  return bfd_is_aarch64_special_symbol_name (sym->name,
					     BFD_AARCH64_SPECIAL_SYM_TYPE_ANY);
}

/* This is a copy of elf_find_function () from elf.c except that
   AArch64 mapping symbols are ignored when looking for function names.  */

static bfd_boolean
aarch64_elf_find_function (bfd *abfd ATTRIBUTE_UNUSED,
			   asymbol **symbols,
			   asection *section,
			   bfd_vma offset,
			   const char **filename_ptr,
			   const char **functionname_ptr)
{
  const char *filename = NULL;
  asymbol *func = NULL;
  bfd_vma low_func = 0;
  asymbol **p;

  for (p = symbols; *p != NULL; p++)
    {
      elf_symbol_type *q;

      q = (elf_symbol_type *) * p;

      switch (ELF_ST_TYPE (q->internal_elf_sym.st_info))
	{
	default:
	  break;
	case STT_FILE:
	  filename = bfd_asymbol_name (&q->symbol);
	  break;
	case STT_FUNC:
	case STT_NOTYPE:
	  /* Skip mapping symbols.  */
	  if ((q->symbol.flags & BSF_LOCAL)
	      && (bfd_is_aarch64_special_symbol_name
		  (q->symbol.name, BFD_AARCH64_SPECIAL_SYM_TYPE_ANY)))
	    continue;
	  /* Fall through.  */
	  if (bfd_get_section (&q->symbol) == section
	      && q->symbol.value >= low_func && q->symbol.value <= offset)
	    {
	      func = (asymbol *) q;
	      low_func = q->symbol.value;
	    }
	  break;
	}
    }

  if (func == NULL)
    return FALSE;

  if (filename_ptr)
    *filename_ptr = filename;
  if (functionname_ptr)
    *functionname_ptr = bfd_asymbol_name (func);

  return TRUE;
}


/* Find the nearest line to a particular section and offset, for error
   reporting.   This code is a duplicate of the code in elf.c, except
   that it uses aarch64_elf_find_function.  */

static bfd_boolean
elfNN_aarch64_find_nearest_line (bfd *abfd,
				 asymbol **symbols,
				 asection *section,
				 bfd_vma offset,
				 const char **filename_ptr,
				 const char **functionname_ptr,
				 unsigned int *line_ptr,
				 unsigned int *discriminator_ptr)
{
  bfd_boolean found = FALSE;

  if (_bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr, discriminator_ptr,
				     dwarf_debug_sections, 0,
				     &elf_tdata (abfd)->dwarf2_find_line_info))
    {
      if (!*functionname_ptr)
	aarch64_elf_find_function (abfd, symbols, section, offset,
				   *filename_ptr ? NULL : filename_ptr,
				   functionname_ptr);

      return TRUE;
    }

  /* Skip _bfd_dwarf1_find_nearest_line since no known AArch64
     toolchain uses DWARF1.  */

  if (!_bfd_stab_section_find_nearest_line (abfd, symbols, section, offset,
					    &found, filename_ptr,
					    functionname_ptr, line_ptr,
					    &elf_tdata (abfd)->line_info))
    return FALSE;

  if (found && (*functionname_ptr || *line_ptr))
    return TRUE;

  if (symbols == NULL)
    return FALSE;

  if (!aarch64_elf_find_function (abfd, symbols, section, offset,
				  filename_ptr, functionname_ptr))
    return FALSE;

  *line_ptr = 0;
  return TRUE;
}

static bfd_boolean
elfNN_aarch64_find_inliner_info (bfd *abfd,
				 const char **filename_ptr,
				 const char **functionname_ptr,
				 unsigned int *line_ptr)
{
  bfd_boolean found;
  found = _bfd_dwarf2_find_inliner_info
    (abfd, filename_ptr,
     functionname_ptr, line_ptr, &elf_tdata (abfd)->dwarf2_find_line_info);
  return found;
}


static void
elfNN_aarch64_post_process_headers (bfd *abfd,
				    struct bfd_link_info *link_info)
{
  Elf_Internal_Ehdr *i_ehdrp;	/* ELF file header, internal form.  */

  i_ehdrp = elf_elfheader (abfd);
  i_ehdrp->e_ident[EI_ABIVERSION] = AARCH64_ELF_ABI_VERSION;

  _bfd_elf_post_process_headers (abfd, link_info);
}

static enum elf_reloc_type_class
elfNN_aarch64_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
				const asection *rel_sec ATTRIBUTE_UNUSED,
				const Elf_Internal_Rela *rela)
{
  switch ((int) ELFNN_R_TYPE (rela->r_info))
    {
    case AARCH64_R (RELATIVE):
      return reloc_class_relative;
    case AARCH64_R (JUMP_SLOT):
      return reloc_class_plt;
    case AARCH64_R (COPY):
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Handle an AArch64 specific section when reading an object file.  This is
   called when bfd_section_from_shdr finds a section with an unknown
   type.  */

static bfd_boolean
elfNN_aarch64_section_from_shdr (bfd *abfd,
				 Elf_Internal_Shdr *hdr,
				 const char *name, int shindex)
{
  /* There ought to be a place to keep ELF backend specific flags, but
     at the moment there isn't one.  We just keep track of the
     sections by their name, instead.  Fortunately, the ABI gives
     names for all the AArch64 specific sections, so we will probably get
     away with this.  */
  switch (hdr->sh_type)
    {
    case SHT_AARCH64_ATTRIBUTES:
      break;

    default:
      return FALSE;
    }

  if (!_bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return FALSE;

  return TRUE;
}

/* A structure used to record a list of sections, independently
   of the next and prev fields in the asection structure.  */
typedef struct section_list
{
  asection *sec;
  struct section_list *next;
  struct section_list *prev;
}
section_list;

/* Unfortunately we need to keep a list of sections for which
   an _aarch64_elf_section_data structure has been allocated.  This
   is because it is possible for functions like elfNN_aarch64_write_section
   to be called on a section which has had an elf_data_structure
   allocated for it (and so the used_by_bfd field is valid) but
   for which the AArch64 extended version of this structure - the
   _aarch64_elf_section_data structure - has not been allocated.  */
static section_list *sections_with_aarch64_elf_section_data = NULL;

static void
record_section_with_aarch64_elf_section_data (asection *sec)
{
  struct section_list *entry;

  entry = bfd_malloc (sizeof (*entry));
  if (entry == NULL)
    return;
  entry->sec = sec;
  entry->next = sections_with_aarch64_elf_section_data;
  entry->prev = NULL;
  if (entry->next != NULL)
    entry->next->prev = entry;
  sections_with_aarch64_elf_section_data = entry;
}

static struct section_list *
find_aarch64_elf_section_entry (asection *sec)
{
  struct section_list *entry;
  static struct section_list *last_entry = NULL;

  /* This is a short cut for the typical case where the sections are added
     to the sections_with_aarch64_elf_section_data list in forward order and
     then looked up here in backwards order.  This makes a real difference
     to the ld-srec/sec64k.exp linker test.  */
  entry = sections_with_aarch64_elf_section_data;
  if (last_entry != NULL)
    {
      if (last_entry->sec == sec)
	entry = last_entry;
      else if (last_entry->next != NULL && last_entry->next->sec == sec)
	entry = last_entry->next;
    }

  for (; entry; entry = entry->next)
    if (entry->sec == sec)
      break;

  if (entry)
    /* Record the entry prior to this one - it is the entry we are
       most likely to want to locate next time.  Also this way if we
       have been called from
       unrecord_section_with_aarch64_elf_section_data () we will not
       be caching a pointer that is about to be freed.  */
    last_entry = entry->prev;

  return entry;
}

static void
unrecord_section_with_aarch64_elf_section_data (asection *sec)
{
  struct section_list *entry;

  entry = find_aarch64_elf_section_entry (sec);

  if (entry)
    {
      if (entry->prev != NULL)
	entry->prev->next = entry->next;
      if (entry->next != NULL)
	entry->next->prev = entry->prev;
      if (entry == sections_with_aarch64_elf_section_data)
	sections_with_aarch64_elf_section_data = entry->next;
      free (entry);
    }
}


typedef struct
{
  void *finfo;
  struct bfd_link_info *info;
  asection *sec;
  int sec_shndx;
  int (*func) (void *, const char *, Elf_Internal_Sym *,
	       asection *, struct elf_link_hash_entry *);
} output_arch_syminfo;

enum map_symbol_type
{
  AARCH64_MAP_INSN,
  AARCH64_MAP_DATA
};


/* Output a single mapping symbol.  */

static bfd_boolean
elfNN_aarch64_output_map_sym (output_arch_syminfo *osi,
			      enum map_symbol_type type, bfd_vma offset)
{
  static const char *names[2] = { "$x", "$d" };
  Elf_Internal_Sym sym;

  sym.st_value = (osi->sec->output_section->vma
		  + osi->sec->output_offset + offset);
  sym.st_size = 0;
  sym.st_other = 0;
  sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_NOTYPE);
  sym.st_shndx = osi->sec_shndx;
  return osi->func (osi->finfo, names[type], &sym, osi->sec, NULL) == 1;
}



/* Output mapping symbols for PLT entries associated with H.  */

static bfd_boolean
elfNN_aarch64_output_plt_map (struct elf_link_hash_entry *h, void *inf)
{
  output_arch_syminfo *osi = (output_arch_syminfo *) inf;
  bfd_vma addr;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  if (h->plt.offset == (bfd_vma) - 1)
    return TRUE;

  addr = h->plt.offset;
  if (addr == 32)
    {
      if (!elfNN_aarch64_output_map_sym (osi, AARCH64_MAP_INSN, addr))
	return FALSE;
    }
  return TRUE;
}


/* Output a single local symbol for a generated stub.  */

static bfd_boolean
elfNN_aarch64_output_stub_sym (output_arch_syminfo *osi, const char *name,
			       bfd_vma offset, bfd_vma size)
{
  Elf_Internal_Sym sym;

  sym.st_value = (osi->sec->output_section->vma
		  + osi->sec->output_offset + offset);
  sym.st_size = size;
  sym.st_other = 0;
  sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  sym.st_shndx = osi->sec_shndx;
  return osi->func (osi->finfo, name, &sym, osi->sec, NULL) == 1;
}

static bfd_boolean
aarch64_map_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct elf_aarch64_stub_hash_entry *stub_entry;
  asection *stub_sec;
  bfd_vma addr;
  char *stub_name;
  output_arch_syminfo *osi;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf_aarch64_stub_hash_entry *) gen_entry;
  osi = (output_arch_syminfo *) in_arg;

  stub_sec = stub_entry->stub_sec;

  /* Ensure this stub is attached to the current section being
     processed.  */
  if (stub_sec != osi->sec)
    return TRUE;

  addr = (bfd_vma) stub_entry->stub_offset;

  stub_name = stub_entry->output_name;

  switch (stub_entry->stub_type)
    {
    case aarch64_stub_adrp_branch:
      if (!elfNN_aarch64_output_stub_sym (osi, stub_name, addr,
					  sizeof (aarch64_adrp_branch_stub)))
	return FALSE;
      if (!elfNN_aarch64_output_map_sym (osi, AARCH64_MAP_INSN, addr))
	return FALSE;
      break;
    case aarch64_stub_long_branch:
      if (!elfNN_aarch64_output_stub_sym
	  (osi, stub_name, addr, sizeof (aarch64_long_branch_stub)))
	return FALSE;
      if (!elfNN_aarch64_output_map_sym (osi, AARCH64_MAP_INSN, addr))
	return FALSE;
      if (!elfNN_aarch64_output_map_sym (osi, AARCH64_MAP_DATA, addr + 16))
	return FALSE;
      break;
    case aarch64_stub_erratum_835769_veneer:
      if (!elfNN_aarch64_output_stub_sym (osi, stub_name, addr,
					  sizeof (aarch64_erratum_835769_stub)))
	return FALSE;
      if (!elfNN_aarch64_output_map_sym (osi, AARCH64_MAP_INSN, addr))
	return FALSE;
      break;
    default:
      BFD_FAIL ();
    }

  return TRUE;
}

/* Output mapping symbols for linker generated sections.  */

static bfd_boolean
elfNN_aarch64_output_arch_local_syms (bfd *output_bfd,
				      struct bfd_link_info *info,
				      void *finfo,
				      int (*func) (void *, const char *,
						   Elf_Internal_Sym *,
						   asection *,
						   struct elf_link_hash_entry
						   *))
{
  output_arch_syminfo osi;
  struct elf_aarch64_link_hash_table *htab;

  htab = elf_aarch64_hash_table (info);

  osi.finfo = finfo;
  osi.info = info;
  osi.func = func;

  /* Long calls stubs.  */
  if (htab->stub_bfd && htab->stub_bfd->sections)
    {
      asection *stub_sec;

      for (stub_sec = htab->stub_bfd->sections;
	   stub_sec != NULL; stub_sec = stub_sec->next)
	{
	  /* Ignore non-stub sections.  */
	  if (!strstr (stub_sec->name, STUB_SUFFIX))
	    continue;

	  osi.sec = stub_sec;

	  osi.sec_shndx = _bfd_elf_section_from_bfd_section
	    (output_bfd, osi.sec->output_section);

	  bfd_hash_traverse (&htab->stub_hash_table, aarch64_map_one_stub,
			     &osi);
	}
    }

  /* Finally, output mapping symbols for the PLT.  */
  if (!htab->root.splt || htab->root.splt->size == 0)
    return TRUE;

  /* For now live without mapping symbols for the plt.  */
  osi.sec_shndx = _bfd_elf_section_from_bfd_section
    (output_bfd, htab->root.splt->output_section);
  osi.sec = htab->root.splt;

  elf_link_hash_traverse (&htab->root, elfNN_aarch64_output_plt_map,
			  (void *) &osi);

  return TRUE;

}

/* Allocate target specific section data.  */

static bfd_boolean
elfNN_aarch64_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      _aarch64_elf_section_data *sdata;
      bfd_size_type amt = sizeof (*sdata);

      sdata = bfd_zalloc (abfd, amt);
      if (sdata == NULL)
	return FALSE;
      sec->used_by_bfd = sdata;
    }

  record_section_with_aarch64_elf_section_data (sec);

  return _bfd_elf_new_section_hook (abfd, sec);
}


static void
unrecord_section_via_map_over_sections (bfd *abfd ATTRIBUTE_UNUSED,
					asection *sec,
					void *ignore ATTRIBUTE_UNUSED)
{
  unrecord_section_with_aarch64_elf_section_data (sec);
}

static bfd_boolean
elfNN_aarch64_close_and_cleanup (bfd *abfd)
{
  if (abfd->sections)
    bfd_map_over_sections (abfd,
			   unrecord_section_via_map_over_sections, NULL);

  return _bfd_elf_close_and_cleanup (abfd);
}

static bfd_boolean
elfNN_aarch64_bfd_free_cached_info (bfd *abfd)
{
  if (abfd->sections)
    bfd_map_over_sections (abfd,
			   unrecord_section_via_map_over_sections, NULL);

  return _bfd_free_cached_info (abfd);
}

/* Create dynamic sections. This is different from the ARM backend in that
   the got, plt, gotplt and their relocation sections are all created in the
   standard part of the bfd elf backend.  */

static bfd_boolean
elfNN_aarch64_create_dynamic_sections (bfd *dynobj,
				       struct bfd_link_info *info)
{
  struct elf_aarch64_link_hash_table *htab;

  /* We need to create .got section.  */
  if (!aarch64_elf_create_got_section (dynobj, info))
    return FALSE;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return FALSE;

  htab = elf_aarch64_hash_table (info);
  htab->sdynbss = bfd_get_linker_section (dynobj, ".dynbss");
  if (!info->shared)
    htab->srelbss = bfd_get_linker_section (dynobj, ".rela.bss");

  if (!htab->sdynbss || (!info->shared && !htab->srelbss))
    abort ();

  return TRUE;
}


/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bfd_boolean
elfNN_aarch64_allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf_aarch64_link_hash_table *htab;
  struct elf_aarch64_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  /* An example of a bfd_link_hash_indirect symbol is versioned
     symbol. For example: __gxx_personality_v0(bfd_link_hash_indirect)
     -> __gxx_personality_v0(bfd_link_hash_defined)

     There is no need to process bfd_link_hash_indirect symbols here
     because we will also be presented with the concrete instance of
     the symbol and elfNN_aarch64_copy_indirect_symbol () will have been
     called to copy all relevant data from the generic to the concrete
     symbol instance.
   */
  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;
  htab = elf_aarch64_hash_table (info);

  /* Since STT_GNU_IFUNC symbol must go through PLT, we handle it
     here if it is defined and referenced in a non-shared object.  */
  if (h->type == STT_GNU_IFUNC
      && h->def_regular)
    return TRUE;
  else if (htab->root.dynamic_sections_created && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (info->shared || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  asection *s = htab->root.splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += htab->plt_header_size;

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (!info->shared && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry. For now we only create the
	     small model PLT entries. We later need to find a way
	     of relaxing into these from the large model PLT entries.  */
	  s->size += PLT_SMALL_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->root.sgotplt->size += GOT_ENTRY_SIZE;

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->root.srelplt->size += RELOC_SIZE (htab);

	  /* We need to ensure that all GOT entries that serve the PLT
	     are consecutive with the special GOT slots [0] [1] and
	     [2]. Any addtional relocations, such as
	     R_AARCH64_TLSDESC, must be placed after the PLT related
	     entries.  We abuse the reloc_count such that during
	     sizing we adjust reloc_count to indicate the number of
	     PLT related reserved entries.  In subsequent phases when
	     filling in the contents of the reloc entries, PLT related
	     entries are placed by computing their PLT index (0
	     .. reloc_count). While other none PLT relocs are placed
	     at the slot indicated by reloc_count and reloc_count is
	     updated.  */

	  htab->root.srelplt->reloc_count++;
	}
      else
	{
	  h->plt.offset = (bfd_vma) - 1;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) - 1;
      h->needs_plt = 0;
    }

  eh = (struct elf_aarch64_link_hash_entry *) h;
  eh->tlsdesc_got_jump_table_offset = (bfd_vma) - 1;

  if (h->got.refcount > 0)
    {
      bfd_boolean dyn;
      unsigned got_type = elf_aarch64_hash_entry (h)->got_type;

      h->got.offset = (bfd_vma) - 1;

      dyn = htab->root.dynamic_sections_created;

      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (dyn && h->dynindx == -1 && !h->forced_local)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (got_type == GOT_UNKNOWN)
	{
	}
      else if (got_type == GOT_NORMAL)
	{
	  h->got.offset = htab->root.sgot->size;
	  htab->root.sgot->size += GOT_ENTRY_SIZE;
	  if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	       || h->root.type != bfd_link_hash_undefweak)
	      && (info->shared
		  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	    {
	      htab->root.srelgot->size += RELOC_SIZE (htab);
	    }
	}
      else
	{
	  int indx;
	  if (got_type & GOT_TLSDESC_GD)
	    {
	      eh->tlsdesc_got_jump_table_offset =
		(htab->root.sgotplt->size
		 - aarch64_compute_jump_table_size (htab));
	      htab->root.sgotplt->size += GOT_ENTRY_SIZE * 2;
	      h->got.offset = (bfd_vma) - 2;
	    }

	  if (got_type & GOT_TLS_GD)
	    {
	      h->got.offset = htab->root.sgot->size;
	      htab->root.sgot->size += GOT_ENTRY_SIZE * 2;
	    }

	  if (got_type & GOT_TLS_IE)
	    {
	      h->got.offset = htab->root.sgot->size;
	      htab->root.sgot->size += GOT_ENTRY_SIZE;
	    }

	  indx = h && h->dynindx != -1 ? h->dynindx : 0;
	  if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	       || h->root.type != bfd_link_hash_undefweak)
	      && (info->shared
		  || indx != 0
		  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	    {
	      if (got_type & GOT_TLSDESC_GD)
		{
		  htab->root.srelplt->size += RELOC_SIZE (htab);
		  /* Note reloc_count not incremented here!  We have
		     already adjusted reloc_count for this relocation
		     type.  */

		  /* TLSDESC PLT is now needed, but not yet determined.  */
		  htab->tlsdesc_plt = (bfd_vma) - 1;
		}

	      if (got_type & GOT_TLS_GD)
		htab->root.srelgot->size += RELOC_SIZE (htab) * 2;

	      if (got_type & GOT_TLS_IE)
		htab->root.srelgot->size += RELOC_SIZE (htab);
	    }
	}
    }
  else
    {
      h->got.offset = (bfd_vma) - 1;
    }

  if (eh->dyn_relocs == NULL)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (info->shared)
    {
      /* Relocs that use pc_count are those that appear on a call
         insn, or certain REL relocs that can generated via assembly.
         We want calls to protected symbols to resolve directly to the
         function rather than going via the plt.  If people want
         function pointer comparisons to work as expected then they
         should avoid writing weird assembly.  */
      if (SYMBOL_CALLS_LOCAL (info, h))
	{
	  struct elf_dyn_relocs **pp;

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
      if (eh->dyn_relocs != NULL && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	    eh->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && !h->forced_local
		   && !bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

    }
  else if (ELIMINATE_COPY_RELOCS)
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
	      && !h->forced_local
	      && !bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      eh->dyn_relocs = NULL;

    keep:;
    }

  /* Finally, allocate space.  */
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc;

      sreloc = elf_section_data (p->sec)->sreloc;

      BFD_ASSERT (sreloc != NULL);

      sreloc->size += p->count * RELOC_SIZE (htab);
    }

  return TRUE;
}

/* Allocate space in .plt, .got and associated reloc sections for
   ifunc dynamic relocs.  */

static bfd_boolean
elfNN_aarch64_allocate_ifunc_dynrelocs (struct elf_link_hash_entry *h,
					void *inf)
{
  struct bfd_link_info *info;
  struct elf_aarch64_link_hash_table *htab;
  struct elf_aarch64_link_hash_entry *eh;

  /* An example of a bfd_link_hash_indirect symbol is versioned
     symbol. For example: __gxx_personality_v0(bfd_link_hash_indirect)
     -> __gxx_personality_v0(bfd_link_hash_defined)

     There is no need to process bfd_link_hash_indirect symbols here
     because we will also be presented with the concrete instance of
     the symbol and elfNN_aarch64_copy_indirect_symbol () will have been
     called to copy all relevant data from the generic to the concrete
     symbol instance.
   */
  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;
  htab = elf_aarch64_hash_table (info);

  eh = (struct elf_aarch64_link_hash_entry *) h;

  /* Since STT_GNU_IFUNC symbol must go through PLT, we handle it
     here if it is defined and referenced in a non-shared object.  */
  if (h->type == STT_GNU_IFUNC
      && h->def_regular)
    return _bfd_elf_allocate_ifunc_dyn_relocs (info, h,
					       &eh->dyn_relocs,
					       htab->plt_entry_size,
					       htab->plt_header_size,
					       GOT_ENTRY_SIZE);
  return TRUE;
}

/* Allocate space in .plt, .got and associated reloc sections for
   local dynamic relocs.  */

static bfd_boolean
elfNN_aarch64_allocate_local_dynrelocs (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;

  if (h->type != STT_GNU_IFUNC
      || !h->def_regular
      || !h->ref_regular
      || !h->forced_local
      || h->root.type != bfd_link_hash_defined)
    abort ();

  return elfNN_aarch64_allocate_dynrelocs (h, inf);
}

/* Allocate space in .plt, .got and associated reloc sections for
   local ifunc dynamic relocs.  */

static bfd_boolean
elfNN_aarch64_allocate_local_ifunc_dynrelocs (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;

  if (h->type != STT_GNU_IFUNC
      || !h->def_regular
      || !h->ref_regular
      || !h->forced_local
      || h->root.type != bfd_link_hash_defined)
    abort ();

  return elfNN_aarch64_allocate_ifunc_dynrelocs (h, inf);
}

/* Find any dynamic relocs that apply to read-only sections.  */

static bfd_boolean
aarch64_readonly_dynrelocs (struct elf_link_hash_entry * h, void * inf)
{
  struct elf_aarch64_link_hash_entry * eh;
  struct elf_dyn_relocs * p;

  eh = (struct elf_aarch64_link_hash_entry *) h;
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *s = p->sec;

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

/* This is the most important function of all . Innocuosly named
   though !  */
static bfd_boolean
elfNN_aarch64_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				     struct bfd_link_info *info)
{
  struct elf_aarch64_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd_boolean relocs;
  bfd *ibfd;

  htab = elf_aarch64_hash_table ((info));
  dynobj = htab->root.dynobj;

  BFD_ASSERT (dynobj != NULL);

  if (htab->root.dynamic_sections_created)
    {
      if (info->executable)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  if (s == NULL)
	    abort ();
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct elf_aarch64_local_symbol *locals = NULL;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;
      unsigned int i;

      if (!is_aarch64_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = (struct elf_dyn_relocs *)
	       (elf_section_data (s)->local_dynrel); p != NULL; p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
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
		  srel->size += p->count * RELOC_SIZE (htab);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      locals = elf_aarch64_locals (ibfd);
      if (!locals)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      srel = htab->root.srelgot;
      for (i = 0; i < symtab_hdr->sh_info; i++)
	{
	  locals[i].got_offset = (bfd_vma) - 1;
	  locals[i].tlsdesc_got_jump_table_offset = (bfd_vma) - 1;
	  if (locals[i].got_refcount > 0)
	    {
	      unsigned got_type = locals[i].got_type;
	      if (got_type & GOT_TLSDESC_GD)
		{
		  locals[i].tlsdesc_got_jump_table_offset =
		    (htab->root.sgotplt->size
		     - aarch64_compute_jump_table_size (htab));
		  htab->root.sgotplt->size += GOT_ENTRY_SIZE * 2;
		  locals[i].got_offset = (bfd_vma) - 2;
		}

	      if (got_type & GOT_TLS_GD)
		{
		  locals[i].got_offset = htab->root.sgot->size;
		  htab->root.sgot->size += GOT_ENTRY_SIZE * 2;
		}

	      if (got_type & GOT_TLS_IE
		  || got_type & GOT_NORMAL)
		{
		  locals[i].got_offset = htab->root.sgot->size;
		  htab->root.sgot->size += GOT_ENTRY_SIZE;
		}

	      if (got_type == GOT_UNKNOWN)
		{
		}

	      if (info->shared)
		{
		  if (got_type & GOT_TLSDESC_GD)
		    {
		      htab->root.srelplt->size += RELOC_SIZE (htab);
		      /* Note RELOC_COUNT not incremented here! */
		      htab->tlsdesc_plt = (bfd_vma) - 1;
		    }

		  if (got_type & GOT_TLS_GD)
		    htab->root.srelgot->size += RELOC_SIZE (htab) * 2;

		  if (got_type & GOT_TLS_IE
		      || got_type & GOT_NORMAL)
		    htab->root.srelgot->size += RELOC_SIZE (htab);
		}
	    }
	  else
	    {
	      locals[i].got_refcount = (bfd_vma) - 1;
	    }
	}
    }


  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, elfNN_aarch64_allocate_dynrelocs,
			  info);

  /* Allocate global ifunc sym .plt and .got entries, and space for global
     ifunc sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, elfNN_aarch64_allocate_ifunc_dynrelocs,
			  info);

  /* Allocate .plt and .got entries, and space for local symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elfNN_aarch64_allocate_local_dynrelocs,
		 info);

  /* Allocate .plt and .got entries, and space for local ifunc symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elfNN_aarch64_allocate_local_ifunc_dynrelocs,
		 info);

  /* For every jump slot reserved in the sgotplt, reloc_count is
     incremented.  However, when we reserve space for TLS descriptors,
     it's not incremented, so in order to compute the space reserved
     for them, it suffices to multiply the reloc count by the jump
     slot size.  */

  if (htab->root.srelplt)
    htab->sgotplt_jump_table_size = aarch64_compute_jump_table_size (htab);

  if (htab->tlsdesc_plt)
    {
      if (htab->root.splt->size == 0)
	htab->root.splt->size += PLT_ENTRY_SIZE;

      htab->tlsdesc_plt = htab->root.splt->size;
      htab->root.splt->size += PLT_TLSDESC_ENTRY_SIZE;

      /* If we're not using lazy TLS relocations, don't generate the
         GOT entry required.  */
      if (!(info->flags & DF_BIND_NOW))
	{
	  htab->dt_tlsdesc_got = htab->root.sgot->size;
	  htab->root.sgot->size += GOT_ENTRY_SIZE;
	}
    }

  /* Init mapping symbols information to use later to distingush between
     code and data while scanning for erratam 835769.  */
  if (htab->fix_erratum_835769)
    for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
      {
	if (!is_aarch64_elf (ibfd))
	  continue;
	bfd_elfNN_aarch64_init_maps (ibfd);
      }

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = FALSE;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->root.splt
	  || s == htab->root.sgot
	  || s == htab->root.sgotplt
	  || s == htab->root.iplt
	  || s == htab->root.igotplt || s == htab->sdynbss)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (CONST_STRNEQ (bfd_get_section_name (dynobj, s), ".rela"))
	{
	  if (s->size != 0 && s != htab->root.srelplt)
	    relocs = TRUE;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  if (s != htab->root.srelplt)
	    s->reloc_count = 0;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

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
         but this way if it does, we get a R_AARCH64_NONE reloc instead
         of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return FALSE;
    }

  if (htab->root.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
         values later, in elfNN_aarch64_finish_dynamic_sections, but we
         must add the entries now so that we get the correct size for
         the .dynamic section.  The DT_DEBUG entry is filled in by the
         dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL)			\
      _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (info->executable)
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return FALSE;
	}

      if (htab->root.splt->size != 0)
	{
	  if (!add_dynamic_entry (DT_PLTGOT, 0)
	      || !add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return FALSE;

	  if (htab->tlsdesc_plt
	      && (!add_dynamic_entry (DT_TLSDESC_PLT, 0)
		  || !add_dynamic_entry (DT_TLSDESC_GOT, 0)))
	    return FALSE;
	}

      if (relocs)
	{
	  if (!add_dynamic_entry (DT_RELA, 0)
	      || !add_dynamic_entry (DT_RELASZ, 0)
	      || !add_dynamic_entry (DT_RELAENT, RELOC_SIZE (htab)))
	    return FALSE;

	  /* If any dynamic relocs apply to a read-only section,
	     then we need a DT_TEXTREL entry.  */
	  if ((info->flags & DF_TEXTREL) == 0)
	    elf_link_hash_traverse (& htab->root, aarch64_readonly_dynrelocs,
				    info);

	  if ((info->flags & DF_TEXTREL) != 0)
	    {
	      if (!add_dynamic_entry (DT_TEXTREL, 0))
		return FALSE;
	    }
	}
    }
#undef add_dynamic_entry

  return TRUE;
}

static inline void
elf_aarch64_update_plt_entry (bfd *output_bfd,
			      bfd_reloc_code_real_type r_type,
			      bfd_byte *plt_entry, bfd_vma value)
{
  reloc_howto_type *howto = elfNN_aarch64_howto_from_bfd_reloc (r_type);

  _bfd_aarch64_elf_put_addend (output_bfd, plt_entry, r_type, howto, value);
}

static void
elfNN_aarch64_create_small_pltn_entry (struct elf_link_hash_entry *h,
				       struct elf_aarch64_link_hash_table
				       *htab, bfd *output_bfd,
				       struct bfd_link_info *info)
{
  bfd_byte *plt_entry;
  bfd_vma plt_index;
  bfd_vma got_offset;
  bfd_vma gotplt_entry_address;
  bfd_vma plt_entry_address;
  Elf_Internal_Rela rela;
  bfd_byte *loc;
  asection *plt, *gotplt, *relplt;

  /* When building a static executable, use .iplt, .igot.plt and
     .rela.iplt sections for STT_GNU_IFUNC symbols.  */
  if (htab->root.splt != NULL)
    {
      plt = htab->root.splt;
      gotplt = htab->root.sgotplt;
      relplt = htab->root.srelplt;
    }
  else
    {
      plt = htab->root.iplt;
      gotplt = htab->root.igotplt;
      relplt = htab->root.irelplt;
    }

  /* Get the index in the procedure linkage table which
     corresponds to this symbol.  This is the index of this symbol
     in all the symbols for which we are making plt entries.  The
     first entry in the procedure linkage table is reserved.

     Get the offset into the .got table of the entry that
     corresponds to this function.	Each .got entry is GOT_ENTRY_SIZE
     bytes. The first three are reserved for the dynamic linker.

     For static executables, we don't reserve anything.  */

  if (plt == htab->root.splt)
    {
      plt_index = (h->plt.offset - htab->plt_header_size) / htab->plt_entry_size;
      got_offset = (plt_index + 3) * GOT_ENTRY_SIZE;
    }
  else
    {
      plt_index = h->plt.offset / htab->plt_entry_size;
      got_offset = plt_index * GOT_ENTRY_SIZE;
    }

  plt_entry = plt->contents + h->plt.offset;
  plt_entry_address = plt->output_section->vma
    + plt->output_offset + h->plt.offset;
  gotplt_entry_address = gotplt->output_section->vma +
    gotplt->output_offset + got_offset;

  /* Copy in the boiler-plate for the PLTn entry.  */
  memcpy (plt_entry, elfNN_aarch64_small_plt_entry, PLT_SMALL_ENTRY_SIZE);

  /* Fill in the top 21 bits for this: ADRP x16, PLT_GOT + n * 8.
     ADRP:   ((PG(S+A)-PG(P)) >> 12) & 0x1fffff */
  elf_aarch64_update_plt_entry (output_bfd, BFD_RELOC_AARCH64_ADR_HI21_PCREL,
				plt_entry,
				PG (gotplt_entry_address) -
				PG (plt_entry_address));

  /* Fill in the lo12 bits for the load from the pltgot.  */
  elf_aarch64_update_plt_entry (output_bfd, BFD_RELOC_AARCH64_LDSTNN_LO12,
				plt_entry + 4,
				PG_OFFSET (gotplt_entry_address));

  /* Fill in the lo12 bits for the add from the pltgot entry.  */
  elf_aarch64_update_plt_entry (output_bfd, BFD_RELOC_AARCH64_ADD_LO12,
				plt_entry + 8,
				PG_OFFSET (gotplt_entry_address));

  /* All the GOTPLT Entries are essentially initialized to PLT0.  */
  bfd_put_NN (output_bfd,
	      plt->output_section->vma + plt->output_offset,
	      gotplt->contents + got_offset);

  rela.r_offset = gotplt_entry_address;

  if (h->dynindx == -1
      || ((info->executable
	   || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	  && h->def_regular
	  && h->type == STT_GNU_IFUNC))
    {
      /* If an STT_GNU_IFUNC symbol is locally defined, generate
	 R_AARCH64_IRELATIVE instead of R_AARCH64_JUMP_SLOT.  */
      rela.r_info = ELFNN_R_INFO (0, AARCH64_R (IRELATIVE));
      rela.r_addend = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
    }
  else
    {
      /* Fill in the entry in the .rela.plt section.  */
      rela.r_info = ELFNN_R_INFO (h->dynindx, AARCH64_R (JUMP_SLOT));
      rela.r_addend = 0;
    }

  /* Compute the relocation entry to used based on PLT index and do
     not adjust reloc_count. The reloc_count has already been adjusted
     to account for this entry.  */
  loc = relplt->contents + plt_index * RELOC_SIZE (htab);
  bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);
}

/* Size sections even though they're not dynamic.  We use it to setup
   _TLS_MODULE_BASE_, if needed.  */

static bfd_boolean
elfNN_aarch64_always_size_sections (bfd *output_bfd,
				    struct bfd_link_info *info)
{
  asection *tls_sec;

  if (info->relocatable)
    return TRUE;

  tls_sec = elf_hash_table (info)->tls_sec;

  if (tls_sec)
    {
      struct elf_link_hash_entry *tlsbase;

      tlsbase = elf_link_hash_lookup (elf_hash_table (info),
				      "_TLS_MODULE_BASE_", TRUE, TRUE, FALSE);

      if (tlsbase)
	{
	  struct bfd_link_hash_entry *h = NULL;
	  const struct elf_backend_data *bed =
	    get_elf_backend_data (output_bfd);

	  if (!(_bfd_generic_link_add_one_symbol
		(info, output_bfd, "_TLS_MODULE_BASE_", BSF_LOCAL,
		 tls_sec, 0, NULL, FALSE, bed->collect, &h)))
	    return FALSE;

	  tlsbase->type = STT_TLS;
	  tlsbase = (struct elf_link_hash_entry *) h;
	  tlsbase->def_regular = 1;
	  tlsbase->other = STV_HIDDEN;
	  (*bed->elf_backend_hide_symbol) (info, tlsbase, TRUE);
	}
    }

  return TRUE;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */
static bfd_boolean
elfNN_aarch64_finish_dynamic_symbol (bfd *output_bfd,
				     struct bfd_link_info *info,
				     struct elf_link_hash_entry *h,
				     Elf_Internal_Sym *sym)
{
  struct elf_aarch64_link_hash_table *htab;
  htab = elf_aarch64_hash_table (info);

  if (h->plt.offset != (bfd_vma) - 1)
    {
      asection *plt, *gotplt, *relplt;

      /* This symbol has an entry in the procedure linkage table.  Set
         it up.  */

      /* When building a static executable, use .iplt, .igot.plt and
	 .rela.iplt sections for STT_GNU_IFUNC symbols.  */
      if (htab->root.splt != NULL)
	{
	  plt = htab->root.splt;
	  gotplt = htab->root.sgotplt;
	  relplt = htab->root.srelplt;
	}
      else
	{
	  plt = htab->root.iplt;
	  gotplt = htab->root.igotplt;
	  relplt = htab->root.irelplt;
	}

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.	 */
      if ((h->dynindx == -1
	   && !((h->forced_local || info->executable)
		&& h->def_regular
		&& h->type == STT_GNU_IFUNC))
	  || plt == NULL
	  || gotplt == NULL
	  || relplt == NULL)
	abort ();

      elfNN_aarch64_create_small_pltn_entry (h, htab, output_bfd, info);
      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  This is a clue
	     for the dynamic linker, to make function pointer
	     comparisons work between an application and shared
	     library.  */
	  sym->st_shndx = SHN_UNDEF;
	}
    }

  if (h->got.offset != (bfd_vma) - 1
      && elf_aarch64_hash_entry (h)->got_type == GOT_NORMAL)
    {
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol has an entry in the global offset table.  Set it
         up.  */
      if (htab->root.sgot == NULL || htab->root.srelgot == NULL)
	abort ();

      rela.r_offset = (htab->root.sgot->output_section->vma
		       + htab->root.sgot->output_offset
		       + (h->got.offset & ~(bfd_vma) 1));

      if (h->def_regular
	  && h->type == STT_GNU_IFUNC)
	{
	  if (info->shared)
	    {
	      /* Generate R_AARCH64_GLOB_DAT.  */
	      goto do_glob_dat;
	    }
	  else
	    {
	      asection *plt;

	      if (!h->pointer_equality_needed)
		abort ();

	      /* For non-shared object, we can't use .got.plt, which
		 contains the real function address if we need pointer
		 equality.  We load the GOT entry with the PLT entry.  */
	      plt = htab->root.splt ? htab->root.splt : htab->root.iplt;
	      bfd_put_NN (output_bfd, (plt->output_section->vma
				       + plt->output_offset
				       + h->plt.offset),
			  htab->root.sgot->contents
			  + (h->got.offset & ~(bfd_vma) 1));
	      return TRUE;
	    }
	}
      else if (info->shared && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  if (!h->def_regular)
	    return FALSE;

	  BFD_ASSERT ((h->got.offset & 1) != 0);
	  rela.r_info = ELFNN_R_INFO (0, AARCH64_R (RELATIVE));
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);
	}
      else
	{
do_glob_dat:
	  BFD_ASSERT ((h->got.offset & 1) == 0);
	  bfd_put_NN (output_bfd, (bfd_vma) 0,
		      htab->root.sgot->contents + h->got.offset);
	  rela.r_info = ELFNN_R_INFO (h->dynindx, AARCH64_R (GLOB_DAT));
	  rela.r_addend = 0;
	}

      loc = htab->root.srelgot->contents;
      loc += htab->root.srelgot->reloc_count++ * RELOC_SIZE (htab);
      bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);
    }

  if (h->needs_copy)
    {
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol needs a copy reloc.  Set it up.  */

      if (h->dynindx == -1
	  || (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	  || htab->srelbss == NULL)
	abort ();

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELFNN_R_INFO (h->dynindx, AARCH64_R (COPY));
      rela.r_addend = 0;
      loc = htab->srelbss->contents;
      loc += htab->srelbss->reloc_count++ * RELOC_SIZE (htab);
      bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  SYM may
     be NULL for local symbols.  */
  if (sym != NULL
      && (h == elf_hash_table (info)->hdynamic
	  || h == elf_hash_table (info)->hgot))
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

/* Finish up local dynamic symbol handling.  We set the contents of
   various dynamic sections here.  */

static bfd_boolean
elfNN_aarch64_finish_local_dynamic_symbol (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;
  struct bfd_link_info *info
    = (struct bfd_link_info *) inf;

  return elfNN_aarch64_finish_dynamic_symbol (info->output_bfd,
					      info, h, NULL);
}

static void
elfNN_aarch64_init_small_plt0_entry (bfd *output_bfd ATTRIBUTE_UNUSED,
				     struct elf_aarch64_link_hash_table
				     *htab)
{
  /* Fill in PLT0. Fixme:RR Note this doesn't distinguish between
     small and large plts and at the minute just generates
     the small PLT.  */

  /* PLT0 of the small PLT looks like this in ELF64 -
     stp x16, x30, [sp, #-16]!		// Save the reloc and lr on stack.
     adrp x16, PLT_GOT + 16		// Get the page base of the GOTPLT
     ldr  x17, [x16, #:lo12:PLT_GOT+16] // Load the address of the
					// symbol resolver
     add  x16, x16, #:lo12:PLT_GOT+16   // Load the lo12 bits of the
					// GOTPLT entry for this.
     br   x17
     PLT0 will be slightly different in ELF32 due to different got entry
     size.
   */
  bfd_vma plt_got_2nd_ent;	/* Address of GOT[2].  */
  bfd_vma plt_base;


  memcpy (htab->root.splt->contents, elfNN_aarch64_small_plt0_entry,
	  PLT_ENTRY_SIZE);
  elf_section_data (htab->root.splt->output_section)->this_hdr.sh_entsize =
    PLT_ENTRY_SIZE;

  plt_got_2nd_ent = (htab->root.sgotplt->output_section->vma
		  + htab->root.sgotplt->output_offset
		  + GOT_ENTRY_SIZE * 2);

  plt_base = htab->root.splt->output_section->vma +
    htab->root.splt->output_offset;

  /* Fill in the top 21 bits for this: ADRP x16, PLT_GOT + n * 8.
     ADRP:   ((PG(S+A)-PG(P)) >> 12) & 0x1fffff */
  elf_aarch64_update_plt_entry (output_bfd, BFD_RELOC_AARCH64_ADR_HI21_PCREL,
				htab->root.splt->contents + 4,
				PG (plt_got_2nd_ent) - PG (plt_base + 4));

  elf_aarch64_update_plt_entry (output_bfd, BFD_RELOC_AARCH64_LDSTNN_LO12,
				htab->root.splt->contents + 8,
				PG_OFFSET (plt_got_2nd_ent));

  elf_aarch64_update_plt_entry (output_bfd, BFD_RELOC_AARCH64_ADD_LO12,
				htab->root.splt->contents + 12,
				PG_OFFSET (plt_got_2nd_ent));
}

static bfd_boolean
elfNN_aarch64_finish_dynamic_sections (bfd *output_bfd,
				       struct bfd_link_info *info)
{
  struct elf_aarch64_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;

  htab = elf_aarch64_hash_table (info);
  dynobj = htab->root.dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->root.dynamic_sections_created)
    {
      ElfNN_External_Dyn *dyncon, *dynconend;

      if (sdyn == NULL || htab->root.sgot == NULL)
	abort ();

      dyncon = (ElfNN_External_Dyn *) sdyn->contents;
      dynconend = (ElfNN_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elfNN_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;

	    case DT_PLTGOT:
	      s = htab->root.sgotplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_JMPREL:
	      dyn.d_un.d_ptr = htab->root.srelplt->output_section->vma;
	      break;

	    case DT_PLTRELSZ:
	      s = htab->root.srelplt;
	      dyn.d_un.d_val = s->size;
	      break;

	    case DT_RELASZ:
	      /* The procedure linkage table relocs (DT_JMPREL) should
		 not be included in the overall relocs (DT_RELA).
		 Therefore, we override the DT_RELASZ entry here to
		 make it not include the JMPREL relocs.  Since the
		 linker script arranges for .rela.plt to follow all
		 other relocation sections, we don't have to worry
		 about changing the DT_RELA entry.  */
	      if (htab->root.srelplt != NULL)
		{
		  s = htab->root.srelplt;
		  dyn.d_un.d_val -= s->size;
		}
	      break;

	    case DT_TLSDESC_PLT:
	      s = htab->root.splt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
		+ htab->tlsdesc_plt;
	      break;

	    case DT_TLSDESC_GOT:
	      s = htab->root.sgot;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
		+ htab->dt_tlsdesc_got;
	      break;
	    }

	  bfd_elfNN_swap_dyn_out (output_bfd, &dyn, dyncon);
	}

    }

  /* Fill in the special first entry in the procedure linkage table.  */
  if (htab->root.splt && htab->root.splt->size > 0)
    {
      elfNN_aarch64_init_small_plt0_entry (output_bfd, htab);

      elf_section_data (htab->root.splt->output_section)->
	this_hdr.sh_entsize = htab->plt_entry_size;


      if (htab->tlsdesc_plt)
	{
	  bfd_put_NN (output_bfd, (bfd_vma) 0,
		      htab->root.sgot->contents + htab->dt_tlsdesc_got);

	  memcpy (htab->root.splt->contents + htab->tlsdesc_plt,
		  elfNN_aarch64_tlsdesc_small_plt_entry,
		  sizeof (elfNN_aarch64_tlsdesc_small_plt_entry));

	  {
	    bfd_vma adrp1_addr =
	      htab->root.splt->output_section->vma
	      + htab->root.splt->output_offset + htab->tlsdesc_plt + 4;

	    bfd_vma adrp2_addr = adrp1_addr + 4;

	    bfd_vma got_addr =
	      htab->root.sgot->output_section->vma
	      + htab->root.sgot->output_offset;

	    bfd_vma pltgot_addr =
	      htab->root.sgotplt->output_section->vma
	      + htab->root.sgotplt->output_offset;

	    bfd_vma dt_tlsdesc_got = got_addr + htab->dt_tlsdesc_got;

	    bfd_byte *plt_entry =
	      htab->root.splt->contents + htab->tlsdesc_plt;

	    /* adrp x2, DT_TLSDESC_GOT */
	    elf_aarch64_update_plt_entry (output_bfd,
					  BFD_RELOC_AARCH64_ADR_HI21_PCREL,
					  plt_entry + 4,
					  (PG (dt_tlsdesc_got)
					   - PG (adrp1_addr)));

	    /* adrp x3, 0 */
	    elf_aarch64_update_plt_entry (output_bfd,
					  BFD_RELOC_AARCH64_ADR_HI21_PCREL,
					  plt_entry + 8,
					  (PG (pltgot_addr)
					   - PG (adrp2_addr)));

	    /* ldr x2, [x2, #0] */
	    elf_aarch64_update_plt_entry (output_bfd,
					  BFD_RELOC_AARCH64_LDSTNN_LO12,
					  plt_entry + 12,
					  PG_OFFSET (dt_tlsdesc_got));

	    /* add x3, x3, 0 */
	    elf_aarch64_update_plt_entry (output_bfd,
					  BFD_RELOC_AARCH64_ADD_LO12,
					  plt_entry + 16,
					  PG_OFFSET (pltgot_addr));
	  }
	}
    }

  if (htab->root.sgotplt)
    {
      if (bfd_is_abs_section (htab->root.sgotplt->output_section))
	{
	  (*_bfd_error_handler)
	    (_("discarded output section: `%A'"), htab->root.sgotplt);
	  return FALSE;
	}

      /* Fill in the first three entries in the global offset table.  */
      if (htab->root.sgotplt->size > 0)
	{
	  bfd_put_NN (output_bfd, (bfd_vma) 0, htab->root.sgotplt->contents);

	  /* Write GOT[1] and GOT[2], needed for the dynamic linker.  */
	  bfd_put_NN (output_bfd,
		      (bfd_vma) 0,
		      htab->root.sgotplt->contents + GOT_ENTRY_SIZE);
	  bfd_put_NN (output_bfd,
		      (bfd_vma) 0,
		      htab->root.sgotplt->contents + GOT_ENTRY_SIZE * 2);
	}

      if (htab->root.sgot)
	{
	  if (htab->root.sgot->size > 0)
	    {
	      bfd_vma addr =
		sdyn ? sdyn->output_section->vma + sdyn->output_offset : 0;
	      bfd_put_NN (output_bfd, addr, htab->root.sgot->contents);
	    }
	}

      elf_section_data (htab->root.sgotplt->output_section)->
	this_hdr.sh_entsize = GOT_ENTRY_SIZE;
    }

  if (htab->root.sgot && htab->root.sgot->size > 0)
    elf_section_data (htab->root.sgot->output_section)->this_hdr.sh_entsize
      = GOT_ENTRY_SIZE;

  /* Fill PLT and GOT entries for local STT_GNU_IFUNC symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elfNN_aarch64_finish_local_dynamic_symbol,
		 info);

  return TRUE;
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
elfNN_aarch64_plt_sym_val (bfd_vma i, const asection *plt,
			   const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + PLT_ENTRY_SIZE + i * PLT_SMALL_ENTRY_SIZE;
}


/* We use this so we can override certain functions
   (though currently we don't).  */

const struct elf_size_info elfNN_aarch64_size_info =
{
  sizeof (ElfNN_External_Ehdr),
  sizeof (ElfNN_External_Phdr),
  sizeof (ElfNN_External_Shdr),
  sizeof (ElfNN_External_Rel),
  sizeof (ElfNN_External_Rela),
  sizeof (ElfNN_External_Sym),
  sizeof (ElfNN_External_Dyn),
  sizeof (Elf_External_Note),
  4,				/* Hash table entry size.  */
  1,				/* Internal relocs per external relocs.  */
  ARCH_SIZE,			/* Arch size.  */
  LOG_FILE_ALIGN,		/* Log_file_align.  */
  ELFCLASSNN, EV_CURRENT,
  bfd_elfNN_write_out_phdrs,
  bfd_elfNN_write_shdrs_and_ehdr,
  bfd_elfNN_checksum_contents,
  bfd_elfNN_write_relocs,
  bfd_elfNN_swap_symbol_in,
  bfd_elfNN_swap_symbol_out,
  bfd_elfNN_slurp_reloc_table,
  bfd_elfNN_slurp_symbol_table,
  bfd_elfNN_swap_dyn_in,
  bfd_elfNN_swap_dyn_out,
  bfd_elfNN_swap_reloc_in,
  bfd_elfNN_swap_reloc_out,
  bfd_elfNN_swap_reloca_in,
  bfd_elfNN_swap_reloca_out
};

#define ELF_ARCH			bfd_arch_aarch64
#define ELF_MACHINE_CODE		EM_AARCH64
#define ELF_MAXPAGESIZE			0x10000
#define ELF_MINPAGESIZE			0x1000
#define ELF_COMMONPAGESIZE		0x1000

#define bfd_elfNN_close_and_cleanup             \
  elfNN_aarch64_close_and_cleanup

#define bfd_elfNN_bfd_free_cached_info          \
  elfNN_aarch64_bfd_free_cached_info

#define bfd_elfNN_bfd_is_target_special_symbol	\
  elfNN_aarch64_is_target_special_symbol

#define bfd_elfNN_bfd_link_hash_table_create    \
  elfNN_aarch64_link_hash_table_create

#define bfd_elfNN_bfd_merge_private_bfd_data	\
  elfNN_aarch64_merge_private_bfd_data

#define bfd_elfNN_bfd_print_private_bfd_data	\
  elfNN_aarch64_print_private_bfd_data

#define bfd_elfNN_bfd_reloc_type_lookup		\
  elfNN_aarch64_reloc_type_lookup

#define bfd_elfNN_bfd_reloc_name_lookup		\
  elfNN_aarch64_reloc_name_lookup

#define bfd_elfNN_bfd_set_private_flags		\
  elfNN_aarch64_set_private_flags

#define bfd_elfNN_find_inliner_info		\
  elfNN_aarch64_find_inliner_info

#define bfd_elfNN_find_nearest_line		\
  elfNN_aarch64_find_nearest_line

#define bfd_elfNN_mkobject			\
  elfNN_aarch64_mkobject

#define bfd_elfNN_new_section_hook		\
  elfNN_aarch64_new_section_hook

#define elf_backend_adjust_dynamic_symbol	\
  elfNN_aarch64_adjust_dynamic_symbol

#define elf_backend_always_size_sections	\
  elfNN_aarch64_always_size_sections

#define elf_backend_check_relocs		\
  elfNN_aarch64_check_relocs

#define elf_backend_copy_indirect_symbol	\
  elfNN_aarch64_copy_indirect_symbol

/* Create .dynbss, and .rela.bss sections in DYNOBJ, and set up shortcuts
   to them in our hash.  */
#define elf_backend_create_dynamic_sections	\
  elfNN_aarch64_create_dynamic_sections

#define elf_backend_init_index_section		\
  _bfd_elf_init_2_index_sections

#define elf_backend_finish_dynamic_sections	\
  elfNN_aarch64_finish_dynamic_sections

#define elf_backend_finish_dynamic_symbol	\
  elfNN_aarch64_finish_dynamic_symbol

#define elf_backend_gc_sweep_hook		\
  elfNN_aarch64_gc_sweep_hook

#define elf_backend_object_p			\
  elfNN_aarch64_object_p

#define elf_backend_output_arch_local_syms      \
  elfNN_aarch64_output_arch_local_syms

#define elf_backend_plt_sym_val			\
  elfNN_aarch64_plt_sym_val

#define elf_backend_post_process_headers	\
  elfNN_aarch64_post_process_headers

#define elf_backend_relocate_section		\
  elfNN_aarch64_relocate_section

#define elf_backend_reloc_type_class		\
  elfNN_aarch64_reloc_type_class

#define elf_backend_section_from_shdr		\
  elfNN_aarch64_section_from_shdr

#define elf_backend_size_dynamic_sections	\
  elfNN_aarch64_size_dynamic_sections

#define elf_backend_size_info			\
  elfNN_aarch64_size_info

#define elf_backend_write_section		\
  elfNN_aarch64_write_section

#define elf_backend_can_refcount       1
#define elf_backend_can_gc_sections    1
#define elf_backend_plt_readonly       1
#define elf_backend_want_got_plt       1
#define elf_backend_want_plt_sym       0
#define elf_backend_may_use_rel_p      0
#define elf_backend_may_use_rela_p     1
#define elf_backend_default_use_rela_p 1
#define elf_backend_rela_normal        1
#define elf_backend_got_header_size (GOT_ENTRY_SIZE * 3)
#define elf_backend_default_execstack  0

#undef  elf_backend_obj_attrs_section
#define elf_backend_obj_attrs_section		".ARM.attributes"

#include "elfNN-target.h"

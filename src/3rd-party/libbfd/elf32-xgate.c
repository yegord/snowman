/* Freescale XGATE-specific support for 32-bit ELF
   Copyright (C) 2010-2014 Free Software Foundation, Inc.
   Contributed by Sean Keys(skeys@ipdatasys.com)

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf32-xgate.h"
#include "elf/xgate.h"
#include "opcode/xgate.h"
#include "libiberty.h"

/* Relocation functions.  */
static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *, const char *);
static void
xgate_info_to_howto_rel (bfd *, arelent *, Elf_Internal_Rela *);
static bfd_boolean
xgate_elf_set_mach_from_flags (bfd *);
static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *, struct bfd_hash_table *,
    const char *);
static struct bfd_link_hash_table*
xgate_elf_bfd_link_hash_table_create (bfd *);

/* Use REL instead of RELA to save space */
#define USE_REL	1

static reloc_howto_type elf_xgate_howto_table[] =
{
  /* This reloc does nothing.  */
  HOWTO (R_XGATE_NONE, /* type */
	 0, /* rightshift */
	 3, /* size (0 = byte, 1 = short, 2 = long) */
	 0, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_NONE", /* name */
	 FALSE, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 8 bit absolute relocation.  */
  HOWTO (R_XGATE_8, /* type */
	 0, /* rightshift */
	 0, /* size (0 = byte, 1 = short, 2 = long) */
	 8, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_8", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 8 bit PC-rel relocation.  */
  HOWTO (R_XGATE_PCREL_8, /* type */
	 0, /* rightshift */
	 0, /* size (0 = byte, 1 = short, 2 = long) */
	 8, /* bitsize */
	 TRUE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_8", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 TRUE), /* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_XGATE_16, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont /*bitfield */, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_16", /* name */
	 FALSE, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 32 bit absolute relocation.  This one is never used for the
     code relocation.  It's used by gas for -gstabs generation.  */
  HOWTO (R_XGATE_32, /* type */
	 0, /* rightshift */
	 2, /* size (0 = byte, 1 = short, 2 = long) */
	 32, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_32", /* name */
	 FALSE, /* partial_inplace */
	 0xffffffff, /* src_mask */
	 0xffffffff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 16 bit PC-rel relocation.  */
  HOWTO (R_XGATE_PCREL_16, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 TRUE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_16", /* name */
	 FALSE, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 TRUE), /* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_XGATE_GNU_VTINHERIT, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 0, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL, /* special_function */
	 "R_XGATE_GNU_VTINHERIT", /* name */
	 FALSE, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_XGATE_GNU_VTENTRY, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 0, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_XGATE_GNU_VTENTRY", /* name */
	 FALSE, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 24 bit relocation.  */
  HOWTO (R_XGATE_24, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_LO", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 16-bit low relocation.  */
  HOWTO (R_XGATE_LO16, /* type */
	 8, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_HI", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A page relocation.  */
  HOWTO (R_XGATE_GPAGE, /* type */
	 0, /* rightshift */
	 0, /* size (0 = byte, 1 = short, 2 = long) */
	 8, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 xgate_elf_special_reloc,/* special_function */
	 "R_XGATE_GPAGE", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 9 bit absolute relocation.   */
  HOWTO (R_XGATE_PCREL_9, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 9, /* bitsize */
	 TRUE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_9", /* name */
	 FALSE, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 TRUE), /* pcrel_offset */

  /* A 8 bit absolute relocation (upper address).  */
  HOWTO (R_XGATE_PCREL_10, /* type */
	 8, /* rightshift */
	 0, /* size (0 = byte, 1 = short, 2 = long) */
	 10, /* bitsize */
	 TRUE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_10", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 TRUE), /* pcrel_offset */

  /* A 8 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM8_LO, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_LO", /* name */
	 FALSE, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 16 bit absolute relocation (upper address).  */
  HOWTO (R_XGATE_IMM8_HI, /* type */
	 8, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_HI", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 3 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM3, /* type */
	 8, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM3", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 4 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM4, /* type */
	 8, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM4", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* A 5 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM5, /* type */
	 8, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 16, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM5", /* name */
	 FALSE, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 FALSE), /* pcrel_offset */

  /* Mark beginning of a jump instruction (any form).  */
  HOWTO (R_XGATE_RL_JUMP, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 0, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 xgate_elf_ignore_reloc, /* special_function */
	 "R_XGATE_RL_JUMP", /* name */
	 TRUE, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 TRUE), /* pcrel_offset */

  /* Mark beginning of Gcc relaxation group instruction.  */
  HOWTO (R_XGATE_RL_GROUP, /* type */
	 0, /* rightshift */
	 1, /* size (0 = byte, 1 = short, 2 = long) */
	 0, /* bitsize */
	 FALSE, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 xgate_elf_ignore_reloc, /* special_function */
	 "R_XGATE_RL_GROUP", /* name */
	 TRUE, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 TRUE), /* pcrel_offset */
};

/* Map BFD reloc types to XGATE ELF reloc types.  */

struct xgate_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct xgate_reloc_map xgate_reloc_map[] =
{
  {BFD_RELOC_NONE, R_XGATE_NONE},
  {BFD_RELOC_8, R_XGATE_8},
  {BFD_RELOC_8_PCREL, R_XGATE_PCREL_8},
  {BFD_RELOC_16_PCREL, R_XGATE_PCREL_16},
  {BFD_RELOC_16, R_XGATE_16},
  {BFD_RELOC_32, R_XGATE_32},

  {BFD_RELOC_VTABLE_INHERIT, R_XGATE_GNU_VTINHERIT},
  {BFD_RELOC_VTABLE_ENTRY, R_XGATE_GNU_VTENTRY},

  {BFD_RELOC_XGATE_LO16, R_XGATE_LO16},
  {BFD_RELOC_XGATE_GPAGE, R_XGATE_GPAGE},
  {BFD_RELOC_XGATE_24, R_XGATE_24},
  {BFD_RELOC_XGATE_PCREL_9, R_XGATE_PCREL_9},
  {BFD_RELOC_XGATE_PCREL_10,  R_XGATE_PCREL_10},
  {BFD_RELOC_XGATE_IMM8_LO, R_XGATE_IMM8_LO},
  {BFD_RELOC_XGATE_IMM8_HI, R_XGATE_IMM8_HI},
  {BFD_RELOC_XGATE_IMM3, R_XGATE_IMM3},
  {BFD_RELOC_XGATE_IMM4, R_XGATE_IMM4},
  {BFD_RELOC_XGATE_IMM5, R_XGATE_IMM5},

  {BFD_RELOC_XGATE_RL_JUMP, R_XGATE_RL_JUMP},
  {BFD_RELOC_XGATE_RL_GROUP, R_XGATE_RL_GROUP},
};

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (xgate_reloc_map); i++)
    if (xgate_reloc_map[i].bfd_reloc_val == code)
      return &elf_xgate_howto_table[xgate_reloc_map[i].elf_reloc_val];

  return NULL;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf_xgate_howto_table); i++)
    if (elf_xgate_howto_table[i].name != NULL
        && strcasecmp (elf_xgate_howto_table[i].name, r_name) == 0)
      return &elf_xgate_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an XGATE ELF reloc.  */

static void
xgate_info_to_howto_rel (bfd *abfd ATTRIBUTE_UNUSED,
			 arelent *cache_ptr,
			 Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_XGATE_max)
    {
      _bfd_error_handler (_("%B: invalid XGate reloc number: %d"), abfd, r_type);
      r_type = 0;
    }
  cache_ptr->howto = &elf_xgate_howto_table[r_type];
}

/* Destroy an XGATE ELF linker hash table.  */

static void
xgate_elf_bfd_link_hash_table_free (bfd *obfd)
{
  struct xgate_elf_link_hash_table *ret =
      (struct xgate_elf_link_hash_table *) obfd->link.hash;

  bfd_hash_table_free (ret->stub_hash_table);
  free (ret->stub_hash_table);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create an XGATE ELF linker hash table.  */

static struct bfd_link_hash_table*
xgate_elf_bfd_link_hash_table_create (bfd *abfd)
{
  struct xgate_elf_link_hash_table *ret;
  bfd_size_type amt = sizeof(struct xgate_elf_link_hash_table);

  ret = (struct xgate_elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == (struct xgate_elf_link_hash_table *) NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
      _bfd_elf_link_hash_newfunc, sizeof(struct elf_link_hash_entry),
      XGATE_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  /* Init the stub hash table too.  */
  amt = sizeof(struct bfd_hash_table);
  ret->stub_hash_table = (struct bfd_hash_table*) bfd_zmalloc (amt);
  if (ret->stub_hash_table == NULL)
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }

  if (!bfd_hash_table_init (ret->stub_hash_table, stub_hash_newfunc,
      sizeof(struct elf32_xgate_stub_hash_entry)))
    {
      free (ret->stub_hash_table);
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.root.hash_table_free = xgate_elf_bfd_link_hash_table_free;

  return &ret->root.root;
}

static bfd_boolean
xgate_elf_set_mach_from_flags (bfd *abfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Specific sections:
 - The .page0 is a data section that is mapped in [0x0000..0x00FF].
   Page0 accesses are faster on the M68HC12.
 - The .vectors is the section that represents the interrupt
   vectors.
 - The .xgate section is starts in 0xE08800 or as xgate sees it 0x0800. */
static const struct bfd_elf_special_section elf32_xgate_special_sections[] =
{
  { STRING_COMMA_LEN (".eeprom"), 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".page0"), 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".softregs"), 0, SHT_NOBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".vectors"), 0, SHT_PROGBITS, SHF_ALLOC },
/*{ STRING_COMMA_LEN (".xgate"),    0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  TODO finish this implementation */
  { NULL, 0, 0, 0, 0 }
};

struct xgate_scan_param
{
  struct xgate_page_info* pinfo;
  bfd_boolean use_memory_banks;
};

/* Assorted hash table functions.  */

/* Initialize an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table ATTRIBUTE_UNUSED,
		   const char *string ATTRIBUTE_UNUSED)
{
  return entry;
}

/* Hook called by the linker routine which adds symbols from an object
   file. */

bfd_boolean
elf32_xgate_add_symbol_hook (bfd *abfd ATTRIBUTE_UNUSED,
			     struct bfd_link_info *info ATTRIBUTE_UNUSED,
			     Elf_Internal_Sym *sym,
			     const char **namep ATTRIBUTE_UNUSED,
			     flagword *flagsp ATTRIBUTE_UNUSED,
			     asection **secp ATTRIBUTE_UNUSED,
			     bfd_vma *valp ATTRIBUTE_UNUSED)
{
  /* For some reason the st_target_internal value is not retained
     after xgate_frob_symbol is called, hence this temp hack.  */
  sym->st_target_internal = 1;
  return TRUE;
}

/* External entry points for sizing and building linker stubs.  */

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf32_xgate_setup_section_lists (bfd *output_bfd ATTRIBUTE_UNUSED,
				 struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 1;
}

/* Determine and set the size of the stub section for a final link.
   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with any "9-bit PC-REL"
   instruction.  */

bfd_boolean
elf32_xgate_size_stubs (bfd *output_bfd ATTRIBUTE_UNUSED,
			bfd *stub_bfd ATTRIBUTE_UNUSED,
			struct bfd_link_info *info ATTRIBUTE_UNUSED,
			asection * (*add_stub_section) (const char*, asection*) ATTRIBUTE_UNUSED)
{
  return FALSE;
}

/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  This function is called via xgateelf_finish in the
   linker.  */

bfd_boolean
elf32_xgate_build_stubs (bfd *abfd ATTRIBUTE_UNUSED,
			 struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

void
xgate_elf_get_bank_parameters (struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return;
}

/* This function is used for relocs which are only used for relaxing,
   which the linker should otherwise ignore.  */

bfd_reloc_status_type
xgate_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			arelent *reloc_entry,
			asymbol *symbol ATTRIBUTE_UNUSED,
			void *data ATTRIBUTE_UNUSED,
			asection *input_section,
			bfd *output_bfd,
			char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

bfd_reloc_status_type
xgate_elf_special_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			 arelent *reloc_entry ATTRIBUTE_UNUSED,
			 asymbol *symbol ATTRIBUTE_UNUSED,
			 void *data ATTRIBUTE_UNUSED,
			 asection *input_section ATTRIBUTE_UNUSED,
			 bfd *output_bfd ATTRIBUTE_UNUSED,
			 char **error_message ATTRIBUTE_UNUSED)
{
  abort ();
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

bfd_boolean
elf32_xgate_check_relocs (bfd *abfd ATTRIBUTE_UNUSED,
			  struct bfd_link_info *info ATTRIBUTE_UNUSED,
			  asection *sec ATTRIBUTE_UNUSED,
			  const Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Relocate a XGATE/S12x ELF section.  */

bfd_boolean
elf32_xgate_relocate_section (bfd *output_bfd ATTRIBUTE_UNUSED,
			      struct bfd_link_info *info ATTRIBUTE_UNUSED,
			      bfd *input_bfd ATTRIBUTE_UNUSED,
			      asection *input_section ATTRIBUTE_UNUSED,
			      bfd_byte *contents ATTRIBUTE_UNUSED,
			      Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED,
			      Elf_Internal_Sym *local_syms ATTRIBUTE_UNUSED,
			      asection **local_sections ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Set and control ELF flags in ELF header.  */

bfd_boolean
_bfd_xgate_elf_set_private_flags (bfd *abfd ATTRIBUTE_UNUSED,
				  flagword flags ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

bfd_boolean
_bfd_xgate_elf_merge_private_bfd_data (bfd *ibfd ATTRIBUTE_UNUSED,
				       bfd *obfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd_boolean
_bfd_xgate_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (elf_elfheader (abfd)->e_flags & E_XGATE_I32)
    fprintf (file, _("[abi=32-bit int, "));
  else
    fprintf (file, _("[abi=16-bit int, "));

  if (elf_elfheader (abfd)->e_flags & E_XGATE_F64)
    fprintf (file, _("64-bit double, "));
  else
    fprintf (file, _("32-bit double, "));
  if (elf_elfheader (abfd)->e_flags & EF_XGATE_MACH)
    fprintf (file, _("cpu=XGATE]"));
  else
    fprintf (file, _("error reading cpu type from elf private data"));
  fputc ('\n', file);

  return TRUE;
}

void
elf32_xgate_post_process_headers (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{

}

#define ELF_ARCH                             bfd_arch_xgate
#define ELF_MACHINE_CODE                     EM_XGATE
#define ELF_TARGET_ID                        XGATE_ELF_DATA

#define ELF_MAXPAGESIZE                      0x1000

#define TARGET_BIG_SYM                       xgate_elf32_vec
#define TARGET_BIG_NAME                      "elf32-xgate"

#define elf_info_to_howto                    0
#define elf_info_to_howto_rel                xgate_info_to_howto_rel
#define elf_backend_check_relocs             elf32_xgate_check_relocs
#define elf_backend_relocate_section         elf32_xgate_relocate_section
#define elf_backend_object_p                 xgate_elf_set_mach_from_flags
#define elf_backend_final_write_processing   0
#define elf_backend_can_gc_sections          1
#define elf_backend_special_sections         elf32_xgate_special_sections
#define elf_backend_post_process_headers     elf32_xgate_post_process_headers
#define elf_backend_add_symbol_hook          elf32_xgate_add_symbol_hook

#define bfd_elf32_bfd_link_hash_table_create xgate_elf_bfd_link_hash_table_create
#define bfd_elf32_bfd_merge_private_bfd_data _bfd_xgate_elf_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags      _bfd_xgate_elf_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data _bfd_xgate_elf_print_private_bfd_data

#define xgate_stub_hash_lookup(table, string, create, copy)	\
    ((struct elf32_xgate_stub_hash_entry *) \
        bfd_hash_lookup ((table), (string), (create), (copy)))

#include "elf32-target.h"

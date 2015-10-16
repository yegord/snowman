/* Freescale XGATE-specific support for 32-bit ELF
   Copyright (C) 2012-2014 Free Software Foundation, Inc.

   Contributed by Sean Keys (skeys@ipdatasys.com)
   (Heavily copied from the HC11 port by Stephane Carrez (stcarrez@nerim.fr))

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

#ifndef _ELF32_XGATE_H
#define _ELF32_XGATE_H

#include "elf-bfd.h"
#include "bfdlink.h"
#include "elf/xgate.h"

/* Set and control ELF flags in ELF header.  */
extern bfd_boolean _bfd_xgate_elf_merge_private_bfd_data (bfd*,bfd*);
extern bfd_boolean _bfd_xgate_elf_set_private_flags (bfd*,flagword);
extern bfd_boolean _bfd_xgate_elf_print_private_bfd_data (bfd*, void*);

struct elf32_xgate_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump.  */
  bfd_vma target_value;
  asection *target_section;
};

struct xgate_page_info
{
  bfd_vma bank_virtual;
  bfd_vma bank_physical;
  bfd_vma bank_physical_end;
  bfd_vma bank_mask;
  bfd_vma bank_size;
  int bank_shift;
  int bank_param_initialized;
  bfd_vma trampoline_addr;
};

struct xgate_elf_link_hash_table
{
  struct elf_link_hash_table root;
  struct xgate_page_info pinfo;

  /* The stub hash table.  */
  struct bfd_hash_table* stub_hash_table;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  asection* stub_section;
  asection* tramp_section;

  /* Linker call-backs.  */
  asection * (*add_stub_section) (const char *, asection *);

  /* Assorted information used by elf32_hppa_size_stubs.  */
  unsigned int bfd_count;
  int          top_index;
  asection **  input_list;

  /* Small local sym cache.  */
  struct sym_cache sym_cache;

  bfd_boolean (*size_one_stub) (struct bfd_hash_entry*, void*);
  bfd_boolean (*build_one_stub) (struct bfd_hash_entry*, void*);
};

/* Get the XGate ELF linker hash table from a link_info structure.  */

#define xgate_elf_hash_table(p) \
  ((struct xgate_elf_link_hash_table *) ((p)->hash))

/* Create a XGATE ELF linker hash table.  */

extern struct xgate_elf_link_hash_table* xgate_elf_hash_table_create
  (bfd *);

extern void xgate_elf_get_bank_parameters (struct bfd_link_info *);

/* Return 1 if the address is in banked memory.
   This can be applied to a virtual address and to a physical address.  */
extern int xgate_addr_is_banked (struct xgate_page_info *, bfd_vma);

/* Return the physical address seen by the processor, taking
   into account banked memory.  */
extern bfd_vma xgate_phys_addr (struct xgate_page_info *, bfd_vma);

/* Return the page number corresponding to an address in banked memory.  */
extern bfd_vma xgate_phys_page (struct xgate_page_info *, bfd_vma);

bfd_reloc_status_type xgate_elf_ignore_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
bfd_reloc_status_type xgate_elf_special_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);

bfd_boolean elf32_xgate_check_relocs
  (bfd *, struct bfd_link_info *, asection *, const Elf_Internal_Rela *);
bfd_boolean elf32_xgate_relocate_section
  (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
   Elf_Internal_Rela *, Elf_Internal_Sym *, asection **);

bfd_boolean elf32_xgate_add_symbol_hook
  (bfd *, struct bfd_link_info *, Elf_Internal_Sym *, const char **,
   flagword *, asection **, bfd_vma *);

/* Tweak the OSABI field of the elf header.  */
extern void elf32_xgate_post_process_headers (bfd *, struct bfd_link_info *);

int elf32_xgate_setup_section_lists (bfd *, struct bfd_link_info *);

bfd_boolean elf32_xgate_size_stubs
  (bfd *, bfd *, struct bfd_link_info *,
   asection * (*) (const char *, asection *));

bfd_boolean elf32_xgate_build_stubs (bfd * abfd, struct bfd_link_info *);

#endif /* _ELF32_XGATE_H */

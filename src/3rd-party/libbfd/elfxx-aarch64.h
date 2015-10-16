/* AArch64-specific backend routines.
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

#include "bfd.h"
#include "elf-bfd.h"
#include "stdint.h"

/* Take the PAGE component of an address or offset.  */
#define PG(x)        ((x) & ~ (bfd_vma) 0xfff)
#define PG_OFFSET(x) ((x) &   (bfd_vma) 0xfff)

extern bfd_reloc_status_type
_bfd_aarch64_elf_put_addend (bfd *, bfd_byte *, bfd_reloc_code_real_type,
			     reloc_howto_type *, bfd_signed_vma);

extern bfd_vma
_bfd_aarch64_elf_resolve_relocation (bfd_reloc_code_real_type, bfd_vma, bfd_vma,
				     bfd_vma, bfd_boolean);

extern bfd_boolean
_bfd_aarch64_elf_add_symbol_hook (bfd *, struct bfd_link_info *,
				  Elf_Internal_Sym *, const char **,
				  flagword *, asection **, bfd_vma *);

extern bfd_boolean
_bfd_aarch64_elf_grok_prstatus (bfd *, Elf_Internal_Note *);

extern bfd_boolean
_bfd_aarch64_elf_grok_psinfo (bfd *, Elf_Internal_Note *);

extern char *
_bfd_aarch64_elf_write_core_note (bfd *, char *, int *, int, ...);

#define elf_backend_add_symbol_hook	_bfd_aarch64_elf_add_symbol_hook
#define elf_backend_grok_prstatus	_bfd_aarch64_elf_grok_prstatus
#define elf_backend_grok_psinfo         _bfd_aarch64_elf_grok_psinfo
#define elf_backend_write_core_note     _bfd_aarch64_elf_write_core_note

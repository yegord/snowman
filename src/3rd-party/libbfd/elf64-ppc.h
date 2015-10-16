/* PowerPC64-specific support for 64-bit ELF.
   Copyright (C) 2002-2014 Free Software Foundation, Inc.

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

/* Used to pass info between ld and bfd.  */
struct ppc64_elf_params
{
  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection * (*add_stub_section) (const char *, asection *);
  void (*layout_sections_again) (void);

  /* Maximum size of a group of input sections that can be handled by
     one stub section.  A value of +/-1 indicates the bfd back-end
     should use a suitable default size.  */
  bfd_signed_vma group_size;

  /* Whether to use a special call stub for __tls_get_addr.  */
  int no_tls_get_addr_opt;

  /* Whether to allow multiple toc sections.  */
  int no_multi_toc;

  /* Set if PLT call stubs should load r11.  */
  int plt_static_chain;

  /* Set if PLT call stubs need to be thread safe on power7+.  */
  int plt_thread_safe;

  /* Set if individual PLT call stubs should be aligned.  */
  int plt_stub_align;

  /* Whether to canonicalize .opd so that there are no overlapping
     .opd entries.  */
  int non_overlapping_opd;

  /* Whether to emit symbols for stubs.  */
  int emit_stub_syms;

  /* Whether to generate out-of-line register save/restore for gcc -Os code.  */
  int save_restore_funcs;
};

bfd_boolean ppc64_elf_init_stub_bfd
  (struct bfd_link_info *, struct ppc64_elf_params *);
bfd_boolean ppc64_elf_edit_opd
  (struct bfd_link_info *);
asection *ppc64_elf_tls_setup
  (struct bfd_link_info *);
bfd_boolean ppc64_elf_tls_optimize
  (struct bfd_link_info *);
bfd_boolean ppc64_elf_edit_toc
  (struct bfd_link_info *);
bfd_boolean ppc64_elf_has_small_toc_reloc
  (asection *);
bfd_vma ppc64_elf_set_toc
  (struct bfd_link_info *, bfd *);
int ppc64_elf_setup_section_lists
  (struct bfd_link_info *);
void ppc64_elf_start_multitoc_partition
  (struct bfd_link_info *);
bfd_boolean ppc64_elf_next_toc_section
  (struct bfd_link_info *, asection *);
bfd_boolean ppc64_elf_layout_multitoc
  (struct bfd_link_info *);
void ppc64_elf_finish_multitoc_partition
  (struct bfd_link_info *);
bfd_boolean ppc64_elf_check_init_fini
  (struct bfd_link_info *);
bfd_boolean ppc64_elf_next_input_section
  (struct bfd_link_info *, asection *);
bfd_boolean ppc64_elf_size_stubs
(struct bfd_link_info *);
bfd_boolean ppc64_elf_build_stubs
  (struct bfd_link_info *, char **);
void ppc64_elf_restore_symbols
  (struct bfd_link_info *info);

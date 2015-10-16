/* IBM S/390-specific support for ELF 32 and 64 bit functions
   Copyright (C) 2000-2014 Free Software Foundation, Inc.
   Contributed by Andreas Krebbel.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */


/* Return TRUE if H is an IFUNC symbol.  Simply checking for the
   symbol type might not be enough since it might get changed to
   STT_FUNC for pointer equality reasons.  */
static inline bfd_boolean
s390_is_ifunc_symbol_p (struct elf_link_hash_entry *h)
{
  struct elf_s390_link_hash_entry *eh = (struct elf_s390_link_hash_entry*)h;
  return h->type == STT_GNU_IFUNC || eh->ifunc_resolver_address != 0;
}

/* Create sections needed by STT_GNU_IFUNC symbol.  */

static bfd_boolean
s390_elf_create_ifunc_sections (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags;
  asection *s;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (htab->iplt != NULL)
    return TRUE;

  flags = bed->dynamic_sec_flags;

  if (info->shared)
    {
      s = bfd_make_section_with_flags (abfd, ".rela.ifunc",
				       flags | SEC_READONLY);
      if (s == NULL
	  || ! bfd_set_section_alignment (abfd, s,
					  bed->s->log_file_align))
	return FALSE;
      htab->irelifunc = s;
    }

  /* Create .iplt, .rel[a].iplt, and .igot.plt.  */
  s = bfd_make_section_with_flags (abfd, ".iplt",
				   flags | SEC_CODE | SEC_READONLY);
  if (s == NULL
      || ! bfd_set_section_alignment (abfd, s, bed->plt_alignment))
    return FALSE;
  htab->iplt = s;

  s = bfd_make_section_with_flags (abfd, ".rela.iplt", flags | SEC_READONLY);
  if (s == NULL
      || ! bfd_set_section_alignment (abfd, s,
				      bed->s->log_file_align))
    return FALSE;
  htab->irelplt = s;

  s = bfd_make_section_with_flags (abfd, ".igot.plt", flags);
  if (s == NULL
      || !bfd_set_section_alignment (abfd, s,
				     bed->s->log_file_align))
    return FALSE;
  htab->igotplt = s;

  return TRUE;
}


/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs against a STT_GNU_IFUNC symbol definition.  */

static bfd_boolean
s390_elf_allocate_ifunc_dyn_relocs (struct bfd_link_info *info,
				    struct elf_link_hash_entry *h,
				    struct elf_dyn_relocs **head)
{
  struct elf_dyn_relocs *p;
  struct elf_link_hash_table *htab;
  struct elf_s390_link_hash_entry *eh = (struct elf_s390_link_hash_entry*)h;

  htab = elf_hash_table (info);
  eh->ifunc_resolver_address = h->root.u.def.value;
  eh->ifunc_resolver_section = h->root.u.def.section;

  /* Support garbage collection against STT_GNU_IFUNC symbols.  */
  if (h->plt.refcount <= 0 && h->got.refcount <= 0)
    {
      /* When building shared library, we need to handle the case
         where it is marked with regular reference, but not non-GOT
	 reference.  It may happen if we didn't see STT_GNU_IFUNC
	 symbol at the time when checking relocations.  */
      if (info->shared
	  && !h->non_got_ref
	  && h->ref_regular)
	for (p = *head; p != NULL; p = p->next)
	  if (p->count)
	    {
	      h->non_got_ref = 1;
	      goto keep;
	    }

      h->got = htab->init_got_offset;
      h->plt = htab->init_plt_offset;
      *head = NULL;
      return TRUE;
    }

  /* Return and discard space for dynamic relocations against it if
     it is never referenced in a non-shared object.  */
  if (!h->ref_regular)
    {
      if (h->plt.refcount > 0
	  || h->got.refcount > 0)
	abort ();
      h->got = htab->init_got_offset;
      h->plt = htab->init_plt_offset;
      *head = NULL;
      return TRUE;
    }

keep:
  /* Without checking h->plt.refcount here we allocate a PLT slot.
     When setting plt.refcount in check_relocs it might not have been
     known that this will be an IFUNC symol.  */
  h->plt.offset = htab->iplt->size;
  h->needs_plt = 1;
  htab->iplt->size += PLT_ENTRY_SIZE;
  htab->igotplt->size += GOT_ENTRY_SIZE;
  htab->irelplt->size += RELA_ENTRY_SIZE;
  htab->irelplt->reloc_count++;

  /* In order to make pointer equality work with IFUNC symbols defined
     in a non-PIE executable and referenced in a shared lib, we turn
     the symbol into a STT_FUNC symbol and make the symbol value to
     point to the IPLT slot.  That way the referencing shared lib will
     always get the PLT slot address when resolving the respective
     R_390_GLOB_DAT/R_390_64 relocs on that symbol.  */
  if (info->executable && !info->shared && h->def_regular && h->ref_dynamic)
    {
      h->root.u.def.section = htab->iplt;
      h->root.u.def.value = h->plt.offset;
      h->size = PLT_ENTRY_SIZE;
      h->type = STT_FUNC;
    }

  /* We need dynamic relocation for STT_GNU_IFUNC symbol only when
     there is a non-GOT reference in a shared object.  */
  if (!info->shared || !h->non_got_ref)
    *head = NULL;

  /* Finally, allocate space.  */
  p = *head;
  if (p != NULL)
    {
      bfd_size_type count = 0;
      do
	{
	  count += p->count;
	  p = p->next;
	}
      while (p != NULL);
      htab->irelifunc->size += count * RELA_ENTRY_SIZE;
    }

  /* Decide whether the got.iplt slot can be used.  This has to be
     avoided if the values in the GOT slots could differ for pointer
     equality reasons.  */
  if (h->got.refcount <= 0
      || (info->shared
	  && (h->dynindx == -1 || h->forced_local))
      || (info->executable && info->shared)
      || htab->sgot == NULL)
    {
      /* Use .got.iplt.  */
      h->got.offset = (bfd_vma) -1;
    }
  else
    {
      h->got.offset = htab->sgot->size;
      htab->sgot->size += GOT_ENTRY_SIZE;
      if (info->shared)
	htab->srelgot->size += RELA_ENTRY_SIZE;
    }

  return TRUE;
}

static bfd_boolean
elf_s390_allocate_local_syminfo (bfd *abfd, Elf_Internal_Shdr *symtab_hdr)
{
  bfd_size_type size;

  size = symtab_hdr->sh_info;
  size *= (sizeof (bfd_signed_vma)       /* local got */
	   + sizeof (struct plt_entry)   /* local plt */
	   + sizeof(char));              /* local tls type */
  elf_local_got_refcounts (abfd) = ((bfd_signed_vma *)
				    bfd_zalloc (abfd, size));
  if (elf_local_got_refcounts (abfd) == NULL)
    return FALSE;
  elf_s390_local_plt (abfd)
    = (struct plt_entry*)(elf_local_got_refcounts (abfd)
			  + symtab_hdr->sh_info);
  elf_s390_local_got_tls_type (abfd)
    = (char *) (elf_s390_local_plt (abfd) + symtab_hdr->sh_info);

  return TRUE;
}

/* Pick ELFOSABI_GNU if IFUNC symbols are used.  */

static bfd_boolean
elf_s390_add_symbol_hook (bfd *abfd,
			  struct bfd_link_info *info,
			  Elf_Internal_Sym *sym,
			  const char **namep ATTRIBUTE_UNUSED,
			  flagword *flagsp ATTRIBUTE_UNUSED,
			  asection **secp ATTRIBUTE_UNUSED,
			  bfd_vma *valp ATTRIBUTE_UNUSED)
{
  if ((ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC
       || ELF_ST_BIND (sym->st_info) == STB_GNU_UNIQUE)
      && (abfd->flags & DYNAMIC) == 0
      && bfd_get_flavour (info->output_bfd) == bfd_target_elf_flavour)
    elf_tdata (info->output_bfd)->has_gnu_symbols = TRUE;

  return TRUE;
}

/* Whether to sort relocs output by ld -r or ld --emit-relocs, by
   r_offset.  Don't do so for code sections.  We want to keep ordering
   of GDCALL / PLT32DBL for TLS optimizations as is.  On the other
   hand, elf-eh-frame.c processing requires .eh_frame relocs to be
   sorted.  */

static bfd_boolean
elf_s390_elf_sort_relocs_p (asection *sec)
{
  return (sec->flags & SEC_CODE) == 0;
}

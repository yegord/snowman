/* ELF executable support for BFD.

   Copyright (C) 1993-2014 Free Software Foundation, Inc.

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


/*
SECTION
	ELF backends

	BFD support for ELF formats is being worked on.
	Currently, the best supported back ends are for sparc and i386
	(running svr4 or Solaris 2).

	Documentation of the internals of the support code still needs
	to be written.  The code is changing quickly enough that we
	haven't bothered yet.  */

/* For sparc64-cross-sparc32.  */
#define _SYSCALL32
#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#define ARCH_SIZE 0
#include "elf-bfd.h"
#include "libiberty.h"
#include "safe-ctype.h"
#include "elf-linux-psinfo.h"

#ifdef CORE_HEADER
#include CORE_HEADER
#endif

static int elf_sort_sections (const void *, const void *);
static bfd_boolean assign_file_positions_except_relocs (bfd *, struct bfd_link_info *);
static bfd_boolean prep_headers (bfd *);
static bfd_boolean swap_out_syms (bfd *, struct bfd_strtab_hash **, int) ;
static bfd_boolean elf_read_notes (bfd *, file_ptr, bfd_size_type) ;
static bfd_boolean elf_parse_notes (bfd *abfd, char *buf, size_t size,
				    file_ptr offset);

/* Swap version information in and out.  The version information is
   currently size independent.  If that ever changes, this code will
   need to move into elfcode.h.  */

/* Swap in a Verdef structure.  */

void
_bfd_elf_swap_verdef_in (bfd *abfd,
			 const Elf_External_Verdef *src,
			 Elf_Internal_Verdef *dst)
{
  dst->vd_version = H_GET_16 (abfd, src->vd_version);
  dst->vd_flags   = H_GET_16 (abfd, src->vd_flags);
  dst->vd_ndx     = H_GET_16 (abfd, src->vd_ndx);
  dst->vd_cnt     = H_GET_16 (abfd, src->vd_cnt);
  dst->vd_hash    = H_GET_32 (abfd, src->vd_hash);
  dst->vd_aux     = H_GET_32 (abfd, src->vd_aux);
  dst->vd_next    = H_GET_32 (abfd, src->vd_next);
}

/* Swap out a Verdef structure.  */

void
_bfd_elf_swap_verdef_out (bfd *abfd,
			  const Elf_Internal_Verdef *src,
			  Elf_External_Verdef *dst)
{
  H_PUT_16 (abfd, src->vd_version, dst->vd_version);
  H_PUT_16 (abfd, src->vd_flags, dst->vd_flags);
  H_PUT_16 (abfd, src->vd_ndx, dst->vd_ndx);
  H_PUT_16 (abfd, src->vd_cnt, dst->vd_cnt);
  H_PUT_32 (abfd, src->vd_hash, dst->vd_hash);
  H_PUT_32 (abfd, src->vd_aux, dst->vd_aux);
  H_PUT_32 (abfd, src->vd_next, dst->vd_next);
}

/* Swap in a Verdaux structure.  */

void
_bfd_elf_swap_verdaux_in (bfd *abfd,
			  const Elf_External_Verdaux *src,
			  Elf_Internal_Verdaux *dst)
{
  dst->vda_name = H_GET_32 (abfd, src->vda_name);
  dst->vda_next = H_GET_32 (abfd, src->vda_next);
}

/* Swap out a Verdaux structure.  */

void
_bfd_elf_swap_verdaux_out (bfd *abfd,
			   const Elf_Internal_Verdaux *src,
			   Elf_External_Verdaux *dst)
{
  H_PUT_32 (abfd, src->vda_name, dst->vda_name);
  H_PUT_32 (abfd, src->vda_next, dst->vda_next);
}

/* Swap in a Verneed structure.  */

void
_bfd_elf_swap_verneed_in (bfd *abfd,
			  const Elf_External_Verneed *src,
			  Elf_Internal_Verneed *dst)
{
  dst->vn_version = H_GET_16 (abfd, src->vn_version);
  dst->vn_cnt     = H_GET_16 (abfd, src->vn_cnt);
  dst->vn_file    = H_GET_32 (abfd, src->vn_file);
  dst->vn_aux     = H_GET_32 (abfd, src->vn_aux);
  dst->vn_next    = H_GET_32 (abfd, src->vn_next);
}

/* Swap out a Verneed structure.  */

void
_bfd_elf_swap_verneed_out (bfd *abfd,
			   const Elf_Internal_Verneed *src,
			   Elf_External_Verneed *dst)
{
  H_PUT_16 (abfd, src->vn_version, dst->vn_version);
  H_PUT_16 (abfd, src->vn_cnt, dst->vn_cnt);
  H_PUT_32 (abfd, src->vn_file, dst->vn_file);
  H_PUT_32 (abfd, src->vn_aux, dst->vn_aux);
  H_PUT_32 (abfd, src->vn_next, dst->vn_next);
}

/* Swap in a Vernaux structure.  */

void
_bfd_elf_swap_vernaux_in (bfd *abfd,
			  const Elf_External_Vernaux *src,
			  Elf_Internal_Vernaux *dst)
{
  dst->vna_hash  = H_GET_32 (abfd, src->vna_hash);
  dst->vna_flags = H_GET_16 (abfd, src->vna_flags);
  dst->vna_other = H_GET_16 (abfd, src->vna_other);
  dst->vna_name  = H_GET_32 (abfd, src->vna_name);
  dst->vna_next  = H_GET_32 (abfd, src->vna_next);
}

/* Swap out a Vernaux structure.  */

void
_bfd_elf_swap_vernaux_out (bfd *abfd,
			   const Elf_Internal_Vernaux *src,
			   Elf_External_Vernaux *dst)
{
  H_PUT_32 (abfd, src->vna_hash, dst->vna_hash);
  H_PUT_16 (abfd, src->vna_flags, dst->vna_flags);
  H_PUT_16 (abfd, src->vna_other, dst->vna_other);
  H_PUT_32 (abfd, src->vna_name, dst->vna_name);
  H_PUT_32 (abfd, src->vna_next, dst->vna_next);
}

/* Swap in a Versym structure.  */

void
_bfd_elf_swap_versym_in (bfd *abfd,
			 const Elf_External_Versym *src,
			 Elf_Internal_Versym *dst)
{
  dst->vs_vers = H_GET_16 (abfd, src->vs_vers);
}

/* Swap out a Versym structure.  */

void
_bfd_elf_swap_versym_out (bfd *abfd,
			  const Elf_Internal_Versym *src,
			  Elf_External_Versym *dst)
{
  H_PUT_16 (abfd, src->vs_vers, dst->vs_vers);
}

/* Standard ELF hash function.  Do not change this function; you will
   cause invalid hash tables to be generated.  */

unsigned long
bfd_elf_hash (const char *namearg)
{
  const unsigned char *name = (const unsigned char *) namearg;
  unsigned long h = 0;
  unsigned long g;
  int ch;

  while ((ch = *name++) != '\0')
    {
      h = (h << 4) + ch;
      if ((g = (h & 0xf0000000)) != 0)
	{
	  h ^= g >> 24;
	  /* The ELF ABI says `h &= ~g', but this is equivalent in
	     this case and on some machines one insn instead of two.  */
	  h ^= g;
	}
    }
  return h & 0xffffffff;
}

/* DT_GNU_HASH hash function.  Do not change this function; you will
   cause invalid hash tables to be generated.  */

unsigned long
bfd_elf_gnu_hash (const char *namearg)
{
  const unsigned char *name = (const unsigned char *) namearg;
  unsigned long h = 5381;
  unsigned char ch;

  while ((ch = *name++) != '\0')
    h = (h << 5) + h + ch;
  return h & 0xffffffff;
}

/* Create a tdata field OBJECT_SIZE bytes in length, zeroed out and with
   the object_id field of an elf_obj_tdata field set to OBJECT_ID.  */
bfd_boolean
bfd_elf_allocate_object (bfd *abfd,
			 size_t object_size,
			 enum elf_target_id object_id)
{
  BFD_ASSERT (object_size >= sizeof (struct elf_obj_tdata));
  abfd->tdata.any = bfd_zalloc (abfd, object_size);
  if (abfd->tdata.any == NULL)
    return FALSE;

  elf_object_id (abfd) = object_id;
  if (abfd->direction != read_direction)
    {
      struct output_elf_obj_tdata *o = bfd_zalloc (abfd, sizeof *o);
      if (o == NULL)
	return FALSE;
      elf_tdata (abfd)->o = o;
      elf_program_header_size (abfd) = (bfd_size_type) -1;
    }
  return TRUE;
}


bfd_boolean
bfd_elf_make_object (bfd *abfd)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_obj_tdata),
				  bed->target_id);
}

bfd_boolean
bfd_elf_mkcorefile (bfd *abfd)
{
  /* I think this can be done just like an object file.  */
  if (!abfd->xvec->_bfd_set_format[(int) bfd_object] (abfd))
    return FALSE;
  elf_tdata (abfd)->core = bfd_zalloc (abfd, sizeof (*elf_tdata (abfd)->core));
  return elf_tdata (abfd)->core != NULL;
}

static char *
bfd_elf_get_str_section (bfd *abfd, unsigned int shindex)
{
  Elf_Internal_Shdr **i_shdrp;
  bfd_byte *shstrtab = NULL;
  file_ptr offset;
  bfd_size_type shstrtabsize;

  i_shdrp = elf_elfsections (abfd);
  if (i_shdrp == 0
      || shindex >= elf_numsections (abfd)
      || i_shdrp[shindex] == 0)
    return NULL;

  shstrtab = i_shdrp[shindex]->contents;
  if (shstrtab == NULL)
    {
      /* No cached one, attempt to read, and cache what we read.  */
      offset = i_shdrp[shindex]->sh_offset;
      shstrtabsize = i_shdrp[shindex]->sh_size;

      /* Allocate and clear an extra byte at the end, to prevent crashes
	 in case the string table is not terminated.  */
      if (shstrtabsize + 1 <= 1
	  || bfd_seek (abfd, offset, SEEK_SET) != 0
	  || (shstrtab = (bfd_byte *) bfd_alloc (abfd, shstrtabsize + 1)) == NULL)
	shstrtab = NULL;
      else if (bfd_bread (shstrtab, shstrtabsize, abfd) != shstrtabsize)
	{
	  if (bfd_get_error () != bfd_error_system_call)
	    bfd_set_error (bfd_error_file_truncated);
	  bfd_release (abfd, shstrtab);
	  shstrtab = NULL;
	  /* Once we've failed to read it, make sure we don't keep
	     trying.  Otherwise, we'll keep allocating space for
	     the string table over and over.  */
	  i_shdrp[shindex]->sh_size = 0;
	}
      else
	shstrtab[shstrtabsize] = '\0';
      i_shdrp[shindex]->contents = shstrtab;
    }
  return (char *) shstrtab;
}

char *
bfd_elf_string_from_elf_section (bfd *abfd,
				 unsigned int shindex,
				 unsigned int strindex)
{
  Elf_Internal_Shdr *hdr;

  if (strindex == 0)
    return "";

  if (elf_elfsections (abfd) == NULL || shindex >= elf_numsections (abfd))
    return NULL;

  hdr = elf_elfsections (abfd)[shindex];

  if (hdr->contents == NULL)
    {
      if (hdr->sh_type != SHT_STRTAB && hdr->sh_type < SHT_LOOS)
	{
	  /* PR 17512: file: f057ec89.  */
	  _bfd_error_handler (_("%B: attempt to load strings from a non-string section (number %d)"),
			      abfd, shindex);
	  return NULL;
	}
  
      if (bfd_elf_get_str_section (abfd, shindex) == NULL)
	return NULL;
    }

  if (strindex >= hdr->sh_size)
    {
      unsigned int shstrndx = elf_elfheader(abfd)->e_shstrndx;
      (*_bfd_error_handler)
	(_("%B: invalid string offset %u >= %lu for section `%s'"),
	 abfd, strindex, (unsigned long) hdr->sh_size,
	 (shindex == shstrndx && strindex == hdr->sh_name
	  ? ".shstrtab"
	  : bfd_elf_string_from_elf_section (abfd, shstrndx, hdr->sh_name)));
      return NULL;
    }

  return ((char *) hdr->contents) + strindex;
}

/* Read and convert symbols to internal format.
   SYMCOUNT specifies the number of symbols to read, starting from
   symbol SYMOFFSET.  If any of INTSYM_BUF, EXTSYM_BUF or EXTSHNDX_BUF
   are non-NULL, they are used to store the internal symbols, external
   symbols, and symbol section index extensions, respectively.
   Returns a pointer to the internal symbol buffer (malloced if necessary)
   or NULL if there were no symbols or some kind of problem.  */

Elf_Internal_Sym *
bfd_elf_get_elf_syms (bfd *ibfd,
		      Elf_Internal_Shdr *symtab_hdr,
		      size_t symcount,
		      size_t symoffset,
		      Elf_Internal_Sym *intsym_buf,
		      void *extsym_buf,
		      Elf_External_Sym_Shndx *extshndx_buf)
{
  Elf_Internal_Shdr *shndx_hdr;
  void *alloc_ext;
  const bfd_byte *esym;
  Elf_External_Sym_Shndx *alloc_extshndx;
  Elf_External_Sym_Shndx *shndx;
  Elf_Internal_Sym *alloc_intsym;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  const struct elf_backend_data *bed;
  size_t extsym_size;
  bfd_size_type amt;
  file_ptr pos;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
    abort ();

  if (symcount == 0)
    return intsym_buf;

  /* Normal syms might have section extension entries.  */
  shndx_hdr = NULL;
  if (symtab_hdr == &elf_tdata (ibfd)->symtab_hdr)
    shndx_hdr = &elf_tdata (ibfd)->symtab_shndx_hdr;

  /* Read the symbols.  */
  alloc_ext = NULL;
  alloc_extshndx = NULL;
  alloc_intsym = NULL;
  bed = get_elf_backend_data (ibfd);
  extsym_size = bed->s->sizeof_sym;
  amt = symcount * extsym_size;
  pos = symtab_hdr->sh_offset + symoffset * extsym_size;
  if (extsym_buf == NULL)
    {
      alloc_ext = bfd_malloc2 (symcount, extsym_size);
      extsym_buf = alloc_ext;
    }
  if (extsym_buf == NULL
      || bfd_seek (ibfd, pos, SEEK_SET) != 0
      || bfd_bread (extsym_buf, amt, ibfd) != amt)
    {
      intsym_buf = NULL;
      goto out;
    }

  if (shndx_hdr == NULL || shndx_hdr->sh_size == 0)
    extshndx_buf = NULL;
  else
    {
      amt = symcount * sizeof (Elf_External_Sym_Shndx);
      pos = shndx_hdr->sh_offset + symoffset * sizeof (Elf_External_Sym_Shndx);
      if (extshndx_buf == NULL)
	{
	  alloc_extshndx = (Elf_External_Sym_Shndx *)
              bfd_malloc2 (symcount, sizeof (Elf_External_Sym_Shndx));
	  extshndx_buf = alloc_extshndx;
	}
      if (extshndx_buf == NULL
	  || bfd_seek (ibfd, pos, SEEK_SET) != 0
	  || bfd_bread (extshndx_buf, amt, ibfd) != amt)
	{
	  intsym_buf = NULL;
	  goto out;
	}
    }

  if (intsym_buf == NULL)
    {
      alloc_intsym = (Elf_Internal_Sym *)
          bfd_malloc2 (symcount, sizeof (Elf_Internal_Sym));
      intsym_buf = alloc_intsym;
      if (intsym_buf == NULL)
	goto out;
    }

  /* Convert the symbols to internal form.  */
  isymend = intsym_buf + symcount;
  for (esym = (const bfd_byte *) extsym_buf, isym = intsym_buf,
           shndx = extshndx_buf;
       isym < isymend;
       esym += extsym_size, isym++, shndx = shndx != NULL ? shndx + 1 : NULL)
    if (!(*bed->s->swap_symbol_in) (ibfd, esym, shndx, isym))
      {
	symoffset += (esym - (bfd_byte *) extsym_buf) / extsym_size;
	(*_bfd_error_handler) (_("%B symbol number %lu references "
				 "nonexistent SHT_SYMTAB_SHNDX section"),
			       ibfd, (unsigned long) symoffset);
	if (alloc_intsym != NULL)
	  free (alloc_intsym);
	intsym_buf = NULL;
	goto out;
      }

 out:
  if (alloc_ext != NULL)
    free (alloc_ext);
  if (alloc_extshndx != NULL)
    free (alloc_extshndx);

  return intsym_buf;
}

/* Look up a symbol name.  */
const char *
bfd_elf_sym_name (bfd *abfd,
		  Elf_Internal_Shdr *symtab_hdr,
		  Elf_Internal_Sym *isym,
		  asection *sym_sec)
{
  const char *name;
  unsigned int iname = isym->st_name;
  unsigned int shindex = symtab_hdr->sh_link;

  if (iname == 0 && ELF_ST_TYPE (isym->st_info) == STT_SECTION
      /* Check for a bogus st_shndx to avoid crashing.  */
      && isym->st_shndx < elf_numsections (abfd))
    {
      iname = elf_elfsections (abfd)[isym->st_shndx]->sh_name;
      shindex = elf_elfheader (abfd)->e_shstrndx;
    }

  name = bfd_elf_string_from_elf_section (abfd, shindex, iname);
  if (name == NULL)
    name = "(null)";
  else if (sym_sec && *name == '\0')
    name = bfd_section_name (abfd, sym_sec);

  return name;
}

/* Elf_Internal_Shdr->contents is an array of these for SHT_GROUP
   sections.  The first element is the flags, the rest are section
   pointers.  */

typedef union elf_internal_group {
  Elf_Internal_Shdr *shdr;
  unsigned int flags;
} Elf_Internal_Group;

/* Return the name of the group signature symbol.  Why isn't the
   signature just a string?  */

static const char *
group_signature (bfd *abfd, Elf_Internal_Shdr *ghdr)
{
  Elf_Internal_Shdr *hdr;
  unsigned char esym[sizeof (Elf64_External_Sym)];
  Elf_External_Sym_Shndx eshndx;
  Elf_Internal_Sym isym;

  /* First we need to ensure the symbol table is available.  Make sure
     that it is a symbol table section.  */
  if (ghdr->sh_link >= elf_numsections (abfd))
    return NULL;
  hdr = elf_elfsections (abfd) [ghdr->sh_link];
  if (hdr->sh_type != SHT_SYMTAB
      || ! bfd_section_from_shdr (abfd, ghdr->sh_link))
    return NULL;

  /* Go read the symbol.  */
  hdr = &elf_tdata (abfd)->symtab_hdr;
  if (bfd_elf_get_elf_syms (abfd, hdr, 1, ghdr->sh_info,
			    &isym, esym, &eshndx) == NULL)
    return NULL;

  return bfd_elf_sym_name (abfd, hdr, &isym, NULL);
}

/* Set next_in_group list pointer, and group name for NEWSECT.  */

static bfd_boolean
setup_group (bfd *abfd, Elf_Internal_Shdr *hdr, asection *newsect)
{
  unsigned int num_group = elf_tdata (abfd)->num_group;

  /* If num_group is zero, read in all SHT_GROUP sections.  The count
     is set to -1 if there are no SHT_GROUP sections.  */
  if (num_group == 0)
    {
      unsigned int i, shnum;

      /* First count the number of groups.  If we have a SHT_GROUP
	 section with just a flag word (ie. sh_size is 4), ignore it.  */
      shnum = elf_numsections (abfd);
      num_group = 0;

#define IS_VALID_GROUP_SECTION_HEADER(shdr, minsize)	\
	(   (shdr)->sh_type == SHT_GROUP		\
	 && (shdr)->sh_size >= minsize			\
	 && (shdr)->sh_entsize == GRP_ENTRY_SIZE	\
	 && ((shdr)->sh_size % GRP_ENTRY_SIZE) == 0)

      for (i = 0; i < shnum; i++)
	{
	  Elf_Internal_Shdr *shdr = elf_elfsections (abfd)[i];

	  if (IS_VALID_GROUP_SECTION_HEADER (shdr, 2 * GRP_ENTRY_SIZE))
	    num_group += 1;
	}

      if (num_group == 0)
	{
	  num_group = (unsigned) -1;
	  elf_tdata (abfd)->num_group = num_group;
	}
      else
	{
	  /* We keep a list of elf section headers for group sections,
	     so we can find them quickly.  */
	  bfd_size_type amt;

	  elf_tdata (abfd)->num_group = num_group;
	  elf_tdata (abfd)->group_sect_ptr = (Elf_Internal_Shdr **)
              bfd_alloc2 (abfd, num_group, sizeof (Elf_Internal_Shdr *));
	  if (elf_tdata (abfd)->group_sect_ptr == NULL)
	    return FALSE;

	  num_group = 0;
	  for (i = 0; i < shnum; i++)
	    {
	      Elf_Internal_Shdr *shdr = elf_elfsections (abfd)[i];

	      if (IS_VALID_GROUP_SECTION_HEADER (shdr, 2 * GRP_ENTRY_SIZE))
		{
		  unsigned char *src;
		  Elf_Internal_Group *dest;

		  /* Add to list of sections.  */
		  elf_tdata (abfd)->group_sect_ptr[num_group] = shdr;
		  num_group += 1;

		  /* Read the raw contents.  */
		  BFD_ASSERT (sizeof (*dest) >= 4);
		  amt = shdr->sh_size * sizeof (*dest) / 4;
		  shdr->contents = (unsigned char *)
                      bfd_alloc2 (abfd, shdr->sh_size, sizeof (*dest) / 4);
		  /* PR binutils/4110: Handle corrupt group headers.  */
		  if (shdr->contents == NULL)
		    {
		      _bfd_error_handler
			(_("%B: corrupt size field in group section header: 0x%lx"), abfd, shdr->sh_size);
		      bfd_set_error (bfd_error_bad_value);
		      -- num_group;
		      continue;
		    }

		  memset (shdr->contents, 0, amt);

		  if (bfd_seek (abfd, shdr->sh_offset, SEEK_SET) != 0
		      || (bfd_bread (shdr->contents, shdr->sh_size, abfd)
			  != shdr->sh_size))
		    {
		      _bfd_error_handler
			(_("%B: invalid size field in group section header: 0x%lx"), abfd, shdr->sh_size);
		      bfd_set_error (bfd_error_bad_value);
		      -- num_group;
		      /* PR 17510: If the group contents are even partially
			 corrupt, do not allow any of the contents to be used.  */
		      memset (shdr->contents, 0, amt);
		      continue;
		    }

		  /* Translate raw contents, a flag word followed by an
		     array of elf section indices all in target byte order,
		     to the flag word followed by an array of elf section
		     pointers.  */
		  src = shdr->contents + shdr->sh_size;
		  dest = (Elf_Internal_Group *) (shdr->contents + amt);
		  while (1)
		    {
		      unsigned int idx;

		      src -= 4;
		      --dest;
		      idx = H_GET_32 (abfd, src);
		      if (src == shdr->contents)
			{
			  dest->flags = idx;
			  if (shdr->bfd_section != NULL && (idx & GRP_COMDAT))
			    shdr->bfd_section->flags
			      |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
			  break;
			}
		      if (idx >= shnum)
			{
			  ((*_bfd_error_handler)
			   (_("%B: invalid SHT_GROUP entry"), abfd));
			  idx = 0;
			}
		      dest->shdr = elf_elfsections (abfd)[idx];
		    }
		}
	    }

	  /* PR 17510: Corrupt binaries might contain invalid groups.  */
	  if (num_group != (unsigned) elf_tdata (abfd)->num_group)
	    {
	      elf_tdata (abfd)->num_group = num_group;

	      /* If all groups are invalid then fail.  */
	      if (num_group == 0)
		{
		  elf_tdata (abfd)->group_sect_ptr = NULL;
		  elf_tdata (abfd)->num_group = num_group = -1;
		  (*_bfd_error_handler) (_("%B: no valid group sections found"), abfd);
		  bfd_set_error (bfd_error_bad_value);
		}
	    }
	}
    }

  if (num_group != (unsigned) -1)
    {
      unsigned int i;

      for (i = 0; i < num_group; i++)
	{
	  Elf_Internal_Shdr *shdr = elf_tdata (abfd)->group_sect_ptr[i];
	  Elf_Internal_Group *idx = (Elf_Internal_Group *) shdr->contents;
	  unsigned int n_elt = shdr->sh_size / 4;

	  /* Look through this group's sections to see if current
	     section is a member.  */
	  while (--n_elt != 0)
	    if ((++idx)->shdr == hdr)
	      {
		asection *s = NULL;

		/* We are a member of this group.  Go looking through
		   other members to see if any others are linked via
		   next_in_group.  */
		idx = (Elf_Internal_Group *) shdr->contents;
		n_elt = shdr->sh_size / 4;
		while (--n_elt != 0)
		  if ((s = (++idx)->shdr->bfd_section) != NULL
		      && elf_next_in_group (s) != NULL)
		    break;
		if (n_elt != 0)
		  {
		    /* Snarf the group name from other member, and
		       insert current section in circular list.  */
		    elf_group_name (newsect) = elf_group_name (s);
		    elf_next_in_group (newsect) = elf_next_in_group (s);
		    elf_next_in_group (s) = newsect;
		  }
		else
		  {
		    const char *gname;

		    gname = group_signature (abfd, shdr);
		    if (gname == NULL)
		      return FALSE;
		    elf_group_name (newsect) = gname;

		    /* Start a circular list with one element.  */
		    elf_next_in_group (newsect) = newsect;
		  }

		/* If the group section has been created, point to the
		   new member.  */
		if (shdr->bfd_section != NULL)
		  elf_next_in_group (shdr->bfd_section) = newsect;

		i = num_group - 1;
		break;
	      }
	}
    }

  if (elf_group_name (newsect) == NULL)
    {
      (*_bfd_error_handler) (_("%B: no group info for section %A"),
			     abfd, newsect);
      return FALSE;
    }
  return TRUE;
}

bfd_boolean
_bfd_elf_setup_sections (bfd *abfd)
{
  unsigned int i;
  unsigned int num_group = elf_tdata (abfd)->num_group;
  bfd_boolean result = TRUE;
  asection *s;

  /* Process SHF_LINK_ORDER.  */
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      Elf_Internal_Shdr *this_hdr = &elf_section_data (s)->this_hdr;
      if ((this_hdr->sh_flags & SHF_LINK_ORDER) != 0)
	{
	  unsigned int elfsec = this_hdr->sh_link;
	  /* FIXME: The old Intel compiler and old strip/objcopy may
	     not set the sh_link or sh_info fields.  Hence we could
	     get the situation where elfsec is 0.  */
	  if (elfsec == 0)
	    {
	      const struct elf_backend_data *bed = get_elf_backend_data (abfd);
	      if (bed->link_order_error_handler)
		bed->link_order_error_handler
		  (_("%B: warning: sh_link not set for section `%A'"),
		   abfd, s);
	    }
	  else
	    {
	      asection *linksec = NULL;

	      if (elfsec < elf_numsections (abfd))
		{
		  this_hdr = elf_elfsections (abfd)[elfsec];
		  linksec = this_hdr->bfd_section;
		}

	      /* PR 1991, 2008:
		 Some strip/objcopy may leave an incorrect value in
		 sh_link.  We don't want to proceed.  */
	      if (linksec == NULL)
		{
		  (*_bfd_error_handler)
		    (_("%B: sh_link [%d] in section `%A' is incorrect"),
		     s->owner, s, elfsec);
		  result = FALSE;
		}

	      elf_linked_to_section (s) = linksec;
	    }
	}
    }

  /* Process section groups.  */
  if (num_group == (unsigned) -1)
    return result;

  for (i = 0; i < num_group; i++)
    {
      Elf_Internal_Shdr *shdr = elf_tdata (abfd)->group_sect_ptr[i];
      Elf_Internal_Group *idx = (Elf_Internal_Group *) shdr->contents;
      unsigned int n_elt = shdr->sh_size / 4;

      while (--n_elt != 0)
	if ((++idx)->shdr->bfd_section)
	  elf_sec_group (idx->shdr->bfd_section) = shdr->bfd_section;
	else if (idx->shdr->sh_type == SHT_RELA
		 || idx->shdr->sh_type == SHT_REL)
	  /* We won't include relocation sections in section groups in
	     output object files. We adjust the group section size here
	     so that relocatable link will work correctly when
	     relocation sections are in section group in input object
	     files.  */
	  shdr->bfd_section->size -= 4;
	else
	  {
	    /* There are some unknown sections in the group.  */
	    (*_bfd_error_handler)
	      (_("%B: unknown [%d] section `%s' in group [%s]"),
	       abfd,
	       (unsigned int) idx->shdr->sh_type,
	       bfd_elf_string_from_elf_section (abfd,
						(elf_elfheader (abfd)
						 ->e_shstrndx),
						idx->shdr->sh_name),
	       shdr->bfd_section->name);
	    result = FALSE;
	  }
    }
  return result;
}

bfd_boolean
bfd_elf_is_group_section (bfd *abfd ATTRIBUTE_UNUSED, const asection *sec)
{
  return elf_next_in_group (sec) != NULL;
}

/* Make a BFD section from an ELF section.  We store a pointer to the
   BFD section in the bfd_section field of the header.  */

bfd_boolean
_bfd_elf_make_section_from_shdr (bfd *abfd,
				 Elf_Internal_Shdr *hdr,
				 const char *name,
				 int shindex)
{
  asection *newsect;
  flagword flags;
  const struct elf_backend_data *bed;

  if (hdr->bfd_section != NULL)
    return TRUE;

  newsect = bfd_make_section_anyway (abfd, name);
  if (newsect == NULL)
    return FALSE;

  hdr->bfd_section = newsect;
  elf_section_data (newsect)->this_hdr = *hdr;
  elf_section_data (newsect)->this_idx = shindex;

  /* Always use the real type/flags.  */
  elf_section_type (newsect) = hdr->sh_type;
  elf_section_flags (newsect) = hdr->sh_flags;

  newsect->filepos = hdr->sh_offset;

  if (! bfd_set_section_vma (abfd, newsect, hdr->sh_addr)
      || ! bfd_set_section_size (abfd, newsect, hdr->sh_size)
      || ! bfd_set_section_alignment (abfd, newsect,
				      bfd_log2 (hdr->sh_addralign)))
    return FALSE;

  flags = SEC_NO_FLAGS;
  if (hdr->sh_type != SHT_NOBITS)
    flags |= SEC_HAS_CONTENTS;
  if (hdr->sh_type == SHT_GROUP)
    flags |= SEC_GROUP | SEC_EXCLUDE;
  if ((hdr->sh_flags & SHF_ALLOC) != 0)
    {
      flags |= SEC_ALLOC;
      if (hdr->sh_type != SHT_NOBITS)
	flags |= SEC_LOAD;
    }
  if ((hdr->sh_flags & SHF_WRITE) == 0)
    flags |= SEC_READONLY;
  if ((hdr->sh_flags & SHF_EXECINSTR) != 0)
    flags |= SEC_CODE;
  else if ((flags & SEC_LOAD) != 0)
    flags |= SEC_DATA;
  if ((hdr->sh_flags & SHF_MERGE) != 0)
    {
      flags |= SEC_MERGE;
      newsect->entsize = hdr->sh_entsize;
      if ((hdr->sh_flags & SHF_STRINGS) != 0)
	flags |= SEC_STRINGS;
    }
  if (hdr->sh_flags & SHF_GROUP)
    if (!setup_group (abfd, hdr, newsect))
      return FALSE;
  if ((hdr->sh_flags & SHF_TLS) != 0)
    flags |= SEC_THREAD_LOCAL;
  if ((hdr->sh_flags & SHF_EXCLUDE) != 0)
    flags |= SEC_EXCLUDE;

  if ((flags & SEC_ALLOC) == 0)
    {
      /* The debugging sections appear to be recognized only by name,
	 not any sort of flag.  Their SEC_ALLOC bits are cleared.  */
      if (name [0] == '.')
	{
	  const char *p;
	  int n;
	  if (name[1] == 'd')
	    p = ".debug", n = 6;
	  else if (name[1] == 'g' && name[2] == 'n')
	    p = ".gnu.linkonce.wi.", n = 17;
	  else if (name[1] == 'g' && name[2] == 'd')
	    p = ".gdb_index", n = 11; /* yes we really do mean 11.  */
	  else if (name[1] == 'l')
	    p = ".line", n = 5;
	  else if (name[1] == 's')
	    p = ".stab", n = 5;
	  else if (name[1] == 'z')
	    p = ".zdebug", n = 7;
	  else
	    p = NULL, n = 0;
	  if (p != NULL && strncmp (name, p, n) == 0)
	    flags |= SEC_DEBUGGING;
	}
    }

  /* As a GNU extension, if the name begins with .gnu.linkonce, we
     only link a single copy of the section.  This is used to support
     g++.  g++ will emit each template expansion in its own section.
     The symbols will be defined as weak, so that multiple definitions
     are permitted.  The GNU linker extension is to actually discard
     all but one of the sections.  */
  if (CONST_STRNEQ (name, ".gnu.linkonce")
      && elf_next_in_group (newsect) == NULL)
    flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_section_flags)
    if (! bed->elf_backend_section_flags (&flags, hdr))
      return FALSE;

  if (! bfd_set_section_flags (abfd, newsect, flags))
    return FALSE;

  /* We do not parse the PT_NOTE segments as we are interested even in the
     separate debug info files which may have the segments offsets corrupted.
     PT_NOTEs from the core files are currently not parsed using BFD.  */
  if (hdr->sh_type == SHT_NOTE)
    {
      bfd_byte *contents;

      if (!bfd_malloc_and_get_section (abfd, newsect, &contents))
	return FALSE;

      elf_parse_notes (abfd, (char *) contents, hdr->sh_size, -1);
      free (contents);
    }

  if ((flags & SEC_ALLOC) != 0)
    {
      Elf_Internal_Phdr *phdr;
      unsigned int i, nload;

      /* Some ELF linkers produce binaries with all the program header
	 p_paddr fields zero.  If we have such a binary with more than
	 one PT_LOAD header, then leave the section lma equal to vma
	 so that we don't create sections with overlapping lma.  */
      phdr = elf_tdata (abfd)->phdr;
      for (nload = 0, i = 0; i < elf_elfheader (abfd)->e_phnum; i++, phdr++)
	if (phdr->p_paddr != 0)
	  break;
	else if (phdr->p_type == PT_LOAD && phdr->p_memsz != 0)
	  ++nload;
      if (i >= elf_elfheader (abfd)->e_phnum && nload > 1)
	return TRUE;

      phdr = elf_tdata (abfd)->phdr;
      for (i = 0; i < elf_elfheader (abfd)->e_phnum; i++, phdr++)
	{
	  if (((phdr->p_type == PT_LOAD
		&& (hdr->sh_flags & SHF_TLS) == 0)
	       || phdr->p_type == PT_TLS)
	      && ELF_SECTION_IN_SEGMENT (hdr, phdr))
	    {
	      if ((flags & SEC_LOAD) == 0)
		newsect->lma = (phdr->p_paddr
				+ hdr->sh_addr - phdr->p_vaddr);
	      else
		/* We used to use the same adjustment for SEC_LOAD
		   sections, but that doesn't work if the segment
		   is packed with code from multiple VMAs.
		   Instead we calculate the section LMA based on
		   the segment LMA.  It is assumed that the
		   segment will contain sections with contiguous
		   LMAs, even if the VMAs are not.  */
		newsect->lma = (phdr->p_paddr
				+ hdr->sh_offset - phdr->p_offset);

	      /* With contiguous segments, we can't tell from file
		 offsets whether a section with zero size should
		 be placed at the end of one segment or the
		 beginning of the next.  Decide based on vaddr.  */
	      if (hdr->sh_addr >= phdr->p_vaddr
		  && (hdr->sh_addr + hdr->sh_size
		      <= phdr->p_vaddr + phdr->p_memsz))
		break;
	    }
	}
    }

  /* Compress/decompress DWARF debug sections with names: .debug_* and
     .zdebug_*, after the section flags is set.  */
  if ((flags & SEC_DEBUGGING)
      && ((name[1] == 'd' && name[6] == '_')
	  || (name[1] == 'z' && name[7] == '_')))
    {
      enum { nothing, compress, decompress } action = nothing;
      char *new_name;

      if (bfd_is_section_compressed (abfd, newsect))
	{
	  /* Compressed section.  Check if we should decompress.  */
	  if ((abfd->flags & BFD_DECOMPRESS))
	    action = decompress;
	}
      else
	{
	  /* Normal section.  Check if we should compress.  */
	  if ((abfd->flags & BFD_COMPRESS) && newsect->size != 0)
	    action = compress;
	}

      new_name = NULL;
      switch (action)
	{
	case nothing:
	  break;
	case compress:
	  if (!bfd_init_section_compress_status (abfd, newsect))
	    {
	      (*_bfd_error_handler)
		(_("%B: unable to initialize compress status for section %s"),
		 abfd, name);
	      return FALSE;
	    }
	  if (name[1] != 'z')
	    {
	      unsigned int len = strlen (name);

	      new_name = bfd_alloc (abfd, len + 2);
	      if (new_name == NULL)
		return FALSE;
	      new_name[0] = '.';
	      new_name[1] = 'z';
	      memcpy (new_name + 2, name + 1, len);
	    }
	  break;
	case decompress:
	  if (!bfd_init_section_decompress_status (abfd, newsect))
	    {
	      (*_bfd_error_handler)
		(_("%B: unable to initialize decompress status for section %s"),
		 abfd, name);
	      return FALSE;
	    }
	  if (name[1] == 'z')
	    {
	      unsigned int len = strlen (name);

	      new_name = bfd_alloc (abfd, len);
	      if (new_name == NULL)
		return FALSE;
	      new_name[0] = '.';
	      memcpy (new_name + 1, name + 2, len - 1);
	    }
	  break;
	}
      if (new_name != NULL)
	bfd_rename_section (abfd, newsect, new_name);
    }

  return TRUE;
}

const char *const bfd_elf_section_type_names[] = {
  "SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB",
  "SHT_RELA", "SHT_HASH", "SHT_DYNAMIC", "SHT_NOTE",
  "SHT_NOBITS", "SHT_REL", "SHT_SHLIB", "SHT_DYNSYM",
};

/* ELF relocs are against symbols.  If we are producing relocatable
   output, and the reloc is against an external symbol, and nothing
   has given us any additional addend, the resulting reloc will also
   be against the same symbol.  In such a case, we don't want to
   change anything about the way the reloc is handled, since it will
   all be done at final link time.  Rather than put special case code
   into bfd_perform_relocation, all the reloc types use this howto
   function.  It just short circuits the reloc if producing
   relocatable output against an external symbol.  */

bfd_reloc_status_type
bfd_elf_generic_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		       arelent *reloc_entry,
		       asymbol *symbol,
		       void *data ATTRIBUTE_UNUSED,
		       asection *input_section,
		       bfd *output_bfd,
		       char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  return bfd_reloc_continue;
}

/* Copy the program header and other data from one object module to
   another.  */

bfd_boolean
_bfd_elf_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  if (!elf_flags_init (obfd))
    {
      elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
      elf_flags_init (obfd) = TRUE;
    }

  elf_gp (obfd) = elf_gp (ibfd);

  /* Also copy the EI_OSABI field.  */
  elf_elfheader (obfd)->e_ident[EI_OSABI] =
    elf_elfheader (ibfd)->e_ident[EI_OSABI];

  /* Copy object attributes.  */
  _bfd_elf_copy_obj_attributes (ibfd, obfd);
  return TRUE;
}

static const char *
get_segment_type (unsigned int p_type)
{
  const char *pt;
  switch (p_type)
    {
    case PT_NULL: pt = "NULL"; break;
    case PT_LOAD: pt = "LOAD"; break;
    case PT_DYNAMIC: pt = "DYNAMIC"; break;
    case PT_INTERP: pt = "INTERP"; break;
    case PT_NOTE: pt = "NOTE"; break;
    case PT_SHLIB: pt = "SHLIB"; break;
    case PT_PHDR: pt = "PHDR"; break;
    case PT_TLS: pt = "TLS"; break;
    case PT_GNU_EH_FRAME: pt = "EH_FRAME"; break;
    case PT_GNU_STACK: pt = "STACK"; break;
    case PT_GNU_RELRO: pt = "RELRO"; break;
    default: pt = NULL; break;
    }
  return pt;
}

/* Print out the program headers.  */

bfd_boolean
_bfd_elf_print_private_bfd_data (bfd *abfd, void *farg)
{
  FILE *f = (FILE *) farg;
  Elf_Internal_Phdr *p;
  asection *s;
  bfd_byte *dynbuf = NULL;

  p = elf_tdata (abfd)->phdr;
  if (p != NULL)
    {
      unsigned int i, c;

      fprintf (f, _("\nProgram Header:\n"));
      c = elf_elfheader (abfd)->e_phnum;
      for (i = 0; i < c; i++, p++)
	{
	  const char *pt = get_segment_type (p->p_type);
	  char buf[20];

	  if (pt == NULL)
	    {
	      sprintf (buf, "0x%lx", p->p_type);
	      pt = buf;
	    }
	  fprintf (f, "%8s off    0x", pt);
	  bfd_fprintf_vma (abfd, f, p->p_offset);
	  fprintf (f, " vaddr 0x");
	  bfd_fprintf_vma (abfd, f, p->p_vaddr);
	  fprintf (f, " paddr 0x");
	  bfd_fprintf_vma (abfd, f, p->p_paddr);
	  fprintf (f, " align 2**%u\n", bfd_log2 (p->p_align));
	  fprintf (f, "         filesz 0x");
	  bfd_fprintf_vma (abfd, f, p->p_filesz);
	  fprintf (f, " memsz 0x");
	  bfd_fprintf_vma (abfd, f, p->p_memsz);
	  fprintf (f, " flags %c%c%c",
		   (p->p_flags & PF_R) != 0 ? 'r' : '-',
		   (p->p_flags & PF_W) != 0 ? 'w' : '-',
		   (p->p_flags & PF_X) != 0 ? 'x' : '-');
	  if ((p->p_flags &~ (unsigned) (PF_R | PF_W | PF_X)) != 0)
	    fprintf (f, " %lx", p->p_flags &~ (unsigned) (PF_R | PF_W | PF_X));
	  fprintf (f, "\n");
	}
    }

  s = bfd_get_section_by_name (abfd, ".dynamic");
  if (s != NULL)
    {
      unsigned int elfsec;
      unsigned long shlink;
      bfd_byte *extdyn, *extdynend;
      size_t extdynsize;
      void (*swap_dyn_in) (bfd *, const void *, Elf_Internal_Dyn *);

      fprintf (f, _("\nDynamic Section:\n"));

      if (!bfd_malloc_and_get_section (abfd, s, &dynbuf))
	goto error_return;

      elfsec = _bfd_elf_section_from_bfd_section (abfd, s);
      if (elfsec == SHN_BAD)
	goto error_return;
      shlink = elf_elfsections (abfd)[elfsec]->sh_link;

      extdynsize = get_elf_backend_data (abfd)->s->sizeof_dyn;
      swap_dyn_in = get_elf_backend_data (abfd)->s->swap_dyn_in;

      extdyn = dynbuf;
      /* PR 17512: file: 6f427532.  */
      if (s->size < extdynsize)
	goto error_return;
      extdynend = extdyn + s->size;
      /* PR 17512: file: id:000006,sig:06,src:000000,op:flip4,pos:5664.
         Fix range check.  */
      for (; extdyn <= (extdynend - extdynsize); extdyn += extdynsize)
	{
	  Elf_Internal_Dyn dyn;
	  const char *name = "";
	  char ab[20];
	  bfd_boolean stringp;
	  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

	  (*swap_dyn_in) (abfd, extdyn, &dyn);

	  if (dyn.d_tag == DT_NULL)
	    break;

	  stringp = FALSE;
	  switch (dyn.d_tag)
	    {
	    default:
	      if (bed->elf_backend_get_target_dtag)
		name = (*bed->elf_backend_get_target_dtag) (dyn.d_tag);

	      if (!strcmp (name, ""))
		{
		  sprintf (ab, "0x%lx", (unsigned long) dyn.d_tag);
		  name = ab;
		}
	      break;

	    case DT_NEEDED: name = "NEEDED"; stringp = TRUE; break;
	    case DT_PLTRELSZ: name = "PLTRELSZ"; break;
	    case DT_PLTGOT: name = "PLTGOT"; break;
	    case DT_HASH: name = "HASH"; break;
	    case DT_STRTAB: name = "STRTAB"; break;
	    case DT_SYMTAB: name = "SYMTAB"; break;
	    case DT_RELA: name = "RELA"; break;
	    case DT_RELASZ: name = "RELASZ"; break;
	    case DT_RELAENT: name = "RELAENT"; break;
	    case DT_STRSZ: name = "STRSZ"; break;
	    case DT_SYMENT: name = "SYMENT"; break;
	    case DT_INIT: name = "INIT"; break;
	    case DT_FINI: name = "FINI"; break;
	    case DT_SONAME: name = "SONAME"; stringp = TRUE; break;
	    case DT_RPATH: name = "RPATH"; stringp = TRUE; break;
	    case DT_SYMBOLIC: name = "SYMBOLIC"; break;
	    case DT_REL: name = "REL"; break;
	    case DT_RELSZ: name = "RELSZ"; break;
	    case DT_RELENT: name = "RELENT"; break;
	    case DT_PLTREL: name = "PLTREL"; break;
	    case DT_DEBUG: name = "DEBUG"; break;
	    case DT_TEXTREL: name = "TEXTREL"; break;
	    case DT_JMPREL: name = "JMPREL"; break;
	    case DT_BIND_NOW: name = "BIND_NOW"; break;
	    case DT_INIT_ARRAY: name = "INIT_ARRAY"; break;
	    case DT_FINI_ARRAY: name = "FINI_ARRAY"; break;
	    case DT_INIT_ARRAYSZ: name = "INIT_ARRAYSZ"; break;
	    case DT_FINI_ARRAYSZ: name = "FINI_ARRAYSZ"; break;
	    case DT_RUNPATH: name = "RUNPATH"; stringp = TRUE; break;
	    case DT_FLAGS: name = "FLAGS"; break;
	    case DT_PREINIT_ARRAY: name = "PREINIT_ARRAY"; break;
	    case DT_PREINIT_ARRAYSZ: name = "PREINIT_ARRAYSZ"; break;
	    case DT_CHECKSUM: name = "CHECKSUM"; break;
	    case DT_PLTPADSZ: name = "PLTPADSZ"; break;
	    case DT_MOVEENT: name = "MOVEENT"; break;
	    case DT_MOVESZ: name = "MOVESZ"; break;
	    case DT_FEATURE: name = "FEATURE"; break;
	    case DT_POSFLAG_1: name = "POSFLAG_1"; break;
	    case DT_SYMINSZ: name = "SYMINSZ"; break;
	    case DT_SYMINENT: name = "SYMINENT"; break;
	    case DT_CONFIG: name = "CONFIG"; stringp = TRUE; break;
	    case DT_DEPAUDIT: name = "DEPAUDIT"; stringp = TRUE; break;
	    case DT_AUDIT: name = "AUDIT"; stringp = TRUE; break;
	    case DT_PLTPAD: name = "PLTPAD"; break;
	    case DT_MOVETAB: name = "MOVETAB"; break;
	    case DT_SYMINFO: name = "SYMINFO"; break;
	    case DT_RELACOUNT: name = "RELACOUNT"; break;
	    case DT_RELCOUNT: name = "RELCOUNT"; break;
	    case DT_FLAGS_1: name = "FLAGS_1"; break;
	    case DT_VERSYM: name = "VERSYM"; break;
	    case DT_VERDEF: name = "VERDEF"; break;
	    case DT_VERDEFNUM: name = "VERDEFNUM"; break;
	    case DT_VERNEED: name = "VERNEED"; break;
	    case DT_VERNEEDNUM: name = "VERNEEDNUM"; break;
	    case DT_AUXILIARY: name = "AUXILIARY"; stringp = TRUE; break;
	    case DT_USED: name = "USED"; break;
	    case DT_FILTER: name = "FILTER"; stringp = TRUE; break;
	    case DT_GNU_HASH: name = "GNU_HASH"; break;
	    }

	  fprintf (f, "  %-20s ", name);
	  if (! stringp)
	    {
	      fprintf (f, "0x");
	      bfd_fprintf_vma (abfd, f, dyn.d_un.d_val);
	    }
	  else
	    {
	      const char *string;
	      unsigned int tagv = dyn.d_un.d_val;

	      string = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
	      if (string == NULL)
		goto error_return;
	      fprintf (f, "%s", string);
	    }
	  fprintf (f, "\n");
	}

      free (dynbuf);
      dynbuf = NULL;
    }

  if ((elf_dynverdef (abfd) != 0 && elf_tdata (abfd)->verdef == NULL)
      || (elf_dynverref (abfd) != 0 && elf_tdata (abfd)->verref == NULL))
    {
      if (! _bfd_elf_slurp_version_tables (abfd, FALSE))
	return FALSE;
    }

  if (elf_dynverdef (abfd) != 0)
    {
      Elf_Internal_Verdef *t;

      fprintf (f, _("\nVersion definitions:\n"));
      for (t = elf_tdata (abfd)->verdef; t != NULL; t = t->vd_nextdef)
	{
	  fprintf (f, "%d 0x%2.2x 0x%8.8lx %s\n", t->vd_ndx,
		   t->vd_flags, t->vd_hash,
		   t->vd_nodename ? t->vd_nodename : "<corrupt>");
	  if (t->vd_auxptr != NULL && t->vd_auxptr->vda_nextptr != NULL)
	    {
	      Elf_Internal_Verdaux *a;

	      fprintf (f, "\t");
	      for (a = t->vd_auxptr->vda_nextptr;
		   a != NULL;
		   a = a->vda_nextptr)
		fprintf (f, "%s ",
			 a->vda_nodename ? a->vda_nodename : "<corrupt>");
	      fprintf (f, "\n");
	    }
	}
    }

  if (elf_dynverref (abfd) != 0)
    {
      Elf_Internal_Verneed *t;

      fprintf (f, _("\nVersion References:\n"));
      for (t = elf_tdata (abfd)->verref; t != NULL; t = t->vn_nextref)
	{
	  Elf_Internal_Vernaux *a;

	  fprintf (f, _("  required from %s:\n"),
		   t->vn_filename ? t->vn_filename : "<corrupt>");
	  for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
	    fprintf (f, "    0x%8.8lx 0x%2.2x %2.2d %s\n", a->vna_hash,
		     a->vna_flags, a->vna_other,
		     a->vna_nodename ? a->vna_nodename : "<corrupt>");
	}
    }

  return TRUE;

 error_return:
  if (dynbuf != NULL)
    free (dynbuf);
  return FALSE;
}

/* Display ELF-specific fields of a symbol.  */

void
bfd_elf_print_symbol (bfd *abfd,
		      void *filep,
		      asymbol *symbol,
		      bfd_print_symbol_type how)
{
  FILE *file = (FILE *) filep;
  switch (how)
    {
    case bfd_print_symbol_name:
      fprintf (file, "%s", symbol->name);
      break;
    case bfd_print_symbol_more:
      fprintf (file, "elf ");
      bfd_fprintf_vma (abfd, file, symbol->value);
      fprintf (file, " %lx", (unsigned long) symbol->flags);
      break;
    case bfd_print_symbol_all:
      {
	const char *section_name;
	const char *name = NULL;
	const struct elf_backend_data *bed;
	unsigned char st_other;
	bfd_vma val;

	section_name = symbol->section ? symbol->section->name : "(*none*)";

	bed = get_elf_backend_data (abfd);
	if (bed->elf_backend_print_symbol_all)
	  name = (*bed->elf_backend_print_symbol_all) (abfd, filep, symbol);

	if (name == NULL)
	  {
	    name = symbol->name;
	    bfd_print_symbol_vandf (abfd, file, symbol);
	  }

	fprintf (file, " %s\t", section_name);
	/* Print the "other" value for a symbol.  For common symbols,
	   we've already printed the size; now print the alignment.
	   For other symbols, we have no specified alignment, and
	   we've printed the address; now print the size.  */
	if (symbol->section && bfd_is_com_section (symbol->section))
	  val = ((elf_symbol_type *) symbol)->internal_elf_sym.st_value;
	else
	  val = ((elf_symbol_type *) symbol)->internal_elf_sym.st_size;
	bfd_fprintf_vma (abfd, file, val);

	/* If we have version information, print it.  */
	if (elf_dynversym (abfd) != 0
	    && (elf_dynverdef (abfd) != 0
		|| elf_dynverref (abfd) != 0))
	  {
	    unsigned int vernum;
	    const char *version_string;

	    vernum = ((elf_symbol_type *) symbol)->version & VERSYM_VERSION;

	    if (vernum == 0)
	      version_string = "";
	    else if (vernum == 1)
	      version_string = "Base";
	    else if (vernum <= elf_tdata (abfd)->cverdefs)
	      version_string =
		elf_tdata (abfd)->verdef[vernum - 1].vd_nodename;
	    else
	      {
		Elf_Internal_Verneed *t;

		version_string = "";
		for (t = elf_tdata (abfd)->verref;
		     t != NULL;
		     t = t->vn_nextref)
		  {
		    Elf_Internal_Vernaux *a;

		    for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
		      {
			if (a->vna_other == vernum)
			  {
			    version_string = a->vna_nodename;
			    break;
			  }
		      }
		  }
	      }

	    if ((((elf_symbol_type *) symbol)->version & VERSYM_HIDDEN) == 0)
	      fprintf (file, "  %-11s", version_string);
	    else
	      {
		int i;

		fprintf (file, " (%s)", version_string);
		for (i = 10 - strlen (version_string); i > 0; --i)
		  putc (' ', file);
	      }
	  }

	/* If the st_other field is not zero, print it.  */
	st_other = ((elf_symbol_type *) symbol)->internal_elf_sym.st_other;

	switch (st_other)
	  {
	  case 0: break;
	  case STV_INTERNAL:  fprintf (file, " .internal");  break;
	  case STV_HIDDEN:    fprintf (file, " .hidden");    break;
	  case STV_PROTECTED: fprintf (file, " .protected"); break;
	  default:
	    /* Some other non-defined flags are also present, so print
	       everything hex.  */
	    fprintf (file, " 0x%02x", (unsigned int) st_other);
	  }

	fprintf (file, " %s", name);
      }
      break;
    }
}

/* Allocate an ELF string table--force the first byte to be zero.  */

struct bfd_strtab_hash *
_bfd_elf_stringtab_init (void)
{
  struct bfd_strtab_hash *ret;

  ret = _bfd_stringtab_init ();
  if (ret != NULL)
    {
      bfd_size_type loc;

      loc = _bfd_stringtab_add (ret, "", TRUE, FALSE);
      BFD_ASSERT (loc == 0 || loc == (bfd_size_type) -1);
      if (loc == (bfd_size_type) -1)
	{
	  _bfd_stringtab_free (ret);
	  ret = NULL;
	}
    }
  return ret;
}

/* ELF .o/exec file reading */

/* Create a new bfd section from an ELF section header.  */

bfd_boolean
bfd_section_from_shdr (bfd *abfd, unsigned int shindex)
{
  Elf_Internal_Shdr *hdr;
  Elf_Internal_Ehdr *ehdr;
  const struct elf_backend_data *bed;
  const char *name;
  bfd_boolean ret = TRUE;
  static bfd_boolean * sections_being_created = NULL;
  static bfd * sections_being_created_abfd = NULL;
  static unsigned int nesting = 0;

  if (shindex >= elf_numsections (abfd))
    return FALSE;

  if (++ nesting > 3)
    {
      /* PR17512: A corrupt ELF binary might contain a recursive group of
	 sections, each with string indicies pointing to the next in the
	 loop.  Detect this here, by refusing to load a section that we are
	 already in the process of loading.  We only trigger this test if
	 we have nested at least three sections deep as normal ELF binaries
	 can expect to recurse at least once.

	 FIXME: It would be better if this array was attached to the bfd,
	 rather than being held in a static pointer.  */

      if (sections_being_created_abfd != abfd)
	sections_being_created = NULL;
      if (sections_being_created == NULL)
	{
	  /* FIXME: It would be more efficient to attach this array to the bfd somehow.  */
	  sections_being_created = (bfd_boolean *)
	    bfd_zalloc (abfd, elf_numsections (abfd) * sizeof (bfd_boolean));
	  sections_being_created_abfd = abfd;
	}
      if (sections_being_created [shindex])
	{
	  (*_bfd_error_handler)
	    (_("%B: warning: loop in section dependencies detected"), abfd);
	  return FALSE;
	}
      sections_being_created [shindex] = TRUE;
    }

  hdr = elf_elfsections (abfd)[shindex];
  ehdr = elf_elfheader (abfd);
  name = bfd_elf_string_from_elf_section (abfd, ehdr->e_shstrndx,
					  hdr->sh_name);
  if (name == NULL)
    goto fail;

  bed = get_elf_backend_data (abfd);
  switch (hdr->sh_type)
    {
    case SHT_NULL:
      /* Inactive section. Throw it away.  */
      goto success;

    case SHT_PROGBITS:		/* Normal section with contents.  */
    case SHT_NOBITS:		/* .bss section.  */
    case SHT_HASH:		/* .hash section.  */
    case SHT_NOTE:		/* .note section.  */
    case SHT_INIT_ARRAY:	/* .init_array section.  */
    case SHT_FINI_ARRAY:	/* .fini_array section.  */
    case SHT_PREINIT_ARRAY:	/* .preinit_array section.  */
    case SHT_GNU_LIBLIST:	/* .gnu.liblist section.  */
    case SHT_GNU_HASH:		/* .gnu.hash section.  */
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_DYNAMIC:	/* Dynamic linking information.  */
      if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	goto fail;

      if (hdr->sh_link > elf_numsections (abfd))
	{
	  /* PR 10478: Accept Solaris binaries with a sh_link
	     field set to SHN_BEFORE or SHN_AFTER.  */
	  switch (bfd_get_arch (abfd))
	    {
	    case bfd_arch_i386:
	    case bfd_arch_sparc:
	      if (hdr->sh_link == (SHN_LORESERVE & 0xffff) /* SHN_BEFORE */
		  || hdr->sh_link == ((SHN_LORESERVE + 1) & 0xffff) /* SHN_AFTER */)
		break;
	      /* Otherwise fall through.  */
	    default:
	      goto fail;
	    }
	}
      else if (elf_elfsections (abfd)[hdr->sh_link] == NULL)
	goto fail;
      else if (elf_elfsections (abfd)[hdr->sh_link]->sh_type != SHT_STRTAB)
	{
	  Elf_Internal_Shdr *dynsymhdr;

	  /* The shared libraries distributed with hpux11 have a bogus
	     sh_link field for the ".dynamic" section.  Find the
	     string table for the ".dynsym" section instead.  */
	  if (elf_dynsymtab (abfd) != 0)
	    {
	      dynsymhdr = elf_elfsections (abfd)[elf_dynsymtab (abfd)];
	      hdr->sh_link = dynsymhdr->sh_link;
	    }
	  else
	    {
	      unsigned int i, num_sec;

	      num_sec = elf_numsections (abfd);
	      for (i = 1; i < num_sec; i++)
		{
		  dynsymhdr = elf_elfsections (abfd)[i];
		  if (dynsymhdr->sh_type == SHT_DYNSYM)
		    {
		      hdr->sh_link = dynsymhdr->sh_link;
		      break;
		    }
		}
	    }
	}
      goto success;

    case SHT_SYMTAB:		/* A symbol table.  */
      if (elf_onesymtab (abfd) == shindex)
	goto success;

      if (hdr->sh_entsize != bed->s->sizeof_sym)
	goto fail;

      if (hdr->sh_info * hdr->sh_entsize > hdr->sh_size)
	{
	  if (hdr->sh_size != 0)
	    goto fail;
	  /* Some assemblers erroneously set sh_info to one with a
	     zero sh_size.  ld sees this as a global symbol count
	     of (unsigned) -1.  Fix it here.  */
	  hdr->sh_info = 0;
	  goto success;
	}

      BFD_ASSERT (elf_onesymtab (abfd) == 0);
      elf_onesymtab (abfd) = shindex;
      elf_tdata (abfd)->symtab_hdr = *hdr;
      elf_elfsections (abfd)[shindex] = hdr = &elf_tdata (abfd)->symtab_hdr;
      abfd->flags |= HAS_SYMS;

      /* Sometimes a shared object will map in the symbol table.  If
	 SHF_ALLOC is set, and this is a shared object, then we also
	 treat this section as a BFD section.  We can not base the
	 decision purely on SHF_ALLOC, because that flag is sometimes
	 set in a relocatable object file, which would confuse the
	 linker.  */
      if ((hdr->sh_flags & SHF_ALLOC) != 0
	  && (abfd->flags & DYNAMIC) != 0
	  && ! _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						shindex))
	goto fail;

      /* Go looking for SHT_SYMTAB_SHNDX too, since if there is one we
	 can't read symbols without that section loaded as well.  It
	 is most likely specified by the next section header.  */
      if (elf_elfsections (abfd)[elf_symtab_shndx (abfd)]->sh_link != shindex)
	{
	  unsigned int i, num_sec;

	  num_sec = elf_numsections (abfd);
	  for (i = shindex + 1; i < num_sec; i++)
	    {
	      Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];
	      if (hdr2->sh_type == SHT_SYMTAB_SHNDX
		  && hdr2->sh_link == shindex)
		break;
	    }
	  if (i == num_sec)
	    for (i = 1; i < shindex; i++)
	      {
		Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];
		if (hdr2->sh_type == SHT_SYMTAB_SHNDX
		    && hdr2->sh_link == shindex)
		  break;
	      }
	  if (i != shindex)
	    ret = bfd_section_from_shdr (abfd, i);
	}
      goto success;

    case SHT_DYNSYM:		/* A dynamic symbol table.  */
      if (elf_dynsymtab (abfd) == shindex)
	goto success;

      if (hdr->sh_entsize != bed->s->sizeof_sym)
	goto fail;

      if (hdr->sh_info * hdr->sh_entsize > hdr->sh_size)
	{
	  if (hdr->sh_size != 0)
	    goto fail;

	  /* Some linkers erroneously set sh_info to one with a
	     zero sh_size.  ld sees this as a global symbol count
	     of (unsigned) -1.  Fix it here.  */
	  hdr->sh_info = 0;
	  goto success;
	}

      BFD_ASSERT (elf_dynsymtab (abfd) == 0);
      elf_dynsymtab (abfd) = shindex;
      elf_tdata (abfd)->dynsymtab_hdr = *hdr;
      elf_elfsections (abfd)[shindex] = hdr = &elf_tdata (abfd)->dynsymtab_hdr;
      abfd->flags |= HAS_SYMS;

      /* Besides being a symbol table, we also treat this as a regular
	 section, so that objcopy can handle it.  */
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_SYMTAB_SHNDX:	/* Symbol section indices when >64k sections.  */
      if (elf_symtab_shndx (abfd) == shindex)
	goto success;

      BFD_ASSERT (elf_symtab_shndx (abfd) == 0);
      elf_symtab_shndx (abfd) = shindex;
      elf_tdata (abfd)->symtab_shndx_hdr = *hdr;
      elf_elfsections (abfd)[shindex] = &elf_tdata (abfd)->symtab_shndx_hdr;
      goto success;

    case SHT_STRTAB:		/* A string table.  */
      if (hdr->bfd_section != NULL)
	goto success;

      if (ehdr->e_shstrndx == shindex)
	{
	  elf_tdata (abfd)->shstrtab_hdr = *hdr;
	  elf_elfsections (abfd)[shindex] = &elf_tdata (abfd)->shstrtab_hdr;
	  goto success;
	}

      if (elf_elfsections (abfd)[elf_onesymtab (abfd)]->sh_link == shindex)
	{
	symtab_strtab:
	  elf_tdata (abfd)->strtab_hdr = *hdr;
	  elf_elfsections (abfd)[shindex] = &elf_tdata (abfd)->strtab_hdr;
	  goto success;
	}

      if (elf_elfsections (abfd)[elf_dynsymtab (abfd)]->sh_link == shindex)
	{
	dynsymtab_strtab:
	  elf_tdata (abfd)->dynstrtab_hdr = *hdr;
	  hdr = &elf_tdata (abfd)->dynstrtab_hdr;
	  elf_elfsections (abfd)[shindex] = hdr;
	  /* We also treat this as a regular section, so that objcopy
	     can handle it.  */
	  ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						 shindex);
	  goto success;
	}

      /* If the string table isn't one of the above, then treat it as a
	 regular section.  We need to scan all the headers to be sure,
	 just in case this strtab section appeared before the above.  */
      if (elf_onesymtab (abfd) == 0 || elf_dynsymtab (abfd) == 0)
	{
	  unsigned int i, num_sec;

	  num_sec = elf_numsections (abfd);
	  for (i = 1; i < num_sec; i++)
	    {
	      Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];
	      if (hdr2->sh_link == shindex)
		{
		  /* Prevent endless recursion on broken objects.  */
		  if (i == shindex)
		    goto fail;
		  if (! bfd_section_from_shdr (abfd, i))
		    goto fail;
		  if (elf_onesymtab (abfd) == i)
		    goto symtab_strtab;
		  if (elf_dynsymtab (abfd) == i)
		    goto dynsymtab_strtab;
		}
	    }
	}
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_REL:
    case SHT_RELA:
      /* *These* do a lot of work -- but build no sections!  */
      {
	asection *target_sect;
	Elf_Internal_Shdr *hdr2, **p_hdr;
	unsigned int num_sec = elf_numsections (abfd);
	struct bfd_elf_section_data *esdt;
	bfd_size_type amt;

	if (hdr->sh_entsize
	    != (bfd_size_type) (hdr->sh_type == SHT_REL
				? bed->s->sizeof_rel : bed->s->sizeof_rela))
	  goto fail;

	/* Check for a bogus link to avoid crashing.  */
	if (hdr->sh_link >= num_sec)
	  {
	    ((*_bfd_error_handler)
	     (_("%B: invalid link %lu for reloc section %s (index %u)"),
	      abfd, hdr->sh_link, name, shindex));
	    ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						   shindex);
	    goto success;
	  }

	/* For some incomprehensible reason Oracle distributes
	   libraries for Solaris in which some of the objects have
	   bogus sh_link fields.  It would be nice if we could just
	   reject them, but, unfortunately, some people need to use
	   them.  We scan through the section headers; if we find only
	   one suitable symbol table, we clobber the sh_link to point
	   to it.  I hope this doesn't break anything.

	   Don't do it on executable nor shared library.  */
	if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0
	    && elf_elfsections (abfd)[hdr->sh_link]->sh_type != SHT_SYMTAB
	    && elf_elfsections (abfd)[hdr->sh_link]->sh_type != SHT_DYNSYM)
	  {
	    unsigned int scan;
	    int found;

	    found = 0;
	    for (scan = 1; scan < num_sec; scan++)
	      {
		if (elf_elfsections (abfd)[scan]->sh_type == SHT_SYMTAB
		    || elf_elfsections (abfd)[scan]->sh_type == SHT_DYNSYM)
		  {
		    if (found != 0)
		      {
			found = 0;
			break;
		      }
		    found = scan;
		  }
	      }
	    if (found != 0)
	      hdr->sh_link = found;
	  }

	/* Get the symbol table.  */
	if ((elf_elfsections (abfd)[hdr->sh_link]->sh_type == SHT_SYMTAB
	     || elf_elfsections (abfd)[hdr->sh_link]->sh_type == SHT_DYNSYM)
	    && ! bfd_section_from_shdr (abfd, hdr->sh_link))
	  goto fail;

	/* If this reloc section does not use the main symbol table we
	   don't treat it as a reloc section.  BFD can't adequately
	   represent such a section, so at least for now, we don't
	   try.  We just present it as a normal section.  We also
	   can't use it as a reloc section if it points to the null
	   section, an invalid section, another reloc section, or its
	   sh_link points to the null section.  */
	if (hdr->sh_link != elf_onesymtab (abfd)
	    || hdr->sh_link == SHN_UNDEF
	    || hdr->sh_info == SHN_UNDEF
	    || hdr->sh_info >= num_sec
	    || elf_elfsections (abfd)[hdr->sh_info]->sh_type == SHT_REL
	    || elf_elfsections (abfd)[hdr->sh_info]->sh_type == SHT_RELA)
	  {
	    ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						   shindex);
	    goto success;
	  }

	if (! bfd_section_from_shdr (abfd, hdr->sh_info))
	  goto fail;

	target_sect = bfd_section_from_elf_index (abfd, hdr->sh_info);
	if (target_sect == NULL)
	  goto fail;

	esdt = elf_section_data (target_sect);
	if (hdr->sh_type == SHT_RELA)
	  p_hdr = &esdt->rela.hdr;
	else
	  p_hdr = &esdt->rel.hdr;

	/* PR 17512: file: 0b4f81b7.  */
	if (*p_hdr != NULL)
	  goto fail;
	amt = sizeof (*hdr2);
	hdr2 = (Elf_Internal_Shdr *) bfd_alloc (abfd, amt);
	if (hdr2 == NULL)
	  goto fail;
	*hdr2 = *hdr;
	*p_hdr = hdr2;
	elf_elfsections (abfd)[shindex] = hdr2;
	target_sect->reloc_count += NUM_SHDR_ENTRIES (hdr);
	target_sect->flags |= SEC_RELOC;
	target_sect->relocation = NULL;
	target_sect->rel_filepos = hdr->sh_offset;
	/* In the section to which the relocations apply, mark whether
	   its relocations are of the REL or RELA variety.  */
	if (hdr->sh_size != 0)
	  {
	    if (hdr->sh_type == SHT_RELA)
	      target_sect->use_rela_p = 1;
	  }
	abfd->flags |= HAS_RELOC;
	goto success;
      }

    case SHT_GNU_verdef:
      elf_dynverdef (abfd) = shindex;
      elf_tdata (abfd)->dynverdef_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_GNU_versym:
      if (hdr->sh_entsize != sizeof (Elf_External_Versym))
	goto fail;

      elf_dynversym (abfd) = shindex;
      elf_tdata (abfd)->dynversym_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_GNU_verneed:
      elf_dynverref (abfd) = shindex;
      elf_tdata (abfd)->dynverref_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_SHLIB:
      goto success;

    case SHT_GROUP:
      if (! IS_VALID_GROUP_SECTION_HEADER (hdr, GRP_ENTRY_SIZE))
	goto fail;

      if (!_bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	goto fail;

      if (hdr->contents != NULL)
	{
	  Elf_Internal_Group *idx = (Elf_Internal_Group *) hdr->contents;
	  unsigned int n_elt = hdr->sh_size / sizeof (* idx);
	  asection *s;

	  if (n_elt == 0)
	    goto fail;
	  if (idx->flags & GRP_COMDAT)
	    hdr->bfd_section->flags
	      |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;

	  /* We try to keep the same section order as it comes in.  */
	  idx += n_elt;
	  while (--n_elt != 0)
	    {
	      --idx;

	      if (idx->shdr != NULL
		  && (s = idx->shdr->bfd_section) != NULL
		  && elf_next_in_group (s) != NULL)
		{
		  elf_next_in_group (hdr->bfd_section) = s;
		  break;
		}
	    }
	}
      goto success;

    default:
      /* Possibly an attributes section.  */
      if (hdr->sh_type == SHT_GNU_ATTRIBUTES
	  || hdr->sh_type == bed->obj_attrs_section_type)
	{
	  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	    goto fail;
	  _bfd_elf_parse_attributes (abfd, hdr);
	  goto success;
	}

      /* Check for any processor-specific section types.  */
      if (bed->elf_backend_section_from_shdr (abfd, hdr, name, shindex))
	goto success;

      if (hdr->sh_type >= SHT_LOUSER && hdr->sh_type <= SHT_HIUSER)
	{
	  if ((hdr->sh_flags & SHF_ALLOC) != 0)
	    /* FIXME: How to properly handle allocated section reserved
	       for applications?  */
	    (*_bfd_error_handler)
	      (_("%B: don't know how to handle allocated, application "
		 "specific section `%s' [0x%8x]"),
	       abfd, name, hdr->sh_type);
	  else
	    {
	      /* Allow sections reserved for applications.  */
	      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						     shindex);
	      goto success;
	    }
	}
      else if (hdr->sh_type >= SHT_LOPROC
	       && hdr->sh_type <= SHT_HIPROC)
	/* FIXME: We should handle this section.  */
	(*_bfd_error_handler)
	  (_("%B: don't know how to handle processor specific section "
	     "`%s' [0x%8x]"),
	   abfd, name, hdr->sh_type);
      else if (hdr->sh_type >= SHT_LOOS && hdr->sh_type <= SHT_HIOS)
	{
	  /* Unrecognised OS-specific sections.  */
	  if ((hdr->sh_flags & SHF_OS_NONCONFORMING) != 0)
	    /* SHF_OS_NONCONFORMING indicates that special knowledge is
	       required to correctly process the section and the file should
	       be rejected with an error message.  */
	    (*_bfd_error_handler)
	      (_("%B: don't know how to handle OS specific section "
		 "`%s' [0x%8x]"),
	       abfd, name, hdr->sh_type);
	  else
	    {
	      /* Otherwise it should be processed.  */
	      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
	      goto success;
	    }
	}
      else
	/* FIXME: We should handle this section.  */
	(*_bfd_error_handler)
	  (_("%B: don't know how to handle section `%s' [0x%8x]"),
	   abfd, name, hdr->sh_type);

      goto fail;
    }

 fail:
  ret = FALSE;
 success:
  if (sections_being_created && sections_being_created_abfd == abfd)
    sections_being_created [shindex] = FALSE;
  if (-- nesting == 0)
    {
      sections_being_created = NULL;
      sections_being_created_abfd = abfd;
    }
  return ret;
}

/* Return the local symbol specified by ABFD, R_SYMNDX.  */

Elf_Internal_Sym *
bfd_sym_from_r_symndx (struct sym_cache *cache,
		       bfd *abfd,
		       unsigned long r_symndx)
{
  unsigned int ent = r_symndx % LOCAL_SYM_CACHE_SIZE;

  if (cache->abfd != abfd || cache->indx[ent] != r_symndx)
    {
      Elf_Internal_Shdr *symtab_hdr;
      unsigned char esym[sizeof (Elf64_External_Sym)];
      Elf_External_Sym_Shndx eshndx;

      symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
      if (bfd_elf_get_elf_syms (abfd, symtab_hdr, 1, r_symndx,
				&cache->sym[ent], esym, &eshndx) == NULL)
	return NULL;

      if (cache->abfd != abfd)
	{
	  memset (cache->indx, -1, sizeof (cache->indx));
	  cache->abfd = abfd;
	}
      cache->indx[ent] = r_symndx;
    }

  return &cache->sym[ent];
}

/* Given an ELF section number, retrieve the corresponding BFD
   section.  */

asection *
bfd_section_from_elf_index (bfd *abfd, unsigned int sec_index)
{
  if (sec_index >= elf_numsections (abfd))
    return NULL;
  return elf_elfsections (abfd)[sec_index]->bfd_section;
}

static const struct bfd_elf_special_section special_sections_b[] =
{
  { STRING_COMMA_LEN (".bss"), -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { NULL,                   0,  0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_c[] =
{
  { STRING_COMMA_LEN (".comment"), 0, SHT_PROGBITS, 0 },
  { NULL,                       0, 0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_d[] =
{
  { STRING_COMMA_LEN (".data"),         -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".data1"),         0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  /* There are more DWARF sections than these, but they needn't be added here
     unless you have to cope with broken compilers that don't emit section
     attributes or you want to help the user writing assembler.  */
  { STRING_COMMA_LEN (".debug"),         0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_line"),    0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_info"),    0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_abbrev"),  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_aranges"), 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".dynamic"),       0, SHT_DYNAMIC,  SHF_ALLOC },
  { STRING_COMMA_LEN (".dynstr"),        0, SHT_STRTAB,   SHF_ALLOC },
  { STRING_COMMA_LEN (".dynsym"),        0, SHT_DYNSYM,   SHF_ALLOC },
  { NULL,                      0,        0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_f[] =
{
  { STRING_COMMA_LEN (".fini"),       0, SHT_PROGBITS,   SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".fini_array"), 0, SHT_FINI_ARRAY, SHF_ALLOC + SHF_WRITE },
  { NULL,                          0, 0, 0,              0 }
};

static const struct bfd_elf_special_section special_sections_g[] =
{
  { STRING_COMMA_LEN (".gnu.linkonce.b"), -2, SHT_NOBITS,      SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.lto_"),       -1, SHT_PROGBITS,    SHF_EXCLUDE },
  { STRING_COMMA_LEN (".got"),             0, SHT_PROGBITS,    SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.version"),     0, SHT_GNU_versym,  0 },
  { STRING_COMMA_LEN (".gnu.version_d"),   0, SHT_GNU_verdef,  0 },
  { STRING_COMMA_LEN (".gnu.version_r"),   0, SHT_GNU_verneed, 0 },
  { STRING_COMMA_LEN (".gnu.liblist"),     0, SHT_GNU_LIBLIST, SHF_ALLOC },
  { STRING_COMMA_LEN (".gnu.conflict"),    0, SHT_RELA,        SHF_ALLOC },
  { STRING_COMMA_LEN (".gnu.hash"),        0, SHT_GNU_HASH,    SHF_ALLOC },
  { NULL,                        0,        0, 0,               0 }
};

static const struct bfd_elf_special_section special_sections_h[] =
{
  { STRING_COMMA_LEN (".hash"), 0, SHT_HASH,     SHF_ALLOC },
  { NULL,                    0, 0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_i[] =
{
  { STRING_COMMA_LEN (".init"),       0, SHT_PROGBITS,   SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".init_array"), 0, SHT_INIT_ARRAY, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".interp"),     0, SHT_PROGBITS,   0 },
  { NULL,                      0,     0, 0,              0 }
};

static const struct bfd_elf_special_section special_sections_l[] =
{
  { STRING_COMMA_LEN (".line"), 0, SHT_PROGBITS, 0 },
  { NULL,                    0, 0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_n[] =
{
  { STRING_COMMA_LEN (".note.GNU-stack"), 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".note"),          -1, SHT_NOTE,     0 },
  { NULL,                    0,           0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_p[] =
{
  { STRING_COMMA_LEN (".preinit_array"), 0, SHT_PREINIT_ARRAY, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".plt"),           0, SHT_PROGBITS,      SHF_ALLOC + SHF_EXECINSTR },
  { NULL,                   0,           0, 0,                 0 }
};

static const struct bfd_elf_special_section special_sections_r[] =
{
  { STRING_COMMA_LEN (".rodata"), -2, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".rodata1"), 0, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".rela"),   -1, SHT_RELA,     0 },
  { STRING_COMMA_LEN (".rel"),    -1, SHT_REL,      0 },
  { NULL,                   0,     0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_s[] =
{
  { STRING_COMMA_LEN (".shstrtab"), 0, SHT_STRTAB, 0 },
  { STRING_COMMA_LEN (".strtab"),   0, SHT_STRTAB, 0 },
  { STRING_COMMA_LEN (".symtab"),   0, SHT_SYMTAB, 0 },
  /* See struct bfd_elf_special_section declaration for the semantics of
     this special case where .prefix_length != strlen (.prefix).  */
  { ".stabstr",			5,  3, SHT_STRTAB, 0 },
  { NULL,                       0,  0, 0,          0 }
};

static const struct bfd_elf_special_section special_sections_t[] =
{
  { STRING_COMMA_LEN (".text"),  -2, SHT_PROGBITS, SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".tbss"),  -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_TLS },
  { STRING_COMMA_LEN (".tdata"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_TLS },
  { NULL,                     0,  0, 0,            0 }
};

static const struct bfd_elf_special_section special_sections_z[] =
{
  { STRING_COMMA_LEN (".zdebug_line"),    0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_info"),    0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_abbrev"),  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_aranges"), 0, SHT_PROGBITS, 0 },
  { NULL,                     0,  0, 0,            0 }
};

static const struct bfd_elf_special_section * const special_sections[] =
{
  special_sections_b,		/* 'b' */
  special_sections_c,		/* 'c' */
  special_sections_d,		/* 'd' */
  NULL,				/* 'e' */
  special_sections_f,		/* 'f' */
  special_sections_g,		/* 'g' */
  special_sections_h,		/* 'h' */
  special_sections_i,		/* 'i' */
  NULL,				/* 'j' */
  NULL,				/* 'k' */
  special_sections_l,		/* 'l' */
  NULL,				/* 'm' */
  special_sections_n,		/* 'n' */
  NULL,				/* 'o' */
  special_sections_p,		/* 'p' */
  NULL,				/* 'q' */
  special_sections_r,		/* 'r' */
  special_sections_s,		/* 's' */
  special_sections_t,		/* 't' */
  NULL,				/* 'u' */
  NULL,				/* 'v' */
  NULL,				/* 'w' */
  NULL,				/* 'x' */
  NULL,				/* 'y' */
  special_sections_z		/* 'z' */
};

const struct bfd_elf_special_section *
_bfd_elf_get_special_section (const char *name,
			      const struct bfd_elf_special_section *spec,
			      unsigned int rela)
{
  int i;
  int len;

  len = strlen (name);

  for (i = 0; spec[i].prefix != NULL; i++)
    {
      int suffix_len;
      int prefix_len = spec[i].prefix_length;

      if (len < prefix_len)
	continue;
      if (memcmp (name, spec[i].prefix, prefix_len) != 0)
	continue;

      suffix_len = spec[i].suffix_length;
      if (suffix_len <= 0)
	{
	  if (name[prefix_len] != 0)
	    {
	      if (suffix_len == 0)
		continue;
	      if (name[prefix_len] != '.'
		  && (suffix_len == -2
		      || (rela && spec[i].type == SHT_REL)))
		continue;
	    }
	}
      else
	{
	  if (len < prefix_len + suffix_len)
	    continue;
	  if (memcmp (name + len - suffix_len,
		      spec[i].prefix + prefix_len,
		      suffix_len) != 0)
	    continue;
	}
      return &spec[i];
    }

  return NULL;
}

const struct bfd_elf_special_section *
_bfd_elf_get_sec_type_attr (bfd *abfd, asection *sec)
{
  int i;
  const struct bfd_elf_special_section *spec;
  const struct elf_backend_data *bed;

  /* See if this is one of the special sections.  */
  if (sec->name == NULL)
    return NULL;

  bed = get_elf_backend_data (abfd);
  spec = bed->special_sections;
  if (spec)
    {
      spec = _bfd_elf_get_special_section (sec->name,
					   bed->special_sections,
					   sec->use_rela_p);
      if (spec != NULL)
	return spec;
    }

  if (sec->name[0] != '.')
    return NULL;

  i = sec->name[1] - 'b';
  if (i < 0 || i > 'z' - 'b')
    return NULL;

  spec = special_sections[i];

  if (spec == NULL)
    return NULL;

  return _bfd_elf_get_special_section (sec->name, spec, sec->use_rela_p);
}

bfd_boolean
_bfd_elf_new_section_hook (bfd *abfd, asection *sec)
{
  struct bfd_elf_section_data *sdata;
  const struct elf_backend_data *bed;
  const struct bfd_elf_special_section *ssect;

  sdata = (struct bfd_elf_section_data *) sec->used_by_bfd;
  if (sdata == NULL)
    {
      sdata = (struct bfd_elf_section_data *) bfd_zalloc (abfd,
                                                          sizeof (*sdata));
      if (sdata == NULL)
	return FALSE;
      sec->used_by_bfd = sdata;
    }

  /* Indicate whether or not this section should use RELA relocations.  */
  bed = get_elf_backend_data (abfd);
  sec->use_rela_p = bed->default_use_rela_p;

  /* When we read a file, we don't need to set ELF section type and
     flags.  They will be overridden in _bfd_elf_make_section_from_shdr
     anyway.  We will set ELF section type and flags for all linker
     created sections.  If user specifies BFD section flags, we will
     set ELF section type and flags based on BFD section flags in
     elf_fake_sections.  Special handling for .init_array/.fini_array
     output sections since they may contain .ctors/.dtors input
     sections.  We don't want _bfd_elf_init_private_section_data to
     copy ELF section type from .ctors/.dtors input sections.  */
  if (abfd->direction != read_direction
      || (sec->flags & SEC_LINKER_CREATED) != 0)
    {
      ssect = (*bed->get_sec_type_attr) (abfd, sec);
      if (ssect != NULL
	  && (!sec->flags
	      || (sec->flags & SEC_LINKER_CREATED) != 0
	      || ssect->type == SHT_INIT_ARRAY
	      || ssect->type == SHT_FINI_ARRAY))
	{
	  elf_section_type (sec) = ssect->type;
	  elf_section_flags (sec) = ssect->attr;
	}
    }

  return _bfd_generic_new_section_hook (abfd, sec);
}

/* Create a new bfd section from an ELF program header.

   Since program segments have no names, we generate a synthetic name
   of the form segment<NUM>, where NUM is generally the index in the
   program header table.  For segments that are split (see below) we
   generate the names segment<NUM>a and segment<NUM>b.

   Note that some program segments may have a file size that is different than
   (less than) the memory size.  All this means is that at execution the
   system must allocate the amount of memory specified by the memory size,
   but only initialize it with the first "file size" bytes read from the
   file.  This would occur for example, with program segments consisting
   of combined data+bss.

   To handle the above situation, this routine generates TWO bfd sections
   for the single program segment.  The first has the length specified by
   the file size of the segment, and the second has the length specified
   by the difference between the two sizes.  In effect, the segment is split
   into its initialized and uninitialized parts.

 */

bfd_boolean
_bfd_elf_make_section_from_phdr (bfd *abfd,
				 Elf_Internal_Phdr *hdr,
				 int hdr_index,
				 const char *type_name)
{
  asection *newsect;
  char *name;
  char namebuf[64];
  size_t len;
  int split;

  split = ((hdr->p_memsz > 0)
	    && (hdr->p_filesz > 0)
	    && (hdr->p_memsz > hdr->p_filesz));

  if (hdr->p_filesz > 0)
    {
      sprintf (namebuf, "%s%d%s", type_name, hdr_index, split ? "a" : "");
      len = strlen (namebuf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (!name)
	return FALSE;
      memcpy (name, namebuf, len);
      newsect = bfd_make_section (abfd, name);
      if (newsect == NULL)
	return FALSE;
      newsect->vma = hdr->p_vaddr;
      newsect->lma = hdr->p_paddr;
      newsect->size = hdr->p_filesz;
      newsect->filepos = hdr->p_offset;
      newsect->flags |= SEC_HAS_CONTENTS;
      newsect->alignment_power = bfd_log2 (hdr->p_align);
      if (hdr->p_type == PT_LOAD)
	{
	  newsect->flags |= SEC_ALLOC;
	  newsect->flags |= SEC_LOAD;
	  if (hdr->p_flags & PF_X)
	    {
	      /* FIXME: all we known is that it has execute PERMISSION,
		 may be data.  */
	      newsect->flags |= SEC_CODE;
	    }
	}
      if (!(hdr->p_flags & PF_W))
	{
	  newsect->flags |= SEC_READONLY;
	}
    }

  if (hdr->p_memsz > hdr->p_filesz)
    {
      bfd_vma align;

      sprintf (namebuf, "%s%d%s", type_name, hdr_index, split ? "b" : "");
      len = strlen (namebuf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (!name)
	return FALSE;
      memcpy (name, namebuf, len);
      newsect = bfd_make_section (abfd, name);
      if (newsect == NULL)
	return FALSE;
      newsect->vma = hdr->p_vaddr + hdr->p_filesz;
      newsect->lma = hdr->p_paddr + hdr->p_filesz;
      newsect->size = hdr->p_memsz - hdr->p_filesz;
      newsect->filepos = hdr->p_offset + hdr->p_filesz;
      align = newsect->vma & -newsect->vma;
      if (align == 0 || align > hdr->p_align)
	align = hdr->p_align;
      newsect->alignment_power = bfd_log2 (align);
      if (hdr->p_type == PT_LOAD)
	{
	  /* Hack for gdb.  Segments that have not been modified do
	     not have their contents written to a core file, on the
	     assumption that a debugger can find the contents in the
	     executable.  We flag this case by setting the fake
	     section size to zero.  Note that "real" bss sections will
	     always have their contents dumped to the core file.  */
	  if (bfd_get_format (abfd) == bfd_core)
	    newsect->size = 0;
	  newsect->flags |= SEC_ALLOC;
	  if (hdr->p_flags & PF_X)
	    newsect->flags |= SEC_CODE;
	}
      if (!(hdr->p_flags & PF_W))
	newsect->flags |= SEC_READONLY;
    }

  return TRUE;
}

bfd_boolean
bfd_section_from_phdr (bfd *abfd, Elf_Internal_Phdr *hdr, int hdr_index)
{
  const struct elf_backend_data *bed;

  switch (hdr->p_type)
    {
    case PT_NULL:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "null");

    case PT_LOAD:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "load");

    case PT_DYNAMIC:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "dynamic");

    case PT_INTERP:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "interp");

    case PT_NOTE:
      if (! _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "note"))
	return FALSE;
      if (! elf_read_notes (abfd, hdr->p_offset, hdr->p_filesz))
	return FALSE;
      return TRUE;

    case PT_SHLIB:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "shlib");

    case PT_PHDR:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "phdr");

    case PT_GNU_EH_FRAME:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index,
					      "eh_frame_hdr");

    case PT_GNU_STACK:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "stack");

    case PT_GNU_RELRO:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "relro");

    default:
      /* Check for any processor-specific program segment types.  */
      bed = get_elf_backend_data (abfd);
      return bed->elf_backend_section_from_phdr (abfd, hdr, hdr_index, "proc");
    }
}

/* Return the REL_HDR for SEC, assuming there is only a single one, either
   REL or RELA.  */

Elf_Internal_Shdr *
_bfd_elf_single_rel_hdr (asection *sec)
{
  if (elf_section_data (sec)->rel.hdr)
    {
      BFD_ASSERT (elf_section_data (sec)->rela.hdr == NULL);
      return elf_section_data (sec)->rel.hdr;
    }
  else
    return elf_section_data (sec)->rela.hdr;
}

/* Allocate and initialize a section-header for a new reloc section,
   containing relocations against ASECT.  It is stored in RELDATA.  If
   USE_RELA_P is TRUE, we use RELA relocations; otherwise, we use REL
   relocations.  */

static bfd_boolean
_bfd_elf_init_reloc_shdr (bfd *abfd,
			  struct bfd_elf_section_reloc_data *reldata,
			  asection *asect,
			  bfd_boolean use_rela_p)
{
  Elf_Internal_Shdr *rel_hdr;
  char *name;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_size_type amt;

  amt = sizeof (Elf_Internal_Shdr);
  BFD_ASSERT (reldata->hdr == NULL);
  rel_hdr = bfd_zalloc (abfd, amt);
  reldata->hdr = rel_hdr;

  amt = sizeof ".rela" + strlen (asect->name);
  name = (char *) bfd_alloc (abfd, amt);
  if (name == NULL)
    return FALSE;
  sprintf (name, "%s%s", use_rela_p ? ".rela" : ".rel", asect->name);
  rel_hdr->sh_name =
    (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd), name,
					FALSE);
  if (rel_hdr->sh_name == (unsigned int) -1)
    return FALSE;
  rel_hdr->sh_type = use_rela_p ? SHT_RELA : SHT_REL;
  rel_hdr->sh_entsize = (use_rela_p
			 ? bed->s->sizeof_rela
			 : bed->s->sizeof_rel);
  rel_hdr->sh_addralign = (bfd_vma) 1 << bed->s->log_file_align;
  rel_hdr->sh_flags = 0;
  rel_hdr->sh_addr = 0;
  rel_hdr->sh_size = 0;
  rel_hdr->sh_offset = 0;

  return TRUE;
}

/* Return the default section type based on the passed in section flags.  */

int
bfd_elf_get_default_section_type (flagword flags)
{
  if ((flags & SEC_ALLOC) != 0
      && (flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
    return SHT_NOBITS;
  return SHT_PROGBITS;
}

struct fake_section_arg
{
  struct bfd_link_info *link_info;
  bfd_boolean failed;
};

/* Set up an ELF internal section header for a section.  */

static void
elf_fake_sections (bfd *abfd, asection *asect, void *fsarg)
{
  struct fake_section_arg *arg = (struct fake_section_arg *)fsarg;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct bfd_elf_section_data *esd = elf_section_data (asect);
  Elf_Internal_Shdr *this_hdr;
  unsigned int sh_type;

  if (arg->failed)
    {
      /* We already failed; just get out of the bfd_map_over_sections
	 loop.  */
      return;
    }

  this_hdr = &esd->this_hdr;

  this_hdr->sh_name = (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd),
							  asect->name, FALSE);
  if (this_hdr->sh_name == (unsigned int) -1)
    {
      arg->failed = TRUE;
      return;
    }

  /* Don't clear sh_flags. Assembler may set additional bits.  */

  if ((asect->flags & SEC_ALLOC) != 0
      || asect->user_set_vma)
    this_hdr->sh_addr = asect->vma;
  else
    this_hdr->sh_addr = 0;

  this_hdr->sh_offset = 0;
  this_hdr->sh_size = asect->size;
  this_hdr->sh_link = 0;
  /* PR 17512: file: 0eb809fe, 8b0535ee.  */
  if (asect->alignment_power >= (sizeof (bfd_vma) * 8) - 1)
    {
      (*_bfd_error_handler)
	(_("%B: error: Alignment power %d of section `%A' is too big"),
	 abfd, asect, asect->alignment_power);
      arg->failed = TRUE;
      return;
    }
  this_hdr->sh_addralign = (bfd_vma) 1 << asect->alignment_power;
  /* The sh_entsize and sh_info fields may have been set already by
     copy_private_section_data.  */

  this_hdr->bfd_section = asect;
  this_hdr->contents = NULL;

  /* If the section type is unspecified, we set it based on
     asect->flags.  */
  if ((asect->flags & SEC_GROUP) != 0)
    sh_type = SHT_GROUP;
  else
    sh_type = bfd_elf_get_default_section_type (asect->flags);

  if (this_hdr->sh_type == SHT_NULL)
    this_hdr->sh_type = sh_type;
  else if (this_hdr->sh_type == SHT_NOBITS
	   && sh_type == SHT_PROGBITS
	   && (asect->flags & SEC_ALLOC) != 0)
    {
      /* Warn if we are changing a NOBITS section to PROGBITS, but
	 allow the link to proceed.  This can happen when users link
	 non-bss input sections to bss output sections, or emit data
	 to a bss output section via a linker script.  */
      (*_bfd_error_handler)
	(_("warning: section `%A' type changed to PROGBITS"), asect);
      this_hdr->sh_type = sh_type;
    }

  switch (this_hdr->sh_type)
    {
    default:
      break;

    case SHT_STRTAB:
    case SHT_INIT_ARRAY:
    case SHT_FINI_ARRAY:
    case SHT_PREINIT_ARRAY:
    case SHT_NOTE:
    case SHT_NOBITS:
    case SHT_PROGBITS:
      break;

    case SHT_HASH:
      this_hdr->sh_entsize = bed->s->sizeof_hash_entry;
      break;

    case SHT_DYNSYM:
      this_hdr->sh_entsize = bed->s->sizeof_sym;
      break;

    case SHT_DYNAMIC:
      this_hdr->sh_entsize = bed->s->sizeof_dyn;
      break;

    case SHT_RELA:
      if (get_elf_backend_data (abfd)->may_use_rela_p)
	this_hdr->sh_entsize = bed->s->sizeof_rela;
      break;

     case SHT_REL:
      if (get_elf_backend_data (abfd)->may_use_rel_p)
	this_hdr->sh_entsize = bed->s->sizeof_rel;
      break;

     case SHT_GNU_versym:
      this_hdr->sh_entsize = sizeof (Elf_External_Versym);
      break;

     case SHT_GNU_verdef:
      this_hdr->sh_entsize = 0;
      /* objcopy or strip will copy over sh_info, but may not set
	 cverdefs.  The linker will set cverdefs, but sh_info will be
	 zero.  */
      if (this_hdr->sh_info == 0)
	this_hdr->sh_info = elf_tdata (abfd)->cverdefs;
      else
	BFD_ASSERT (elf_tdata (abfd)->cverdefs == 0
		    || this_hdr->sh_info == elf_tdata (abfd)->cverdefs);
      break;

    case SHT_GNU_verneed:
      this_hdr->sh_entsize = 0;
      /* objcopy or strip will copy over sh_info, but may not set
	 cverrefs.  The linker will set cverrefs, but sh_info will be
	 zero.  */
      if (this_hdr->sh_info == 0)
	this_hdr->sh_info = elf_tdata (abfd)->cverrefs;
      else
	BFD_ASSERT (elf_tdata (abfd)->cverrefs == 0
		    || this_hdr->sh_info == elf_tdata (abfd)->cverrefs);
      break;

    case SHT_GROUP:
      this_hdr->sh_entsize = GRP_ENTRY_SIZE;
      break;

    case SHT_GNU_HASH:
      this_hdr->sh_entsize = bed->s->arch_size == 64 ? 0 : 4;
      break;
    }

  if ((asect->flags & SEC_ALLOC) != 0)
    this_hdr->sh_flags |= SHF_ALLOC;
  if ((asect->flags & SEC_READONLY) == 0)
    this_hdr->sh_flags |= SHF_WRITE;
  if ((asect->flags & SEC_CODE) != 0)
    this_hdr->sh_flags |= SHF_EXECINSTR;
  if ((asect->flags & SEC_MERGE) != 0)
    {
      this_hdr->sh_flags |= SHF_MERGE;
      this_hdr->sh_entsize = asect->entsize;
      if ((asect->flags & SEC_STRINGS) != 0)
	this_hdr->sh_flags |= SHF_STRINGS;
    }
  if ((asect->flags & SEC_GROUP) == 0 && elf_group_name (asect) != NULL)
    this_hdr->sh_flags |= SHF_GROUP;
  if ((asect->flags & SEC_THREAD_LOCAL) != 0)
    {
      this_hdr->sh_flags |= SHF_TLS;
      if (asect->size == 0
	  && (asect->flags & SEC_HAS_CONTENTS) == 0)
	{
	  struct bfd_link_order *o = asect->map_tail.link_order;

	  this_hdr->sh_size = 0;
	  if (o != NULL)
	    {
	      this_hdr->sh_size = o->offset + o->size;
	      if (this_hdr->sh_size != 0)
		this_hdr->sh_type = SHT_NOBITS;
	    }
	}
    }
  if ((asect->flags & (SEC_GROUP | SEC_EXCLUDE)) == SEC_EXCLUDE)
    this_hdr->sh_flags |= SHF_EXCLUDE;

  /* If the section has relocs, set up a section header for the
     SHT_REL[A] section.  If two relocation sections are required for
     this section, it is up to the processor-specific back-end to
     create the other.  */
  if ((asect->flags & SEC_RELOC) != 0)
    {
      /* When doing a relocatable link, create both REL and RELA sections if
	 needed.  */
      if (arg->link_info
	  /* Do the normal setup if we wouldn't create any sections here.  */
	  && esd->rel.count + esd->rela.count > 0
	  && (arg->link_info->relocatable || arg->link_info->emitrelocations))
	{
	  if (esd->rel.count && esd->rel.hdr == NULL
	      && !_bfd_elf_init_reloc_shdr (abfd, &esd->rel, asect, FALSE))
	    {
	      arg->failed = TRUE;
	      return;
	    }
	  if (esd->rela.count && esd->rela.hdr == NULL
	      && !_bfd_elf_init_reloc_shdr (abfd, &esd->rela, asect, TRUE))
	    {
	      arg->failed = TRUE;
	      return;
	    }
	}
      else if (!_bfd_elf_init_reloc_shdr (abfd,
					  (asect->use_rela_p
					   ? &esd->rela : &esd->rel),
					  asect,
					  asect->use_rela_p))
	  arg->failed = TRUE;
    }

  /* Check for processor-specific section types.  */
  sh_type = this_hdr->sh_type;
  if (bed->elf_backend_fake_sections
      && !(*bed->elf_backend_fake_sections) (abfd, this_hdr, asect))
    arg->failed = TRUE;

  if (sh_type == SHT_NOBITS && asect->size != 0)
    {
      /* Don't change the header type from NOBITS if we are being
	 called for objcopy --only-keep-debug.  */
      this_hdr->sh_type = sh_type;
    }
}

/* Fill in the contents of a SHT_GROUP section.  Called from
   _bfd_elf_compute_section_file_positions for gas, objcopy, and
   when ELF targets use the generic linker, ld.  Called for ld -r
   from bfd_elf_final_link.  */

void
bfd_elf_set_group_contents (bfd *abfd, asection *sec, void *failedptrarg)
{
  bfd_boolean *failedptr = (bfd_boolean *) failedptrarg;
  asection *elt, *first;
  unsigned char *loc;
  bfd_boolean gas;

  /* Ignore linker created group section.  See elfNN_ia64_object_p in
     elfxx-ia64.c.  */
  if (((sec->flags & (SEC_GROUP | SEC_LINKER_CREATED)) != SEC_GROUP)
      || *failedptr)
    return;

  if (elf_section_data (sec)->this_hdr.sh_info == 0)
    {
      unsigned long symindx = 0;

      /* elf_group_id will have been set up by objcopy and the
	 generic linker.  */
      if (elf_group_id (sec) != NULL)
	symindx = elf_group_id (sec)->udata.i;

      if (symindx == 0)
	{
	  /* If called from the assembler, swap_out_syms will have set up
	     elf_section_syms.  */
	  BFD_ASSERT (elf_section_syms (abfd) != NULL);
	  symindx = elf_section_syms (abfd)[sec->index]->udata.i;
	}
      elf_section_data (sec)->this_hdr.sh_info = symindx;
    }
  else if (elf_section_data (sec)->this_hdr.sh_info == (unsigned int) -2)
    {
      /* The ELF backend linker sets sh_info to -2 when the group
	 signature symbol is global, and thus the index can't be
	 set until all local symbols are output.  */
      asection *igroup = elf_sec_group (elf_next_in_group (sec));
      struct bfd_elf_section_data *sec_data = elf_section_data (igroup);
      unsigned long symndx = sec_data->this_hdr.sh_info;
      unsigned long extsymoff = 0;
      struct elf_link_hash_entry *h;

      if (!elf_bad_symtab (igroup->owner))
	{
	  Elf_Internal_Shdr *symtab_hdr;

	  symtab_hdr = &elf_tdata (igroup->owner)->symtab_hdr;
	  extsymoff = symtab_hdr->sh_info;
	}
      h = elf_sym_hashes (igroup->owner)[symndx - extsymoff];
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      elf_section_data (sec)->this_hdr.sh_info = h->indx;
    }

  /* The contents won't be allocated for "ld -r" or objcopy.  */
  gas = TRUE;
  if (sec->contents == NULL)
    {
      gas = FALSE;
      sec->contents = (unsigned char *) bfd_alloc (abfd, sec->size);

      /* Arrange for the section to be written out.  */
      elf_section_data (sec)->this_hdr.contents = sec->contents;
      if (sec->contents == NULL)
	{
	  *failedptr = TRUE;
	  return;
	}
    }

  loc = sec->contents + sec->size;

  /* Get the pointer to the first section in the group that gas
     squirreled away here.  objcopy arranges for this to be set to the
     start of the input section group.  */
  first = elt = elf_next_in_group (sec);

  /* First element is a flag word.  Rest of section is elf section
     indices for all the sections of the group.  Write them backwards
     just to keep the group in the same order as given in .section
     directives, not that it matters.  */
  while (elt != NULL)
    {
      asection *s;

      s = elt;
      if (!gas)
	s = s->output_section;
      if (s != NULL
	  && !bfd_is_abs_section (s))
	{
	  unsigned int idx = elf_section_data (s)->this_idx;

	  loc -= 4;
	  H_PUT_32 (abfd, idx, loc);
	}
      elt = elf_next_in_group (elt);
      if (elt == first)
	break;
    }

  if ((loc -= 4) != sec->contents)
    abort ();

  H_PUT_32 (abfd, sec->flags & SEC_LINK_ONCE ? GRP_COMDAT : 0, loc);
}

/* Assign all ELF section numbers.  The dummy first section is handled here
   too.  The link/info pointers for the standard section types are filled
   in here too, while we're at it.  */

static bfd_boolean
assign_section_numbers (bfd *abfd, struct bfd_link_info *link_info)
{
  struct elf_obj_tdata *t = elf_tdata (abfd);
  asection *sec;
  unsigned int section_number, secn;
  Elf_Internal_Shdr **i_shdrp;
  struct bfd_elf_section_data *d;
  bfd_boolean need_symtab;

  section_number = 1;

  _bfd_elf_strtab_clear_all_refs (elf_shstrtab (abfd));

  /* SHT_GROUP sections are in relocatable files only.  */
  if (link_info == NULL || link_info->relocatable)
    {
      /* Put SHT_GROUP sections first.  */
      for (sec = abfd->sections; sec != NULL; sec = sec->next)
	{
	  d = elf_section_data (sec);

	  if (d->this_hdr.sh_type == SHT_GROUP)
	    {
	      if (sec->flags & SEC_LINKER_CREATED)
		{
		  /* Remove the linker created SHT_GROUP sections.  */
		  bfd_section_list_remove (abfd, sec);
		  abfd->section_count--;
		}
	      else
		d->this_idx = section_number++;
	    }
	}
    }

  for (sec = abfd->sections; sec; sec = sec->next)
    {
      d = elf_section_data (sec);

      if (d->this_hdr.sh_type != SHT_GROUP)
	d->this_idx = section_number++;
      _bfd_elf_strtab_addref (elf_shstrtab (abfd), d->this_hdr.sh_name);
      if (d->rel.hdr)
	{
	  d->rel.idx = section_number++;
	  _bfd_elf_strtab_addref (elf_shstrtab (abfd), d->rel.hdr->sh_name);
	}
      else
	d->rel.idx = 0;

      if (d->rela.hdr)
	{
	  d->rela.idx = section_number++;
	  _bfd_elf_strtab_addref (elf_shstrtab (abfd), d->rela.hdr->sh_name);
	}
      else
	d->rela.idx = 0;
    }

  elf_shstrtab_sec (abfd) = section_number++;
  _bfd_elf_strtab_addref (elf_shstrtab (abfd), t->shstrtab_hdr.sh_name);
  elf_elfheader (abfd)->e_shstrndx = elf_shstrtab_sec (abfd);

  need_symtab = (bfd_get_symcount (abfd) > 0
		|| (link_info == NULL
		    && ((abfd->flags & (EXEC_P | DYNAMIC | HAS_RELOC))
			== HAS_RELOC)));
  if (need_symtab)
    {
      elf_onesymtab (abfd) = section_number++;
      _bfd_elf_strtab_addref (elf_shstrtab (abfd), t->symtab_hdr.sh_name);
      if (section_number > ((SHN_LORESERVE - 2) & 0xFFFF))
	{
	  elf_symtab_shndx (abfd) = section_number++;
	  t->symtab_shndx_hdr.sh_name
	    = (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd),
						  ".symtab_shndx", FALSE);
	  if (t->symtab_shndx_hdr.sh_name == (unsigned int) -1)
	    return FALSE;
	}
      elf_strtab_sec (abfd) = section_number++;
      _bfd_elf_strtab_addref (elf_shstrtab (abfd), t->strtab_hdr.sh_name);
    }

  if (section_number >= SHN_LORESERVE)
    {
      _bfd_error_handler (_("%B: too many sections: %u"),
			  abfd, section_number);
      return FALSE;
    }

  _bfd_elf_strtab_finalize (elf_shstrtab (abfd));
  t->shstrtab_hdr.sh_size = _bfd_elf_strtab_size (elf_shstrtab (abfd));

  elf_numsections (abfd) = section_number;
  elf_elfheader (abfd)->e_shnum = section_number;

  /* Set up the list of section header pointers, in agreement with the
     indices.  */
  i_shdrp = (Elf_Internal_Shdr **) bfd_zalloc2 (abfd, section_number,
                                                sizeof (Elf_Internal_Shdr *));
  if (i_shdrp == NULL)
    return FALSE;

  i_shdrp[0] = (Elf_Internal_Shdr *) bfd_zalloc (abfd,
                                                 sizeof (Elf_Internal_Shdr));
  if (i_shdrp[0] == NULL)
    {
      bfd_release (abfd, i_shdrp);
      return FALSE;
    }

  elf_elfsections (abfd) = i_shdrp;

  i_shdrp[elf_shstrtab_sec (abfd)] = &t->shstrtab_hdr;
  if (need_symtab)
    {
      i_shdrp[elf_onesymtab (abfd)] = &t->symtab_hdr;
      if (elf_numsections (abfd) > (SHN_LORESERVE & 0xFFFF))
	{
	  i_shdrp[elf_symtab_shndx (abfd)] = &t->symtab_shndx_hdr;
	  t->symtab_shndx_hdr.sh_link = elf_onesymtab (abfd);
	}
      i_shdrp[elf_strtab_sec (abfd)] = &t->strtab_hdr;
      t->symtab_hdr.sh_link = elf_strtab_sec (abfd);
    }

  for (sec = abfd->sections; sec; sec = sec->next)
    {
      asection *s;
      const char *name;

      d = elf_section_data (sec);

      i_shdrp[d->this_idx] = &d->this_hdr;
      if (d->rel.idx != 0)
	i_shdrp[d->rel.idx] = d->rel.hdr;
      if (d->rela.idx != 0)
	i_shdrp[d->rela.idx] = d->rela.hdr;

      /* Fill in the sh_link and sh_info fields while we're at it.  */

      /* sh_link of a reloc section is the section index of the symbol
	 table.  sh_info is the section index of the section to which
	 the relocation entries apply.  */
      if (d->rel.idx != 0)
	{
	  d->rel.hdr->sh_link = elf_onesymtab (abfd);
	  d->rel.hdr->sh_info = d->this_idx;
	  d->rel.hdr->sh_flags |= SHF_INFO_LINK;
	}
      if (d->rela.idx != 0)
	{
	  d->rela.hdr->sh_link = elf_onesymtab (abfd);
	  d->rela.hdr->sh_info = d->this_idx;
	  d->rela.hdr->sh_flags |= SHF_INFO_LINK;
	}

      /* We need to set up sh_link for SHF_LINK_ORDER.  */
      if ((d->this_hdr.sh_flags & SHF_LINK_ORDER) != 0)
	{
	  s = elf_linked_to_section (sec);
	  if (s)
	    {
	      /* elf_linked_to_section points to the input section.  */
	      if (link_info != NULL)
		{
		  /* Check discarded linkonce section.  */
		  if (discarded_section (s))
		    {
		      asection *kept;
		      (*_bfd_error_handler)
			(_("%B: sh_link of section `%A' points to discarded section `%A' of `%B'"),
			 abfd, d->this_hdr.bfd_section,
			 s, s->owner);
		      /* Point to the kept section if it has the same
			 size as the discarded one.  */
		      kept = _bfd_elf_check_kept_section (s, link_info);
		      if (kept == NULL)
			{
			  bfd_set_error (bfd_error_bad_value);
			  return FALSE;
			}
		      s = kept;
		    }

		  s = s->output_section;
		  BFD_ASSERT (s != NULL);
		}
	      else
		{
		  /* Handle objcopy. */
		  if (s->output_section == NULL)
		    {
		      (*_bfd_error_handler)
			(_("%B: sh_link of section `%A' points to removed section `%A' of `%B'"),
			 abfd, d->this_hdr.bfd_section, s, s->owner);
		      bfd_set_error (bfd_error_bad_value);
		      return FALSE;
		    }
		  s = s->output_section;
		}
	      d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	    }
	  else
	    {
	      /* PR 290:
		 The Intel C compiler generates SHT_IA_64_UNWIND with
		 SHF_LINK_ORDER.  But it doesn't set the sh_link or
		 sh_info fields.  Hence we could get the situation
		 where s is NULL.  */
	      const struct elf_backend_data *bed
		= get_elf_backend_data (abfd);
	      if (bed->link_order_error_handler)
		bed->link_order_error_handler
		  (_("%B: warning: sh_link not set for section `%A'"),
		   abfd, sec);
	    }
	}

      switch (d->this_hdr.sh_type)
	{
	case SHT_REL:
	case SHT_RELA:
	  /* A reloc section which we are treating as a normal BFD
	     section.  sh_link is the section index of the symbol
	     table.  sh_info is the section index of the section to
	     which the relocation entries apply.  We assume that an
	     allocated reloc section uses the dynamic symbol table.
	     FIXME: How can we be sure?  */
	  s = bfd_get_section_by_name (abfd, ".dynsym");
	  if (s != NULL)
	    d->this_hdr.sh_link = elf_section_data (s)->this_idx;

	  /* We look up the section the relocs apply to by name.  */
	  name = sec->name;
	  if (d->this_hdr.sh_type == SHT_REL)
	    name += 4;
	  else
	    name += 5;
	  s = bfd_get_section_by_name (abfd, name);
	  if (s != NULL)
	    {
	      d->this_hdr.sh_info = elf_section_data (s)->this_idx;
	      d->this_hdr.sh_flags |= SHF_INFO_LINK;
	    }
	  break;

	case SHT_STRTAB:
	  /* We assume that a section named .stab*str is a stabs
	     string section.  We look for a section with the same name
	     but without the trailing ``str'', and set its sh_link
	     field to point to this section.  */
	  if (CONST_STRNEQ (sec->name, ".stab")
	      && strcmp (sec->name + strlen (sec->name) - 3, "str") == 0)
	    {
	      size_t len;
	      char *alc;

	      len = strlen (sec->name);
	      alc = (char *) bfd_malloc (len - 2);
	      if (alc == NULL)
		return FALSE;
	      memcpy (alc, sec->name, len - 3);
	      alc[len - 3] = '\0';
	      s = bfd_get_section_by_name (abfd, alc);
	      free (alc);
	      if (s != NULL)
		{
		  elf_section_data (s)->this_hdr.sh_link = d->this_idx;

		  /* This is a .stab section.  */
		  if (elf_section_data (s)->this_hdr.sh_entsize == 0)
		    elf_section_data (s)->this_hdr.sh_entsize
		      = 4 + 2 * bfd_get_arch_size (abfd) / 8;
		}
	    }
	  break;

	case SHT_DYNAMIC:
	case SHT_DYNSYM:
	case SHT_GNU_verneed:
	case SHT_GNU_verdef:
	  /* sh_link is the section header index of the string table
	     used for the dynamic entries, or the symbol table, or the
	     version strings.  */
	  s = bfd_get_section_by_name (abfd, ".dynstr");
	  if (s != NULL)
	    d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	  break;

	case SHT_GNU_LIBLIST:
	  /* sh_link is the section header index of the prelink library
	     list used for the dynamic entries, or the symbol table, or
	     the version strings.  */
	  s = bfd_get_section_by_name (abfd, (sec->flags & SEC_ALLOC)
					     ? ".dynstr" : ".gnu.libstr");
	  if (s != NULL)
	    d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	  break;

	case SHT_HASH:
	case SHT_GNU_HASH:
	case SHT_GNU_versym:
	  /* sh_link is the section header index of the symbol table
	     this hash table or version table is for.  */
	  s = bfd_get_section_by_name (abfd, ".dynsym");
	  if (s != NULL)
	    d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	  break;

	case SHT_GROUP:
	  d->this_hdr.sh_link = elf_onesymtab (abfd);
	}
    }

  for (secn = 1; secn < section_number; ++secn)
    if (i_shdrp[secn] == NULL)
      i_shdrp[secn] = i_shdrp[0];
    else
      i_shdrp[secn]->sh_name = _bfd_elf_strtab_offset (elf_shstrtab (abfd),
						       i_shdrp[secn]->sh_name);
  return TRUE;
}

static bfd_boolean
sym_is_global (bfd *abfd, asymbol *sym)
{
  /* If the backend has a special mapping, use it.  */
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_sym_is_global)
    return (*bed->elf_backend_sym_is_global) (abfd, sym);

  return ((sym->flags & (BSF_GLOBAL | BSF_WEAK | BSF_GNU_UNIQUE)) != 0
	  || bfd_is_und_section (bfd_get_section (sym))
	  || bfd_is_com_section (bfd_get_section (sym)));
}

/* Don't output section symbols for sections that are not going to be
   output, that are duplicates or there is no BFD section.  */

static bfd_boolean
ignore_section_sym (bfd *abfd, asymbol *sym)
{
  elf_symbol_type *type_ptr;

  if ((sym->flags & BSF_SECTION_SYM) == 0)
    return FALSE;

  type_ptr = elf_symbol_from (abfd, sym);
  return ((type_ptr != NULL
	   && type_ptr->internal_elf_sym.st_shndx != 0
	   && bfd_is_abs_section (sym->section))
	  || !(sym->section->owner == abfd
	       || (sym->section->output_section->owner == abfd
		   && sym->section->output_offset == 0)
	       || bfd_is_abs_section (sym->section)));
}

/* Map symbol from it's internal number to the external number, moving
   all local symbols to be at the head of the list.  */

static bfd_boolean
elf_map_symbols (bfd *abfd, unsigned int *pnum_locals)
{
  unsigned int symcount = bfd_get_symcount (abfd);
  asymbol **syms = bfd_get_outsymbols (abfd);
  asymbol **sect_syms;
  unsigned int num_locals = 0;
  unsigned int num_globals = 0;
  unsigned int num_locals2 = 0;
  unsigned int num_globals2 = 0;
  int max_index = 0;
  unsigned int idx;
  asection *asect;
  asymbol **new_syms;

#ifdef DEBUG
  fprintf (stderr, "elf_map_symbols\n");
  fflush (stderr);
#endif

  for (asect = abfd->sections; asect; asect = asect->next)
    {
      if (max_index < asect->index)
	max_index = asect->index;
    }

  max_index++;
  sect_syms = (asymbol **) bfd_zalloc2 (abfd, max_index, sizeof (asymbol *));
  if (sect_syms == NULL)
    return FALSE;
  elf_section_syms (abfd) = sect_syms;
  elf_num_section_syms (abfd) = max_index;

  /* Init sect_syms entries for any section symbols we have already
     decided to output.  */
  for (idx = 0; idx < symcount; idx++)
    {
      asymbol *sym = syms[idx];

      if ((sym->flags & BSF_SECTION_SYM) != 0
	  && sym->value == 0
	  && !ignore_section_sym (abfd, sym)
	  && !bfd_is_abs_section (sym->section))
	{
	  asection *sec = sym->section;

	  if (sec->owner != abfd)
	    sec = sec->output_section;

	  sect_syms[sec->index] = syms[idx];
	}
    }

  /* Classify all of the symbols.  */
  for (idx = 0; idx < symcount; idx++)
    {
      if (sym_is_global (abfd, syms[idx]))
	num_globals++;
      else if (!ignore_section_sym (abfd, syms[idx]))
	num_locals++;
    }

  /* We will be adding a section symbol for each normal BFD section.  Most
     sections will already have a section symbol in outsymbols, but
     eg. SHT_GROUP sections will not, and we need the section symbol mapped
     at least in that case.  */
  for (asect = abfd->sections; asect; asect = asect->next)
    {
      if (sect_syms[asect->index] == NULL)
	{
	  if (!sym_is_global (abfd, asect->symbol))
	    num_locals++;
	  else
	    num_globals++;
	}
    }

  /* Now sort the symbols so the local symbols are first.  */
  new_syms = (asymbol **) bfd_alloc2 (abfd, num_locals + num_globals,
                                      sizeof (asymbol *));

  if (new_syms == NULL)
    return FALSE;

  for (idx = 0; idx < symcount; idx++)
    {
      asymbol *sym = syms[idx];
      unsigned int i;

      if (sym_is_global (abfd, sym))
	i = num_locals + num_globals2++;
      else if (!ignore_section_sym (abfd, sym))
	i = num_locals2++;
      else
	continue;
      new_syms[i] = sym;
      sym->udata.i = i + 1;
    }
  for (asect = abfd->sections; asect; asect = asect->next)
    {
      if (sect_syms[asect->index] == NULL)
	{
	  asymbol *sym = asect->symbol;
	  unsigned int i;

	  sect_syms[asect->index] = sym;
	  if (!sym_is_global (abfd, sym))
	    i = num_locals2++;
	  else
	    i = num_locals + num_globals2++;
	  new_syms[i] = sym;
	  sym->udata.i = i + 1;
	}
    }

  bfd_set_symtab (abfd, new_syms, num_locals + num_globals);

  *pnum_locals = num_locals;
  return TRUE;
}

/* Align to the maximum file alignment that could be required for any
   ELF data structure.  */

static inline file_ptr
align_file_position (file_ptr off, int align)
{
  return (off + align - 1) & ~(align - 1);
}

/* Assign a file position to a section, optionally aligning to the
   required section alignment.  */

file_ptr
_bfd_elf_assign_file_position_for_section (Elf_Internal_Shdr *i_shdrp,
					   file_ptr offset,
					   bfd_boolean align)
{
  if (align && i_shdrp->sh_addralign > 1)
    offset = BFD_ALIGN (offset, i_shdrp->sh_addralign);
  i_shdrp->sh_offset = offset;
  if (i_shdrp->bfd_section != NULL)
    i_shdrp->bfd_section->filepos = offset;
  if (i_shdrp->sh_type != SHT_NOBITS)
    offset += i_shdrp->sh_size;
  return offset;
}

/* Compute the file positions we are going to put the sections at, and
   otherwise prepare to begin writing out the ELF file.  If LINK_INFO
   is not NULL, this is being called by the ELF backend linker.  */

bfd_boolean
_bfd_elf_compute_section_file_positions (bfd *abfd,
					 struct bfd_link_info *link_info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct fake_section_arg fsargs;
  bfd_boolean failed;
  struct bfd_strtab_hash *strtab = NULL;
  Elf_Internal_Shdr *shstrtab_hdr;
  bfd_boolean need_symtab;

  if (abfd->output_has_begun)
    return TRUE;

  /* Do any elf backend specific processing first.  */
  if (bed->elf_backend_begin_write_processing)
    (*bed->elf_backend_begin_write_processing) (abfd, link_info);

  if (! prep_headers (abfd))
    return FALSE;

  /* Post process the headers if necessary.  */
  (*bed->elf_backend_post_process_headers) (abfd, link_info);

  fsargs.failed = FALSE;
  fsargs.link_info = link_info;
  bfd_map_over_sections (abfd, elf_fake_sections, &fsargs);
  if (fsargs.failed)
    return FALSE;

  if (!assign_section_numbers (abfd, link_info))
    return FALSE;

  /* The backend linker builds symbol table information itself.  */
  need_symtab = (link_info == NULL
		 && (bfd_get_symcount (abfd) > 0
		     || ((abfd->flags & (EXEC_P | DYNAMIC | HAS_RELOC))
			 == HAS_RELOC)));
  if (need_symtab)
    {
      /* Non-zero if doing a relocatable link.  */
      int relocatable_p = ! (abfd->flags & (EXEC_P | DYNAMIC));

      if (! swap_out_syms (abfd, &strtab, relocatable_p))
	return FALSE;
    }

  failed = FALSE;
  if (link_info == NULL)
    {
      bfd_map_over_sections (abfd, bfd_elf_set_group_contents, &failed);
      if (failed)
	return FALSE;
    }

  shstrtab_hdr = &elf_tdata (abfd)->shstrtab_hdr;
  /* sh_name was set in prep_headers.  */
  shstrtab_hdr->sh_type = SHT_STRTAB;
  shstrtab_hdr->sh_flags = 0;
  shstrtab_hdr->sh_addr = 0;
  shstrtab_hdr->sh_size = _bfd_elf_strtab_size (elf_shstrtab (abfd));
  shstrtab_hdr->sh_entsize = 0;
  shstrtab_hdr->sh_link = 0;
  shstrtab_hdr->sh_info = 0;
  /* sh_offset is set in assign_file_positions_except_relocs.  */
  shstrtab_hdr->sh_addralign = 1;

  if (!assign_file_positions_except_relocs (abfd, link_info))
    return FALSE;

  if (need_symtab)
    {
      file_ptr off;
      Elf_Internal_Shdr *hdr;

      off = elf_next_file_pos (abfd);

      hdr = &elf_tdata (abfd)->symtab_hdr;
      off = _bfd_elf_assign_file_position_for_section (hdr, off, TRUE);

      hdr = &elf_tdata (abfd)->symtab_shndx_hdr;
      if (hdr->sh_size != 0)
	off = _bfd_elf_assign_file_position_for_section (hdr, off, TRUE);

      hdr = &elf_tdata (abfd)->strtab_hdr;
      off = _bfd_elf_assign_file_position_for_section (hdr, off, TRUE);

      elf_next_file_pos (abfd) = off;

      /* Now that we know where the .strtab section goes, write it
	 out.  */
      if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) != 0
	  || ! _bfd_stringtab_emit (abfd, strtab))
	return FALSE;
      _bfd_stringtab_free (strtab);
    }

  abfd->output_has_begun = TRUE;

  return TRUE;
}

/* Make an initial estimate of the size of the program header.  If we
   get the number wrong here, we'll redo section placement.  */

static bfd_size_type
get_program_header_size (bfd *abfd, struct bfd_link_info *info)
{
  size_t segs;
  asection *s;
  const struct elf_backend_data *bed;

  /* Assume we will need exactly two PT_LOAD segments: one for text
     and one for data.  */
  segs = 2;

  s = bfd_get_section_by_name (abfd, ".interp");
  if (s != NULL && (s->flags & SEC_LOAD) != 0)
    {
      /* If we have a loadable interpreter section, we need a
	 PT_INTERP segment.  In this case, assume we also need a
	 PT_PHDR segment, although that may not be true for all
	 targets.  */
      segs += 2;
    }

  if (bfd_get_section_by_name (abfd, ".dynamic") != NULL)
    {
      /* We need a PT_DYNAMIC segment.  */
      ++segs;
    }

  if (info != NULL && info->relro)
    {
      /* We need a PT_GNU_RELRO segment.  */
      ++segs;
    }

  if (elf_eh_frame_hdr (abfd))
    {
      /* We need a PT_GNU_EH_FRAME segment.  */
      ++segs;
    }

  if (elf_stack_flags (abfd))
    {
      /* We need a PT_GNU_STACK segment.  */
      ++segs;
    }

  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LOAD) != 0
	  && CONST_STRNEQ (s->name, ".note"))
	{
	  /* We need a PT_NOTE segment.  */
	  ++segs;
	  /* Try to create just one PT_NOTE segment
	     for all adjacent loadable .note* sections.
	     gABI requires that within a PT_NOTE segment
	     (and also inside of each SHT_NOTE section)
	     each note is padded to a multiple of 4 size,
	     so we check whether the sections are correctly
	     aligned.  */
	  if (s->alignment_power == 2)
	    while (s->next != NULL
		   && s->next->alignment_power == 2
		   && (s->next->flags & SEC_LOAD) != 0
		   && CONST_STRNEQ (s->next->name, ".note"))
	      s = s->next;
	}
    }

  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (s->flags & SEC_THREAD_LOCAL)
	{
	  /* We need a PT_TLS segment.  */
	  ++segs;
	  break;
	}
    }

  /* Let the backend count up any program headers it might need.  */
  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_additional_program_headers)
    {
      int a;

      a = (*bed->elf_backend_additional_program_headers) (abfd, info);
      if (a == -1)
	abort ();
      segs += a;
    }

  return segs * bed->s->sizeof_phdr;
}

/* Find the segment that contains the output_section of section.  */

Elf_Internal_Phdr *
_bfd_elf_find_segment_containing_section (bfd * abfd, asection * section)
{
  struct elf_segment_map *m;
  Elf_Internal_Phdr *p;

  for (m = elf_seg_map (abfd), p = elf_tdata (abfd)->phdr;
       m != NULL;
       m = m->next, p++)
    {
      int i;

      for (i = m->count - 1; i >= 0; i--)
	if (m->sections[i] == section)
	  return p;
    }

  return NULL;
}

/* Create a mapping from a set of sections to a program segment.  */

static struct elf_segment_map *
make_mapping (bfd *abfd,
	      asection **sections,
	      unsigned int from,
	      unsigned int to,
	      bfd_boolean phdr)
{
  struct elf_segment_map *m;
  unsigned int i;
  asection **hdrpp;
  bfd_size_type amt;

  amt = sizeof (struct elf_segment_map);
  amt += (to - from - 1) * sizeof (asection *);
  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
  if (m == NULL)
    return NULL;
  m->next = NULL;
  m->p_type = PT_LOAD;
  for (i = from, hdrpp = sections + from; i < to; i++, hdrpp++)
    m->sections[i - from] = *hdrpp;
  m->count = to - from;

  if (from == 0 && phdr)
    {
      /* Include the headers in the first PT_LOAD segment.  */
      m->includes_filehdr = 1;
      m->includes_phdrs = 1;
    }

  return m;
}

/* Create the PT_DYNAMIC segment, which includes DYNSEC.  Returns NULL
   on failure.  */

struct elf_segment_map *
_bfd_elf_make_dynamic_segment (bfd *abfd, asection *dynsec)
{
  struct elf_segment_map *m;

  m = (struct elf_segment_map *) bfd_zalloc (abfd,
                                             sizeof (struct elf_segment_map));
  if (m == NULL)
    return NULL;
  m->next = NULL;
  m->p_type = PT_DYNAMIC;
  m->count = 1;
  m->sections[0] = dynsec;

  return m;
}

/* Possibly add or remove segments from the segment map.  */

static bfd_boolean
elf_modify_segment_map (bfd *abfd,
			struct bfd_link_info *info,
			bfd_boolean remove_empty_load)
{
  struct elf_segment_map **m;
  const struct elf_backend_data *bed;

  /* The placement algorithm assumes that non allocated sections are
     not in PT_LOAD segments.  We ensure this here by removing such
     sections from the segment map.  We also remove excluded
     sections.  Finally, any PT_LOAD segment without sections is
     removed.  */
  m = &elf_seg_map (abfd);
  while (*m)
    {
      unsigned int i, new_count;

      for (new_count = 0, i = 0; i < (*m)->count; i++)
	{
	  if (((*m)->sections[i]->flags & SEC_EXCLUDE) == 0
	      && (((*m)->sections[i]->flags & SEC_ALLOC) != 0
		  || (*m)->p_type != PT_LOAD))
	    {
	      (*m)->sections[new_count] = (*m)->sections[i];
	      new_count++;
	    }
	}
      (*m)->count = new_count;

      if (remove_empty_load && (*m)->p_type == PT_LOAD && (*m)->count == 0)
	*m = (*m)->next;
      else
	m = &(*m)->next;
    }

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_modify_segment_map != NULL)
    {
      if (!(*bed->elf_backend_modify_segment_map) (abfd, info))
	return FALSE;
    }

  return TRUE;
}

/* Set up a mapping from BFD sections to program segments.  */

bfd_boolean
_bfd_elf_map_sections_to_segments (bfd *abfd, struct bfd_link_info *info)
{
  unsigned int count;
  struct elf_segment_map *m;
  asection **sections = NULL;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_boolean no_user_phdrs;

  no_user_phdrs = elf_seg_map (abfd) == NULL;

  if (info != NULL)
    info->user_phdrs = !no_user_phdrs;

  if (no_user_phdrs && bfd_count_sections (abfd) != 0)
    {
      asection *s;
      unsigned int i;
      struct elf_segment_map *mfirst;
      struct elf_segment_map **pm;
      asection *last_hdr;
      bfd_vma last_size;
      unsigned int phdr_index;
      bfd_vma maxpagesize;
      asection **hdrpp;
      bfd_boolean phdr_in_segment = TRUE;
      bfd_boolean writable;
      int tls_count = 0;
      asection *first_tls = NULL;
      asection *dynsec, *eh_frame_hdr;
      bfd_size_type amt;
      bfd_vma addr_mask, wrap_to = 0;

      /* Select the allocated sections, and sort them.  */

      sections = (asection **) bfd_malloc2 (bfd_count_sections (abfd),
                                            sizeof (asection *));
      if (sections == NULL)
	goto error_return;

      /* Calculate top address, avoiding undefined behaviour of shift
	 left operator when shift count is equal to size of type
	 being shifted.  */
      addr_mask = ((bfd_vma) 1 << (bfd_arch_bits_per_address (abfd) - 1)) - 1;
      addr_mask = (addr_mask << 1) + 1;

      i = 0;
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  if ((s->flags & SEC_ALLOC) != 0)
	    {
	      sections[i] = s;
	      ++i;
	      /* A wrapping section potentially clashes with header.  */
	      if (((s->lma + s->size) & addr_mask) < (s->lma & addr_mask))
		wrap_to = (s->lma + s->size) & addr_mask;
	    }
	}
      BFD_ASSERT (i <= bfd_count_sections (abfd));
      count = i;

      qsort (sections, (size_t) count, sizeof (asection *), elf_sort_sections);

      /* Build the mapping.  */

      mfirst = NULL;
      pm = &mfirst;

      /* If we have a .interp section, then create a PT_PHDR segment for
	 the program headers and a PT_INTERP segment for the .interp
	 section.  */
      s = bfd_get_section_by_name (abfd, ".interp");
      if (s != NULL && (s->flags & SEC_LOAD) != 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_PHDR;
	  /* FIXME: UnixWare and Solaris set PF_X, Irix 5 does not.  */
	  m->p_flags = PF_R | PF_X;
	  m->p_flags_valid = 1;
	  m->includes_phdrs = 1;

	  *pm = m;
	  pm = &m->next;

	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_INTERP;
	  m->count = 1;
	  m->sections[0] = s;

	  *pm = m;
	  pm = &m->next;
	}

      /* Look through the sections.  We put sections in the same program
	 segment when the start of the second section can be placed within
	 a few bytes of the end of the first section.  */
      last_hdr = NULL;
      last_size = 0;
      phdr_index = 0;
      maxpagesize = bed->maxpagesize;
      /* PR 17512: file: c8455299.
	 Avoid divide-by-zero errors later on.
	 FIXME: Should we abort if the maxpagesize is zero ?  */
      if (maxpagesize == 0)
	maxpagesize = 1;
      writable = FALSE;
      dynsec = bfd_get_section_by_name (abfd, ".dynamic");
      if (dynsec != NULL
	  && (dynsec->flags & SEC_LOAD) == 0)
	dynsec = NULL;

      /* Deal with -Ttext or something similar such that the first section
	 is not adjacent to the program headers.  This is an
	 approximation, since at this point we don't know exactly how many
	 program headers we will need.  */
      if (count > 0)
	{
	  bfd_size_type phdr_size = elf_program_header_size (abfd);

	  if (phdr_size == (bfd_size_type) -1)
	    phdr_size = get_program_header_size (abfd, info);
	  phdr_size += bed->s->sizeof_ehdr;
	  if ((abfd->flags & D_PAGED) == 0
	      || (sections[0]->lma & addr_mask) < phdr_size
	      || ((sections[0]->lma & addr_mask) % maxpagesize
		  < phdr_size % maxpagesize)
	      || (sections[0]->lma & addr_mask & -maxpagesize) < wrap_to)
	    phdr_in_segment = FALSE;
	}

      for (i = 0, hdrpp = sections; i < count; i++, hdrpp++)
	{
	  asection *hdr;
	  bfd_boolean new_segment;

	  hdr = *hdrpp;

	  /* See if this section and the last one will fit in the same
	     segment.  */

	  if (last_hdr == NULL)
	    {
	      /* If we don't have a segment yet, then we don't need a new
		 one (we build the last one after this loop).  */
	      new_segment = FALSE;
	    }
	  else if (last_hdr->lma - last_hdr->vma != hdr->lma - hdr->vma)
	    {
	      /* If this section has a different relation between the
		 virtual address and the load address, then we need a new
		 segment.  */
	      new_segment = TRUE;
	    }
	  else if (hdr->lma < last_hdr->lma + last_size
		   || last_hdr->lma + last_size < last_hdr->lma)
	    {
	      /* If this section has a load address that makes it overlap
		 the previous section, then we need a new segment.  */
	      new_segment = TRUE;
	    }
	  /* In the next test we have to be careful when last_hdr->lma is close
	     to the end of the address space.  If the aligned address wraps
	     around to the start of the address space, then there are no more
	     pages left in memory and it is OK to assume that the current
	     section can be included in the current segment.  */
	  else if ((BFD_ALIGN (last_hdr->lma + last_size, maxpagesize) + maxpagesize
		    > last_hdr->lma)
		   && (BFD_ALIGN (last_hdr->lma + last_size, maxpagesize) + maxpagesize
		       <= hdr->lma))
	    {
	      /* If putting this section in this segment would force us to
		 skip a page in the segment, then we need a new segment.  */
	      new_segment = TRUE;
	    }
	  else if ((last_hdr->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) == 0
		   && (hdr->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) != 0)
	    {
	      /* We don't want to put a loadable section after a
		 nonloadable section in the same segment.
		 Consider .tbss sections as loadable for this purpose.  */
	      new_segment = TRUE;
	    }
	  else if ((abfd->flags & D_PAGED) == 0)
	    {
	      /* If the file is not demand paged, which means that we
		 don't require the sections to be correctly aligned in the
		 file, then there is no other reason for a new segment.  */
	      new_segment = FALSE;
	    }
	  else if (! writable
		   && (hdr->flags & SEC_READONLY) == 0
		   && (((last_hdr->lma + last_size - 1) & -maxpagesize)
		       != (hdr->lma & -maxpagesize)))
	    {
	      /* We don't want to put a writable section in a read only
		 segment, unless they are on the same page in memory
		 anyhow.  We already know that the last section does not
		 bring us past the current section on the page, so the
		 only case in which the new section is not on the same
		 page as the previous section is when the previous section
		 ends precisely on a page boundary.  */
	      new_segment = TRUE;
	    }
	  else
	    {
	      /* Otherwise, we can use the same segment.  */
	      new_segment = FALSE;
	    }

	  /* Allow interested parties a chance to override our decision.  */
	  if (last_hdr != NULL
	      && info != NULL
	      && info->callbacks->override_segment_assignment != NULL)
	    new_segment
	      = info->callbacks->override_segment_assignment (info, abfd, hdr,
							      last_hdr,
							      new_segment);

	  if (! new_segment)
	    {
	      if ((hdr->flags & SEC_READONLY) == 0)
		writable = TRUE;
	      last_hdr = hdr;
	      /* .tbss sections effectively have zero size.  */
	      if ((hdr->flags & (SEC_THREAD_LOCAL | SEC_LOAD))
		  != SEC_THREAD_LOCAL)
		last_size = hdr->size;
	      else
		last_size = 0;
	      continue;
	    }

	  /* We need a new program segment.  We must create a new program
	     header holding all the sections from phdr_index until hdr.  */

	  m = make_mapping (abfd, sections, phdr_index, i, phdr_in_segment);
	  if (m == NULL)
	    goto error_return;

	  *pm = m;
	  pm = &m->next;

	  if ((hdr->flags & SEC_READONLY) == 0)
	    writable = TRUE;
	  else
	    writable = FALSE;

	  last_hdr = hdr;
	  /* .tbss sections effectively have zero size.  */
	  if ((hdr->flags & (SEC_THREAD_LOCAL | SEC_LOAD)) != SEC_THREAD_LOCAL)
	    last_size = hdr->size;
	  else
	    last_size = 0;
	  phdr_index = i;
	  phdr_in_segment = FALSE;
	}

      /* Create a final PT_LOAD program segment, but not if it's just
	 for .tbss.  */
      if (last_hdr != NULL
	  && (i - phdr_index != 1
	      || ((last_hdr->flags & (SEC_THREAD_LOCAL | SEC_LOAD))
		  != SEC_THREAD_LOCAL)))
	{
	  m = make_mapping (abfd, sections, phdr_index, i, phdr_in_segment);
	  if (m == NULL)
	    goto error_return;

	  *pm = m;
	  pm = &m->next;
	}

      /* If there is a .dynamic section, throw in a PT_DYNAMIC segment.  */
      if (dynsec != NULL)
	{
	  m = _bfd_elf_make_dynamic_segment (abfd, dynsec);
	  if (m == NULL)
	    goto error_return;
	  *pm = m;
	  pm = &m->next;
	}

      /* For each batch of consecutive loadable .note sections,
	 add a PT_NOTE segment.  We don't use bfd_get_section_by_name,
	 because if we link together nonloadable .note sections and
	 loadable .note sections, we will generate two .note sections
	 in the output file.  FIXME: Using names for section types is
	 bogus anyhow.  */
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  if ((s->flags & SEC_LOAD) != 0
	      && CONST_STRNEQ (s->name, ".note"))
	    {
	      asection *s2;

	      count = 1;
	      amt = sizeof (struct elf_segment_map);
	      if (s->alignment_power == 2)
		for (s2 = s; s2->next != NULL; s2 = s2->next)
		  {
		    if (s2->next->alignment_power == 2
			&& (s2->next->flags & SEC_LOAD) != 0
			&& CONST_STRNEQ (s2->next->name, ".note")
			&& align_power (s2->lma + s2->size, 2)
			   == s2->next->lma)
		      count++;
		    else
		      break;
		  }
	      amt += (count - 1) * sizeof (asection *);
	      m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = PT_NOTE;
	      m->count = count;
	      while (count > 1)
		{
		  m->sections[m->count - count--] = s;
		  BFD_ASSERT ((s->flags & SEC_THREAD_LOCAL) == 0);
		  s = s->next;
		}
	      m->sections[m->count - 1] = s;
	      BFD_ASSERT ((s->flags & SEC_THREAD_LOCAL) == 0);
	      *pm = m;
	      pm = &m->next;
	    }
	  if (s->flags & SEC_THREAD_LOCAL)
	    {
	      if (! tls_count)
		first_tls = s;
	      tls_count++;
	    }
	}

      /* If there are any SHF_TLS output sections, add PT_TLS segment.  */
      if (tls_count > 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  amt += (tls_count - 1) * sizeof (asection *);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_TLS;
	  m->count = tls_count;
	  /* Mandated PF_R.  */
	  m->p_flags = PF_R;
	  m->p_flags_valid = 1;
	  s = first_tls;
	  for (i = 0; i < (unsigned int) tls_count; ++i)
	    {
	      if ((s->flags & SEC_THREAD_LOCAL) == 0)
		{
		  _bfd_error_handler
		    (_("%B: TLS sections are not adjacent:"), abfd);
		  s = first_tls;
		  i = 0;
		  while (i < (unsigned int) tls_count)
		    {
		      if ((s->flags & SEC_THREAD_LOCAL) != 0)
			{
			  _bfd_error_handler (_("	    TLS: %A"), s);
			  i++;
			}
		      else
			_bfd_error_handler (_("	non-TLS: %A"), s);
		      s = s->next;
		    }
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}
	      m->sections[i] = s;
	      s = s->next;
	    }

	  *pm = m;
	  pm = &m->next;
	}

      /* If there is a .eh_frame_hdr section, throw in a PT_GNU_EH_FRAME
	 segment.  */
      eh_frame_hdr = elf_eh_frame_hdr (abfd);
      if (eh_frame_hdr != NULL
	  && (eh_frame_hdr->output_section->flags & SEC_LOAD) != 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_EH_FRAME;
	  m->count = 1;
	  m->sections[0] = eh_frame_hdr->output_section;

	  *pm = m;
	  pm = &m->next;
	}

      if (elf_stack_flags (abfd))
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_STACK;
	  m->p_flags = elf_stack_flags (abfd);
	  m->p_align = bed->stack_align;
	  m->p_flags_valid = 1;
	  m->p_align_valid = m->p_align != 0;
	  if (info->stacksize > 0)
	    {
	      m->p_size = info->stacksize;
	      m->p_size_valid = 1;
	    }

	  *pm = m;
	  pm = &m->next;
	}

      if (info != NULL && info->relro)
	{
	  for (m = mfirst; m != NULL; m = m->next)
	    {
	      if (m->p_type == PT_LOAD
		  && m->count != 0
		  && m->sections[0]->vma >= info->relro_start
		  && m->sections[0]->vma < info->relro_end)
		{
		  i = m->count;
		  while (--i != (unsigned) -1)
		    if ((m->sections[i]->flags & (SEC_LOAD | SEC_HAS_CONTENTS))
			== (SEC_LOAD | SEC_HAS_CONTENTS))
		      break;

		  if (i != (unsigned) -1)
		    break;
		}
	    }

	  /* Make a PT_GNU_RELRO segment only when it isn't empty.  */
	  if (m != NULL)
	    {
	      amt = sizeof (struct elf_segment_map);
	      m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = PT_GNU_RELRO;
	      m->p_flags = PF_R;
	      m->p_flags_valid = 1;

	      *pm = m;
	      pm = &m->next;
	    }
	}

      free (sections);
      elf_seg_map (abfd) = mfirst;
    }

  if (!elf_modify_segment_map (abfd, info, no_user_phdrs))
    return FALSE;

  for (count = 0, m = elf_seg_map (abfd); m != NULL; m = m->next)
    ++count;
  elf_program_header_size (abfd) = count * bed->s->sizeof_phdr;

  return TRUE;

 error_return:
  if (sections != NULL)
    free (sections);
  return FALSE;
}

/* Sort sections by address.  */

static int
elf_sort_sections (const void *arg1, const void *arg2)
{
  const asection *sec1 = *(const asection **) arg1;
  const asection *sec2 = *(const asection **) arg2;
  bfd_size_type size1, size2;

  /* Sort by LMA first, since this is the address used to
     place the section into a segment.  */
  if (sec1->lma < sec2->lma)
    return -1;
  else if (sec1->lma > sec2->lma)
    return 1;

  /* Then sort by VMA.  Normally the LMA and the VMA will be
     the same, and this will do nothing.  */
  if (sec1->vma < sec2->vma)
    return -1;
  else if (sec1->vma > sec2->vma)
    return 1;

  /* Put !SEC_LOAD sections after SEC_LOAD ones.  */

#define TOEND(x) (((x)->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) == 0)

  if (TOEND (sec1))
    {
      if (TOEND (sec2))
	{
	  /* If the indicies are the same, do not return 0
	     here, but continue to try the next comparison.  */
	  if (sec1->target_index - sec2->target_index != 0)
	    return sec1->target_index - sec2->target_index;
	}
      else
	return 1;
    }
  else if (TOEND (sec2))
    return -1;

#undef TOEND

  /* Sort by size, to put zero sized sections
     before others at the same address.  */

  size1 = (sec1->flags & SEC_LOAD) ? sec1->size : 0;
  size2 = (sec2->flags & SEC_LOAD) ? sec2->size : 0;

  if (size1 < size2)
    return -1;
  if (size1 > size2)
    return 1;

  return sec1->target_index - sec2->target_index;
}

/* Ian Lance Taylor writes:

   We shouldn't be using % with a negative signed number.  That's just
   not good.  We have to make sure either that the number is not
   negative, or that the number has an unsigned type.  When the types
   are all the same size they wind up as unsigned.  When file_ptr is a
   larger signed type, the arithmetic winds up as signed long long,
   which is wrong.

   What we're trying to say here is something like ``increase OFF by
   the least amount that will cause it to be equal to the VMA modulo
   the page size.''  */
/* In other words, something like:

   vma_offset = m->sections[0]->vma % bed->maxpagesize;
   off_offset = off % bed->maxpagesize;
   if (vma_offset < off_offset)
     adjustment = vma_offset + bed->maxpagesize - off_offset;
   else
     adjustment = vma_offset - off_offset;

   which can can be collapsed into the expression below.  */

static file_ptr
vma_page_aligned_bias (bfd_vma vma, ufile_ptr off, bfd_vma maxpagesize)
{
  /* PR binutils/16199: Handle an alignment of zero.  */
  if (maxpagesize == 0)
    maxpagesize = 1;
  return ((vma - off) % maxpagesize);
}

static void
print_segment_map (const struct elf_segment_map *m)
{
  unsigned int j;
  const char *pt = get_segment_type (m->p_type);
  char buf[32];

  if (pt == NULL)
    {
      if (m->p_type >= PT_LOPROC && m->p_type <= PT_HIPROC)
	sprintf (buf, "LOPROC+%7.7x",
		 (unsigned int) (m->p_type - PT_LOPROC));
      else if (m->p_type >= PT_LOOS && m->p_type <= PT_HIOS)
	sprintf (buf, "LOOS+%7.7x",
		 (unsigned int) (m->p_type - PT_LOOS));
      else
	snprintf (buf, sizeof (buf), "%8.8x",
		  (unsigned int) m->p_type);
      pt = buf;
    }
  fflush (stdout);
  fprintf (stderr, "%s:", pt);
  for (j = 0; j < m->count; j++)
    fprintf (stderr, " %s", m->sections [j]->name);
  putc ('\n',stderr);
  fflush (stderr);
}

static bfd_boolean
write_zeros (bfd *abfd, file_ptr pos, bfd_size_type len)
{
  void *buf;
  bfd_boolean ret;

  if (bfd_seek (abfd, pos, SEEK_SET) != 0)
    return FALSE;
  buf = bfd_zmalloc (len);
  if (buf == NULL)
    return FALSE;
  ret = bfd_bwrite (buf, len, abfd) == len;
  free (buf);
  return ret;
}

/* Assign file positions to the sections based on the mapping from
   sections to segments.  This function also sets up some fields in
   the file header.  */

static bfd_boolean
assign_file_positions_for_load_sections (bfd *abfd,
					 struct bfd_link_info *link_info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_segment_map *m;
  Elf_Internal_Phdr *phdrs;
  Elf_Internal_Phdr *p;
  file_ptr off;
  bfd_size_type maxpagesize;
  unsigned int alloc;
  unsigned int i, j;
  bfd_vma header_pad = 0;

  if (link_info == NULL
      && !_bfd_elf_map_sections_to_segments (abfd, link_info))
    return FALSE;

  alloc = 0;
  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
    {
      ++alloc;
      if (m->header_size)
	header_pad = m->header_size;
    }

  if (alloc)
    {
      elf_elfheader (abfd)->e_phoff = bed->s->sizeof_ehdr;
      elf_elfheader (abfd)->e_phentsize = bed->s->sizeof_phdr;
    }
  else
    {
      /* PR binutils/12467.  */
      elf_elfheader (abfd)->e_phoff = 0;
      elf_elfheader (abfd)->e_phentsize = 0;
    }

  elf_elfheader (abfd)->e_phnum = alloc;

  if (elf_program_header_size (abfd) == (bfd_size_type) -1)
    elf_program_header_size (abfd) = alloc * bed->s->sizeof_phdr;
  else
    BFD_ASSERT (elf_program_header_size (abfd)
		>= alloc * bed->s->sizeof_phdr);

  if (alloc == 0)
    {
      elf_next_file_pos (abfd) = bed->s->sizeof_ehdr;
      return TRUE;
    }

  /* We're writing the size in elf_program_header_size (abfd),
     see assign_file_positions_except_relocs, so make sure we have
     that amount allocated, with trailing space cleared.
     The variable alloc contains the computed need, while
     elf_program_header_size (abfd) contains the size used for the
     layout.
     See ld/emultempl/elf-generic.em:gld${EMULATION_NAME}_map_segments
     where the layout is forced to according to a larger size in the
     last iterations for the testcase ld-elf/header.  */
  BFD_ASSERT (elf_program_header_size (abfd) % bed->s->sizeof_phdr
	      == 0);
  phdrs = (Elf_Internal_Phdr *)
     bfd_zalloc2 (abfd,
                  (elf_program_header_size (abfd) / bed->s->sizeof_phdr),
                  sizeof (Elf_Internal_Phdr));
  elf_tdata (abfd)->phdr = phdrs;
  if (phdrs == NULL)
    return FALSE;

  maxpagesize = 1;
  if ((abfd->flags & D_PAGED) != 0)
    maxpagesize = bed->maxpagesize;

  off = bed->s->sizeof_ehdr;
  off += alloc * bed->s->sizeof_phdr;
  if (header_pad < (bfd_vma) off)
    header_pad = 0;
  else
    header_pad -= off;
  off += header_pad;

  for (m = elf_seg_map (abfd), p = phdrs, j = 0;
       m != NULL;
       m = m->next, p++, j++)
    {
      asection **secpp;
      bfd_vma off_adjust;
      bfd_boolean no_contents;

      /* If elf_segment_map is not from map_sections_to_segments, the
	 sections may not be correctly ordered.  NOTE: sorting should
	 not be done to the PT_NOTE section of a corefile, which may
	 contain several pseudo-sections artificially created by bfd.
	 Sorting these pseudo-sections breaks things badly.  */
      if (m->count > 1
	  && !(elf_elfheader (abfd)->e_type == ET_CORE
	       && m->p_type == PT_NOTE))
	qsort (m->sections, (size_t) m->count, sizeof (asection *),
	       elf_sort_sections);

      /* An ELF segment (described by Elf_Internal_Phdr) may contain a
	 number of sections with contents contributing to both p_filesz
	 and p_memsz, followed by a number of sections with no contents
	 that just contribute to p_memsz.  In this loop, OFF tracks next
	 available file offset for PT_LOAD and PT_NOTE segments.  */
      p->p_type = m->p_type;
      p->p_flags = m->p_flags;

      if (m->count == 0)
	p->p_vaddr = 0;
      else
	p->p_vaddr = m->sections[0]->vma - m->p_vaddr_offset;

      if (m->p_paddr_valid)
	p->p_paddr = m->p_paddr;
      else if (m->count == 0)
	p->p_paddr = 0;
      else
	p->p_paddr = m->sections[0]->lma - m->p_vaddr_offset;

      if (p->p_type == PT_LOAD
	  && (abfd->flags & D_PAGED) != 0)
	{
	  /* p_align in demand paged PT_LOAD segments effectively stores
	     the maximum page size.  When copying an executable with
	     objcopy, we set m->p_align from the input file.  Use this
	     value for maxpagesize rather than bed->maxpagesize, which
	     may be different.  Note that we use maxpagesize for PT_TLS
	     segment alignment later in this function, so we are relying
	     on at least one PT_LOAD segment appearing before a PT_TLS
	     segment.  */
	  if (m->p_align_valid)
	    maxpagesize = m->p_align;

	  p->p_align = maxpagesize;
	}
      else if (m->p_align_valid)
	p->p_align = m->p_align;
      else if (m->count == 0)
	p->p_align = 1 << bed->s->log_file_align;
      else
	p->p_align = 0;

      no_contents = FALSE;
      off_adjust = 0;
      if (p->p_type == PT_LOAD
	  && m->count > 0)
	{
	  bfd_size_type align;
	  unsigned int align_power = 0;

	  if (m->p_align_valid)
	    align = p->p_align;
	  else
	    {
	      for (i = 0, secpp = m->sections; i < m->count; i++, secpp++)
		{
		  unsigned int secalign;

		  secalign = bfd_get_section_alignment (abfd, *secpp);
		  if (secalign > align_power)
		    align_power = secalign;
		}
	      align = (bfd_size_type) 1 << align_power;
	      if (align < maxpagesize)
		align = maxpagesize;
	    }

	  for (i = 0; i < m->count; i++)
	    if ((m->sections[i]->flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
	      /* If we aren't making room for this section, then
		 it must be SHT_NOBITS regardless of what we've
		 set via struct bfd_elf_special_section.  */
	      elf_section_type (m->sections[i]) = SHT_NOBITS;

	  /* Find out whether this segment contains any loadable
	     sections.  */
	  no_contents = TRUE;
	  for (i = 0; i < m->count; i++)
	    if (elf_section_type (m->sections[i]) != SHT_NOBITS)
	      {
		no_contents = FALSE;
		break;
	      }

	  off_adjust = vma_page_aligned_bias (p->p_vaddr, off, align);
	  off += off_adjust;
	  if (no_contents)
	    {
	      /* We shouldn't need to align the segment on disk since
		 the segment doesn't need file space, but the gABI
		 arguably requires the alignment and glibc ld.so
		 checks it.  So to comply with the alignment
		 requirement but not waste file space, we adjust
		 p_offset for just this segment.  (OFF_ADJUST is
		 subtracted from OFF later.)  This may put p_offset
		 past the end of file, but that shouldn't matter.  */
	    }
	  else
	    off_adjust = 0;
	}
      /* Make sure the .dynamic section is the first section in the
	 PT_DYNAMIC segment.  */
      else if (p->p_type == PT_DYNAMIC
	       && m->count > 1
	       && strcmp (m->sections[0]->name, ".dynamic") != 0)
	{
	  _bfd_error_handler
	    (_("%B: The first section in the PT_DYNAMIC segment is not the .dynamic section"),
	     abfd);
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}
      /* Set the note section type to SHT_NOTE.  */
      else if (p->p_type == PT_NOTE)
	for (i = 0; i < m->count; i++)
	  elf_section_type (m->sections[i]) = SHT_NOTE;

      p->p_offset = 0;
      p->p_filesz = 0;
      p->p_memsz = 0;

      if (m->includes_filehdr)
	{
	  if (!m->p_flags_valid)
	    p->p_flags |= PF_R;
	  p->p_filesz = bed->s->sizeof_ehdr;
	  p->p_memsz = bed->s->sizeof_ehdr;
	  if (m->count > 0)
	    {
	      if (p->p_vaddr < (bfd_vma) off)
		{
		  (*_bfd_error_handler)
		    (_("%B: Not enough room for program headers, try linking with -N"),
		     abfd);
		  bfd_set_error (bfd_error_bad_value);
		  return FALSE;
		}

	      p->p_vaddr -= off;
	      if (!m->p_paddr_valid)
		p->p_paddr -= off;
	    }
	}

      if (m->includes_phdrs)
	{
	  if (!m->p_flags_valid)
	    p->p_flags |= PF_R;

	  if (!m->includes_filehdr)
	    {
	      p->p_offset = bed->s->sizeof_ehdr;

	      if (m->count > 0)
		{
		  p->p_vaddr -= off - p->p_offset;
		  if (!m->p_paddr_valid)
		    p->p_paddr -= off - p->p_offset;
		}
	    }

	  p->p_filesz += alloc * bed->s->sizeof_phdr;
	  p->p_memsz += alloc * bed->s->sizeof_phdr;
	  if (m->count)
	    {
	      p->p_filesz += header_pad;
	      p->p_memsz += header_pad;
	    }
	}

      if (p->p_type == PT_LOAD
	  || (p->p_type == PT_NOTE && bfd_get_format (abfd) == bfd_core))
	{
	  if (!m->includes_filehdr && !m->includes_phdrs)
	    p->p_offset = off;
	  else
	    {
	      file_ptr adjust;

	      adjust = off - (p->p_offset + p->p_filesz);
	      if (!no_contents)
		p->p_filesz += adjust;
	      p->p_memsz += adjust;
	    }
	}

      /* Set up p_filesz, p_memsz, p_align and p_flags from the section
	 maps.  Set filepos for sections in PT_LOAD segments, and in
	 core files, for sections in PT_NOTE segments.
	 assign_file_positions_for_non_load_sections will set filepos
	 for other sections and update p_filesz for other segments.  */
      for (i = 0, secpp = m->sections; i < m->count; i++, secpp++)
	{
	  asection *sec;
	  bfd_size_type align;
	  Elf_Internal_Shdr *this_hdr;

	  sec = *secpp;
	  this_hdr = &elf_section_data (sec)->this_hdr;
	  align = (bfd_size_type) 1 << bfd_get_section_alignment (abfd, sec);

	  if ((p->p_type == PT_LOAD
	       || p->p_type == PT_TLS)
	      && (this_hdr->sh_type != SHT_NOBITS
		  || ((this_hdr->sh_flags & SHF_ALLOC) != 0
		      && ((this_hdr->sh_flags & SHF_TLS) == 0
			  || p->p_type == PT_TLS))))
	    {
	      bfd_vma p_start = p->p_paddr;
	      bfd_vma p_end = p_start + p->p_memsz;
	      bfd_vma s_start = sec->lma;
	      bfd_vma adjust = s_start - p_end;

	      if (adjust != 0
		  && (s_start < p_end
		      || p_end < p_start))
		{
		  (*_bfd_error_handler)
		    (_("%B: section %A lma %#lx adjusted to %#lx"), abfd, sec,
		     (unsigned long) s_start, (unsigned long) p_end);
		  adjust = 0;
		  sec->lma = p_end;
		}
	      p->p_memsz += adjust;

	      if (this_hdr->sh_type != SHT_NOBITS)
		{
		  if (p->p_filesz + adjust < p->p_memsz)
		    {
		      /* We have a PROGBITS section following NOBITS ones.
		         Allocate file space for the NOBITS section(s) and
			 zero it.  */
		      adjust = p->p_memsz - p->p_filesz;
		      if (!write_zeros (abfd, off, adjust))
			return FALSE;
		    }
		  off += adjust;
		  p->p_filesz += adjust;
		}
	    }

	  if (p->p_type == PT_NOTE && bfd_get_format (abfd) == bfd_core)
	    {
	      /* The section at i == 0 is the one that actually contains
		 everything.  */
	      if (i == 0)
		{
		  this_hdr->sh_offset = sec->filepos = off;
		  off += this_hdr->sh_size;
		  p->p_filesz = this_hdr->sh_size;
		  p->p_memsz = 0;
		  p->p_align = 1;
		}
	      else
		{
		  /* The rest are fake sections that shouldn't be written.  */
		  sec->filepos = 0;
		  sec->size = 0;
		  sec->flags = 0;
		  continue;
		}
	    }
	  else
	    {
	      if (p->p_type == PT_LOAD)
		{
		  this_hdr->sh_offset = sec->filepos = off;
		  if (this_hdr->sh_type != SHT_NOBITS)
		    off += this_hdr->sh_size;
		}
	      else if (this_hdr->sh_type == SHT_NOBITS
		       && (this_hdr->sh_flags & SHF_TLS) != 0
		       && this_hdr->sh_offset == 0)
		{
		  /* This is a .tbss section that didn't get a PT_LOAD.
		     (See _bfd_elf_map_sections_to_segments "Create a
		     final PT_LOAD".)  Set sh_offset to the value it
		     would have if we had created a zero p_filesz and
		     p_memsz PT_LOAD header for the section.  This
		     also makes the PT_TLS header have the same
		     p_offset value.  */
		  bfd_vma adjust = vma_page_aligned_bias (this_hdr->sh_addr,
							  off, align);
		  this_hdr->sh_offset = sec->filepos = off + adjust;
		}

	      if (this_hdr->sh_type != SHT_NOBITS)
		{
		  p->p_filesz += this_hdr->sh_size;
		  /* A load section without SHF_ALLOC is something like
		     a note section in a PT_NOTE segment.  These take
		     file space but are not loaded into memory.  */
		  if ((this_hdr->sh_flags & SHF_ALLOC) != 0)
		    p->p_memsz += this_hdr->sh_size;
		}
	      else if ((this_hdr->sh_flags & SHF_ALLOC) != 0)
		{
		  if (p->p_type == PT_TLS)
		    p->p_memsz += this_hdr->sh_size;

		  /* .tbss is special.  It doesn't contribute to p_memsz of
		     normal segments.  */
		  else if ((this_hdr->sh_flags & SHF_TLS) == 0)
		    p->p_memsz += this_hdr->sh_size;
		}

	      if (align > p->p_align
		  && !m->p_align_valid
		  && (p->p_type != PT_LOAD
		      || (abfd->flags & D_PAGED) == 0))
		p->p_align = align;
	    }

	  if (!m->p_flags_valid)
	    {
	      p->p_flags |= PF_R;
	      if ((this_hdr->sh_flags & SHF_EXECINSTR) != 0)
		p->p_flags |= PF_X;
	      if ((this_hdr->sh_flags & SHF_WRITE) != 0)
		p->p_flags |= PF_W;
	    }
	}

      off -= off_adjust;

      /* Check that all sections are in a PT_LOAD segment.
	 Don't check funky gdb generated core files.  */
      if (p->p_type == PT_LOAD && bfd_get_format (abfd) != bfd_core)
	{
	  bfd_boolean check_vma = TRUE;

	  for (i = 1; i < m->count; i++)
	    if (m->sections[i]->vma == m->sections[i - 1]->vma
		&& ELF_SECTION_SIZE (&(elf_section_data (m->sections[i])
				       ->this_hdr), p) != 0
		&& ELF_SECTION_SIZE (&(elf_section_data (m->sections[i - 1])
				       ->this_hdr), p) != 0)
	      {
		/* Looks like we have overlays packed into the segment.  */
		check_vma = FALSE;
		break;
	      }

	  for (i = 0; i < m->count; i++)
	    {
	      Elf_Internal_Shdr *this_hdr;
	      asection *sec;

	      sec = m->sections[i];
	      this_hdr = &(elf_section_data(sec)->this_hdr);
	      if (!ELF_SECTION_IN_SEGMENT_1 (this_hdr, p, check_vma, 0)
		  && !ELF_TBSS_SPECIAL (this_hdr, p))
		{
		  (*_bfd_error_handler)
		    (_("%B: section `%A' can't be allocated in segment %d"),
		     abfd, sec, j);
		  print_segment_map (m);
		}
	    }
	}
    }

  elf_next_file_pos (abfd) = off;
  return TRUE;
}

/* Assign file positions for the other sections.  */

static bfd_boolean
assign_file_positions_for_non_load_sections (bfd *abfd,
					     struct bfd_link_info *link_info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  Elf_Internal_Shdr **i_shdrpp;
  Elf_Internal_Shdr **hdrpp;
  Elf_Internal_Phdr *phdrs;
  Elf_Internal_Phdr *p;
  struct elf_segment_map *m;
  struct elf_segment_map *hdrs_segment;
  bfd_vma filehdr_vaddr, filehdr_paddr;
  bfd_vma phdrs_vaddr, phdrs_paddr;
  file_ptr off;
  unsigned int num_sec;
  unsigned int i;
  unsigned int count;

  i_shdrpp = elf_elfsections (abfd);
  num_sec = elf_numsections (abfd);
  off = elf_next_file_pos (abfd);
  for (i = 1, hdrpp = i_shdrpp + 1; i < num_sec; i++, hdrpp++)
    {
      Elf_Internal_Shdr *hdr;

      hdr = *hdrpp;
      if (hdr->bfd_section != NULL
	  && (hdr->bfd_section->filepos != 0
	      || (hdr->sh_type == SHT_NOBITS
		  && hdr->contents == NULL)))
	BFD_ASSERT (hdr->sh_offset == hdr->bfd_section->filepos);
      else if ((hdr->sh_flags & SHF_ALLOC) != 0)
	{
	  if (hdr->sh_size != 0)
	    (*_bfd_error_handler)
	      (_("%B: warning: allocated section `%s' not in segment"),
	       abfd,
	       (hdr->bfd_section == NULL
		? "*unknown*"
		: hdr->bfd_section->name));
	  /* We don't need to page align empty sections.  */
	  if ((abfd->flags & D_PAGED) != 0 && hdr->sh_size != 0)
	    off += vma_page_aligned_bias (hdr->sh_addr, off,
					  bed->maxpagesize);
	  else
	    off += vma_page_aligned_bias (hdr->sh_addr, off,
					  hdr->sh_addralign);
	  off = _bfd_elf_assign_file_position_for_section (hdr, off,
							   FALSE);
	}
      else if (((hdr->sh_type == SHT_REL || hdr->sh_type == SHT_RELA)
		&& hdr->bfd_section == NULL)
	       || hdr == i_shdrpp[elf_onesymtab (abfd)]
	       || hdr == i_shdrpp[elf_symtab_shndx (abfd)]
	       || hdr == i_shdrpp[elf_strtab_sec (abfd)])
	hdr->sh_offset = -1;
      else
	off = _bfd_elf_assign_file_position_for_section (hdr, off, TRUE);
    }

  /* Now that we have set the section file positions, we can set up
     the file positions for the non PT_LOAD segments.  */
  count = 0;
  filehdr_vaddr = 0;
  filehdr_paddr = 0;
  phdrs_vaddr = bed->maxpagesize + bed->s->sizeof_ehdr;
  phdrs_paddr = 0;
  hdrs_segment = NULL;
  phdrs = elf_tdata (abfd)->phdr;
  for (m = elf_seg_map (abfd), p = phdrs; m != NULL; m = m->next, p++)
    {
      ++count;
      if (p->p_type != PT_LOAD)
	continue;

      if (m->includes_filehdr)
	{
	  filehdr_vaddr = p->p_vaddr;
	  filehdr_paddr = p->p_paddr;
	}
      if (m->includes_phdrs)
	{
	  phdrs_vaddr = p->p_vaddr;
	  phdrs_paddr = p->p_paddr;
	  if (m->includes_filehdr)
	    {
	      hdrs_segment = m;
	      phdrs_vaddr += bed->s->sizeof_ehdr;
	      phdrs_paddr += bed->s->sizeof_ehdr;
	    }
	}
    }

  if (hdrs_segment != NULL && link_info != NULL)
    {
      /* There is a segment that contains both the file headers and the
	 program headers, so provide a symbol __ehdr_start pointing there.
	 A program can use this to examine itself robustly.  */

      struct elf_link_hash_entry *hash
	= elf_link_hash_lookup (elf_hash_table (link_info), "__ehdr_start",
				FALSE, FALSE, TRUE);
      /* If the symbol was referenced and not defined, define it.  */
      if (hash != NULL
	  && (hash->root.type == bfd_link_hash_new
	      || hash->root.type == bfd_link_hash_undefined
	      || hash->root.type == bfd_link_hash_undefweak
	      || hash->root.type == bfd_link_hash_common))
	{
	  asection *s = NULL;
	  if (hdrs_segment->count != 0)
	    /* The segment contains sections, so use the first one.  */
	    s = hdrs_segment->sections[0];
	  else
	    /* Use the first (i.e. lowest-addressed) section in any segment.  */
	    for (m = elf_seg_map (abfd); m != NULL; m = m->next)
	      if (m->count != 0)
		{
		  s = m->sections[0];
		  break;
		}

	  if (s != NULL)
	    {
	      hash->root.u.def.value = filehdr_vaddr - s->vma;
	      hash->root.u.def.section = s;
	    }
	  else
	    {
	      hash->root.u.def.value = filehdr_vaddr;
	      hash->root.u.def.section = bfd_abs_section_ptr;
	    }

	  hash->root.type = bfd_link_hash_defined;
	  hash->def_regular = 1;
	  hash->non_elf = 0;
	}
    }

  for (m = elf_seg_map (abfd), p = phdrs; m != NULL; m = m->next, p++)
    {
      if (p->p_type == PT_GNU_RELRO)
	{
	  const Elf_Internal_Phdr *lp;
	  struct elf_segment_map *lm;

	  if (link_info != NULL)
	    {
	      /* During linking the range of the RELRO segment is passed
		 in link_info.  */
	      for (lm = elf_seg_map (abfd), lp = phdrs;
		   lm != NULL;
		   lm = lm->next, lp++)
		{
		  if (lp->p_type == PT_LOAD
		      && lp->p_vaddr < link_info->relro_end
		      && lm->count != 0
		      && lm->sections[0]->vma >= link_info->relro_start)
		    break;
		}

	      BFD_ASSERT (lm != NULL);
	    }
	  else
	    {
	      /* Otherwise we are copying an executable or shared
		 library, but we need to use the same linker logic.  */
	      for (lp = phdrs; lp < phdrs + count; ++lp)
		{
		  if (lp->p_type == PT_LOAD
		      && lp->p_paddr == p->p_paddr)
		    break;
		}
	    }

	  if (lp < phdrs + count)
	    {
	      p->p_vaddr = lp->p_vaddr;
	      p->p_paddr = lp->p_paddr;
	      p->p_offset = lp->p_offset;
	      if (link_info != NULL)
		p->p_filesz = link_info->relro_end - lp->p_vaddr;
	      else if (m->p_size_valid)
		p->p_filesz = m->p_size;
	      else
		abort ();
	      p->p_memsz = p->p_filesz;
	      /* Preserve the alignment and flags if they are valid. The
	         gold linker generates RW/4 for the PT_GNU_RELRO section.
		 It is better for objcopy/strip to honor these attributes
		 otherwise gdb will choke when using separate debug files.
	       */
	      if (!m->p_align_valid)
		p->p_align = 1;
	      if (!m->p_flags_valid)
		p->p_flags = (lp->p_flags & ~PF_W);
	    }
	  else
	    {
	      memset (p, 0, sizeof *p);
	      p->p_type = PT_NULL;
	    }
	}
      else if (p->p_type == PT_GNU_STACK)
	{
	  if (m->p_size_valid)
	    p->p_memsz = m->p_size;
	}
      else if (m->count != 0)
	{
	  if (p->p_type != PT_LOAD
	      && (p->p_type != PT_NOTE
		  || bfd_get_format (abfd) != bfd_core))
	    {
	      if (m->includes_filehdr || m->includes_phdrs)
		{
		  /* PR 17512: file: 2195325e.  */ 
		  (*_bfd_error_handler)
		    (_("%B: warning: non-load segment includes file header and/or program header"),
		     abfd);
		  return FALSE;
		}

	      p->p_filesz = 0;
	      p->p_offset = m->sections[0]->filepos;
	      for (i = m->count; i-- != 0;)
		{
		  asection *sect = m->sections[i];
		  Elf_Internal_Shdr *hdr = &elf_section_data (sect)->this_hdr;
		  if (hdr->sh_type != SHT_NOBITS)
		    {
		      p->p_filesz = (sect->filepos - m->sections[0]->filepos
				     + hdr->sh_size);
		      break;
		    }
		}
	    }
	}
      else if (m->includes_filehdr)
	{
	  p->p_vaddr = filehdr_vaddr;
	  if (! m->p_paddr_valid)
	    p->p_paddr = filehdr_paddr;
	}
      else if (m->includes_phdrs)
	{
	  p->p_vaddr = phdrs_vaddr;
	  if (! m->p_paddr_valid)
	    p->p_paddr = phdrs_paddr;
	}
    }

  elf_next_file_pos (abfd) = off;

  return TRUE;
}

/* Work out the file positions of all the sections.  This is called by
   _bfd_elf_compute_section_file_positions.  All the section sizes and
   VMAs must be known before this is called.

   Reloc sections come in two flavours: Those processed specially as
   "side-channel" data attached to a section to which they apply, and
   those that bfd doesn't process as relocations.  The latter sort are
   stored in a normal bfd section by bfd_section_from_shdr.   We don't
   consider the former sort here, unless they form part of the loadable
   image.  Reloc sections not assigned here will be handled later by
   assign_file_positions_for_relocs.

   We also don't set the positions of the .symtab and .strtab here.  */

static bfd_boolean
assign_file_positions_except_relocs (bfd *abfd,
				     struct bfd_link_info *link_info)
{
  struct elf_obj_tdata *tdata = elf_tdata (abfd);
  Elf_Internal_Ehdr *i_ehdrp = elf_elfheader (abfd);
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if ((abfd->flags & (EXEC_P | DYNAMIC)) == 0
      && bfd_get_format (abfd) != bfd_core)
    {
      Elf_Internal_Shdr ** const i_shdrpp = elf_elfsections (abfd);
      unsigned int num_sec = elf_numsections (abfd);
      Elf_Internal_Shdr **hdrpp;
      unsigned int i;
      file_ptr off;

      /* Start after the ELF header.  */
      off = i_ehdrp->e_ehsize;

      /* We are not creating an executable, which means that we are
	 not creating a program header, and that the actual order of
	 the sections in the file is unimportant.  */
      for (i = 1, hdrpp = i_shdrpp + 1; i < num_sec; i++, hdrpp++)
	{
	  Elf_Internal_Shdr *hdr;

	  hdr = *hdrpp;
	  if (((hdr->sh_type == SHT_REL || hdr->sh_type == SHT_RELA)
	       && hdr->bfd_section == NULL)
	      || i == elf_onesymtab (abfd)
	      || i == elf_symtab_shndx (abfd)
	      || i == elf_strtab_sec (abfd))
	    {
	      hdr->sh_offset = -1;
	    }
	  else
	    off = _bfd_elf_assign_file_position_for_section (hdr, off, TRUE);
	}

      elf_next_file_pos (abfd) = off;
    }
  else
    {
      unsigned int alloc;

      /* Assign file positions for the loaded sections based on the
	 assignment of sections to segments.  */
      if (!assign_file_positions_for_load_sections (abfd, link_info))
	return FALSE;

      /* And for non-load sections.  */
      if (!assign_file_positions_for_non_load_sections (abfd, link_info))
	return FALSE;

      if (bed->elf_backend_modify_program_headers != NULL)
	{
	  if (!(*bed->elf_backend_modify_program_headers) (abfd, link_info))
	    return FALSE;
	}

      /* Set e_type in ELF header to ET_EXEC for -pie -Ttext-segment=.  */
      if (link_info != NULL
	  && link_info->executable
	  && link_info->shared)
	{
	  unsigned int num_segments = elf_elfheader (abfd)->e_phnum;
	  Elf_Internal_Phdr *segment = elf_tdata (abfd)->phdr;
	  Elf_Internal_Phdr *end_segment = &segment[num_segments];

	  /* Find the lowest p_vaddr in PT_LOAD segments.  */
	  bfd_vma p_vaddr = (bfd_vma) -1;
	  for (; segment < end_segment; segment++)
	    if (segment->p_type == PT_LOAD && p_vaddr > segment->p_vaddr)
	      p_vaddr = segment->p_vaddr;

	  /* Set e_type to ET_EXEC if the lowest p_vaddr in PT_LOAD
	     segments is non-zero.  */
	  if (p_vaddr)
	    i_ehdrp->e_type = ET_EXEC;
	}

      /* Write out the program headers.  */
      alloc = elf_program_header_size (abfd) / bed->s->sizeof_phdr;
      if (bfd_seek (abfd, (bfd_signed_vma) bed->s->sizeof_ehdr, SEEK_SET) != 0
	  || bed->s->write_out_phdrs (abfd, tdata->phdr, alloc) != 0)
	return FALSE;
    }

  return TRUE;
}

static bfd_boolean
prep_headers (bfd *abfd)
{
  Elf_Internal_Ehdr *i_ehdrp;	/* Elf file header, internal form.  */
  struct elf_strtab_hash *shstrtab;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  i_ehdrp = elf_elfheader (abfd);

  shstrtab = _bfd_elf_strtab_init ();
  if (shstrtab == NULL)
    return FALSE;

  elf_shstrtab (abfd) = shstrtab;

  i_ehdrp->e_ident[EI_MAG0] = ELFMAG0;
  i_ehdrp->e_ident[EI_MAG1] = ELFMAG1;
  i_ehdrp->e_ident[EI_MAG2] = ELFMAG2;
  i_ehdrp->e_ident[EI_MAG3] = ELFMAG3;

  i_ehdrp->e_ident[EI_CLASS] = bed->s->elfclass;
  i_ehdrp->e_ident[EI_DATA] =
    bfd_big_endian (abfd) ? ELFDATA2MSB : ELFDATA2LSB;
  i_ehdrp->e_ident[EI_VERSION] = bed->s->ev_current;

  if ((abfd->flags & DYNAMIC) != 0)
    i_ehdrp->e_type = ET_DYN;
  else if ((abfd->flags & EXEC_P) != 0)
    i_ehdrp->e_type = ET_EXEC;
  else if (bfd_get_format (abfd) == bfd_core)
    i_ehdrp->e_type = ET_CORE;
  else
    i_ehdrp->e_type = ET_REL;

  switch (bfd_get_arch (abfd))
    {
    case bfd_arch_unknown:
      i_ehdrp->e_machine = EM_NONE;
      break;

      /* There used to be a long list of cases here, each one setting
	 e_machine to the same EM_* macro #defined as ELF_MACHINE_CODE
	 in the corresponding bfd definition.  To avoid duplication,
	 the switch was removed.  Machines that need special handling
	 can generally do it in elf_backend_final_write_processing(),
	 unless they need the information earlier than the final write.
	 Such need can generally be supplied by replacing the tests for
	 e_machine with the conditions used to determine it.  */
    default:
      i_ehdrp->e_machine = bed->elf_machine_code;
    }

  i_ehdrp->e_version = bed->s->ev_current;
  i_ehdrp->e_ehsize = bed->s->sizeof_ehdr;

  /* No program header, for now.  */
  i_ehdrp->e_phoff = 0;
  i_ehdrp->e_phentsize = 0;
  i_ehdrp->e_phnum = 0;

  /* Each bfd section is section header entry.  */
  i_ehdrp->e_entry = bfd_get_start_address (abfd);
  i_ehdrp->e_shentsize = bed->s->sizeof_shdr;

  /* If we're building an executable, we'll need a program header table.  */
  if (abfd->flags & EXEC_P)
    /* It all happens later.  */
    ;
  else
    {
      i_ehdrp->e_phentsize = 0;
      i_ehdrp->e_phoff = 0;
    }

  elf_tdata (abfd)->symtab_hdr.sh_name =
    (unsigned int) _bfd_elf_strtab_add (shstrtab, ".symtab", FALSE);
  elf_tdata (abfd)->strtab_hdr.sh_name =
    (unsigned int) _bfd_elf_strtab_add (shstrtab, ".strtab", FALSE);
  elf_tdata (abfd)->shstrtab_hdr.sh_name =
    (unsigned int) _bfd_elf_strtab_add (shstrtab, ".shstrtab", FALSE);
  if (elf_tdata (abfd)->symtab_hdr.sh_name == (unsigned int) -1
      || elf_tdata (abfd)->strtab_hdr.sh_name == (unsigned int) -1
      || elf_tdata (abfd)->shstrtab_hdr.sh_name == (unsigned int) -1)
    return FALSE;

  return TRUE;
}

/* Assign file positions for all the reloc sections which are not part
   of the loadable file image, and the file position of section headers.  */

static void
_bfd_elf_assign_file_positions_for_relocs (bfd *abfd)
{
  file_ptr off;
  unsigned int i, num_sec;
  Elf_Internal_Shdr **shdrpp;
  Elf_Internal_Ehdr *i_ehdrp;
  const struct elf_backend_data *bed;

  off = elf_next_file_pos (abfd);

  num_sec = elf_numsections (abfd);
  for (i = 1, shdrpp = elf_elfsections (abfd) + 1; i < num_sec; i++, shdrpp++)
    {
      Elf_Internal_Shdr *shdrp;

      shdrp = *shdrpp;
      if ((shdrp->sh_type == SHT_REL || shdrp->sh_type == SHT_RELA)
	  && shdrp->sh_offset == -1)
	off = _bfd_elf_assign_file_position_for_section (shdrp, off, TRUE);
    }

/* Place the section headers.  */
  i_ehdrp = elf_elfheader (abfd);
  bed = get_elf_backend_data (abfd);
  off = align_file_position (off, 1 << bed->s->log_file_align);
  i_ehdrp->e_shoff = off;
  off += i_ehdrp->e_shnum * i_ehdrp->e_shentsize;
  elf_next_file_pos (abfd) = off;
}

bfd_boolean
_bfd_elf_write_object_contents (bfd *abfd)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  Elf_Internal_Shdr **i_shdrp;
  bfd_boolean failed;
  unsigned int count, num_sec;
  struct elf_obj_tdata *t;

  if (! abfd->output_has_begun
      && ! _bfd_elf_compute_section_file_positions (abfd, NULL))
    return FALSE;

  i_shdrp = elf_elfsections (abfd);

  failed = FALSE;
  bfd_map_over_sections (abfd, bed->s->write_relocs, &failed);
  if (failed)
    return FALSE;

  _bfd_elf_assign_file_positions_for_relocs (abfd);

  /* After writing the headers, we need to write the sections too...  */
  num_sec = elf_numsections (abfd);
  for (count = 1; count < num_sec; count++)
    {
      if (bed->elf_backend_section_processing)
	(*bed->elf_backend_section_processing) (abfd, i_shdrp[count]);
      if (i_shdrp[count]->contents)
	{
	  bfd_size_type amt = i_shdrp[count]->sh_size;

	  if (bfd_seek (abfd, i_shdrp[count]->sh_offset, SEEK_SET) != 0
	      || bfd_bwrite (i_shdrp[count]->contents, amt, abfd) != amt)
	    return FALSE;
	}
    }

  /* Write out the section header names.  */
  t = elf_tdata (abfd);
  if (elf_shstrtab (abfd) != NULL
      && (bfd_seek (abfd, t->shstrtab_hdr.sh_offset, SEEK_SET) != 0
	  || !_bfd_elf_strtab_emit (abfd, elf_shstrtab (abfd))))
    return FALSE;

  if (bed->elf_backend_final_write_processing)
    (*bed->elf_backend_final_write_processing) (abfd, elf_linker (abfd));

  if (!bed->s->write_shdrs_and_ehdr (abfd))
    return FALSE;

  /* This is last since write_shdrs_and_ehdr can touch i_shdrp[0].  */
  if (t->o->build_id.after_write_object_contents != NULL)
    return (*t->o->build_id.after_write_object_contents) (abfd);

  return TRUE;
}

bfd_boolean
_bfd_elf_write_corefile_contents (bfd *abfd)
{
  /* Hopefully this can be done just like an object file.  */
  return _bfd_elf_write_object_contents (abfd);
}

/* Given a section, search the header to find them.  */

unsigned int
_bfd_elf_section_from_bfd_section (bfd *abfd, struct bfd_section *asect)
{
  const struct elf_backend_data *bed;
  unsigned int sec_index;

  if (elf_section_data (asect) != NULL
      && elf_section_data (asect)->this_idx != 0)
    return elf_section_data (asect)->this_idx;

  if (bfd_is_abs_section (asect))
    sec_index = SHN_ABS;
  else if (bfd_is_com_section (asect))
    sec_index = SHN_COMMON;
  else if (bfd_is_und_section (asect))
    sec_index = SHN_UNDEF;
  else
    sec_index = SHN_BAD;

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_section_from_bfd_section)
    {
      int retval = sec_index;

      if ((*bed->elf_backend_section_from_bfd_section) (abfd, asect, &retval))
	return retval;
    }

  if (sec_index == SHN_BAD)
    bfd_set_error (bfd_error_nonrepresentable_section);

  return sec_index;
}

/* Given a BFD symbol, return the index in the ELF symbol table, or -1
   on error.  */

int
_bfd_elf_symbol_from_bfd_symbol (bfd *abfd, asymbol **asym_ptr_ptr)
{
  asymbol *asym_ptr = *asym_ptr_ptr;
  int idx;
  flagword flags = asym_ptr->flags;

  /* When gas creates relocations against local labels, it creates its
     own symbol for the section, but does put the symbol into the
     symbol chain, so udata is 0.  When the linker is generating
     relocatable output, this section symbol may be for one of the
     input sections rather than the output section.  */
  if (asym_ptr->udata.i == 0
      && (flags & BSF_SECTION_SYM)
      && asym_ptr->section)
    {
      asection *sec;
      int indx;

      sec = asym_ptr->section;
      if (sec->owner != abfd && sec->output_section != NULL)
	sec = sec->output_section;
      if (sec->owner == abfd
	  && (indx = sec->index) < elf_num_section_syms (abfd)
	  && elf_section_syms (abfd)[indx] != NULL)
	asym_ptr->udata.i = elf_section_syms (abfd)[indx]->udata.i;
    }

  idx = asym_ptr->udata.i;

  if (idx == 0)
    {
      /* This case can occur when using --strip-symbol on a symbol
	 which is used in a relocation entry.  */
      (*_bfd_error_handler)
	(_("%B: symbol `%s' required but not present"),
	 abfd, bfd_asymbol_name (asym_ptr));
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

#if DEBUG & 4
  {
    fprintf (stderr,
	     "elf_symbol_from_bfd_symbol 0x%.8lx, name = %s, sym num = %d, flags = 0x%.8lx\n",
	     (long) asym_ptr, asym_ptr->name, idx, (long) flags);
    fflush (stderr);
  }
#endif

  return idx;
}

/* Rewrite program header information.  */

static bfd_boolean
rewrite_elf_program_header (bfd *ibfd, bfd *obfd)
{
  Elf_Internal_Ehdr *iehdr;
  struct elf_segment_map *map;
  struct elf_segment_map *map_first;
  struct elf_segment_map **pointer_to_map;
  Elf_Internal_Phdr *segment;
  asection *section;
  unsigned int i;
  unsigned int num_segments;
  bfd_boolean phdr_included = FALSE;
  bfd_boolean p_paddr_valid;
  bfd_vma maxpagesize;
  struct elf_segment_map *phdr_adjust_seg = NULL;
  unsigned int phdr_adjust_num = 0;
  const struct elf_backend_data *bed;

  bed = get_elf_backend_data (ibfd);
  iehdr = elf_elfheader (ibfd);

  map_first = NULL;
  pointer_to_map = &map_first;

  num_segments = elf_elfheader (ibfd)->e_phnum;
  maxpagesize = get_elf_backend_data (obfd)->maxpagesize;

  /* Returns the end address of the segment + 1.  */
#define SEGMENT_END(segment, start)					\
  (start + (segment->p_memsz > segment->p_filesz			\
	    ? segment->p_memsz : segment->p_filesz))

#define SECTION_SIZE(section, segment)					\
  (((section->flags & (SEC_HAS_CONTENTS | SEC_THREAD_LOCAL))		\
    != SEC_THREAD_LOCAL || segment->p_type == PT_TLS)			\
   ? section->size : 0)

  /* Returns TRUE if the given section is contained within
     the given segment.  VMA addresses are compared.  */
#define IS_CONTAINED_BY_VMA(section, segment)				\
  (section->vma >= segment->p_vaddr					\
   && (section->vma + SECTION_SIZE (section, segment)			\
       <= (SEGMENT_END (segment, segment->p_vaddr))))

  /* Returns TRUE if the given section is contained within
     the given segment.  LMA addresses are compared.  */
#define IS_CONTAINED_BY_LMA(section, segment, base)			\
  (section->lma >= base							\
   && (section->lma + SECTION_SIZE (section, segment)			\
       <= SEGMENT_END (segment, base)))

  /* Handle PT_NOTE segment.  */
#define IS_NOTE(p, s)							\
  (p->p_type == PT_NOTE							\
   && elf_section_type (s) == SHT_NOTE					\
   && (bfd_vma) s->filepos >= p->p_offset				\
   && ((bfd_vma) s->filepos + s->size					\
       <= p->p_offset + p->p_filesz))

  /* Special case: corefile "NOTE" section containing regs, prpsinfo
     etc.  */
#define IS_COREFILE_NOTE(p, s)						\
  (IS_NOTE (p, s)							\
   && bfd_get_format (ibfd) == bfd_core					\
   && s->vma == 0							\
   && s->lma == 0)

  /* The complicated case when p_vaddr is 0 is to handle the Solaris
     linker, which generates a PT_INTERP section with p_vaddr and
     p_memsz set to 0.  */
#define IS_SOLARIS_PT_INTERP(p, s)					\
  (p->p_vaddr == 0							\
   && p->p_paddr == 0							\
   && p->p_memsz == 0							\
   && p->p_filesz > 0							\
   && (s->flags & SEC_HAS_CONTENTS) != 0				\
   && s->size > 0							\
   && (bfd_vma) s->filepos >= p->p_offset				\
   && ((bfd_vma) s->filepos + s->size					\
       <= p->p_offset + p->p_filesz))

  /* Decide if the given section should be included in the given segment.
     A section will be included if:
       1. It is within the address space of the segment -- we use the LMA
	  if that is set for the segment and the VMA otherwise,
       2. It is an allocated section or a NOTE section in a PT_NOTE
	  segment.
       3. There is an output section associated with it,
       4. The section has not already been allocated to a previous segment.
       5. PT_GNU_STACK segments do not include any sections.
       6. PT_TLS segment includes only SHF_TLS sections.
       7. SHF_TLS sections are only in PT_TLS or PT_LOAD segments.
       8. PT_DYNAMIC should not contain empty sections at the beginning
	  (with the possible exception of .dynamic).  */
#define IS_SECTION_IN_INPUT_SEGMENT(section, segment, bed)		\
  ((((segment->p_paddr							\
      ? IS_CONTAINED_BY_LMA (section, segment, segment->p_paddr)	\
      : IS_CONTAINED_BY_VMA (section, segment))				\
     && (section->flags & SEC_ALLOC) != 0)				\
    || IS_NOTE (segment, section))					\
   && segment->p_type != PT_GNU_STACK					\
   && (segment->p_type != PT_TLS					\
       || (section->flags & SEC_THREAD_LOCAL))				\
   && (segment->p_type == PT_LOAD					\
       || segment->p_type == PT_TLS					\
       || (section->flags & SEC_THREAD_LOCAL) == 0)			\
   && (segment->p_type != PT_DYNAMIC					\
       || SECTION_SIZE (section, segment) > 0				\
       || (segment->p_paddr						\
	   ? segment->p_paddr != section->lma				\
	   : segment->p_vaddr != section->vma)				\
       || (strcmp (bfd_get_section_name (ibfd, section), ".dynamic")	\
	   == 0))							\
   && !section->segment_mark)

/* If the output section of a section in the input segment is NULL,
   it is removed from the corresponding output segment.   */
#define INCLUDE_SECTION_IN_SEGMENT(section, segment, bed)		\
  (IS_SECTION_IN_INPUT_SEGMENT (section, segment, bed)		\
   && section->output_section != NULL)

  /* Returns TRUE iff seg1 starts after the end of seg2.  */
#define SEGMENT_AFTER_SEGMENT(seg1, seg2, field)			\
  (seg1->field >= SEGMENT_END (seg2, seg2->field))

  /* Returns TRUE iff seg1 and seg2 overlap. Segments overlap iff both
     their VMA address ranges and their LMA address ranges overlap.
     It is possible to have overlapping VMA ranges without overlapping LMA
     ranges.  RedBoot images for example can have both .data and .bss mapped
     to the same VMA range, but with the .data section mapped to a different
     LMA.  */
#define SEGMENT_OVERLAPS(seg1, seg2)					\
  (   !(SEGMENT_AFTER_SEGMENT (seg1, seg2, p_vaddr)			\
	|| SEGMENT_AFTER_SEGMENT (seg2, seg1, p_vaddr))			\
   && !(SEGMENT_AFTER_SEGMENT (seg1, seg2, p_paddr)			\
	|| SEGMENT_AFTER_SEGMENT (seg2, seg1, p_paddr)))

  /* Initialise the segment mark field.  */
  for (section = ibfd->sections; section != NULL; section = section->next)
    section->segment_mark = FALSE;

  /* The Solaris linker creates program headers in which all the
     p_paddr fields are zero.  When we try to objcopy or strip such a
     file, we get confused.  Check for this case, and if we find it
     don't set the p_paddr_valid fields.  */
  p_paddr_valid = FALSE;
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    if (segment->p_paddr != 0)
      {
	p_paddr_valid = TRUE;
	break;
      }

  /* Scan through the segments specified in the program header
     of the input BFD.  For this first scan we look for overlaps
     in the loadable segments.  These can be created by weird
     parameters to objcopy.  Also, fix some solaris weirdness.  */
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      unsigned int j;
      Elf_Internal_Phdr *segment2;

      if (segment->p_type == PT_INTERP)
	for (section = ibfd->sections; section; section = section->next)
	  if (IS_SOLARIS_PT_INTERP (segment, section))
	    {
	      /* Mininal change so that the normal section to segment
		 assignment code will work.  */
	      segment->p_vaddr = section->vma;
	      break;
	    }

      if (segment->p_type != PT_LOAD)
	{
	  /* Remove PT_GNU_RELRO segment.  */
	  if (segment->p_type == PT_GNU_RELRO)
	    segment->p_type = PT_NULL;
	  continue;
	}

      /* Determine if this segment overlaps any previous segments.  */
      for (j = 0, segment2 = elf_tdata (ibfd)->phdr; j < i; j++, segment2++)
	{
	  bfd_signed_vma extra_length;

	  if (segment2->p_type != PT_LOAD
	      || !SEGMENT_OVERLAPS (segment, segment2))
	    continue;

	  /* Merge the two segments together.  */
	  if (segment2->p_vaddr < segment->p_vaddr)
	    {
	      /* Extend SEGMENT2 to include SEGMENT and then delete
		 SEGMENT.  */
	      extra_length = (SEGMENT_END (segment, segment->p_vaddr)
			      - SEGMENT_END (segment2, segment2->p_vaddr));

	      if (extra_length > 0)
		{
		  segment2->p_memsz += extra_length;
		  segment2->p_filesz += extra_length;
		}

	      segment->p_type = PT_NULL;

	      /* Since we have deleted P we must restart the outer loop.  */
	      i = 0;
	      segment = elf_tdata (ibfd)->phdr;
	      break;
	    }
	  else
	    {
	      /* Extend SEGMENT to include SEGMENT2 and then delete
		 SEGMENT2.  */
	      extra_length = (SEGMENT_END (segment2, segment2->p_vaddr)
			      - SEGMENT_END (segment, segment->p_vaddr));

	      if (extra_length > 0)
		{
		  segment->p_memsz += extra_length;
		  segment->p_filesz += extra_length;
		}

	      segment2->p_type = PT_NULL;
	    }
	}
    }

  /* The second scan attempts to assign sections to segments.  */
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      unsigned int section_count;
      asection **sections;
      asection *output_section;
      unsigned int isec;
      bfd_vma matching_lma;
      bfd_vma suggested_lma;
      unsigned int j;
      bfd_size_type amt;
      asection *first_section;
      bfd_boolean first_matching_lma;
      bfd_boolean first_suggested_lma;

      if (segment->p_type == PT_NULL)
	continue;

      first_section = NULL;
      /* Compute how many sections might be placed into this segment.  */
      for (section = ibfd->sections, section_count = 0;
	   section != NULL;
	   section = section->next)
	{
	  /* Find the first section in the input segment, which may be
	     removed from the corresponding output segment.   */
	  if (IS_SECTION_IN_INPUT_SEGMENT (section, segment, bed))
	    {
	      if (first_section == NULL)
		first_section = section;
	      if (section->output_section != NULL)
		++section_count;
	    }
	}

      /* Allocate a segment map big enough to contain
	 all of the sections we have selected.  */
      amt = sizeof (struct elf_segment_map);
      amt += ((bfd_size_type) section_count - 1) * sizeof (asection *);
      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
      if (map == NULL)
	return FALSE;

      /* Initialise the fields of the segment map.  Default to
	 using the physical address of the segment in the input BFD.  */
      map->next = NULL;
      map->p_type = segment->p_type;
      map->p_flags = segment->p_flags;
      map->p_flags_valid = 1;

      /* If the first section in the input segment is removed, there is
	 no need to preserve segment physical address in the corresponding
	 output segment.  */
      if (!first_section || first_section->output_section != NULL)
	{
	  map->p_paddr = segment->p_paddr;
	  map->p_paddr_valid = p_paddr_valid;
	}

      /* Determine if this segment contains the ELF file header
	 and if it contains the program headers themselves.  */
      map->includes_filehdr = (segment->p_offset == 0
			       && segment->p_filesz >= iehdr->e_ehsize);
      map->includes_phdrs = 0;

      if (!phdr_included || segment->p_type != PT_LOAD)
	{
	  map->includes_phdrs =
	    (segment->p_offset <= (bfd_vma) iehdr->e_phoff
	     && (segment->p_offset + segment->p_filesz
		 >= ((bfd_vma) iehdr->e_phoff
		     + iehdr->e_phnum * iehdr->e_phentsize)));

	  if (segment->p_type == PT_LOAD && map->includes_phdrs)
	    phdr_included = TRUE;
	}

      if (section_count == 0)
	{
	  /* Special segments, such as the PT_PHDR segment, may contain
	     no sections, but ordinary, loadable segments should contain
	     something.  They are allowed by the ELF spec however, so only
	     a warning is produced.  */
	  if (segment->p_type == PT_LOAD)
	    (*_bfd_error_handler) (_("%B: warning: Empty loadable segment"
				     " detected, is this intentional ?\n"),
				   ibfd);

	  map->count = 0;
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;

	  continue;
	}

      /* Now scan the sections in the input BFD again and attempt
	 to add their corresponding output sections to the segment map.
	 The problem here is how to handle an output section which has
	 been moved (ie had its LMA changed).  There are four possibilities:

	 1. None of the sections have been moved.
	    In this case we can continue to use the segment LMA from the
	    input BFD.

	 2. All of the sections have been moved by the same amount.
	    In this case we can change the segment's LMA to match the LMA
	    of the first section.

	 3. Some of the sections have been moved, others have not.
	    In this case those sections which have not been moved can be
	    placed in the current segment which will have to have its size,
	    and possibly its LMA changed, and a new segment or segments will
	    have to be created to contain the other sections.

	 4. The sections have been moved, but not by the same amount.
	    In this case we can change the segment's LMA to match the LMA
	    of the first section and we will have to create a new segment
	    or segments to contain the other sections.

	 In order to save time, we allocate an array to hold the section
	 pointers that we are interested in.  As these sections get assigned
	 to a segment, they are removed from this array.  */

      sections = (asection **) bfd_malloc2 (section_count, sizeof (asection *));
      if (sections == NULL)
	return FALSE;

      /* Step One: Scan for segment vs section LMA conflicts.
	 Also add the sections to the section array allocated above.
	 Also add the sections to the current segment.  In the common
	 case, where the sections have not been moved, this means that
	 we have completely filled the segment, and there is nothing
	 more to do.  */
      isec = 0;
      matching_lma = 0;
      suggested_lma = 0;
      first_matching_lma = TRUE;
      first_suggested_lma = TRUE;

      for (section = ibfd->sections;
	   section != NULL;
	   section = section->next)
	if (section == first_section)
	  break;

      for (j = 0; section != NULL; section = section->next)
	{
	  if (INCLUDE_SECTION_IN_SEGMENT (section, segment, bed))
	    {
	      output_section = section->output_section;

	      sections[j++] = section;

	      /* The Solaris native linker always sets p_paddr to 0.
		 We try to catch that case here, and set it to the
		 correct value.  Note - some backends require that
		 p_paddr be left as zero.  */
	      if (!p_paddr_valid
		  && segment->p_vaddr != 0
		  && !bed->want_p_paddr_set_to_zero
		  && isec == 0
		  && output_section->lma != 0
		  && output_section->vma == (segment->p_vaddr
					     + (map->includes_filehdr
						? iehdr->e_ehsize
						: 0)
					     + (map->includes_phdrs
						? (iehdr->e_phnum
						   * iehdr->e_phentsize)
						: 0)))
		map->p_paddr = segment->p_vaddr;

	      /* Match up the physical address of the segment with the
		 LMA address of the output section.  */
	      if (IS_CONTAINED_BY_LMA (output_section, segment, map->p_paddr)
		  || IS_COREFILE_NOTE (segment, section)
		  || (bed->want_p_paddr_set_to_zero
		      && IS_CONTAINED_BY_VMA (output_section, segment)))
		{
		  if (first_matching_lma || output_section->lma < matching_lma)
		    {
		      matching_lma = output_section->lma;
		      first_matching_lma = FALSE;
		    }

		  /* We assume that if the section fits within the segment
		     then it does not overlap any other section within that
		     segment.  */
		  map->sections[isec++] = output_section;
		}
	      else if (first_suggested_lma)
		{
		  suggested_lma = output_section->lma;
		  first_suggested_lma = FALSE;
		}

	      if (j == section_count)
		break;
	    }
	}

      BFD_ASSERT (j == section_count);

      /* Step Two: Adjust the physical address of the current segment,
	 if necessary.  */
      if (isec == section_count)
	{
	  /* All of the sections fitted within the segment as currently
	     specified.  This is the default case.  Add the segment to
	     the list of built segments and carry on to process the next
	     program header in the input BFD.  */
	  map->count = section_count;
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;

	  if (p_paddr_valid
	      && !bed->want_p_paddr_set_to_zero
	      && matching_lma != map->p_paddr
	      && !map->includes_filehdr
	      && !map->includes_phdrs)
	    /* There is some padding before the first section in the
	       segment.  So, we must account for that in the output
	       segment's vma.  */
	    map->p_vaddr_offset = matching_lma - map->p_paddr;

	  free (sections);
	  continue;
	}
      else
	{
	  if (!first_matching_lma)
	    {
	      /* At least one section fits inside the current segment.
		 Keep it, but modify its physical address to match the
		 LMA of the first section that fitted.  */
	      map->p_paddr = matching_lma;
	    }
	  else
	    {
	      /* None of the sections fitted inside the current segment.
		 Change the current segment's physical address to match
		 the LMA of the first section.  */
	      map->p_paddr = suggested_lma;
	    }

	  /* Offset the segment physical address from the lma
	     to allow for space taken up by elf headers.  */
	  if (map->includes_filehdr)
	    {
	      if (map->p_paddr >= iehdr->e_ehsize)
		map->p_paddr -= iehdr->e_ehsize;
	      else
		{
		  map->includes_filehdr = FALSE;
		  map->includes_phdrs = FALSE;
		}
	    }

	  if (map->includes_phdrs)
	    {
	      if (map->p_paddr >= iehdr->e_phnum * iehdr->e_phentsize)
		{
		  map->p_paddr -= iehdr->e_phnum * iehdr->e_phentsize;

		  /* iehdr->e_phnum is just an estimate of the number
		     of program headers that we will need.  Make a note
		     here of the number we used and the segment we chose
		     to hold these headers, so that we can adjust the
		     offset when we know the correct value.  */
		  phdr_adjust_num = iehdr->e_phnum;
		  phdr_adjust_seg = map;
		}
	      else
		map->includes_phdrs = FALSE;
	    }
	}

      /* Step Three: Loop over the sections again, this time assigning
	 those that fit to the current segment and removing them from the
	 sections array; but making sure not to leave large gaps.  Once all
	 possible sections have been assigned to the current segment it is
	 added to the list of built segments and if sections still remain
	 to be assigned, a new segment is constructed before repeating
	 the loop.  */
      isec = 0;
      do
	{
	  map->count = 0;
	  suggested_lma = 0;
	  first_suggested_lma = TRUE;

	  /* Fill the current segment with sections that fit.  */
	  for (j = 0; j < section_count; j++)
	    {
	      section = sections[j];

	      if (section == NULL)
		continue;

	      output_section = section->output_section;

	      BFD_ASSERT (output_section != NULL);

	      if (IS_CONTAINED_BY_LMA (output_section, segment, map->p_paddr)
		  || IS_COREFILE_NOTE (segment, section))
		{
		  if (map->count == 0)
		    {
		      /* If the first section in a segment does not start at
			 the beginning of the segment, then something is
			 wrong.  */
		      if (output_section->lma
			  != (map->p_paddr
			      + (map->includes_filehdr ? iehdr->e_ehsize : 0)
			      + (map->includes_phdrs
				 ? iehdr->e_phnum * iehdr->e_phentsize
				 : 0)))
			abort ();
		    }
		  else
		    {
		      asection *prev_sec;

		      prev_sec = map->sections[map->count - 1];

		      /* If the gap between the end of the previous section
			 and the start of this section is more than
			 maxpagesize then we need to start a new segment.  */
		      if ((BFD_ALIGN (prev_sec->lma + prev_sec->size,
				      maxpagesize)
			   < BFD_ALIGN (output_section->lma, maxpagesize))
			  || (prev_sec->lma + prev_sec->size
			      > output_section->lma))
			{
			  if (first_suggested_lma)
			    {
			      suggested_lma = output_section->lma;
			      first_suggested_lma = FALSE;
			    }

			  continue;
			}
		    }

		  map->sections[map->count++] = output_section;
		  ++isec;
		  sections[j] = NULL;
		  section->segment_mark = TRUE;
		}
	      else if (first_suggested_lma)
		{
		  suggested_lma = output_section->lma;
		  first_suggested_lma = FALSE;
		}
	    }

	  BFD_ASSERT (map->count > 0);

	  /* Add the current segment to the list of built segments.  */
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;

	  if (isec < section_count)
	    {
	      /* We still have not allocated all of the sections to
		 segments.  Create a new segment here, initialise it
		 and carry on looping.  */
	      amt = sizeof (struct elf_segment_map);
	      amt += ((bfd_size_type) section_count - 1) * sizeof (asection *);
	      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
	      if (map == NULL)
		{
		  free (sections);
		  return FALSE;
		}

	      /* Initialise the fields of the segment map.  Set the physical
		 physical address to the LMA of the first section that has
		 not yet been assigned.  */
	      map->next = NULL;
	      map->p_type = segment->p_type;
	      map->p_flags = segment->p_flags;
	      map->p_flags_valid = 1;
	      map->p_paddr = suggested_lma;
	      map->p_paddr_valid = p_paddr_valid;
	      map->includes_filehdr = 0;
	      map->includes_phdrs = 0;
	    }
	}
      while (isec < section_count);

      free (sections);
    }

  elf_seg_map (obfd) = map_first;

  /* If we had to estimate the number of program headers that were
     going to be needed, then check our estimate now and adjust
     the offset if necessary.  */
  if (phdr_adjust_seg != NULL)
    {
      unsigned int count;

      for (count = 0, map = map_first; map != NULL; map = map->next)
	count++;

      if (count > phdr_adjust_num)
	phdr_adjust_seg->p_paddr
	  -= (count - phdr_adjust_num) * iehdr->e_phentsize;
    }

#undef SEGMENT_END
#undef SECTION_SIZE
#undef IS_CONTAINED_BY_VMA
#undef IS_CONTAINED_BY_LMA
#undef IS_NOTE
#undef IS_COREFILE_NOTE
#undef IS_SOLARIS_PT_INTERP
#undef IS_SECTION_IN_INPUT_SEGMENT
#undef INCLUDE_SECTION_IN_SEGMENT
#undef SEGMENT_AFTER_SEGMENT
#undef SEGMENT_OVERLAPS
  return TRUE;
}

/* Copy ELF program header information.  */

static bfd_boolean
copy_elf_program_header (bfd *ibfd, bfd *obfd)
{
  Elf_Internal_Ehdr *iehdr;
  struct elf_segment_map *map;
  struct elf_segment_map *map_first;
  struct elf_segment_map **pointer_to_map;
  Elf_Internal_Phdr *segment;
  unsigned int i;
  unsigned int num_segments;
  bfd_boolean phdr_included = FALSE;
  bfd_boolean p_paddr_valid;

  iehdr = elf_elfheader (ibfd);

  map_first = NULL;
  pointer_to_map = &map_first;

  /* If all the segment p_paddr fields are zero, don't set
     map->p_paddr_valid.  */
  p_paddr_valid = FALSE;
  num_segments = elf_elfheader (ibfd)->e_phnum;
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    if (segment->p_paddr != 0)
      {
	p_paddr_valid = TRUE;
	break;
      }

  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      asection *section;
      unsigned int section_count;
      bfd_size_type amt;
      Elf_Internal_Shdr *this_hdr;
      asection *first_section = NULL;
      asection *lowest_section;

      /* Compute how many sections are in this segment.  */
      for (section = ibfd->sections, section_count = 0;
	   section != NULL;
	   section = section->next)
	{
	  this_hdr = &(elf_section_data(section)->this_hdr);
	  if (ELF_SECTION_IN_SEGMENT (this_hdr, segment))
	    {
	      if (first_section == NULL)
		first_section = section;
	      section_count++;
	    }
	}

      /* Allocate a segment map big enough to contain
	 all of the sections we have selected.  */
      amt = sizeof (struct elf_segment_map);
      if (section_count != 0)
	amt += ((bfd_size_type) section_count - 1) * sizeof (asection *);
      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
      if (map == NULL)
	return FALSE;

      /* Initialize the fields of the output segment map with the
	 input segment.  */
      map->next = NULL;
      map->p_type = segment->p_type;
      map->p_flags = segment->p_flags;
      map->p_flags_valid = 1;
      map->p_paddr = segment->p_paddr;
      map->p_paddr_valid = p_paddr_valid;
      map->p_align = segment->p_align;
      map->p_align_valid = 1;
      map->p_vaddr_offset = 0;

      if (map->p_type == PT_GNU_RELRO
	  || map->p_type == PT_GNU_STACK)
	{
	  /* The PT_GNU_RELRO segment may contain the first a few
	     bytes in the .got.plt section even if the whole .got.plt
	     section isn't in the PT_GNU_RELRO segment.  We won't
	     change the size of the PT_GNU_RELRO segment.
	     Similarly, PT_GNU_STACK size is significant on uclinux
	     systems.    */
	  map->p_size = segment->p_memsz;
	  map->p_size_valid = 1;
	}

      /* Determine if this segment contains the ELF file header
	 and if it contains the program headers themselves.  */
      map->includes_filehdr = (segment->p_offset == 0
			       && segment->p_filesz >= iehdr->e_ehsize);

      map->includes_phdrs = 0;
      if (! phdr_included || segment->p_type != PT_LOAD)
	{
	  map->includes_phdrs =
	    (segment->p_offset <= (bfd_vma) iehdr->e_phoff
	     && (segment->p_offset + segment->p_filesz
		 >= ((bfd_vma) iehdr->e_phoff
		     + iehdr->e_phnum * iehdr->e_phentsize)));

	  if (segment->p_type == PT_LOAD && map->includes_phdrs)
	    phdr_included = TRUE;
	}

      lowest_section = NULL;
      if (section_count != 0)
	{
	  unsigned int isec = 0;

	  for (section = first_section;
	       section != NULL;
	       section = section->next)
	    {
	      this_hdr = &(elf_section_data(section)->this_hdr);
	      if (ELF_SECTION_IN_SEGMENT (this_hdr, segment))
		{
		  map->sections[isec++] = section->output_section;
		  if ((section->flags & SEC_ALLOC) != 0)
		    {
		      bfd_vma seg_off;

		      if (lowest_section == NULL
			  || section->lma < lowest_section->lma)
			lowest_section = section;

		      /* Section lmas are set up from PT_LOAD header
			 p_paddr in _bfd_elf_make_section_from_shdr.
			 If this header has a p_paddr that disagrees
			 with the section lma, flag the p_paddr as
			 invalid.  */
		      if ((section->flags & SEC_LOAD) != 0)
			seg_off = this_hdr->sh_offset - segment->p_offset;
		      else
			seg_off = this_hdr->sh_addr - segment->p_vaddr;
		      if (section->lma - segment->p_paddr != seg_off)
			map->p_paddr_valid = FALSE;
		    }
		  if (isec == section_count)
		    break;
		}
	    }
	}

      if (map->includes_filehdr && lowest_section != NULL)
	/* We need to keep the space used by the headers fixed.  */
	map->header_size = lowest_section->vma - segment->p_vaddr;

      if (!map->includes_phdrs
	  && !map->includes_filehdr
	  && map->p_paddr_valid)
	/* There is some other padding before the first section.  */
	map->p_vaddr_offset = ((lowest_section ? lowest_section->lma : 0)
			       - segment->p_paddr);

      map->count = section_count;
      *pointer_to_map = map;
      pointer_to_map = &map->next;
    }

  elf_seg_map (obfd) = map_first;
  return TRUE;
}

/* Copy private BFD data.  This copies or rewrites ELF program header
   information.  */

static bfd_boolean
copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  if (elf_tdata (ibfd)->phdr == NULL)
    return TRUE;

  if (ibfd->xvec == obfd->xvec)
    {
      /* Check to see if any sections in the input BFD
	 covered by ELF program header have changed.  */
      Elf_Internal_Phdr *segment;
      asection *section, *osec;
      unsigned int i, num_segments;
      Elf_Internal_Shdr *this_hdr;
      const struct elf_backend_data *bed;

      bed = get_elf_backend_data (ibfd);

      /* Regenerate the segment map if p_paddr is set to 0.  */
      if (bed->want_p_paddr_set_to_zero)
	goto rewrite;

      /* Initialize the segment mark field.  */
      for (section = obfd->sections; section != NULL;
	   section = section->next)
	section->segment_mark = FALSE;

      num_segments = elf_elfheader (ibfd)->e_phnum;
      for (i = 0, segment = elf_tdata (ibfd)->phdr;
	   i < num_segments;
	   i++, segment++)
	{
	  /* PR binutils/3535.  The Solaris linker always sets the p_paddr
	     and p_memsz fields of special segments (DYNAMIC, INTERP) to 0
	     which severly confuses things, so always regenerate the segment
	     map in this case.  */
	  if (segment->p_paddr == 0
	      && segment->p_memsz == 0
	      && (segment->p_type == PT_INTERP || segment->p_type == PT_DYNAMIC))
	    goto rewrite;

	  for (section = ibfd->sections;
	       section != NULL; section = section->next)
	    {
	      /* We mark the output section so that we know it comes
		 from the input BFD.  */
	      osec = section->output_section;
	      if (osec)
		osec->segment_mark = TRUE;

	      /* Check if this section is covered by the segment.  */
	      this_hdr = &(elf_section_data(section)->this_hdr);
	      if (ELF_SECTION_IN_SEGMENT (this_hdr, segment))
		{
		  /* FIXME: Check if its output section is changed or
		     removed.  What else do we need to check?  */
		  if (osec == NULL
		      || section->flags != osec->flags
		      || section->lma != osec->lma
		      || section->vma != osec->vma
		      || section->size != osec->size
		      || section->rawsize != osec->rawsize
		      || section->alignment_power != osec->alignment_power)
		    goto rewrite;
		}
	    }
	}

      /* Check to see if any output section do not come from the
	 input BFD.  */
      for (section = obfd->sections; section != NULL;
	   section = section->next)
	{
	  if (section->segment_mark == FALSE)
	    goto rewrite;
	  else
	    section->segment_mark = FALSE;
	}

      return copy_elf_program_header (ibfd, obfd);
    }

rewrite:
  if (ibfd->xvec == obfd->xvec)
    {
      /* When rewriting program header, set the output maxpagesize to
	 the maximum alignment of input PT_LOAD segments.  */
      Elf_Internal_Phdr *segment;
      unsigned int i;
      unsigned int num_segments = elf_elfheader (ibfd)->e_phnum;
      bfd_vma maxpagesize = 0;

      for (i = 0, segment = elf_tdata (ibfd)->phdr;
	   i < num_segments;
	   i++, segment++)
	if (segment->p_type == PT_LOAD
	    && maxpagesize < segment->p_align)
	  {
	    /* PR 17512: file: f17299af.  */
	    if (segment->p_align > (bfd_vma) 1 << ((sizeof (bfd_vma) * 8) - 2))
	      (*_bfd_error_handler) (_("\
%B: warning: segment alignment of 0x%llx is too large"),
				     ibfd, (long long) segment->p_align);
	    else
	      maxpagesize = segment->p_align;
	  }

      if (maxpagesize != get_elf_backend_data (obfd)->maxpagesize)
	bfd_emul_set_maxpagesize (bfd_get_target (obfd), maxpagesize);
    }

  return rewrite_elf_program_header (ibfd, obfd);
}

/* Initialize private output section information from input section.  */

bfd_boolean
_bfd_elf_init_private_section_data (bfd *ibfd,
				    asection *isec,
				    bfd *obfd,
				    asection *osec,
				    struct bfd_link_info *link_info)

{
  Elf_Internal_Shdr *ihdr, *ohdr;
  bfd_boolean final_link = link_info != NULL && !link_info->relocatable;

  if (ibfd->xvec->flavour != bfd_target_elf_flavour
      || obfd->xvec->flavour != bfd_target_elf_flavour)
    return TRUE;

  BFD_ASSERT (elf_section_data (osec) != NULL);

  /* For objcopy and relocatable link, don't copy the output ELF
     section type from input if the output BFD section flags have been
     set to something different.  For a final link allow some flags
     that the linker clears to differ.  */
  if (elf_section_type (osec) == SHT_NULL
      && (osec->flags == isec->flags
	  || (final_link
	      && ((osec->flags ^ isec->flags)
		  & ~(SEC_LINK_ONCE | SEC_LINK_DUPLICATES | SEC_RELOC)) == 0)))
    elf_section_type (osec) = elf_section_type (isec);

  /* FIXME: Is this correct for all OS/PROC specific flags?  */
  elf_section_flags (osec) |= (elf_section_flags (isec)
			       & (SHF_MASKOS | SHF_MASKPROC));

  /* Set things up for objcopy and relocatable link.  The output
     SHT_GROUP section will have its elf_next_in_group pointing back
     to the input group members.  Ignore linker created group section.
     See elfNN_ia64_object_p in elfxx-ia64.c.  */
  if (!final_link)
    {
      if (elf_sec_group (isec) == NULL
	  || (elf_sec_group (isec)->flags & SEC_LINKER_CREATED) == 0)
	{
	  if (elf_section_flags (isec) & SHF_GROUP)
	    elf_section_flags (osec) |= SHF_GROUP;
	  elf_next_in_group (osec) = elf_next_in_group (isec);
	  elf_section_data (osec)->group = elf_section_data (isec)->group;
	}
    }

  ihdr = &elf_section_data (isec)->this_hdr;

  /* We need to handle elf_linked_to_section for SHF_LINK_ORDER. We
     don't use the output section of the linked-to section since it
     may be NULL at this point.  */
  if ((ihdr->sh_flags & SHF_LINK_ORDER) != 0)
    {
      ohdr = &elf_section_data (osec)->this_hdr;
      ohdr->sh_flags |= SHF_LINK_ORDER;
      elf_linked_to_section (osec) = elf_linked_to_section (isec);
    }

  osec->use_rela_p = isec->use_rela_p;

  return TRUE;
}

/* Copy private section information.  This copies over the entsize
   field, and sometimes the info field.  */

bfd_boolean
_bfd_elf_copy_private_section_data (bfd *ibfd,
				    asection *isec,
				    bfd *obfd,
				    asection *osec)
{
  Elf_Internal_Shdr *ihdr, *ohdr;

  if (ibfd->xvec->flavour != bfd_target_elf_flavour
      || obfd->xvec->flavour != bfd_target_elf_flavour)
    return TRUE;

  ihdr = &elf_section_data (isec)->this_hdr;
  ohdr = &elf_section_data (osec)->this_hdr;

  ohdr->sh_entsize = ihdr->sh_entsize;

  if (ihdr->sh_type == SHT_SYMTAB
      || ihdr->sh_type == SHT_DYNSYM
      || ihdr->sh_type == SHT_GNU_verneed
      || ihdr->sh_type == SHT_GNU_verdef)
    ohdr->sh_info = ihdr->sh_info;

  return _bfd_elf_init_private_section_data (ibfd, isec, obfd, osec,
					     NULL);
}

/* Look at all the SHT_GROUP sections in IBFD, making any adjustments
   necessary if we are removing either the SHT_GROUP section or any of
   the group member sections.  DISCARDED is the value that a section's
   output_section has if the section will be discarded, NULL when this
   function is called from objcopy, bfd_abs_section_ptr when called
   from the linker.  */

bfd_boolean
_bfd_elf_fixup_group_sections (bfd *ibfd, asection *discarded)
{
  asection *isec;

  for (isec = ibfd->sections; isec != NULL; isec = isec->next)
    if (elf_section_type (isec) == SHT_GROUP)
      {
	asection *first = elf_next_in_group (isec);
	asection *s = first;
	bfd_size_type removed = 0;

	while (s != NULL)
	  {
	    /* If this member section is being output but the
	       SHT_GROUP section is not, then clear the group info
	       set up by _bfd_elf_copy_private_section_data.  */
	    if (s->output_section != discarded
		&& isec->output_section == discarded)
	      {
		elf_section_flags (s->output_section) &= ~SHF_GROUP;
		elf_group_name (s->output_section) = NULL;
	      }
	    /* Conversely, if the member section is not being output
	       but the SHT_GROUP section is, then adjust its size.  */
	    else if (s->output_section == discarded
		     && isec->output_section != discarded)
	      removed += 4;
	    s = elf_next_in_group (s);
	    if (s == first)
	      break;
	  }
	if (removed != 0)
	  {
	    if (discarded != NULL)
	      {
		/* If we've been called for ld -r, then we need to
		   adjust the input section size.  This function may
		   be called multiple times, so save the original
		   size.  */
		if (isec->rawsize == 0)
		  isec->rawsize = isec->size;
		isec->size = isec->rawsize - removed;
	      }
	    else
	      {
		/* Adjust the output section size when called from
		   objcopy. */
		isec->output_section->size -= removed;
	      }
	  }
      }

  return TRUE;
}

/* Copy private header information.  */

bfd_boolean
_bfd_elf_copy_private_header_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  /* Copy over private BFD data if it has not already been copied.
     This must be done here, rather than in the copy_private_bfd_data
     entry point, because the latter is called after the section
     contents have been set, which means that the program headers have
     already been worked out.  */
  if (elf_seg_map (obfd) == NULL && elf_tdata (ibfd)->phdr != NULL)
    {
      if (! copy_private_bfd_data (ibfd, obfd))
	return FALSE;
    }

  return _bfd_elf_fixup_group_sections (ibfd, NULL);
}

/* Copy private symbol information.  If this symbol is in a section
   which we did not map into a BFD section, try to map the section
   index correctly.  We use special macro definitions for the mapped
   section indices; these definitions are interpreted by the
   swap_out_syms function.  */

#define MAP_ONESYMTAB (SHN_HIOS + 1)
#define MAP_DYNSYMTAB (SHN_HIOS + 2)
#define MAP_STRTAB    (SHN_HIOS + 3)
#define MAP_SHSTRTAB  (SHN_HIOS + 4)
#define MAP_SYM_SHNDX (SHN_HIOS + 5)

bfd_boolean
_bfd_elf_copy_private_symbol_data (bfd *ibfd,
				   asymbol *isymarg,
				   bfd *obfd,
				   asymbol *osymarg)
{
  elf_symbol_type *isym, *osym;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  isym = elf_symbol_from (ibfd, isymarg);
  osym = elf_symbol_from (obfd, osymarg);

  if (isym != NULL
      && isym->internal_elf_sym.st_shndx != 0
      && osym != NULL
      && bfd_is_abs_section (isym->symbol.section))
    {
      unsigned int shndx;

      shndx = isym->internal_elf_sym.st_shndx;
      if (shndx == elf_onesymtab (ibfd))
	shndx = MAP_ONESYMTAB;
      else if (shndx == elf_dynsymtab (ibfd))
	shndx = MAP_DYNSYMTAB;
      else if (shndx == elf_strtab_sec (ibfd))
	shndx = MAP_STRTAB;
      else if (shndx == elf_shstrtab_sec (ibfd))
	shndx = MAP_SHSTRTAB;
      else if (shndx == elf_symtab_shndx (ibfd))
	shndx = MAP_SYM_SHNDX;
      osym->internal_elf_sym.st_shndx = shndx;
    }

  return TRUE;
}

/* Swap out the symbols.  */

static bfd_boolean
swap_out_syms (bfd *abfd,
	       struct bfd_strtab_hash **sttp,
	       int relocatable_p)
{
  const struct elf_backend_data *bed;
  int symcount;
  asymbol **syms;
  struct bfd_strtab_hash *stt;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Shdr *symtab_shndx_hdr;
  Elf_Internal_Shdr *symstrtab_hdr;
  bfd_byte *outbound_syms;
  bfd_byte *outbound_shndx;
  int idx;
  unsigned int num_locals;
  bfd_size_type amt;
  bfd_boolean name_local_sections;

  if (!elf_map_symbols (abfd, &num_locals))
    return FALSE;

  /* Dump out the symtabs.  */
  stt = _bfd_elf_stringtab_init ();
  if (stt == NULL)
    return FALSE;

  bed = get_elf_backend_data (abfd);
  symcount = bfd_get_symcount (abfd);
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  symtab_hdr->sh_type = SHT_SYMTAB;
  symtab_hdr->sh_entsize = bed->s->sizeof_sym;
  symtab_hdr->sh_size = symtab_hdr->sh_entsize * (symcount + 1);
  symtab_hdr->sh_info = num_locals + 1;
  symtab_hdr->sh_addralign = (bfd_vma) 1 << bed->s->log_file_align;

  symstrtab_hdr = &elf_tdata (abfd)->strtab_hdr;
  symstrtab_hdr->sh_type = SHT_STRTAB;

  outbound_syms = (bfd_byte *) bfd_alloc2 (abfd, 1 + symcount,
                                           bed->s->sizeof_sym);
  if (outbound_syms == NULL)
    {
      _bfd_stringtab_free (stt);
      return FALSE;
    }
  symtab_hdr->contents = outbound_syms;

  outbound_shndx = NULL;
  symtab_shndx_hdr = &elf_tdata (abfd)->symtab_shndx_hdr;
  if (symtab_shndx_hdr->sh_name != 0)
    {
      amt = (bfd_size_type) (1 + symcount) * sizeof (Elf_External_Sym_Shndx);
      outbound_shndx =  (bfd_byte *)
          bfd_zalloc2 (abfd, 1 + symcount, sizeof (Elf_External_Sym_Shndx));
      if (outbound_shndx == NULL)
	{
	  _bfd_stringtab_free (stt);
	  return FALSE;
	}

      symtab_shndx_hdr->contents = outbound_shndx;
      symtab_shndx_hdr->sh_type = SHT_SYMTAB_SHNDX;
      symtab_shndx_hdr->sh_size = amt;
      symtab_shndx_hdr->sh_addralign = sizeof (Elf_External_Sym_Shndx);
      symtab_shndx_hdr->sh_entsize = sizeof (Elf_External_Sym_Shndx);
    }

  /* Now generate the data (for "contents").  */
  {
    /* Fill in zeroth symbol and swap it out.  */
    Elf_Internal_Sym sym;
    sym.st_name = 0;
    sym.st_value = 0;
    sym.st_size = 0;
    sym.st_info = 0;
    sym.st_other = 0;
    sym.st_shndx = SHN_UNDEF;
    sym.st_target_internal = 0;
    bed->s->swap_symbol_out (abfd, &sym, outbound_syms, outbound_shndx);
    outbound_syms += bed->s->sizeof_sym;
    if (outbound_shndx != NULL)
      outbound_shndx += sizeof (Elf_External_Sym_Shndx);
  }

  name_local_sections
    = (bed->elf_backend_name_local_section_symbols
       && bed->elf_backend_name_local_section_symbols (abfd));

  syms = bfd_get_outsymbols (abfd);
  for (idx = 0; idx < symcount; idx++)
    {
      Elf_Internal_Sym sym;
      bfd_vma value = syms[idx]->value;
      elf_symbol_type *type_ptr;
      flagword flags = syms[idx]->flags;
      int type;

      if (!name_local_sections
	  && (flags & (BSF_SECTION_SYM | BSF_GLOBAL)) == BSF_SECTION_SYM)
	{
	  /* Local section symbols have no name.  */
	  sym.st_name = 0;
	}
      else
	{
	  sym.st_name = (unsigned long) _bfd_stringtab_add (stt,
							    syms[idx]->name,
							    TRUE, FALSE);
	  if (sym.st_name == (unsigned long) -1)
	    {
	      _bfd_stringtab_free (stt);
	      return FALSE;
	    }
	}

      type_ptr = elf_symbol_from (abfd, syms[idx]);

      if ((flags & BSF_SECTION_SYM) == 0
	  && bfd_is_com_section (syms[idx]->section))
	{
	  /* ELF common symbols put the alignment into the `value' field,
	     and the size into the `size' field.  This is backwards from
	     how BFD handles it, so reverse it here.  */
	  sym.st_size = value;
	  if (type_ptr == NULL
	      || type_ptr->internal_elf_sym.st_value == 0)
	    sym.st_value = value >= 16 ? 16 : (1 << bfd_log2 (value));
	  else
	    sym.st_value = type_ptr->internal_elf_sym.st_value;
	  sym.st_shndx = _bfd_elf_section_from_bfd_section
	    (abfd, syms[idx]->section);
	}
      else
	{
	  asection *sec = syms[idx]->section;
	  unsigned int shndx;

	  if (sec->output_section)
	    {
	      value += sec->output_offset;
	      sec = sec->output_section;
	    }

	  /* Don't add in the section vma for relocatable output.  */
	  if (! relocatable_p)
	    value += sec->vma;
	  sym.st_value = value;
	  sym.st_size = type_ptr ? type_ptr->internal_elf_sym.st_size : 0;

	  if (bfd_is_abs_section (sec)
	      && type_ptr != NULL
	      && type_ptr->internal_elf_sym.st_shndx != 0)
	    {
	      /* This symbol is in a real ELF section which we did
		 not create as a BFD section.  Undo the mapping done
		 by copy_private_symbol_data.  */
	      shndx = type_ptr->internal_elf_sym.st_shndx;
	      switch (shndx)
		{
		case MAP_ONESYMTAB:
		  shndx = elf_onesymtab (abfd);
		  break;
		case MAP_DYNSYMTAB:
		  shndx = elf_dynsymtab (abfd);
		  break;
		case MAP_STRTAB:
		  shndx = elf_strtab_sec (abfd);
		  break;
		case MAP_SHSTRTAB:
		  shndx = elf_shstrtab_sec (abfd);
		  break;
		case MAP_SYM_SHNDX:
		  shndx = elf_symtab_shndx (abfd);
		  break;
		default:
		  shndx = SHN_ABS;
		  break;
		}
	    }
	  else
	    {
	      shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

	      if (shndx == SHN_BAD)
		{
		  asection *sec2;

		  /* Writing this would be a hell of a lot easier if
		     we had some decent documentation on bfd, and
		     knew what to expect of the library, and what to
		     demand of applications.  For example, it
		     appears that `objcopy' might not set the
		     section of a symbol to be a section that is
		     actually in the output file.  */
		  sec2 = bfd_get_section_by_name (abfd, sec->name);
		  if (sec2 == NULL)
		    {
		      _bfd_error_handler (_("\
Unable to find equivalent output section for symbol '%s' from section '%s'"),
					  syms[idx]->name ? syms[idx]->name : "<Local sym>",
					  sec->name);
		      bfd_set_error (bfd_error_invalid_operation);
		      _bfd_stringtab_free (stt);
		      return FALSE;
		    }

		  shndx = _bfd_elf_section_from_bfd_section (abfd, sec2);
		  BFD_ASSERT (shndx != SHN_BAD);
		}
	    }

	  sym.st_shndx = shndx;
	}

      if ((flags & BSF_THREAD_LOCAL) != 0)
	type = STT_TLS;
      else if ((flags & BSF_GNU_INDIRECT_FUNCTION) != 0)
	type = STT_GNU_IFUNC;
      else if ((flags & BSF_FUNCTION) != 0)
	type = STT_FUNC;
      else if ((flags & BSF_OBJECT) != 0)
	type = STT_OBJECT;
      else if ((flags & BSF_RELC) != 0)
	type = STT_RELC;
      else if ((flags & BSF_SRELC) != 0)
	type = STT_SRELC;
      else
	type = STT_NOTYPE;

      if (syms[idx]->section->flags & SEC_THREAD_LOCAL)
	type = STT_TLS;

      /* Processor-specific types.  */
      if (type_ptr != NULL
	  && bed->elf_backend_get_symbol_type)
	type = ((*bed->elf_backend_get_symbol_type)
		(&type_ptr->internal_elf_sym, type));

      if (flags & BSF_SECTION_SYM)
	{
	  if (flags & BSF_GLOBAL)
	    sym.st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
	  else
	    sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_SECTION);
	}
      else if (bfd_is_com_section (syms[idx]->section))
	{
#ifdef USE_STT_COMMON
	  if (type == STT_OBJECT)
	    sym.st_info = ELF_ST_INFO (STB_GLOBAL, STT_COMMON);
	  else
#endif
	    sym.st_info = ELF_ST_INFO (STB_GLOBAL, type);
	}
      else if (bfd_is_und_section (syms[idx]->section))
	sym.st_info = ELF_ST_INFO (((flags & BSF_WEAK)
				    ? STB_WEAK
				    : STB_GLOBAL),
				   type);
      else if (flags & BSF_FILE)
	sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_FILE);
      else
	{
	  int bind = STB_LOCAL;

	  if (flags & BSF_LOCAL)
	    bind = STB_LOCAL;
	  else if (flags & BSF_GNU_UNIQUE)
	    bind = STB_GNU_UNIQUE;
	  else if (flags & BSF_WEAK)
	    bind = STB_WEAK;
	  else if (flags & BSF_GLOBAL)
	    bind = STB_GLOBAL;

	  sym.st_info = ELF_ST_INFO (bind, type);
	}

      if (type_ptr != NULL)
	{
	  sym.st_other = type_ptr->internal_elf_sym.st_other;
	  sym.st_target_internal
	    = type_ptr->internal_elf_sym.st_target_internal;
	}
      else
	{
	  sym.st_other = 0;
	  sym.st_target_internal = 0;
	}

      bed->s->swap_symbol_out (abfd, &sym, outbound_syms, outbound_shndx);
      outbound_syms += bed->s->sizeof_sym;
      if (outbound_shndx != NULL)
	outbound_shndx += sizeof (Elf_External_Sym_Shndx);
    }

  *sttp = stt;
  symstrtab_hdr->sh_size = _bfd_stringtab_size (stt);
  symstrtab_hdr->sh_type = SHT_STRTAB;

  symstrtab_hdr->sh_flags = 0;
  symstrtab_hdr->sh_addr = 0;
  symstrtab_hdr->sh_entsize = 0;
  symstrtab_hdr->sh_link = 0;
  symstrtab_hdr->sh_info = 0;
  symstrtab_hdr->sh_addralign = 1;

  return TRUE;
}

/* Return the number of bytes required to hold the symtab vector.

   Note that we base it on the count plus 1, since we will null terminate
   the vector allocated based on this size.  However, the ELF symbol table
   always has a dummy entry as symbol #0, so it ends up even.  */

long
_bfd_elf_get_symtab_upper_bound (bfd *abfd)
{
  long symcount;
  long symtab_size;
  Elf_Internal_Shdr *hdr = &elf_tdata (abfd)->symtab_hdr;

  symcount = hdr->sh_size / get_elf_backend_data (abfd)->s->sizeof_sym;
  symtab_size = (symcount + 1) * (sizeof (asymbol *));
  if (symcount > 0)
    symtab_size -= sizeof (asymbol *);

  return symtab_size;
}

long
_bfd_elf_get_dynamic_symtab_upper_bound (bfd *abfd)
{
  long symcount;
  long symtab_size;
  Elf_Internal_Shdr *hdr = &elf_tdata (abfd)->dynsymtab_hdr;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  symcount = hdr->sh_size / get_elf_backend_data (abfd)->s->sizeof_sym;
  symtab_size = (symcount + 1) * (sizeof (asymbol *));
  if (symcount > 0)
    symtab_size -= sizeof (asymbol *);

  return symtab_size;
}

long
_bfd_elf_get_reloc_upper_bound (bfd *abfd ATTRIBUTE_UNUSED,
				sec_ptr asect)
{
  return (asect->reloc_count + 1) * sizeof (arelent *);
}

/* Canonicalize the relocs.  */

long
_bfd_elf_canonicalize_reloc (bfd *abfd,
			     sec_ptr section,
			     arelent **relptr,
			     asymbol **symbols)
{
  arelent *tblptr;
  unsigned int i;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (! bed->s->slurp_reloc_table (abfd, section, symbols, FALSE))
    return -1;

  tblptr = section->relocation;
  for (i = 0; i < section->reloc_count; i++)
    *relptr++ = tblptr++;

  *relptr = NULL;

  return section->reloc_count;
}

long
_bfd_elf_canonicalize_symtab (bfd *abfd, asymbol **allocation)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  long symcount = bed->s->slurp_symbol_table (abfd, allocation, FALSE);

  if (symcount >= 0)
    bfd_get_symcount (abfd) = symcount;
  return symcount;
}

long
_bfd_elf_canonicalize_dynamic_symtab (bfd *abfd,
				      asymbol **allocation)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  long symcount = bed->s->slurp_symbol_table (abfd, allocation, TRUE);

  if (symcount >= 0)
    bfd_get_dynamic_symcount (abfd) = symcount;
  return symcount;
}

/* Return the size required for the dynamic reloc entries.  Any loadable
   section that was actually installed in the BFD, and has type SHT_REL
   or SHT_RELA, and uses the dynamic symbol table, is considered to be a
   dynamic reloc section.  */

long
_bfd_elf_get_dynamic_reloc_upper_bound (bfd *abfd)
{
  long ret;
  asection *s;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  ret = sizeof (arelent *);
  for (s = abfd->sections; s != NULL; s = s->next)
    if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	&& (elf_section_data (s)->this_hdr.sh_type == SHT_REL
	    || elf_section_data (s)->this_hdr.sh_type == SHT_RELA))
      ret += ((s->size / elf_section_data (s)->this_hdr.sh_entsize)
	      * sizeof (arelent *));

  return ret;
}

/* Canonicalize the dynamic relocation entries.  Note that we return the
   dynamic relocations as a single block, although they are actually
   associated with particular sections; the interface, which was
   designed for SunOS style shared libraries, expects that there is only
   one set of dynamic relocs.  Any loadable section that was actually
   installed in the BFD, and has type SHT_REL or SHT_RELA, and uses the
   dynamic symbol table, is considered to be a dynamic reloc section.  */

long
_bfd_elf_canonicalize_dynamic_reloc (bfd *abfd,
				     arelent **storage,
				     asymbol **syms)
{
  bfd_boolean (*slurp_relocs) (bfd *, asection *, asymbol **, bfd_boolean);
  asection *s;
  long ret;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  ret = 0;
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	  && (elf_section_data (s)->this_hdr.sh_type == SHT_REL
	      || elf_section_data (s)->this_hdr.sh_type == SHT_RELA))
	{
	  arelent *p;
	  long count, i;

	  if (! (*slurp_relocs) (abfd, s, syms, TRUE))
	    return -1;
	  count = s->size / elf_section_data (s)->this_hdr.sh_entsize;
	  p = s->relocation;
	  for (i = 0; i < count; i++)
	    *storage++ = p++;
	  ret += count;
	}
    }

  *storage = NULL;

  return ret;
}

/* Read in the version information.  */

bfd_boolean
_bfd_elf_slurp_version_tables (bfd *abfd, bfd_boolean default_imported_symver)
{
  bfd_byte *contents = NULL;
  unsigned int freeidx = 0;

  if (elf_dynverref (abfd) != 0)
    {
      Elf_Internal_Shdr *hdr;
      Elf_External_Verneed *everneed;
      Elf_Internal_Verneed *iverneed;
      unsigned int i;
      bfd_byte *contents_end;

      hdr = &elf_tdata (abfd)->dynverref_hdr;

      if (hdr->sh_info)
	elf_tdata (abfd)->verref = (Elf_Internal_Verneed *)
          bfd_zalloc2 (abfd, hdr->sh_info, sizeof (Elf_Internal_Verneed));
      else
	elf_tdata (abfd)->verref = NULL;
	
      if (elf_tdata (abfd)->verref == NULL)
	goto error_return;

      elf_tdata (abfd)->cverrefs = hdr->sh_info;

      contents = (bfd_byte *) bfd_malloc (hdr->sh_size);
      if (contents == NULL)
	{
error_return_verref:
	  elf_tdata (abfd)->verref = NULL;
	  elf_tdata (abfd)->cverrefs = 0;
	  goto error_return;
	}
      if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) != 0
	  || bfd_bread (contents, hdr->sh_size, abfd) != hdr->sh_size)
	goto error_return_verref;

      if (hdr->sh_info && hdr->sh_size < sizeof (Elf_External_Verneed))
	goto error_return_verref;

      BFD_ASSERT (sizeof (Elf_External_Verneed)
		  == sizeof (Elf_External_Vernaux));
      contents_end = contents + hdr->sh_size - sizeof (Elf_External_Verneed);
      everneed = (Elf_External_Verneed *) contents;
      iverneed = elf_tdata (abfd)->verref;
      for (i = 0; i < hdr->sh_info; i++, iverneed++)
	{
	  Elf_External_Vernaux *evernaux;
	  Elf_Internal_Vernaux *ivernaux;
	  unsigned int j;

	  _bfd_elf_swap_verneed_in (abfd, everneed, iverneed);

	  iverneed->vn_bfd = abfd;

	  iverneed->vn_filename =
	    bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
					     iverneed->vn_file);
	  if (iverneed->vn_filename == NULL)
	    goto error_return_verref;

	  if (iverneed->vn_cnt == 0)
	    iverneed->vn_auxptr = NULL;
	  else
	    {
	      iverneed->vn_auxptr = (struct elf_internal_vernaux *)
                  bfd_alloc2 (abfd, iverneed->vn_cnt,
                              sizeof (Elf_Internal_Vernaux));
	      if (iverneed->vn_auxptr == NULL)
		goto error_return_verref;
	    }

	  if (iverneed->vn_aux
	      > (size_t) (contents_end - (bfd_byte *) everneed))
	    goto error_return_verref;

	  evernaux = ((Elf_External_Vernaux *)
		      ((bfd_byte *) everneed + iverneed->vn_aux));
	  ivernaux = iverneed->vn_auxptr;
	  for (j = 0; j < iverneed->vn_cnt; j++, ivernaux++)
	    {
	      _bfd_elf_swap_vernaux_in (abfd, evernaux, ivernaux);

	      ivernaux->vna_nodename =
		bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
						 ivernaux->vna_name);
	      if (ivernaux->vna_nodename == NULL)
		goto error_return_verref;

	      if (j + 1 < iverneed->vn_cnt)
		ivernaux->vna_nextptr = ivernaux + 1;
	      else
		ivernaux->vna_nextptr = NULL;

	      if (ivernaux->vna_next
		  > (size_t) (contents_end - (bfd_byte *) evernaux))
		goto error_return_verref;

	      evernaux = ((Elf_External_Vernaux *)
			  ((bfd_byte *) evernaux + ivernaux->vna_next));

	      if (ivernaux->vna_other > freeidx)
		freeidx = ivernaux->vna_other;
	    }

	  if (i + 1 < hdr->sh_info)
	    iverneed->vn_nextref = iverneed + 1;
	  else
	    iverneed->vn_nextref = NULL;

	  if (iverneed->vn_next
	      > (size_t) (contents_end - (bfd_byte *) everneed))
	    goto error_return_verref;

	  everneed = ((Elf_External_Verneed *)
		      ((bfd_byte *) everneed + iverneed->vn_next));
	}

      free (contents);
      contents = NULL;
    }

  if (elf_dynverdef (abfd) != 0)
    {
      Elf_Internal_Shdr *hdr;
      Elf_External_Verdef *everdef;
      Elf_Internal_Verdef *iverdef;
      Elf_Internal_Verdef *iverdefarr;
      Elf_Internal_Verdef iverdefmem;
      unsigned int i;
      unsigned int maxidx;
      bfd_byte *contents_end_def, *contents_end_aux;

      hdr = &elf_tdata (abfd)->dynverdef_hdr;

      contents = (bfd_byte *) bfd_malloc (hdr->sh_size);
      if (contents == NULL)
	goto error_return;
      if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) != 0
	  || bfd_bread (contents, hdr->sh_size, abfd) != hdr->sh_size)
	goto error_return;

      if (hdr->sh_info && hdr->sh_size < sizeof (Elf_External_Verdef))
	goto error_return;

      BFD_ASSERT (sizeof (Elf_External_Verdef)
		  >= sizeof (Elf_External_Verdaux));
      contents_end_def = contents + hdr->sh_size
			 - sizeof (Elf_External_Verdef);
      contents_end_aux = contents + hdr->sh_size
			 - sizeof (Elf_External_Verdaux);

      /* We know the number of entries in the section but not the maximum
	 index.  Therefore we have to run through all entries and find
	 the maximum.  */
      everdef = (Elf_External_Verdef *) contents;
      maxidx = 0;
      for (i = 0; i < hdr->sh_info; ++i)
	{
	  _bfd_elf_swap_verdef_in (abfd, everdef, &iverdefmem);

	  if ((iverdefmem.vd_ndx & ((unsigned) VERSYM_VERSION)) > maxidx)
	    maxidx = iverdefmem.vd_ndx & ((unsigned) VERSYM_VERSION);

	  if (iverdefmem.vd_next
	      > (size_t) (contents_end_def - (bfd_byte *) everdef))
	    goto error_return;

	  everdef = ((Elf_External_Verdef *)
		     ((bfd_byte *) everdef + iverdefmem.vd_next));
	}

      if (default_imported_symver)
	{
	  if (freeidx > maxidx)
	    maxidx = ++freeidx;
	  else
	    freeidx = ++maxidx;
	}
      if (maxidx)
	elf_tdata (abfd)->verdef = (Elf_Internal_Verdef *)
          bfd_zalloc2 (abfd, maxidx, sizeof (Elf_Internal_Verdef));
      else
	elf_tdata (abfd)->verdef = NULL;

      if (elf_tdata (abfd)->verdef == NULL)
	goto error_return;

      elf_tdata (abfd)->cverdefs = maxidx;

      everdef = (Elf_External_Verdef *) contents;
      iverdefarr = elf_tdata (abfd)->verdef;
      for (i = 0; i < hdr->sh_info; i++)
	{
	  Elf_External_Verdaux *everdaux;
	  Elf_Internal_Verdaux *iverdaux;
	  unsigned int j;

	  _bfd_elf_swap_verdef_in (abfd, everdef, &iverdefmem);

	  if ((iverdefmem.vd_ndx & VERSYM_VERSION) == 0)
	    {
error_return_verdef:
	      elf_tdata (abfd)->verdef = NULL;
	      elf_tdata (abfd)->cverdefs = 0;
	      goto error_return;
	    }

	  iverdef = &iverdefarr[(iverdefmem.vd_ndx & VERSYM_VERSION) - 1];
	  memcpy (iverdef, &iverdefmem, sizeof (Elf_Internal_Verdef));

	  iverdef->vd_bfd = abfd;

	  if (iverdef->vd_cnt == 0)
	    iverdef->vd_auxptr = NULL;
	  else
	    {
	      iverdef->vd_auxptr = (struct elf_internal_verdaux *)
                  bfd_alloc2 (abfd, iverdef->vd_cnt,
                              sizeof (Elf_Internal_Verdaux));
	      if (iverdef->vd_auxptr == NULL)
		goto error_return_verdef;
	    }

	  if (iverdef->vd_aux
	      > (size_t) (contents_end_aux - (bfd_byte *) everdef))
	    goto error_return_verdef;

	  everdaux = ((Elf_External_Verdaux *)
		      ((bfd_byte *) everdef + iverdef->vd_aux));
	  iverdaux = iverdef->vd_auxptr;
	  for (j = 0; j < iverdef->vd_cnt; j++, iverdaux++)
	    {
	      _bfd_elf_swap_verdaux_in (abfd, everdaux, iverdaux);

	      iverdaux->vda_nodename =
		bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
						 iverdaux->vda_name);
	      if (iverdaux->vda_nodename == NULL)
		goto error_return_verdef;

	      if (j + 1 < iverdef->vd_cnt)
		iverdaux->vda_nextptr = iverdaux + 1;
	      else
		iverdaux->vda_nextptr = NULL;

	      if (iverdaux->vda_next
		  > (size_t) (contents_end_aux - (bfd_byte *) everdaux))
		goto error_return_verdef;

	      everdaux = ((Elf_External_Verdaux *)
			  ((bfd_byte *) everdaux + iverdaux->vda_next));
	    }

	  if (iverdef->vd_cnt)
	    iverdef->vd_nodename = iverdef->vd_auxptr->vda_nodename;

	  if ((size_t) (iverdef - iverdefarr) + 1 < maxidx)
	    iverdef->vd_nextdef = iverdef + 1;
	  else
	    iverdef->vd_nextdef = NULL;

	  everdef = ((Elf_External_Verdef *)
		     ((bfd_byte *) everdef + iverdef->vd_next));
	}

      free (contents);
      contents = NULL;
    }
  else if (default_imported_symver)
    {
      if (freeidx < 3)
	freeidx = 3;
      else
	freeidx++;

      elf_tdata (abfd)->verdef = (Elf_Internal_Verdef *)
          bfd_zalloc2 (abfd, freeidx, sizeof (Elf_Internal_Verdef));
      if (elf_tdata (abfd)->verdef == NULL)
	goto error_return;

      elf_tdata (abfd)->cverdefs = freeidx;
    }

  /* Create a default version based on the soname.  */
  if (default_imported_symver)
    {
      Elf_Internal_Verdef *iverdef;
      Elf_Internal_Verdaux *iverdaux;

      iverdef = &elf_tdata (abfd)->verdef[freeidx - 1];

      iverdef->vd_version = VER_DEF_CURRENT;
      iverdef->vd_flags = 0;
      iverdef->vd_ndx = freeidx;
      iverdef->vd_cnt = 1;

      iverdef->vd_bfd = abfd;

      iverdef->vd_nodename = bfd_elf_get_dt_soname (abfd);
      if (iverdef->vd_nodename == NULL)
	goto error_return_verdef;
      iverdef->vd_nextdef = NULL;
      iverdef->vd_auxptr = (struct elf_internal_verdaux *)
          bfd_alloc (abfd, sizeof (Elf_Internal_Verdaux));
      if (iverdef->vd_auxptr == NULL)
	goto error_return_verdef;

      iverdaux = iverdef->vd_auxptr;
      iverdaux->vda_nodename = iverdef->vd_nodename;
      iverdaux->vda_nextptr = NULL;
    }

  return TRUE;

 error_return:
  if (contents != NULL)
    free (contents);
  return FALSE;
}

asymbol *
_bfd_elf_make_empty_symbol (bfd *abfd)
{
  elf_symbol_type *newsym;

  newsym = (elf_symbol_type *) bfd_zalloc (abfd, sizeof * newsym);
  if (!newsym)
    return NULL;
  newsym->symbol.the_bfd = abfd;
  return &newsym->symbol;
}

void
_bfd_elf_get_symbol_info (bfd *abfd ATTRIBUTE_UNUSED,
			  asymbol *symbol,
			  symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);
}

/* Return whether a symbol name implies a local symbol.  Most targets
   use this function for the is_local_label_name entry point, but some
   override it.  */

bfd_boolean
_bfd_elf_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
			      const char *name)
{
  /* Normal local symbols start with ``.L''.  */
  if (name[0] == '.' && name[1] == 'L')
    return TRUE;

  /* At least some SVR4 compilers (e.g., UnixWare 2.1 cc) generate
     DWARF debugging symbols starting with ``..''.  */
  if (name[0] == '.' && name[1] == '.')
    return TRUE;

  /* gcc will sometimes generate symbols beginning with ``_.L_'' when
     emitting DWARF debugging output.  I suspect this is actually a
     small bug in gcc (it calls ASM_OUTPUT_LABEL when it should call
     ASM_GENERATE_INTERNAL_LABEL, and this causes the leading
     underscore to be emitted on some ELF targets).  For ease of use,
     we treat such symbols as local.  */
  if (name[0] == '_' && name[1] == '.' && name[2] == 'L' && name[3] == '_')
    return TRUE;

  return FALSE;
}

alent *
_bfd_elf_get_lineno (bfd *abfd ATTRIBUTE_UNUSED,
		     asymbol *symbol ATTRIBUTE_UNUSED)
{
  abort ();
  return NULL;
}

bfd_boolean
_bfd_elf_set_arch_mach (bfd *abfd,
			enum bfd_architecture arch,
			unsigned long machine)
{
  /* If this isn't the right architecture for this backend, and this
     isn't the generic backend, fail.  */
  if (arch != get_elf_backend_data (abfd)->arch
      && arch != bfd_arch_unknown
      && get_elf_backend_data (abfd)->arch != bfd_arch_unknown)
    return FALSE;

  return bfd_default_set_arch_mach (abfd, arch, machine);
}

/* Find the nearest line to a particular section and offset,
   for error reporting.  */

bfd_boolean
_bfd_elf_find_nearest_line (bfd *abfd,
			    asymbol **symbols,
			    asection *section,
			    bfd_vma offset,
			    const char **filename_ptr,
			    const char **functionname_ptr,
			    unsigned int *line_ptr,
			    unsigned int *discriminator_ptr)
{
  bfd_boolean found;

  if (_bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr, discriminator_ptr,
				     dwarf_debug_sections, 0,
				     &elf_tdata (abfd)->dwarf2_find_line_info)
      || _bfd_dwarf1_find_nearest_line (abfd, symbols, section, offset,
					filename_ptr, functionname_ptr,
					line_ptr))
    {
      if (!*functionname_ptr)
	_bfd_elf_find_function (abfd, symbols, section, offset,
				*filename_ptr ? NULL : filename_ptr,
				functionname_ptr);
      return TRUE;
    }

  if (! _bfd_stab_section_find_nearest_line (abfd, symbols, section, offset,
					     &found, filename_ptr,
					     functionname_ptr, line_ptr,
					     &elf_tdata (abfd)->line_info))
    return FALSE;
  if (found && (*functionname_ptr || *line_ptr))
    return TRUE;

  if (symbols == NULL)
    return FALSE;

  if (! _bfd_elf_find_function (abfd, symbols, section, offset,
				filename_ptr, functionname_ptr))
    return FALSE;

  *line_ptr = 0;
  return TRUE;
}

/* Find the line for a symbol.  */

bfd_boolean
_bfd_elf_find_line (bfd *abfd, asymbol **symbols, asymbol *symbol,
		    const char **filename_ptr, unsigned int *line_ptr)
{
  return _bfd_dwarf2_find_nearest_line (abfd, symbols, symbol, NULL, 0,
					filename_ptr, NULL, line_ptr, NULL,
					dwarf_debug_sections, 0,
					&elf_tdata (abfd)->dwarf2_find_line_info);
}

/* After a call to bfd_find_nearest_line, successive calls to
   bfd_find_inliner_info can be used to get source information about
   each level of function inlining that terminated at the address
   passed to bfd_find_nearest_line.  Currently this is only supported
   for DWARF2 with appropriate DWARF3 extensions. */

bfd_boolean
_bfd_elf_find_inliner_info (bfd *abfd,
			    const char **filename_ptr,
			    const char **functionname_ptr,
			    unsigned int *line_ptr)
{
  bfd_boolean found;
  found = _bfd_dwarf2_find_inliner_info (abfd, filename_ptr,
					 functionname_ptr, line_ptr,
					 & elf_tdata (abfd)->dwarf2_find_line_info);
  return found;
}

int
_bfd_elf_sizeof_headers (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int ret = bed->s->sizeof_ehdr;

  if (!info->relocatable)
    {
      bfd_size_type phdr_size = elf_program_header_size (abfd);

      if (phdr_size == (bfd_size_type) -1)
	{
	  struct elf_segment_map *m;

	  phdr_size = 0;
	  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
	    phdr_size += bed->s->sizeof_phdr;

	  if (phdr_size == 0)
	    phdr_size = get_program_header_size (abfd, info);
	}

      elf_program_header_size (abfd) = phdr_size;
      ret += phdr_size;
    }

  return ret;
}

bfd_boolean
_bfd_elf_set_section_contents (bfd *abfd,
			       sec_ptr section,
			       const void *location,
			       file_ptr offset,
			       bfd_size_type count)
{
  Elf_Internal_Shdr *hdr;
  file_ptr pos;

  if (! abfd->output_has_begun
      && ! _bfd_elf_compute_section_file_positions (abfd, NULL))
    return FALSE;

  hdr = &elf_section_data (section)->this_hdr;
  pos = hdr->sh_offset + offset;
  if (bfd_seek (abfd, pos, SEEK_SET) != 0
      || bfd_bwrite (location, count, abfd) != count)
    return FALSE;

  return TRUE;
}

void
_bfd_elf_no_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			   arelent *cache_ptr ATTRIBUTE_UNUSED,
			   Elf_Internal_Rela *dst ATTRIBUTE_UNUSED)
{
  abort ();
}

/* Try to convert a non-ELF reloc into an ELF one.  */

bfd_boolean
_bfd_elf_validate_reloc (bfd *abfd, arelent *areloc)
{
  /* Check whether we really have an ELF howto.  */

  if ((*areloc->sym_ptr_ptr)->the_bfd->xvec != abfd->xvec)
    {
      bfd_reloc_code_real_type code;
      reloc_howto_type *howto;

      /* Alien reloc: Try to determine its type to replace it with an
	 equivalent ELF reloc.  */

      if (areloc->howto->pc_relative)
	{
	  switch (areloc->howto->bitsize)
	    {
	    case 8:
	      code = BFD_RELOC_8_PCREL;
	      break;
	    case 12:
	      code = BFD_RELOC_12_PCREL;
	      break;
	    case 16:
	      code = BFD_RELOC_16_PCREL;
	      break;
	    case 24:
	      code = BFD_RELOC_24_PCREL;
	      break;
	    case 32:
	      code = BFD_RELOC_32_PCREL;
	      break;
	    case 64:
	      code = BFD_RELOC_64_PCREL;
	      break;
	    default:
	      goto fail;
	    }

	  howto = bfd_reloc_type_lookup (abfd, code);

	  if (areloc->howto->pcrel_offset != howto->pcrel_offset)
	    {
	      if (howto->pcrel_offset)
		areloc->addend += areloc->address;
	      else
		areloc->addend -= areloc->address; /* addend is unsigned!! */
	    }
	}
      else
	{
	  switch (areloc->howto->bitsize)
	    {
	    case 8:
	      code = BFD_RELOC_8;
	      break;
	    case 14:
	      code = BFD_RELOC_14;
	      break;
	    case 16:
	      code = BFD_RELOC_16;
	      break;
	    case 26:
	      code = BFD_RELOC_26;
	      break;
	    case 32:
	      code = BFD_RELOC_32;
	      break;
	    case 64:
	      code = BFD_RELOC_64;
	      break;
	    default:
	      goto fail;
	    }

	  howto = bfd_reloc_type_lookup (abfd, code);
	}

      if (howto)
	areloc->howto = howto;
      else
	goto fail;
    }

  return TRUE;

 fail:
  (*_bfd_error_handler)
    (_("%B: unsupported relocation type %s"),
     abfd, areloc->howto->name);
  bfd_set_error (bfd_error_bad_value);
  return FALSE;
}

bfd_boolean
_bfd_elf_close_and_cleanup (bfd *abfd)
{
  struct elf_obj_tdata *tdata = elf_tdata (abfd);
  if (bfd_get_format (abfd) == bfd_object && tdata != NULL)
    {
      if (elf_tdata (abfd)->o != NULL && elf_shstrtab (abfd) != NULL)
	_bfd_elf_strtab_free (elf_shstrtab (abfd));
      _bfd_dwarf2_cleanup_debug_info (abfd, &tdata->dwarf2_find_line_info);
    }

  return _bfd_generic_close_and_cleanup (abfd);
}

/* For Rel targets, we encode meaningful data for BFD_RELOC_VTABLE_ENTRY
   in the relocation's offset.  Thus we cannot allow any sort of sanity
   range-checking to interfere.  There is nothing else to do in processing
   this reloc.  */

bfd_reloc_status_type
_bfd_elf_rel_vtable_reloc_fn
  (bfd *abfd ATTRIBUTE_UNUSED, arelent *re ATTRIBUTE_UNUSED,
   struct bfd_symbol *symbol ATTRIBUTE_UNUSED,
   void *data ATTRIBUTE_UNUSED, asection *is ATTRIBUTE_UNUSED,
   bfd *obfd ATTRIBUTE_UNUSED, char **errmsg ATTRIBUTE_UNUSED)
{
  return bfd_reloc_ok;
}

/* Elf core file support.  Much of this only works on native
   toolchains, since we rely on knowing the
   machine-dependent procfs structure in order to pick
   out details about the corefile.  */

#ifdef HAVE_SYS_PROCFS_H
/* Needed for new procfs interface on sparc-solaris.  */
# define _STRUCTURED_PROC 1
# include <sys/procfs.h>
#endif

/* Return a PID that identifies a "thread" for threaded cores, or the
   PID of the main process for non-threaded cores.  */

static int
elfcore_make_pid (bfd *abfd)
{
  int pid;

  pid = elf_tdata (abfd)->core->lwpid;
  if (pid == 0)
    pid = elf_tdata (abfd)->core->pid;

  return pid;
}

/* If there isn't a section called NAME, make one, using
   data from SECT.  Note, this function will generate a
   reference to NAME, so you shouldn't deallocate or
   overwrite it.  */

static bfd_boolean
elfcore_maybe_make_sect (bfd *abfd, char *name, asection *sect)
{
  asection *sect2;

  if (bfd_get_section_by_name (abfd, name) != NULL)
    return TRUE;

  sect2 = bfd_make_section_with_flags (abfd, name, sect->flags);
  if (sect2 == NULL)
    return FALSE;

  sect2->size = sect->size;
  sect2->filepos = sect->filepos;
  sect2->alignment_power = sect->alignment_power;
  return TRUE;
}

/* Create a pseudosection containing SIZE bytes at FILEPOS.  This
   actually creates up to two pseudosections:
   - For the single-threaded case, a section named NAME, unless
     such a section already exists.
   - For the multi-threaded case, a section named "NAME/PID", where
     PID is elfcore_make_pid (abfd).
   Both pseudosections have identical contents. */
bfd_boolean
_bfd_elfcore_make_pseudosection (bfd *abfd,
				 char *name,
				 size_t size,
				 ufile_ptr filepos)
{
  char buf[100];
  char *threaded_name;
  size_t len;
  asection *sect;

  /* Build the section name.  */

  sprintf (buf, "%s/%d", name, elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  threaded_name = (char *) bfd_alloc (abfd, len);
  if (threaded_name == NULL)
    return FALSE;
  memcpy (threaded_name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, threaded_name,
					     SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;
  sect->size = size;
  sect->filepos = filepos;
  sect->alignment_power = 2;

  return elfcore_maybe_make_sect (abfd, name, sect);
}

/* prstatus_t exists on:
     solaris 2.5+
     linux 2.[01] + glibc
     unixware 4.2
*/

#if defined (HAVE_PRSTATUS_T)

static bfd_boolean
elfcore_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  size_t size;
  int offset;

  if (note->descsz == sizeof (prstatus_t))
    {
      prstatus_t prstat;

      size = sizeof (prstat.pr_reg);
      offset   = offsetof (prstatus_t, pr_reg);
      memcpy (&prstat, note->descdata, sizeof (prstat));

      /* Do not overwrite the core signal if it
	 has already been set by another thread.  */
      if (elf_tdata (abfd)->core->signal == 0)
	elf_tdata (abfd)->core->signal = prstat.pr_cursig;
      if (elf_tdata (abfd)->core->pid == 0)
	elf_tdata (abfd)->core->pid = prstat.pr_pid;

      /* pr_who exists on:
	 solaris 2.5+
	 unixware 4.2
	 pr_who doesn't exist on:
	 linux 2.[01]
	 */
#if defined (HAVE_PRSTATUS_T_PR_WHO)
      elf_tdata (abfd)->core->lwpid = prstat.pr_who;
#else
      elf_tdata (abfd)->core->lwpid = prstat.pr_pid;
#endif
    }
#if defined (HAVE_PRSTATUS32_T)
  else if (note->descsz == sizeof (prstatus32_t))
    {
      /* 64-bit host, 32-bit corefile */
      prstatus32_t prstat;

      size = sizeof (prstat.pr_reg);
      offset   = offsetof (prstatus32_t, pr_reg);
      memcpy (&prstat, note->descdata, sizeof (prstat));

      /* Do not overwrite the core signal if it
	 has already been set by another thread.  */
      if (elf_tdata (abfd)->core->signal == 0)
	elf_tdata (abfd)->core->signal = prstat.pr_cursig;
      if (elf_tdata (abfd)->core->pid == 0)
	elf_tdata (abfd)->core->pid = prstat.pr_pid;

      /* pr_who exists on:
	 solaris 2.5+
	 unixware 4.2
	 pr_who doesn't exist on:
	 linux 2.[01]
	 */
#if defined (HAVE_PRSTATUS32_T_PR_WHO)
      elf_tdata (abfd)->core->lwpid = prstat.pr_who;
#else
      elf_tdata (abfd)->core->lwpid = prstat.pr_pid;
#endif
    }
#endif /* HAVE_PRSTATUS32_T */
  else
    {
      /* Fail - we don't know how to handle any other
	 note size (ie. data object type).  */
      return TRUE;
    }

  /* Make a ".reg/999" section and a ".reg" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}
#endif /* defined (HAVE_PRSTATUS_T) */

/* Create a pseudosection containing the exact contents of NOTE.  */
static bfd_boolean
elfcore_make_note_pseudosection (bfd *abfd,
				 char *name,
				 Elf_Internal_Note *note)
{
  return _bfd_elfcore_make_pseudosection (abfd, name,
					  note->descsz, note->descpos);
}

/* There isn't a consistent prfpregset_t across platforms,
   but it doesn't matter, because we don't have to pick this
   data structure apart.  */

static bfd_boolean
elfcore_grok_prfpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg2", note);
}

/* Linux dumps the Intel SSE regs in a note named "LINUX" with a note
   type of NT_PRXFPREG.  Just include the whole note's contents
   literally.  */

static bfd_boolean
elfcore_grok_prxfpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-xfp", note);
}

/* Linux dumps the Intel XSAVE extended state in a note named "LINUX"
   with a note type of NT_X86_XSTATE.  Just include the whole note's
   contents literally.  */

static bfd_boolean
elfcore_grok_xstatereg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-xstate", note);
}

static bfd_boolean
elfcore_grok_ppc_vmx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-vmx", note);
}

static bfd_boolean
elfcore_grok_ppc_vsx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-vsx", note);
}

static bfd_boolean
elfcore_grok_s390_high_gprs (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-high-gprs", note);
}

static bfd_boolean
elfcore_grok_s390_timer (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-timer", note);
}

static bfd_boolean
elfcore_grok_s390_todcmp (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-todcmp", note);
}

static bfd_boolean
elfcore_grok_s390_todpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-todpreg", note);
}

static bfd_boolean
elfcore_grok_s390_ctrs (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-ctrs", note);
}

static bfd_boolean
elfcore_grok_s390_prefix (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-prefix", note);
}

static bfd_boolean
elfcore_grok_s390_last_break (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-last-break", note);
}

static bfd_boolean
elfcore_grok_s390_system_call (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-system-call", note);
}

static bfd_boolean
elfcore_grok_s390_tdb (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-tdb", note);
}

static bfd_boolean
elfcore_grok_arm_vfp (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-arm-vfp", note);
}

static bfd_boolean
elfcore_grok_aarch_tls (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-tls", note);
}

static bfd_boolean
elfcore_grok_aarch_hw_break (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-hw-break", note);
}

static bfd_boolean
elfcore_grok_aarch_hw_watch (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-hw-watch", note);
}

#if defined (HAVE_PRPSINFO_T)
typedef prpsinfo_t   elfcore_psinfo_t;
#if defined (HAVE_PRPSINFO32_T)		/* Sparc64 cross Sparc32 */
typedef prpsinfo32_t elfcore_psinfo32_t;
#endif
#endif

#if defined (HAVE_PSINFO_T)
typedef psinfo_t   elfcore_psinfo_t;
#if defined (HAVE_PSINFO32_T)		/* Sparc64 cross Sparc32 */
typedef psinfo32_t elfcore_psinfo32_t;
#endif
#endif

/* return a malloc'ed copy of a string at START which is at
   most MAX bytes long, possibly without a terminating '\0'.
   the copy will always have a terminating '\0'.  */

char *
_bfd_elfcore_strndup (bfd *abfd, char *start, size_t max)
{
  char *dups;
  char *end = (char *) memchr (start, '\0', max);
  size_t len;

  if (end == NULL)
    len = max;
  else
    len = end - start;

  dups = (char *) bfd_alloc (abfd, len + 1);
  if (dups == NULL)
    return NULL;

  memcpy (dups, start, len);
  dups[len] = '\0';

  return dups;
}

#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
static bfd_boolean
elfcore_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz == sizeof (elfcore_psinfo_t))
    {
      elfcore_psinfo_t psinfo;

      memcpy (&psinfo, note->descdata, sizeof (psinfo));

#if defined (HAVE_PSINFO_T_PR_PID) || defined (HAVE_PRPSINFO_T_PR_PID)
      elf_tdata (abfd)->core->pid = psinfo.pr_pid;
#endif
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, psinfo.pr_fname,
				sizeof (psinfo.pr_fname));

      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, psinfo.pr_psargs,
				sizeof (psinfo.pr_psargs));
    }
#if defined (HAVE_PRPSINFO32_T) || defined (HAVE_PSINFO32_T)
  else if (note->descsz == sizeof (elfcore_psinfo32_t))
    {
      /* 64-bit host, 32-bit corefile */
      elfcore_psinfo32_t psinfo;

      memcpy (&psinfo, note->descdata, sizeof (psinfo));

#if defined (HAVE_PSINFO32_T_PR_PID) || defined (HAVE_PRPSINFO32_T_PR_PID)
      elf_tdata (abfd)->core->pid = psinfo.pr_pid;
#endif
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, psinfo.pr_fname,
				sizeof (psinfo.pr_fname));

      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, psinfo.pr_psargs,
				sizeof (psinfo.pr_psargs));
    }
#endif

  else
    {
      /* Fail - we don't know how to handle any other
	 note size (ie. data object type).  */
      return TRUE;
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
#endif /* defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T) */

#if defined (HAVE_PSTATUS_T)
static bfd_boolean
elfcore_grok_pstatus (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz == sizeof (pstatus_t)
#if defined (HAVE_PXSTATUS_T)
      || note->descsz == sizeof (pxstatus_t)
#endif
      )
    {
      pstatus_t pstat;

      memcpy (&pstat, note->descdata, sizeof (pstat));

      elf_tdata (abfd)->core->pid = pstat.pr_pid;
    }
#if defined (HAVE_PSTATUS32_T)
  else if (note->descsz == sizeof (pstatus32_t))
    {
      /* 64-bit host, 32-bit corefile */
      pstatus32_t pstat;

      memcpy (&pstat, note->descdata, sizeof (pstat));

      elf_tdata (abfd)->core->pid = pstat.pr_pid;
    }
#endif
  /* Could grab some more details from the "representative"
     lwpstatus_t in pstat.pr_lwp, but we'll catch it all in an
     NT_LWPSTATUS note, presumably.  */

  return TRUE;
}
#endif /* defined (HAVE_PSTATUS_T) */

#if defined (HAVE_LWPSTATUS_T)
static bfd_boolean
elfcore_grok_lwpstatus (bfd *abfd, Elf_Internal_Note *note)
{
  lwpstatus_t lwpstat;
  char buf[100];
  char *name;
  size_t len;
  asection *sect;

  if (note->descsz != sizeof (lwpstat)
#if defined (HAVE_LWPXSTATUS_T)
      && note->descsz != sizeof (lwpxstatus_t)
#endif
      )
    return TRUE;

  memcpy (&lwpstat, note->descdata, sizeof (lwpstat));

  elf_tdata (abfd)->core->lwpid = lwpstat.pr_lwpid;
  /* Do not overwrite the core signal if it has already been set by
     another thread.  */
  if (elf_tdata (abfd)->core->signal == 0)
    elf_tdata (abfd)->core->signal = lwpstat.pr_cursig;

  /* Make a ".reg/999" section.  */

  sprintf (buf, ".reg/%d", elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  name = bfd_alloc (abfd, len);
  if (name == NULL)
    return FALSE;
  memcpy (name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

#if defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
  sect->size = sizeof (lwpstat.pr_context.uc_mcontext.gregs);
  sect->filepos = note->descpos
    + offsetof (lwpstatus_t, pr_context.uc_mcontext.gregs);
#endif

#if defined (HAVE_LWPSTATUS_T_PR_REG)
  sect->size = sizeof (lwpstat.pr_reg);
  sect->filepos = note->descpos + offsetof (lwpstatus_t, pr_reg);
#endif

  sect->alignment_power = 2;

  if (!elfcore_maybe_make_sect (abfd, ".reg", sect))
    return FALSE;

  /* Make a ".reg2/999" section */

  sprintf (buf, ".reg2/%d", elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  name = bfd_alloc (abfd, len);
  if (name == NULL)
    return FALSE;
  memcpy (name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

#if defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
  sect->size = sizeof (lwpstat.pr_context.uc_mcontext.fpregs);
  sect->filepos = note->descpos
    + offsetof (lwpstatus_t, pr_context.uc_mcontext.fpregs);
#endif

#if defined (HAVE_LWPSTATUS_T_PR_FPREG)
  sect->size = sizeof (lwpstat.pr_fpreg);
  sect->filepos = note->descpos + offsetof (lwpstatus_t, pr_fpreg);
#endif

  sect->alignment_power = 2;

  return elfcore_maybe_make_sect (abfd, ".reg2", sect);
}
#endif /* defined (HAVE_LWPSTATUS_T) */

static bfd_boolean
elfcore_grok_win32pstatus (bfd *abfd, Elf_Internal_Note *note)
{
  char buf[30];
  char *name;
  size_t len;
  asection *sect;
  int type;
  int is_active_thread;
  bfd_vma base_addr;

  if (note->descsz < 728)
    return TRUE;

  if (! CONST_STRNEQ (note->namedata, "win32"))
    return TRUE;

  type = bfd_get_32 (abfd, note->descdata);

  switch (type)
    {
    case 1 /* NOTE_INFO_PROCESS */:
      /* FIXME: need to add ->core->command.  */
      /* process_info.pid */
      elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, note->descdata + 8);
      /* process_info.signal */
      elf_tdata (abfd)->core->signal = bfd_get_32 (abfd, note->descdata + 12);
      break;

    case 2 /* NOTE_INFO_THREAD */:
      /* Make a ".reg/999" section.  */
      /* thread_info.tid */
      sprintf (buf, ".reg/%ld", (long) bfd_get_32 (abfd, note->descdata + 8));

      len = strlen (buf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (name == NULL)
	return FALSE;

      memcpy (name, buf, len);

      sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
      if (sect == NULL)
	return FALSE;

      /* sizeof (thread_info.thread_context) */
      sect->size = 716;
      /* offsetof (thread_info.thread_context) */
      sect->filepos = note->descpos + 12;
      sect->alignment_power = 2;

      /* thread_info.is_active_thread */
      is_active_thread = bfd_get_32 (abfd, note->descdata + 8);

      if (is_active_thread)
	if (! elfcore_maybe_make_sect (abfd, ".reg", sect))
	  return FALSE;
      break;

    case 3 /* NOTE_INFO_MODULE */:
      /* Make a ".module/xxxxxxxx" section.  */
      /* module_info.base_address */
      base_addr = bfd_get_32 (abfd, note->descdata + 4);
      sprintf (buf, ".module/%08lx", (unsigned long) base_addr);

      len = strlen (buf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (name == NULL)
	return FALSE;

      memcpy (name, buf, len);

      sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);

      if (sect == NULL)
	return FALSE;

      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 2;
      break;

    default:
      return TRUE;
    }

  return TRUE;
}

static bfd_boolean
elfcore_grok_note (bfd *abfd, Elf_Internal_Note *note)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  switch (note->type)
    {
    default:
      return TRUE;

    case NT_PRSTATUS:
      if (bed->elf_backend_grok_prstatus)
	if ((*bed->elf_backend_grok_prstatus) (abfd, note))
	  return TRUE;
#if defined (HAVE_PRSTATUS_T)
      return elfcore_grok_prstatus (abfd, note);
#else
      return TRUE;
#endif

#if defined (HAVE_PSTATUS_T)
    case NT_PSTATUS:
      return elfcore_grok_pstatus (abfd, note);
#endif

#if defined (HAVE_LWPSTATUS_T)
    case NT_LWPSTATUS:
      return elfcore_grok_lwpstatus (abfd, note);
#endif

    case NT_FPREGSET:		/* FIXME: rename to NT_PRFPREG */
      return elfcore_grok_prfpreg (abfd, note);

    case NT_WIN32PSTATUS:
      return elfcore_grok_win32pstatus (abfd, note);

    case NT_PRXFPREG:		/* Linux SSE extension */
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_prxfpreg (abfd, note);
      else
	return TRUE;

    case NT_X86_XSTATE:		/* Linux XSAVE extension */
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_xstatereg (abfd, note);
      else
	return TRUE;

    case NT_PPC_VMX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_vmx (abfd, note);
      else
	return TRUE;

    case NT_PPC_VSX:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_vsx (abfd, note);
      else
        return TRUE;

    case NT_S390_HIGH_GPRS:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_high_gprs (abfd, note);
      else
        return TRUE;

    case NT_S390_TIMER:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_timer (abfd, note);
      else
        return TRUE;

    case NT_S390_TODCMP:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_todcmp (abfd, note);
      else
        return TRUE;

    case NT_S390_TODPREG:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_todpreg (abfd, note);
      else
        return TRUE;

    case NT_S390_CTRS:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_ctrs (abfd, note);
      else
        return TRUE;

    case NT_S390_PREFIX:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_prefix (abfd, note);
      else
        return TRUE;

    case NT_S390_LAST_BREAK:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_last_break (abfd, note);
      else
        return TRUE;

    case NT_S390_SYSTEM_CALL:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_system_call (abfd, note);
      else
        return TRUE;

    case NT_S390_TDB:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_s390_tdb (abfd, note);
      else
        return TRUE;

    case NT_ARM_VFP:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_arm_vfp (abfd, note);
      else
	return TRUE;

    case NT_ARM_TLS:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_tls (abfd, note);
      else
	return TRUE;

    case NT_ARM_HW_BREAK:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_hw_break (abfd, note);
      else
	return TRUE;

    case NT_ARM_HW_WATCH:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_hw_watch (abfd, note);
      else
	return TRUE;

    case NT_PRPSINFO:
    case NT_PSINFO:
      if (bed->elf_backend_grok_psinfo)
	if ((*bed->elf_backend_grok_psinfo) (abfd, note))
	  return TRUE;
#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
      return elfcore_grok_psinfo (abfd, note);
#else
      return TRUE;
#endif

    case NT_AUXV:
      {
	asection *sect = bfd_make_section_anyway_with_flags (abfd, ".auxv",
							     SEC_HAS_CONTENTS);

	if (sect == NULL)
	  return FALSE;
	sect->size = note->descsz;
	sect->filepos = note->descpos;
	sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

	return TRUE;
      }

    case NT_FILE:
      return elfcore_make_note_pseudosection (abfd, ".note.linuxcore.file",
					      note);

    case NT_SIGINFO:
      return elfcore_make_note_pseudosection (abfd, ".note.linuxcore.siginfo",
					      note);
    }
}

static bfd_boolean
elfobj_grok_gnu_build_id (bfd *abfd, Elf_Internal_Note *note)
{
  struct elf_obj_tdata *t;

  if (note->descsz == 0)
    return FALSE;

  t = elf_tdata (abfd);
  t->build_id = bfd_alloc (abfd, sizeof (*t->build_id) - 1 + note->descsz);
  if (t->build_id == NULL)
    return FALSE;

  t->build_id->size = note->descsz;
  memcpy (t->build_id->data, note->descdata, note->descsz);

  return TRUE;
}

static bfd_boolean
elfobj_grok_gnu_note (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->type)
    {
    default:
      return TRUE;

    case NT_GNU_BUILD_ID:
      return elfobj_grok_gnu_build_id (abfd, note);
    }
}

static bfd_boolean
elfobj_grok_stapsdt_note_1 (bfd *abfd, Elf_Internal_Note *note)
{
  struct sdt_note *cur =
    (struct sdt_note *) bfd_alloc (abfd, sizeof (struct sdt_note)
				   + note->descsz);

  cur->next = (struct sdt_note *) (elf_tdata (abfd))->sdt_note_head;
  cur->size = (bfd_size_type) note->descsz;
  memcpy (cur->data, note->descdata, note->descsz);

  elf_tdata (abfd)->sdt_note_head = cur;

  return TRUE;
}

static bfd_boolean
elfobj_grok_stapsdt_note (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->type)
    {
    case NT_STAPSDT:
      return elfobj_grok_stapsdt_note_1 (abfd, note);

    default:
      return TRUE;
    }
}

static bfd_boolean
elfcore_netbsd_get_lwpid (Elf_Internal_Note *note, int *lwpidp)
{
  char *cp;

  cp = strchr (note->namedata, '@');
  if (cp != NULL)
    {
      *lwpidp = atoi(cp + 1);
      return TRUE;
    }
  return FALSE;
}

static bfd_boolean
elfcore_grok_netbsd_procinfo (bfd *abfd, Elf_Internal_Note *note)
{
  /* Signal number at offset 0x08. */
  elf_tdata (abfd)->core->signal
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x08);

  /* Process ID at offset 0x50. */
  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x50);

  /* Command name at 0x7c (max 32 bytes, including nul). */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + 0x7c, 31);

  return elfcore_make_note_pseudosection (abfd, ".note.netbsdcore.procinfo",
					  note);
}

static bfd_boolean
elfcore_grok_netbsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  int lwp;

  if (elfcore_netbsd_get_lwpid (note, &lwp))
    elf_tdata (abfd)->core->lwpid = lwp;

  if (note->type == NT_NETBSDCORE_PROCINFO)
    {
      /* NetBSD-specific core "procinfo".  Note that we expect to
	 find this note before any of the others, which is fine,
	 since the kernel writes this note out first when it
	 creates a core file.  */

      return elfcore_grok_netbsd_procinfo (abfd, note);
    }

  /* As of Jan 2002 there are no other machine-independent notes
     defined for NetBSD core files.  If the note type is less
     than the start of the machine-dependent note types, we don't
     understand it.  */

  if (note->type < NT_NETBSDCORE_FIRSTMACH)
    return TRUE;


  switch (bfd_get_arch (abfd))
    {
      /* On the Alpha, SPARC (32-bit and 64-bit), PT_GETREGS == mach+0 and
	 PT_GETFPREGS == mach+2.  */

    case bfd_arch_alpha:
    case bfd_arch_sparc:
      switch (note->type)
	{
	case NT_NETBSDCORE_FIRSTMACH+0:
	  return elfcore_make_note_pseudosection (abfd, ".reg", note);

	case NT_NETBSDCORE_FIRSTMACH+2:
	  return elfcore_make_note_pseudosection (abfd, ".reg2", note);

	default:
	  return TRUE;
	}

      /* On all other arch's, PT_GETREGS == mach+1 and
	 PT_GETFPREGS == mach+3.  */

    default:
      switch (note->type)
	{
	case NT_NETBSDCORE_FIRSTMACH+1:
	  return elfcore_make_note_pseudosection (abfd, ".reg", note);

	case NT_NETBSDCORE_FIRSTMACH+3:
	  return elfcore_make_note_pseudosection (abfd, ".reg2", note);

	default:
	  return TRUE;
	}
    }
    /* NOTREACHED */
}

static bfd_boolean
elfcore_grok_openbsd_procinfo (bfd *abfd, Elf_Internal_Note *note)
{
  /* Signal number at offset 0x08. */
  elf_tdata (abfd)->core->signal
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x08);

  /* Process ID at offset 0x20. */
  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x20);

  /* Command name at 0x48 (max 32 bytes, including nul). */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + 0x48, 31);

  return TRUE;
}

static bfd_boolean
elfcore_grok_openbsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->type == NT_OPENBSD_PROCINFO)
    return elfcore_grok_openbsd_procinfo (abfd, note);

  if (note->type == NT_OPENBSD_REGS)
    return elfcore_make_note_pseudosection (abfd, ".reg", note);

  if (note->type == NT_OPENBSD_FPREGS)
    return elfcore_make_note_pseudosection (abfd, ".reg2", note);

  if (note->type == NT_OPENBSD_XFPREGS)
    return elfcore_make_note_pseudosection (abfd, ".reg-xfp", note);

  if (note->type == NT_OPENBSD_AUXV)
    {
      asection *sect = bfd_make_section_anyway_with_flags (abfd, ".auxv",
							   SEC_HAS_CONTENTS);

      if (sect == NULL)
	return FALSE;
      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

      return TRUE;
    }

  if (note->type == NT_OPENBSD_WCOOKIE)
    {
      asection *sect = bfd_make_section_anyway_with_flags (abfd, ".wcookie",
							   SEC_HAS_CONTENTS);

      if (sect == NULL)
	return FALSE;
      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

      return TRUE;
    }

  return TRUE;
}

static bfd_boolean
elfcore_grok_nto_status (bfd *abfd, Elf_Internal_Note *note, long *tid)
{
  void *ddata = note->descdata;
  char buf[100];
  char *name;
  asection *sect;
  short sig;
  unsigned flags;

  /* nto_procfs_status 'pid' field is at offset 0.  */
  elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, (bfd_byte *) ddata);

  /* nto_procfs_status 'tid' field is at offset 4.  Pass it back.  */
  *tid = bfd_get_32 (abfd, (bfd_byte *) ddata + 4);

  /* nto_procfs_status 'flags' field is at offset 8.  */
  flags = bfd_get_32 (abfd, (bfd_byte *) ddata + 8);

  /* nto_procfs_status 'what' field is at offset 14.  */
  if ((sig = bfd_get_16 (abfd, (bfd_byte *) ddata + 14)) > 0)
    {
      elf_tdata (abfd)->core->signal = sig;
      elf_tdata (abfd)->core->lwpid = *tid;
    }

  /* _DEBUG_FLAG_CURTID (current thread) is 0x80.  Some cores
     do not come from signals so we make sure we set the current
     thread just in case.  */
  if (flags & 0x00000080)
    elf_tdata (abfd)->core->lwpid = *tid;

  /* Make a ".qnx_core_status/%d" section.  */
  sprintf (buf, ".qnx_core_status/%ld", *tid);

  name = (char *) bfd_alloc (abfd, strlen (buf) + 1);
  if (name == NULL)
    return FALSE;
  strcpy (name, buf);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

  sect->size            = note->descsz;
  sect->filepos         = note->descpos;
  sect->alignment_power = 2;

  return (elfcore_maybe_make_sect (abfd, ".qnx_core_status", sect));
}

static bfd_boolean
elfcore_grok_nto_regs (bfd *abfd,
		       Elf_Internal_Note *note,
		       long tid,
		       char *base)
{
  char buf[100];
  char *name;
  asection *sect;

  /* Make a "(base)/%d" section.  */
  sprintf (buf, "%s/%ld", base, tid);

  name = (char *) bfd_alloc (abfd, strlen (buf) + 1);
  if (name == NULL)
    return FALSE;
  strcpy (name, buf);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

  sect->size            = note->descsz;
  sect->filepos         = note->descpos;
  sect->alignment_power = 2;

  /* This is the current thread.  */
  if (elf_tdata (abfd)->core->lwpid == tid)
    return elfcore_maybe_make_sect (abfd, base, sect);

  return TRUE;
}

#define BFD_QNT_CORE_INFO	7
#define BFD_QNT_CORE_STATUS	8
#define BFD_QNT_CORE_GREG	9
#define BFD_QNT_CORE_FPREG	10

static bfd_boolean
elfcore_grok_nto_note (bfd *abfd, Elf_Internal_Note *note)
{
  /* Every GREG section has a STATUS section before it.  Store the
     tid from the previous call to pass down to the next gregs
     function.  */
  static long tid = 1;

  switch (note->type)
    {
    case BFD_QNT_CORE_INFO:
      return elfcore_make_note_pseudosection (abfd, ".qnx_core_info", note);
    case BFD_QNT_CORE_STATUS:
      return elfcore_grok_nto_status (abfd, note, &tid);
    case BFD_QNT_CORE_GREG:
      return elfcore_grok_nto_regs (abfd, note, tid, ".reg");
    case BFD_QNT_CORE_FPREG:
      return elfcore_grok_nto_regs (abfd, note, tid, ".reg2");
    default:
      return TRUE;
    }
}

static bfd_boolean
elfcore_grok_spu_note (bfd *abfd, Elf_Internal_Note *note)
{
  char *name;
  asection *sect;
  size_t len;

  /* Use note name as section name.  */
  len = note->namesz;
  name = (char *) bfd_alloc (abfd, len);
  if (name == NULL)
    return FALSE;
  memcpy (name, note->namedata, len);
  name[len - 1] = '\0';

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

  sect->size            = note->descsz;
  sect->filepos         = note->descpos;
  sect->alignment_power = 1;

  return TRUE;
}

/* Function: elfcore_write_note

   Inputs:
     buffer to hold note, and current size of buffer
     name of note
     type of note
     data for note
     size of data for note

   Writes note to end of buffer.  ELF64 notes are written exactly as
   for ELF32, despite the current (as of 2006) ELF gabi specifying
   that they ought to have 8-byte namesz and descsz field, and have
   8-byte alignment.  Other writers, eg. Linux kernel, do the same.

   Return:
   Pointer to realloc'd buffer, *BUFSIZ updated.  */

char *
elfcore_write_note (bfd *abfd,
		    char *buf,
		    int *bufsiz,
		    const char *name,
		    int type,
		    const void *input,
		    int size)
{
  Elf_External_Note *xnp;
  size_t namesz;
  size_t newspace;
  char *dest;

  namesz = 0;
  if (name != NULL)
    namesz = strlen (name) + 1;

  newspace = 12 + ((namesz + 3) & -4) + ((size + 3) & -4);

  buf = (char *) realloc (buf, *bufsiz + newspace);
  if (buf == NULL)
    return buf;
  dest = buf + *bufsiz;
  *bufsiz += newspace;
  xnp = (Elf_External_Note *) dest;
  H_PUT_32 (abfd, namesz, xnp->namesz);
  H_PUT_32 (abfd, size, xnp->descsz);
  H_PUT_32 (abfd, type, xnp->type);
  dest = xnp->name;
  if (name != NULL)
    {
      memcpy (dest, name, namesz);
      dest += namesz;
      while (namesz & 3)
	{
	  *dest++ = '\0';
	  ++namesz;
	}
    }
  memcpy (dest, input, size);
  dest += size;
  while (size & 3)
    {
      *dest++ = '\0';
      ++size;
    }
  return buf;
}

char *
elfcore_write_prpsinfo (bfd  *abfd,
			char *buf,
			int  *bufsiz,
			const char *fname,
			const char *psargs)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->elf_backend_write_core_note != NULL)
    {
      char *ret;
      ret = (*bed->elf_backend_write_core_note) (abfd, buf, bufsiz,
						 NT_PRPSINFO, fname, psargs);
      if (ret != NULL)
	return ret;
    }

#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
#if defined (HAVE_PRPSINFO32_T) || defined (HAVE_PSINFO32_T)
  if (bed->s->elfclass == ELFCLASS32)
    {
#if defined (HAVE_PSINFO32_T)
      psinfo32_t data;
      int note_type = NT_PSINFO;
#else
      prpsinfo32_t data;
      int note_type = NT_PRPSINFO;
#endif

      memset (&data, 0, sizeof (data));
      strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
      strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", note_type, &data, sizeof (data));
    }
  else
#endif
    {
#if defined (HAVE_PSINFO_T)
      psinfo_t data;
      int note_type = NT_PSINFO;
#else
      prpsinfo_t data;
      int note_type = NT_PRPSINFO;
#endif

      memset (&data, 0, sizeof (data));
      strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
      strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", note_type, &data, sizeof (data));
    }
#endif	/* PSINFO_T or PRPSINFO_T */

  free (buf);
  return NULL;
}

char *
elfcore_write_linux_prpsinfo32
  (bfd *abfd, char *buf, int *bufsiz,
   const struct elf_internal_linux_prpsinfo *prpsinfo)
{
  struct elf_external_linux_prpsinfo32 data;

  memset (&data, 0, sizeof (data));
  LINUX_PRPSINFO32_SWAP_FIELDS (abfd, prpsinfo, data);

  return elfcore_write_note (abfd, buf, bufsiz, "CORE", NT_PRPSINFO,
			     &data, sizeof (data));
}

char *
elfcore_write_linux_prpsinfo64
  (bfd *abfd, char *buf, int *bufsiz,
   const struct elf_internal_linux_prpsinfo *prpsinfo)
{
  struct elf_external_linux_prpsinfo64 data;

  memset (&data, 0, sizeof (data));
  LINUX_PRPSINFO64_SWAP_FIELDS (abfd, prpsinfo, data);

  return elfcore_write_note (abfd, buf, bufsiz,
			     "CORE", NT_PRPSINFO, &data, sizeof (data));
}

char *
elfcore_write_prstatus (bfd *abfd,
			char *buf,
			int *bufsiz,
			long pid,
			int cursig,
			const void *gregs)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->elf_backend_write_core_note != NULL)
    {
      char *ret;
      ret = (*bed->elf_backend_write_core_note) (abfd, buf, bufsiz,
						 NT_PRSTATUS,
						 pid, cursig, gregs);
      if (ret != NULL)
	return ret;
    }

#if defined (HAVE_PRSTATUS_T)
#if defined (HAVE_PRSTATUS32_T)
  if (bed->s->elfclass == ELFCLASS32)
    {
      prstatus32_t prstat;

      memset (&prstat, 0, sizeof (prstat));
      prstat.pr_pid = pid;
      prstat.pr_cursig = cursig;
      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
      return elfcore_write_note (abfd, buf, bufsiz, "CORE",
				 NT_PRSTATUS, &prstat, sizeof (prstat));
    }
  else
#endif
    {
      prstatus_t prstat;

      memset (&prstat, 0, sizeof (prstat));
      prstat.pr_pid = pid;
      prstat.pr_cursig = cursig;
      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
      return elfcore_write_note (abfd, buf, bufsiz, "CORE",
				 NT_PRSTATUS, &prstat, sizeof (prstat));
    }
#endif /* HAVE_PRSTATUS_T */

  free (buf);
  return NULL;
}

#if defined (HAVE_LWPSTATUS_T)
char *
elfcore_write_lwpstatus (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 long pid,
			 int cursig,
			 const void *gregs)
{
  lwpstatus_t lwpstat;
  const char *note_name = "CORE";

  memset (&lwpstat, 0, sizeof (lwpstat));
  lwpstat.pr_lwpid  = pid >> 16;
  lwpstat.pr_cursig = cursig;
#if defined (HAVE_LWPSTATUS_T_PR_REG)
  memcpy (&lwpstat.pr_reg, gregs, sizeof (lwpstat.pr_reg));
#elif defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
#if !defined(gregs)
  memcpy (lwpstat.pr_context.uc_mcontext.gregs,
	  gregs, sizeof (lwpstat.pr_context.uc_mcontext.gregs));
#else
  memcpy (lwpstat.pr_context.uc_mcontext.__gregs,
	  gregs, sizeof (lwpstat.pr_context.uc_mcontext.__gregs));
#endif
#endif
  return elfcore_write_note (abfd, buf, bufsiz, note_name,
			     NT_LWPSTATUS, &lwpstat, sizeof (lwpstat));
}
#endif /* HAVE_LWPSTATUS_T */

#if defined (HAVE_PSTATUS_T)
char *
elfcore_write_pstatus (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       long pid,
		       int cursig ATTRIBUTE_UNUSED,
		       const void *gregs ATTRIBUTE_UNUSED)
{
  const char *note_name = "CORE";
#if defined (HAVE_PSTATUS32_T)
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->s->elfclass == ELFCLASS32)
    {
      pstatus32_t pstat;

      memset (&pstat, 0, sizeof (pstat));
      pstat.pr_pid = pid & 0xffff;
      buf = elfcore_write_note (abfd, buf, bufsiz, note_name,
				NT_PSTATUS, &pstat, sizeof (pstat));
      return buf;
    }
  else
#endif
    {
      pstatus_t pstat;

      memset (&pstat, 0, sizeof (pstat));
      pstat.pr_pid = pid & 0xffff;
      buf = elfcore_write_note (abfd, buf, bufsiz, note_name,
				NT_PSTATUS, &pstat, sizeof (pstat));
      return buf;
    }
}
#endif /* HAVE_PSTATUS_T */

char *
elfcore_write_prfpreg (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *fpregs,
		       int size)
{
  const char *note_name = "CORE";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_FPREGSET, fpregs, size);
}

char *
elfcore_write_prxfpreg (bfd *abfd,
			char *buf,
			int *bufsiz,
			const void *xfpregs,
			int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PRXFPREG, xfpregs, size);
}

char *
elfcore_write_xstatereg (bfd *abfd, char *buf, int *bufsiz,
			 const void *xfpregs, int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_X86_XSTATE, xfpregs, size);
}

char *
elfcore_write_ppc_vmx (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_vmx,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_VMX, ppc_vmx, size);
}

char *
elfcore_write_ppc_vsx (bfd *abfd,
                       char *buf,
                       int *bufsiz,
                       const void *ppc_vsx,
                       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_VSX, ppc_vsx, size);
}

static char *
elfcore_write_s390_high_gprs (bfd *abfd,
			      char *buf,
			      int *bufsiz,
			      const void *s390_high_gprs,
			      int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_HIGH_GPRS,
			     s390_high_gprs, size);
}

char *
elfcore_write_s390_timer (bfd *abfd,
                          char *buf,
                          int *bufsiz,
                          const void *s390_timer,
                          int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_TIMER, s390_timer, size);
}

char *
elfcore_write_s390_todcmp (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *s390_todcmp,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_TODCMP, s390_todcmp, size);
}

char *
elfcore_write_s390_todpreg (bfd *abfd,
                            char *buf,
                            int *bufsiz,
                            const void *s390_todpreg,
                            int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_TODPREG, s390_todpreg, size);
}

char *
elfcore_write_s390_ctrs (bfd *abfd,
                         char *buf,
                         int *bufsiz,
                         const void *s390_ctrs,
                         int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_CTRS, s390_ctrs, size);
}

char *
elfcore_write_s390_prefix (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *s390_prefix,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_PREFIX, s390_prefix, size);
}

char *
elfcore_write_s390_last_break (bfd *abfd,
			       char *buf,
			       int *bufsiz,
			       const void *s390_last_break,
			       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_LAST_BREAK,
			     s390_last_break, size);
}

char *
elfcore_write_s390_system_call (bfd *abfd,
				char *buf,
				int *bufsiz,
				const void *s390_system_call,
				int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_SYSTEM_CALL,
			     s390_system_call, size);
}

char *
elfcore_write_s390_tdb (bfd *abfd,
			char *buf,
			int *bufsiz,
			const void *s390_tdb,
			int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_S390_TDB, s390_tdb, size);
}

char *
elfcore_write_arm_vfp (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *arm_vfp,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_VFP, arm_vfp, size);
}

char *
elfcore_write_aarch_tls (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *aarch_tls,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_TLS, aarch_tls, size);
}

char *
elfcore_write_aarch_hw_break (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *aarch_hw_break,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_HW_BREAK, aarch_hw_break, size);
}

char *
elfcore_write_aarch_hw_watch (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *aarch_hw_watch,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_HW_WATCH, aarch_hw_watch, size);
}

char *
elfcore_write_register_note (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const char *section,
			     const void *data,
			     int size)
{
  if (strcmp (section, ".reg2") == 0)
    return elfcore_write_prfpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-xfp") == 0)
    return elfcore_write_prxfpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-xstate") == 0)
    return elfcore_write_xstatereg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-vmx") == 0)
    return elfcore_write_ppc_vmx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-vsx") == 0)
    return elfcore_write_ppc_vsx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-high-gprs") == 0)
    return elfcore_write_s390_high_gprs (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-timer") == 0)
    return elfcore_write_s390_timer (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-todcmp") == 0)
    return elfcore_write_s390_todcmp (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-todpreg") == 0)
    return elfcore_write_s390_todpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-ctrs") == 0)
    return elfcore_write_s390_ctrs (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-prefix") == 0)
    return elfcore_write_s390_prefix (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-last-break") == 0)
    return elfcore_write_s390_last_break (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-system-call") == 0)
    return elfcore_write_s390_system_call (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-tdb") == 0)
    return elfcore_write_s390_tdb (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-arm-vfp") == 0)
    return elfcore_write_arm_vfp (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-tls") == 0)
    return elfcore_write_aarch_tls (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-hw-break") == 0)
    return elfcore_write_aarch_hw_break (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-hw-watch") == 0)
    return elfcore_write_aarch_hw_watch (abfd, buf, bufsiz, data, size);
  return NULL;
}

static bfd_boolean
elf_parse_notes (bfd *abfd, char *buf, size_t size, file_ptr offset)
{
  char *p;

  p = buf;
  while (p < buf + size)
    {
      /* FIXME: bad alignment assumption.  */
      Elf_External_Note *xnp = (Elf_External_Note *) p;
      Elf_Internal_Note in;

      if (offsetof (Elf_External_Note, name) > buf - p + size)
	return FALSE;

      in.type = H_GET_32 (abfd, xnp->type);

      in.namesz = H_GET_32 (abfd, xnp->namesz);
      in.namedata = xnp->name;
      if (in.namesz > buf - in.namedata + size)
	return FALSE;

      in.descsz = H_GET_32 (abfd, xnp->descsz);
      in.descdata = in.namedata + BFD_ALIGN (in.namesz, 4);
      in.descpos = offset + (in.descdata - buf);
      if (in.descsz != 0
	  && (in.descdata >= buf + size
	      || in.descsz > buf - in.descdata + size))
	return FALSE;

      switch (bfd_get_format (abfd))
        {
	default:
	  return TRUE;

	case bfd_core:
	  if (CONST_STRNEQ (in.namedata, "NetBSD-CORE"))
	    {
	      if (! elfcore_grok_netbsd_note (abfd, &in))
		return FALSE;
	    }
	  else if (CONST_STRNEQ (in.namedata, "OpenBSD"))
	    {
	      if (! elfcore_grok_openbsd_note (abfd, &in))
		return FALSE;
	    }
	  else if (CONST_STRNEQ (in.namedata, "QNX"))
	    {
	      if (! elfcore_grok_nto_note (abfd, &in))
		return FALSE;
	    }
	  else if (CONST_STRNEQ (in.namedata, "SPU/"))
	    {
	      if (! elfcore_grok_spu_note (abfd, &in))
		return FALSE;
	    }
	  else
	    {
	      if (! elfcore_grok_note (abfd, &in))
		return FALSE;
	    }
	  break;

	case bfd_object:
	  if (in.namesz == sizeof "GNU" && strcmp (in.namedata, "GNU") == 0)
	    {
	      if (! elfobj_grok_gnu_note (abfd, &in))
		return FALSE;
	    }
	  else if (in.namesz == sizeof "stapsdt"
		   && strcmp (in.namedata, "stapsdt") == 0)
	    {
	      if (! elfobj_grok_stapsdt_note (abfd, &in))
		return FALSE;
	    }
	  break;
	}

      p = in.descdata + BFD_ALIGN (in.descsz, 4);
    }

  return TRUE;
}

static bfd_boolean
elf_read_notes (bfd *abfd, file_ptr offset, bfd_size_type size)
{
  char *buf;

  if (size <= 0)
    return TRUE;

  if (bfd_seek (abfd, offset, SEEK_SET) != 0)
    return FALSE;

  buf = (char *) bfd_malloc (size + 1);
  if (buf == NULL)
    return FALSE;

  /* PR 17512: file: ec08f814
     0-termintate the buffer so that string searches will not overflow.  */
  buf[size] = 0;

  if (bfd_bread (buf, size, abfd) != size
      || !elf_parse_notes (abfd, buf, size, offset))
    {
      free (buf);
      return FALSE;
    }

  free (buf);
  return TRUE;
}

/* Providing external access to the ELF program header table.  */

/* Return an upper bound on the number of bytes required to store a
   copy of ABFD's program header table entries.  Return -1 if an error
   occurs; bfd_get_error will return an appropriate code.  */

long
bfd_get_elf_phdr_upper_bound (bfd *abfd)
{
  if (abfd->xvec->flavour != bfd_target_elf_flavour)
    {
      bfd_set_error (bfd_error_wrong_format);
      return -1;
    }

  return elf_elfheader (abfd)->e_phnum * sizeof (Elf_Internal_Phdr);
}

/* Copy ABFD's program header table entries to *PHDRS.  The entries
   will be stored as an array of Elf_Internal_Phdr structures, as
   defined in include/elf/internal.h.  To find out how large the
   buffer needs to be, call bfd_get_elf_phdr_upper_bound.

   Return the number of program header table entries read, or -1 if an
   error occurs; bfd_get_error will return an appropriate code.  */

int
bfd_get_elf_phdrs (bfd *abfd, void *phdrs)
{
  int num_phdrs;

  if (abfd->xvec->flavour != bfd_target_elf_flavour)
    {
      bfd_set_error (bfd_error_wrong_format);
      return -1;
    }

  num_phdrs = elf_elfheader (abfd)->e_phnum;
  memcpy (phdrs, elf_tdata (abfd)->phdr,
	  num_phdrs * sizeof (Elf_Internal_Phdr));

  return num_phdrs;
}

enum elf_reloc_type_class
_bfd_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela ATTRIBUTE_UNUSED)
{
  return reloc_class_normal;
}

/* For RELA architectures, return the relocation value for a
   relocation against a local symbol.  */

bfd_vma
_bfd_elf_rela_local_sym (bfd *abfd,
			 Elf_Internal_Sym *sym,
			 asection **psec,
			 Elf_Internal_Rela *rel)
{
  asection *sec = *psec;
  bfd_vma relocation;

  relocation = (sec->output_section->vma
		+ sec->output_offset
		+ sym->st_value);
  if ((sec->flags & SEC_MERGE)
      && ELF_ST_TYPE (sym->st_info) == STT_SECTION
      && sec->sec_info_type == SEC_INFO_TYPE_MERGE)
    {
      rel->r_addend =
	_bfd_merged_section_offset (abfd, psec,
				    elf_section_data (sec)->sec_info,
				    sym->st_value + rel->r_addend);
      if (sec != *psec)
	{
	  /* If we have changed the section, and our original section is
	     marked with SEC_EXCLUDE, it means that the original
	     SEC_MERGE section has been completely subsumed in some
	     other SEC_MERGE section.  In this case, we need to leave
	     some info around for --emit-relocs.  */
	  if ((sec->flags & SEC_EXCLUDE) != 0)
	    sec->kept_section = *psec;
	  sec = *psec;
	}
      rel->r_addend -= relocation;
      rel->r_addend += sec->output_section->vma + sec->output_offset;
    }
  return relocation;
}

bfd_vma
_bfd_elf_rel_local_sym (bfd *abfd,
			Elf_Internal_Sym *sym,
			asection **psec,
			bfd_vma addend)
{
  asection *sec = *psec;

  if (sec->sec_info_type != SEC_INFO_TYPE_MERGE)
    return sym->st_value + addend;

  return _bfd_merged_section_offset (abfd, psec,
				     elf_section_data (sec)->sec_info,
				     sym->st_value + addend);
}

bfd_vma
_bfd_elf_section_offset (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 bfd_vma offset)
{
  switch (sec->sec_info_type)
    {
    case SEC_INFO_TYPE_STABS:
      return _bfd_stab_section_offset (sec, elf_section_data (sec)->sec_info,
				       offset);
    case SEC_INFO_TYPE_EH_FRAME:
      return _bfd_elf_eh_frame_section_offset (abfd, info, sec, offset);
    default:
      if ((sec->flags & SEC_ELF_REVERSE_COPY) != 0)
	{
	  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
	  bfd_size_type address_size = bed->s->arch_size / 8;
	  offset = sec->size - offset - address_size;
	}
      return offset;
    }
}

/* Create a new BFD as if by bfd_openr.  Rather than opening a file,
   reconstruct an ELF file by reading the segments out of remote memory
   based on the ELF file header at EHDR_VMA and the ELF program headers it
   points to.  If not null, *LOADBASEP is filled in with the difference
   between the VMAs from which the segments were read, and the VMAs the
   file headers (and hence BFD's idea of each section's VMA) put them at.

   The function TARGET_READ_MEMORY is called to copy LEN bytes from the
   remote memory at target address VMA into the local buffer at MYADDR; it
   should return zero on success or an `errno' code on failure.  TEMPL must
   be a BFD for an ELF target with the word size and byte order found in
   the remote memory.  */

bfd *
bfd_elf_bfd_from_remote_memory
  (bfd *templ,
   bfd_vma ehdr_vma,
   bfd_size_type size,
   bfd_vma *loadbasep,
   int (*target_read_memory) (bfd_vma, bfd_byte *, bfd_size_type))
{
  return (*get_elf_backend_data (templ)->elf_backend_bfd_from_remote_memory)
    (templ, ehdr_vma, size, loadbasep, target_read_memory);
}

long
_bfd_elf_get_synthetic_symtab (bfd *abfd,
			       long symcount ATTRIBUTE_UNUSED,
			       asymbol **syms ATTRIBUTE_UNUSED,
			       long dynsymcount,
			       asymbol **dynsyms,
			       asymbol **ret)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  asection *relplt;
  asymbol *s;
  const char *relplt_name;
  bfd_boolean (*slurp_relocs) (bfd *, asection *, asymbol **, bfd_boolean);
  arelent *p;
  long count, i, n;
  size_t size;
  Elf_Internal_Shdr *hdr;
  char *names;
  asection *plt;

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0)
    return 0;

  if (dynsymcount <= 0)
    return 0;

  if (!bed->plt_sym_val)
    return 0;

  relplt_name = bed->relplt_name;
  if (relplt_name == NULL)
    relplt_name = bed->rela_plts_and_copies_p ? ".rela.plt" : ".rel.plt";
  relplt = bfd_get_section_by_name (abfd, relplt_name);
  if (relplt == NULL)
    return 0;

  hdr = &elf_section_data (relplt)->this_hdr;
  if (hdr->sh_link != elf_dynsymtab (abfd)
      || (hdr->sh_type != SHT_REL && hdr->sh_type != SHT_RELA))
    return 0;

  plt = bfd_get_section_by_name (abfd, ".plt");
  if (plt == NULL)
    return 0;

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  if (! (*slurp_relocs) (abfd, relplt, dynsyms, TRUE))
    return -1;

  count = relplt->size / hdr->sh_entsize;
  size = count * sizeof (asymbol);
  p = relplt->relocation;
  for (i = 0; i < count; i++, p += bed->s->int_rels_per_ext_rel)
    {
      size += strlen ((*p->sym_ptr_ptr)->name) + sizeof ("@plt");
      if (p->addend != 0)
	{
#ifdef BFD64
	  size += sizeof ("+0x") - 1 + 8 + 8 * (bed->s->elfclass == ELFCLASS64);
#else
	  size += sizeof ("+0x") - 1 + 8;
#endif
	}
    }

  s = *ret = (asymbol *) bfd_malloc (size);
  if (s == NULL)
    return -1;

  names = (char *) (s + count);
  p = relplt->relocation;
  n = 0;
  for (i = 0; i < count; i++, p += bed->s->int_rels_per_ext_rel)
    {
      size_t len;
      bfd_vma addr;

      addr = bed->plt_sym_val (i, plt, p);
      if (addr == (bfd_vma) -1)
	continue;

      *s = **p->sym_ptr_ptr;
      /* Undefined syms won't have BSF_LOCAL or BSF_GLOBAL set.  Since
	 we are defining a symbol, ensure one of them is set.  */
      if ((s->flags & BSF_LOCAL) == 0)
	s->flags |= BSF_GLOBAL;
      s->flags |= BSF_SYNTHETIC;
      s->section = plt;
      s->value = addr - plt->vma;
      s->name = names;
      s->udata.p = NULL;
      len = strlen ((*p->sym_ptr_ptr)->name);
      memcpy (names, (*p->sym_ptr_ptr)->name, len);
      names += len;
      if (p->addend != 0)
	{
	  char buf[30], *a;

	  memcpy (names, "+0x", sizeof ("+0x") - 1);
	  names += sizeof ("+0x") - 1;
	  bfd_sprintf_vma (abfd, buf, p->addend);
	  for (a = buf; *a == '0'; ++a)
	    ;
	  len = strlen (a);
	  memcpy (names, a, len);
	  names += len;
	}
      memcpy (names, "@plt", sizeof ("@plt"));
      names += sizeof ("@plt");
      ++s, ++n;
    }

  return n;
}

/* It is only used by x86-64 so far.  */
asection _bfd_elf_large_com_section
  = BFD_FAKE_SECTION (_bfd_elf_large_com_section,
		      SEC_IS_COMMON, NULL, "LARGE_COMMON", 0);

void
_bfd_elf_post_process_headers (bfd * abfd,
			       struct bfd_link_info * link_info ATTRIBUTE_UNUSED)
{
  Elf_Internal_Ehdr * i_ehdrp;	/* ELF file header, internal form.  */

  i_ehdrp = elf_elfheader (abfd);

  i_ehdrp->e_ident[EI_OSABI] = get_elf_backend_data (abfd)->elf_osabi;

  /* To make things simpler for the loader on Linux systems we set the
     osabi field to ELFOSABI_GNU if the binary contains symbols of
     the STT_GNU_IFUNC type or STB_GNU_UNIQUE binding.  */
  if (i_ehdrp->e_ident[EI_OSABI] == ELFOSABI_NONE
      && elf_tdata (abfd)->has_gnu_symbols)
    i_ehdrp->e_ident[EI_OSABI] = ELFOSABI_GNU;
}


/* Return TRUE for ELF symbol types that represent functions.
   This is the default version of this function, which is sufficient for
   most targets.  It returns true if TYPE is STT_FUNC or STT_GNU_IFUNC.  */

bfd_boolean
_bfd_elf_is_function_type (unsigned int type)
{
  return (type == STT_FUNC
	  || type == STT_GNU_IFUNC);
}

/* If the ELF symbol SYM might be a function in SEC, return the
   function size and set *CODE_OFF to the function's entry point,
   otherwise return zero.  */

bfd_size_type
_bfd_elf_maybe_function_sym (const asymbol *sym, asection *sec,
			     bfd_vma *code_off)
{
  bfd_size_type size;

  if ((sym->flags & (BSF_SECTION_SYM | BSF_FILE | BSF_OBJECT
		     | BSF_THREAD_LOCAL | BSF_RELC | BSF_SRELC)) != 0
      || sym->section != sec)
    return 0;

  *code_off = sym->value;
  size = 0;
  if (!(sym->flags & BSF_SYNTHETIC))
    size = ((elf_symbol_type *) sym)->internal_elf_sym.st_size;
  if (size == 0)
    size = 1;
  return size;
}

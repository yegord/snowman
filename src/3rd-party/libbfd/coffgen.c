/* Support for the generic parts of COFF, for BFD.
   Copyright (C) 1990-2014 Free Software Foundation, Inc.
   Written by Cygnus Support.

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

/* Most of this hacked by  Steve Chamberlain, sac@cygnus.com.
   Split out of coffcode.h by Ian Taylor, ian@cygnus.com.  */

/* This file contains COFF code that is not dependent on any
   particular COFF target.  There is only one version of this file in
   libbfd.a, so no target specific code may be put in here.  Or, to
   put it another way,

   ********** DO NOT PUT TARGET SPECIFIC CODE IN THIS FILE **********

   If you need to add some target specific behaviour, add a new hook
   function to bfd_coff_backend_data.

   Some of these functions are also called by the ECOFF routines.
   Those functions may not use any COFF specific information, such as
   coff_data (abfd).  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "libcoff.h"

/* Take a section header read from a coff file (in HOST byte order),
   and make a BFD "section" out of it.  This is used by ECOFF.  */

static bfd_boolean
make_a_section_from_file (bfd *abfd,
			  struct internal_scnhdr *hdr,
			  unsigned int target_index)
{
  asection *return_section;
  char *name;
  bfd_boolean result = TRUE;
  flagword flags;

  name = NULL;

  /* Handle long section names as in PE.  On reading, we want to
    accept long names if the format permits them at all, regardless
    of the current state of the flag that dictates if we would generate
    them in outputs; this construct checks if that is the case by
    attempting to set the flag, without changing its state; the call
    will fail for formats that do not support long names at all.  */
  if (bfd_coff_set_long_section_names (abfd, bfd_coff_long_section_names (abfd))
      && hdr->s_name[0] == '/')
    {
      char buf[SCNNMLEN];
      long strindex;
      char *p;
      const char *strings;

      /* Flag that this BFD uses long names, even though the format might
         expect them to be off by default.  This won't directly affect the
         format of any output BFD created from this one, but the information
         can be used to decide what to do.  */
      bfd_coff_set_long_section_names (abfd, TRUE);
      memcpy (buf, hdr->s_name + 1, SCNNMLEN - 1);
      buf[SCNNMLEN - 1] = '\0';
      strindex = strtol (buf, &p, 10);
      if (*p == '\0' && strindex >= 0)
	{
	  strings = _bfd_coff_read_string_table (abfd);
	  if (strings == NULL)
	    return FALSE;
	  if ((bfd_size_type)(strindex + 2) >= obj_coff_strings_len (abfd))
	    return FALSE;
	  strings += strindex;
	  name = (char *) bfd_alloc (abfd,
                                     (bfd_size_type) strlen (strings) + 1 + 1);
	  if (name == NULL)
	    return FALSE;
	  strcpy (name, strings);
	}
    }

  if (name == NULL)
    {
      /* Assorted wastage to null-terminate the name, thanks AT&T! */
      name = (char *) bfd_alloc (abfd,
                                 (bfd_size_type) sizeof (hdr->s_name) + 1 + 1);
      if (name == NULL)
	return FALSE;
      strncpy (name, (char *) &hdr->s_name[0], sizeof (hdr->s_name));
      name[sizeof (hdr->s_name)] = 0;
    }

  return_section = bfd_make_section_anyway (abfd, name);
  if (return_section == NULL)
    return FALSE;

  return_section->vma = hdr->s_vaddr;
  return_section->lma = hdr->s_paddr;
  return_section->size = hdr->s_size;
  return_section->filepos = hdr->s_scnptr;
  return_section->rel_filepos = hdr->s_relptr;
  return_section->reloc_count = hdr->s_nreloc;

  bfd_coff_set_alignment_hook (abfd, return_section, hdr);

  return_section->line_filepos = hdr->s_lnnoptr;

  return_section->lineno_count = hdr->s_nlnno;
  return_section->userdata = NULL;
  return_section->next = NULL;
  return_section->target_index = target_index;

  if (! bfd_coff_styp_to_sec_flags_hook (abfd, hdr, name, return_section,
					 & flags))
    result = FALSE;

  return_section->flags = flags;

  /* At least on i386-coff, the line number count for a shared library
     section must be ignored.  */
  if ((return_section->flags & SEC_COFF_SHARED_LIBRARY) != 0)
    return_section->lineno_count = 0;

  if (hdr->s_nreloc != 0)
    return_section->flags |= SEC_RELOC;
  /* FIXME: should this check 'hdr->s_size > 0'.  */
  if (hdr->s_scnptr != 0)
    return_section->flags |= SEC_HAS_CONTENTS;

  /* Compress/decompress DWARF debug sections with names: .debug_* and
     .zdebug_*, after the section flags is set.  */
  if ((flags & SEC_DEBUGGING)
      && strlen (name) > 7
      && ((name[1] == 'd' && name[6] == '_')
	  || (strlen (name) > 8 && name[1] == 'z' && name[7] == '_')))
    {
      enum { nothing, compress, decompress } action = nothing;
      char *new_name = NULL;

      if (bfd_is_section_compressed (abfd, return_section))
	{
	  /* Compressed section.  Check if we should decompress.  */
	  if ((abfd->flags & BFD_DECOMPRESS))
	    action = decompress;
	}
      else if (!bfd_is_section_compressed (abfd, return_section))
	{
	  /* Normal section.  Check if we should compress.  */
	  if ((abfd->flags & BFD_COMPRESS) && return_section->size != 0)
	    action = compress;
	}

      switch (action)
	{
	case nothing:
	  break;
	case compress:
	  if (!bfd_init_section_compress_status (abfd, return_section))
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
	  if (!bfd_init_section_decompress_status (abfd, return_section))
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
	bfd_rename_section (abfd, return_section, new_name);
    }

  return result;
}

/* Read in a COFF object and make it into a BFD.  This is used by
   ECOFF as well.  */
const bfd_target *
coff_real_object_p (bfd *,
                    unsigned,
                    struct internal_filehdr *,
                    struct internal_aouthdr *);
const bfd_target *
coff_real_object_p (bfd *abfd,
		    unsigned nscns,
		    struct internal_filehdr *internal_f,
		    struct internal_aouthdr *internal_a)
{
  flagword oflags = abfd->flags;
  bfd_vma ostart = bfd_get_start_address (abfd);
  void * tdata;
  void * tdata_save;
  bfd_size_type readsize;	/* Length of file_info.  */
  unsigned int scnhsz;
  char *external_sections;

  if (!(internal_f->f_flags & F_RELFLG))
    abfd->flags |= HAS_RELOC;
  if ((internal_f->f_flags & F_EXEC))
    abfd->flags |= EXEC_P;
  if (!(internal_f->f_flags & F_LNNO))
    abfd->flags |= HAS_LINENO;
  if (!(internal_f->f_flags & F_LSYMS))
    abfd->flags |= HAS_LOCALS;

  /* FIXME: How can we set D_PAGED correctly?  */
  if ((internal_f->f_flags & F_EXEC) != 0)
    abfd->flags |= D_PAGED;

  bfd_get_symcount (abfd) = internal_f->f_nsyms;
  if (internal_f->f_nsyms)
    abfd->flags |= HAS_SYMS;

  if (internal_a != (struct internal_aouthdr *) NULL)
    bfd_get_start_address (abfd) = internal_a->entry;
  else
    bfd_get_start_address (abfd) = 0;

  /* Set up the tdata area.  ECOFF uses its own routine, and overrides
     abfd->flags.  */
  tdata_save = abfd->tdata.any;
  tdata = bfd_coff_mkobject_hook (abfd, (void *) internal_f, (void *) internal_a);
  if (tdata == NULL)
    goto fail2;

  scnhsz = bfd_coff_scnhsz (abfd);
  readsize = (bfd_size_type) nscns * scnhsz;
  external_sections = (char *) bfd_alloc (abfd, readsize);
  if (!external_sections)
    goto fail;

  if (bfd_bread ((void *) external_sections, readsize, abfd) != readsize)
    goto fail;

  /* Set the arch/mach *before* swapping in sections; section header swapping
     may depend on arch/mach info.  */
  if (! bfd_coff_set_arch_mach_hook (abfd, (void *) internal_f))
    goto fail;

  /* Now copy data as required; construct all asections etc.  */
  if (nscns != 0)
    {
      unsigned int i;
      for (i = 0; i < nscns; i++)
	{
	  struct internal_scnhdr tmp;
	  bfd_coff_swap_scnhdr_in (abfd,
				   (void *) (external_sections + i * scnhsz),
				   (void *) & tmp);
	  if (! make_a_section_from_file (abfd, &tmp, i + 1))
	    goto fail;
	}
    }

  return abfd->xvec;

 fail:
  bfd_release (abfd, tdata);
 fail2:
  abfd->tdata.any = tdata_save;
  abfd->flags = oflags;
  bfd_get_start_address (abfd) = ostart;
  return (const bfd_target *) NULL;
}

/* Turn a COFF file into a BFD, but fail with bfd_error_wrong_format if it is
   not a COFF file.  This is also used by ECOFF.  */

const bfd_target *
coff_object_p (bfd *abfd)
{
  bfd_size_type filhsz;
  bfd_size_type aoutsz;
  unsigned int nscns;
  void * filehdr;
  struct internal_filehdr internal_f;
  struct internal_aouthdr internal_a;

  /* Figure out how much to read.  */
  filhsz = bfd_coff_filhsz (abfd);
  aoutsz = bfd_coff_aoutsz (abfd);

  filehdr = bfd_alloc (abfd, filhsz);
  if (filehdr == NULL)
    return NULL;
  if (bfd_bread (filehdr, filhsz, abfd) != filhsz)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      bfd_release (abfd, filehdr);
      return NULL;
    }
  bfd_coff_swap_filehdr_in (abfd, filehdr, &internal_f);
  bfd_release (abfd, filehdr);

  /* The XCOFF format has two sizes for the f_opthdr.  SMALL_AOUTSZ
     (less than aoutsz) used in object files and AOUTSZ (equal to
     aoutsz) in executables.  The bfd_coff_swap_aouthdr_in function
     expects this header to be aoutsz bytes in length, so we use that
     value in the call to bfd_alloc below.  But we must be careful to
     only read in f_opthdr bytes in the call to bfd_bread.  We should
     also attempt to catch corrupt or non-COFF binaries with a strange
     value for f_opthdr.  */
  if (! bfd_coff_bad_format_hook (abfd, &internal_f)
      || internal_f.f_opthdr > aoutsz)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }
  nscns = internal_f.f_nscns;

  if (internal_f.f_opthdr)
    {
      void * opthdr;

      opthdr = bfd_alloc (abfd, aoutsz);
      if (opthdr == NULL)
	return NULL;
      if (bfd_bread (opthdr, (bfd_size_type) internal_f.f_opthdr, abfd)
	  != internal_f.f_opthdr)
	{
	  bfd_release (abfd, opthdr);
	  return NULL;
	}
      /* PR 17512: file: 11056-1136-0.004.  */
      if (internal_f.f_opthdr < aoutsz)
	memset (((char *) opthdr) + internal_f.f_opthdr, 0, aoutsz - internal_f.f_opthdr);

      bfd_coff_swap_aouthdr_in (abfd, opthdr, (void *) &internal_a);
      bfd_release (abfd, opthdr);
    }

  return coff_real_object_p (abfd, nscns, &internal_f,
			     (internal_f.f_opthdr != 0
			      ? &internal_a
			      : (struct internal_aouthdr *) NULL));
}

/* Get the BFD section from a COFF symbol section number.  */

asection *
coff_section_from_bfd_index (bfd *abfd, int section_index)
{
  struct bfd_section *answer = abfd->sections;

  if (section_index == N_ABS)
    return bfd_abs_section_ptr;
  if (section_index == N_UNDEF)
    return bfd_und_section_ptr;
  if (section_index == N_DEBUG)
    return bfd_abs_section_ptr;

  while (answer)
    {
      if (answer->target_index == section_index)
	return answer;
      answer = answer->next;
    }

  /* We should not reach this point, but the SCO 3.2v4 /lib/libc_s.a
     has a bad symbol table in biglitpow.o.  */
  return bfd_und_section_ptr;
}

/* Get the upper bound of a COFF symbol table.  */

long
coff_get_symtab_upper_bound (bfd *abfd)
{
  if (!bfd_coff_slurp_symbol_table (abfd))
    return -1;

  return (bfd_get_symcount (abfd) + 1) * (sizeof (coff_symbol_type *));
}

/* Canonicalize a COFF symbol table.  */

long
coff_canonicalize_symtab (bfd *abfd, asymbol **alocation)
{
  unsigned int counter;
  coff_symbol_type *symbase;
  coff_symbol_type **location = (coff_symbol_type **) alocation;

  if (!bfd_coff_slurp_symbol_table (abfd))
    return -1;

  symbase = obj_symbols (abfd);
  counter = bfd_get_symcount (abfd);
  while (counter-- > 0)
    *location++ = symbase++;

  *location = NULL;

  return bfd_get_symcount (abfd);
}

/* Get the name of a symbol.  The caller must pass in a buffer of size
   >= SYMNMLEN + 1.  */

const char *
_bfd_coff_internal_syment_name (bfd *abfd,
				const struct internal_syment *sym,
				char *buf)
{
  /* FIXME: It's not clear this will work correctly if sizeof
     (_n_zeroes) != 4.  */
  if (sym->_n._n_n._n_zeroes != 0
      || sym->_n._n_n._n_offset == 0)
    {
      memcpy (buf, sym->_n._n_name, SYMNMLEN);
      buf[SYMNMLEN] = '\0';
      return buf;
    }
  else
    {
      const char *strings;

      BFD_ASSERT (sym->_n._n_n._n_offset >= STRING_SIZE_SIZE);
      strings = obj_coff_strings (abfd);
      if (strings == NULL)
	{
	  strings = _bfd_coff_read_string_table (abfd);
	  if (strings == NULL)
	    return NULL;
	}
      /* PR 17910: Only check for string overflow if the length has been set.
	 Some DLLs, eg those produced by Visual Studio, may not set the length field.  */
      if (obj_coff_strings_len (abfd) > 0
	  && sym->_n._n_n._n_offset >= obj_coff_strings_len (abfd))
	return NULL;
      return strings + sym->_n._n_n._n_offset;
    }
}

/* Read in and swap the relocs.  This returns a buffer holding the
   relocs for section SEC in file ABFD.  If CACHE is TRUE and
   INTERNAL_RELOCS is NULL, the relocs read in will be saved in case
   the function is called again.  If EXTERNAL_RELOCS is not NULL, it
   is a buffer large enough to hold the unswapped relocs.  If
   INTERNAL_RELOCS is not NULL, it is a buffer large enough to hold
   the swapped relocs.  If REQUIRE_INTERNAL is TRUE, then the return
   value must be INTERNAL_RELOCS.  The function returns NULL on error.  */

struct internal_reloc *
_bfd_coff_read_internal_relocs (bfd *abfd,
				asection *sec,
				bfd_boolean cache,
				bfd_byte *external_relocs,
				bfd_boolean require_internal,
				struct internal_reloc *internal_relocs)
{
  bfd_size_type relsz;
  bfd_byte *free_external = NULL;
  struct internal_reloc *free_internal = NULL;
  bfd_byte *erel;
  bfd_byte *erel_end;
  struct internal_reloc *irel;
  bfd_size_type amt;

  if (sec->reloc_count == 0)
    return internal_relocs;	/* Nothing to do.  */

  if (coff_section_data (abfd, sec) != NULL
      && coff_section_data (abfd, sec)->relocs != NULL)
    {
      if (! require_internal)
	return coff_section_data (abfd, sec)->relocs;
      memcpy (internal_relocs, coff_section_data (abfd, sec)->relocs,
	      sec->reloc_count * sizeof (struct internal_reloc));
      return internal_relocs;
    }

  relsz = bfd_coff_relsz (abfd);

  amt = sec->reloc_count * relsz;
  if (external_relocs == NULL)
    {
      free_external = (bfd_byte *) bfd_malloc (amt);
      if (free_external == NULL)
	goto error_return;
      external_relocs = free_external;
    }

  if (bfd_seek (abfd, sec->rel_filepos, SEEK_SET) != 0
      || bfd_bread (external_relocs, amt, abfd) != amt)
    goto error_return;

  if (internal_relocs == NULL)
    {
      amt = sec->reloc_count;
      amt *= sizeof (struct internal_reloc);
      free_internal = (struct internal_reloc *) bfd_malloc (amt);
      if (free_internal == NULL)
	goto error_return;
      internal_relocs = free_internal;
    }

  /* Swap in the relocs.  */
  erel = external_relocs;
  erel_end = erel + relsz * sec->reloc_count;
  irel = internal_relocs;
  for (; erel < erel_end; erel += relsz, irel++)
    bfd_coff_swap_reloc_in (abfd, (void *) erel, (void *) irel);

  if (free_external != NULL)
    {
      free (free_external);
      free_external = NULL;
    }

  if (cache && free_internal != NULL)
    {
      if (coff_section_data (abfd, sec) == NULL)
	{
	  amt = sizeof (struct coff_section_tdata);
	  sec->used_by_bfd = bfd_zalloc (abfd, amt);
	  if (sec->used_by_bfd == NULL)
	    goto error_return;
	  coff_section_data (abfd, sec)->contents = NULL;
	}
      coff_section_data (abfd, sec)->relocs = free_internal;
    }

  return internal_relocs;

 error_return:
  if (free_external != NULL)
    free (free_external);
  if (free_internal != NULL)
    free (free_internal);
  return NULL;
}

/* Set lineno_count for the output sections of a COFF file.  */

int
coff_count_linenumbers (bfd *abfd)
{
  unsigned int limit = bfd_get_symcount (abfd);
  unsigned int i;
  int total = 0;
  asymbol **p;
  asection *s;

  if (limit == 0)
    {
      /* This may be from the backend linker, in which case the
         lineno_count in the sections is correct.  */
      for (s = abfd->sections; s != NULL; s = s->next)
	total += s->lineno_count;
      return total;
    }

  for (s = abfd->sections; s != NULL; s = s->next)
    BFD_ASSERT (s->lineno_count == 0);

  for (p = abfd->outsymbols, i = 0; i < limit; i++, p++)
    {
      asymbol *q_maybe = *p;

      if (bfd_family_coff (bfd_asymbol_bfd (q_maybe)))
	{
	  coff_symbol_type *q = coffsymbol (q_maybe);

	  /* The AIX 4.1 compiler can sometimes generate line numbers
             attached to debugging symbols.  We try to simply ignore
             those here.  */
	  if (q->lineno != NULL
	      && q->symbol.section->owner != NULL)
	    {
	      /* This symbol has line numbers.  Increment the owning
	         section's linenumber count.  */
	      alent *l = q->lineno;

	      do
		{
		  asection * sec = q->symbol.section->output_section;

		  /* Do not try to update fields in read-only sections.  */
		  if (! bfd_is_const_section (sec))
		    sec->lineno_count ++;

		  ++total;
		  ++l;
		}
	      while (l->line_number != 0);
	    }
	}
    }

  return total;
}

/* Takes a bfd and a symbol, returns a pointer to the coff specific
   area of the symbol if there is one.  */

coff_symbol_type *
coff_symbol_from (bfd *ignore_abfd ATTRIBUTE_UNUSED,
		  asymbol *symbol)
{
  if (!bfd_family_coff (bfd_asymbol_bfd (symbol)))
    return (coff_symbol_type *) NULL;

  if (bfd_asymbol_bfd (symbol)->tdata.coff_obj_data == (coff_data_type *) NULL)
    return (coff_symbol_type *) NULL;

  return (coff_symbol_type *) symbol;
}

static void
fixup_symbol_value (bfd *abfd,
		    coff_symbol_type *coff_symbol_ptr,
		    struct internal_syment *syment)
{
  /* Normalize the symbol flags.  */
  if (coff_symbol_ptr->symbol.section
      && bfd_is_com_section (coff_symbol_ptr->symbol.section))
    {
      /* A common symbol is undefined with a value.  */
      syment->n_scnum = N_UNDEF;
      syment->n_value = coff_symbol_ptr->symbol.value;
    }
  else if ((coff_symbol_ptr->symbol.flags & BSF_DEBUGGING) != 0
	   && (coff_symbol_ptr->symbol.flags & BSF_DEBUGGING_RELOC) == 0)
    {
      syment->n_value = coff_symbol_ptr->symbol.value;
    }
  else if (bfd_is_und_section (coff_symbol_ptr->symbol.section))
    {
      syment->n_scnum = N_UNDEF;
      syment->n_value = 0;
    }
  /* FIXME: Do we need to handle the absolute section here?  */
  else
    {
      if (coff_symbol_ptr->symbol.section)
	{
	  syment->n_scnum =
	    coff_symbol_ptr->symbol.section->output_section->target_index;

	  syment->n_value = (coff_symbol_ptr->symbol.value
			     + coff_symbol_ptr->symbol.section->output_offset);
	  if (! obj_pe (abfd))
            {
              syment->n_value += (syment->n_sclass == C_STATLAB)
                ? coff_symbol_ptr->symbol.section->output_section->lma
                : coff_symbol_ptr->symbol.section->output_section->vma;
            }
	}
      else
	{
	  BFD_ASSERT (0);
	  /* This can happen, but I don't know why yet (steve@cygnus.com) */
	  syment->n_scnum = N_ABS;
	  syment->n_value = coff_symbol_ptr->symbol.value;
	}
    }
}

/* Run through all the symbols in the symbol table and work out what
   their indexes into the symbol table will be when output.

   Coff requires that each C_FILE symbol points to the next one in the
   chain, and that the last one points to the first external symbol. We
   do that here too.  */

bfd_boolean
coff_renumber_symbols (bfd *bfd_ptr, int *first_undef)
{
  unsigned int symbol_count = bfd_get_symcount (bfd_ptr);
  asymbol **symbol_ptr_ptr = bfd_ptr->outsymbols;
  unsigned int native_index = 0;
  struct internal_syment *last_file = NULL;
  unsigned int symbol_index;

  /* COFF demands that undefined symbols come after all other symbols.
     Since we don't need to impose this extra knowledge on all our
     client programs, deal with that here.  Sort the symbol table;
     just move the undefined symbols to the end, leaving the rest
     alone.  The O'Reilly book says that defined global symbols come
     at the end before the undefined symbols, so we do that here as
     well.  */
  /* @@ Do we have some condition we could test for, so we don't always
     have to do this?  I don't think relocatability is quite right, but
     I'm not certain.  [raeburn:19920508.1711EST]  */
  {
    asymbol **newsyms;
    unsigned int i;
    bfd_size_type amt;

    amt = sizeof (asymbol *) * ((bfd_size_type) symbol_count + 1);
    newsyms = (asymbol **) bfd_alloc (bfd_ptr, amt);
    if (!newsyms)
      return FALSE;
    bfd_ptr->outsymbols = newsyms;
    for (i = 0; i < symbol_count; i++)
      if ((symbol_ptr_ptr[i]->flags & BSF_NOT_AT_END) != 0
	  || (!bfd_is_und_section (symbol_ptr_ptr[i]->section)
	      && !bfd_is_com_section (symbol_ptr_ptr[i]->section)
	      && ((symbol_ptr_ptr[i]->flags & BSF_FUNCTION) != 0
		  || ((symbol_ptr_ptr[i]->flags & (BSF_GLOBAL | BSF_WEAK))
		      == 0))))
	*newsyms++ = symbol_ptr_ptr[i];

    for (i = 0; i < symbol_count; i++)
      if ((symbol_ptr_ptr[i]->flags & BSF_NOT_AT_END) == 0
	  && !bfd_is_und_section (symbol_ptr_ptr[i]->section)
	  && (bfd_is_com_section (symbol_ptr_ptr[i]->section)
	      || ((symbol_ptr_ptr[i]->flags & BSF_FUNCTION) == 0
		  && ((symbol_ptr_ptr[i]->flags & (BSF_GLOBAL | BSF_WEAK))
		      != 0))))
	*newsyms++ = symbol_ptr_ptr[i];

    *first_undef = newsyms - bfd_ptr->outsymbols;

    for (i = 0; i < symbol_count; i++)
      if ((symbol_ptr_ptr[i]->flags & BSF_NOT_AT_END) == 0
	  && bfd_is_und_section (symbol_ptr_ptr[i]->section))
	*newsyms++ = symbol_ptr_ptr[i];
    *newsyms = (asymbol *) NULL;
    symbol_ptr_ptr = bfd_ptr->outsymbols;
  }

  for (symbol_index = 0; symbol_index < symbol_count; symbol_index++)
    {
      coff_symbol_type *coff_symbol_ptr;

      coff_symbol_ptr = coff_symbol_from (bfd_ptr, symbol_ptr_ptr[symbol_index]);
      symbol_ptr_ptr[symbol_index]->udata.i = symbol_index;
      if (coff_symbol_ptr && coff_symbol_ptr->native)
	{
	  combined_entry_type *s = coff_symbol_ptr->native;
	  int i;

	  BFD_ASSERT (s->is_sym);
	  if (s->u.syment.n_sclass == C_FILE)
	    {
	      if (last_file != NULL)
		last_file->n_value = native_index;
	      last_file = &(s->u.syment);
	    }
	  else
	    /* Modify the symbol values according to their section and
	       type.  */
	    fixup_symbol_value (bfd_ptr, coff_symbol_ptr, &(s->u.syment));

	  for (i = 0; i < s->u.syment.n_numaux + 1; i++)
	    s[i].offset = native_index++;
	}
      else
	native_index++;
    }

  obj_conv_table_size (bfd_ptr) = native_index;

  return TRUE;
}

/* Run thorough the symbol table again, and fix it so that all
   pointers to entries are changed to the entries' index in the output
   symbol table.  */

void
coff_mangle_symbols (bfd *bfd_ptr)
{
  unsigned int symbol_count = bfd_get_symcount (bfd_ptr);
  asymbol **symbol_ptr_ptr = bfd_ptr->outsymbols;
  unsigned int symbol_index;

  for (symbol_index = 0; symbol_index < symbol_count; symbol_index++)
    {
      coff_symbol_type *coff_symbol_ptr;

      coff_symbol_ptr = coff_symbol_from (bfd_ptr, symbol_ptr_ptr[symbol_index]);
      if (coff_symbol_ptr && coff_symbol_ptr->native)
	{
	  int i;
	  combined_entry_type *s = coff_symbol_ptr->native;

	  BFD_ASSERT (s->is_sym);
	  if (s->fix_value)
	    {
	      /* FIXME: We should use a union here.  */
	      s->u.syment.n_value =
		(bfd_hostptr_t) ((combined_entry_type *)
			  ((bfd_hostptr_t) s->u.syment.n_value))->offset;
	      s->fix_value = 0;
	    }
	  if (s->fix_line)
	    {
	      /* The value is the offset into the line number entries
                 for the symbol's section.  On output, the symbol's
                 section should be N_DEBUG.  */
	      s->u.syment.n_value =
		(coff_symbol_ptr->symbol.section->output_section->line_filepos
		 + s->u.syment.n_value * bfd_coff_linesz (bfd_ptr));
	      coff_symbol_ptr->symbol.section =
		coff_section_from_bfd_index (bfd_ptr, N_DEBUG);
	      BFD_ASSERT (coff_symbol_ptr->symbol.flags & BSF_DEBUGGING);
	    }
	  for (i = 0; i < s->u.syment.n_numaux; i++)
	    {
	      combined_entry_type *a = s + i + 1;

	      BFD_ASSERT (! a->is_sym);
	      if (a->fix_tag)
		{
		  a->u.auxent.x_sym.x_tagndx.l =
		    a->u.auxent.x_sym.x_tagndx.p->offset;
		  a->fix_tag = 0;
		}
	      if (a->fix_end)
		{
		  a->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l =
		    a->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.p->offset;
		  a->fix_end = 0;
		}
	      if (a->fix_scnlen)
		{
		  a->u.auxent.x_csect.x_scnlen.l =
		    a->u.auxent.x_csect.x_scnlen.p->offset;
		  a->fix_scnlen = 0;
		}
	    }
	}
    }
}

static void
coff_fix_symbol_name (bfd *abfd,
		      asymbol *symbol,
		      combined_entry_type *native,
		      bfd_size_type *string_size_p,
		      asection **debug_string_section_p,
		      bfd_size_type *debug_string_size_p)
{
  unsigned int name_length;
  union internal_auxent *auxent;
  char *name = (char *) (symbol->name);

  if (name == NULL)
    {
      /* COFF symbols always have names, so we'll make one up.  */
      symbol->name = "strange";
      name = (char *) symbol->name;
    }
  name_length = strlen (name);

  BFD_ASSERT (native->is_sym);
  if (native->u.syment.n_sclass == C_FILE
      && native->u.syment.n_numaux > 0)
    {
      unsigned int filnmlen;

      if (bfd_coff_force_symnames_in_strings (abfd))
	{
          native->u.syment._n._n_n._n_offset =
	      (*string_size_p + STRING_SIZE_SIZE);
	  native->u.syment._n._n_n._n_zeroes = 0;
	  *string_size_p += 6;  /* strlen(".file") + 1 */
	}
      else
  	strncpy (native->u.syment._n._n_name, ".file", SYMNMLEN);

      BFD_ASSERT (! (native + 1)->is_sym);
      auxent = &(native + 1)->u.auxent;

      filnmlen = bfd_coff_filnmlen (abfd);

      if (bfd_coff_long_filenames (abfd))
	{
	  if (name_length <= filnmlen)
	    strncpy (auxent->x_file.x_fname, name, filnmlen);
	  else
	    {
	      auxent->x_file.x_n.x_offset = *string_size_p + STRING_SIZE_SIZE;
	      auxent->x_file.x_n.x_zeroes = 0;
	      *string_size_p += name_length + 1;
	    }
	}
      else
	{
	  strncpy (auxent->x_file.x_fname, name, filnmlen);
	  if (name_length > filnmlen)
	    name[filnmlen] = '\0';
	}
    }
  else
    {
      if (name_length <= SYMNMLEN && !bfd_coff_force_symnames_in_strings (abfd))
	/* This name will fit into the symbol neatly.  */
	strncpy (native->u.syment._n._n_name, symbol->name, SYMNMLEN);

      else if (!bfd_coff_symname_in_debug (abfd, &native->u.syment))
	{
	  native->u.syment._n._n_n._n_offset = (*string_size_p
						+ STRING_SIZE_SIZE);
	  native->u.syment._n._n_n._n_zeroes = 0;
	  *string_size_p += name_length + 1;
	}
      else
	{
	  file_ptr filepos;
	  bfd_byte buf[4];
	  int prefix_len = bfd_coff_debug_string_prefix_length (abfd);

	  /* This name should be written into the .debug section.  For
	     some reason each name is preceded by a two byte length
	     and also followed by a null byte.  FIXME: We assume that
	     the .debug section has already been created, and that it
	     is large enough.  */
	  if (*debug_string_section_p == (asection *) NULL)
	    *debug_string_section_p = bfd_get_section_by_name (abfd, ".debug");
	  filepos = bfd_tell (abfd);
	  if (prefix_len == 4)
	    bfd_put_32 (abfd, (bfd_vma) (name_length + 1), buf);
	  else
	    bfd_put_16 (abfd, (bfd_vma) (name_length + 1), buf);

	  if (!bfd_set_section_contents (abfd,
					 *debug_string_section_p,
					 (void *) buf,
					 (file_ptr) *debug_string_size_p,
					 (bfd_size_type) prefix_len)
	      || !bfd_set_section_contents (abfd,
					    *debug_string_section_p,
					    (void *) symbol->name,
					    (file_ptr) (*debug_string_size_p
							+ prefix_len),
					    (bfd_size_type) name_length + 1))
	    abort ();
	  if (bfd_seek (abfd, filepos, SEEK_SET) != 0)
	    abort ();
	  native->u.syment._n._n_n._n_offset =
	      *debug_string_size_p + prefix_len;
	  native->u.syment._n._n_n._n_zeroes = 0;
	  *debug_string_size_p += name_length + 1 + prefix_len;
	}
    }
}

/* We need to keep track of the symbol index so that when we write out
   the relocs we can get the index for a symbol.  This method is a
   hack.  FIXME.  */

#define set_index(symbol, idx)	((symbol)->udata.i = (idx))

/* Write a symbol out to a COFF file.  */

static bfd_boolean
coff_write_symbol (bfd *abfd,
		   asymbol *symbol,
		   combined_entry_type *native,
		   bfd_vma *written,
		   bfd_size_type *string_size_p,
		   asection **debug_string_section_p,
		   bfd_size_type *debug_string_size_p)
{
  unsigned int numaux = native->u.syment.n_numaux;
  int type = native->u.syment.n_type;
  int n_sclass = (int) native->u.syment.n_sclass;
  asection *output_section = symbol->section->output_section
			       ? symbol->section->output_section
			       : symbol->section;
  void * buf;
  bfd_size_type symesz;

  BFD_ASSERT (native->is_sym);

  if (native->u.syment.n_sclass == C_FILE)
    symbol->flags |= BSF_DEBUGGING;

  if (symbol->flags & BSF_DEBUGGING
      && bfd_is_abs_section (symbol->section))
    native->u.syment.n_scnum = N_DEBUG;

  else if (bfd_is_abs_section (symbol->section))
    native->u.syment.n_scnum = N_ABS;

  else if (bfd_is_und_section (symbol->section))
    native->u.syment.n_scnum = N_UNDEF;

  else
    native->u.syment.n_scnum =
      output_section->target_index;

  coff_fix_symbol_name (abfd, symbol, native, string_size_p,
			debug_string_section_p, debug_string_size_p);

  symesz = bfd_coff_symesz (abfd);
  buf = bfd_alloc (abfd, symesz);
  if (!buf)
    return FALSE;
  bfd_coff_swap_sym_out (abfd, &native->u.syment, buf);
  if (bfd_bwrite (buf, symesz, abfd) != symesz)
    return FALSE;
  bfd_release (abfd, buf);

  if (native->u.syment.n_numaux > 0)
    {
      bfd_size_type auxesz;
      unsigned int j;

      auxesz = bfd_coff_auxesz (abfd);
      buf = bfd_alloc (abfd, auxesz);
      if (!buf)
	return FALSE;
      for (j = 0; j < native->u.syment.n_numaux; j++)
	{
	  BFD_ASSERT (! (native + j + 1)->is_sym);
	  bfd_coff_swap_aux_out (abfd,
				 &((native + j + 1)->u.auxent),
				 type, n_sclass, (int) j,
				 native->u.syment.n_numaux,
				 buf);
	  if (bfd_bwrite (buf, auxesz, abfd) != auxesz)
	    return FALSE;
	}
      bfd_release (abfd, buf);
    }

  /* Store the index for use when we write out the relocs.  */
  set_index (symbol, *written);

  *written += numaux + 1;
  return TRUE;
}

/* Write out a symbol to a COFF file that does not come from a COFF
   file originally.  This symbol may have been created by the linker,
   or we may be linking a non COFF file to a COFF file.  */

bfd_boolean
coff_write_alien_symbol (bfd *abfd,
			 asymbol *symbol,
			 struct internal_syment *isym,
			 bfd_vma *written,
			 bfd_size_type *string_size_p,
			 asection **debug_string_section_p,
			 bfd_size_type *debug_string_size_p)
{
  combined_entry_type *native;
  combined_entry_type dummy[2];
  asection *output_section = symbol->section->output_section
			       ? symbol->section->output_section
			       : symbol->section;
  struct bfd_link_info *link_info = coff_data (abfd)->link_info;
  bfd_boolean ret;

  if ((!link_info || link_info->strip_discarded)
      && !bfd_is_abs_section (symbol->section)
      && symbol->section->output_section == bfd_abs_section_ptr)
    {
      symbol->name = "";
      if (isym != NULL)
        memset (isym, 0, sizeof (*isym));
      return TRUE;
    }
  native = dummy;
  native->is_sym = TRUE;
  native[1].is_sym = FALSE;
  native->u.syment.n_type = T_NULL;
  native->u.syment.n_flags = 0;
  native->u.syment.n_numaux = 0;
  if (bfd_is_und_section (symbol->section))
    {
      native->u.syment.n_scnum = N_UNDEF;
      native->u.syment.n_value = symbol->value;
    }
  else if (bfd_is_com_section (symbol->section))
    {
      native->u.syment.n_scnum = N_UNDEF;
      native->u.syment.n_value = symbol->value;
    }
  else if (symbol->flags & BSF_FILE)
    {
      native->u.syment.n_scnum = N_DEBUG;
      native->u.syment.n_numaux = 1;
    }
  else if (symbol->flags & BSF_DEBUGGING)
    {
      /* There isn't much point to writing out a debugging symbol
         unless we are prepared to convert it into COFF debugging
         format.  So, we just ignore them.  We must clobber the symbol
         name to keep it from being put in the string table.  */
      symbol->name = "";
      if (isym != NULL)
        memset (isym, 0, sizeof (*isym));
      return TRUE;
    }
  else
    {
      native->u.syment.n_scnum = output_section->target_index;
      native->u.syment.n_value = (symbol->value
				  + symbol->section->output_offset);
      if (! obj_pe (abfd))
	native->u.syment.n_value += output_section->vma;

      /* Copy the any flags from the file header into the symbol.
         FIXME: Why?  */
      {
	coff_symbol_type *c = coff_symbol_from (abfd, symbol);
	if (c != (coff_symbol_type *) NULL)
	  native->u.syment.n_flags = bfd_asymbol_bfd (&c->symbol)->flags;
      }
    }

  native->u.syment.n_type = 0;
  if (symbol->flags & BSF_FILE)
    native->u.syment.n_sclass = C_FILE;
  else if (symbol->flags & BSF_LOCAL)
    native->u.syment.n_sclass = C_STAT;
  else if (symbol->flags & BSF_WEAK)
    native->u.syment.n_sclass = obj_pe (abfd) ? C_NT_WEAK : C_WEAKEXT;
  else
    native->u.syment.n_sclass = C_EXT;

  ret = coff_write_symbol (abfd, symbol, native, written, string_size_p,
			   debug_string_section_p, debug_string_size_p);
  if (isym != NULL)
    *isym = native->u.syment;
  return ret;
}

/* Write a native symbol to a COFF file.  */

static bfd_boolean
coff_write_native_symbol (bfd *abfd,
			  coff_symbol_type *symbol,
			  bfd_vma *written,
			  bfd_size_type *string_size_p,
			  asection **debug_string_section_p,
			  bfd_size_type *debug_string_size_p)
{
  combined_entry_type *native = symbol->native;
  alent *lineno = symbol->lineno;
  struct bfd_link_info *link_info = coff_data (abfd)->link_info;

  if ((!link_info || link_info->strip_discarded)
      && !bfd_is_abs_section (symbol->symbol.section)
      && symbol->symbol.section->output_section == bfd_abs_section_ptr)
    {
      symbol->symbol.name = "";
      return TRUE;
    }

  BFD_ASSERT (native->is_sym);
  /* If this symbol has an associated line number, we must store the
     symbol index in the line number field.  We also tag the auxent to
     point to the right place in the lineno table.  */
  if (lineno && !symbol->done_lineno && symbol->symbol.section->owner != NULL)
    {
      unsigned int count = 0;

      lineno[count].u.offset = *written;
      if (native->u.syment.n_numaux)
	{
	  union internal_auxent *a = &((native + 1)->u.auxent);

	  a->x_sym.x_fcnary.x_fcn.x_lnnoptr =
	    symbol->symbol.section->output_section->moving_line_filepos;
	}

      /* Count and relocate all other linenumbers.  */
      count++;
      while (lineno[count].line_number != 0)
	{
	  lineno[count].u.offset +=
	    (symbol->symbol.section->output_section->vma
	     + symbol->symbol.section->output_offset);
	  count++;
	}
      symbol->done_lineno = TRUE;

      if (! bfd_is_const_section (symbol->symbol.section->output_section))
	symbol->symbol.section->output_section->moving_line_filepos +=
	  count * bfd_coff_linesz (abfd);
    }

  return coff_write_symbol (abfd, &(symbol->symbol), native, written,
			    string_size_p, debug_string_section_p,
			    debug_string_size_p);
}

static void
null_error_handler (const char * fmt ATTRIBUTE_UNUSED, ...)
{
}

/* Write out the COFF symbols.  */

bfd_boolean
coff_write_symbols (bfd *abfd)
{
  bfd_size_type string_size;
  asection *debug_string_section;
  bfd_size_type debug_string_size;
  unsigned int i;
  unsigned int limit = bfd_get_symcount (abfd);
  bfd_vma written = 0;
  asymbol **p;

  string_size = 0;
  debug_string_section = NULL;
  debug_string_size = 0;

  /* If this target supports long section names, they must be put into
     the string table.  This is supported by PE.  This code must
     handle section names just as they are handled in
     coff_write_object_contents.  */
  if (bfd_coff_long_section_names (abfd))
    {
      asection *o;

      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  size_t len;

	  len = strlen (o->name);
	  if (len > SCNNMLEN)
	    string_size += len + 1;
	}
    }

  /* Seek to the right place.  */
  if (bfd_seek (abfd, obj_sym_filepos (abfd), SEEK_SET) != 0)
    return FALSE;

  /* Output all the symbols we have.  */
  written = 0;
  for (p = abfd->outsymbols, i = 0; i < limit; i++, p++)
    {
      asymbol *symbol = *p;
      coff_symbol_type *c_symbol = coff_symbol_from (abfd, symbol);

      if (c_symbol == (coff_symbol_type *) NULL
	  || c_symbol->native == (combined_entry_type *) NULL)
	{
	  if (!coff_write_alien_symbol (abfd, symbol, NULL, &written,
					&string_size, &debug_string_section,
					&debug_string_size))
	    return FALSE;
	}
      else
	{
	  if (coff_backend_info (abfd)->_bfd_coff_classify_symbol != NULL)
	    {
	      bfd_error_handler_type current_error_handler;
	      enum coff_symbol_classification sym_class;
	      unsigned char *n_sclass;

	      /* Suppress error reporting by bfd_coff_classify_symbol.
		 Error messages can be generated when we are processing a local
		 symbol which has no associated section and we do not have to
		 worry about this, all we need to know is that it is local.  */
	      current_error_handler = bfd_set_error_handler (null_error_handler);
	      BFD_ASSERT (c_symbol->native->is_sym);
	      sym_class = bfd_coff_classify_symbol (abfd,
						    &c_symbol->native->u.syment);
	      (void) bfd_set_error_handler (current_error_handler);

	      n_sclass = &c_symbol->native->u.syment.n_sclass;

	      /* If the symbol class has been changed (eg objcopy/ld script/etc)
		 we cannot retain the existing sclass from the original symbol.
		 Weak symbols only have one valid sclass, so just set it always.
		 If it is not local class and should be, set it C_STAT.
		 If it is global and not classified as global, or if it is
		 weak (which is also classified as global), set it C_EXT.  */

	      if (symbol->flags & BSF_WEAK)
		*n_sclass = obj_pe (abfd) ? C_NT_WEAK : C_WEAKEXT;
	      else if (symbol->flags & BSF_LOCAL && sym_class != COFF_SYMBOL_LOCAL)
		*n_sclass = C_STAT;
	      else if (symbol->flags & BSF_GLOBAL
		       && (sym_class != COFF_SYMBOL_GLOBAL
#ifdef COFF_WITH_PE
			   || *n_sclass == C_NT_WEAK
#endif
			   || *n_sclass == C_WEAKEXT))
		c_symbol->native->u.syment.n_sclass = C_EXT;
	    }

	  if (!coff_write_native_symbol (abfd, c_symbol, &written,
					 &string_size, &debug_string_section,
					 &debug_string_size))
	    return FALSE;
	}
    }

  obj_raw_syment_count (abfd) = written;

  /* Now write out strings.  */
  if (string_size != 0)
    {
      unsigned int size = string_size + STRING_SIZE_SIZE;
      bfd_byte buffer[STRING_SIZE_SIZE];

#if STRING_SIZE_SIZE == 4
      H_PUT_32 (abfd, size, buffer);
#else
 #error Change H_PUT_32
#endif
      if (bfd_bwrite ((void *) buffer, (bfd_size_type) sizeof (buffer), abfd)
	  != sizeof (buffer))
	return FALSE;

      /* Handle long section names.  This code must handle section
	 names just as they are handled in coff_write_object_contents.  */
      if (bfd_coff_long_section_names (abfd))
	{
	  asection *o;

	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      size_t len;

	      len = strlen (o->name);
	      if (len > SCNNMLEN)
		{
		  if (bfd_bwrite (o->name, (bfd_size_type) (len + 1), abfd)
		      != len + 1)
		    return FALSE;
		}
	    }
	}

      for (p = abfd->outsymbols, i = 0;
	   i < limit;
	   i++, p++)
	{
	  asymbol *q = *p;
	  size_t name_length = strlen (q->name);
	  coff_symbol_type *c_symbol = coff_symbol_from (abfd, q);
	  size_t maxlen;

	  /* Figure out whether the symbol name should go in the string
	     table.  Symbol names that are short enough are stored
	     directly in the syment structure.  File names permit a
	     different, longer, length in the syment structure.  On
	     XCOFF, some symbol names are stored in the .debug section
	     rather than in the string table.  */

	  if (c_symbol == NULL
	      || c_symbol->native == NULL)
	    /* This is not a COFF symbol, so it certainly is not a
	       file name, nor does it go in the .debug section.  */
	    maxlen = bfd_coff_force_symnames_in_strings (abfd) ? 0 : SYMNMLEN;

	  else if (! c_symbol->native->is_sym)
	    maxlen = bfd_coff_force_symnames_in_strings (abfd) ? 0 : SYMNMLEN;
	    
	  else if (bfd_coff_symname_in_debug (abfd,
					      &c_symbol->native->u.syment))
	    /* This symbol name is in the XCOFF .debug section.
	       Don't write it into the string table.  */
	    maxlen = name_length;

	  else if (c_symbol->native->u.syment.n_sclass == C_FILE
		   && c_symbol->native->u.syment.n_numaux > 0)
	    {
	      if (bfd_coff_force_symnames_in_strings (abfd))
		{
		  if (bfd_bwrite (".file", (bfd_size_type) 6, abfd) != 6)
		    return FALSE;
		}
	      maxlen = bfd_coff_filnmlen (abfd);
	    }
	  else
	    maxlen = bfd_coff_force_symnames_in_strings (abfd) ? 0 : SYMNMLEN;

	  if (name_length > maxlen)
	    {
	      if (bfd_bwrite ((void *) (q->name), (bfd_size_type) name_length + 1,
			     abfd) != name_length + 1)
		return FALSE;
	    }
	}
    }
  else
    {
      /* We would normally not write anything here, but we'll write
         out 4 so that any stupid coff reader which tries to read the
         string table even when there isn't one won't croak.  */
      unsigned int size = STRING_SIZE_SIZE;
      bfd_byte buffer[STRING_SIZE_SIZE];

#if STRING_SIZE_SIZE == 4
      H_PUT_32 (abfd, size, buffer);
#else
 #error Change H_PUT_32
#endif
      if (bfd_bwrite ((void *) buffer, (bfd_size_type) STRING_SIZE_SIZE, abfd)
	  != STRING_SIZE_SIZE)
	return FALSE;
    }

  /* Make sure the .debug section was created to be the correct size.
     We should create it ourselves on the fly, but we don't because
     BFD won't let us write to any section until we know how large all
     the sections are.  We could still do it by making another pass
     over the symbols.  FIXME.  */
  BFD_ASSERT (debug_string_size == 0
	      || (debug_string_section != (asection *) NULL
		  && (BFD_ALIGN (debug_string_size,
				 1 << debug_string_section->alignment_power)
		      == debug_string_section->size)));

  return TRUE;
}

bfd_boolean
coff_write_linenumbers (bfd *abfd)
{
  asection *s;
  bfd_size_type linesz;
  void * buff;

  linesz = bfd_coff_linesz (abfd);
  buff = bfd_alloc (abfd, linesz);
  if (!buff)
    return FALSE;
  for (s = abfd->sections; s != (asection *) NULL; s = s->next)
    {
      if (s->lineno_count)
	{
	  asymbol **q = abfd->outsymbols;
	  if (bfd_seek (abfd, s->line_filepos, SEEK_SET) != 0)
	    return FALSE;
	  /* Find all the linenumbers in this section.  */
	  while (*q)
	    {
	      asymbol *p = *q;
	      if (p->section->output_section == s)
		{
		  alent *l =
		  BFD_SEND (bfd_asymbol_bfd (p), _get_lineno,
			    (bfd_asymbol_bfd (p), p));
		  if (l)
		    {
		      /* Found a linenumber entry, output.  */
		      struct internal_lineno out;

		      memset ((void *) & out, 0, sizeof (out));
		      out.l_lnno = 0;
		      out.l_addr.l_symndx = l->u.offset;
		      bfd_coff_swap_lineno_out (abfd, &out, buff);
		      if (bfd_bwrite (buff, (bfd_size_type) linesz, abfd)
			  != linesz)
			return FALSE;
		      l++;
		      while (l->line_number)
			{
			  out.l_lnno = l->line_number;
			  out.l_addr.l_symndx = l->u.offset;
			  bfd_coff_swap_lineno_out (abfd, &out, buff);
			  if (bfd_bwrite (buff, (bfd_size_type) linesz, abfd)
			      != linesz)
			    return FALSE;
			  l++;
			}
		    }
		}
	      q++;
	    }
	}
    }
  bfd_release (abfd, buff);
  return TRUE;
}

alent *
coff_get_lineno (bfd *ignore_abfd ATTRIBUTE_UNUSED, asymbol *symbol)
{
  return coffsymbol (symbol)->lineno;
}

/* This function transforms the offsets into the symbol table into
   pointers to syments.  */

static void
coff_pointerize_aux (bfd *abfd,
		     combined_entry_type *table_base,
		     combined_entry_type *symbol,
		     unsigned int indaux,
		     combined_entry_type *auxent)
{
  unsigned int type = symbol->u.syment.n_type;
  unsigned int n_sclass = symbol->u.syment.n_sclass;

  BFD_ASSERT (symbol->is_sym);
  if (coff_backend_info (abfd)->_bfd_coff_pointerize_aux_hook)
    {
      if ((*coff_backend_info (abfd)->_bfd_coff_pointerize_aux_hook)
	  (abfd, table_base, symbol, indaux, auxent))
	return;
    }

  /* Don't bother if this is a file or a section.  */
  if (n_sclass == C_STAT && type == T_NULL)
    return;
  if (n_sclass == C_FILE)
    return;

  BFD_ASSERT (! auxent->is_sym);
  /* Otherwise patch up.  */
#define N_TMASK coff_data  (abfd)->local_n_tmask
#define N_BTSHFT coff_data (abfd)->local_n_btshft

  if ((ISFCN (type) || ISTAG (n_sclass) || n_sclass == C_BLOCK
       || n_sclass == C_FCN)
      && auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l > 0)
    {
      auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.p =
	table_base + auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l;
      auxent->fix_end = 1;
    }
  /* A negative tagndx is meaningless, but the SCO 3.2v4 cc can
     generate one, so we must be careful to ignore it.  */
  if (auxent->u.auxent.x_sym.x_tagndx.l > 0)
    {
      auxent->u.auxent.x_sym.x_tagndx.p =
	table_base + auxent->u.auxent.x_sym.x_tagndx.l;
      auxent->fix_tag = 1;
    }
}

/* Allocate space for the ".debug" section, and read it.
   We did not read the debug section until now, because
   we didn't want to go to the trouble until someone needed it.  */

static char *
build_debug_section (bfd *abfd, asection ** sect_return)
{
  char *debug_section;
  file_ptr position;
  bfd_size_type sec_size;

  asection *sect = bfd_get_section_by_name (abfd, ".debug");

  if (!sect)
    {
      bfd_set_error (bfd_error_no_debug_section);
      return NULL;
    }

  sec_size = sect->size;
  debug_section = (char *) bfd_alloc (abfd, sec_size);
  if (debug_section == NULL)
    return NULL;

  /* Seek to the beginning of the `.debug' section and read it.
     Save the current position first; it is needed by our caller.
     Then read debug section and reset the file pointer.  */

  position = bfd_tell (abfd);
  if (bfd_seek (abfd, sect->filepos, SEEK_SET) != 0
      || bfd_bread (debug_section, sec_size, abfd) != sec_size
      || bfd_seek (abfd, position, SEEK_SET) != 0)
    return NULL;

  * sect_return = sect;
  return debug_section;
}

/* Return a pointer to a malloc'd copy of 'name'.  'name' may not be
   \0-terminated, but will not exceed 'maxlen' characters.  The copy *will*
   be \0-terminated.  */

static char *
copy_name (bfd *abfd, char *name, size_t maxlen)
{
  size_t len;
  char *newname;

  for (len = 0; len < maxlen; ++len)
    if (name[len] == '\0')
      break;

  if ((newname = (char *) bfd_alloc (abfd, (bfd_size_type) len + 1)) == NULL)
    return NULL;

  strncpy (newname, name, len);
  newname[len] = '\0';
  return newname;
}

/* Read in the external symbols.  */

bfd_boolean
_bfd_coff_get_external_symbols (bfd *abfd)
{
  bfd_size_type symesz;
  bfd_size_type size;
  void * syms;

  if (obj_coff_external_syms (abfd) != NULL)
    return TRUE;

  symesz = bfd_coff_symesz (abfd);

  size = obj_raw_syment_count (abfd) * symesz;
  if (size == 0)
    return TRUE;

  syms = bfd_malloc (size);
  if (syms == NULL)
    return FALSE;

  if (bfd_seek (abfd, obj_sym_filepos (abfd), SEEK_SET) != 0
      || bfd_bread (syms, size, abfd) != size)
    {
      if (syms != NULL)
	free (syms);
      return FALSE;
    }

  obj_coff_external_syms (abfd) = syms;

  return TRUE;
}

/* Read in the external strings.  The strings are not loaded until
   they are needed.  This is because we have no simple way of
   detecting a missing string table in an archive.  If the strings
   are loaded then the STRINGS and STRINGS_LEN fields in the
   coff_tdata structure will be set.  */

const char *
_bfd_coff_read_string_table (bfd *abfd)
{
  char extstrsize[STRING_SIZE_SIZE];
  bfd_size_type strsize;
  char *strings;
  file_ptr pos;

  if (obj_coff_strings (abfd) != NULL)
    return obj_coff_strings (abfd);

  if (obj_sym_filepos (abfd) == 0)
    {
      bfd_set_error (bfd_error_no_symbols);
      return NULL;
    }

  pos = obj_sym_filepos (abfd);
  pos += obj_raw_syment_count (abfd) * bfd_coff_symesz (abfd);
  if (bfd_seek (abfd, pos, SEEK_SET) != 0)
    return NULL;

  if (bfd_bread (extstrsize, (bfd_size_type) sizeof extstrsize, abfd)
      != sizeof extstrsize)
    {
      if (bfd_get_error () != bfd_error_file_truncated)
	return NULL;

      /* There is no string table.  */
      strsize = STRING_SIZE_SIZE;
    }
  else
    {
#if STRING_SIZE_SIZE == 4
      strsize = H_GET_32 (abfd, extstrsize);
#else
 #error Change H_GET_32
#endif
    }

  if (strsize < STRING_SIZE_SIZE)
    {
      (*_bfd_error_handler)
	(_("%B: bad string table size %lu"), abfd, (unsigned long) strsize);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }

  strings = (char *) bfd_malloc (strsize + 1);
  if (strings == NULL)
    return NULL;

  /* PR 17521 file: 079-54929-0.004.
     A corrupt file could contain an index that points into the first
     STRING_SIZE_SIZE bytes of the string table, so make sure that
     they are zero.  */
  memset (strings, 0, STRING_SIZE_SIZE);

  if (bfd_bread (strings + STRING_SIZE_SIZE, strsize - STRING_SIZE_SIZE, abfd)
      != strsize - STRING_SIZE_SIZE)
    {
      free (strings);
      return NULL;
    }

  obj_coff_strings (abfd) = strings;
  obj_coff_strings_len (abfd) = strsize;
  /* Terminate the string table, just in case.  */
  strings[strsize] = 0;
  return strings;
}

/* Free up the external symbols and strings read from a COFF file.  */

bfd_boolean
_bfd_coff_free_symbols (bfd *abfd)
{
  if (obj_coff_external_syms (abfd) != NULL
      && ! obj_coff_keep_syms (abfd))
    {
      free (obj_coff_external_syms (abfd));
      obj_coff_external_syms (abfd) = NULL;
    }
  if (obj_coff_strings (abfd) != NULL
      && ! obj_coff_keep_strings (abfd))
    {
      free (obj_coff_strings (abfd));
      obj_coff_strings (abfd) = NULL;
      obj_coff_strings_len (abfd) = 0;
    }
  return TRUE;
}

/* Read a symbol table into freshly bfd_allocated memory, swap it, and
   knit the symbol names into a normalized form.  By normalized here I
   mean that all symbols have an n_offset pointer that points to a null-
   terminated string.  */

combined_entry_type *
coff_get_normalized_symtab (bfd *abfd)
{
  combined_entry_type *internal;
  combined_entry_type *internal_ptr;
  combined_entry_type *symbol_ptr;
  combined_entry_type *internal_end;
  size_t symesz;
  char *raw_src;
  char *raw_end;
  const char *string_table = NULL;
  asection * debug_sec = NULL;
  char *debug_sec_data = NULL;
  bfd_size_type size;

  if (obj_raw_syments (abfd) != NULL)
    return obj_raw_syments (abfd);

  if (! _bfd_coff_get_external_symbols (abfd))
    return NULL;

  size = obj_raw_syment_count (abfd) * sizeof (combined_entry_type);
  internal = (combined_entry_type *) bfd_zalloc (abfd, size);
  if (internal == NULL && size != 0)
    return NULL;
  internal_end = internal + obj_raw_syment_count (abfd);
  
  raw_src = (char *) obj_coff_external_syms (abfd);

  /* Mark the end of the symbols.  */
  symesz = bfd_coff_symesz (abfd);
  raw_end = (char *) raw_src + obj_raw_syment_count (abfd) * symesz;

  /* FIXME SOMEDAY.  A string table size of zero is very weird, but
     probably possible.  If one shows up, it will probably kill us.  */

  /* Swap all the raw entries.  */
  for (internal_ptr = internal;
       raw_src < raw_end;
       raw_src += symesz, internal_ptr++)
    {
      unsigned int i;

      bfd_coff_swap_sym_in (abfd, (void *) raw_src,
			    (void *) & internal_ptr->u.syment);
      symbol_ptr = internal_ptr;
      internal_ptr->is_sym = TRUE;

      /* PR 17512: file: 1353-1166-0.004.  */
      if (symbol_ptr->u.syment.n_sclass == C_FILE
	  && symbol_ptr->u.syment.n_numaux > 0
	  && raw_src + symesz + symbol_ptr->u.syment.n_numaux
	  * symesz > raw_end)
	{
	  bfd_release (abfd, internal);
	  return NULL;
	}

      for (i = 0;
	   i < symbol_ptr->u.syment.n_numaux;
	   i++)
	{
	  internal_ptr++;
	  /* PR 17512: Prevent buffer overrun.  */
	  if (internal_ptr >= internal_end)
	    {
	      bfd_release (abfd, internal);
	      return NULL;
	    }

	  raw_src += symesz;
	  bfd_coff_swap_aux_in (abfd, (void *) raw_src,
				symbol_ptr->u.syment.n_type,
				symbol_ptr->u.syment.n_sclass,
				(int) i, symbol_ptr->u.syment.n_numaux,
				&(internal_ptr->u.auxent));

	  internal_ptr->is_sym = FALSE;
	  coff_pointerize_aux (abfd, internal, symbol_ptr, i,
			       internal_ptr);
	}
    }

  /* Free the raw symbols, but not the strings (if we have them).  */
  obj_coff_keep_strings (abfd) = TRUE;
  if (! _bfd_coff_free_symbols (abfd))
    return NULL;

  for (internal_ptr = internal; internal_ptr < internal_end;
       internal_ptr++)
    {
      BFD_ASSERT (internal_ptr->is_sym);

      if (internal_ptr->u.syment.n_sclass == C_FILE
	  && internal_ptr->u.syment.n_numaux > 0)
	{
	  combined_entry_type * aux = internal_ptr + 1;

	  /* Make a file symbol point to the name in the auxent, since
	     the text ".file" is redundant.  */
	  BFD_ASSERT (! aux->is_sym);

	  if (aux->u.auxent.x_file.x_n.x_zeroes == 0)
	    {
	      /* The filename is a long one, point into the string table.  */
	      if (string_table == NULL)
		{
		  string_table = _bfd_coff_read_string_table (abfd);
		  if (string_table == NULL)
		    return NULL;
		}

	      if ((bfd_size_type)(aux->u.auxent.x_file.x_n.x_offset)
		  >= obj_coff_strings_len (abfd))
		internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) _("<corrupt>");
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  (bfd_hostptr_t) (string_table + (aux->u.auxent.x_file.x_n.x_offset));
	    }
	  else
	    {
	      /* Ordinary short filename, put into memory anyway.  The
                 Microsoft PE tools sometimes store a filename in
                 multiple AUX entries.  */
	      if (internal_ptr->u.syment.n_numaux > 1
		  && coff_data (abfd)->pe)
		internal_ptr->u.syment._n._n_n._n_offset =
		  (bfd_hostptr_t)
		  copy_name (abfd,
			     aux->u.auxent.x_file.x_fname,
			     internal_ptr->u.syment.n_numaux * symesz);
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  ((bfd_hostptr_t)
		   copy_name (abfd,
			      aux->u.auxent.x_file.x_fname,
			      (size_t) bfd_coff_filnmlen (abfd)));
	    }
	}
      else
	{
	  if (internal_ptr->u.syment._n._n_n._n_zeroes != 0)
	    {
	      /* This is a "short" name.  Make it long.  */
	      size_t i;
	      char *newstring;

	      /* Find the length of this string without walking into memory
	         that isn't ours.  */
	      for (i = 0; i < 8; ++i)
		if (internal_ptr->u.syment._n._n_name[i] == '\0')
		  break;

	      newstring = (char *) bfd_zalloc (abfd, (bfd_size_type) (i + 1));
	      if (newstring == NULL)
		return NULL;
	      strncpy (newstring, internal_ptr->u.syment._n._n_name, i);
	      internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) newstring;
	      internal_ptr->u.syment._n._n_n._n_zeroes = 0;
	    }
	  else if (internal_ptr->u.syment._n._n_n._n_offset == 0)
	    internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) "";
	  else if (!bfd_coff_symname_in_debug (abfd, &internal_ptr->u.syment))
	    {
	      /* Long name already.  Point symbol at the string in the
                 table.  */
	      if (string_table == NULL)
		{
		  string_table = _bfd_coff_read_string_table (abfd);
		  if (string_table == NULL)
		    return NULL;
		}
	      if (internal_ptr->u.syment._n._n_n._n_offset >= obj_coff_strings_len (abfd)
		  || string_table + internal_ptr->u.syment._n._n_n._n_offset < string_table)
		internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) _("<corrupt>");
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  ((bfd_hostptr_t)
		   (string_table
		    + internal_ptr->u.syment._n._n_n._n_offset));
	    }
	  else
	    {
	      /* Long name in debug section.  Very similar.  */
	      if (debug_sec_data == NULL)
		debug_sec_data = build_debug_section (abfd, & debug_sec);
	      if (debug_sec_data != NULL)
		{
		  BFD_ASSERT (debug_sec != NULL);
		  /* PR binutils/17512: Catch out of range offsets into the debug data.  */
		  if (internal_ptr->u.syment._n._n_n._n_offset > debug_sec->size
		      || debug_sec_data + internal_ptr->u.syment._n._n_n._n_offset < debug_sec_data)
		    internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) _("<corrupt>");
		  else
		    internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t)
		      (debug_sec_data + internal_ptr->u.syment._n._n_n._n_offset);
		}
	      else
		internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) "";
	    }
	}
      internal_ptr += internal_ptr->u.syment.n_numaux;
    }

  obj_raw_syments (abfd) = internal;
  BFD_ASSERT (obj_raw_syment_count (abfd)
	      == (unsigned int) (internal_ptr - internal));

  return internal;
}

long
coff_get_reloc_upper_bound (bfd *abfd, sec_ptr asect)
{
  if (bfd_get_format (abfd) != bfd_object)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }
  return (asect->reloc_count + 1) * sizeof (arelent *);
}

asymbol *
coff_make_empty_symbol (bfd *abfd)
{
  bfd_size_type amt = sizeof (coff_symbol_type);
  coff_symbol_type *new_symbol = (coff_symbol_type *) bfd_zalloc (abfd, amt);

  if (new_symbol == NULL)
    return NULL;
  new_symbol->symbol.section = 0;
  new_symbol->native = NULL;
  new_symbol->lineno = NULL;
  new_symbol->done_lineno = FALSE;
  new_symbol->symbol.the_bfd = abfd;

  return & new_symbol->symbol;
}

/* Make a debugging symbol.  */

asymbol *
coff_bfd_make_debug_symbol (bfd *abfd,
			    void * ptr ATTRIBUTE_UNUSED,
			    unsigned long sz ATTRIBUTE_UNUSED)
{
  bfd_size_type amt = sizeof (coff_symbol_type);
  coff_symbol_type *new_symbol = (coff_symbol_type *) bfd_alloc (abfd, amt);

  if (new_symbol == NULL)
    return NULL;
  /* @@ The 10 is a guess at a plausible maximum number of aux entries
     (but shouldn't be a constant).  */
  amt = sizeof (combined_entry_type) * 10;
  new_symbol->native = (combined_entry_type *) bfd_zalloc (abfd, amt);
  if (!new_symbol->native)
    return NULL;
  new_symbol->native->is_sym = TRUE;
  new_symbol->symbol.section = bfd_abs_section_ptr;
  new_symbol->symbol.flags = BSF_DEBUGGING;
  new_symbol->lineno = NULL;
  new_symbol->done_lineno = FALSE;
  new_symbol->symbol.the_bfd = abfd;

  return & new_symbol->symbol;
}

void
coff_get_symbol_info (bfd *abfd, asymbol *symbol, symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);

  if (coffsymbol (symbol)->native != NULL
      && coffsymbol (symbol)->native->fix_value
      && coffsymbol (symbol)->native->is_sym)
    ret->value = coffsymbol (symbol)->native->u.syment.n_value -
      (bfd_hostptr_t) obj_raw_syments (abfd);
}

/* Return the COFF syment for a symbol.  */

bfd_boolean
bfd_coff_get_syment (bfd *abfd,
		     asymbol *symbol,
		     struct internal_syment *psyment)
{
  coff_symbol_type *csym;

  csym = coff_symbol_from (abfd, symbol);
  if (csym == NULL || csym->native == NULL
      || ! csym->native->is_sym)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  *psyment = csym->native->u.syment;

  if (csym->native->fix_value)
    psyment->n_value = psyment->n_value -
      (bfd_hostptr_t) obj_raw_syments (abfd);

  /* FIXME: We should handle fix_line here.  */

  return TRUE;
}

/* Return the COFF auxent for a symbol.  */

bfd_boolean
bfd_coff_get_auxent (bfd *abfd,
		     asymbol *symbol,
		     int indx,
		     union internal_auxent *pauxent)
{
  coff_symbol_type *csym;
  combined_entry_type *ent;

  csym = coff_symbol_from (abfd, symbol);

  if (csym == NULL
      || csym->native == NULL
      || ! csym->native->is_sym
      || indx >= csym->native->u.syment.n_numaux)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  ent = csym->native + indx + 1;

  BFD_ASSERT (! ent->is_sym);
  *pauxent = ent->u.auxent;

  if (ent->fix_tag)
    pauxent->x_sym.x_tagndx.l =
      ((combined_entry_type *) pauxent->x_sym.x_tagndx.p
       - obj_raw_syments (abfd));

  if (ent->fix_end)
    pauxent->x_sym.x_fcnary.x_fcn.x_endndx.l =
      ((combined_entry_type *) pauxent->x_sym.x_fcnary.x_fcn.x_endndx.p
       - obj_raw_syments (abfd));

  if (ent->fix_scnlen)
    pauxent->x_csect.x_scnlen.l =
      ((combined_entry_type *) pauxent->x_csect.x_scnlen.p
       - obj_raw_syments (abfd));

  return TRUE;
}

/* Print out information about COFF symbol.  */

void
coff_print_symbol (bfd *abfd,
		   void * filep,
		   asymbol *symbol,
		   bfd_print_symbol_type how)
{
  FILE * file = (FILE *) filep;

  switch (how)
    {
    case bfd_print_symbol_name:
      fprintf (file, "%s", symbol->name);
      break;

    case bfd_print_symbol_more:
      fprintf (file, "coff %s %s",
	       coffsymbol (symbol)->native ? "n" : "g",
	       coffsymbol (symbol)->lineno ? "l" : " ");
      break;

    case bfd_print_symbol_all:
      if (coffsymbol (symbol)->native)
	{
	  bfd_vma val;
	  unsigned int aux;
	  combined_entry_type *combined = coffsymbol (symbol)->native;
	  combined_entry_type *root = obj_raw_syments (abfd);
	  struct lineno_cache_entry *l = coffsymbol (symbol)->lineno;

	  fprintf (file, "[%3ld]", (long) (combined - root));

	  /* PR 17512: file: 079-33786-0.001:0.1.  */
	  if (combined < obj_raw_syments (abfd)
	      || combined >= obj_raw_syments (abfd) + obj_raw_syment_count (abfd))
	    {
	      fprintf (file, _("<corrupt info> %s"), symbol->name);
	      break;
	    }

	  BFD_ASSERT (combined->is_sym);
	  if (! combined->fix_value)
	    val = (bfd_vma) combined->u.syment.n_value;
	  else
	    val = combined->u.syment.n_value - (bfd_hostptr_t) root;

	  fprintf (file, "(sec %2d)(fl 0x%02x)(ty %3x)(scl %3d) (nx %d) 0x",
		   combined->u.syment.n_scnum,
		   combined->u.syment.n_flags,
		   combined->u.syment.n_type,
		   combined->u.syment.n_sclass,
		   combined->u.syment.n_numaux);
	  bfd_fprintf_vma (abfd, file, val);
	  fprintf (file, " %s", symbol->name);

	  for (aux = 0; aux < combined->u.syment.n_numaux; aux++)
	    {
	      combined_entry_type *auxp = combined + aux + 1;
	      long tagndx;

	      BFD_ASSERT (! auxp->is_sym);
	      if (auxp->fix_tag)
		tagndx = auxp->u.auxent.x_sym.x_tagndx.p - root;
	      else
		tagndx = auxp->u.auxent.x_sym.x_tagndx.l;

	      fprintf (file, "\n");

	      if (bfd_coff_print_aux (abfd, file, root, combined, auxp, aux))
		continue;

	      switch (combined->u.syment.n_sclass)
		{
		case C_FILE:
		  fprintf (file, "File ");
		  break;

		case C_STAT:
		  if (combined->u.syment.n_type == T_NULL)
		    /* Probably a section symbol ?  */
		    {
		      fprintf (file, "AUX scnlen 0x%lx nreloc %d nlnno %d",
			       (unsigned long) auxp->u.auxent.x_scn.x_scnlen,
			       auxp->u.auxent.x_scn.x_nreloc,
			       auxp->u.auxent.x_scn.x_nlinno);
		      if (auxp->u.auxent.x_scn.x_checksum != 0
			  || auxp->u.auxent.x_scn.x_associated != 0
			  || auxp->u.auxent.x_scn.x_comdat != 0)
			fprintf (file, " checksum 0x%lx assoc %d comdat %d",
				 auxp->u.auxent.x_scn.x_checksum,
				 auxp->u.auxent.x_scn.x_associated,
				 auxp->u.auxent.x_scn.x_comdat);
		      break;
		    }
		    /* Otherwise fall through.  */
		case C_EXT:
		case C_AIX_WEAKEXT:
		  if (ISFCN (combined->u.syment.n_type))
		    {
		      long next, llnos;

		      if (auxp->fix_end)
			next = (auxp->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.p
			       - root);
		      else
			next = auxp->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l;
		      llnos = auxp->u.auxent.x_sym.x_fcnary.x_fcn.x_lnnoptr;
		      fprintf (file,
			       "AUX tagndx %ld ttlsiz 0x%lx lnnos %ld next %ld",
			       tagndx,
			       (unsigned long) auxp->u.auxent.x_sym.x_misc.x_fsize,
			       llnos, next);
		      break;
		    }
		  /* Otherwise fall through.  */
		default:
		  fprintf (file, "AUX lnno %d size 0x%x tagndx %ld",
			   auxp->u.auxent.x_sym.x_misc.x_lnsz.x_lnno,
			   auxp->u.auxent.x_sym.x_misc.x_lnsz.x_size,
			   tagndx);
		  if (auxp->fix_end)
		    fprintf (file, " endndx %ld",
			     ((long)
			      (auxp->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.p
			       - root)));
		  break;
		}
	    }

	  if (l)
	    {
	      fprintf (file, "\n%s :", l->u.sym->name);
	      l++;
	      while (l->line_number)
		{
		  if (l->line_number > 0)
		    {
		      fprintf (file, "\n%4d : ", l->line_number);
		      bfd_fprintf_vma (abfd, file, l->u.offset + symbol->section->vma);
		    }
		  l++;
		}
	    }
	}
      else
	{
	  bfd_print_symbol_vandf (abfd, (void *) file, symbol);
	  fprintf (file, " %-5s %s %s %s",
		   symbol->section->name,
		   coffsymbol (symbol)->native ? "n" : "g",
		   coffsymbol (symbol)->lineno ? "l" : " ",
		   symbol->name);
	}
    }
}

/* Return whether a symbol name implies a local symbol.  In COFF,
   local symbols generally start with ``.L''.  Most targets use this
   function for the is_local_label_name entry point, but some may
   override it.  */

bfd_boolean
_bfd_coff_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
			       const char *name)
{
  return name[0] == '.' && name[1] == 'L';
}

/* Provided a BFD, a section and an offset (in bytes, not octets) into the
   section, calculate and return the name of the source file and the line
   nearest to the wanted location.  */

bfd_boolean
coff_find_nearest_line_with_names (bfd *abfd,
                                   asymbol **symbols,
                                   asection *section,
                                   bfd_vma offset,
                                   const char **filename_ptr,
                                   const char **functionname_ptr,
                                   unsigned int *line_ptr,
                                   const struct dwarf_debug_section *debug_sections)
{
  bfd_boolean found;
  unsigned int i;
  unsigned int line_base;
  coff_data_type *cof = coff_data (abfd);
  /* Run through the raw syments if available.  */
  combined_entry_type *p;
  combined_entry_type *pend;
  alent *l;
  struct coff_section_tdata *sec_data;
  bfd_size_type amt;

  /* Before looking through the symbol table, try to use a .stab
     section to find the information.  */
  if (! _bfd_stab_section_find_nearest_line (abfd, symbols, section, offset,
					     &found, filename_ptr,
					     functionname_ptr, line_ptr,
					     &coff_data(abfd)->line_info))
    return FALSE;

  if (found)
    return TRUE;

  /* Also try examining DWARF2 debugging information.  */
  if (_bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr, NULL, debug_sections, 0,
				     &coff_data(abfd)->dwarf2_find_line_info))
    return TRUE;

  *filename_ptr = 0;
  *functionname_ptr = 0;
  *line_ptr = 0;

  /* Don't try and find line numbers in a non coff file.  */
  if (!bfd_family_coff (abfd))
    return FALSE;

  if (cof == NULL)
    return FALSE;

  /* Find the first C_FILE symbol.  */
  p = cof->raw_syments;
  if (!p)
    return FALSE;

  pend = p + cof->raw_syment_count;
  while (p < pend)
    {
      BFD_ASSERT (p->is_sym);
      if (p->u.syment.n_sclass == C_FILE)
	break;
      p += 1 + p->u.syment.n_numaux;
    }

  if (p < pend)
    {
      bfd_vma sec_vma;
      bfd_vma maxdiff;

      /* Look through the C_FILE symbols to find the best one.  */
      sec_vma = bfd_get_section_vma (abfd, section);
      *filename_ptr = (char *) p->u.syment._n._n_n._n_offset;
      maxdiff = (bfd_vma) 0 - (bfd_vma) 1;
      while (1)
	{
	  bfd_vma file_addr;
	  combined_entry_type *p2;

	  for (p2 = p + 1 + p->u.syment.n_numaux;
	       p2 < pend;
	       p2 += 1 + p2->u.syment.n_numaux)
	    {
	      BFD_ASSERT (p2->is_sym);
	      if (p2->u.syment.n_scnum > 0
		  && (section
		      == coff_section_from_bfd_index (abfd,
						      p2->u.syment.n_scnum)))
		break;
	      if (p2->u.syment.n_sclass == C_FILE)
		{
		  p2 = pend;
		  break;
		}
	    }
	  if (p2 >= pend)
	    break;

	  file_addr = (bfd_vma) p2->u.syment.n_value;
	  /* PR 11512: Include the section address of the function name symbol.  */
	  if (p2->u.syment.n_scnum > 0)
	    file_addr += coff_section_from_bfd_index (abfd,
						      p2->u.syment.n_scnum)->vma;
	  /* We use <= MAXDIFF here so that if we get a zero length
             file, we actually use the next file entry.  */
	  if (p2 < pend
	      && offset + sec_vma >= file_addr
	      && offset + sec_vma - file_addr <= maxdiff)
	    {
	      *filename_ptr = (char *) p->u.syment._n._n_n._n_offset;
	      maxdiff = offset + sec_vma - p2->u.syment.n_value;
	    }

	  /* Avoid endless loops on erroneous files by ensuring that
	     we always move forward in the file.  */
	  if (p >= cof->raw_syments + p->u.syment.n_value)
	    break;

	  p = cof->raw_syments + p->u.syment.n_value;
	  if (p > pend || p->u.syment.n_sclass != C_FILE)
	    break;
	}
    }

  /* Now wander though the raw linenumbers of the section.  */
  /* If we have been called on this section before, and the offset we
     want is further down then we can prime the lookup loop.  */
  sec_data = coff_section_data (abfd, section);
  if (sec_data != NULL
      && sec_data->i > 0
      && offset >= sec_data->offset)
    {
      i = sec_data->i;
      *functionname_ptr = sec_data->function;
      line_base = sec_data->line_base;
    }
  else
    {
      i = 0;
      line_base = 0;
    }

  if (section->lineno != NULL)
    {
      bfd_vma last_value = 0;

      l = &section->lineno[i];

      for (; i < section->lineno_count; i++)
	{
	  if (l->line_number == 0)
	    {
	      /* Get the symbol this line number points at.  */
	      coff_symbol_type *coff = (coff_symbol_type *) (l->u.sym);
	      if (coff->symbol.value > offset)
		break;
	      *functionname_ptr = coff->symbol.name;
	      last_value = coff->symbol.value;
	      if (coff->native)
		{
		  combined_entry_type *s = coff->native;

		  BFD_ASSERT (s->is_sym);
		  s = s + 1 + s->u.syment.n_numaux;

		  /* In XCOFF a debugging symbol can follow the
		     function symbol.  */
		  if (s->u.syment.n_scnum == N_DEBUG)
		    s = s + 1 + s->u.syment.n_numaux;

		  /* S should now point to the .bf of the function.  */
		  if (s->u.syment.n_numaux)
		    {
		      /* The linenumber is stored in the auxent.  */
		      union internal_auxent *a = &((s + 1)->u.auxent);

		      line_base = a->x_sym.x_misc.x_lnsz.x_lnno;
		      *line_ptr = line_base;
		    }
		}
	    }
	  else
	    {
	      if (l->u.offset > offset)
		break;
	      *line_ptr = l->line_number + line_base - 1;
	    }
	  l++;
	}

      /* If we fell off the end of the loop, then assume that this
	 symbol has no line number info.  Otherwise, symbols with no
	 line number info get reported with the line number of the
	 last line of the last symbol which does have line number
	 info.  We use 0x100 as a slop to account for cases where the
	 last line has executable code.  */
      if (i >= section->lineno_count
	  && last_value != 0
	  && offset - last_value > 0x100)
	{
	  *functionname_ptr = NULL;
	  *line_ptr = 0;
	}
    }

  /* Cache the results for the next call.  */
  if (sec_data == NULL && section->owner == abfd)
    {
      amt = sizeof (struct coff_section_tdata);
      section->used_by_bfd = bfd_zalloc (abfd, amt);
      sec_data = (struct coff_section_tdata *) section->used_by_bfd;
    }
  if (sec_data != NULL)
    {
      sec_data->offset = offset;
      sec_data->i = i - 1;
      sec_data->function = *functionname_ptr;
      sec_data->line_base = line_base;
    }

  return TRUE;
}

bfd_boolean
coff_find_nearest_line (bfd *abfd,
			asymbol **symbols,
			asection *section,
			bfd_vma offset,
			const char **filename_ptr,
			const char **functionname_ptr,
			unsigned int *line_ptr,
			unsigned int *discriminator_ptr)
{
  if (discriminator_ptr)
    *discriminator_ptr = 0;
  return coff_find_nearest_line_with_names (abfd, symbols, section, offset,
                                            filename_ptr, functionname_ptr,
                                            line_ptr, dwarf_debug_sections);
}

bfd_boolean
coff_find_inliner_info (bfd *abfd,
			const char **filename_ptr,
			const char **functionname_ptr,
			unsigned int *line_ptr)
{
  bfd_boolean found;

  found = _bfd_dwarf2_find_inliner_info (abfd, filename_ptr,
					 functionname_ptr, line_ptr,
					 &coff_data(abfd)->dwarf2_find_line_info);
  return (found);
}

int
coff_sizeof_headers (bfd *abfd, struct bfd_link_info *info)
{
  size_t size;

  if (!info->relocatable)
    size = bfd_coff_filhsz (abfd) + bfd_coff_aoutsz (abfd);
  else
    size = bfd_coff_filhsz (abfd);

  size += abfd->section_count * bfd_coff_scnhsz (abfd);
  return size;
}

/* Change the class of a coff symbol held by BFD.  */

bfd_boolean
bfd_coff_set_symbol_class (bfd *         abfd,
			   asymbol *     symbol,
			   unsigned int  symbol_class)
{
  coff_symbol_type * csym;

  csym = coff_symbol_from (abfd, symbol);
  if (csym == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }
  else if (csym->native == NULL)
    {
      /* This is an alien symbol which no native coff backend data.
	 We cheat here by creating a fake native entry for it and
	 then filling in the class.  This code is based on that in
	 coff_write_alien_symbol().  */

      combined_entry_type * native;
      bfd_size_type amt = sizeof (* native);

      native = (combined_entry_type *) bfd_zalloc (abfd, amt);
      if (native == NULL)
	return FALSE;

      native->is_sym = TRUE;
      native->u.syment.n_type   = T_NULL;
      native->u.syment.n_sclass = symbol_class;

      if (bfd_is_und_section (symbol->section))
	{
	  native->u.syment.n_scnum = N_UNDEF;
	  native->u.syment.n_value = symbol->value;
	}
      else if (bfd_is_com_section (symbol->section))
	{
	  native->u.syment.n_scnum = N_UNDEF;
	  native->u.syment.n_value = symbol->value;
	}
      else
	{
	  native->u.syment.n_scnum =
	    symbol->section->output_section->target_index;
	  native->u.syment.n_value = (symbol->value
				      + symbol->section->output_offset);
	  if (! obj_pe (abfd))
	    native->u.syment.n_value += symbol->section->output_section->vma;

	  /* Copy the any flags from the file header into the symbol.
	     FIXME: Why?  */
	  native->u.syment.n_flags = bfd_asymbol_bfd (& csym->symbol)->flags;
	}

      csym->native = native;
    }
  else
    csym->native->u.syment.n_sclass = symbol_class;

  return TRUE;
}

struct coff_comdat_info *
bfd_coff_get_comdat_section (bfd *abfd, struct bfd_section *sec)
{
  if (bfd_get_flavour (abfd) == bfd_target_coff_flavour
      && coff_section_data (abfd, sec) != NULL)
    return coff_section_data (abfd, sec)->comdat;
  else
    return NULL;
}

bfd_boolean
_bfd_coff_section_already_linked (bfd *abfd,
				  asection *sec,
				  struct bfd_link_info *info)
{
  flagword flags;
  const char *name, *key;
  struct bfd_section_already_linked *l;
  struct bfd_section_already_linked_hash_entry *already_linked_list;
  struct coff_comdat_info *s_comdat;

  flags = sec->flags;
  if ((flags & SEC_LINK_ONCE) == 0)
    return FALSE;

  /* The COFF backend linker doesn't support group sections.  */
  if ((flags & SEC_GROUP) != 0)
    return FALSE;

  name = bfd_get_section_name (abfd, sec);
  s_comdat = bfd_coff_get_comdat_section (abfd, sec);

  if (s_comdat != NULL)
    key = s_comdat->name;
  else
    {
      if (CONST_STRNEQ (name, ".gnu.linkonce.")
	  && (key = strchr (name + sizeof (".gnu.linkonce.") - 1, '.')) != NULL)
	key++;
      else
	/* FIXME: gcc as of 2011-09 emits sections like .text$<key>,
	   .xdata$<key> and .pdata$<key> only the first of which has a
	   comdat key.  Should these all match the LTO IR key?  */
	key = name;
    }

  already_linked_list = bfd_section_already_linked_table_lookup (key);

  for (l = already_linked_list->entry; l != NULL; l = l->next)
    {
      struct coff_comdat_info *l_comdat;

      l_comdat = bfd_coff_get_comdat_section (l->sec->owner, l->sec);

      /* The section names must match, and both sections must be
	 comdat and have the same comdat name, or both sections must
	 be non-comdat.  LTO IR plugin sections are an exception.  They
	 are always named .gnu.linkonce.t.<key> (<key> is some string)
	 and match any comdat section with comdat name of <key>, and
	 any linkonce section with the same suffix, ie.
	 .gnu.linkonce.*.<key>.  */
      if (((s_comdat != NULL) == (l_comdat != NULL)
	   && strcmp (name, l->sec->name) == 0)
	  || (l->sec->owner->flags & BFD_PLUGIN) != 0)
	{
	  /* The section has already been linked.  See if we should
	     issue a warning.  */
	  return _bfd_handle_already_linked (sec, l, info);
	}
    }

  /* This is the first section with this name.  Record it.  */
  if (!bfd_section_already_linked_table_insert (already_linked_list, sec))
    info->callbacks->einfo (_("%F%P: already_linked_table: %E\n"));
  return FALSE;
}

/* BFD back-end for Apple M68K COFF A/UX 3.x files.
   Copyright (C) 1996-2014 Free Software Foundation, Inc.
   Written by Richard Henderson <rth@tamu.edu>.

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

#define TARGET_SYM	m68k_coff_aux_vec
#define TARGET_NAME	"coff-m68k-aux"

#ifndef TARG_AUX
#define TARG_AUX
#endif

#define COFF_LONG_FILENAMES

/* 4k pages */
#define COFF_PAGE_SIZE 0x1000

/* On AUX, a STYP_NOLOAD|STYP_BSS section is part of a shared library.  */
#define BSS_NOLOAD_IS_SHARED_LIBRARY

#define STATIC_RELOCS

#define COFF_COMMON_ADDEND

#include "sysdep.h"
#include "bfd.h"

#define coff_link_add_one_symbol coff_m68k_aux_link_add_one_symbol
static bfd_boolean
coff_m68k_aux_link_add_one_symbol
  (struct bfd_link_info *, bfd *, const char *, flagword, asection *,
   bfd_vma, const char *, bfd_boolean, bfd_boolean,
   struct bfd_link_hash_entry **);

#ifndef bfd_pe_print_pdata
#define bfd_pe_print_pdata	NULL
#endif

#include "coff/aux-coff.h"  /* override coff/internal.h and coff/m68k.h */
#include "coff-m68k.c"

/* We need non-absolute symbols to override absolute symbols.  This
   mirrors Apple's "solution" to let a static library symbol override
   a shared library symbol.  On the whole not a good thing, given how
   shared libraries work here, but can work if you are careful with
   what you include in the shared object.  */

static bfd_boolean
coff_m68k_aux_link_add_one_symbol (struct bfd_link_info *info,
				   bfd *abfd,
				   const char *name,
				   flagword flags,
				   asection *section,
				   bfd_vma value,
				   const char *string,
				   bfd_boolean copy,
				   bfd_boolean collect,
				   struct bfd_link_hash_entry **hashp)
{
  struct bfd_link_hash_entry *h, *inh, *t;

  if ((flags & (BSF_WARNING | BSF_CONSTRUCTOR | BSF_WEAK)) == 0
      && !bfd_is_und_section (section)
      && !bfd_is_com_section (section))
    {
      /* The new symbol is a definition or an indirect definition */

      /* This bit copied from linker.c */
      if (hashp != NULL && *hashp != NULL)
	h = *hashp;
      else
	{
	  h = bfd_link_hash_lookup (info->hash, name, TRUE, copy, FALSE);
	  if (h == NULL)
	    {
	      if (hashp != NULL)
		*hashp = NULL;
	      return FALSE;
	    }
	}

      if (hashp != (struct bfd_link_hash_entry **) NULL)
	*hashp = h;
      /* end duplication from linker.c */

      t = h;
      inh = NULL;
      if (h->type == bfd_link_hash_indirect)
	{
	  inh = h->u.i.link;
	  t = inh;
	}

      if (t->type == bfd_link_hash_defined)
	{
	  asection *msec = t->u.def.section;
	  bfd_boolean special = FALSE;

	  if (bfd_is_abs_section (msec) && !bfd_is_abs_section (section))
	    {
	      t->u.def.section = section;
	      t->u.def.value = value;
	      special = TRUE;
	    }
	  else if (bfd_is_abs_section (section) && !bfd_is_abs_section (msec))
	    special = TRUE;

	  if (special)
	    {
	      if (info->notice_all
		  || (info->notice_hash != NULL
		      && bfd_hash_lookup (info->notice_hash, name,
					  FALSE, FALSE) != NULL))
		{
		  if (!(*info->callbacks->notice) (info, h, inh,
						   abfd, section, value, flags))
		    return FALSE;
		}

	      return TRUE;
	    }
	}
    }

  /* If we didn't exit early, finish processing in the generic routine */
  return _bfd_generic_link_add_one_symbol (info, abfd, name, flags, section,
					   value, string, copy, collect,
					   hashp);
}

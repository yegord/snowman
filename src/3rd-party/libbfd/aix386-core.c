/* BFD back-end for AIX on PS/2 core files.
   This was based on trad-core.c, which was written by John Gilmore of
        Cygnus Support.
   Copyright (C) 1988-2014 Free Software Foundation, Inc.
   Written by Minh Tran-Le <TRANLE@INTELLICORP.COM>.
   Converted to back end form by Ian Lance Taylor <ian@cygnus.com>.

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
#include "libbfd.h"
#include "coff/i386.h"
#include "coff/internal.h"
#include "libcoff.h"

#include <signal.h>

#if defined (_AIX) && defined (_I386)
#define NOCHECKS		/* This is for coredump.h.  */
#define _h_USER			/* Avoid including user.h from coredump.h.  */
#include <uinfo.h>
#include <sys/i386/coredump.h>
#endif /* _AIX && _I386 */

/* Maybe this could work on some other i386 but I have not tried it
 * mtranle@paris - Tue Sep 24 12:49:35 1991
 */

#ifndef COR_MAGIC
# define COR_MAGIC "core"
#endif

/* Need this cast because ptr is really void *.  */
#define core_hdr(bfd) \
    (((bfd->tdata.trad_core_data))->hdr)
#define core_section(bfd,n) \
    (((bfd)->tdata.trad_core_data)->sections[n])
#define core_regsec(bfd) \
    (((bfd)->tdata.trad_core_data)->reg_section)
#define core_reg2sec(bfd) \
    (((bfd)->tdata.trad_core_data)->reg2_section)

/* These are stored in the bfd's tdata.  */
struct trad_core_struct
{
  struct corehdr *hdr;		/* core file header */
  asection *reg_section;
  asection *reg2_section;
  asection *sections[MAX_CORE_SEGS];
};

static const bfd_target *
aix386_core_file_p (bfd *abfd)
{
  int i, n;
  unsigned char longbuf[4];	/* Raw bytes of various header fields */
  bfd_size_type core_size = sizeof (struct corehdr);
  bfd_size_type amt;
  struct corehdr *core;
  struct mergem
  {
    struct trad_core_struct coredata;
    struct corehdr internal_core;
  } *mergem;
  flagword flags;

  amt = sizeof (longbuf);
  if (bfd_bread (longbuf, amt, abfd) != amt)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return 0;
    }

  if (strncmp (longbuf, COR_MAGIC, 4))
    return 0;

  if (bfd_seek (abfd, (file_ptr) 0, 0) != 0)
    return 0;

  amt = sizeof (struct mergem);
  mergem = (struct mergem *) bfd_zalloc (abfd, amt);
  if (mergem == NULL)
    return 0;

  core = &mergem->internal_core;

  if ((bfd_bread (core, core_size, abfd)) != core_size)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
    loser:
      bfd_release (abfd, (char *) mergem);
      abfd->tdata.any = NULL;
      bfd_section_list_clear (abfd);
      return 0;
    }

  set_tdata (abfd, &mergem->coredata);
  core_hdr (abfd) = core;

  /* Create the sections.  */
  flags = SEC_HAS_CONTENTS;
  core_regsec (abfd) = bfd_make_section_anyway_with_flags (abfd, ".reg",
							   flags);
  if (core_regsec (abfd) == NULL)
    goto loser;

  core_regsec (abfd)->size = sizeof (core->cd_regs);
  core_regsec (abfd)->vma = (bfd_vma) -1;

  /* We'll access the regs afresh in the core file, like any section.  */
  core_regsec (abfd)->filepos =
    (file_ptr) offsetof (struct corehdr, cd_regs[0]);

  flags = SEC_HAS_CONTENTS;
  core_reg2sec (abfd) = bfd_make_section_anyway_with_flags (abfd, ".reg2",
							    flags);
  if (core_reg2sec (abfd) == NULL)
    /* bfd_release frees everything allocated after it's arg.  */
    goto loser;

  core_reg2sec (abfd)->size = sizeof (core->cd_fpregs);
  core_reg2sec (abfd)->vma = (bfd_vma) -1;
  core_reg2sec (abfd)->filepos =
    (file_ptr) offsetof (struct corehdr, cd_fpregs);

  for (i = 0, n = 0; (i < MAX_CORE_SEGS) && (core->cd_segs[i].cs_type); i++)
    {
      const char *sname;
      flagword flags;

      if (core->cd_segs[i].cs_offset == 0)
	continue;

      switch (core->cd_segs[i].cs_type)
	{
	case COR_TYPE_DATA:
	  sname = ".data";
	  flags = SEC_ALLOC + SEC_LOAD + SEC_HAS_CONTENTS;
	  break;
	case COR_TYPE_STACK:
	  sname = ".stack";
	  flags = SEC_ALLOC + SEC_LOAD + SEC_HAS_CONTENTS;
	  break;
	case COR_TYPE_LIBDATA:
	  sname = ".libdata";
	  flags = SEC_ALLOC + SEC_HAS_CONTENTS;
	  break;
	case COR_TYPE_WRITE:
	  sname = ".writeable";
	  flags = SEC_ALLOC + SEC_HAS_CONTENTS;
	  break;
	case COR_TYPE_MSC:
	  sname = ".misc";
	  flags = SEC_ALLOC + SEC_HAS_CONTENTS;
	  break;
	default:
	  sname = ".unknown";
	  flags = SEC_ALLOC + SEC_HAS_CONTENTS;
	  break;
	}
      core_section (abfd, n) = bfd_make_section_anyway_with_flags (abfd,
								   sname,
								   flags);
      if (core_section (abfd, n) == NULL)
	goto loser;

      core_section (abfd, n)->size = core->cd_segs[i].cs_len;
      core_section (abfd, n)->vma       = core->cd_segs[i].cs_address;
      core_section (abfd, n)->filepos   = core->cd_segs[i].cs_offset;
      core_section (abfd, n)->alignment_power = 2;
      n++;
    }

  return abfd->xvec;
}

static char *
aix386_core_file_failing_command (bfd *abfd)
{
  return core_hdr (abfd)->cd_comm;
}

static int
aix386_core_file_failing_signal (bfd *abfd)
{
  return core_hdr (abfd)->cd_cursig;
}

#define aix386_core_file_matches_executable_p generic_core_file_matches_executable_p

#define aix386_core_file_pid _bfd_nocore_core_file_pid

/* If somebody calls any byte-swapping routines, shoot them.  */

static void
swap_abort (void)
{
  /* This way doesn't require any declaration for ANSI to fuck up.  */
  abort ();
}

#define	NO_GET ((bfd_vma (*) (const void *)) swap_abort)
#define	NO_PUT ((void (*) (bfd_vma, void *)) swap_abort)
#define	NO_GETS ((bfd_signed_vma (*) (const void *)) swap_abort)
#define	NO_GET64 ((bfd_uint64_t (*) (const void *)) swap_abort)
#define	NO_PUT64 ((void (*) (bfd_uint64_t, void *)) swap_abort)
#define	NO_GETS64 ((bfd_int64_t (*) (const void *)) swap_abort)

const bfd_target core_aix386_vec =
{
  "aix386-core",
  bfd_target_unknown_flavour,
  BFD_ENDIAN_BIG,		/* target byte order */
  BFD_ENDIAN_BIG,		/* target headers byte order */
  (HAS_RELOC | EXEC_P |		/* object flags */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | WP_TEXT),

  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* section flags */
  0,				/* leading underscore */
  ' ',				/* ar_pad_char */
  16,				/* ar_max_namelen */
  0,				/* match priority.  */
  NO_GET64, NO_GETS64, NO_PUT64,
  NO_GET, NO_GETS, NO_PUT,
  NO_GET, NO_GETS, NO_PUT,	/* data */
  NO_GET64, NO_GETS64, NO_PUT64,
  NO_GET, NO_GETS, NO_PUT,
  NO_GET, NO_GETS, NO_PUT,	/* hdrs */

  {_bfd_dummy_target, _bfd_dummy_target,
   _bfd_dummy_target, aix386_core_file_p},
  {bfd_false, bfd_false,	/* bfd_create_object */
   bfd_false, bfd_false},
  {bfd_false, bfd_false,	/* bfd_write_contents */
   bfd_false, bfd_false},

  BFD_JUMP_TABLE_GENERIC (_bfd_generic),
  BFD_JUMP_TABLE_COPY (_bfd_generic),
  BFD_JUMP_TABLE_CORE (aix386),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
  BFD_JUMP_TABLE_SYMBOLS (_bfd_nosymbols),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (_bfd_generic),
  BFD_JUMP_TABLE_LINK (_bfd_nolink),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL
};

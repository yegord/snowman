/* BFD back-end for archive files (libraries).
   Copyright (C) 1990-2014 Free Software Foundation, Inc.
   Written by Cygnus Support.  Mostly Gumby Henkel-Wallace's fault.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/*
@setfilename archive-info
SECTION
	Archives

DESCRIPTION
	An archive (or library) is just another BFD.  It has a symbol
	table, although there's not much a user program will do with it.

	The big difference between an archive BFD and an ordinary BFD
	is that the archive doesn't have sections.  Instead it has a
	chain of BFDs that are considered its contents.  These BFDs can
	be manipulated like any other.  The BFDs contained in an
	archive opened for reading will all be opened for reading.  You
	may put either input or output BFDs into an archive opened for
	output; they will be handled correctly when the archive is closed.

	Use <<bfd_openr_next_archived_file>> to step through
	the contents of an archive opened for input.  You don't
	have to read the entire archive if you don't want
	to!  Read it until you find what you want.

	A BFD returned by <<bfd_openr_next_archived_file>> can be
	closed manually with <<bfd_close>>.  If you do not close it,
	then a second iteration through the members of an archive may
	return the same BFD.  If you close the archive BFD, then all
	the member BFDs will automatically be closed as well.

	Archive contents of output BFDs are chained through the
	<<archive_next>> pointer in a BFD.  The first one is findable
	through the <<archive_head>> slot of the archive.  Set it with
	<<bfd_set_archive_head>> (q.v.).  A given BFD may be in only
	one open output archive at a time.

	As expected, the BFD archive code is more general than the
	archive code of any given environment.  BFD archives may
	contain files of different formats (e.g., a.out and coff) and
	even different architectures.  You may even place archives
	recursively into archives!

	This can cause unexpected confusion, since some archive
	formats are more expressive than others.  For instance, Intel
	COFF archives can preserve long filenames; SunOS a.out archives
	cannot.  If you move a file from the first to the second
	format and back again, the filename may be truncated.
	Likewise, different a.out environments have different
	conventions as to how they truncate filenames, whether they
	preserve directory names in filenames, etc.  When
	interoperating with native tools, be sure your files are
	homogeneous.

	Beware: most of these formats do not react well to the
	presence of spaces in filenames.  We do the best we can, but
	can't always handle this case due to restrictions in the format of
	archives.  Many Unix utilities are braindead in regards to
	spaces and such in filenames anyway, so this shouldn't be much
	of a restriction.

	Archives are supported in BFD in <<archive.c>>.

SUBSECTION
	Archive functions
*/

/* Assumes:
   o - all archive elements start on an even boundary, newline padded;
   o - all arch headers are char *;
   o - all arch headers are the same size (across architectures).
*/

/* Some formats provide a way to cram a long filename into the short
   (16 chars) space provided by a BSD archive.  The trick is: make a
   special "file" in the front of the archive, sort of like the SYMDEF
   entry.  If the filename is too long to fit, put it in the extended
   name table, and use its index as the filename.  To prevent
   confusion prepend the index with a space.  This means you can't
   have filenames that start with a space, but then again, many Unix
   utilities can't handle that anyway.

   This scheme unfortunately requires that you stand on your head in
   order to write an archive since you need to put a magic file at the
   front, and need to touch every entry to do so.  C'est la vie.

   We support two variants of this idea:
   The SVR4 format (extended name table is named "//"),
   and an extended pseudo-BSD variant (extended name table is named
   "ARFILENAMES/").  The origin of the latter format is uncertain.

   BSD 4.4 uses a third scheme:  It writes a long filename
   directly after the header.  This allows 'ar q' to work.
*/

/* Summary of archive member names:

 Symbol table (must be first):
 "__.SYMDEF       " - Symbol table, Berkeley style, produced by ranlib.
 "/               " - Symbol table, system 5 style.

 Long name table (must be before regular file members):
 "//              " - Long name table, System 5 R4 style.
 "ARFILENAMES/    " - Long name table, non-standard extended BSD (not BSD 4.4).

 Regular file members with short names:
 "filename.o/     " - Regular file, System 5 style (embedded spaces ok).
 "filename.o      " - Regular file, Berkeley style (no embedded spaces).

 Regular files with long names (or embedded spaces, for BSD variants):
 "/18             " - SVR4 style, name at offset 18 in name table.
 "#1/23           " - Long name (or embedded spaces) 23 characters long,
		      BSD 4.4 style, full name follows header.
 " 18             " - Long name 18 characters long, extended pseudo-BSD.
 */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "libbfd.h"
#include "aout/ar.h"
#include "aout/ranlib.h"
#include "safe-ctype.h"
#include "hashtab.h"
#include "filenames.h"
#include "bfdlink.h"

#ifndef errno
extern int errno;
#endif

/* We keep a cache of archive filepointers to archive elements to
   speed up searching the archive by filepos.  We only add an entry to
   the cache when we actually read one.  We also don't sort the cache;
   it's generally short enough to search linearly.
   Note that the pointers here point to the front of the ar_hdr, not
   to the front of the contents!  */
struct ar_cache
{
  file_ptr ptr;
  bfd *arbfd;
};

#define ar_padchar(abfd) ((abfd)->xvec->ar_pad_char)
#define ar_maxnamelen(abfd) ((abfd)->xvec->ar_max_namelen)

#define arch_eltdata(bfd) ((struct areltdata *) ((bfd)->arelt_data))
#define arch_hdr(bfd) ((struct ar_hdr *) arch_eltdata (bfd)->arch_header)

/* True iff NAME designated a BSD 4.4 extended name.  */

#define is_bsd44_extended_name(NAME) \
  (NAME[0] == '#'  && NAME[1] == '1' && NAME[2] == '/' && ISDIGIT (NAME[3]))

void
_bfd_ar_spacepad (char *p, size_t n, const char *fmt, long val)
{
  static char buf[20];
  size_t len;

  snprintf (buf, sizeof (buf), fmt, val);
  len = strlen (buf);
  if (len < n)
    {
      memcpy (p, buf, len);
      memset (p + len, ' ', n - len);
    }
  else
    memcpy (p, buf, n);
}

bfd_boolean
_bfd_ar_sizepad (char *p, size_t n, bfd_size_type size)
{
  static char buf[21];
  size_t len;

  snprintf (buf, sizeof (buf), "%-10" BFD_VMA_FMT "u", size);
  len = strlen (buf);
  if (len > n)
    {
      bfd_set_error (bfd_error_file_too_big);
      return FALSE;
    }
  if (len < n)
    {
      memcpy (p, buf, len);
      memset (p + len, ' ', n - len);
    }
  else
    memcpy (p, buf, n);
  return TRUE;
}

bfd_boolean
_bfd_generic_mkarchive (bfd *abfd)
{
  bfd_size_type amt = sizeof (struct artdata);

  abfd->tdata.aout_ar_data = (struct artdata *) bfd_zalloc (abfd, amt);
  if (bfd_ardata (abfd) == NULL)
    return FALSE;

  /* Already cleared by bfd_zalloc above.
     bfd_ardata (abfd)->cache = NULL;
     bfd_ardata (abfd)->archive_head = NULL;
     bfd_ardata (abfd)->symdefs = NULL;
     bfd_ardata (abfd)->extended_names = NULL;
     bfd_ardata (abfd)->extended_names_size = 0;
     bfd_ardata (abfd)->tdata = NULL;  */

  return TRUE;
}

/*
FUNCTION
	bfd_get_next_mapent

SYNOPSIS
	symindex bfd_get_next_mapent
	  (bfd *abfd, symindex previous, carsym **sym);

DESCRIPTION
	Step through archive @var{abfd}'s symbol table (if it
	has one).  Successively update @var{sym} with the next symbol's
	information, returning that symbol's (internal) index into the
	symbol table.

	Supply <<BFD_NO_MORE_SYMBOLS>> as the @var{previous} entry to get
	the first one; returns <<BFD_NO_MORE_SYMBOLS>> when you've already
	got the last one.

	A <<carsym>> is a canonical archive symbol.  The only
	user-visible element is its name, a null-terminated string.
*/

symindex
bfd_get_next_mapent (bfd *abfd, symindex prev, carsym **entry)
{
  if (!bfd_has_map (abfd))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return BFD_NO_MORE_SYMBOLS;
    }

  if (prev == BFD_NO_MORE_SYMBOLS)
    prev = 0;
  else
    ++prev;
  if (prev >= bfd_ardata (abfd)->symdef_count)
    return BFD_NO_MORE_SYMBOLS;

  *entry = (bfd_ardata (abfd)->symdefs + prev);
  return prev;
}

/* To be called by backends only.  */

bfd *
_bfd_create_empty_archive_element_shell (bfd *obfd)
{
  return _bfd_new_bfd_contained_in (obfd);
}

/*
FUNCTION
	bfd_set_archive_head

SYNOPSIS
	bfd_boolean bfd_set_archive_head (bfd *output, bfd *new_head);

DESCRIPTION
	Set the head of the chain of
	BFDs contained in the archive @var{output} to @var{new_head}.
*/

bfd_boolean
bfd_set_archive_head (bfd *output_archive, bfd *new_head)
{
  output_archive->archive_head = new_head;
  return TRUE;
}

bfd *
_bfd_look_for_bfd_in_cache (bfd *arch_bfd, file_ptr filepos)
{
  htab_t hash_table = bfd_ardata (arch_bfd)->cache;
  struct ar_cache m;

  m.ptr = filepos;

  if (hash_table)
    {
      struct ar_cache *entry = (struct ar_cache *) htab_find (hash_table, &m);
      if (!entry)
	return NULL;
      return entry->arbfd;
    }
  else
    return NULL;
}

static hashval_t
hash_file_ptr (const void * p)
{
  return (hashval_t) (((struct ar_cache *) p)->ptr);
}

/* Returns non-zero if P1 and P2 are equal.  */

static int
eq_file_ptr (const void * p1, const void * p2)
{
  struct ar_cache *arc1 = (struct ar_cache *) p1;
  struct ar_cache *arc2 = (struct ar_cache *) p2;
  return arc1->ptr == arc2->ptr;
}

/* The calloc function doesn't always take size_t (e.g. on VMS)
   so wrap it to avoid a compile time warning.   */

static void *
_bfd_calloc_wrapper (size_t a, size_t b)
{
  return calloc (a, b);
}

/* Kind of stupid to call cons for each one, but we don't do too many.  */

bfd_boolean
_bfd_add_bfd_to_archive_cache (bfd *arch_bfd, file_ptr filepos, bfd *new_elt)
{
  struct ar_cache *cache;
  htab_t hash_table = bfd_ardata (arch_bfd)->cache;

  /* If the hash table hasn't been created, create it.  */
  if (hash_table == NULL)
    {
      hash_table = htab_create_alloc (16, hash_file_ptr, eq_file_ptr,
				      NULL, _bfd_calloc_wrapper, free);
      if (hash_table == NULL)
	return FALSE;
      bfd_ardata (arch_bfd)->cache = hash_table;
    }

  /* Insert new_elt into the hash table by filepos.  */
  cache = (struct ar_cache *) bfd_zalloc (arch_bfd, sizeof (struct ar_cache));
  cache->ptr = filepos;
  cache->arbfd = new_elt;
  *htab_find_slot (hash_table, (const void *) cache, INSERT) = cache;

  /* Provide a means of accessing this from child.  */
  arch_eltdata (new_elt)->parent_cache = hash_table;
  arch_eltdata (new_elt)->key = filepos;

  return TRUE;
}

static bfd *
_bfd_find_nested_archive (bfd *arch_bfd, const char *filename)
{
  bfd *abfd;
  const char *target;

  /* PR 15140: Don't allow a nested archive pointing to itself.  */
  if (filename_cmp (filename, arch_bfd->filename) == 0)
    {
      bfd_set_error (bfd_error_malformed_archive);
      return NULL;
    }

  for (abfd = arch_bfd->nested_archives;
       abfd != NULL;
       abfd = abfd->archive_next)
    {
      if (filename_cmp (filename, abfd->filename) == 0)
	return abfd;
    }
  target = NULL;
  if (!arch_bfd->target_defaulted)
    target = arch_bfd->xvec->name;
  abfd = bfd_openr (filename, target);
  if (abfd)
    {
      abfd->archive_next = arch_bfd->nested_archives;
      arch_bfd->nested_archives = abfd;
    }
  return abfd;
}

/* The name begins with space.  Hence the rest of the name is an index into
   the string table.  */

static char *
get_extended_arelt_filename (bfd *arch, const char *name, file_ptr *originp)
{
  unsigned long table_index = 0;
  const char *endp;

  /* Should extract string so that I can guarantee not to overflow into
     the next region, but I'm too lazy.  */
  errno = 0;
  /* Skip first char, which is '/' in SVR4 or ' ' in some other variants.  */
  table_index = strtol (name + 1, (char **) &endp, 10);
  if (errno != 0 || table_index >= bfd_ardata (arch)->extended_names_size)
    {
      bfd_set_error (bfd_error_malformed_archive);
      return NULL;
    }
  /* In a thin archive, a member of an archive-within-an-archive
     will have the offset in the inner archive encoded here.  */
  if (bfd_is_thin_archive (arch) && endp != NULL && *endp == ':')
    {
      file_ptr origin = strtol (endp + 1, NULL, 10);

      if (errno != 0)
	{
	  bfd_set_error (bfd_error_malformed_archive);
	  return NULL;
	}
      *originp = origin;
    }
  else
    *originp = 0;

  return bfd_ardata (arch)->extended_names + table_index;
}

/* This functions reads an arch header and returns an areltdata pointer, or
   NULL on error.

   Presumes the file pointer is already in the right place (ie pointing
   to the ar_hdr in the file).   Moves the file pointer; on success it
   should be pointing to the front of the file contents; on failure it
   could have been moved arbitrarily.  */

void *
_bfd_generic_read_ar_hdr (bfd *abfd)
{
  return _bfd_generic_read_ar_hdr_mag (abfd, NULL);
}

/* Alpha ECOFF uses an optional different ARFMAG value, so we have a
   variant of _bfd_generic_read_ar_hdr which accepts a magic string.  */

void *
_bfd_generic_read_ar_hdr_mag (bfd *abfd, const char *mag)
{
  struct ar_hdr hdr;
  char *hdrp = (char *) &hdr;
  bfd_size_type parsed_size;
  struct areltdata *ared;
  char *filename = NULL;
  bfd_size_type namelen = 0;
  bfd_size_type allocsize = sizeof (struct areltdata) + sizeof (struct ar_hdr);
  char *allocptr = 0;
  file_ptr origin = 0;
  unsigned int extra_size = 0;
  char fmag_save;
  int scan;

  if (bfd_bread (hdrp, sizeof (struct ar_hdr), abfd) != sizeof (struct ar_hdr))
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_no_more_archived_files);
      return NULL;
    }
  if (strncmp (hdr.ar_fmag, ARFMAG, 2) != 0
      && (mag == NULL
	  || strncmp (hdr.ar_fmag, mag, 2) != 0))
    {
      bfd_set_error (bfd_error_malformed_archive);
      return NULL;
    }

  errno = 0;
  fmag_save = hdr.ar_fmag[0];
  hdr.ar_fmag[0] = 0;
  scan = sscanf (hdr.ar_size, "%" BFD_VMA_FMT "u", &parsed_size);
  hdr.ar_fmag[0] = fmag_save;
  if (scan != 1)
    {
      bfd_set_error (bfd_error_malformed_archive);
      return NULL;
    }

  /* Extract the filename from the archive - there are two ways to
     specify an extended name table, either the first char of the
     name is a space, or it's a slash.  */
  if ((hdr.ar_name[0] == '/'
       || (hdr.ar_name[0] == ' '
	   && memchr (hdr.ar_name, '/', ar_maxnamelen (abfd)) == NULL))
      && bfd_ardata (abfd)->extended_names != NULL)
    {
      filename = get_extended_arelt_filename (abfd, hdr.ar_name, &origin);
      if (filename == NULL)
	return NULL;
    }
  /* BSD4.4-style long filename.  */
  else if (is_bsd44_extended_name (hdr.ar_name))
    {
      /* BSD-4.4 extended name */
      namelen = atoi (&hdr.ar_name[3]);
      allocsize += namelen + 1;
      parsed_size -= namelen;
      extra_size = namelen;

      allocptr = (char *) bfd_zmalloc (allocsize);
      if (allocptr == NULL)
	return NULL;
      filename = (allocptr
		  + sizeof (struct areltdata)
		  + sizeof (struct ar_hdr));
      if (bfd_bread (filename, namelen, abfd) != namelen)
	{
	  free (allocptr);
	  if (bfd_get_error () != bfd_error_system_call)
	    bfd_set_error (bfd_error_no_more_archived_files);
	  return NULL;
	}
      filename[namelen] = '\0';
    }
  else
    {
      /* We judge the end of the name by looking for '/' or ' '.
	 Note:  The SYSV format (terminated by '/') allows embedded
	 spaces, so only look for ' ' if we don't find '/'.  */

      char *e;
      e = (char *) memchr (hdr.ar_name, '\0', ar_maxnamelen (abfd));
      if (e == NULL)
	{
	  e = (char *) memchr (hdr.ar_name, '/', ar_maxnamelen (abfd));
	  if (e == NULL)
	    e = (char *) memchr (hdr.ar_name, ' ', ar_maxnamelen (abfd));
	}

      if (e != NULL)
	namelen = e - hdr.ar_name;
      else
	{
	  /* If we didn't find a termination character, then the name
	     must be the entire field.  */
	  namelen = ar_maxnamelen (abfd);
	}

      allocsize += namelen + 1;
    }

  if (!allocptr)
    {
      allocptr = (char *) bfd_zmalloc (allocsize);
      if (allocptr == NULL)
	return NULL;
    }

  ared = (struct areltdata *) allocptr;

  ared->arch_header = allocptr + sizeof (struct areltdata);
  memcpy (ared->arch_header, &hdr, sizeof (struct ar_hdr));
  ared->parsed_size = parsed_size;
  ared->extra_size = extra_size;
  ared->origin = origin;

  if (filename != NULL)
    ared->filename = filename;
  else
    {
      ared->filename = allocptr + (sizeof (struct areltdata) +
				   sizeof (struct ar_hdr));
      if (namelen)
	memcpy (ared->filename, hdr.ar_name, namelen);
      ared->filename[namelen] = '\0';
    }

  return ared;
}

/* Append the relative pathname for a member of the thin archive
   to the pathname of the directory containing the archive.  */

char *
_bfd_append_relative_path (bfd *arch, char *elt_name)
{
  const char *arch_name = arch->filename;
  const char *base_name = lbasename (arch_name);
  size_t prefix_len;
  char *filename;

  if (base_name == arch_name)
    return elt_name;

  prefix_len = base_name - arch_name;
  filename = (char *) bfd_alloc (arch, prefix_len + strlen (elt_name) + 1);
  if (filename == NULL)
    return NULL;

  strncpy (filename, arch_name, prefix_len);
  strcpy (filename + prefix_len, elt_name);
  return filename;
}

/* This is an internal function; it's mainly used when indexing
   through the archive symbol table, but also used to get the next
   element, since it handles the bookkeeping so nicely for us.  */

bfd *
_bfd_get_elt_at_filepos (bfd *archive, file_ptr filepos)
{
  struct areltdata *new_areldata;
  bfd *n_nfd;
  char *filename;

  n_nfd = _bfd_look_for_bfd_in_cache (archive, filepos);
  if (n_nfd)
    return n_nfd;

  if (0 > bfd_seek (archive, filepos, SEEK_SET))
    return NULL;

  if ((new_areldata = (struct areltdata *) _bfd_read_ar_hdr (archive)) == NULL)
    return NULL;

  filename = new_areldata->filename;

  if (bfd_is_thin_archive (archive))
    {
      const char *target;

      /* This is a proxy entry for an external file.  */
      if (! IS_ABSOLUTE_PATH (filename))
	{
	  filename = _bfd_append_relative_path (archive, filename);
	  if (filename == NULL)
	    {
	      free (new_areldata);
	      return NULL;
	    }
	}

      if (new_areldata->origin > 0)
	{
	  /* This proxy entry refers to an element of a nested archive.
	     Locate the member of that archive and return a bfd for it.  */
	  bfd *ext_arch = _bfd_find_nested_archive (archive, filename);

	  if (ext_arch == NULL
	      || ! bfd_check_format (ext_arch, bfd_archive))
	    {
	      free (new_areldata);
	      return NULL;
	    }
	  n_nfd = _bfd_get_elt_at_filepos (ext_arch, new_areldata->origin);
	  if (n_nfd == NULL)
	    {
	      free (new_areldata);
	      return NULL;
	    }
	  n_nfd->proxy_origin = bfd_tell (archive);
	  return n_nfd;
	}
      /* It's not an element of a nested archive;
	 open the external file as a bfd.  */
      target = NULL;
      if (!archive->target_defaulted)
	target = archive->xvec->name;
      n_nfd = bfd_openr (filename, target);
      if (n_nfd == NULL)
	bfd_set_error (bfd_error_malformed_archive);
    }
  else
    {
      n_nfd = _bfd_create_empty_archive_element_shell (archive);
    }

  if (n_nfd == NULL)
    {
      free (new_areldata);
      return NULL;
    }

  n_nfd->proxy_origin = bfd_tell (archive);

  if (bfd_is_thin_archive (archive))
    {
      n_nfd->origin = 0;
    }
  else
    {
      n_nfd->origin = n_nfd->proxy_origin;
      n_nfd->filename = xstrdup (filename);
    }

  n_nfd->arelt_data = new_areldata;

  /* Copy BFD_COMPRESS and BFD_DECOMPRESS flags.  */
  n_nfd->flags |= archive->flags & (BFD_COMPRESS | BFD_DECOMPRESS);

  if (_bfd_add_bfd_to_archive_cache (archive, filepos, n_nfd))
    return n_nfd;

  free (new_areldata);
  n_nfd->arelt_data = NULL;
  return NULL;
}

/* Return the BFD which is referenced by the symbol in ABFD indexed by
   SYM_INDEX.  SYM_INDEX should have been returned by bfd_get_next_mapent.  */

bfd *
_bfd_generic_get_elt_at_index (bfd *abfd, symindex sym_index)
{
  carsym *entry;

  entry = bfd_ardata (abfd)->symdefs + sym_index;
  return _bfd_get_elt_at_filepos (abfd, entry->file_offset);
}

/*
FUNCTION
	bfd_openr_next_archived_file

SYNOPSIS
	bfd *bfd_openr_next_archived_file (bfd *archive, bfd *previous);

DESCRIPTION
	Provided a BFD, @var{archive}, containing an archive and NULL, open
	an input BFD on the first contained element and returns that.
	Subsequent calls should pass
	the archive and the previous return value to return a created
	BFD to the next contained element. NULL is returned when there
	are no more.
*/

bfd *
bfd_openr_next_archived_file (bfd *archive, bfd *last_file)
{
  if ((bfd_get_format (archive) != bfd_archive)
      || (archive->direction == write_direction))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  return BFD_SEND (archive,
		   openr_next_archived_file, (archive, last_file));
}

bfd *
bfd_generic_openr_next_archived_file (bfd *archive, bfd *last_file)
{
  file_ptr filestart;

  if (!last_file)
    filestart = bfd_ardata (archive)->first_file_filepos;
  else
    {
      bfd_size_type size = arelt_size (last_file);

      filestart = last_file->proxy_origin;
      if (! bfd_is_thin_archive (archive))
	filestart += size;
      /* Pad to an even boundary...
	 Note that last_file->origin can be odd in the case of
	 BSD-4.4-style element with a long odd size.  */
      filestart += filestart % 2;
    }

  return _bfd_get_elt_at_filepos (archive, filestart);
}

const bfd_target *
bfd_generic_archive_p (bfd *abfd)
{
  struct artdata *tdata_hold;
  char armag[SARMAG + 1];
  bfd_size_type amt;

  if (bfd_bread (armag, SARMAG, abfd) != SARMAG)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  bfd_is_thin_archive (abfd) = (strncmp (armag, ARMAGT, SARMAG) == 0);

  if (strncmp (armag, ARMAG, SARMAG) != 0
      && strncmp (armag, ARMAGB, SARMAG) != 0
      && ! bfd_is_thin_archive (abfd))
    return NULL;

  tdata_hold = bfd_ardata (abfd);

  amt = sizeof (struct artdata);
  bfd_ardata (abfd) = (struct artdata *) bfd_zalloc (abfd, amt);
  if (bfd_ardata (abfd) == NULL)
    {
      bfd_ardata (abfd) = tdata_hold;
      return NULL;
    }

  bfd_ardata (abfd)->first_file_filepos = SARMAG;
  /* Cleared by bfd_zalloc above.
     bfd_ardata (abfd)->cache = NULL;
     bfd_ardata (abfd)->archive_head = NULL;
     bfd_ardata (abfd)->symdefs = NULL;
     bfd_ardata (abfd)->extended_names = NULL;
     bfd_ardata (abfd)->extended_names_size = 0;
     bfd_ardata (abfd)->tdata = NULL;  */

  if (!BFD_SEND (abfd, _bfd_slurp_armap, (abfd))
      || !BFD_SEND (abfd, _bfd_slurp_extended_name_table, (abfd)))
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      bfd_release (abfd, bfd_ardata (abfd));
      bfd_ardata (abfd) = tdata_hold;
      return NULL;
    }

  if (abfd->target_defaulted && bfd_has_map (abfd))
    {
      bfd *first;

      /* This archive has a map, so we may presume that the contents
	 are object files.  Make sure that if the first file in the
	 archive can be recognized as an object file, it is for this
	 target.  If not, assume that this is the wrong format.  If
	 the first file is not an object file, somebody is doing
	 something weird, and we permit it so that ar -t will work.

	 This is done because any normal format will recognize any
	 normal archive, regardless of the format of the object files.
	 We do accept an empty archive.  */

      first = bfd_openr_next_archived_file (abfd, NULL);
      if (first != NULL)
	{
	  first->target_defaulted = FALSE;
	  if (bfd_check_format (first, bfd_object)
	      && first->xvec != abfd->xvec)
	    bfd_set_error (bfd_error_wrong_object_format);
	  /* And we ought to close `first' here too.  */
	}
    }

  return abfd->xvec;
}

/* Some constants for a 32 bit BSD archive structure.  We do not
   support 64 bit archives presently; so far as I know, none actually
   exist.  Supporting them would require changing these constants, and
   changing some H_GET_32 to H_GET_64.  */

/* The size of an external symdef structure.  */
#define BSD_SYMDEF_SIZE 8

/* The offset from the start of a symdef structure to the file offset.  */
#define BSD_SYMDEF_OFFSET_SIZE 4

/* The size of the symdef count.  */
#define BSD_SYMDEF_COUNT_SIZE 4

/* The size of the string count.  */
#define BSD_STRING_COUNT_SIZE 4

/* Read a BSD-style archive symbol table.  Returns FALSE on error,
   TRUE otherwise.  */

static bfd_boolean
do_slurp_bsd_armap (bfd *abfd)
{
  struct areltdata *mapdata;
  unsigned int counter;
  bfd_byte *raw_armap, *rbase;
  struct artdata *ardata = bfd_ardata (abfd);
  char *stringbase;
  bfd_size_type parsed_size, amt;
  carsym *set;

  mapdata = (struct areltdata *) _bfd_read_ar_hdr (abfd);
  if (mapdata == NULL)
    return FALSE;
  parsed_size = mapdata->parsed_size;
  free (mapdata);
  /* PR 17512: file: 883ff754.  */
  /* PR 17512: file: 0458885f.  */
  if (parsed_size < 4)
    return FALSE;

  raw_armap = (bfd_byte *) bfd_zalloc (abfd, parsed_size);
  if (raw_armap == NULL)
    return FALSE;

  if (bfd_bread (raw_armap, parsed_size, abfd) != parsed_size)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_malformed_archive);
    byebye:
      bfd_release (abfd, raw_armap);
      return FALSE;
    }

  ardata->symdef_count = H_GET_32 (abfd, raw_armap) / BSD_SYMDEF_SIZE;

  if (ardata->symdef_count * BSD_SYMDEF_SIZE >
      parsed_size - BSD_SYMDEF_COUNT_SIZE)
    {
      /* Probably we're using the wrong byte ordering.  */
      bfd_set_error (bfd_error_wrong_format);
      goto byebye;
    }

  ardata->cache = 0;
  rbase = raw_armap + BSD_SYMDEF_COUNT_SIZE;
  stringbase = ((char *) rbase
		+ ardata->symdef_count * BSD_SYMDEF_SIZE
		+ BSD_STRING_COUNT_SIZE);
  amt = ardata->symdef_count * sizeof (carsym);
  ardata->symdefs = (struct carsym *) bfd_alloc (abfd, amt);
  if (!ardata->symdefs)
    return FALSE;

  for (counter = 0, set = ardata->symdefs;
       counter < ardata->symdef_count;
       counter++, set++, rbase += BSD_SYMDEF_SIZE)
    {
      set->name = H_GET_32 (abfd, rbase) + stringbase;
      set->file_offset = H_GET_32 (abfd, rbase + BSD_SYMDEF_OFFSET_SIZE);
    }

  ardata->first_file_filepos = bfd_tell (abfd);
  /* Pad to an even boundary if you have to.  */
  ardata->first_file_filepos += (ardata->first_file_filepos) % 2;
  /* FIXME, we should provide some way to free raw_ardata when
     we are done using the strings from it.  For now, it seems
     to be allocated on an objalloc anyway...  */
  bfd_has_map (abfd) = TRUE;
  return TRUE;
}

/* Read a COFF archive symbol table.  Returns FALSE on error, TRUE
   otherwise.  */

static bfd_boolean
do_slurp_coff_armap (bfd *abfd)
{
  struct areltdata *mapdata;
  int *raw_armap, *rawptr;
  struct artdata *ardata = bfd_ardata (abfd);
  char *stringbase;
  bfd_size_type stringsize;
  bfd_size_type parsed_size;
  carsym *carsyms;
  bfd_size_type nsymz;		/* Number of symbols in armap.  */
  bfd_vma (*swap) (const void *);
  char int_buf[sizeof (long)];
  bfd_size_type carsym_size, ptrsize;
  unsigned int i;

  mapdata = (struct areltdata *) _bfd_read_ar_hdr (abfd);
  if (mapdata == NULL)
    return FALSE;
  parsed_size = mapdata->parsed_size;
  free (mapdata);

  if (bfd_bread (int_buf, 4, abfd) != 4)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_malformed_archive);
      return FALSE;
    }
  /* It seems that all numeric information in a coff archive is always
     in big endian format, nomatter the host or target.  */
  swap = bfd_getb32;
  nsymz = bfd_getb32 (int_buf);
  stringsize = parsed_size - (4 * nsymz) - 4;

  /* ... except that some archive formats are broken, and it may be our
     fault - the i960 little endian coff sometimes has big and sometimes
     little, because our tools changed.  Here's a horrible hack to clean
     up the crap.  */

  if (stringsize > 0xfffff
      && bfd_get_arch (abfd) == bfd_arch_i960
      && bfd_get_flavour (abfd) == bfd_target_coff_flavour)
    {
      /* This looks dangerous, let's do it the other way around.  */
      nsymz = bfd_getl32 (int_buf);
      stringsize = parsed_size - (4 * nsymz) - 4;
      swap = bfd_getl32;
    }

  /* The coff armap must be read sequentially.  So we construct a
     bsd-style one in core all at once, for simplicity.  */

  if (nsymz > ~ (bfd_size_type) 0 / sizeof (carsym))
    return FALSE;

  carsym_size = (nsymz * sizeof (carsym));
  ptrsize = (4 * nsymz);

  if (carsym_size + stringsize + 1 <= carsym_size)
    return FALSE;

  ardata->symdefs = (struct carsym *) bfd_zalloc (abfd,
						  carsym_size + stringsize + 1);
  if (ardata->symdefs == NULL)
    return FALSE;
  carsyms = ardata->symdefs;
  stringbase = ((char *) ardata->symdefs) + carsym_size;

  /* Allocate and read in the raw offsets.  */
  raw_armap = (int *) bfd_alloc (abfd, ptrsize);
  if (raw_armap == NULL)
    goto release_symdefs;
  if (bfd_bread (raw_armap, ptrsize, abfd) != ptrsize
      || (bfd_bread (stringbase, stringsize, abfd) != stringsize))
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_malformed_archive);
      goto release_raw_armap;
    }

  /* OK, build the carsyms.  */
  for (i = 0; i < nsymz && stringsize > 0; i++)
    {
      bfd_size_type len;

      rawptr = raw_armap + i;
      carsyms->file_offset = swap ((bfd_byte *) rawptr);
      carsyms->name = stringbase;
      /* PR 17512: file: 4a1d50c1.  */
      len = strnlen (stringbase, stringsize);
      if (len < stringsize)
	len ++;
      stringbase += len;
      stringsize -= len;
      carsyms++;
    }
  *stringbase = 0;

  ardata->symdef_count = nsymz;
  ardata->first_file_filepos = bfd_tell (abfd);
  /* Pad to an even boundary if you have to.  */
  ardata->first_file_filepos += (ardata->first_file_filepos) % 2;

  bfd_has_map (abfd) = TRUE;
  bfd_release (abfd, raw_armap);

  /* Check for a second archive header (as used by PE).  */
  {
    struct areltdata *tmp;

    bfd_seek (abfd, ardata->first_file_filepos, SEEK_SET);
    tmp = (struct areltdata *) _bfd_read_ar_hdr (abfd);
    if (tmp != NULL)
      {
	if (tmp->arch_header[0] == '/'
	    && tmp->arch_header[1] == ' ')
	  {
	    ardata->first_file_filepos +=
	      (tmp->parsed_size + sizeof (struct ar_hdr) + 1) & ~(unsigned) 1;
	  }
	free (tmp);
      }
  }

  return TRUE;

release_raw_armap:
  bfd_release (abfd, raw_armap);
release_symdefs:
  bfd_release (abfd, (ardata)->symdefs);
  return FALSE;
}

/* This routine can handle either coff-style or bsd-style armaps
   (archive symbol table).  Returns FALSE on error, TRUE otherwise */

bfd_boolean
bfd_slurp_armap (bfd *abfd)
{
  char nextname[17];
  int i = bfd_bread (nextname, 16, abfd);

  if (i == 0)
    return TRUE;
  if (i != 16)
    return FALSE;

  if (bfd_seek (abfd, (file_ptr) -16, SEEK_CUR) != 0)
    return FALSE;

  if (CONST_STRNEQ (nextname, "__.SYMDEF       ")
      || CONST_STRNEQ (nextname, "__.SYMDEF/      ")) /* Old Linux archives.  */
    return do_slurp_bsd_armap (abfd);
  else if (CONST_STRNEQ (nextname, "/               "))
    return do_slurp_coff_armap (abfd);
  else if (CONST_STRNEQ (nextname, "/SYM64/         "))
    {
      /* 64bit ELF (Irix 6) archive.  */
#ifdef BFD64
      extern bfd_boolean bfd_elf64_archive_slurp_armap (bfd *);
      return bfd_elf64_archive_slurp_armap (abfd);
#else
      bfd_set_error (bfd_error_wrong_format);
      return FALSE;
#endif
    }
  else if (CONST_STRNEQ (nextname, "#1/20           "))
    {
      /* Mach-O has a special name for armap when the map is sorted by name.
	 However because this name has a space it is slightly more difficult
	 to check it.  */
      struct ar_hdr hdr;
      char extname[21];

      if (bfd_bread (&hdr, sizeof (hdr), abfd) != sizeof (hdr))
	return FALSE;
      /* Read the extended name.  We know its length.  */
      if (bfd_bread (extname, 20, abfd) != 20)
	return FALSE;
      if (bfd_seek (abfd, -(file_ptr) (sizeof (hdr) + 20), SEEK_CUR) != 0)
	return FALSE;
      extname[20] = 0;
      if (CONST_STRNEQ (extname, "__.SYMDEF SORTED")
	  || CONST_STRNEQ (extname, "__.SYMDEF"))
	return do_slurp_bsd_armap (abfd);
    }

  bfd_has_map (abfd) = FALSE;
  return TRUE;
}

/* Returns FALSE on error, TRUE otherwise.  */
/* Flavor 2 of a bsd armap, similar to bfd_slurp_bsd_armap except the
   header is in a slightly different order and the map name is '/'.
   This flavour is used by hp300hpux.  */

#define HPUX_SYMDEF_COUNT_SIZE 2

bfd_boolean
bfd_slurp_bsd_armap_f2 (bfd *abfd)
{
  struct areltdata *mapdata;
  char nextname[17];
  unsigned int counter;
  bfd_byte *raw_armap, *rbase;
  struct artdata *ardata = bfd_ardata (abfd);
  char *stringbase;
  unsigned int stringsize;
  unsigned int left;
  bfd_size_type amt;
  carsym *set;
  int i = bfd_bread (nextname, 16, abfd);

  if (i == 0)
    return TRUE;
  if (i != 16)
    return FALSE;

  /* The archive has at least 16 bytes in it.  */
  if (bfd_seek (abfd, (file_ptr) -16, SEEK_CUR) != 0)
    return FALSE;

  if (CONST_STRNEQ (nextname, "__.SYMDEF       ")
      || CONST_STRNEQ (nextname, "__.SYMDEF/      ")) /* Old Linux archives.  */
    return do_slurp_bsd_armap (abfd);

  if (! CONST_STRNEQ (nextname, "/               "))
    {
      bfd_has_map (abfd) = FALSE;
      return TRUE;
    }

  mapdata = (struct areltdata *) _bfd_read_ar_hdr (abfd);
  if (mapdata == NULL)
    return FALSE;

  if (mapdata->parsed_size < HPUX_SYMDEF_COUNT_SIZE + BSD_STRING_COUNT_SIZE)
    {
      free (mapdata);
    wrong_format:
      bfd_set_error (bfd_error_wrong_format);
    byebye:
      return FALSE;
    }
  left = mapdata->parsed_size - HPUX_SYMDEF_COUNT_SIZE - BSD_STRING_COUNT_SIZE;

  amt = mapdata->parsed_size;
  free (mapdata);

  raw_armap = (bfd_byte *) bfd_zalloc (abfd, amt);
  if (raw_armap == NULL)
    goto byebye;

  if (bfd_bread (raw_armap, amt, abfd) != amt)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_malformed_archive);
      goto byebye;
    }

  ardata->symdef_count = H_GET_16 (abfd, raw_armap);

  ardata->cache = 0;

  stringsize = H_GET_32 (abfd, raw_armap + HPUX_SYMDEF_COUNT_SIZE);
  if (stringsize > left)
    goto wrong_format;
  left -= stringsize;

  /* Skip sym count and string sz.  */
  stringbase = ((char *) raw_armap
		+ HPUX_SYMDEF_COUNT_SIZE
		+ BSD_STRING_COUNT_SIZE);
  rbase = (bfd_byte *) stringbase + stringsize;
  amt = ardata->symdef_count * BSD_SYMDEF_SIZE;
  if (amt > left)
    goto wrong_format;

  ardata->symdefs = (struct carsym *) bfd_alloc (abfd, amt);
  if (!ardata->symdefs)
    return FALSE;

  for (counter = 0, set = ardata->symdefs;
       counter < ardata->symdef_count;
       counter++, set++, rbase += BSD_SYMDEF_SIZE)
    {
      set->name = H_GET_32 (abfd, rbase) + stringbase;
      set->file_offset = H_GET_32 (abfd, rbase + BSD_SYMDEF_OFFSET_SIZE);
    }

  ardata->first_file_filepos = bfd_tell (abfd);
  /* Pad to an even boundary if you have to.  */
  ardata->first_file_filepos += (ardata->first_file_filepos) % 2;
  /* FIXME, we should provide some way to free raw_ardata when
     we are done using the strings from it.  For now, it seems
     to be allocated on an objalloc anyway...  */
  bfd_has_map (abfd) = TRUE;
  return TRUE;
}

/** Extended name table.

  Normally archives support only 14-character filenames.

  Intel has extended the format: longer names are stored in a special
  element (the first in the archive, or second if there is an armap);
  the name in the ar_hdr is replaced by <space><index into filename
  element>.  Index is the P.R. of an int (decimal).  Data General have
  extended the format by using the prefix // for the special element.  */

/* Returns FALSE on error, TRUE otherwise.  */

bfd_boolean
_bfd_slurp_extended_name_table (bfd *abfd)
{
  char nextname[17];
  struct areltdata *namedata;
  bfd_size_type amt;

  /* FIXME:  Formatting sucks here, and in case of failure of BFD_READ,
     we probably don't want to return TRUE.  */
  if (bfd_seek (abfd, bfd_ardata (abfd)->first_file_filepos, SEEK_SET) != 0)
    return FALSE;

  if (bfd_bread (nextname, 16, abfd) == 16)
    {
      if (bfd_seek (abfd, (file_ptr) -16, SEEK_CUR) != 0)
	return FALSE;

      if (! CONST_STRNEQ (nextname, "ARFILENAMES/    ")
	  && ! CONST_STRNEQ (nextname, "//              "))
	{
	  bfd_ardata (abfd)->extended_names = NULL;
	  bfd_ardata (abfd)->extended_names_size = 0;
	  return TRUE;
	}

      namedata = (struct areltdata *) _bfd_read_ar_hdr (abfd);
      if (namedata == NULL)
	return FALSE;

      amt = namedata->parsed_size;
      if (amt + 1 == 0)
	goto byebye;

      bfd_ardata (abfd)->extended_names_size = amt;
      bfd_ardata (abfd)->extended_names = (char *) bfd_zalloc (abfd, amt + 1);
      if (bfd_ardata (abfd)->extended_names == NULL)
	{
	byebye:
	  free (namedata);
	  bfd_ardata (abfd)->extended_names = NULL;
	  bfd_ardata (abfd)->extended_names_size = 0;
	  return FALSE;
	}

      if (bfd_bread (bfd_ardata (abfd)->extended_names, amt, abfd) != amt)
	{
	  if (bfd_get_error () != bfd_error_system_call)
	    bfd_set_error (bfd_error_malformed_archive);
	  bfd_release (abfd, (bfd_ardata (abfd)->extended_names));
	  bfd_ardata (abfd)->extended_names = NULL;
	  goto byebye;
	}

      /* Since the archive is supposed to be printable if it contains
	 text, the entries in the list are newline-padded, not null
	 padded. In SVR4-style archives, the names also have a
	 trailing '/'.  DOS/NT created archive often have \ in them
	 We'll fix all problems here.  */
      {
	char *ext_names = bfd_ardata (abfd)->extended_names;
	char *temp = ext_names;
	char *limit = temp + namedata->parsed_size;

	for (; temp < limit; ++temp)
	  {
	    if (*temp == ARFMAG[1])
	      temp[temp > ext_names && temp[-1] == '/' ? -1 : 0] = '\0';
	    if (*temp == '\\')
	      *temp = '/';
	  }
	*limit = '\0';
      }

      /* Pad to an even boundary if you have to.  */
      bfd_ardata (abfd)->first_file_filepos = bfd_tell (abfd);
      bfd_ardata (abfd)->first_file_filepos +=
	(bfd_ardata (abfd)->first_file_filepos) % 2;

      free (namedata);
    }
  return TRUE;
}

#ifdef VMS

/* Return a copy of the stuff in the filename between any :]> and a
   semicolon.  */

static const char *
normalize (bfd *abfd, const char *file)
{
  const char *first;
  const char *last;
  char *copy;

  first = file + strlen (file) - 1;
  last = first + 1;

  while (first != file)
    {
      if (*first == ';')
	last = first;
      if (*first == ':' || *first == ']' || *first == '>')
	{
	  first++;
	  break;
	}
      first--;
    }

  copy = bfd_alloc (abfd, last - first + 1);
  if (copy == NULL)
    return NULL;

  memcpy (copy, first, last - first);
  copy[last - first] = 0;

  return copy;
}

#else
static const char *
normalize (bfd *abfd ATTRIBUTE_UNUSED, const char *file)
{
  return lbasename (file);
}
#endif

/* Adjust a relative path name based on the reference path.
   For example:

     Relative path  Reference path  Result
     -------------  --------------  ------
     bar.o          lib.a           bar.o
     foo/bar.o      lib.a           foo/bar.o
     bar.o          foo/lib.a       ../bar.o
     foo/bar.o      baz/lib.a       ../foo/bar.o
     bar.o          ../lib.a        <parent of current dir>/bar.o
   ; ../bar.o       ../lib.a        bar.o
   ; ../bar.o       lib.a           ../bar.o
     foo/bar.o      ../lib.a        <parent of current dir>/foo/bar.o
     bar.o          ../../lib.a     <grandparent>/<parent>/bar.o
     bar.o          foo/baz/lib.a   ../../bar.o

   Note - the semicolons above are there to prevent the BFD chew
   utility from interpreting those lines as prototypes to put into
   the autogenerated bfd.h header...

   Note - the string is returned in a static buffer.  */

static const char *
adjust_relative_path (const char * path, const char * ref_path)
{
  static char *pathbuf = NULL;
  static unsigned int pathbuf_len = 0;
  const char *pathp;
  const char *refp;
  char * lpath;
  char * rpath;
  unsigned int len;
  unsigned int dir_up = 0;
  unsigned int dir_down = 0;
  char *newp;
  char * pwd = getpwd ();
  const char * down;

  /* Remove symlinks, '.' and '..' from the paths, if possible.  */
  lpath = lrealpath (path);
  pathp = lpath == NULL ? path : lpath;

  rpath = lrealpath (ref_path);
  refp = rpath == NULL ? ref_path : rpath;

  /* Remove common leading path elements.  */
  for (;;)
    {
      const char *e1 = pathp;
      const char *e2 = refp;

      while (*e1 && ! IS_DIR_SEPARATOR (*e1))
	++e1;
      while (*e2 && ! IS_DIR_SEPARATOR (*e2))
	++e2;
      if (*e1 == '\0' || *e2 == '\0' || e1 - pathp != e2 - refp
	  || filename_ncmp (pathp, refp, e1 - pathp) != 0)
	break;
      pathp = e1 + 1;
      refp = e2 + 1;
    }

  len = strlen (pathp) + 1;
  /* For each leading path element in the reference path,
     insert "../" into the path.  */
  for (; *refp; ++refp)
    if (IS_DIR_SEPARATOR (*refp))
      {
	/* PR 12710:  If the path element is "../" then instead of
	   inserting "../" we need to insert the name of the directory
	   at the current level.  */
	if (refp > ref_path + 1
	    && refp[-1] == '.'
	    && refp[-2] == '.')
	  dir_down ++;
	else
	  dir_up ++;
      }

  /* If the lrealpath calls above succeeded then we should never
     see dir_up and dir_down both being non-zero.  */

  len += 3 * dir_up;

  if (dir_down)
    {
      down = pwd + strlen (pwd) - 1;

      while (dir_down && down > pwd)
	{
	  if (IS_DIR_SEPARATOR (*down))
	    --dir_down;
	}
      BFD_ASSERT (dir_down == 0);
      len += strlen (down) + 1;
    }
  else
    down = NULL;

  if (len > pathbuf_len)
    {
      if (pathbuf != NULL)
	free (pathbuf);
      pathbuf_len = 0;
      pathbuf = (char *) bfd_malloc (len);
      if (pathbuf == NULL)
	goto out;
      pathbuf_len = len;
    }

  newp = pathbuf;
  while (dir_up-- > 0)
    {
      /* FIXME: Support Windows style path separators as well.  */
      strcpy (newp, "../");
      newp += 3;
    }

  if (down)
    sprintf (newp, "%s/%s", down, pathp);
  else
    strcpy (newp, pathp);

 out:
  free (lpath);
  free (rpath);
  return pathbuf;
}

/* Build a BFD style extended name table.  */

bfd_boolean
_bfd_archive_bsd_construct_extended_name_table (bfd *abfd,
						char **tabloc,
						bfd_size_type *tablen,
						const char **name)
{
  *name = "ARFILENAMES/";
  return _bfd_construct_extended_name_table (abfd, FALSE, tabloc, tablen);
}

/* Build an SVR4 style extended name table.  */

bfd_boolean
_bfd_archive_coff_construct_extended_name_table (bfd *abfd,
						 char **tabloc,
						 bfd_size_type *tablen,
						 const char **name)
{
  *name = "//";
  return _bfd_construct_extended_name_table (abfd, TRUE, tabloc, tablen);
}

/* Follows archive_head and produces an extended name table if
   necessary.  Returns (in tabloc) a pointer to an extended name
   table, and in tablen the length of the table.  If it makes an entry
   it clobbers the filename so that the element may be written without
   further massage.  Returns TRUE if it ran successfully, FALSE if
   something went wrong.  A successful return may still involve a
   zero-length tablen!  */

bfd_boolean
_bfd_construct_extended_name_table (bfd *abfd,
				    bfd_boolean trailing_slash,
				    char **tabloc,
				    bfd_size_type *tablen)
{
  unsigned int maxname = ar_maxnamelen (abfd);
  bfd_size_type total_namelen = 0;
  bfd *current;
  char *strptr;
  const char *last_filename;
  long last_stroff;

  *tablen = 0;
  last_filename = NULL;

  /* Figure out how long the table should be.  */
  for (current = abfd->archive_head;
       current != NULL;
       current = current->archive_next)
    {
      const char *normal;
      unsigned int thislen;

      if (bfd_is_thin_archive (abfd))
	{
	  const char *filename = current->filename;

	  /* If the element being added is a member of another archive
	     (i.e., we are flattening), use the containing archive's name.  */
	  if (current->my_archive
	      && ! bfd_is_thin_archive (current->my_archive))
	    filename = current->my_archive->filename;

	  /* If the path is the same as the previous path seen,
	     reuse it.  This can happen when flattening a thin
	     archive that contains other archives.  */
	  if (last_filename && filename_cmp (last_filename, filename) == 0)
	    continue;

	  last_filename = filename;

	  /* If the path is relative, adjust it relative to
	     the containing archive. */
	  if (! IS_ABSOLUTE_PATH (filename)
	      && ! IS_ABSOLUTE_PATH (abfd->filename))
	    normal = adjust_relative_path (filename, abfd->filename);
	  else
	    normal = filename;

	  /* In a thin archive, always store the full pathname
	     in the extended name table.  */
	  total_namelen += strlen (normal) + 1;
	  if (trailing_slash)
	    /* Leave room for trailing slash.  */
	    ++total_namelen;

	  continue;
	}

      normal = normalize (current, current->filename);
      if (normal == NULL)
	return FALSE;

      thislen = strlen (normal);

      if (thislen > maxname
	  && (bfd_get_file_flags (abfd) & BFD_TRADITIONAL_FORMAT) != 0)
	thislen = maxname;

      if (thislen > maxname)
	{
	  /* Add one to leave room for \n.  */
	  total_namelen += thislen + 1;
	  if (trailing_slash)
	    {
	      /* Leave room for trailing slash.  */
	      ++total_namelen;
	    }
	}
      else
	{
	  struct ar_hdr *hdr = arch_hdr (current);
	  if (filename_ncmp (normal, hdr->ar_name, thislen) != 0
	      || (thislen < sizeof hdr->ar_name
		  && hdr->ar_name[thislen] != ar_padchar (current)))
	    {
	      /* Must have been using extended format even though it
		 didn't need to.  Fix it to use normal format.  */
	      memcpy (hdr->ar_name, normal, thislen);
	      if (thislen < maxname
		  || (thislen == maxname && thislen < sizeof hdr->ar_name))
		hdr->ar_name[thislen] = ar_padchar (current);
	    }
	}
    }

  if (total_namelen == 0)
    return TRUE;

  *tabloc = (char *) bfd_zalloc (abfd, total_namelen);
  if (*tabloc == NULL)
    return FALSE;

  *tablen = total_namelen;
  strptr = *tabloc;

  last_filename = NULL;
  last_stroff = 0;

  for (current = abfd->archive_head;
       current != NULL;
       current = current->archive_next)
    {
      const char *normal;
      unsigned int thislen;
      long stroff;
      const char *filename = current->filename;

      if (bfd_is_thin_archive (abfd))
	{
	  /* If the element being added is a member of another archive
	     (i.e., we are flattening), use the containing archive's name.  */
	  if (current->my_archive
	      && ! bfd_is_thin_archive (current->my_archive))
	    filename = current->my_archive->filename;
	  /* If the path is the same as the previous path seen,
	     reuse it.  This can happen when flattening a thin
	     archive that contains other archives.
	     If the path is relative, adjust it relative to
	     the containing archive.  */
	  if (last_filename && filename_cmp (last_filename, filename) == 0)
	    normal = last_filename;
	  else if (! IS_ABSOLUTE_PATH (filename)
		   && ! IS_ABSOLUTE_PATH (abfd->filename))
	    normal = adjust_relative_path (filename, abfd->filename);
	  else
	    normal = filename;
	}
      else
	{
	  normal = normalize (current, filename);
	  if (normal == NULL)
	    return FALSE;
	}

      thislen = strlen (normal);
      if (thislen > maxname || bfd_is_thin_archive (abfd))
	{
	  /* Works for now; may need to be re-engineered if we
	     encounter an oddball archive format and want to
	     generalise this hack.  */
	  struct ar_hdr *hdr = arch_hdr (current);
	  if (normal == last_filename)
	    stroff = last_stroff;
	  else
	    {
	      strcpy (strptr, normal);
	      if (! trailing_slash)
		strptr[thislen] = ARFMAG[1];
	      else
		{
		  strptr[thislen] = '/';
		  strptr[thislen + 1] = ARFMAG[1];
		}
	      stroff = strptr - *tabloc;
	      last_stroff = stroff;
	    }
	  hdr->ar_name[0] = ar_padchar (current);
	  if (bfd_is_thin_archive (abfd) && current->origin > 0)
	    {
	      int len = snprintf (hdr->ar_name + 1, maxname - 1, "%-ld:",
				  stroff);
	      _bfd_ar_spacepad (hdr->ar_name + 1 + len, maxname - 1 - len,
				"%-ld",
				current->origin - sizeof (struct ar_hdr));
	    }
	  else
	    _bfd_ar_spacepad (hdr->ar_name + 1, maxname - 1, "%-ld", stroff);
	  if (normal != last_filename)
	    {
	      strptr += thislen + 1;
	      if (trailing_slash)
		++strptr;
	      last_filename = filename;
	    }
	}
    }

  return TRUE;
}

/* Do not construct an extended name table but transforms name field into
   its extended form.  */

bfd_boolean
_bfd_archive_bsd44_construct_extended_name_table (bfd *abfd,
						  char **tabloc,
						  bfd_size_type *tablen,
						  const char **name)
{
  unsigned int maxname = ar_maxnamelen (abfd);
  bfd *current;

  *tablen = 0;
  *tabloc = NULL;
  *name = NULL;

  for (current = abfd->archive_head;
       current != NULL;
       current = current->archive_next)
    {
      const char *normal = normalize (current, current->filename);
      int has_space = 0;
      unsigned int len;

      if (normal == NULL)
	return FALSE;

      for (len = 0; normal[len]; len++)
	if (normal[len] == ' ')
	  has_space = 1;

      if (len > maxname || has_space)
	{
	  struct ar_hdr *hdr = arch_hdr (current);

	  len = (len + 3) & ~3;
	  arch_eltdata (current)->extra_size = len;
	  _bfd_ar_spacepad (hdr->ar_name, maxname, "#1/%lu", len);
	}
    }

  return TRUE;
}

/* Write an archive header.  */

bfd_boolean
_bfd_generic_write_ar_hdr (bfd *archive, bfd *abfd)
{
  struct ar_hdr *hdr = arch_hdr (abfd);

  if (bfd_bwrite (hdr, sizeof (*hdr), archive) != sizeof (*hdr))
    return FALSE;
  return TRUE;
}

/* Write an archive header using BSD4.4 convention.  */

bfd_boolean
_bfd_bsd44_write_ar_hdr (bfd *archive, bfd *abfd)
{
  struct ar_hdr *hdr = arch_hdr (abfd);

  if (is_bsd44_extended_name (hdr->ar_name))
    {
      /* This is a BSD 4.4 extended name.  */
      const char *fullname = normalize (abfd, abfd->filename);
      unsigned int len = strlen (fullname);
      unsigned int padded_len = (len + 3) & ~3;

      BFD_ASSERT (padded_len == arch_eltdata (abfd)->extra_size);

      if (!_bfd_ar_sizepad (hdr->ar_size, sizeof (hdr->ar_size),
			    arch_eltdata (abfd)->parsed_size + padded_len))
	return FALSE;

      if (bfd_bwrite (hdr, sizeof (*hdr), archive) != sizeof (*hdr))
	return FALSE;

      if (bfd_bwrite (fullname, len, archive) != len)
	return FALSE;

      if (len & 3)
	{
	  static const char pad[3] = { 0, 0, 0 };

	  len = 4 - (len & 3);
	  if (bfd_bwrite (pad, len, archive) != len)
	    return FALSE;
	}
    }
  else
    {
      if (bfd_bwrite (hdr, sizeof (*hdr), archive) != sizeof (*hdr))
	return FALSE;
    }
  return TRUE;
}

/* A couple of functions for creating ar_hdrs.  */

#ifdef HPUX_LARGE_AR_IDS
/* Function to encode large UID/GID values according to HP.  */

static void
hpux_uid_gid_encode (char str[6], long int id)
{
  int cnt;

  str[5] = '@' + (id & 3);
  id >>= 2;

  for (cnt = 4; cnt >= 0; --cnt, id >>= 6)
    str[cnt] = ' ' + (id & 0x3f);
}
#endif	/* HPUX_LARGE_AR_IDS */

#ifndef HAVE_GETUID
#define getuid() 0
#endif

#ifndef HAVE_GETGID
#define getgid() 0
#endif

/* Takes a filename, returns an arelt_data for it, or NULL if it can't
   make one.  The filename must refer to a filename in the filesystem.
   The filename field of the ar_hdr will NOT be initialized.  If member
   is set, and it's an in-memory bfd, we fake it.  */

static struct areltdata *
bfd_ar_hdr_from_filesystem (bfd *abfd, const char *filename, bfd *member)
{
  struct stat status;
  struct areltdata *ared;
  struct ar_hdr *hdr;
  bfd_size_type amt;

  if (member && (member->flags & BFD_IN_MEMORY) != 0)
    {
      /* Assume we just "made" the member, and fake it.  */
      struct bfd_in_memory *bim = (struct bfd_in_memory *) member->iostream;
      time (&status.st_mtime);
      status.st_uid = getuid ();
      status.st_gid = getgid ();
      status.st_mode = 0644;
      status.st_size = bim->size;
    }
  else if (stat (filename, &status) != 0)
    {
      bfd_set_error (bfd_error_system_call);
      return NULL;
    }

  /* If the caller requested that the BFD generate deterministic output,
     fake values for modification time, UID, GID, and file mode.  */
  if ((abfd->flags & BFD_DETERMINISTIC_OUTPUT) != 0)
    {
      status.st_mtime = 0;
      status.st_uid = 0;
      status.st_gid = 0;
      status.st_mode = 0644;
    }

  amt = sizeof (struct ar_hdr) + sizeof (struct areltdata);
  ared = (struct areltdata *) bfd_zmalloc (amt);
  if (ared == NULL)
    return NULL;
  hdr = (struct ar_hdr *) (((char *) ared) + sizeof (struct areltdata));

  /* ar headers are space padded, not null padded!  */
  memset (hdr, ' ', sizeof (struct ar_hdr));

  _bfd_ar_spacepad (hdr->ar_date, sizeof (hdr->ar_date), "%-12ld",
		    status.st_mtime);
#ifdef HPUX_LARGE_AR_IDS
  /* HP has a very "special" way to handle UID/GID's with numeric values
     > 99999.  */
  if (status.st_uid > 99999)
    hpux_uid_gid_encode (hdr->ar_uid, (long) status.st_uid);
  else
#endif
    _bfd_ar_spacepad (hdr->ar_uid, sizeof (hdr->ar_uid), "%ld",
		      status.st_uid);
#ifdef HPUX_LARGE_AR_IDS
  /* HP has a very "special" way to handle UID/GID's with numeric values
     > 99999.  */
  if (status.st_gid > 99999)
    hpux_uid_gid_encode (hdr->ar_gid, (long) status.st_gid);
  else
#endif
    _bfd_ar_spacepad (hdr->ar_gid, sizeof (hdr->ar_gid), "%ld",
		      status.st_gid);
  _bfd_ar_spacepad (hdr->ar_mode, sizeof (hdr->ar_mode), "%-8lo",
		    status.st_mode);
  if (!_bfd_ar_sizepad (hdr->ar_size, sizeof (hdr->ar_size), status.st_size))
    {
      free (ared);
      return NULL;
    }
  memcpy (hdr->ar_fmag, ARFMAG, 2);
  ared->parsed_size = status.st_size;
  ared->arch_header = (char *) hdr;

  return ared;
}

/* Analogous to stat call.  */

int
bfd_generic_stat_arch_elt (bfd *abfd, struct stat *buf)
{
  struct ar_hdr *hdr;
  char *aloser;

  if (abfd->arelt_data == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  hdr = arch_hdr (abfd);
  /* PR 17512: file: 3d9e9fe9.  */
  if (hdr == NULL)
    return -1;

#define foo(arelt, stelt, size)				\
  buf->stelt = strtol (hdr->arelt, &aloser, size);	\
  if (aloser == hdr->arelt)	      			\
    return -1;

  /* Some platforms support special notations for large IDs.  */
#ifdef HPUX_LARGE_AR_IDS
# define foo2(arelt, stelt, size)					\
  if (hdr->arelt[5] == ' ')						\
    {									\
      foo (arelt, stelt, size);						\
    }									\
  else									\
    {									\
      int cnt;								\
      for (buf->stelt = cnt = 0; cnt < 5; ++cnt)			\
	{								\
	  if (hdr->arelt[cnt] < ' ' || hdr->arelt[cnt] > ' ' + 0x3f)	\
	    return -1;							\
	  buf->stelt <<= 6;						\
	  buf->stelt += hdr->arelt[cnt] - ' ';				\
	}								\
      if (hdr->arelt[5] < '@' || hdr->arelt[5] > '@' + 3)		\
	return -1;							\
      buf->stelt <<= 2;							\
      buf->stelt += hdr->arelt[5] - '@';				\
    }
#else
# define foo2(arelt, stelt, size) foo (arelt, stelt, size)
#endif

  foo (ar_date, st_mtime, 10);
  foo2 (ar_uid, st_uid, 10);
  foo2 (ar_gid, st_gid, 10);
  foo (ar_mode, st_mode, 8);

  buf->st_size = arch_eltdata (abfd)->parsed_size;

  return 0;
}

void
bfd_dont_truncate_arname (bfd *abfd, const char *pathname, char *arhdr)
{
  /* FIXME: This interacts unpleasantly with ar's quick-append option.
     Fortunately ic960 users will never use that option.  Fixing this
     is very hard; fortunately I know how to do it and will do so once
     intel's release is out the door.  */

  struct ar_hdr *hdr = (struct ar_hdr *) arhdr;
  size_t length;
  const char *filename;
  size_t maxlen = ar_maxnamelen (abfd);

  if ((bfd_get_file_flags (abfd) & BFD_TRADITIONAL_FORMAT) != 0)
    {
      bfd_bsd_truncate_arname (abfd, pathname, arhdr);
      return;
    }

  filename = normalize (abfd, pathname);
  if (filename == NULL)
    {
      /* FIXME */
      abort ();
    }

  length = strlen (filename);

  if (length <= maxlen)
    memcpy (hdr->ar_name, filename, length);

  /* Add the padding character if there is room for it.  */
  if (length < maxlen
      || (length == maxlen && length < sizeof hdr->ar_name))
    (hdr->ar_name)[length] = ar_padchar (abfd);
}

void
bfd_bsd_truncate_arname (bfd *abfd, const char *pathname, char *arhdr)
{
  struct ar_hdr *hdr = (struct ar_hdr *) arhdr;
  size_t length;
  const char *filename = lbasename (pathname);
  size_t maxlen = ar_maxnamelen (abfd);

  length = strlen (filename);

  if (length <= maxlen)
    memcpy (hdr->ar_name, filename, length);
  else
    {
      /* pathname: meet procrustes */
      memcpy (hdr->ar_name, filename, maxlen);
      length = maxlen;
    }

  if (length < maxlen)
    (hdr->ar_name)[length] = ar_padchar (abfd);
}

/* Store name into ar header.  Truncates the name to fit.
   1> strip pathname to be just the basename.
   2> if it's short enuf to fit, stuff it in.
   3> If it doesn't end with .o, truncate it to fit
   4> truncate it before the .o, append .o, stuff THAT in.  */

/* This is what gnu ar does.  It's better but incompatible with the
   bsd ar.  */

void
bfd_gnu_truncate_arname (bfd *abfd, const char *pathname, char *arhdr)
{
  struct ar_hdr *hdr = (struct ar_hdr *) arhdr;
  size_t length;
  const char *filename = lbasename (pathname);
  size_t maxlen = ar_maxnamelen (abfd);

  length = strlen (filename);

  if (length <= maxlen)
    memcpy (hdr->ar_name, filename, length);
  else
    {
      /* pathname: meet procrustes.  */
      memcpy (hdr->ar_name, filename, maxlen);
      if ((filename[length - 2] == '.') && (filename[length - 1] == 'o'))
	{
	  hdr->ar_name[maxlen - 2] = '.';
	  hdr->ar_name[maxlen - 1] = 'o';
	}
      length = maxlen;
    }

  if (length < 16)
    (hdr->ar_name)[length] = ar_padchar (abfd);
}

/* The BFD is open for write and has its format set to bfd_archive.  */

bfd_boolean
_bfd_write_archive_contents (bfd *arch)
{
  bfd *current;
  char *etable = NULL;
  bfd_size_type elength = 0;
  const char *ename = NULL;
  bfd_boolean makemap = bfd_has_map (arch);
  /* If no .o's, don't bother to make a map.  */
  bfd_boolean hasobjects = FALSE;
  bfd_size_type wrote;
  int tries;
  char *armag;

  /* Verify the viability of all entries; if any of them live in the
     filesystem (as opposed to living in an archive open for input)
     then construct a fresh ar_hdr for them.  */
  for (current = arch->archive_head;
       current != NULL;
       current = current->archive_next)
    {
      /* This check is checking the bfds for the objects we're reading
	 from (which are usually either an object file or archive on
	 disk), not the archive entries we're writing to.  We don't
	 actually create bfds for the archive members, we just copy
	 them byte-wise when we write out the archive.  */
      if (bfd_write_p (current))
	{
	  bfd_set_error (bfd_error_invalid_operation);
	  goto input_err;
	}
      if (!current->arelt_data)
	{
	  current->arelt_data =
	    bfd_ar_hdr_from_filesystem (arch, current->filename, current);
	  if (!current->arelt_data)
	    goto input_err;

	  /* Put in the file name.  */
	  BFD_SEND (arch, _bfd_truncate_arname,
		    (arch, current->filename, (char *) arch_hdr (current)));
	}

      if (makemap && ! hasobjects)
	{			/* Don't bother if we won't make a map!  */
	  if ((bfd_check_format (current, bfd_object)))
	    hasobjects = TRUE;
	}
    }

  if (!BFD_SEND (arch, _bfd_construct_extended_name_table,
		 (arch, &etable, &elength, &ename)))
    return FALSE;

  if (bfd_seek (arch, (file_ptr) 0, SEEK_SET) != 0)
    return FALSE;
  armag = ARMAG;
  if (bfd_is_thin_archive (arch))
    armag = ARMAGT;
  wrote = bfd_bwrite (armag, SARMAG, arch);
  if (wrote != SARMAG)
    return FALSE;

  if (makemap && hasobjects)
    {
      if (! _bfd_compute_and_write_armap (arch, (unsigned int) elength))
	return FALSE;
    }

  if (elength != 0)
    {
      struct ar_hdr hdr;

      memset (&hdr, ' ', sizeof (struct ar_hdr));
      memcpy (hdr.ar_name, ename, strlen (ename));
      /* Round size up to even number in archive header.  */
      if (!_bfd_ar_sizepad (hdr.ar_size, sizeof (hdr.ar_size),
			    (elength + 1) & ~(bfd_size_type) 1))
	return FALSE;
      memcpy (hdr.ar_fmag, ARFMAG, 2);
      if ((bfd_bwrite (&hdr, sizeof (struct ar_hdr), arch)
	   != sizeof (struct ar_hdr))
	  || bfd_bwrite (etable, elength, arch) != elength)
	return FALSE;
      if ((elength % 2) == 1)
	{
	  if (bfd_bwrite (&ARFMAG[1], 1, arch) != 1)
	    return FALSE;
	}
    }

  for (current = arch->archive_head;
       current != NULL;
       current = current->archive_next)
    {
      char buffer[DEFAULT_BUFFERSIZE];
      bfd_size_type remaining = arelt_size (current);

      /* Write ar header.  */
      if (!_bfd_write_ar_hdr (arch, current))
	return FALSE;
      if (bfd_is_thin_archive (arch))
	continue;
      if (bfd_seek (current, (file_ptr) 0, SEEK_SET) != 0)
	goto input_err;

      while (remaining)
	{
	  unsigned int amt = DEFAULT_BUFFERSIZE;

	  if (amt > remaining)
	    amt = remaining;
	  errno = 0;
	  if (bfd_bread (buffer, amt, current) != amt)
	    {
	      if (bfd_get_error () != bfd_error_system_call)
		bfd_set_error (bfd_error_file_truncated);
	      goto input_err;
	    }
	  if (bfd_bwrite (buffer, amt, arch) != amt)
	    return FALSE;
	  remaining -= amt;
	}

      if ((arelt_size (current) % 2) == 1)
	{
	  if (bfd_bwrite (&ARFMAG[1], 1, arch) != 1)
	    return FALSE;
	}
    }

  if (makemap && hasobjects)
    {
      /* Verify the timestamp in the archive file.  If it would not be
	 accepted by the linker, rewrite it until it would be.  If
	 anything odd happens, break out and just return.  (The
	 Berkeley linker checks the timestamp and refuses to read the
	 table-of-contents if it is >60 seconds less than the file's
	 modified-time.  That painful hack requires this painful hack.  */
      tries = 1;
      do
	{
	  if (bfd_update_armap_timestamp (arch))
	    break;
	  (*_bfd_error_handler)
	    (_("Warning: writing archive was slow: rewriting timestamp\n"));
	}
      while (++tries < 6);
    }

  return TRUE;

 input_err:
  bfd_set_error (bfd_error_on_input, current, bfd_get_error ());
  return FALSE;
}

/* Note that the namidx for the first symbol is 0.  */

bfd_boolean
_bfd_compute_and_write_armap (bfd *arch, unsigned int elength)
{
  char *first_name = NULL;
  bfd *current;
  file_ptr elt_no = 0;
  struct orl *map = NULL;
  unsigned int orl_max = 1024;		/* Fine initial default.  */
  unsigned int orl_count = 0;
  int stridx = 0;
  asymbol **syms = NULL;
  long syms_max = 0;
  bfd_boolean ret;
  bfd_size_type amt;

  /* Dunno if this is the best place for this info...  */
  if (elength != 0)
    elength += sizeof (struct ar_hdr);
  elength += elength % 2;

  amt = orl_max * sizeof (struct orl);
  map = (struct orl *) bfd_malloc (amt);
  if (map == NULL)
    goto error_return;

  /* We put the symbol names on the arch objalloc, and then discard
     them when done.  */
  first_name = (char *) bfd_alloc (arch, 1);
  if (first_name == NULL)
    goto error_return;

  /* Drop all the files called __.SYMDEF, we're going to make our own.  */
  while (arch->archive_head
	 && strcmp (arch->archive_head->filename, "__.SYMDEF") == 0)
    arch->archive_head = arch->archive_head->archive_next;

  /* Map over each element.  */
  for (current = arch->archive_head;
       current != NULL;
       current = current->archive_next, elt_no++)
    {
      if (bfd_check_format (current, bfd_object)
	  && (bfd_get_file_flags (current) & HAS_SYMS) != 0)
	{
	  long storage;
	  long symcount;
	  long src_count;

	  storage = bfd_get_symtab_upper_bound (current);
	  if (storage < 0)
	    goto error_return;

	  if (storage != 0)
	    {
	      if (storage > syms_max)
		{
		  if (syms_max > 0)
		    free (syms);
		  syms_max = storage;
		  syms = (asymbol **) bfd_malloc (syms_max);
		  if (syms == NULL)
		    goto error_return;
		}
	      symcount = bfd_canonicalize_symtab (current, syms);
	      if (symcount < 0)
		goto error_return;

	      /* Now map over all the symbols, picking out the ones we
		 want.  */
	      for (src_count = 0; src_count < symcount; src_count++)
		{
		  flagword flags = (syms[src_count])->flags;
		  asection *sec = syms[src_count]->section;

		  if (((flags & (BSF_GLOBAL
				 | BSF_WEAK
				 | BSF_INDIRECT
				 | BSF_GNU_UNIQUE)) != 0
		       || bfd_is_com_section (sec))
		      && ! bfd_is_und_section (sec))
		    {
		      bfd_size_type namelen;
		      struct orl *new_map;

		      /* This symbol will go into the archive header.  */
		      if (orl_count == orl_max)
			{
			  orl_max *= 2;
			  amt = orl_max * sizeof (struct orl);
			  new_map = (struct orl *) bfd_realloc (map, amt);
			  if (new_map == NULL)
			    goto error_return;

			  map = new_map;
			}

		      if (strcmp (syms[src_count]->name, "__gnu_lto_slim") == 0)
			(*_bfd_error_handler)
			  (_("%s: plugin needed to handle lto object"),
			   bfd_get_filename (current));
		      namelen = strlen (syms[src_count]->name);
		      amt = sizeof (char *);
		      map[orl_count].name = (char **) bfd_alloc (arch, amt);
		      if (map[orl_count].name == NULL)
			goto error_return;
		      *(map[orl_count].name) = (char *) bfd_alloc (arch,
								   namelen + 1);
		      if (*(map[orl_count].name) == NULL)
			goto error_return;
		      strcpy (*(map[orl_count].name), syms[src_count]->name);
		      map[orl_count].u.abfd = current;
		      map[orl_count].namidx = stridx;

		      stridx += namelen + 1;
		      ++orl_count;
		    }
		}
	    }

	  /* Now ask the BFD to free up any cached information, so we
	     don't fill all of memory with symbol tables.  */
	  if (! bfd_free_cached_info (current))
	    goto error_return;
	}
    }

  /* OK, now we have collected all the data, let's write them out.  */
  ret = BFD_SEND (arch, write_armap,
		  (arch, elength, map, orl_count, stridx));

  if (syms_max > 0)
    free (syms);
  if (map != NULL)
    free (map);
  if (first_name != NULL)
    bfd_release (arch, first_name);

  return ret;

 error_return:
  if (syms_max > 0)
    free (syms);
  if (map != NULL)
    free (map);
  if (first_name != NULL)
    bfd_release (arch, first_name);

  return FALSE;
}

bfd_boolean
bsd_write_armap (bfd *arch,
		 unsigned int elength,
		 struct orl *map,
		 unsigned int orl_count,
		 int stridx)
{
  int padit = stridx & 1;
  unsigned int ranlibsize = orl_count * BSD_SYMDEF_SIZE;
  unsigned int stringsize = stridx + padit;
  /* Include 8 bytes to store ranlibsize and stringsize in output.  */
  unsigned int mapsize = ranlibsize + stringsize + 8;
  file_ptr firstreal;
  bfd *current = arch->archive_head;
  bfd *last_elt = current;	/* Last element arch seen.  */
  bfd_byte temp[4];
  unsigned int count;
  struct ar_hdr hdr;
  long uid, gid;

  firstreal = mapsize + elength + sizeof (struct ar_hdr) + SARMAG;

  /* If deterministic, we use 0 as the timestamp in the map.
     Some linkers may require that the archive filesystem modification
     time is less than (or near to) the archive map timestamp.  Those
     linkers should not be used with deterministic mode.  (GNU ld and
     Gold do not have this restriction.)  */
  bfd_ardata (arch)->armap_timestamp = 0;
  uid = 0;
  gid = 0;
  if ((arch->flags & BFD_DETERMINISTIC_OUTPUT) == 0)
    {
      struct stat statbuf;

      if (stat (arch->filename, &statbuf) == 0)
	bfd_ardata (arch)->armap_timestamp = (statbuf.st_mtime
					      + ARMAP_TIME_OFFSET);
      uid = getuid();
      gid = getgid();
    }

  memset (&hdr, ' ', sizeof (struct ar_hdr));
  memcpy (hdr.ar_name, RANLIBMAG, strlen (RANLIBMAG));
  bfd_ardata (arch)->armap_datepos = (SARMAG
				      + offsetof (struct ar_hdr, ar_date[0]));
  _bfd_ar_spacepad (hdr.ar_date, sizeof (hdr.ar_date), "%ld",
		    bfd_ardata (arch)->armap_timestamp);
  _bfd_ar_spacepad (hdr.ar_uid, sizeof (hdr.ar_uid), "%ld", uid);
  _bfd_ar_spacepad (hdr.ar_gid, sizeof (hdr.ar_gid), "%ld", gid);
  if (!_bfd_ar_sizepad (hdr.ar_size, sizeof (hdr.ar_size), mapsize))
    return FALSE;
  memcpy (hdr.ar_fmag, ARFMAG, 2);
  if (bfd_bwrite (&hdr, sizeof (struct ar_hdr), arch)
      != sizeof (struct ar_hdr))
    return FALSE;
  H_PUT_32 (arch, ranlibsize, temp);
  if (bfd_bwrite (temp, sizeof (temp), arch) != sizeof (temp))
    return FALSE;

  for (count = 0; count < orl_count; count++)
    {
      unsigned int offset;
      bfd_byte buf[BSD_SYMDEF_SIZE];

      if (map[count].u.abfd != last_elt)
	{
	  do
	    {
	      struct areltdata *ared = arch_eltdata (current);

	      firstreal += (ared->parsed_size + ared->extra_size
			    + sizeof (struct ar_hdr));
	      firstreal += firstreal % 2;
	      current = current->archive_next;
	    }
	  while (current != map[count].u.abfd);
	}

      /* The archive file format only has 4 bytes to store the offset
	 of the member.  Check to make sure that firstreal has not grown
	 too big.  */
      offset = (unsigned int) firstreal;
      if (firstreal != (file_ptr) offset)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return FALSE;
	}

      last_elt = current;
      H_PUT_32 (arch, map[count].namidx, buf);
      H_PUT_32 (arch, firstreal, buf + BSD_SYMDEF_OFFSET_SIZE);
      if (bfd_bwrite (buf, BSD_SYMDEF_SIZE, arch)
	  != BSD_SYMDEF_SIZE)
	return FALSE;
    }

  /* Now write the strings themselves.  */
  H_PUT_32 (arch, stringsize, temp);
  if (bfd_bwrite (temp, sizeof (temp), arch) != sizeof (temp))
    return FALSE;
  for (count = 0; count < orl_count; count++)
    {
      size_t len = strlen (*map[count].name) + 1;

      if (bfd_bwrite (*map[count].name, len, arch) != len)
	return FALSE;
    }

  /* The spec sez this should be a newline.  But in order to be
     bug-compatible for sun's ar we use a null.  */
  if (padit)
    {
      if (bfd_bwrite ("", 1, arch) != 1)
	return FALSE;
    }

  return TRUE;
}

/* At the end of archive file handling, update the timestamp in the
   file, so the linker will accept it.

   Return TRUE if the timestamp was OK, or an unusual problem happened.
   Return FALSE if we updated the timestamp.  */

bfd_boolean
_bfd_archive_bsd_update_armap_timestamp (bfd *arch)
{
  struct stat archstat;
  struct ar_hdr hdr;

  /* If creating deterministic archives, just leave the timestamp as-is.  */
  if ((arch->flags & BFD_DETERMINISTIC_OUTPUT) != 0)
    return TRUE;

  /* Flush writes, get last-write timestamp from file, and compare it
     to the timestamp IN the file.  */
  bfd_flush (arch);
  if (bfd_stat (arch, &archstat) == -1)
    {
      bfd_perror (_("Reading archive file mod timestamp"));

      /* Can't read mod time for some reason.  */
      return TRUE;
    }
  if (((long) archstat.st_mtime) <= bfd_ardata (arch)->armap_timestamp)
    /* OK by the linker's rules.  */
    return TRUE;

  /* Update the timestamp.  */
  bfd_ardata (arch)->armap_timestamp = archstat.st_mtime + ARMAP_TIME_OFFSET;

  /* Prepare an ASCII version suitable for writing.  */
  memset (hdr.ar_date, ' ', sizeof (hdr.ar_date));
  _bfd_ar_spacepad (hdr.ar_date, sizeof (hdr.ar_date), "%ld",
		    bfd_ardata (arch)->armap_timestamp);

  /* Write it into the file.  */
  bfd_ardata (arch)->armap_datepos = (SARMAG
				      + offsetof (struct ar_hdr, ar_date[0]));
  if (bfd_seek (arch, bfd_ardata (arch)->armap_datepos, SEEK_SET) != 0
      || (bfd_bwrite (hdr.ar_date, sizeof (hdr.ar_date), arch)
	  != sizeof (hdr.ar_date)))
    {
      bfd_perror (_("Writing updated armap timestamp"));

      /* Some error while writing.  */
      return TRUE;
    }

  /* We updated the timestamp successfully.  */
  return FALSE;
}

/* A coff armap looks like :
   lARMAG
   struct ar_hdr with name = '/'
   number of symbols
   offset of file for symbol 0
   offset of file for symbol 1

   offset of file for symbol n-1
   symbol name 0
   symbol name 1

   symbol name n-1  */

bfd_boolean
coff_write_armap (bfd *arch,
		  unsigned int elength,
		  struct orl *map,
		  unsigned int symbol_count,
		  int stridx)
{
  /* The size of the ranlib is the number of exported symbols in the
     archive * the number of bytes in an int, + an int for the count.  */
  unsigned int ranlibsize = (symbol_count * 4) + 4;
  unsigned int stringsize = stridx;
  unsigned int mapsize = stringsize + ranlibsize;
  file_ptr archive_member_file_ptr;
  bfd *current = arch->archive_head;
  unsigned int count;
  struct ar_hdr hdr;
  int padit = mapsize & 1;

  if (padit)
    mapsize++;

  /* Work out where the first object file will go in the archive.  */
  archive_member_file_ptr = (mapsize
			     + elength
			     + sizeof (struct ar_hdr)
			     + SARMAG);

  memset (&hdr, ' ', sizeof (struct ar_hdr));
  hdr.ar_name[0] = '/';
  if (!_bfd_ar_sizepad (hdr.ar_size, sizeof (hdr.ar_size), mapsize))
    return FALSE;
  _bfd_ar_spacepad (hdr.ar_date, sizeof (hdr.ar_date), "%ld",
		    ((arch->flags & BFD_DETERMINISTIC_OUTPUT) == 0
		     ? time (NULL) : 0));
  /* This, at least, is what Intel coff sets the values to.  */
  _bfd_ar_spacepad (hdr.ar_uid, sizeof (hdr.ar_uid), "%ld", 0);
  _bfd_ar_spacepad (hdr.ar_gid, sizeof (hdr.ar_gid), "%ld", 0);
  _bfd_ar_spacepad (hdr.ar_mode, sizeof (hdr.ar_mode), "%-7lo", 0);
  memcpy (hdr.ar_fmag, ARFMAG, 2);

  /* Write the ar header for this item and the number of symbols.  */
  if (bfd_bwrite (&hdr, sizeof (struct ar_hdr), arch)
      != sizeof (struct ar_hdr))
    return FALSE;

  if (!bfd_write_bigendian_4byte_int (arch, symbol_count))
    return FALSE;

  /* Two passes, first write the file offsets for each symbol -
     remembering that each offset is on a two byte boundary.  */

  /* Write out the file offset for the file associated with each
     symbol, and remember to keep the offsets padded out.  */

  current = arch->archive_head;
  count = 0;
  while (current != NULL && count < symbol_count)
    {
      /* For each symbol which is used defined in this object, write
	 out the object file's address in the archive.  */

      while (count < symbol_count && map[count].u.abfd == current)
	{
	  unsigned int offset = (unsigned int) archive_member_file_ptr;

	  /* Catch an attempt to grow an archive past its 4Gb limit.  */
	  if (archive_member_file_ptr != (file_ptr) offset)
	    {
	      bfd_set_error (bfd_error_file_truncated);
	      return FALSE;
	    }
	  if (!bfd_write_bigendian_4byte_int (arch, offset))
	    return FALSE;
	  count++;
	}
      archive_member_file_ptr += sizeof (struct ar_hdr);
      if (! bfd_is_thin_archive (arch))
	{
	  /* Add size of this archive entry.  */
	  archive_member_file_ptr += arelt_size (current);
	  /* Remember about the even alignment.  */
	  archive_member_file_ptr += archive_member_file_ptr % 2;
	}
      current = current->archive_next;
    }

  /* Now write the strings themselves.  */
  for (count = 0; count < symbol_count; count++)
    {
      size_t len = strlen (*map[count].name) + 1;

      if (bfd_bwrite (*map[count].name, len, arch) != len)
	return FALSE;
    }

  /* The spec sez this should be a newline.  But in order to be
     bug-compatible for arc960 we use a null.  */
  if (padit)
    {
      if (bfd_bwrite ("", 1, arch) != 1)
	return FALSE;
    }

  return TRUE;
}

static int
archive_close_worker (void **slot, void *inf ATTRIBUTE_UNUSED)
{
  struct ar_cache *ent = (struct ar_cache *) *slot;

  bfd_close_all_done (ent->arbfd);
  return 1;
}

bfd_boolean
_bfd_archive_close_and_cleanup (bfd *abfd)
{
  if (bfd_read_p (abfd) && abfd->format == bfd_archive)
    {
      bfd *nbfd;
      bfd *next;
      htab_t htab;

      /* Close nested archives (if this bfd is a thin archive).  */
      for (nbfd = abfd->nested_archives; nbfd; nbfd = next)
	{
	  next = nbfd->archive_next;
	  bfd_close (nbfd);
	}

      htab = bfd_ardata (abfd)->cache;
      if (htab)
	{
	  htab_traverse_noresize (htab, archive_close_worker, NULL);
	  htab_delete (htab);
	  bfd_ardata (abfd)->cache = NULL;
	}
    }
  if (arch_eltdata (abfd) != NULL)
    {
      struct areltdata *ared = arch_eltdata (abfd);
      htab_t htab = (htab_t) ared->parent_cache;

      if (htab)
	{
	  struct ar_cache ent;
	  void **slot;

	  ent.ptr = ared->key;
	  slot = htab_find_slot (htab, &ent, NO_INSERT);
	  if (slot != NULL)
	    {
	      BFD_ASSERT (((struct ar_cache *) *slot)->arbfd == abfd);
	      htab_clear_slot (htab, slot);
	    }
	}
    }
  if (abfd->is_linker_output)
    (*abfd->link.hash->hash_table_free) (abfd);

  return TRUE;
}

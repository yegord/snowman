/* Compressed section support (intended for debug sections).
   Copyright (C) 2008-2014 Free Software Foundation, Inc.

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
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
#include "safe-ctype.h"

#ifdef HAVE_ZLIB_H
static bfd_boolean
decompress_contents (bfd_byte *compressed_buffer,
		     bfd_size_type compressed_size,
		     bfd_byte *uncompressed_buffer,
		     bfd_size_type uncompressed_size)
{
  z_stream strm;
  int rc;

  /* It is possible the section consists of several compressed
     buffers concatenated together, so we uncompress in a loop.  */
  strm.zalloc = NULL;
  strm.zfree = NULL;
  strm.opaque = NULL;
  strm.avail_in = compressed_size - 12;
  strm.next_in = (Bytef*) compressed_buffer + 12;
  strm.avail_out = uncompressed_size;

  BFD_ASSERT (Z_OK == 0);
  rc = inflateInit (&strm);
  while (strm.avail_in > 0 && strm.avail_out > 0)
    {
      if (rc != Z_OK)
	break;
      strm.next_out = ((Bytef*) uncompressed_buffer
                       + (uncompressed_size - strm.avail_out));
      rc = inflate (&strm, Z_FINISH);
      if (rc != Z_STREAM_END)
	break;
      rc = inflateReset (&strm);
    }
  rc |= inflateEnd (&strm);
  return rc == Z_OK && strm.avail_out == 0;
}
#endif

/*
FUNCTION
	bfd_compress_section_contents

SYNOPSIS
	bfd_boolean bfd_compress_section_contents
	  (bfd *abfd, asection *section, bfd_byte *uncompressed_buffer,
	   bfd_size_type uncompressed_size);

DESCRIPTION

	Compress data of the size specified in @var{uncompressed_size}
	and pointed to by @var{uncompressed_buffer} using zlib and store
	as the contents field.  This function assumes the contents
	field was allocated using bfd_malloc() or equivalent.  If zlib
	is not installed on this machine, the input is unmodified.

	Return @code{TRUE} if the full section contents is compressed
	successfully.
*/

bfd_boolean
bfd_compress_section_contents (bfd *abfd ATTRIBUTE_UNUSED,
			       sec_ptr sec ATTRIBUTE_UNUSED,
			       bfd_byte *uncompressed_buffer ATTRIBUTE_UNUSED,
			       bfd_size_type uncompressed_size ATTRIBUTE_UNUSED)
{
#ifndef HAVE_ZLIB_H
  bfd_set_error (bfd_error_invalid_operation);
  return FALSE;
#else
  uLong compressed_size;
  bfd_byte *compressed_buffer;

  compressed_size = compressBound (uncompressed_size) + 12;
  compressed_buffer = (bfd_byte *) bfd_malloc (compressed_size);

  if (compressed_buffer == NULL)
    return FALSE;

  if (compress ((Bytef*) compressed_buffer + 12,
		&compressed_size,
		(const Bytef*) uncompressed_buffer,
		uncompressed_size) != Z_OK)
    {
      free (compressed_buffer);
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  /* Write the zlib header.  In this case, it should be "ZLIB" followed
     by the uncompressed section size, 8 bytes in big-endian order.  */
  memcpy (compressed_buffer, "ZLIB", 4);
  compressed_buffer[11] = uncompressed_size; uncompressed_size >>= 8;
  compressed_buffer[10] = uncompressed_size; uncompressed_size >>= 8;
  compressed_buffer[9] = uncompressed_size; uncompressed_size >>= 8;
  compressed_buffer[8] = uncompressed_size; uncompressed_size >>= 8;
  compressed_buffer[7] = uncompressed_size; uncompressed_size >>= 8;
  compressed_buffer[6] = uncompressed_size; uncompressed_size >>= 8;
  compressed_buffer[5] = uncompressed_size; uncompressed_size >>= 8;
  compressed_buffer[4] = uncompressed_size;
  compressed_size += 12;

  /* Free the uncompressed contents if we compress in place.  */
  if (uncompressed_buffer == sec->contents)
    free (uncompressed_buffer);

  sec->contents = compressed_buffer;
  sec->size = compressed_size;
  sec->compress_status = COMPRESS_SECTION_DONE;

  return TRUE;
#endif  /* HAVE_ZLIB_H */
}

/*
FUNCTION
	bfd_get_full_section_contents

SYNOPSIS
	bfd_boolean bfd_get_full_section_contents
	  (bfd *abfd, asection *section, bfd_byte **ptr);

DESCRIPTION
	Read all data from @var{section} in BFD @var{abfd}, decompress
	if needed, and store in @var{*ptr}.  If @var{*ptr} is NULL,
	return @var{*ptr} with memory malloc'd by this function.

	Return @code{TRUE} if the full section contents is retrieved
	successfully.
*/

bfd_boolean
bfd_get_full_section_contents (bfd *abfd, sec_ptr sec, bfd_byte **ptr)
{
  bfd_size_type sz;
  bfd_byte *p = *ptr;
#ifdef HAVE_ZLIB_H
  bfd_boolean ret;
  bfd_size_type save_size;
  bfd_size_type save_rawsize;
  bfd_byte *compressed_buffer;
#endif

  if (abfd->direction != write_direction && sec->rawsize != 0)
    sz = sec->rawsize;
  else
    sz = sec->size;
  if (sz == 0)
    return TRUE;

  switch (sec->compress_status)
    {
    case COMPRESS_SECTION_NONE:
      if (p == NULL)
	{
	  p = (bfd_byte *) bfd_malloc (sz);
	  if (p == NULL)
	    return FALSE;
	}
      if (!bfd_get_section_contents (abfd, sec, p, 0, sz))
	{
	  if (*ptr != p)
	    free (p);
	  return FALSE;
	}
      *ptr = p;
      return TRUE;

    case DECOMPRESS_SECTION_SIZED:
#ifndef HAVE_ZLIB_H
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
#else
      /* Read in the full compressed section contents.  */
      compressed_buffer = (bfd_byte *) bfd_malloc (sec->compressed_size);
      if (compressed_buffer == NULL)
	return FALSE;
      save_rawsize = sec->rawsize;
      save_size = sec->size;
      /* Clear rawsize, set size to compressed size and set compress_status
	 to COMPRESS_SECTION_NONE.  If the compressed size is bigger than
	 the uncompressed size, bfd_get_section_contents will fail.  */
      sec->rawsize = 0;
      sec->size = sec->compressed_size;
      sec->compress_status = COMPRESS_SECTION_NONE;
      ret = bfd_get_section_contents (abfd, sec, compressed_buffer,
				      0, sec->compressed_size);
      /* Restore rawsize and size.  */
      sec->rawsize = save_rawsize;
      sec->size = save_size;
      sec->compress_status = DECOMPRESS_SECTION_SIZED;
      if (!ret)
	goto fail_compressed;

      if (p == NULL)
	p = (bfd_byte *) bfd_malloc (sz);
      if (p == NULL)
	goto fail_compressed;

      if (!decompress_contents (compressed_buffer, sec->compressed_size, p, sz))
	{
	  bfd_set_error (bfd_error_bad_value);
	  if (p != *ptr)
	    free (p);
	fail_compressed:
	  free (compressed_buffer);
	  return FALSE;
	}

      free (compressed_buffer);
      *ptr = p;
      return TRUE;
#endif

    case COMPRESS_SECTION_DONE:
      if (sec->contents == NULL)
	return FALSE;
      if (p == NULL)
	{
	  p = (bfd_byte *) bfd_malloc (sz);
	  if (p == NULL)
	    return FALSE;
	  *ptr = p;
	}
      /* PR 17512; file: 5bc29788.  */
      if (p != sec->contents)
	memcpy (p, sec->contents, sz);
      return TRUE;

    default:
      abort ();
    }
}

/*
FUNCTION
	bfd_cache_section_contents

SYNOPSIS
	void bfd_cache_section_contents
	  (asection *sec, void *contents);

DESCRIPTION
	Stash @var(contents) so any following reads of @var(sec) do
	not need to decompress again.
*/

void
bfd_cache_section_contents (asection *sec, void *contents)
{
  if (sec->compress_status == DECOMPRESS_SECTION_SIZED)
    sec->compress_status = COMPRESS_SECTION_DONE;
  sec->contents = contents;
  sec->flags |= SEC_IN_MEMORY;
}


/*
FUNCTION
	bfd_is_section_compressed

SYNOPSIS
	bfd_boolean bfd_is_section_compressed
	  (bfd *abfd, asection *section);

DESCRIPTION
	Return @code{TRUE} if @var{section} is compressed.
*/

bfd_boolean
bfd_is_section_compressed (bfd *abfd, sec_ptr sec)
{
  bfd_byte compressed_buffer [12];
  unsigned int saved = sec->compress_status;
  bfd_boolean compressed;

  /* Don't decompress the section.  */
  sec->compress_status = COMPRESS_SECTION_NONE;

  /* Read the zlib header.  In this case, it should be "ZLIB" followed
     by the uncompressed section size, 8 bytes in big-endian order.  */
  compressed = (bfd_get_section_contents (abfd, sec, compressed_buffer, 0, 12)
		&& CONST_STRNEQ ((char*) compressed_buffer, "ZLIB"));

  /* Check for the pathalogical case of a debug string section that
     contains the string ZLIB.... as the first entry.  We assume that
     no uncompressed .debug_str section would ever be big enough to
     have the first byte of its (big-endian) size be non-zero.  */
  if (compressed
      && strcmp (sec->name, ".debug_str") == 0
      && ISPRINT (compressed_buffer[4]))
    compressed = FALSE;

  /* Restore compress_status.  */
  sec->compress_status = saved;
  return compressed;
}

/*
FUNCTION
	bfd_init_section_decompress_status

SYNOPSIS
	bfd_boolean bfd_init_section_decompress_status
	  (bfd *abfd, asection *section);

DESCRIPTION
	Record compressed section size, update section size with
	decompressed size and set compress_status to
	DECOMPRESS_SECTION_SIZED.

	Return @code{FALSE} if the section is not a valid compressed
	section or zlib is not installed on this machine.  Otherwise,
	return @code{TRUE}.
*/

bfd_boolean
bfd_init_section_decompress_status (bfd *abfd ATTRIBUTE_UNUSED,
				    sec_ptr sec ATTRIBUTE_UNUSED)
{
#ifndef HAVE_ZLIB_H
  bfd_set_error (bfd_error_invalid_operation);
  return FALSE;
#else
  bfd_byte compressed_buffer [12];
  bfd_size_type uncompressed_size;

  if (sec->rawsize != 0
      || sec->contents != NULL
      || sec->compress_status != COMPRESS_SECTION_NONE
      || !bfd_get_section_contents (abfd, sec, compressed_buffer, 0, 12))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  /* Read the zlib header.  In this case, it should be "ZLIB" followed
     by the uncompressed section size, 8 bytes in big-endian order.  */
  if (! CONST_STRNEQ ((char*) compressed_buffer, "ZLIB"))
    {
      bfd_set_error (bfd_error_wrong_format);
      return FALSE;
    }

  uncompressed_size = compressed_buffer[4]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[5]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[6]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[7]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[8]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[9]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[10]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[11];

  sec->compressed_size = sec->size;
  sec->size = uncompressed_size;
  sec->compress_status = DECOMPRESS_SECTION_SIZED;

  return TRUE;
#endif
}

/*
FUNCTION
	bfd_init_section_compress_status

SYNOPSIS
	bfd_boolean bfd_init_section_compress_status
	  (bfd *abfd, asection *section);

DESCRIPTION
	If open for read, compress section, update section size with
	compressed size and set compress_status to COMPRESS_SECTION_DONE.

	Return @code{FALSE} if the section is not a valid compressed
	section or zlib is not installed on this machine.  Otherwise,
	return @code{TRUE}.
*/

bfd_boolean
bfd_init_section_compress_status (bfd *abfd ATTRIBUTE_UNUSED,
				  sec_ptr sec ATTRIBUTE_UNUSED)
{
#ifndef HAVE_ZLIB_H
  bfd_set_error (bfd_error_invalid_operation);
  return FALSE;
#else
  bfd_size_type uncompressed_size;
  bfd_byte *uncompressed_buffer;
  bfd_boolean ret;

  /* Error if not opened for read.  */
  if (abfd->direction != read_direction
      || sec->size == 0
      || sec->rawsize != 0
      || sec->contents != NULL
      || sec->compress_status != COMPRESS_SECTION_NONE)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  /* Read in the full section contents and compress it.  */
  uncompressed_size = sec->size;
  uncompressed_buffer = (bfd_byte *) bfd_malloc (uncompressed_size);
  if (!bfd_get_section_contents (abfd, sec, uncompressed_buffer,
				 0, uncompressed_size))
    ret = FALSE;
  else
    ret = bfd_compress_section_contents (abfd, sec,
					 uncompressed_buffer,
					 uncompressed_size);

  free (uncompressed_buffer);
  return ret;
#endif
}

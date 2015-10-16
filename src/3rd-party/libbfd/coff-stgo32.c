/* BFD back-end for Intel 386 COFF files (DJGPP variant with a stub).
   Copyright (C) 1997-2014 Free Software Foundation, Inc.
   Written by Robert Hoehne.

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

/* This file handles now also stubbed coff images. The stub is a small
   DOS executable program before the coff image to load it in memory
   and execute it. This is needed, because DOS cannot run coff files.

   All the functions below are called by the corresponding functions
   from coffswap.h.
   The only thing what they do is to adjust the information stored in
   the COFF file which are offset into the file.
   This is needed, because DJGPP uses a very special way to load and run
   the coff image. It loads the image in memory and assumes then, that the
   image had no stub by using the filepointers as pointers in the coff
   image and NOT in the file.

   To be compatible with any existing executables I have fixed this
   here and NOT in the DJGPP startup code.  */

#define TARGET_SYM		i386_coff_go32stubbed_vec
#define TARGET_NAME		"coff-go32-exe"
#define TARGET_UNDERSCORE	'_'
#define COFF_GO32_EXE
#define COFF_LONG_SECTION_NAMES
#define COFF_SUPPORT_GNU_LINKONCE
#define COFF_LONG_FILENAMES

#define COFF_SECTION_ALIGNMENT_ENTRIES \
{ COFF_SECTION_NAME_EXACT_MATCH (".data"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_EXACT_MATCH (".text"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".debug"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.wi"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }

#include "sysdep.h"
#include "bfd.h"

/* All that ..._PRE and ...POST functions are called from the corresponding
   coff_swap... functions. The ...PRE functions are called at the beginning
   of the function and the ...POST functions at the end of the swap routines.  */

static void
adjust_filehdr_in_post  (bfd *, void *, void *);
static void
adjust_filehdr_out_pre  (bfd *, void *, void *);
static void
adjust_filehdr_out_post  (bfd *, void *, void *);
static void
adjust_scnhdr_in_post  (bfd *, void *, void *);
static void
adjust_scnhdr_out_pre  (bfd *, void *, void *);
static void
adjust_scnhdr_out_post (bfd *, void *, void *);
static void
adjust_aux_in_post (bfd *, void *, int, int, int, int, void *);
static void
adjust_aux_out_pre (bfd *, void *, int, int, int, int, void *);
static void
adjust_aux_out_post (bfd *, void *, int, int, int, int, void *);
static void
create_go32_stub (bfd *);

#define COFF_ADJUST_FILEHDR_IN_POST adjust_filehdr_in_post
#define COFF_ADJUST_FILEHDR_OUT_PRE adjust_filehdr_out_pre
#define COFF_ADJUST_FILEHDR_OUT_POST adjust_filehdr_out_post

#define COFF_ADJUST_SCNHDR_IN_POST adjust_scnhdr_in_post
#define COFF_ADJUST_SCNHDR_OUT_PRE adjust_scnhdr_out_pre
#define COFF_ADJUST_SCNHDR_OUT_POST adjust_scnhdr_out_post

#define COFF_ADJUST_AUX_IN_POST adjust_aux_in_post
#define COFF_ADJUST_AUX_OUT_PRE adjust_aux_out_pre
#define COFF_ADJUST_AUX_OUT_POST adjust_aux_out_post

static const bfd_target *go32_check_format (bfd *);

#define COFF_CHECK_FORMAT go32_check_format

static bfd_boolean
  go32_stubbed_coff_bfd_copy_private_bfd_data (bfd *, bfd *);

#define coff_bfd_copy_private_bfd_data go32_stubbed_coff_bfd_copy_private_bfd_data

#include "coff-i386.c"

/* This macro is used, because I cannot assume the endianness of the
   host system.  */
#define _H(index) (H_GET_16 (abfd, (header + index * 2)))

/* These bytes are a 2048-byte DOS executable, which loads the COFF
   image into memory and then runs it. It is called 'stub'.  */

static const unsigned char stub_bytes[GO32_STUBSIZE] =
{
#include "go32stub.h"
};

/*
   I have not commented each swap function below, because the
   technique is in any function the same. For the ...in function,
   all the pointers are adjusted by adding GO32_STUBSIZE and for the
   ...out function, it is subtracted first and after calling the
   standard swap function it is reset to the old value.  */

/* This macro is used for adjusting the filepointers, which
   is done only, if the pointer is nonzero.  */

#define ADJUST_VAL(val,diff) \
  if (val != 0) val += diff

static void
adjust_filehdr_in_post  (bfd *  abfd ATTRIBUTE_UNUSED,
			 void * src,
			 void * dst)
{
  FILHDR *filehdr_src = (FILHDR *) src;
  struct internal_filehdr *filehdr_dst = (struct internal_filehdr *) dst;

  ADJUST_VAL (filehdr_dst->f_symptr, GO32_STUBSIZE);

  /* Save now the stub to be used later.  Put the stub data to FILEHDR_DST
     first as coff_data (abfd) still does not exist.  It may not even be ever
     created as we are just checking the file format of ABFD.  */
  memcpy (filehdr_dst->go32stub, filehdr_src->stub, GO32_STUBSIZE);
  filehdr_dst->f_flags |= F_GO32STUB;
}

static void
adjust_filehdr_out_pre  (bfd * abfd, void * in, void * out)
{
  struct internal_filehdr *filehdr_in = (struct internal_filehdr *) in;
  FILHDR *filehdr_out = (FILHDR *) out;

  /* Generate the stub.  */
  create_go32_stub (abfd);

  /* Copy the stub to the file header.  */
  if (coff_data (abfd)->go32stub != NULL)
    memcpy (filehdr_out->stub, coff_data (abfd)->go32stub, GO32_STUBSIZE);
  else
    /* Use the default.  */
    memcpy (filehdr_out->stub, stub_bytes, GO32_STUBSIZE);

  ADJUST_VAL (filehdr_in->f_symptr, -GO32_STUBSIZE);
}

static void
adjust_filehdr_out_post  (bfd *  abfd ATTRIBUTE_UNUSED,
			  void * in,
			  void * out ATTRIBUTE_UNUSED)
{
  struct internal_filehdr *filehdr_in = (struct internal_filehdr *) in;
  /* Undo the above change.  */
  ADJUST_VAL (filehdr_in->f_symptr, GO32_STUBSIZE);
}

static void
adjust_scnhdr_in_post  (bfd *  abfd ATTRIBUTE_UNUSED,
			void * ext ATTRIBUTE_UNUSED,
			void * in)
{
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;

  ADJUST_VAL (scnhdr_int->s_scnptr, GO32_STUBSIZE);
  ADJUST_VAL (scnhdr_int->s_relptr, GO32_STUBSIZE);
  ADJUST_VAL (scnhdr_int->s_lnnoptr, GO32_STUBSIZE);
}

static void
adjust_scnhdr_out_pre  (bfd *  abfd ATTRIBUTE_UNUSED,
			void * in,
			void * out ATTRIBUTE_UNUSED)
{
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;

  ADJUST_VAL (scnhdr_int->s_scnptr, -GO32_STUBSIZE);
  ADJUST_VAL (scnhdr_int->s_relptr, -GO32_STUBSIZE);
  ADJUST_VAL (scnhdr_int->s_lnnoptr, -GO32_STUBSIZE);
}

static void
adjust_scnhdr_out_post (bfd *  abfd ATTRIBUTE_UNUSED,
			void * in,
			void * out ATTRIBUTE_UNUSED)
{
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;

  ADJUST_VAL (scnhdr_int->s_scnptr, GO32_STUBSIZE);
  ADJUST_VAL (scnhdr_int->s_relptr, GO32_STUBSIZE);
  ADJUST_VAL (scnhdr_int->s_lnnoptr, GO32_STUBSIZE);
}

static void
adjust_aux_in_post (bfd * abfd ATTRIBUTE_UNUSED,
		    void * ext1 ATTRIBUTE_UNUSED,
		    int type,
		    int in_class,
		    int indx ATTRIBUTE_UNUSED,
		    int numaux ATTRIBUTE_UNUSED,
		    void * in1)
{
  union internal_auxent *in = (union internal_auxent *) in1;

  if (in_class == C_BLOCK || in_class == C_FCN || ISFCN (type)
      || ISTAG (in_class))
    {
      ADJUST_VAL (in->x_sym.x_fcnary.x_fcn.x_lnnoptr, GO32_STUBSIZE);
    }
}

static void
adjust_aux_out_pre (bfd *abfd ATTRIBUTE_UNUSED,
		    void * inp,
		    int type,
		    int in_class,
		    int indx ATTRIBUTE_UNUSED,
		    int numaux ATTRIBUTE_UNUSED,
		    void * extp ATTRIBUTE_UNUSED)
{
  union internal_auxent *in = (union internal_auxent *) inp;

  if (in_class == C_BLOCK || in_class == C_FCN || ISFCN (type)
      || ISTAG (in_class))
    {
      ADJUST_VAL (in->x_sym.x_fcnary.x_fcn.x_lnnoptr, -GO32_STUBSIZE);
    }
}

static void
adjust_aux_out_post (bfd *abfd ATTRIBUTE_UNUSED,
		     void * inp,
		     int type,
		     int in_class,
		     int indx ATTRIBUTE_UNUSED,
		     int numaux ATTRIBUTE_UNUSED,
		     void * extp ATTRIBUTE_UNUSED)
{
  union internal_auxent *in = (union internal_auxent *) inp;

  if (in_class == C_BLOCK || in_class == C_FCN || ISFCN (type)
      || ISTAG (in_class))
    {
      ADJUST_VAL (in->x_sym.x_fcnary.x_fcn.x_lnnoptr, GO32_STUBSIZE);
    }
}

/* That's the function, which creates the stub. There are
   different cases from where the stub is taken.
   At first the environment variable $(GO32STUB) is checked and then
   $(STUB) if it was not set.
   If it exists and points to a valid stub the stub is taken from
   that file. This file can be also a whole executable file, because
   the stub is computed from the exe information at the start of that
   file.

   If there was any error, the standard stub (compiled in this file)
   is taken.  */

static void
create_go32_stub (bfd *abfd)
{
  /* Do it only once.  */
  if (coff_data (abfd)->go32stub == NULL)
    {
      char *stub;
      struct stat st;
      int f;
      unsigned char header[10];
      char magic[8];
      unsigned long coff_start;
      long exe_start;

      /* Check at first the environment variable $(GO32STUB).  */
      stub = getenv ("GO32STUB");
      /* Now check the environment variable $(STUB).  */
      if (stub == NULL)
	stub = getenv ("STUB");
      if (stub == NULL)
	goto stub_end;
      if (stat (stub, &st) != 0)
	goto stub_end;
#ifdef O_BINARY
      f = open (stub, O_RDONLY | O_BINARY);
#else
      f = open (stub, O_RDONLY);
#endif
      if (f < 0)
	goto stub_end;
      if (read (f, &header, sizeof (header)) < 0)
	{
	  close (f);
	  goto stub_end;
	}
      if (_H (0) != 0x5a4d)	/* It is not an exe file.  */
	{
	  close (f);
	  goto stub_end;
	}
      /* Compute the size of the stub (it is every thing up
         to the beginning of the coff image).  */
      coff_start = (long) _H (2) * 512L;
      if (_H (1))
	coff_start += (long) _H (1) - 512L;

      /* Currently there is only a fixed stub size of 2048 bytes
         supported.  */
      if (coff_start != 2048)
	{
	  close (f);
	  goto stub_end;
	}
      exe_start = _H (4) * 16;
      if ((long) lseek (f, exe_start, SEEK_SET) != exe_start)
	{
	  close (f);
	  goto stub_end;
	}
      if (read (f, &magic, 8) != 8)
	{
	  close (f);
	  goto stub_end;
	}
      if (! CONST_STRNEQ (magic, "go32stub"))
	{
	  close (f);
	  goto stub_end;
	}
      /* Now we found a correct stub (hopefully).  */
      coff_data (abfd)->go32stub = bfd_alloc (abfd, (bfd_size_type) coff_start);
      if (coff_data (abfd)->go32stub == NULL)
	{
	  close (f);
	  return;
	}
      lseek (f, 0L, SEEK_SET);
      if ((unsigned long) read (f, coff_data (abfd)->go32stub, coff_start)
	  != coff_start)
	{
	  bfd_release (abfd, coff_data (abfd)->go32stub);
	  coff_data (abfd)->go32stub = NULL;
	}
      close (f);
    }
stub_end:
  /* There was something wrong above, so use now the standard builtin
     stub.  */
  if (coff_data (abfd)->go32stub == NULL)
    {
      coff_data (abfd)->go32stub
	= bfd_alloc (abfd, (bfd_size_type) GO32_STUBSIZE);
      if (coff_data (abfd)->go32stub == NULL)
	return;
      memcpy (coff_data (abfd)->go32stub, stub_bytes, GO32_STUBSIZE);
    }
}

/* If ibfd was a stubbed coff image, copy the stub from that bfd
   to the new obfd.  */

static bfd_boolean
go32_stubbed_coff_bfd_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  /* Check if both are the same targets.  */
  if (ibfd->xvec != obfd->xvec)
    return TRUE;

  /* Check if we have a source stub.  */
  if (coff_data (ibfd)->go32stub == NULL)
    return TRUE;

  /* As adjust_filehdr_out_pre may get called only after this function,
     optionally allocate the output stub.  */
  if (coff_data (obfd)->go32stub == NULL)
    coff_data (obfd)->go32stub = bfd_alloc (obfd,
					  (bfd_size_type) GO32_STUBSIZE);

  /* Now copy the stub.  */
  if (coff_data (obfd)->go32stub != NULL)
    memcpy (coff_data (obfd)->go32stub, coff_data (ibfd)->go32stub,
	    GO32_STUBSIZE);

  return TRUE;
}

/* coff_object_p only checks 2 bytes F_MAGIC at GO32_STUBSIZE inside the file
   which is too fragile.  */

static const bfd_target *
go32_check_format (bfd *abfd)
{
  char mz[2];

  if (bfd_bread (mz, 2, abfd) != 2 || mz[0] != 'M' || mz[1] != 'Z')
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (bfd_seek (abfd, 0, SEEK_SET) != 0)
    return NULL;

  return coff_object_p (abfd);
}

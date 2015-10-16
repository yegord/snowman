/* Generic BFD support for file formats.
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


/*
SECTION
	File formats

	A format is a BFD concept of high level file contents type. The
	formats supported by BFD are:

	o <<bfd_object>>

	The BFD may contain data, symbols, relocations and debug info.

	o <<bfd_archive>>

	The BFD contains other BFDs and an optional index.

	o <<bfd_core>>

	The BFD contains the result of an executable core dump.

SUBSECTION
	File format functions
*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

/* IMPORT from targets.c.  */
extern const size_t _bfd_target_vector_entries;

/*
FUNCTION
	bfd_check_format

SYNOPSIS
	bfd_boolean bfd_check_format (bfd *abfd, bfd_format format);

DESCRIPTION
	Verify if the file attached to the BFD @var{abfd} is compatible
	with the format @var{format} (i.e., one of <<bfd_object>>,
	<<bfd_archive>> or <<bfd_core>>).

	If the BFD has been set to a specific target before the
	call, only the named target and format combination is
	checked. If the target has not been set, or has been set to
	<<default>>, then all the known target backends is
	interrogated to determine a match.  If the default target
	matches, it is used.  If not, exactly one target must recognize
	the file, or an error results.

	The function returns <<TRUE>> on success, otherwise <<FALSE>>
	with one of the following error codes:

	o <<bfd_error_invalid_operation>> -
	if <<format>> is not one of <<bfd_object>>, <<bfd_archive>> or
	<<bfd_core>>.

	o <<bfd_error_system_call>> -
	if an error occured during a read - even some file mismatches
	can cause bfd_error_system_calls.

	o <<file_not_recognised>> -
	none of the backends recognised the file format.

	o <<bfd_error_file_ambiguously_recognized>> -
	more than one backend recognised the file format.
*/

bfd_boolean
bfd_check_format (bfd *abfd, bfd_format format)
{
  return bfd_check_format_matches (abfd, format, NULL);
}

struct bfd_preserve
{
  void *marker;
  void *tdata;
  flagword flags;
  const struct bfd_arch_info *arch_info;
  struct bfd_section *sections;
  struct bfd_section *section_last;
  unsigned int section_count;
  struct bfd_hash_table section_htab;
};

/* When testing an object for compatibility with a particular target
   back-end, the back-end object_p function needs to set up certain
   fields in the bfd on successfully recognizing the object.  This
   typically happens in a piecemeal fashion, with failures possible at
   many points.  On failure, the bfd is supposed to be restored to its
   initial state, which is virtually impossible.  However, restoring a
   subset of the bfd state works in practice.  This function stores
   the subset.  */

static bfd_boolean
bfd_preserve_save (bfd *abfd, struct bfd_preserve *preserve)
{
  preserve->tdata = abfd->tdata.any;
  preserve->arch_info = abfd->arch_info;
  preserve->flags = abfd->flags;
  preserve->sections = abfd->sections;
  preserve->section_last = abfd->section_last;
  preserve->section_count = abfd->section_count;
  preserve->section_htab = abfd->section_htab;
  preserve->marker = bfd_alloc (abfd, 1);
  if (preserve->marker == NULL)
    return FALSE;

  return bfd_hash_table_init (&abfd->section_htab, bfd_section_hash_newfunc,
			      sizeof (struct section_hash_entry));
}

/* Clear out a subset of BFD state.  */

static void
bfd_reinit (bfd *abfd)
{
  abfd->tdata.any = NULL;
  abfd->arch_info = &bfd_default_arch_struct;
  abfd->flags &= BFD_FLAGS_SAVED;
  bfd_section_list_clear (abfd);
}

/* Restores bfd state saved by bfd_preserve_save.  */

static void
bfd_preserve_restore (bfd *abfd, struct bfd_preserve *preserve)
{
  bfd_hash_table_free (&abfd->section_htab);

  abfd->tdata.any = preserve->tdata;
  abfd->arch_info = preserve->arch_info;
  abfd->flags = preserve->flags;
  abfd->section_htab = preserve->section_htab;
  abfd->sections = preserve->sections;
  abfd->section_last = preserve->section_last;
  abfd->section_count = preserve->section_count;

  /* bfd_release frees all memory more recently bfd_alloc'd than
     its arg, as well as its arg.  */
  bfd_release (abfd, preserve->marker);
  preserve->marker = NULL;
}

/* Called when the bfd state saved by bfd_preserve_save is no longer
   needed.  */

static void
bfd_preserve_finish (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_preserve *preserve)
{
  /* It would be nice to be able to free more memory here, eg. old
     tdata, but that's not possible since these blocks are sitting
     inside bfd_alloc'd memory.  The section hash is on a separate
     objalloc.  */
  bfd_hash_table_free (&preserve->section_htab);
  preserve->marker = NULL;
}

/*
FUNCTION
	bfd_check_format_matches

SYNOPSIS
	bfd_boolean bfd_check_format_matches
	  (bfd *abfd, bfd_format format, char ***matching);

DESCRIPTION
	Like <<bfd_check_format>>, except when it returns FALSE with
	<<bfd_errno>> set to <<bfd_error_file_ambiguously_recognized>>.  In that
	case, if @var{matching} is not NULL, it will be filled in with
	a NULL-terminated list of the names of the formats that matched,
	allocated with <<malloc>>.
	Then the user may choose a format and try again.

	When done with the list that @var{matching} points to, the caller
	should free it.
*/

bfd_boolean
bfd_check_format_matches (bfd *abfd, bfd_format format, char ***matching)
{
  extern const bfd_target binary_vec;
  const bfd_target * const *target;
  const bfd_target **matching_vector = NULL;
  const bfd_target *save_targ, *right_targ, *ar_right_targ, *match_targ;
  int match_count, best_count, best_match;
  int ar_match_index;
  struct bfd_preserve preserve;

  if (matching != NULL)
    *matching = NULL;

  if (!bfd_read_p (abfd)
      || (unsigned int) abfd->format >= (unsigned int) bfd_type_end)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  if (abfd->format != bfd_unknown)
    return abfd->format == format;

  if (matching != NULL || *bfd_associated_vector != NULL)
    {
      bfd_size_type amt;

      amt = sizeof (*matching_vector) * 2 * _bfd_target_vector_entries;
      matching_vector = (const bfd_target **) bfd_malloc (amt);
      if (!matching_vector)
	return FALSE;
    }

  /* Presume the answer is yes.  */
  abfd->format = format;
  save_targ = abfd->xvec;
  preserve.marker = NULL;

  /* If the target type was explicitly specified, just check that target.  */
  if (!abfd->target_defaulted)
    {
      if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)	/* rewind! */
	goto err_ret;

      right_targ = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));

      if (right_targ)
	goto ok_ret;

      /* For a long time the code has dropped through to check all
	 targets if the specified target was wrong.  I don't know why,
	 and I'm reluctant to change it.  However, in the case of an
	 archive, it can cause problems.  If the specified target does
	 not permit archives (e.g., the binary target), then we should
	 not allow some other target to recognize it as an archive, but
	 should instead allow the specified target to recognize it as an
	 object.  When I first made this change, it broke the PE target,
	 because the specified pei-i386 target did not recognize the
	 actual pe-i386 archive.  Since there may be other problems of
	 this sort, I changed this test to check only for the binary
	 target.  */
      if (format == bfd_archive && save_targ == &binary_vec)
	goto err_unrecog;
    }

  /* Since the target type was defaulted, check them all in the hope
     that one will be uniquely recognized.  */
  right_targ = NULL;
  ar_right_targ = NULL;
  match_targ = NULL;
  best_match = 256;
  best_count = 0;
  match_count = 0;
  ar_match_index = _bfd_target_vector_entries;

  for (target = bfd_target_vector; *target != NULL; target++)
    {
      const bfd_target *temp;

      /* Don't check the default target twice.  */
      if (*target == &binary_vec
	  || (!abfd->target_defaulted && *target == save_targ)
	  || (*target)->match_priority > best_match)
	continue;

      /* If we already tried a match, the bfd is modified and may
	 have sections attached, which will confuse the next
	 _bfd_check_format call.  */
      bfd_reinit (abfd);

      /* Change BFD's target temporarily.  */
      abfd->xvec = *target;

      if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
	goto err_ret;

      /* If _bfd_check_format neglects to set bfd_error, assume
	 bfd_error_wrong_format.  We didn't used to even pay any
	 attention to bfd_error, so I suspect that some
	 _bfd_check_format might have this problem.  */
      bfd_set_error (bfd_error_wrong_format);

      temp = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));
      if (temp)
	{
	  match_targ = temp;
	  if (preserve.marker != NULL)
	    bfd_preserve_finish (abfd, &preserve);

	  if (abfd->format != bfd_archive
	      || (bfd_has_map (abfd)
		  && bfd_get_error () != bfd_error_wrong_object_format))
	    {
	      /* This format checks out as ok!  */
	      right_targ = temp;

	      /* If this is the default target, accept it, even if
		 other targets might match.  People who want those
		 other targets have to set the GNUTARGET variable.  */
	      if (temp == bfd_default_vector[0])
		goto ok_ret;

	      if (matching_vector)
		matching_vector[match_count] = temp;
	      match_count++;

	      if (temp->match_priority < best_match)
		{
		  best_match = temp->match_priority;
		  best_count = 0;
		}
	      best_count++;
	    }
	  else
	    {
	      /* An archive with no armap or objects of the wrong
		 type.  We want this target to match if we get no
		 better matches.  */
	      if (ar_right_targ != bfd_default_vector[0])
		ar_right_targ = *target;
	      if (matching_vector)
		matching_vector[ar_match_index] = *target;
	      ar_match_index++;
	    }

	  if (!bfd_preserve_save (abfd, &preserve))
	    goto err_ret;
	}
      else if (bfd_get_error () != bfd_error_wrong_format)
	goto err_ret;
    }

  if (best_count == 1)
    match_count = 1;

  if (match_count == 0)
    {
      /* Try partial matches.  */
      right_targ = ar_right_targ;

      if (right_targ == bfd_default_vector[0])
	{
	  match_count = 1;
	}
      else
	{
	  match_count = ar_match_index - _bfd_target_vector_entries;

	  if (matching_vector && match_count > 1)
	    memcpy (matching_vector,
		    matching_vector + _bfd_target_vector_entries,
		    sizeof (*matching_vector) * match_count);
	}
    }

  /* We have more than one equally good match.  If any of the best
     matches is a target in config.bfd targ_defvec or targ_selvecs,
     choose it.  */
  if (match_count > 1)
    {
      const bfd_target * const *assoc = bfd_associated_vector;

      while ((right_targ = *assoc++) != NULL)
	{
	  int i = match_count;

	  while (--i >= 0)
	    if (matching_vector[i] == right_targ
		&& right_targ->match_priority <= best_match)
	      break;

	  if (i >= 0)
	    {
	      match_count = 1;
	      break;
	    }
	}
    }

  /* We still have more than one equally good match, and at least some
     of the targets support match priority.  Choose the first of the
     best matches.  */
  if (matching_vector && match_count > 1 && best_count != match_count)
    {
      int i;

      for (i = 0; i < match_count; i++)
	{
	  right_targ = matching_vector[i];
	  if (right_targ->match_priority <= best_match)
	    break;
	}
      match_count = 1;
    }

  /* There is way too much undoing of half-known state here.  We
     really shouldn't iterate on live bfd's.  Note that saving the
     whole bfd and restoring it would be even worse; the first thing
     you notice is that the cached bfd file position gets out of sync.  */
  if (preserve.marker != NULL)
    bfd_preserve_restore (abfd, &preserve);

  if (match_count == 1)
    {
      abfd->xvec = right_targ;
      /* If we come out of the loop knowing that the last target that
	 matched is the one we want, then ABFD should still be in a usable
	 state (except possibly for XVEC).  */
      if (match_targ != right_targ)
	{
	  bfd_reinit (abfd);
	  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
	    goto err_ret;
	  match_targ = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));
	  BFD_ASSERT (match_targ != NULL);
	}

    ok_ret:
      /* If the file was opened for update, then `output_has_begun'
	 some time ago when the file was created.  Do not recompute
	 sections sizes or alignments in _bfd_set_section_contents.
	 We can not set this flag until after checking the format,
	 because it will interfere with creation of BFD sections.  */
      if (abfd->direction == both_direction)
	abfd->output_has_begun = TRUE;

      if (matching_vector)
	free (matching_vector);

      /* File position has moved, BTW.  */
      return TRUE;
    }

  if (match_count == 0)
    {
    err_unrecog:
      bfd_set_error (bfd_error_file_not_recognized);
    err_ret:
      abfd->xvec = save_targ;
      abfd->format = bfd_unknown;
      if (matching_vector)
	free (matching_vector);
      if (preserve.marker != NULL)
	bfd_preserve_restore (abfd, &preserve);
      return FALSE;
    }

  /* Restore original target type and format.  */
  abfd->xvec = save_targ;
  abfd->format = bfd_unknown;
  bfd_set_error (bfd_error_file_ambiguously_recognized);

  if (matching)
    {
      *matching = (char **) matching_vector;
      matching_vector[match_count] = NULL;
      /* Return target names.  This is a little nasty.  Maybe we
	 should do another bfd_malloc?  */
      while (--match_count >= 0)
	{
	  const char *name = matching_vector[match_count]->name;
	  *(const char **) &matching_vector[match_count] = name;
	}
    }
  return FALSE;
}

/*
FUNCTION
	bfd_set_format

SYNOPSIS
	bfd_boolean bfd_set_format (bfd *abfd, bfd_format format);

DESCRIPTION
	This function sets the file format of the BFD @var{abfd} to the
	format @var{format}. If the target set in the BFD does not
	support the format requested, the format is invalid, or the BFD
	is not open for writing, then an error occurs.
*/

bfd_boolean
bfd_set_format (bfd *abfd, bfd_format format)
{
  if (bfd_read_p (abfd)
      || (unsigned int) abfd->format >= (unsigned int) bfd_type_end)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  if (abfd->format != bfd_unknown)
    return abfd->format == format;

  /* Presume the answer is yes.  */
  abfd->format = format;

  if (!BFD_SEND_FMT (abfd, _bfd_set_format, (abfd)))
    {
      abfd->format = bfd_unknown;
      return FALSE;
    }

  return TRUE;
}

/*
FUNCTION
	bfd_format_string

SYNOPSIS
	const char *bfd_format_string (bfd_format format);

DESCRIPTION
	Return a pointer to a const string
	<<invalid>>, <<object>>, <<archive>>, <<core>>, or <<unknown>>,
	depending upon the value of @var{format}.
*/

const char *
bfd_format_string (bfd_format format)
{
  if (((int) format < (int) bfd_unknown)
      || ((int) format >= (int) bfd_type_end))
    return "invalid";

  switch (format)
    {
    case bfd_object:
      return "object";		/* Linker/assembler/compiler output.  */
    case bfd_archive:
      return "archive";		/* Object archive file.  */
    case bfd_core:
      return "core";		/* Core dump.  */
    default:
      return "unknown";
    }
}

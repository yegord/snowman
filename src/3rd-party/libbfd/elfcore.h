/* ELF core file support for BFD.
   Copyright (C) 1995-2014 Free Software Foundation, Inc.

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

char*
elf_core_file_failing_command (bfd *abfd)
{
  return elf_tdata (abfd)->core->command;
}

int
elf_core_file_failing_signal (bfd *abfd)
{
  return elf_tdata (abfd)->core->signal;
}

int
elf_core_file_pid (bfd *abfd)
{
  return elf_tdata (abfd)->core->pid;
}

bfd_boolean
elf_core_file_matches_executable_p (bfd *core_bfd, bfd *exec_bfd)
{
  char* corename;

  /* xvecs must match if both are ELF files for the same target.  */

  if (core_bfd->xvec != exec_bfd->xvec)
    {
      bfd_set_error (bfd_error_system_call);
      return FALSE;
    }

  /* See if the name in the corefile matches the executable name.  */
  corename = elf_tdata (core_bfd)->core->program;
  if (corename != NULL)
    {
      const char* execname = strrchr (exec_bfd->filename, '/');

      execname = execname ? execname + 1 : exec_bfd->filename;

      if (strcmp (execname, corename) != 0)
	return FALSE;
    }

  return TRUE;
}

/*  Core files are simply standard ELF formatted files that partition
    the file using the execution view of the file (program header table)
    rather than the linking view.  In fact, there is no section header
    table in a core file.

    The process status information (including the contents of the general
    register set) and the floating point register set are stored in a
    segment of type PT_NOTE.  We handcraft a couple of extra bfd sections
    that allow standard bfd access to the general registers (.reg) and the
    floating point registers (.reg2).  */

const bfd_target *
elf_core_file_p (bfd *abfd)
{
  Elf_External_Ehdr x_ehdr;	/* Elf file header, external form.  */
  Elf_Internal_Ehdr *i_ehdrp;	/* Elf file header, internal form.  */
  Elf_Internal_Phdr *i_phdrp;	/* Elf program header, internal form.  */
  unsigned int phindex;
  const struct elf_backend_data *ebd;
  bfd_size_type amt;

  /* Read in the ELF header in external format.  */
  if (bfd_bread (&x_ehdr, sizeof (x_ehdr), abfd) != sizeof (x_ehdr))
    {
      if (bfd_get_error () != bfd_error_system_call)
	goto wrong;
      else
	goto fail;
    }

  /* Check the magic number.  */
  if (! elf_file_p (&x_ehdr))
    goto wrong;

  /* FIXME: Check EI_VERSION here !  */

  /* Check the address size ("class").  */
  if (x_ehdr.e_ident[EI_CLASS] != ELFCLASS)
    goto wrong;

  /* Check the byteorder.  */
  switch (x_ehdr.e_ident[EI_DATA])
    {
    case ELFDATA2MSB:		/* Big-endian.  */
      if (! bfd_big_endian (abfd))
	goto wrong;
      break;
    case ELFDATA2LSB:		/* Little-endian.  */
      if (! bfd_little_endian (abfd))
	goto wrong;
      break;
    default:
      goto wrong;
    }

  /* Give abfd an elf_obj_tdata.  */
  if (! (*abfd->xvec->_bfd_set_format[bfd_core]) (abfd))
    goto fail;

  /* Swap in the rest of the header, now that we have the byte order.  */
  i_ehdrp = elf_elfheader (abfd);
  elf_swap_ehdr_in (abfd, &x_ehdr, i_ehdrp);

#if DEBUG & 1
  elf_debug_file (i_ehdrp);
#endif

  ebd = get_elf_backend_data (abfd);

  /* Check that the ELF e_machine field matches what this particular
     BFD format expects.  */

  if (ebd->elf_machine_code != i_ehdrp->e_machine
      && (ebd->elf_machine_alt1 == 0
	  || i_ehdrp->e_machine != ebd->elf_machine_alt1)
      && (ebd->elf_machine_alt2 == 0
	  || i_ehdrp->e_machine != ebd->elf_machine_alt2))
    {
      const bfd_target * const *target_ptr;

      if (ebd->elf_machine_code != EM_NONE)
	goto wrong;

      /* This is the generic ELF target.  Let it match any ELF target
	 for which we do not have a specific backend.  */

      for (target_ptr = bfd_target_vector; *target_ptr != NULL; target_ptr++)
	{
	  const struct elf_backend_data *back;

	  if ((*target_ptr)->flavour != bfd_target_elf_flavour)
	    continue;
	  back = xvec_get_elf_backend_data (*target_ptr);
	  if (back->s->arch_size != ARCH_SIZE)
	    continue;
	  if (back->elf_machine_code == i_ehdrp->e_machine
	      || (back->elf_machine_alt1 != 0
	          && i_ehdrp->e_machine == back->elf_machine_alt1)
	      || (back->elf_machine_alt2 != 0
	          && i_ehdrp->e_machine == back->elf_machine_alt2))
	    {
	      /* target_ptr is an ELF backend which matches this
		 object file, so reject the generic ELF target.  */
	      goto wrong;
	    }
	}
    }

  /* If there is no program header, or the type is not a core file, then
     we are hosed.  */
  if (i_ehdrp->e_phoff == 0 || i_ehdrp->e_type != ET_CORE)
    goto wrong;

  /* Does BFD's idea of the phdr size match the size
     recorded in the file? */
  if (i_ehdrp->e_phentsize != sizeof (Elf_External_Phdr))
    goto wrong;

  /* If the program header count is PN_XNUM(0xffff), the actual
     count is in the first section header.  */
  if (i_ehdrp->e_shoff != 0 && i_ehdrp->e_phnum == PN_XNUM)
    {
      Elf_External_Shdr x_shdr;
      Elf_Internal_Shdr i_shdr;
      bfd_signed_vma where = i_ehdrp->e_shoff;

      if (where != (file_ptr) where)
	goto wrong;

      /* Seek to the section header table in the file.  */
      if (bfd_seek (abfd, (file_ptr) where, SEEK_SET) != 0)
	goto fail;

      /* Read the first section header at index 0, and convert to internal
	 form.  */
      if (bfd_bread (&x_shdr, sizeof (x_shdr), abfd) != sizeof (x_shdr))
	goto fail;
      elf_swap_shdr_in (abfd, &x_shdr, &i_shdr);

      if (i_shdr.sh_info != 0)
	{
	  i_ehdrp->e_phnum = i_shdr.sh_info;
	  if (i_ehdrp->e_phnum != i_shdr.sh_info)
	    goto wrong;
	}
    }

  /* Sanity check that we can read all of the program headers.
     It ought to be good enough to just read the last one.  */
  if (i_ehdrp->e_phnum > 1)
    {
      Elf_External_Phdr x_phdr;
      Elf_Internal_Phdr i_phdr;
      bfd_signed_vma where;

      /* Check that we don't have a totally silly number of
	 program headers.  */
      if (i_ehdrp->e_phnum > (unsigned int) -1 / sizeof (x_phdr)
	  || i_ehdrp->e_phnum > (unsigned int) -1 / sizeof (i_phdr))
	goto wrong;

      where = i_ehdrp->e_phoff + (i_ehdrp->e_phnum - 1) * sizeof (x_phdr);
      if (where != (file_ptr) where)
	goto wrong;
      if ((bfd_size_type) where <= i_ehdrp->e_phoff)
	goto wrong;

      if (bfd_seek (abfd, (file_ptr) where, SEEK_SET) != 0)
	goto fail;
      if (bfd_bread (&x_phdr, sizeof (x_phdr), abfd) != sizeof (x_phdr))
	goto fail;
    }

  /* Move to the start of the program headers.  */
  if (bfd_seek (abfd, (file_ptr) i_ehdrp->e_phoff, SEEK_SET) != 0)
    goto wrong;

  /* Allocate space for the program headers.  */
  amt = sizeof (*i_phdrp) * i_ehdrp->e_phnum;
  i_phdrp = (Elf_Internal_Phdr *) bfd_alloc (abfd, amt);
  if (!i_phdrp)
    goto fail;

  elf_tdata (abfd)->phdr = i_phdrp;

  /* Read and convert to internal form.  */
  for (phindex = 0; phindex < i_ehdrp->e_phnum; ++phindex)
    {
      Elf_External_Phdr x_phdr;

      if (bfd_bread (&x_phdr, sizeof (x_phdr), abfd) != sizeof (x_phdr))
	goto fail;

      elf_swap_phdr_in (abfd, &x_phdr, i_phdrp + phindex);
    }

  /* Set the machine architecture.  Do this before processing the
     program headers since we need to know the architecture type
     when processing the notes of some systems' core files.  */
  if (! bfd_default_set_arch_mach (abfd, ebd->arch, 0)
      /* It's OK if this fails for the generic target.  */
      && ebd->elf_machine_code != EM_NONE)
    goto fail;

  /* Let the backend double check the format and override global
     information.  We do this before processing the program headers
     to allow the correct machine (as opposed to just the default
     machine) to be set, making it possible for grok_prstatus and
     grok_psinfo to rely on the mach setting.  */
  if (ebd->elf_backend_object_p != NULL
      && ! ebd->elf_backend_object_p (abfd))
    goto wrong;

  /* Process each program header.  */
  for (phindex = 0; phindex < i_ehdrp->e_phnum; ++phindex)
    if (! bfd_section_from_phdr (abfd, i_phdrp + phindex, (int) phindex))
      goto fail;

  /* Check for core truncation.  */
  {
    bfd_size_type high = 0;
    struct stat statbuf;
    for (phindex = 0; phindex < i_ehdrp->e_phnum; ++phindex)
      {
	Elf_Internal_Phdr *p = i_phdrp + phindex;
	if (p->p_filesz)
	  {
	    bfd_size_type current = p->p_offset + p->p_filesz;
	    if (high < current)
	      high = current;
	  }
      }
    if (bfd_stat (abfd, &statbuf) == 0)
      {
	if ((bfd_size_type) statbuf.st_size < high)
	  {
	    (*_bfd_error_handler)
	      (_("Warning: %B is truncated: expected core file "
		 "size >= %lu, found: %lu."),
	       abfd, (unsigned long) high, (unsigned long) statbuf.st_size);
	  }
      }
  }

  /* Save the entry point from the ELF header.  */
  bfd_get_start_address (abfd) = i_ehdrp->e_entry;
  return abfd->xvec;

wrong:
  bfd_set_error (bfd_error_wrong_format);
fail:
  return NULL;
}

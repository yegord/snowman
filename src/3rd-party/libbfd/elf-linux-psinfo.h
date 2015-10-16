/* Definitions for PRPSINFO structures under ELF on GNU/Linux.
   Copyright (C) 2013-2014 Free Software Foundation, Inc.

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

#ifndef ELF_LINUX_PSINFO_H
#define ELF_LINUX_PSINFO_H

/* The PRPSINFO structures defined below are used by most
   architectures, although some of them define their own versions
   (like e.g., PPC).  */

/* External 32-bit structure for PRPSINFO.  This structure is
   ABI-defined, thus we choose to use char arrays here in order to
   avoid dealing with different types in different architectures.

   This structure will ultimately be written in the corefile's note
   section, as the PRPSINFO.  */

struct elf_external_linux_prpsinfo32
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    char pr_flag[4];			/* Flags.  */
    char pr_uid[2];
    char pr_gid[2];
    char pr_pid[4];
    char pr_ppid[4];
    char pr_pgrp[4];
    char pr_sid[4];
    char pr_fname[16];			/* Filename of executable.  */
    char pr_psargs[80];			/* Initial part of arg list.  */
  };

/* Helper macro to swap (properly handling endianess) things from the
   `elf_internal_linux_prpsinfo' structure to the
   `elf_external_linux_prpsinfo32' structure.

   Note that FROM should be a pointer, and TO should be the explicit
   type.  */

#define LINUX_PRPSINFO32_SWAP_FIELDS(abfd, from, to)			\
  do									\
    {									\
      H_PUT_8 (abfd, from->pr_state, &to.pr_state);			\
      H_PUT_8 (abfd, from->pr_sname, &to.pr_sname);			\
      H_PUT_8 (abfd, from->pr_zomb, &to.pr_zomb);			\
      H_PUT_8 (abfd, from->pr_nice, &to.pr_nice);			\
      H_PUT_32 (abfd, from->pr_flag, to.pr_flag);			\
      H_PUT_16 (abfd, from->pr_uid, to.pr_uid);				\
      H_PUT_16 (abfd, from->pr_gid, to.pr_gid);				\
      H_PUT_32 (abfd, from->pr_pid, to.pr_pid);				\
      H_PUT_32 (abfd, from->pr_ppid, to.pr_ppid);			\
      H_PUT_32 (abfd, from->pr_pgrp, to.pr_pgrp);			\
      H_PUT_32 (abfd, from->pr_sid, to.pr_sid);				\
      strncpy (to.pr_fname, from->pr_fname, sizeof (to.pr_fname));	\
      strncpy (to.pr_psargs, from->pr_psargs, sizeof (to.pr_psargs));	\
    } while (0)

/* External 64-bit structure for PRPSINFO.  This structure is
   ABI-defined, thus we choose to use char arrays here in order to
   avoid dealing with different types in different architectures.

   This structure will ultimately be written in the corefile's note
   section, as the PRPSINFO.  */

struct elf_external_linux_prpsinfo64
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    char pr_flag[8];			/* Flags.  */
    char gap[4];
    char pr_uid[4];
    char pr_gid[4];
    char pr_pid[4];
    char pr_ppid[4];
    char pr_pgrp[4];
    char pr_sid[4];
    char pr_fname[16];			/* Filename of executable.  */
    char pr_psargs[80];			/* Initial part of arg list.  */
  };

/* Helper macro to swap (properly handling endianess) things from the
   `elf_internal_linux_prpsinfo' structure to the
   `elf_external_linux_prpsinfo64' structure.

   Note that FROM should be a pointer, and TO should be the explicit
   type.  */

#define LINUX_PRPSINFO64_SWAP_FIELDS(abfd, from, to)			\
  do									\
    {									\
      H_PUT_8 (abfd, from->pr_state, &to.pr_state);			\
      H_PUT_8 (abfd, from->pr_sname, &to.pr_sname);			\
      H_PUT_8 (abfd, from->pr_zomb, &to.pr_zomb);			\
      H_PUT_8 (abfd, from->pr_nice, &to.pr_nice);			\
      H_PUT_64 (abfd, from->pr_flag, to.pr_flag);			\
      H_PUT_32 (abfd, from->pr_uid, to.pr_uid);				\
      H_PUT_32 (abfd, from->pr_gid, to.pr_gid);				\
      H_PUT_32 (abfd, from->pr_pid, to.pr_pid);				\
      H_PUT_32 (abfd, from->pr_ppid, to.pr_ppid);			\
      H_PUT_32 (abfd, from->pr_pgrp, to.pr_pgrp);			\
      H_PUT_32 (abfd, from->pr_sid, to.pr_sid);				\
      strncpy (to.pr_fname, from->pr_fname, sizeof (to.pr_fname));	\
      strncpy (to.pr_psargs, from->pr_psargs, sizeof (to.pr_psargs));	\
    } while (0)

#endif

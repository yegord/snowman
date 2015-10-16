/* BFD support for the ARC processor
   Copyright (C) 1994-2014 Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).

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

#define ARC(mach, print_name, default_p, next) \
{					\
    32,	/* 32 bits in a word  */	\
    32,	/* 32 bits in an address  */	\
    8,	/* 8 bits in a byte  */		\
    bfd_arch_arc,			\
    mach,				\
    "arc",				\
    print_name,				\
    4, /* section alignment power  */	\
    default_p,				\
    bfd_default_compatible,		\
    bfd_default_scan,			\
    bfd_arch_default_fill,		\
    next,				\
  }

static const bfd_arch_info_type arch_info_struct[] =
{
  ARC ( bfd_mach_arc_5, "arc5", FALSE, &arch_info_struct[1] ),
  ARC ( bfd_mach_arc_5, "base", FALSE, &arch_info_struct[2] ),
  ARC ( bfd_mach_arc_6, "arc6", FALSE, &arch_info_struct[3] ),
  ARC ( bfd_mach_arc_7, "arc7", FALSE, &arch_info_struct[4] ),
  ARC ( bfd_mach_arc_8, "arc8", FALSE, NULL ),
};

const bfd_arch_info_type bfd_arc_arch =
  ARC ( bfd_mach_arc_6, "arc", TRUE, &arch_info_struct[0] );

/* Utility routines.  */

/* Given cpu type NAME, return its bfd_mach_arc_xxx value.
   Returns -1 if not found.  */

int arc_get_mach (char *);

int
arc_get_mach (char *name)
{
  const bfd_arch_info_type *p;

  for (p = &bfd_arc_arch; p != NULL; p = p->next)
    if (strcmp (name, p->printable_name) == 0)
      return p->mach;
  return -1;
}

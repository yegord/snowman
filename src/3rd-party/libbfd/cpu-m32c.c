/* BFD support for the M16C/M32C processors.
   Copyright (C) 2004-2014 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

/* Like bfd_default_scan but if the string is just "m32c" then
   skip the m16c architecture.  */

static bfd_boolean
m32c_scan (const bfd_arch_info_type * info, const char * string)
{
  if (strcmp (string, "m32c") == 0
      && info->mach == bfd_mach_m16c)
    return FALSE;

  return bfd_default_scan (info, string);
}

static const bfd_arch_info_type arch_info_struct[] =
{
  {
    32,				/* bits per word */
    32,				/* bits per address */
    8,				/* bits per byte */
    bfd_arch_m32c,		/* architecture */
    bfd_mach_m32c,		/* machine */
    "m32c",			/* architecture name */
    "m32c",			/* printable name */
    3,				/* section align power */
    FALSE,			/* the default ? */
    bfd_default_compatible,	/* architecture comparison fn */
    m32c_scan,			/* string to architecture convert fn */
    bfd_arch_default_fill,	/* Default fill.  */
    NULL			/* next in list */
  },
};

const bfd_arch_info_type bfd_m32c_arch =
{
  32,				/* Bits per word.  */
  32,				/* Bits per address.  */
  8,				/* Bits per byte.  */
  bfd_arch_m32c,		/* Architecture.  */
  bfd_mach_m16c,		/* Machine.  */
  "m32c",			/* Architecture name.  */
  "m16c",			/* Printable name.  */
  4,				/* Section align power.  */
  TRUE,				/* The default ?  */
  bfd_default_compatible,	/* Architecture comparison fn.  */
  m32c_scan,			/* String to architecture convert fn.  */
  bfd_arch_default_fill,	/* Default fill.  */
  &arch_info_struct[0],		/* Next in list.  */
};

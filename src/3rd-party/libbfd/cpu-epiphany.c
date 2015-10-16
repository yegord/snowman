/* BFD support for the Adapteva EPIPHANY processor.
   Copyright (C) 2009-2014 Free Software Foundation, Inc.
   Contributed by Embecosm on behalf of Adapteva, Inc.

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

const bfd_arch_info_type bfd_epiphany16_arch =
{
  32,				/* Bits per word */
  32,				/* Bits per address.  */
  8,				/* Bits per byte.  */
  bfd_arch_epiphany,		/* Architecture.  */
  bfd_mach_epiphany16,		/* Machine.  */
  "epiphany",			/* Architecture name.  */
  "epiphany16",			/* Machine name.  */
  1,				/* Section align power.  */
  FALSE,			/* The default ?  */
  bfd_default_compatible,	/* Architecture comparison fn.  */
  bfd_default_scan,		/* String to architecture convert fn.  */
  bfd_arch_default_fill,	/* Default fill.  */
  NULL				/* Next in list.  */
};

const bfd_arch_info_type bfd_epiphany_arch =
{
  32,				/* Bits per word - not really true.  */
  32,				/* Bits per address.  */
  8,				/* Bits per byte.  */
  bfd_arch_epiphany,		/* Architecture.  */
  bfd_mach_epiphany32,		/* Machine.  */
  "epiphany",			/* Architecture name.  */
  "epiphany32",			/* Machine name.  */
  2,				/* Section align power.  */
  TRUE,				/* The default ?  */
  bfd_default_compatible,	/* Architecture comparison fn.  */
  bfd_default_scan,		/* String to architecture convert fn.  */
  bfd_arch_default_fill,	/* Default fill.  */
  & bfd_epiphany16_arch	/* Next in list.  */
};

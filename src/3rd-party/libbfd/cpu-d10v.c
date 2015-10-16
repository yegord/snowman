/* BFD support for the D10V processor
   Copyright (C) 1996-2014 Free Software Foundation, Inc.
   Contributed by Martin Hunt (hunt@cygnus.com).

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

static const bfd_arch_info_type d10v_ts3_info =
{
  16,	/* 16 bits in a word.  */
  18,	/* really 16 bits in an address, but code has 18 bit range.  */
  8,	/* 8 bits in a byte.  */
  bfd_arch_d10v,
  bfd_mach_d10v_ts3,
  "d10v",
  "d10v:ts3",
  4, 	/* Section alignment power.  */
  FALSE,
  bfd_default_compatible,
  bfd_default_scan,
  bfd_arch_default_fill,
  0,
};

static const bfd_arch_info_type d10v_ts2_info =
{
  16,
  18,
  8,
  bfd_arch_d10v,
  bfd_mach_d10v_ts2,
  "d10v",
  "d10v:ts2",
  4,
  FALSE,
  bfd_default_compatible,
  bfd_default_scan,
  bfd_arch_default_fill,
  & d10v_ts3_info,
};

const bfd_arch_info_type bfd_d10v_arch =
{
  16,
  18,
  8,
  bfd_arch_d10v,
  bfd_mach_d10v,
  "d10v",
  "d10v",
  4,
  TRUE,
  bfd_default_compatible,
  bfd_default_scan,
  bfd_arch_default_fill,
  & d10v_ts2_info,
};

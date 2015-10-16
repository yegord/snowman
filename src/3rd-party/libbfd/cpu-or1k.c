/* BFD support for the OpenRISC 1000 architecture.
   Copyright 2002-2014 Free Software Foundation, Inc.
   Contributed for OR32 by Ivan Guzvinec  <ivang@opencores.org>

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
   along with this program; if not, see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_or1k_arch;
const bfd_arch_info_type bfd_or1knd_arch;

const bfd_arch_info_type bfd_or1k_arch =
  {
    32,           /* 32 bits in a word.  */
    32,	          /* 32 bits in an address.  */
    8,	          /* 8 bits in a byte.  */
    bfd_arch_or1k,
    bfd_mach_or1k,
    "or1k",
    "or1k",
    4,
    TRUE,
    bfd_default_compatible,
    bfd_default_scan,
    bfd_arch_default_fill,
    &bfd_or1knd_arch,
  };

const bfd_arch_info_type bfd_or1knd_arch =
  {
    32,           /* 32 bits in a word.  */
    32,	          /* 32 bits in an address.  */
    8,	          /* 8 bits in a byte.  */
    bfd_arch_or1k,
    bfd_mach_or1knd,
    "or1knd",
    "or1knd",
    4,
    FALSE,
    bfd_default_compatible,
    bfd_default_scan,
    bfd_arch_default_fill,
    NULL,
  };

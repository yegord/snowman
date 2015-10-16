/* Native Client support for ELF
   Copyright (C) 2012-2014 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.  */

#include "bfd.h"

bfd_boolean nacl_modify_segment_map (bfd *, struct bfd_link_info *);
bfd_boolean nacl_modify_program_headers (bfd *, struct bfd_link_info *);
void nacl_final_write_processing (bfd *, bfd_boolean linker);

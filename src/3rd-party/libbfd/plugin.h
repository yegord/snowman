/* Plugin support for BFD.
   Copyright (C) 2009-2014 Free Software Foundation, Inc.

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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "bfd.h"

void bfd_plugin_set_program_name (const char *);
void bfd_plugin_set_plugin (const char *);

typedef struct plugin_data_struct
{
  int nsyms;
  const struct ld_plugin_symbol *syms;
}
plugin_data_struct;

#endif

/* BFD support for the Intel 386 architecture.
   Copyright (C) 1992-2014 Free Software Foundation, Inc.

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
#include "libiberty.h"

extern void * bfd_arch_i386_short_nop_fill (bfd_size_type, bfd_boolean,
					    bfd_boolean);

static const bfd_arch_info_type *
bfd_i386_compatible (const bfd_arch_info_type *a,
		     const bfd_arch_info_type *b)
{
  const bfd_arch_info_type *compat = bfd_default_compatible (a, b);

  /* Don't allow mixing x64_32 with x86_64.  */
  if (compat
      && (a->mach & bfd_mach_x64_32) != (b->mach & bfd_mach_x64_32))
    compat = NULL;

  return compat;
}

/* Fill the buffer with zero or nop instruction if CODE is TRUE.  Use
   multi byte nop instructions if LONG_NOP is TRUE.  */

static void *
bfd_arch_i386_fill (bfd_size_type count, bfd_boolean code,
		    bfd_boolean long_nop)
{
  /* nop */
  static const char nop_1[] = { 0x90 };
  /* xchg %ax,%ax */
  static const char nop_2[] = { 0x66, 0x90 };
  /* nopl (%[re]ax) */
  static const char nop_3[] = { 0x0f, 0x1f, 0x00 };
  /* nopl 0(%[re]ax) */
  static const char nop_4[] = { 0x0f, 0x1f, 0x40, 0x00 };
  /* nopl 0(%[re]ax,%[re]ax,1) */
  static const char nop_5[] = { 0x0f, 0x1f, 0x44, 0x00, 0x00 };
  /* nopw 0(%[re]ax,%[re]ax,1) */
  static const char nop_6[] = { 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 };
  /* nopl 0L(%[re]ax) */
  static const char nop_7[] = { 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 };
  /* nopl 0L(%[re]ax,%[re]ax,1) */
  static const char nop_8[] =
    { 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00};
  /* nopw 0L(%[re]ax,%[re]ax,1) */
  static const char nop_9[] =
    { 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
  /* nopw %cs:0L(%[re]ax,%[re]ax,1) */
  static const char nop_10[] =
    { 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
  static const char *const nops[] =
    { nop_1, nop_2, nop_3, nop_4, nop_5,
      nop_6, nop_7, nop_8, nop_9, nop_10 };
  bfd_size_type nop_size = long_nop ? ARRAY_SIZE (nops) : 2;

  void *fill = bfd_malloc (count);
  if (fill == NULL)
    return fill;

  if (code)
    {
      bfd_byte *p = fill;
      while (count >= nop_size)
	{
	  memcpy (p, nops[nop_size - 1], nop_size);
	  p += nop_size;
	  count -= nop_size;
	}
      if (count != 0)
	memcpy (p, nops[count - 1], count);
    }
  else
    memset (fill, 0, count);

  return fill;
}

/* Fill the buffer with zero or short nop instruction if CODE is TRUE.  */

void *
bfd_arch_i386_short_nop_fill (bfd_size_type count,
			      bfd_boolean is_bigendian ATTRIBUTE_UNUSED,
			      bfd_boolean code)
{
  return bfd_arch_i386_fill (count, code, FALSE);
}

/* Fill the buffer with zero or long nop instruction if CODE is TRUE.  */

static void *
bfd_arch_i386_long_nop_fill (bfd_size_type count,
			     bfd_boolean is_bigendian ATTRIBUTE_UNUSED,
			     bfd_boolean code)
{
  return bfd_arch_i386_fill (count, code, TRUE);
}

/* Fill the buffer with zero, or one-byte nop instructions if CODE is TRUE.  */

static void *
bfd_arch_i386_onebyte_nop_fill (bfd_size_type count,
				bfd_boolean is_bigendian ATTRIBUTE_UNUSED,
				bfd_boolean code)
{
  void *fill = bfd_malloc (count);
  if (fill != NULL)
    memset (fill, code ? 0x90 : 0, count);
  return fill;
}


static const bfd_arch_info_type bfd_x64_32_nacl_arch =
{
  64, /* 64 bits in a word */
  64, /* 64 bits in an address */
  8,  /* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_x64_32_nacl,
  "i386",
  "i386:x64-32:nacl",
  3,
  FALSE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_onebyte_nop_fill,
  NULL
};

static const bfd_arch_info_type bfd_x86_64_nacl_arch =
{
  64, /* 64 bits in a word */
  64, /* 64 bits in an address */
  8,  /* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_x86_64_nacl,
  "i386",
  "i386:x86-64:nacl",
  3,
  FALSE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_onebyte_nop_fill,
  &bfd_x64_32_nacl_arch
};

const bfd_arch_info_type bfd_i386_nacl_arch =
{
  32,	/* 32 bits in a word */
  32,	/* 32 bits in an address */
  8,	/* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_i386_i386_nacl,
  "i386",
  "i386:nacl",
  3,
  TRUE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_onebyte_nop_fill,
  &bfd_x86_64_nacl_arch
};

static const bfd_arch_info_type bfd_x64_32_arch_intel_syntax =
{
  64, /* 64 bits in a word */
  64, /* 64 bits in an address */
  8,  /* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_x64_32_intel_syntax,
  "i386:intel",
  "i386:x64-32:intel",
  3,
  FALSE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_long_nop_fill,
  &bfd_i386_nacl_arch
};

static const bfd_arch_info_type bfd_x86_64_arch_intel_syntax =
{
  64, /* 64 bits in a word */
  64, /* 64 bits in an address */
  8,  /* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_x86_64_intel_syntax,
  "i386:intel",
  "i386:x86-64:intel",
  3,
  FALSE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_long_nop_fill,
  &bfd_x64_32_arch_intel_syntax,
};

static const bfd_arch_info_type bfd_i386_arch_intel_syntax =
{
  32,	/* 32 bits in a word */
  32,	/* 32 bits in an address */
  8,	/* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_i386_i386_intel_syntax,
  "i386:intel",
  "i386:intel",
  3,
  TRUE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_short_nop_fill,
  &bfd_x86_64_arch_intel_syntax
};

static const bfd_arch_info_type i8086_arch =
{
  32,	/* 32 bits in a word */
  32,	/* 32 bits in an address (well, not really) */
  8,	/* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_i386_i8086,
  "i8086",
  "i8086",
  3,
  FALSE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_short_nop_fill,
  &bfd_i386_arch_intel_syntax
};

static const bfd_arch_info_type bfd_x64_32_arch =
{
  64, /* 64 bits in a word */
  64, /* 64 bits in an address */
  8,  /* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_x64_32,
  "i386",
  "i386:x64-32",
  3,
  FALSE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_long_nop_fill,
  &i8086_arch
};

static const bfd_arch_info_type bfd_x86_64_arch =
{
  64, /* 64 bits in a word */
  64, /* 64 bits in an address */
  8,  /* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_x86_64,
  "i386",
  "i386:x86-64",
  3,
  FALSE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_long_nop_fill,
  &bfd_x64_32_arch
};

const bfd_arch_info_type bfd_i386_arch =
{
  32,	/* 32 bits in a word */
  32,	/* 32 bits in an address */
  8,	/* 8 bits in a byte */
  bfd_arch_i386,
  bfd_mach_i386_i386,
  "i386",
  "i386",
  3,
  TRUE,
  bfd_i386_compatible,
  bfd_default_scan,
  bfd_arch_i386_short_nop_fill,
  &bfd_x86_64_arch
};

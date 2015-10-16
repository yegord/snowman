/* MSP430 ELF support for BFD.
   Copyright (C) 2002-2014 Free Software Foundation, Inc.
   Contributed by Dmitry Diky <diwil@mail.ru>

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_MSP430_H
#define _ELF_MSP430_H

#include "elf/reloc-macros.h"

/* Processor specific flags for the ELF header e_flags field.  */
#define EF_MSP430_MACH 		0xff

#define E_MSP430_MACH_MSP430x11  11
#define E_MSP430_MACH_MSP430x11x1  110
#define E_MSP430_MACH_MSP430x12  12
#define E_MSP430_MACH_MSP430x13  13
#define E_MSP430_MACH_MSP430x14  14
#define E_MSP430_MACH_MSP430x15  15
#define E_MSP430_MACH_MSP430x16  16
#define E_MSP430_MACH_MSP430x20  20
#define E_MSP430_MACH_MSP430x22  22
#define E_MSP430_MACH_MSP430x23  23
#define E_MSP430_MACH_MSP430x24  24
#define E_MSP430_MACH_MSP430x26  26
#define E_MSP430_MACH_MSP430x31  31
#define E_MSP430_MACH_MSP430x32  32
#define E_MSP430_MACH_MSP430x33  33
#define E_MSP430_MACH_MSP430x41  41
#define E_MSP430_MACH_MSP430x42  42
#define E_MSP430_MACH_MSP430x43  43
#define E_MSP430_MACH_MSP430x44  44
#define E_MSP430_MACH_MSP430X    45
#define E_MSP430_MACH_MSP430x46  46
#define E_MSP430_MACH_MSP430x47  47
#define E_MSP430_MACH_MSP430x54  54

#define SHT_MSP430_ATTRIBUTES	0x70000003	/* Section holds ABI attributes.  */
#define SHT_MSP430_SEC_FLAGS	0x7f000005	/* Holds TI compiler's section flags.  */
#define SHT_MSP430_SYM_ALIASES	0x7f000006	/* Holds TI compiler's symbol aliases.  */

/* Tag values for an attribute section.  */
#define OFBA_MSPABI_Tag_ISA		4
#define OFBA_MSPABI_Tag_Code_Model	6
#define OFBA_MSPABI_Tag_Data_Model	8

/* Relocations.  */
START_RELOC_NUMBERS (elf_msp430_reloc_type)
     RELOC_NUMBER (R_MSP430_NONE,		0)
     RELOC_NUMBER (R_MSP430_32,			1)
     RELOC_NUMBER (R_MSP430_10_PCREL,		2)
     RELOC_NUMBER (R_MSP430_16, 		3)
     RELOC_NUMBER (R_MSP430_16_PCREL, 		4)
     RELOC_NUMBER (R_MSP430_16_BYTE, 		5)
     RELOC_NUMBER (R_MSP430_16_PCREL_BYTE, 	6)
     RELOC_NUMBER (R_MSP430_2X_PCREL,		7)
     RELOC_NUMBER (R_MSP430_RL_PCREL,		8)
     RELOC_NUMBER (R_MSP430_8,			9)
     RELOC_NUMBER (R_MSP430_SYM_DIFF,		10)
END_RELOC_NUMBERS (R_MSP430_max)

START_RELOC_NUMBERS (elf_msp430x_reloc_type)
     RELOC_NUMBER (R_MSP430_ABS32, 1)		/* aka R_MSP430_32 */
     RELOC_NUMBER (R_MSP430_ABS16, 2)		/* aka R_MSP430_16 */
     RELOC_NUMBER (R_MSP430_ABS8, 3)
     RELOC_NUMBER (R_MSP430_PCR16, 4)		/* aka R_MSP430_16_PCREL */
     RELOC_NUMBER (R_MSP430X_PCR20_EXT_SRC, 5)
     RELOC_NUMBER (R_MSP430X_PCR20_EXT_DST, 6)
     RELOC_NUMBER (R_MSP430X_PCR20_EXT_ODST, 7)
     RELOC_NUMBER (R_MSP430X_ABS20_EXT_SRC, 8)
     RELOC_NUMBER (R_MSP430X_ABS20_EXT_DST, 9)
     RELOC_NUMBER (R_MSP430X_ABS20_EXT_ODST, 10)
     RELOC_NUMBER (R_MSP430X_ABS20_ADR_SRC, 11)
     RELOC_NUMBER (R_MSP430X_ABS20_ADR_DST, 12)
     RELOC_NUMBER (R_MSP430X_PCR16, 13)		/* Like R_MSP430_PCR16 but with overflow checking.  */
     RELOC_NUMBER (R_MSP430X_PCR20_CALL, 14)
     RELOC_NUMBER (R_MSP430X_ABS16, 15)		/* Like R_MSP430_ABS16 but with overflow checking.  */
     RELOC_NUMBER (R_MSP430_ABS_HI16, 16)
     RELOC_NUMBER (R_MSP430_PREL31, 17)
     RELOC_NUMBER (R_MSP430_EHTYPE, 18)		/* Mentioned in ABI.  */
     RELOC_NUMBER (R_MSP430X_10_PCREL, 19)	/* Red Hat invention.  Used for Jump instructions.  */
     RELOC_NUMBER (R_MSP430X_2X_PCREL, 20)	/* Red Hat invention.  Used for relaxing jumps.  */
     RELOC_NUMBER (R_MSP430X_SYM_DIFF, 21)	/* Red Hat invention.  Used for relaxing debug info.  */
END_RELOC_NUMBERS (R_MSP430x_max)

#endif /* _ELF_MSP430_H */

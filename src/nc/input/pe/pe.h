/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <boost/static_assert.hpp>

#include <nc/common/Types.h>

typedef uint8_t BYTE;
typedef int32_t LONG;
typedef int16_t SHORT;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t ULONGLONG;

/*
 * The following definitions were taken from MinGW64's winnt.h.
 */

#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE 0x00004550

typedef struct _IMAGE_DOS_HEADER {
  WORD e_magic;
  WORD e_cblp;
  WORD e_cp;
  WORD e_crlc;
  WORD e_cparhdr;
  WORD e_minalloc;
  WORD e_maxalloc;
  WORD e_ss;
  WORD e_sp;
  WORD e_csum;
  WORD e_ip;
  WORD e_cs;
  WORD e_lfarlc;
  WORD e_ovno;
  WORD e_res[4];
  WORD e_oemid;
  WORD e_oeminfo;
  WORD e_res2[10];
  LONG e_lfanew;
} IMAGE_DOS_HEADER,*PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
  WORD Machine;
  WORD NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD SizeOfOptionalHeader;
  WORD Characteristics;
} IMAGE_FILE_HEADER,*PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER 20

static_assert(sizeof(IMAGE_FILE_HEADER) == IMAGE_SIZEOF_FILE_HEADER, "");

#define IMAGE_FILE_RELOCS_STRIPPED 0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE 0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED 0x0008
#define IMAGE_FILE_AGGRESIVE_WS_TRIM 0x0010
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define IMAGE_FILE_BYTES_REVERSED_LO 0x0080
#define IMAGE_FILE_32BIT_MACHINE 0x0100
#define IMAGE_FILE_DEBUG_STRIPPED 0x0200
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP 0x0400
#define IMAGE_FILE_NET_RUN_FROM_SWAP 0x0800
#define IMAGE_FILE_SYSTEM 0x1000
#define IMAGE_FILE_DLL 0x2000
#define IMAGE_FILE_UP_SYSTEM_ONLY 0x4000
#define IMAGE_FILE_BYTES_REVERSED_HI 0x8000

#define IMAGE_FILE_MACHINE_I386 0x014c
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_MACHINE_ARM 0x01c0
#define IMAGE_FILE_MACHINE_THUMB 0x01c2

typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD VirtualAddress;
  DWORD Size;
} IMAGE_DATA_DIRECTORY,*PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct _IMAGE_OPTIONAL_HEADER {
  WORD Magic;
  BYTE MajorLinkerVersion;
  BYTE MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;
  DWORD BaseOfCode;
  DWORD BaseOfData;
  DWORD ImageBase;
  DWORD SectionAlignment;
  DWORD FileAlignment;
  WORD MajorOperatingSystemVersion;
  WORD MinorOperatingSystemVersion;
  WORD MajorImageVersion;
  WORD MinorImageVersion;
  WORD MajorSubsystemVersion;
  WORD MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;
  WORD Subsystem;
  WORD DllCharacteristics;
  DWORD SizeOfStackReserve;
  DWORD SizeOfStackCommit;
  DWORD SizeOfHeapReserve;
  DWORD SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32,*PIMAGE_OPTIONAL_HEADER32;

#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER 28

typedef struct _IMAGE_OPTIONAL_HEADER64 {
  WORD Magic;
  BYTE MajorLinkerVersion;
  BYTE MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;
  DWORD BaseOfCode;
  ULONGLONG ImageBase;
  DWORD SectionAlignment;
  DWORD FileAlignment;
  WORD MajorOperatingSystemVersion;
  WORD MinorOperatingSystemVersion;
  WORD MajorImageVersion;
  WORD MinorImageVersion;
  WORD MajorSubsystemVersion;
  WORD MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;
  WORD Subsystem;
  WORD DllCharacteristics;
  ULONGLONG SizeOfStackReserve;
  ULONGLONG SizeOfStackCommit;
  ULONGLONG SizeOfHeapReserve;
  ULONGLONG SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64,*PIMAGE_OPTIONAL_HEADER64;

#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER 224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER 240

BOOST_STATIC_ASSERT(sizeof(IMAGE_OPTIONAL_HEADER32) == IMAGE_SIZEOF_NT_OPTIONAL32_HEADER);
BOOST_STATIC_ASSERT(sizeof(IMAGE_OPTIONAL_HEADER64) == IMAGE_SIZEOF_NT_OPTIONAL64_HEADER);

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
  char Name[IMAGE_SIZEOF_SHORT_NAME];
  union {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
  } Misc;
  DWORD VirtualAddress;
  DWORD SizeOfRawData;
  DWORD PointerToRawData;
  DWORD PointerToRelocations;
  DWORD PointerToLinenumbers;
  WORD NumberOfRelocations;
  WORD NumberOfLinenumbers;
  DWORD Characteristics;
} IMAGE_SECTION_HEADER,*PIMAGE_SECTION_HEADER;

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define IMAGE_DIRECTORY_ENTRY_SECURITY 4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6

#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE 7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8
#define IMAGE_DIRECTORY_ENTRY_TLS 9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11
#define IMAGE_DIRECTORY_ENTRY_IAT 12
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14

#define IMAGE_SIZEOF_SECTION_HEADER 40

BOOST_STATIC_ASSERT(sizeof(IMAGE_SECTION_HEADER) == IMAGE_SIZEOF_SECTION_HEADER);

#define IMAGE_SCN_TYPE_NO_PAD 0x00000008

#define IMAGE_SCN_CNT_CODE 0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_LNK_OTHER 0x00000100
#define IMAGE_SCN_LNK_INFO 0x00000200
#define IMAGE_SCN_LNK_REMOVE 0x00000800
#define IMAGE_SCN_LNK_COMDAT 0x00001000
#define IMAGE_SCN_NO_DEFER_SPEC_EXC 0x00004000
#define IMAGE_SCN_GPREL 0x00008000
#define IMAGE_SCN_MEM_FARDATA 0x00008000
#define IMAGE_SCN_MEM_PURGEABLE 0x00020000
#define IMAGE_SCN_MEM_16BIT 0x00020000
#define IMAGE_SCN_MEM_LOCKED 0x00040000
#define IMAGE_SCN_MEM_PRELOAD 0x00080000

#define IMAGE_SCN_ALIGN_1BYTES 0x00100000
#define IMAGE_SCN_ALIGN_2BYTES 0x00200000
#define IMAGE_SCN_ALIGN_4BYTES 0x00300000
#define IMAGE_SCN_ALIGN_8BYTES 0x00400000
#define IMAGE_SCN_ALIGN_16BYTES 0x00500000
#define IMAGE_SCN_ALIGN_32BYTES 0x00600000
#define IMAGE_SCN_ALIGN_64BYTES 0x00700000
#define IMAGE_SCN_ALIGN_128BYTES 0x00800000
#define IMAGE_SCN_ALIGN_256BYTES 0x00900000
#define IMAGE_SCN_ALIGN_512BYTES 0x00A00000
#define IMAGE_SCN_ALIGN_1024BYTES 0x00B00000
#define IMAGE_SCN_ALIGN_2048BYTES 0x00C00000
#define IMAGE_SCN_ALIGN_4096BYTES 0x00D00000
#define IMAGE_SCN_ALIGN_8192BYTES 0x00E00000

#define IMAGE_SCN_ALIGN_MASK 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL 0x01000000
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000
#define IMAGE_SCN_MEM_NOT_CACHED 0x04000000
#define IMAGE_SCN_MEM_NOT_PAGED 0x08000000
#define IMAGE_SCN_MEM_SHARED 0x10000000
#define IMAGE_SCN_MEM_EXECUTE 0x20000000
#define IMAGE_SCN_MEM_READ 0x40000000
#define IMAGE_SCN_MEM_WRITE 0x80000000

#define IMAGE_SCN_SCALE_INDEX 0x00000001

#pragma pack(push,2)

typedef struct _IMAGE_SYMBOL {
  union {
    char ShortName[8];
    struct {
      DWORD Short;
      DWORD Long;
    } Name;
    DWORD LongName[2];
  } N;
  DWORD Value;
  SHORT SectionNumber;
  WORD Type;
  BYTE StorageClass;
  BYTE NumberOfAuxSymbols;
} IMAGE_SYMBOL;

#pragma pack(pop)

#define IMAGE_SIZEOF_SYMBOL 18

BOOST_STATIC_ASSERT(sizeof(IMAGE_SYMBOL) == IMAGE_SIZEOF_SYMBOL);

#define IMAGE_SYM_UNDEFINED (SHORT)0
#define IMAGE_SYM_ABSOLUTE (SHORT)-1
#define IMAGE_SYM_DEBUG (SHORT)-2
#define IMAGE_SYM_SECTION_MAX 0xFEFF
#define IMAGE_SYM_SECTION_MAX_EX MAXLONG

#define IMAGE_SYM_TYPE_NULL 0x0000
#define IMAGE_SYM_TYPE_VOID 0x0001
#define IMAGE_SYM_TYPE_CHAR 0x0002
#define IMAGE_SYM_TYPE_SHORT 0x0003
#define IMAGE_SYM_TYPE_INT 0x0004
#define IMAGE_SYM_TYPE_LONG 0x0005
#define IMAGE_SYM_TYPE_FLOAT 0x0006
#define IMAGE_SYM_TYPE_DOUBLE 0x0007
#define IMAGE_SYM_TYPE_STRUCT 0x0008
#define IMAGE_SYM_TYPE_UNION 0x0009
#define IMAGE_SYM_TYPE_ENUM 0x000A
#define IMAGE_SYM_TYPE_MOE 0x000B
#define IMAGE_SYM_TYPE_BYTE 0x000C
#define IMAGE_SYM_TYPE_WORD 0x000D
#define IMAGE_SYM_TYPE_UINT 0x000E
#define IMAGE_SYM_TYPE_DWORD 0x000F
#define IMAGE_SYM_TYPE_PCODE 0x8000

#define IMAGE_SYM_DTYPE_NULL 0
#define IMAGE_SYM_DTYPE_POINTER 1
#define IMAGE_SYM_DTYPE_FUNCTION 2
#define IMAGE_SYM_DTYPE_ARRAY 3

#define IMAGE_SYM_CLASS_END_OF_FUNCTION (BYTE)-1
#define IMAGE_SYM_CLASS_NULL 0x0000
#define IMAGE_SYM_CLASS_AUTOMATIC 0x0001
#define IMAGE_SYM_CLASS_EXTERNAL 0x0002
#define IMAGE_SYM_CLASS_STATIC 0x0003
#define IMAGE_SYM_CLASS_REGISTER 0x0004
#define IMAGE_SYM_CLASS_EXTERNAL_DEF 0x0005
#define IMAGE_SYM_CLASS_LABEL 0x0006
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL 0x0007
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT 0x0008
#define IMAGE_SYM_CLASS_ARGUMENT 0x0009
#define IMAGE_SYM_CLASS_STRUCT_TAG 0x000A
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION 0x000B
#define IMAGE_SYM_CLASS_UNION_TAG 0x000C
#define IMAGE_SYM_CLASS_TYPE_DEFINITION 0x000D
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC 0x000E
#define IMAGE_SYM_CLASS_ENUM_TAG 0x000F
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM 0x0010
#define IMAGE_SYM_CLASS_REGISTER_PARAM 0x0011
#define IMAGE_SYM_CLASS_BIT_FIELD 0x0012
#define IMAGE_SYM_CLASS_FAR_EXTERNAL 0x0044
#define IMAGE_SYM_CLASS_BLOCK 0x0064
#define IMAGE_SYM_CLASS_FUNCTION 0x0065
#define IMAGE_SYM_CLASS_END_OF_STRUCT 0x0066
#define IMAGE_SYM_CLASS_FILE 0x0067
#define IMAGE_SYM_CLASS_SECTION 0x0068
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL 0x0069
#define IMAGE_SYM_CLASS_CLR_TOKEN 0x006B

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
  union {
    DWORD Characteristics;
    DWORD OriginalFirstThunk;
  };
  DWORD TimeDateStamp;

  DWORD ForwarderChain;
  DWORD Name;
  DWORD FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR;

typedef IMAGE_IMPORT_DESCRIPTOR *PIMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_IMPORT_BY_NAME {
  WORD Hint;
  BYTE Name[1];
} IMAGE_IMPORT_BY_NAME,*PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_EXPORT_DIRECTORY {
  DWORD   Characteristics;
  DWORD   TimeDataStamp;
  WORD    MajorVersion;
  WORD    MinorVersion;
  DWORD   Name;
  DWORD   Base;
  DWORD   NumberOfFunctions;
  DWORD   NumberOfNames;
  DWORD   AddressOfFunctions;
  DWORD   AddressOfNames;
  DWORD   AddressOfNameOrdinal;
} IMAGE_EXPORT_DIRECTORY;
typedef IMAGE_EXPORT_DIRECTORY *PIMAGE_EXPORT_DIRECTORY;

/* vim:set et sts=4 sw=4: */

/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/common/Types.h>

typedef uint32_t cpu_type_t;
typedef uint32_t cpu_subtype_t;

struct mach_header {
    uint32_t magic;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
};

struct mach_header_64 {
    uint32_t magic;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
};

static const uint32_t MH_MAGIC = 0xfeedface;
static const uint32_t MH_MAGIC_64 = 0xfeedfacf;

static const cpu_type_t CPU_TYPE_I386 = 7;
static const cpu_type_t CPU_TYPE_X86_64 = CPU_TYPE_I386 | 0x1000000;
static const cpu_type_t CPU_TYPE_ARM = 12;
static const cpu_type_t CPU_TYPE_MIPS = 8;

struct load_command {
    uint32_t cmd;
    uint32_t cmdsize;
};

static const uint32_t LC_REQ_DYLD = 0x80000000;

static const uint32_t LC_SEGMENT = 0x1;
static const uint32_t LC_SYMTAB = 0x2;
static const uint32_t LC_DYSYMTAB = 0xb;
static const uint32_t LC_SEGMENT_64 = 0x19;

typedef uint32_t vm_prot_t;

struct segment_command {
    uint32_t cmd;
    uint32_t cmdsize;
    char segname[16];
    uint32_t vmaddr;
    uint32_t vmsize;
    uint32_t fileoff;
    uint32_t filesize;
    vm_prot_t maxprot;
    vm_prot_t initprot;
    uint32_t nsects;
    uint32_t flags;
};

struct segment_command_64 {
    uint32_t cmd;
    uint32_t cmdsize;
    char segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    vm_prot_t maxprot;
    vm_prot_t initprot;
    uint32_t nsects;
    uint32_t flags;
};

vm_prot_t VM_PROT_READ = 0x01;
vm_prot_t VM_PROT_WRITE = 0x02;
vm_prot_t VM_PROT_EXECUTE = 0x04;

struct section {
    char sectname[16];
    char segname[16];
    uint32_t addr;
    uint32_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
};

struct section_64 {
    char sectname[16];
    char segname[16];
    uint64_t addr;
    uint64_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
};

static const uint32_t SECTION_TYPE = 0x000000ff;
static const uint32_t SECTION_ATTRIBUTES = 0xffffff00;

static const uint32_t S_REGULAR = 0;
static const uint32_t S_ZEROFILL = 1;
static const uint32_t S_CSTRING_LITERALS = 2;
static const uint32_t S_4BYTE_LITERALS = 3;
static const uint32_t S_8BYTE_LITERALS = 4;
static const uint32_t S_LITERAL_POINTERS = 5;
static const uint32_t S_NON_LAZY_SYMBOL_POINTERS = 6;
static const uint32_t S_LAZY_SYMBOL_POINTERS = 7;
static const uint32_t S_SYMBOL_STUBS = 8;
static const uint32_t S_MOD_INIT_FUNC_POINTERS = 9;
static const uint32_t S_MOD_TERM_FUNC_POINTERS = 10;
static const uint32_t S_COALESCED = 11;
static const uint32_t S_GB_ZEROFILL = 12;
static const uint32_t S_INTERPOSING = 13;
static const uint32_t S_16BYTE_LITERALS = 14;
static const uint32_t S_DTRACE_DOF = 15;
static const uint32_t S_LAZY_DYLIB_SYMBOL_POINTERS = 16;
static const uint32_t S_THREAD_LOCAL_REGULAR = 17;
static const uint32_t S_THREAD_LOCAL_ZEROFILL = 18;
static const uint32_t S_THREAD_LOCAL_VARIABLES = 19;
static const uint32_t S_THREAD_LOCAL_VARIABLE_POINTERS = 20;
static const uint32_t S_THREAD_LOCAL_INIT_FUNCTION_POINTERS = 21;

static const uint32_t S_ATTR_SOME_INSTRUCTIONS = 0x00000400;
static const uint32_t S_ATTR_PURE_INSTRUCTIONS = 0x80000000;

static const uint32_t INDIRECT_SYMBOL_LOCAL = 0x80000000;
static const uint32_t INDIRECT_SYMBOL_ABS = 0x40000000;

struct symtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t symoff;
    uint32_t nsyms;
    uint32_t stroff;
    uint32_t strsize;
};

struct dysymtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t ilocalsym;
    uint32_t nlocalsym;
    uint32_t iextdefsym;
    uint32_t nextdefsym;
    uint32_t iundefsym;
    uint32_t nundefsym;
    uint32_t tocoff;
    uint32_t ntoc;
    uint32_t modtaboff;
    uint32_t nmodtab;
    uint32_t extrefsymoff;
    uint32_t nextrefsyms;
    uint32_t indirectsymoff;
    uint32_t nindirectsyms;
    uint32_t extreloff;
    uint32_t nextrel;
    uint32_t locreloff;
    uint32_t nlocrel;
};

struct nlist {
    uint32_t n_strx;
    uint8_t n_type;
    uint8_t n_sect;
    int16_t n_desc;
    uint32_t n_value;
};

struct nlist_64 {
    uint32_t n_strx;
    uint8_t n_type;
    uint8_t n_sect;
    uint16_t n_desc;
    uint64_t n_value;
};

static const uint8_t N_TYPE = 0x0e;
static const uint8_t N_UNDF = 0x0;
static const uint8_t N_ABS = 0x2;
static const uint8_t N_SECT = 0xe;
static const uint8_t N_INDR = 0xa;

static const uint8_t NO_SECT = 0;

/* vim:set et sts=4 sw=4: */

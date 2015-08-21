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

static const uint32_t S_ATTR_SOME_INSTRUCTIONS = 0x00000400;
static const uint32_t S_ATTR_PURE_INSTRUCTIONS = 0x80000000;

struct symtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t symoff;
    uint32_t nsyms;
    uint32_t stroff;
    uint32_t strsize;
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

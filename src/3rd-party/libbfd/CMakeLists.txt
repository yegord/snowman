cmake_minimum_required(VERSION 2.8)

# In this file we are doing all of our 'configure' checks. Things like checking
# for headers, functions, libraries, types and size of types.
INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckTypeSize)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(TestBigEndian)
INCLUDE(CheckSymbolExists)
INCLUDE(CMakeDetermineCompileFeatures)
INCLUDE(CheckPrototypeDefinition)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckStructHasMember)

MACRO(CHECK_PROTOTYPE_EXISTS _SYMBOL _HEADER _RESULT)
  SET(_INCLUDE_FILES)
  FOREACH (it ${_HEADER})
    SET(_INCLUDE_FILES "${_INCLUDE_FILES}#include <${it}>\n")
  ENDFOREACH (it)
  
  SET(_CHECK_PROTO_EXISTS_SOURCE_CODE "
${_INCLUDE_FILES}
int main()
{
#ifndef ${_SYMBOL}
    int i = sizeof(&${_SYMBOL});
#endif
    return 0;
}
")
  CHECK_CXX_SOURCE_COMPILES("${_CHECK_PROTO_EXISTS_SOURCE_CODE}" ${_RESULT})
ENDMACRO(CHECK_PROTOTYPE_EXISTS _SYMBOL _HEADER _RESULT)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_COMPILER_IS_CLANGCXX ON)
endif()

# Check if _GNU_SOURCE is available.
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)
  set(_GNU_SOURCE 1)
endif()

find_package(ZLIB)
IF(ZLIB_FOUND)
include_directories(${ZLIB_INCLUDE_DIR})
SET(HAVE_ZLIB_H 1)
ENDIF()

SET(USE_SECUREPLT 1)

# To check for an include file you do this:
CHECK_INCLUDE_FILE("alloca.h" HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE("dirent.h" HAVE_DIRENT_H)
CHECK_INCLUDE_FILE("dlfcn.h" HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("fcntl.h" HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("memory.h" HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("ndir.h" HAVE_NDIR_H)
CHECK_INCLUDE_FILE("stddef.h" HAVE_STDDEF_H)
CHECK_INCLUDE_FILE("stdlib.h" HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)
CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("string.h" HAVE_STRING_H)
CHECK_INCLUDE_FILE("strings.h" HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("unistd.h" HAVE_UNISTD_H)
CHECK_INCLUDE_FILE("time.h" HAVE_TIME_H)
CHECK_INCLUDE_FILE("wchar.h" HAVE_WCHAR_H)
CHECK_INCLUDE_FILE("windows.h" HAVE_WINDOWS_H)
CHECK_INCLUDE_FILE("sys/dir.h" HAVE_SYS_DIR_H)
CHECK_INCLUDE_FILE("sys/file.h" HAVE_SYS_FILE_H)
CHECK_INCLUDE_FILE("sys/ndir.h" HAVE_SYS_NDIR_H)
CHECK_INCLUDE_FILE("sys/param.h" HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILE("sys/procfs.h" HAVE_SYS_PROCFS_H)
CHECK_INCLUDE_FILE("sys/resource.h" HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/time.h" HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)


# To check the size of a primitive type:
CHECK_TYPE_SIZE("char" SIZEOF_CHAR)
CHECK_TYPE_SIZE("int" SIZEOF_INT)
CHECK_TYPE_SIZE("long" SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("short" SIZEOF_SHORT)
CHECK_TYPE_SIZE("off_t" SIZEOF_OFF_T)
CHECK_TYPE_SIZE("void *" SIZEOF_VOID_P)

# To check for an function:
CHECK_PROTOTYPE_EXISTS(basename "libgen.h" HAVE_DECL_BASENAME)
CHECK_PROTOTYPE_EXISTS(fcntl "fcntl.h" HAVE_FCNTL)
CHECK_PROTOTYPE_EXISTS(ffs "string.h;strings.h" HAVE_DECL_FFS)
CHECK_PROTOTYPE_EXISTS(free "stdlib.h;unistd.h" HAVE_DECL_FREE)
CHECK_PROTOTYPE_EXISTS(fseeko "stdio.h" HAVE_DECL_FSEEKO)
CHECK_PROTOTYPE_EXISTS(fseeko64 "stdio.h" HAVE_DECL_FSEEKO64)
CHECK_PROTOTYPE_EXISTS(ftello "stdio.h" HAVE_DECL_FTELLO)
CHECK_PROTOTYPE_EXISTS(ftello64 "stdio.h" HAVE_DECL_FTELLO64)
CHECK_PROTOTYPE_EXISTS(getenv "stdlib.h;unistd.h" HAVE_DECL_GETENV)
CHECK_PROTOTYPE_EXISTS(malloc "stdlib.h;unistd.h" HAVE_DECL_MALLOC)
CHECK_PROTOTYPE_EXISTS(realloc "stdlib.h;unistd.h" HAVE_DECL_REALLOC)
CHECK_PROTOTYPE_EXISTS(snprintf "stdio.h" HAVE_DECL_SNPRINTF)
CHECK_PROTOTYPE_EXISTS(stpcpy "string.h;strings.h" HAVE_DECL_STPCPY)
CHECK_PROTOTYPE_EXISTS(strnlen "string.h;strings.h" HAVE_DECL_STRNLEN)
CHECK_PROTOTYPE_EXISTS(strstr "string.h;strings.h" HAVE_DECL_STRSTR)
CHECK_PROTOTYPE_EXISTS(vsnprintf "stdio.h" HAVE_DECL_VSNPRINTF)

CHECK_FUNCTION_EXISTS(fseeko HAVE_FSEEKO)
CHECK_FUNCTION_EXISTS(fileno HAVE_FILENO)
CHECK_FUNCTION_EXISTS(fseeko64 HAVE_FSEEKO64)
CHECK_FUNCTION_EXISTS(ftello HAVE_FTELLO)
CHECK_FUNCTION_EXISTS(ftello64 HAVE_FTELLO64)
CHECK_FUNCTION_EXISTS(fcntl HAVE_FCNTL)
CHECK_FUNCTION_EXISTS(fdopen HAVE_FDOPEN)
CHECK_FUNCTION_EXISTS(getgid HAVE_GETGID)
CHECK_FUNCTION_EXISTS(getuid HAVE_GETUID)
CHECK_FUNCTION_EXISTS(getpagesize HAVE_GETPAGESIZE)
CHECK_FUNCTION_EXISTS(getrlimit HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS(madvise HAVE_MADVISE)
CHECK_FUNCTION_EXISTS(mmap HAVE_MMAP)
CHECK_FUNCTION_EXISTS(mprotect HAVE_MPROTECT)
CHECK_FUNCTION_EXISTS(setitimer HAVE_SETITIMER)
CHECK_FUNCTION_EXISTS(strtoull HAVE_STRTOULL)
CHECK_FUNCTION_EXISTS(sysconf HAVE_SYSCONF)

#check for stdc headers
CHECK_INCLUDE_FILES("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

IF (UNIX OR MINGW)
  SET (_ALL_SOURCE 1)
  SET (_POSIX_PTHREAD_SEMANTICS 1)
  SET (_TANDEM_SOURCE 1)
  SET (__EXTENSIONS__ 1)
ENDIF (UNIX OR MINGW)

IF (MSVC OR MINGW)
  SET (USE_BINARY_FOPEN 1)
ENDIF (MSVC OR MINGW)

# And you continue doing these types of checks for each item that you want to check. You can even do custom "try-compiles/Try-run" for more elaborate setups.
# Now that you have your variables define you add lines like this to your CMakeLists.txt file:

# Check whether time and sys/time can both be included
check_c_source_compiles(
"
#include <sys/time.h>
#include <time.h>
int main(int argc, char** argv) {return 0;}
"
TIME_WITH_SYS_TIME
)
if (TIME_WITH_SYS_TIME)
  if (DEBUG_CMAKE)
    message("time.h and sys/time.h are compatible.")
  endif ()
  set(TIME_WITH_SYS_TIME 1 CACHE BOOL "Whether time.h and sys/time.h are compatible")
else ()
  if (DEBUG_CMAKE)
    message("time.h and sys/time.h are NOT compatible.")
  endif ()
endif ()

# Check whether string and strings can both be included
check_c_source_compiles(
"
#include <string.h>
#include <strings.h>
int main(int argc, char** argv) {return 0;}
"
STRING_WITH_STRINGS
)
if (STRING_WITH_STRINGS)
  if (DEBUG_CMAKE)
    message("string.h and strings.h are compatible.")
  endif ()
  set(STRING_WITH_STRINGS 1 CACHE BOOL "Whether string.h and strings.h are compatible")
else ()
  if (DEBUG_CMAKE)
    message("string.h and strings.h are NOT compatible.")
  endif ()
endif ()

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
lwpstatus_t avar
  ;
  return 0;
}
"
LWPSTATUS_T
)
if (LWPSTATUS_T)
  set(HAVE_LWPSTATUS_T 1)
endif ()

CHECK_STRUCT_HAS_MEMBER ("lwpstatus_t" "pr_context" "sys/procfs.h" HAVE_LWPSTATUS_T_PR_CONTEXT)
CHECK_STRUCT_HAS_MEMBER ("lwpstatus_t" "pr_fpreg" "sys/procfs.h" HAVE_LWPSTATUS_T_PR_FPREG)
CHECK_STRUCT_HAS_MEMBER ("lwpstatus_t" "pr_reg" "sys/procfs.h" HAVE_LWPSTATUS_T_PR_REG)

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
lwpxstatus_t avar
  ;
  return 0;
}
"
LWPXSTATUS_T
)
if (LWPXSTATUS_T)
  set(HAVE_LWPXSTATUS_T 1)
endif ()

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
prpsinfo32_t avar
  ;
  return 0;
}
"
PRPSINFO32_T
)
if (PRPSINFO32_T)
  set(HAVE_PRPSINFO32_T 1)
endif ()

CHECK_STRUCT_HAS_MEMBER ("prpsinfo32_t" "pr_pid" "sys/procfs.h" HAVE_PRPSINFO32_T_PR_PID)

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
prpsinfo_t avar
  ;
  return 0;
}
"
PRPSINFO_T
)
if (PRPSINFO_T)
  set(HAVE_PRPSINFO_T 1)
endif ()

CHECK_STRUCT_HAS_MEMBER ("prpsinfo_t" "pr_pid" "sys/procfs.h" HAVE_PRPSINFO_T_PR_PID)

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
prstatus32_t avar
  ;
  return 0;
}
"
PRSTATUS32_T
)
if (PRSTATUS32_T)
  set(HAVE_PRSTATUS32_T 1)
endif ()

CHECK_STRUCT_HAS_MEMBER ("prstatus32_t" "pr_who" "sys/procfs.h" HAVE_PRSTATUS32_T_PR_WHO)

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
prstatus_t avar
  ;
  return 0;
}
"
PRSTATUS_T
)
if (PRSTATUS_T)
  set(HAVE_PRSTATUS_T 1)
endif ()

CHECK_STRUCT_HAS_MEMBER ("prstatus_t" "pr_who" "sys/procfs.h" HAVE_PRSTATUS_T_PR_WHO)

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
psinfo32_t avar
  ;
  return 0;
}
"
PSINFO32_T
)
if (PSINFO32_T)
  set(HAVE_PSINFO32_T 1)
endif ()

CHECK_STRUCT_HAS_MEMBER ("psinfo32_t" "pr_pid" "sys/procfs.h" HAVE_PSINFO32_T_PR_PID)

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
psinfo_t avar
  ;
  return 0;
}
"
PSINFO_T
)
if (PSINFO_T)
  set(HAVE_PSINFO_T 1)
endif ()

CHECK_STRUCT_HAS_MEMBER ("psinfo_t" "pr_pid" "sys/procfs.h" HAVE_PSINFO_T_PR_PID)

CHECK_STRUCT_HAS_MEMBER ("struct core_dumpx" "c_impl" "core.h" HAVE_ST_C_IMPL)

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
pstatus32_t avar
  ;
  return 0;
}
"
PSTATUS32_T
)
if (PSTATUS32_T)
  set(HAVE_PSTATUS32_T 1)
endif ()

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
pstatus_t avar
  ;
  return 0;
}
"
PSTATUS_T
)
if (PSTATUS_T)
  set(HAVE_PSTATUS_T 1)
endif ()

check_c_source_compiles(
"
#define _SYSCALL32
/* Needed for new procfs interface on sparc-solaris.  */
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
int
main ()
{
pxstatus_t avar
  ;
  return 0;
}
"
PXSTATUS_T
)
if (PXSTATUS_T)
  set(HAVE_PXSTATUS_T 1)
endif ()


SET(PACKAGE "\"bfd\"")
SET(VERSION "\"2.25.1\"")
SET(PACKAGE_VERSION "\"2.25.1\"")
SET(PACKAGE_BUGREPORT "\"\"")
SET(PACKAGE_URL "\"\"")
SET(PACKAGE_NAME "\"bfd\"")
SET(PACKAGE_TARNAME "\"bfd\"")
SET(PACKAGE_STRING "\"bfd 2.25.1\"")

SET(DEBUGDIR "\".\"")
SET(BINDIR "\".\"")

configure_file ( config.h.cmake config.h )
# Now make sure that you the the build directory on your "Include" path when compiling
include_directories(.)


if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4267 /wd4244)
endif()

#OPTIONAL BACKENDS
#aix386-core.c <- needs core.h
#hpux-core.c <- needs core.h
#netbsd-core.c <- needs sys/core.h 
#osf-core.c
#sco5-core.c
#trad-core.c

set(SOURCES
aix5ppc-core.c
alloca-conf.h
ansidecl.h
aout-adobe.c
aout-arm.c
aout-cris.c
aout-ns32k.c
aout-sparcle.c
aout-target.h
aout-tic30.c
aout0.c
aout32.c
aout64.c
aoutf1.h
aoutx.h
archive.c
archive64.c
archures.c
armnetbsd.c
bfd-in.h
bfd-in2.h
bfd-in3.h
bfd.c
bfd.h
bfd_stdint.h
bfdio.c
bfdlink.h
bfdver.h
bfdwin.c
binary-io.h
binary.c
bout.c
bout.h
cache.c
cf-i386lynx.c
cf-sparclynx.c
cisco-core.c
coff-alpha.c
coff-apollo.c
coff-arm.c
coff-aux.c
coff-go32.c
coff-h8300.c
coff-h8500.c
coff-i386.c
coff-i860.c
coff-i960.c
coff-ia64.c
coff-m68k.c
coff-m88k.c
coff-mips.c
coff-rs6000.c
coff-sh.c
coff-sparc.c
coff-stgo32.c
coff-svm68k.c
coff-tic30.c
coff-tic4x.c
coff-tic54x.c
coff-tic80.c
coff-u68k.c
coff-w65.c
coff-we32k.c
coff-x86_64.c
coff-z80.c
coff-z8k.c
coff64-rs6000.c
coffcode.h
coffgen.c
cofflink.c
coffswap.h
compress.c
config.h
corefile.c
cpu-aarch64.c
cpu-alpha.c
cpu-arc.c
cpu-arm.c
cpu-avr.c
cpu-bfin.c
cpu-cr16.c
cpu-cr16c.c
cpu-cris.c
cpu-crx.c
cpu-d10v.c
cpu-d30v.c
cpu-dlx.c
cpu-epiphany.c
cpu-fr30.c
cpu-frv.c
cpu-h8300.c
cpu-h8500.c
cpu-hppa.c
cpu-i370.c
cpu-i386.c
cpu-i860.c
cpu-i960.c
cpu-ia64.c
cpu-ip2k.c
cpu-iq2000.c
cpu-k1om.c
cpu-l1om.c
cpu-lm32.c
cpu-m10200.c
cpu-m10300.c
cpu-m32c.c
cpu-m32r.c
cpu-m68hc11.c
cpu-m68hc12.c
cpu-m68k.c
cpu-m88k.c
cpu-m9s12x.c
cpu-m9s12xg.c
cpu-mcore.c
cpu-mep.c
cpu-metag.c
cpu-microblaze.c
cpu-mips.c
cpu-mmix.c
cpu-moxie.c
cpu-msp430.c
cpu-mt.c
cpu-nds32.c
cpu-nios2.c
cpu-ns32k.c
cpu-or1k.c
cpu-pdp11.c
cpu-pj.c
cpu-plugin.c
cpu-powerpc.c
cpu-rl78.c
cpu-rs6000.c
cpu-rx.c
cpu-s390.c
cpu-score.c
cpu-sh.c
cpu-sparc.c
cpu-spu.c
cpu-tic30.c
cpu-tic4x.c
cpu-tic54x.c
cpu-tic6x.c
cpu-tic80.c
cpu-tilegx.c
cpu-tilepro.c
cpu-v850.c
cpu-v850_rh850.c
cpu-vax.c
cpu-w65.c
cpu-we32k.c
cpu-xc16x.c
cpu-xgate.c
cpu-xstormy16.c
cpu-xtensa.c
cpu-z80.c
cpu-z8k.c
demangle.h
demo64.c
dis-asm.h
dwarf1.c
dwarf2.c
dwarf2.h
dyn-string.h
ecoff.c
ecofflink.c
ecoffswap.h
elf-attrs.c
elf-bfd.h
elf-eh-frame.c
elf-hppa.h
elf-ifunc.c
elf-linux-psinfo.h
elf-m10200.c
elf-m10300.c
elf-nacl.c
elf-nacl.h
elf-strtab.c
elf-vxworks.c
elf-vxworks.h
elf.c
elf32-aarch64.c
elf32-am33lin.c
elf32-arc.c
elf32-arm.c
elf32-avr.c
elf32-avr.h
elf32-bfin.c
elf32-cr16.c
elf32-cr16c.c
elf32-cris.c
elf32-crx.c
elf32-d10v.c
elf32-d30v.c
elf32-dlx.c
elf32-epiphany.c
elf32-fr30.c
elf32-frv.c
elf32-gen.c
elf32-h8300.c
elf32-hppa.c
elf32-hppa.h
elf32-i370.c
elf32-i386.c
elf32-i860.c
elf32-i960.c
elf32-ia64.c
elf32-ip2k.c
elf32-iq2000.c
elf32-lm32.c
elf32-m32c.c
elf32-m32r.c
elf32-m68hc11.c
elf32-m68hc12.c
elf32-m68hc1x.c
elf32-m68hc1x.h
elf32-m68k.c
elf32-m88k.c
elf32-mcore.c
elf32-mep.c
elf32-metag.c
elf32-metag.h
elf32-microblaze.c
elf32-mips.c
elf32-moxie.c
elf32-msp430.c
elf32-mt.c
elf32-nds32.c
elf32-nds32.h
elf32-nios2.c
elf32-nios2.h
elf32-or1k.c
elf32-pj.c
elf32-ppc.c
elf32-ppc.h
elf32-rl78.c
elf32-rx.c
elf32-rx.h
elf32-s390.c
elf32-score.c
elf32-score.h
elf32-score7.c
elf32-sh-relocs.h
elf32-sh-symbian.c
elf32-sh.c
elf32-sh64-com.c
elf32-sh64.c
elf32-sh64.h
elf32-sparc.c
elf32-spu.c
elf32-spu.h
elf32-target.h
elf32-tic6x.c
elf32-tic6x.h
elf32-tilegx.c
elf32-tilegx.h
elf32-tilepro.c
elf32-tilepro.h
elf32-v850.c
elf32-vax.c
elf32-xc16x.c
elf32-xgate.c
elf32-xgate.h
elf32-xstormy16.c
elf32-xtensa.c
elf32.c
elf64-aarch64.c
elf64-alpha.c
elf64-gen.c
elf64-hppa.c
elf64-hppa.h
elf64-ia64-vms.c
elf64-ia64.c
elf64-mips.c
elf64-mmix.c
elf64-ppc.c
elf64-ppc.h
elf64-s390.c
elf64-sh64.c
elf64-sparc.c
elf64-target.h
elf64-tilegx.c
elf64-tilegx.h
elf64-x86-64.c
elf64.c
elfcode.h
elfcore.h
elflink.c
elfn32-mips.c
elfxx-aarch64.c
elfxx-aarch64.h
elfxx-ia64.c
elfxx-ia64.h
elfxx-mips.c
elfxx-mips.h
elfxx-sparc.c
elfxx-sparc.h
elfxx-target.h
elfxx-tilegx.c
elfxx-tilegx.h
epoc-pe-arm.c
epoc-pei-arm.c
fibheap.h
filenames.h
floatformat.h
fnmatch.h
fopen-bin.h
fopen-same.h
fopen-vms.h
format.c
freebsd.h
genlink.h
getopt.h
go32stub.h
hash.c
hashtab.h
hp-symtab.h
hp300bsd.c
hp300hpux.c
hppabsd-core.c
i386aout.c
i386bsd.c
i386dynix.c
i386freebsd.c
i386linux.c
i386lynx.c
i386mach3.c
i386msdos.c
i386netbsd.c
i386os9k.c
ieee.c
ieee.h
ihex.c
init.c
irix-core.c
leb128.h
libaout.h
libbfd-in.h
libbfd.c
libbfd.h
libcoff-in.h
libcoff.h
libecoff.h
libhppa.h
libiberty.h
libieee.h
libnlm.h
liboasys.h
libpei.h
libxcoff.h
linker.c
longlong.h
lto-symtab.h
lynx-core.c
m68k4knetbsd.c
m68klinux.c
m68knetbsd.c
m88kmach3.c
m88kopenbsd.c
mach-o-i386.c
mach-o-x86-64.c
mach-o.c
mach-o.h
md5.h
merge.c
mipsbsd.c
mmo.c
netbsd.h
newsos3.c
nlm-target.h
nlm.c
nlm32-alpha.c
nlm32-i386.c
nlm32-ppc.c
nlm32-sparc.c
nlm32.c
nlm64.c
nlmcode.h
nlmswap.h
ns32k.h
ns32knetbsd.c
oasys.c
oasys.h
objalloc.h
obstack.h
opncls.c
os9k.h
partition.h
pc532-mach.c
pdp11.c
pe-arm-wince.c
pe-arm.c
pe-i386.c
pe-mcore.c
pe-mips.c
pe-ppc.c
pe-sh.c
pe-x86_64.c
peXXigen.c
pef-traceback.h
pef.c
pef.h
pei-arm-wince.c
pei-arm.c
pei-i386.c
pei-ia64.c
pei-mcore.c
pei-mips.c
pei-ppc.c
pei-sh.c
pei-x86_64.c
peicode.h
peigen.c
pepigen.c
pex64igen.c
plugin-api.h
plugin.c
plugin.h
ppcboot.c
progress.h
ptrace-core.c
reloc.c
reloc16.c
riscix.c
rs6000-core.c
safe-ctype.h
section.c
sha1.h
simple-object.h
simple.c
som.c
som.h
sort.h
sparclinux.c
sparclynx.c
sparcnetbsd.c
splay-tree.h
srec.c
stab-syms.c
stabs.c
sunos.c
symcat.h
syms.c
sysdep.h
targets.c
targmatch.h
tekhex.c
timeval-utils.h
vax1knetbsd.c
vaxbsd.c
vaxnetbsd.c
verilog.c
versados.c
version.h
vms-alpha.c
vms-lib.c
vms-misc.c
vms.h
vtv-change-permission.h
xcofflink.c
xregex.h
xregex2.h
xsym.c
xsym.h
xtensa-config.h
xtensa-isa-internal.h
xtensa-isa.c
xtensa-isa.h
xtensa-modules.c
)

add_library(bfd ${SOURCES})
target_link_libraries(bfd ${ZLIB_LIBRARY})
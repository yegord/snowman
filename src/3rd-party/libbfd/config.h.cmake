/* Check that config.h is #included before system headers
    (this works only for glibc, but that should be enough).  */
#if defined(__GLIBC__) && !defined(__FreeBSD_kernel__) && !defined(__CONFIG_H__)
#  error config.h must be #included before system headers
#endif
#define __CONFIG_H__ 1

/* Name of host specific core header file to include in elf.c. */
#cmakedefine CORE_HEADER @CORE_HEADER@

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#cmakedefine ENABLE_NLS @ENABLE_NLS@

/* Define to 1 if you have the <alloca.h> header file. */
#cmakedefine HAVE_ALLOCA_H @HAVE_ALLOCA_H@

/* Define to 1 if you have the declaration of `basename', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_BASENAME @HAVE_DECL_BASENAME@

/* Define to 1 if you have the declaration of `ffs', and to 0 if you don't. */
#cmakedefine HAVE_DECL_FFS @HAVE_DECL_FFS@

/* Define to 1 if you have the declaration of `free', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_FREE @HAVE_DECL_FREE@

/* Define to 1 if you have the declaration of `fseeko', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_FSEEKO @HAVE_DECL_FSEEKO@

/* Define to 1 if you have the declaration of `fseeko64', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_FSEEKO64 @HAVE_DECL_FSEEKO64@

/* Define to 1 if you have the declaration of `ftello', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_FTELLO @HAVE_DECL_FTELLO@

/* Define to 1 if you have the declaration of `ftello64', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_FTELLO64 @HAVE_DECL_FTELLO64@

/* Define to 1 if you have the declaration of `getenv', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_GETENV @HAVE_DECL_GETENV@

/* Define to 1 if you have the declaration of `malloc', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_MALLOC @HAVE_DECL_MALLOC@

/* Define to 1 if you have the declaration of `realloc', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_REALLOC @HAVE_DECL_REALLOC@

/* Define to 1 if you have the declaration of `snprintf', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_SNPRINTF @HAVE_DECL_SNPRINTF@

/* Define to 1 if you have the declaration of `stpcpy', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_STPCPY @HAVE_DECL_STPCPY@

/* Define to 1 if you have the declaration of `strnlen', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_STRNLEN @HAVE_DECL_STRNLEN@

/* Define to 1 if you have the declaration of `strstr', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_STRSTR @HAVE_DECL_STRSTR@

/* Define to 1 if you have the declaration of `vsnprintf', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_VSNPRINTF @HAVE_DECL_VSNPRINTF@

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H @HAVE_DIRENT_H@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H @HAVE_DLFCN_H@

/* Define to 1 if you have the `fcntl' function. */
#cmakedefine HAVE_FCNTL @HAVE_FNCTL@

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H @HAVE_FNCTL_H@

/* Define to 1 if you have the `fdopen' function. */
#cmakedefine HAVE_FDOPEN @HAVE_FDOPEN@

/* Define to 1 if you have the `fileno' function. */
#cmakedefine HAVE_FILENO @HAVE_FILENO@

/* Define to 1 if you have the `fopen64' function. */
#cmakedefine HAVE_FOPEN64 @HAVE_FOPEN64@

/* Define to 1 if you have the `fseeko' function. */
#cmakedefine HAVE_FSEEKO @HAVE_FSEEKO@

/* Define to 1 if you have the `fseeko64' function. */
#cmakedefine HAVE_FSEEKO64 @HAVE_FSEEKO64@

/* Define to 1 if you have the `ftello' function. */
#cmakedefine HAVE_FTELLO @HAVE_FTELLO@

/* Define to 1 if you have the `ftello64' function. */
#cmakedefine HAVE_FTELLO64 @HAVE_FTELLO64@

/* Define to 1 if you have the `getgid' function. */
#cmakedefine HAVE_GETGID @HAVE_GETGID@

/* Define to 1 if you have the `getpagesize' function. */
#cmakedefine HAVE_GETPAGESIZE @HAVE_GETPAGESIZE@

/* Define to 1 if you have the `getrlimit' function. */
#cmakedefine HAVE_GETRLIMIT @HAVE_GETRLIMIT@

/* Define to 1 if you have the `getuid' function. */
#cmakedefine HAVE_GETUID @HAVE_GETUID@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H @HAVE_INTTYPES_H@

/* Define if <sys/procfs.h> has lwpstatus_t. */
#cmakedefine HAVE_LWPSTATUS_T @HAVE_LWPSTATUS_T@

/* Define if <sys/procfs.h> has lwpstatus_t.pr_context. */
#cmakedefine HAVE_LWPSTATUS_T_PR_CONTEXT @HAVE_LWPSTATUS_T_PR_CONTEXT@

/* Define if <sys/procfs.h> has lwpstatus_t.pr_fpreg. */
#undef HAVE_LWPSTATUS_T_PR_FPREG @HAVE_LWPSTATUS_T_PR_FPREG@

/* Define if <sys/procfs.h> has lwpstatus_t.pr_reg. */
#cmakedefine HAVE_LWPSTATUS_T_PR_REG @HAVE_LWPSTATUS_T_PR_REG@

/* Define if <sys/procfs.h> has lwpxstatus_t. */
#cmakedefine HAVE_LWPXSTATUS_T @HAVE_LWPXSTATUS_T@

/* Define to 1 if you have the `madvise' function. */
#cmakedefine HAVE_MADVISE @HAVE_MADVISE@

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H @HAVE_MEMORY_H@

/* Define to 1 if you have a working `mmap' system call. */
#cmakedefine HAVE_MMAP @HAVE_MMAP@

/* Define to 1 if you have the `mprotect' function. */
#cmakedefine HAVE_MPROTECT @HAVE_MPROTECT@

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H @HAVE_NDIR_H@

/* Define if <sys/procfs.h> has prpsinfo32_t. */
#cmakedefine HAVE_PRPSINFO32_T @HAVE_PRPSINFO32_T@

/* Define if <sys/procfs.h> has prpsinfo32_t.pr_pid. */
#cmakedefine HAVE_PRPSINFO32_T_PR_PID @HAVE_PRPSINFO32_T_PR_PID@

/* Define if <sys/procfs.h> has prpsinfo_t. */
#cmakedefine HAVE_PRPSINFO_T @HAVE_PRPSINFO_T@

/* Define if <sys/procfs.h> has prpsinfo_t.pr_pid. */
#cmakedefine HAVE_PRPSINFO_T_PR_PID @HAVE_PRPSINFO_T_PR_PID@

/* Define if <sys/procfs.h> has prstatus32_t. */
#cmakedefine HAVE_PRSTATUS32_T @HAVE_PRSTATUS32_T@

/* Define if <sys/procfs.h> has prstatus32_t.pr_who. */
#cmakedefine HAVE_PRSTATUS32_T_PR_WHO @HAVE_PRSTATUS32_T_PR_WHO@

/* Define if <sys/procfs.h> has prstatus_t. */
#cmakedefine HAVE_PRSTATUS_T @HAVE_PRSTATUS_T@

/* Define if <sys/procfs.h> has prstatus_t.pr_who. */
#cmakedefine HAVE_PRSTATUS_T_PR_WHO @HAVE_PRSTATUS_T_PR_WHO@

/* Define if <sys/procfs.h> has psinfo32_t. */
#cmakedefine HAVE_PSINFO32_T @HAVE_PSINFO32_T@

/* Define if <sys/procfs.h> has psinfo32_t.pr_pid. */
#cmakedefine HAVE_PSINFO32_T_PR_PID @HAVE_PSINFO32_T_PR_PID@

/* Define if <sys/procfs.h> has psinfo_t. */
#cmakedefine HAVE_PSINFO_T @HAVE_PSINFO_T@

/* Define if <sys/procfs.h> has psinfo_t.pr_pid. */
#cmakedefine HAVE_PSINFO_T_PR_PID @HAVE_PSINFO_T_PR_PID@

/* Define if <sys/procfs.h> has pstatus32_t. */
#cmakedefine HAVE_PSTATUS32_T @HAVE_PSTATUS32_T@

/* Define if <sys/procfs.h> has pstatus_t. */
#cmakedefine HAVE_PSTATUS_T @HAVE_PSTATUS_T@

/* Define if <sys/procfs.h> has pxstatus_t. */
#cmakedefine HAVE_PXSTATUS_T @HAVE_PXSTATUS_T@

/* Define to 1 if you have the `setitimer' function. */
#cmakedefine HAVE_SETITIMER @HAVE_SETITIMER@

/* Define to 1 if you have the <stddef.h> header file. */
#cmakedefine HAVE_STDDEF_H @HAVE_STDDEF_H@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H @HAVE_STDINT_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H  @HAVE_STDLIB_H@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H @HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H @HAVE_STRING_H@

/* Define to 1 if you have the `strtoull' function. */
#cmakedefine HAVE_STRTOULL @HAVE_STRTOULL@

/* Define if struct core_dumpx has member c_impl */
#undef HAVE_ST_C_IMPL @HAVE_ST_C_IMPL@

/* Define to 1 if you have the `sysconf' function. */
#cmakedefine HAVE_SYSCONF @HAVE_SYSCONF@

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H @HAVE_SYS_DIR_H@

/* Define to 1 if you have the <sys/file.h> header file. */
#cmakedefine HAVE_SYS_FILE_H @HAVE_SYS_FILE_H@

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H @HAVE_SYS_NDIR_H@

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H @HAVE_SYS_PARAM_H@

/* Define to 1 if you have the <sys/procfs.h> header file. */
#cmakedefine HAVE_SYS_PROCFS_H @HAVE_SYS_PROCFS_H@

/* Define to 1 if you have the <sys/resource.h> header file. */
#cmakedefine HAVE_SYS_RESOURCE_H @HAVE_SYS_RESOURCE_H@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H @HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H @HAVE_SYS_TIME_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H @HAVE_SYS_TYPES_H@

/* Define to 1 if you have the <time.h> header file. */
#cmakedefine HAVE_TIME_H @HAVE_TIME_H@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H @HAVE_UNISTD_H@

/* Define to 1 if you have the <wchar.h> header file. */
#cmakedefine HAVE_WCHAR_H @HAVE_WCHAR_H@

/* Define if <sys/procfs.h> has win32_pstatus_t. */
#cmakedefine HAVE_WIN32_PSTATUS_T @HAVE_WIN32_PSTATUS_T@

/* Define to 1 if you have the <windows.h> header file. */
#cmakedefine HAVE_WINDOWS_H @HAVE_WINDOWS_H@

/* Define to 1 if you have the <zlib.h> header file. */
#cmakedefine HAVE_ZLIB_H @HAVE_ZLIB_H@

/* Name of package */
#cmakedefine PACKAGE @PACKAGE@

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT @PACKAGE_BUGREPORT@

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME @PACKAGE_NAME@

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING @PACKAGE_STRING@

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME @PACKAGE_TARNAME@

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL @PACKAGE_URL@

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION @PACKAGE_VERSION@

/* The size of `char', as computed by sizeof. */
#cmakedefine SIZEOF_CHAR @SIZEOF_CHAR@

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT @SIZEOF_INT@

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG @SIZEOF_LONG@

/* The size of `long long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG_LONG @SIZEOF_LONG_LONG@

/* The size of `off_t', as computed by sizeof. */
#cmakedefine SIZEOF_OFF_T @SIZEOF_OFF_T@

/* The size of `short', as computed by sizeof. */
#cmakedefine SIZEOF_SHORT @SIZEOF_SHORT@

/* The size of `void *', as computed by sizeof. */
#cmakedefine SIZEOF_VOID_P @SIZEOF_VOID_P@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS @STDC_HEADERS@

/* Define if you can safely include both <string.h> and <strings.h>. */
#cmakedefine STRING_WITH_STRINGS @STRING_WITH_STRINGS@

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME @TIME_WITH_SYS_TIME@

/* Name of host specific header file to include in trad-core.c. */
#cmakedefine TRAD_HEADER @TRAD_HEADER@

/* Use b modifier when opening binary files? */
#cmakedefine USE_BINARY_FOPEN @USE_BINARY_FOPEN@

/* Define if we should use leading underscore on 64 bit mingw targets */
#cmakedefine USE_MINGW64_LEADING_UNDERSCORES @USE_MINGW64_LEADING_UNDERSCORES@

/* Use mmap if it's available? */
#cmakedefine USE_MMAP @USE_MMAP@

/* Define if we should default to creating read-only plt entries */
#cmakedefine USE_SECUREPLT @USE_SECUREPLT@

/* Define if we may generate symbols with ELF's STT_COMMON type */
#cmakedefine USE_STT_COMMON @USE_STT_COMMON@

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
#cmakedefine _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
#cmakedefine _GNU_SOURCE
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
#cmakedefine _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
#cmakedefine _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
#cmakedefine __EXTENSIONS__
#endif


/* Version number of package */
#cmakedefine VERSION @VERSION@

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#cmakedefine _FILE_OFFSET_BITS @_FILE_OFFSET_BITS@

/* Define for large files, on AIX-style hosts. */
#cmakedefine _LARGE_FILES @_LARGE_FILES@

/* Define to 1 if on MINIX. */
#cmakedefine _MINIX @_MINIX@

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#cmakedefine _POSIX_1_SOURCE @_POSIX_1_SOURCE@

/* Define to 1 if you need to in order for `stat' and other things to work. */
#cmakedefine _POSIX_SOURCE @_POSIX_SOURCE@

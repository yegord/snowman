dnl Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright (C) 2012-2014 Free Software Foundation, Inc.
dnl
dnl This file is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; see the file COPYING3.  If not see
dnl <http://www.gnu.org/licenses/>.
dnl

AC_PREREQ(2.59)
m4_include([version.m4])
AC_INIT([bfd], BFD_VERSION)
AC_CONFIG_SRCDIR([libbfd.c])

AC_CANONICAL_TARGET
AC_ISC_POSIX

AM_INIT_AUTOMAKE

dnl These must be called before LT_INIT, because it may want
dnl to call AC_CHECK_PROG.
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, :)

dnl Default to a non shared library.  This may be overridden by the
dnl configure option --enable-shared.
AC_DISABLE_SHARED

AC_PROG_CC
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS

LT_INIT([dlopen])

# AC_PLUGINS setting $plugins is called by ACX_LARGEFILE.
ACX_LARGEFILE

AM_CONDITIONAL(PLUGINS, test "$plugins" = "yes")

if test "$plugins" = "yes"; then
  enable_targets="$enable_targets plugin"
fi

AC_ARG_ENABLE(64-bit-bfd,
[  --enable-64-bit-bfd     64-bit support (on hosts with narrower word sizes)],
[case "${enableval}" in
  yes)  want64=true  ;;
  no)   want64=false ;;
  *)    AC_MSG_ERROR(bad value ${enableval} for 64-bit-bfd option) ;;
esac],[want64=false])dnl

AC_ARG_ENABLE(targets,
[  --enable-targets        alternative target configurations],
[case "${enableval}" in
  yes | "") AC_MSG_ERROR([enable-targets option must specify target names or 'all'])
            ;;
  no)       enable_targets= ;;
  *)        enable_targets=$enableval ;;
esac])dnl

AC_ARG_WITH(mmap,
[  --with-mmap             try using mmap for BFD input files if available],
[case "${withval}" in
  yes)  want_mmap=true ;;
  no)   want_mmap=false ;;
  *)    AC_MSG_ERROR(bad value ${withval} for BFD with-mmap option) ;;
esac],[want_mmap=false])dnl

AC_ARG_ENABLE(secureplt,
[  --enable-secureplt      Default to creating read-only plt entries],
[case "${enableval}" in
  yes)  use_secureplt=true  ;;
  no)   use_secureplt=false ;;
  *)    AC_MSG_ERROR(bad value ${enableval} for secureplt option) ;;
esac],[use_secureplt=true])dnl
if test $use_secureplt = true; then
  AC_DEFINE(USE_SECUREPLT, 1,
    [Define if we should default to creating read-only plt entries])
fi

AC_ARG_ENABLE(leading-mingw64-underscores,
  AS_HELP_STRING([--enable-leading-mingw64-underscores],
                 [Enable leading underscores on 64 bit mingw targets]),
  [],[])
AS_IF([ test x"$enable_leading_mingw64_underscores" = xyes ],
  [AC_DEFINE(USE_MINGW64_LEADING_UNDERSCORES, 1,
    [Define if we should use leading underscore on 64 bit mingw targets])])

DEBUGDIR=${libdir}/debug
AC_ARG_WITH(separate-debug-dir,
  AS_HELP_STRING([--with-separate-debug-dir=DIR],
                 [Look for global separate debug info in DIR [[default=LIBDIR/debug]]]),
[DEBUGDIR="${withval}"])
AC_SUBST(DEBUGDIR)

# Check to see if we should allow the generation of
# symbols with the ELF standard's STT_COMMON type.
AC_ARG_ENABLE(elf-stt-common,
[  --enable-elf-stt-common Allow the generation of ELF symbols with the STT_COMMON type],
[case "${enableval}" in
  yes) want_elf_stt_common=true ;;
  no)  want_elf_stt_common=false ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for ELF STT_COMMON option) ;;
 esac],
# We have to choose a default behaviour.  For native builds we could
# test whether the loader supports the STT_COMMON type, but that would
# mean that built binaries could not be exported to older systems where
# the loader does not support it. So by default we always choose to
# disable this feature.
  want_elf_stt_common=false)dnl
if test $want_elf_stt_common = true; then
  AC_DEFINE(USE_STT_COMMON, 1,
    [Define if we may generate symbols with ELF's STT_COMMON type])
fi

ACX_PKGVERSION([GNU Binutils])
ACX_BUGURL([http://www.sourceware.org/bugzilla/])

AM_BINUTILS_WARNINGS

AC_CONFIG_HEADERS(config.h:config.in)

# PR 14072
AH_VERBATIM([00_CONFIG_H_CHECK],
[/* Check that config.h is #included before system headers
    (this works only for glibc, but that should be enough).  */
#if defined(__GLIBC__) && !defined(__FreeBSD_kernel__) && !defined(__CONFIG_H__)
#  error config.h must be #included before system headers
#endif
#define __CONFIG_H__ 1])

if test -z "$target" ; then
    AC_MSG_ERROR(Unrecognized target system type; please check config.sub.)
fi

AM_MAINTAINER_MODE
AM_CONDITIONAL(GENINSRC_NEVER, false)
AM_INSTALL_LIBBFD
AC_EXEEXT

host64=false
target64=false
bfd_default_target_size=32

# host stuff:

ALL_LINGUAS="fr tr ja es sv da zh_CN ro rw vi fi id ru uk"
ZW_GNU_GETTEXT_SISTER_DIR
AM_PO_SUBDIRS

# Permit host specific settings.
. ${srcdir}/configure.host

AC_SUBST(HDEFINES)
AC_PROG_INSTALL

BFD_HOST_64BIT_LONG=0
BFD_HOST_64BIT_LONG_LONG=0
BFD_HOST_64_BIT_DEFINED=0
BFD_HOST_64_BIT=
BFD_HOST_U_64_BIT=
BFD_HOSTPTR_T="unsigned long"

AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(long)

if test "x${ac_cv_sizeof_void_p}" = "x8"; then
  host64=true
fi

if test "x${ac_cv_sizeof_long}" = "x8"; then
  BFD_HOST_64BIT_LONG=1
  test -n "${HOST_64BIT_TYPE}" || HOST_64BIT_TYPE="long"
  test -n "${HOST_U_64BIT_TYPE}" || HOST_U_64BIT_TYPE="unsigned long"
elif test "x${ac_cv_sizeof_long_long}" = "x8"; then
  BFD_HOST_64BIT_LONG_LONG=1
  test -n "${HOST_64BIT_TYPE}" || HOST_64BIT_TYPE="long long"
  test -n "${HOST_U_64BIT_TYPE}" || HOST_U_64BIT_TYPE="unsigned long long"
  if test "x${ac_cv_sizeof_void_p}" = "x8"; then
    BFD_HOSTPTR_T="unsigned long long"
  fi
fi

if test -n "${HOST_64BIT_TYPE}" -a -n "${HOST_U_64BIT_TYPE}"; then
  BFD_HOST_64_BIT_DEFINED=1
  BFD_HOST_64_BIT="${HOST_64BIT_TYPE}"
  BFD_HOST_U_64_BIT="${HOST_U_64BIT_TYPE}"
fi

AC_SUBST(BFD_HOST_64BIT_LONG)
AC_SUBST(BFD_HOST_64BIT_LONG_LONG)
AC_SUBST(BFD_HOST_64_BIT_DEFINED)
AC_SUBST(BFD_HOST_64_BIT)
AC_SUBST(BFD_HOST_U_64_BIT)
AC_SUBST(BFD_HOSTPTR_T)

BFD_CC_FOR_BUILD

AC_CHECK_HEADERS(alloca.h stddef.h string.h strings.h stdlib.h time.h unistd.h wchar.h)
AC_CHECK_HEADERS(fcntl.h sys/file.h sys/time.h sys/stat.h sys/resource.h)
GCC_HEADER_STDINT(bfd_stdint.h)
AC_HEADER_TIME
AC_HEADER_DIRENT

ACX_HEADER_STRING
AC_CHECK_FUNCS(fcntl getpagesize setitimer sysconf fdopen getuid getgid fileno)
AC_CHECK_FUNCS(strtoull getrlimit)

AC_CHECK_DECLS(basename)
AC_CHECK_DECLS(ftello)
AC_CHECK_DECLS(ftello64)
AC_CHECK_DECLS(fseeko)
AC_CHECK_DECLS(fseeko64)

BFD_BINARY_FOPEN

AC_CHECK_DECLS(ffs)
AC_CHECK_DECLS(free)
AC_CHECK_DECLS(getenv)
AC_CHECK_DECLS(malloc)
AC_CHECK_DECLS(realloc)
AC_CHECK_DECLS(stpcpy)
AC_CHECK_DECLS(strstr)
AC_CHECK_DECLS(snprintf)
AC_CHECK_DECLS(vsnprintf)
AC_CHECK_DECLS(strnlen)

# Link in zlib if we can.  This allows us to read compressed debug sections.
# This is used only by compress.c.
AM_ZLIB

# If we are configured native, pick a core file support file.
COREFILE=
COREFLAG=
CORE_HEADER=
TRAD_HEADER=
if test "${target}" = "${host}"; then
  case "${host}" in
  alpha*-*-freebsd* | alpha*-*-kfreebsd*-gnu | alpha*-*-*vms*)
	COREFILE=''
	;;
  alpha*-*-linux-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/alphalinux.h"'
	;;
  alpha*-*-netbsd* | alpha*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  alpha*-*-*)
        COREFILE=osf-core.lo
	;;
  arm-*-freebsd* | arm-*-kfreebsd*-gnu)
	COREFILE='' ;;
  arm-*-netbsd* | arm-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  arm-*-riscix)		COREFILE=trad-core.lo ;;
  hppa*-*-hpux*)	COREFILE=hpux-core.lo ;;
  hppa*-*-hiux*)	COREFILE=hpux-core.lo ;;
  hppa*-*-mpeix*)	COREFILE=hpux-core.lo ;;
  hppa*-*-bsd*)		COREFILE="hpux-core.lo hppabsd-core.lo"
			COREFLAG="-DHPUX_CORE -DHPPABSD_CORE" ;;
  hppa*-*-netbsd* | hppa*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;

  i370-*-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i370linux.h"'
	;;

changequote(,)dnl
  i[3-7]86-sequent-bsd*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/symmetry.h"'
	;;
changequote(,)dnl
  i[3-7]86-sequent-sysv4*) ;;
  i[3-7]86-sequent-sysv*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/symmetry.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-bsdi)
changequote([,])dnl
	COREFILE=
	;;
changequote(,)dnl
  i[3-7]86-*-bsd* | i[3-7]86-*-freebsd[123] | i[3-7]86-*-freebsd[123]\.* | i[3-7]86-*-freebsd4\.[01234] | i[3-7]86-*-freebsd4\.[01234]\.* | i[3-7]86-*-freebsd*aout*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i386bsd.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-freebsd* | i[3-7]86-*-kfreebsd*-gnu | i[3-7]86-*-dragonfly*)
changequote([,])dnl
	COREFILE=''
	TRAD_HEADER='"hosts/i386bsd.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-netbsd* | i[3-7]86-*-knetbsd*-gnu | i[3-7]86-*-openbsd*)
changequote([,])dnl
	COREFILE=netbsd-core.lo
	;;
changequote(,)dnl
  i[3-7]86-esix-sysv3*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/esix.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-sco3.2v5*)
changequote([,])dnl
	COREFILE=sco5-core.lo
	;;
changequote(,)dnl
  i[3-7]86-*-sco* | i[3-7]86-*-isc*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i386sco.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-mach3*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i386mach3.h"'
	;;
changequote(,)dnl
  i[3-7]86-*-linux-*)
changequote([,])dnl
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i386linux.h"'
	case "$enable_targets"-"$want64" in
	*x86_64-*linux*|*-true)
	  CORE_HEADER='"hosts/x86-64linux.h"'
	esac
	;;
changequote(,)dnl
  i[3-7]86-*-isc*)	COREFILE=trad-core.lo ;;
  i[3-7]86-*-aix*)	COREFILE=aix386-core.lo ;;
changequote([,])dnl
  i860-*-mach3* | i860-*-osf1*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/i860mach3.h"'
	;;
  mips-*-netbsd* | mips*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  mips-dec-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/decstation.h"'
	;;
  mips-sgi-irix4*)	COREFILE=irix-core.lo ;;
  mips-sgi-irix5*)	COREFILE=irix-core.lo ;;
  mips-sgi-irix6*)	COREFILE=irix-core.lo ;;
  mips-*-sysv4*)	;;
  mips-*-sysv* | mips-*-riscos*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/riscos.h"'
	;;
  mips-sony-bsd*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/news-mips.h"'
	;;
  m68*-bull*-sysv*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/dpx2.h"'
	;;
  m68*-hp-hpux*)	COREFILE=hpux-core.lo ;;
  m68*-hp-bsd*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/hp300bsd.h"'
	;;
  m68*-*-linux-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/m68klinux.h"'
	;;
  m68*-motorola-sysv*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/delta68.h"'
	;;
  m68*-sony-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/news.h"'
	;;
  m68*-*-netbsd* | m68*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  m68*-apple-aux*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/m68kaux.h"'
	;;
  m88*-*-sysv4*)
	;;
  m88*-motorola-sysv*)
	COREFILE=ptrace-core.lo
	;;
  m88*-*-mach3*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/m88kmach3.h"'
	;;
  m88*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  ns32k-pc532-mach)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/pc532mach.h"'
	;;
  ns32k-*-netbsd* | ns32k-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  rs6000-*-lynx*)
	COREFILE=lynx-core.lo
	;;
changequote(,)dnl
  rs6000-*-aix[5-9].* | powerpc-*-aix[5-9].*)
changequote([,])dnl
        COREFILE=rs6000-core.lo
	COREFLAG="$COREFLAG -DAIX_5_CORE -DAIX_CORE_DUMPX_CORE"
	;;
changequote(,)dnl
  rs6000-*-aix4.[3-9]* | powerpc-*-aix4.[3-9]*)
changequote([,])dnl
        COREFILE=rs6000-core.lo
	COREFLAG="$COREFLAG -DAIX_CORE_DUMPX_CORE"
	# Not all versions of AIX with -DAIX_CORE_DUMPX_CORE
	# have c_impl as a member of struct core_dumpx
	AC_MSG_CHECKING([for c_impl in struct core_dumpx])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <core.h>]], [[struct core_dumpx c; c.c_impl = 0;]])],[AC_DEFINE(HAVE_ST_C_IMPL, 1,
	    [Define if struct core_dumpx has member c_impl])
	  AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])
	;;
  rs6000-*-aix4*)	COREFILE=rs6000-core.lo ;;
  rs6000-*-*)		COREFILE=rs6000-core.lo ;;
  powerpc-*-aix4*)	COREFILE=rs6000-core.lo ;;
  powerpc-*-aix*)	COREFILE=rs6000-core.lo ;;
  powerpc-*-beos*)	;;
  powerpc-*-freebsd* | powerpc-*-kfreebsd*-gnu)
		        COREFILE='' ;;
  powerpc-*-netbsd*)	COREFILE=netbsd-core.lo ;;
  powerpc-*-*bsd*)	COREFILE=netbsd-core.lo ;;
  s390*-*-*)            COREFILE=trad-core.lo ;;
  sh*-*-netbsd* | sh*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  sparc-*-netbsd* | sparc*-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  tahoe-*-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/tahoe.h"'
	;;
  vax-*-netbsd* | vax-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  vax-*-ultrix2*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/vaxult2.h"'
	;;
  vax-*-ultrix*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/vaxult2.h"'
	;;
  vax-*-linux-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/vaxlinux.h"'
	;;
  vax-*-*)
	COREFILE=trad-core.lo
	TRAD_HEADER='"hosts/vaxbsd.h"'
	;;
  x86_64-*-linux*)
	CORE_HEADER='"hosts/x86-64linux.h"'
	;;
  x86_64-*-netbsd* | x86_64-*-openbsd*)
	COREFILE=netbsd-core.lo
	;;
  esac

  case "$COREFILE" in
  aix386-core.lo)	COREFLAG=-DAIX386_CORE ;;
  hppabsd-core.lo)	COREFLAG=-DHPPABSD_CORE ;;
  hpux-core.lo)		COREFLAG=-DHPUX_CORE ;;
  irix-core.lo)		COREFLAG=-DIRIX_CORE ;;
  lynx-core.lo)		COREFLAG=-DLYNX_CORE ;;
  netbsd-core.lo)	COREFLAG=-DNETBSD_CORE ;;
  osf-core.lo)		COREFLAG=-DOSF_CORE ;;
  ptrace-core.lo)	COREFLAG=-DPTRACE_CORE ;;
  rs6000-core.lo)	COREFLAG="$COREFLAG -DAIX_CORE" ;;
  sco5-core.lo)		COREFLAG="$COREFLAG -DSCO5_CORE" ;;
  trad-core.lo)		COREFLAG="$COREFLAG -DTRAD_CORE" ;;
  esac

  # ELF corefile support has several flavors, but all of
  # them use something called <sys/procfs.h>
  AC_CHECK_HEADERS(sys/procfs.h)
  if test "$ac_cv_header_sys_procfs_h" = yes; then
    BFD_HAVE_SYS_PROCFS_TYPE(prstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(prstatus32_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prstatus_t, pr_who)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prstatus32_t, pr_who)
    BFD_HAVE_SYS_PROCFS_TYPE(pstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(pxstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(pstatus32_t)
    BFD_HAVE_SYS_PROCFS_TYPE(prpsinfo_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prpsinfo_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(prpsinfo32_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(prpsinfo32_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(psinfo_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(psinfo_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(psinfo32_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(psinfo32_t, pr_pid)
    BFD_HAVE_SYS_PROCFS_TYPE(lwpstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE(lwpxstatus_t)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(lwpstatus_t, pr_context)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(lwpstatus_t, pr_reg)
    BFD_HAVE_SYS_PROCFS_TYPE_MEMBER(lwpstatus_t, pr_fpreg)
    BFD_HAVE_SYS_PROCFS_TYPE(win32_pstatus_t)
  fi
fi
AC_SUBST(COREFILE)
AC_SUBST(COREFLAG)
if test -n "$CORE_HEADER"; then
  AC_DEFINE_UNQUOTED(CORE_HEADER, $CORE_HEADER,
    [Name of host specific core header file to include in elf.c.])
fi
if test -n "$TRAD_HEADER"; then
  AC_DEFINE_UNQUOTED(TRAD_HEADER, $TRAD_HEADER,
    [Name of host specific header file to include in trad-core.c.])
fi

# Check if linker supports --as-needed and --no-as-needed options
AC_CACHE_CHECK(linker --as-needed support, bfd_cv_ld_as_needed,
	[bfd_cv_ld_as_needed=no
	if $LD --help 2>/dev/null | grep as-needed > /dev/null; then
		bfd_cv_ld_as_needed=yes
	fi
	])

LT_LIB_M

# When building a shared libbfd, link against the pic version of libiberty
# so that apps that use libbfd won't need libiberty just to satisfy any
# libbfd references.
# We can't do that if a pic libiberty is unavailable since including non-pic
# code would insert text relocations into libbfd.
SHARED_LIBADD=
SHARED_LDFLAGS=
if test "$enable_shared" = "yes"; then
changequote(,)dnl
  x=`sed -n -e 's/^[ 	]*PICFLAG[ 	]*=[ 	]*//p' < ../libiberty/Makefile | sed -n '$p'`
changequote([,])dnl
  if test -n "$x"; then
    SHARED_LIBADD="-L`pwd`/../libiberty/pic -liberty"
  fi

# More hacks to build DLLs on Windows.
  case "${host}" in
  *-*-cygwin*)
    SHARED_LDFLAGS="-no-undefined"
    SHARED_LIBADD="-L`pwd`/../libiberty -liberty -L`pwd`/../intl -lintl -lcygwin -lkernel32"
  ;;

  # Hack to build or1k-src on OSX
  or1k*-*-darwin*)
    SHARED_LIBADD="-L`pwd`/../libiberty/pic -L`pwd`/../intl -liberty -lintl"
  ;;
  esac

  if test -n "$SHARED_LIBADD"; then
    if test -n "$LIBM"; then
      if test x"$bfd_cv_ld_as_needed" = xyes; then
	# Link against libm only when needed.  Put -lc, -lm inside -Wl
	# to stop libtool reordering these options.
	SHARED_LIBADD="$SHARED_LIBADD -Wl,-lc,--as-needed,`echo $LIBM | sed 's/ /,/g'`,--no-as-needed"
      else
	SHARED_LIBADD="$SHARED_LIBADD $LIBM"
      fi
    fi
  fi
fi
AC_SUBST(SHARED_LDFLAGS)
AC_SUBST(SHARED_LIBADD)

# target stuff:

# Canonicalize the secondary target names.
if test -n "$enable_targets" ; then
    for targ in `echo $enable_targets | sed 's/,/ /g'`
    do
	result=`$ac_config_sub $targ 2>/dev/null`
	if test -n "$result" ; then
	    canon_targets="$canon_targets $result"
	else
	    # Allow targets that config.sub doesn't recognize, like "all".
	    canon_targets="$canon_targets $targ"
	fi
    done
fi

all_targets=false
defvec=
selvecs=
assocvecs=
selarchs=
TDEFINES=
for targ in $target $canon_targets
do
    if test "x$targ" = "xall"; then
        all_targets=true
	assocvecs="$assocvecs $targ_defvec $targ_selvecs"
    else
	. $srcdir/config.bfd
	if test "x$targ" = "x$target"; then
	    defvec=$targ_defvec
	fi
	selvecs="$selvecs $targ_defvec $targ_selvecs"
	selarchs="$selarchs $targ_archs"
	TDEFINES="$TDEFINES $targ_cflags"
    fi
done
AC_SUBST(TDEFINES)

# This processing still needs to be done if we're to decide properly whether
# 64-bit support needs to be compiled in.  Currently, it will be included if
# the default or any other explicitly requested target requires it; it
# will not be included on a 32-bit host if no 64-bit target is requested, and
# no "--with-64-bit-bfd" option is given, even if "--enable-targets=all" is
# used.

# uniq the default and selected vectors in all the configured targets.
f=""
for i in $selvecs ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
selvecs="$f"


# uniq the associated vectors in all the configured targets.
f=""
for i in $assocvecs ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
assocvecs="$f"


# uniq the architectures in all the configured targets.
f=""
for i in $selarchs ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
selarchs="$f"

# Target backend .o files.
tb=

elf="elf.lo elflink.lo elf-attrs.lo elf-strtab.lo
     elf-eh-frame.lo dwarf1.lo"

for vec in $selvecs
do
    target_size=32
    case "$vec" in
    # This list is alphabetized to make it easy to compare
    # with the two vector lists in targets.c.  For the same reason,
    # use one entry per line, even though this leads to long lines.
    aarch64_elf32_be_vec)	 tb="$tb elf32-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf32.lo $elf"; target_size=64 ;;
    aarch64_elf32_le_vec)	 tb="$tb elf32-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf32.lo $elf"; target_size=64 ;;
    aarch64_elf64_be_vec)	 tb="$tb elf64-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf64.lo $elf"; target_size=64 ;;
    aarch64_elf64_le_vec)	 tb="$tb elf64-aarch64.lo elfxx-aarch64.lo elf-ifunc.lo elf64.lo $elf"; target_size=64 ;;
    alpha_ecoff_le_vec)		 tb="$tb coff-alpha.lo ecoff.lo ecofflink.lo"; target_size=64 ;;
    alpha_elf64_vec)		 tb="$tb elf64-alpha.lo elf64.lo $elf"; target_size=64 ;;
    alpha_elf64_fbsd_vec)	 tb="$tb elf64-alpha.lo elf64.lo $elf"; target_size=64 ;;
    alpha_nlm32_vec)		 tb="$tb nlm32-alpha.lo nlm32.lo nlm.lo"; target_size=64 ;;
    alpha_vms_vec)		 tb="$tb vms-alpha.lo vms-misc.lo vms-lib.lo"; target_size=64 ;;
    alpha_vms_lib_txt_vec)	 tb="$tb vms-lib.lo vms-misc.lo" ;;
    am33_elf32_linux_vec)	 tb="$tb elf32-am33lin.lo elf32.lo $elf" ;;
    aout0_be_vec)		 tb="$tb aout0.lo aout32.lo" ;;
    aout64_vec)			 tb="$tb demo64.lo aout64.lo"; target_size=64 ;;
    aout_vec)			 tb="$tb host-aout.lo aout32.lo" ;;
    aout_adobe_vec)		 tb="$tb aout-adobe.lo aout32.lo" ;;
    arc_elf32_be_vec)		 tb="$tb elf32-arc.lo elf32.lo $elf" ;;
    arc_elf32_le_vec)		 tb="$tb elf32-arc.lo elf32.lo $elf" ;;
    arm_aout_be_vec)		 tb="$tb aout-arm.lo aout32.lo" ;;
    arm_aout_le_vec)		 tb="$tb aout-arm.lo aout32.lo" ;;
    arm_aout_nbsd_vec)		 tb="$tb armnetbsd.lo aout32.lo" ;;
    arm_aout_riscix_vec)	 tb="$tb aout32.lo riscix.lo" ;;
    arm_coff_be_vec)		 tb="$tb coff-arm.lo cofflink.lo " ;;
    arm_coff_le_vec)		 tb="$tb coff-arm.lo cofflink.lo " ;;
    arm_elf32_be_vec)		 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_le_vec)		 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_nacl_be_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_nacl_le_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_symbian_be_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_symbian_le_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_vxworks_be_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_elf32_vxworks_le_vec)	 tb="$tb elf32-arm.lo elf32.lo elf-nacl.lo elf-vxworks.lo $elf" ;;
    arm_pe_be_vec)		 tb="$tb pe-arm.lo peigen.lo cofflink.lo " ;;
    arm_pe_le_vec)		 tb="$tb pe-arm.lo peigen.lo cofflink.lo " ;;
    arm_pe_epoc_be_vec)		 tb="$tb epoc-pe-arm.lo peigen.lo cofflink.lo " ;;
    arm_pe_epoc_le_vec)		 tb="$tb epoc-pe-arm.lo peigen.lo cofflink.lo " ;;
    arm_pe_wince_be_vec)	 tb="$tb pe-arm-wince.lo pe-arm.lo peigen.lo cofflink.lo " ;;
    arm_pe_wince_le_vec)	 tb="$tb pe-arm-wince.lo pe-arm.lo peigen.lo cofflink.lo " ;;
    arm_pei_be_vec)		 tb="$tb pei-arm.lo peigen.lo cofflink.lo " ;;
    arm_pei_le_vec)		 tb="$tb pei-arm.lo peigen.lo cofflink.lo " ;;
    arm_pei_epoc_be_vec)	 tb="$tb epoc-pei-arm.lo peigen.lo cofflink.lo " ;;
    arm_pei_epoc_le_vec)	 tb="$tb epoc-pei-arm.lo peigen.lo cofflink.lo " ;;
    arm_pei_wince_be_vec)	 tb="$tb pei-arm-wince.lo pei-arm.lo peigen.lo cofflink.lo " ;;
    arm_pei_wince_le_vec)	 tb="$tb pei-arm-wince.lo pei-arm.lo peigen.lo cofflink.lo " ;;
    avr_elf32_vec)		 tb="$tb elf32-avr.lo elf32.lo $elf" ;;
    bfin_elf32_vec)		 tb="$tb elf32-bfin.lo elf32.lo $elf" ;;
    bfin_elf32_fdpic_vec)	 tb="$tb elf32-bfin.lo elf32.lo $elf" ;;
    bout_be_vec)		 tb="$tb bout.lo aout32.lo" ;;
    bout_le_vec)		 tb="$tb bout.lo aout32.lo" ;;
    cr16_elf32_vec)		 tb="$tb elf32-cr16.lo elf32.lo $elf" ;;
    cr16c_elf32_vec)		 tb="$tb elf32-cr16c.lo elf32.lo $elf" ;;
    cris_aout_vec)		 tb="$tb aout-cris.lo" ;;
    cris_elf32_vec)		 tb="$tb elf32-cris.lo elf32.lo $elf" ;;
    cris_elf32_us_vec)		 tb="$tb elf32-cris.lo elf32.lo $elf" ;;
    crx_elf32_vec)		 tb="$tb elf32-crx.lo elf32.lo $elf" ;;
    d10v_elf32_vec)		 tb="$tb elf32-d10v.lo elf32.lo $elf" ;;
    d30v_elf32_vec)		 tb="$tb elf32-d30v.lo elf32.lo $elf" ;;
    dlx_elf32_be_vec)		 tb="$tb elf32-dlx.lo elf32.lo $elf" ;;
    elf32_be_vec)		 tb="$tb elf32-gen.lo elf32.lo $elf" ;;
    elf32_le_vec)		 tb="$tb elf32-gen.lo elf32.lo $elf" ;;
    elf64_be_vec)		 tb="$tb elf64-gen.lo elf64.lo $elf"; target_size=64 ;;
    elf64_le_vec)		 tb="$tb elf64-gen.lo elf64.lo $elf"; target_size=64 ;;
    epiphany_elf32_vec)		 tb="$tb elf32-epiphany.lo elf32.lo $elf" ;;
    fr30_elf32_vec)		 tb="$tb elf32-fr30.lo elf32.lo $elf" ;;
    frv_elf32_vec)		 tb="$tb elf32-frv.lo elf32.lo $elf" ;;
    frv_elf32_fdpic_vec)	 tb="$tb elf32-frv.lo elf32.lo $elf" ;;
    h8300_coff_vec)		 tb="$tb coff-h8300.lo reloc16.lo" ;;
    h8300_elf32_vec)		 tb="$tb elf32-h8300.lo elf32.lo $elf" ;;
    h8500_coff_vec)		 tb="$tb coff-h8500.lo reloc16.lo" ;;
    hppa_elf32_vec)		 tb="$tb elf32-hppa.lo elf32.lo $elf" ;;
    hppa_elf32_linux_vec)	 tb="$tb elf32-hppa.lo elf32.lo $elf" ;;
    hppa_elf32_nbsd_vec)	 tb="$tb elf32-hppa.lo elf32.lo $elf" ;;
    hppa_elf64_vec)		 tb="$tb elf64-hppa.lo elf64.lo $elf"; target_size=64 ;;
    hppa_elf64_linux_vec)	 tb="$tb elf64-hppa.lo elf64.lo $elf"; target_size=64 ;;
    hppa_som_vec)		 tb="$tb som.lo" ;;
    i370_elf32_vec)		 tb="$tb elf32-i370.lo elf32.lo $elf" ;;
    i386_aout_vec)		 tb="$tb i386aout.lo aout32.lo" ;;
    i386_aout_bsd_vec)		 tb="$tb i386bsd.lo aout32.lo" ;;
    i386_aout_dynix_vec)	 tb="$tb i386dynix.lo aout32.lo" ;;
    i386_aout_fbsd_vec)		 tb="$tb i386freebsd.lo aout32.lo" ;;
    i386_aout_linux_vec)	 tb="$tb i386linux.lo aout32.lo" ;;
    i386_aout_lynx_vec)		 tb="$tb i386lynx.lo lynx-core.lo aout32.lo" ;;
    i386_aout_mach3_vec)	 tb="$tb i386mach3.lo aout32.lo" ;;
    i386_aout_nbsd_vec)		 tb="$tb i386netbsd.lo aout32.lo" ;;
    i386_aout_os9k_vec)		 tb="$tb i386os9k.lo aout32.lo" ;;
    i386_coff_vec)		 tb="$tb coff-i386.lo cofflink.lo" ;;
    i386_coff_go32_vec)		 tb="$tb coff-go32.lo cofflink.lo" ;;
    i386_coff_go32stubbed_vec)	 tb="$tb coff-stgo32.lo cofflink.lo" ;;
    i386_coff_lynx_vec)		 tb="$tb cf-i386lynx.lo cofflink.lo lynx-core.lo" ;;
    i386_elf32_vec)		 tb="$tb elf32-i386.lo elf-ifunc.lo elf-nacl.lo elf-vxworks.lo elf32.lo $elf" ;;
    i386_elf32_fbsd_vec)	 tb="$tb elf32-i386.lo elf-ifunc.lo elf-nacl.lo elf-vxworks.lo elf32.lo $elf" ;;
    i386_elf32_nacl_vec)	 tb="$tb elf32-i386.lo elf-ifunc.lo elf-nacl.lo elf-vxworks.lo elf32.lo $elf" ;;
    i386_elf32_sol2_vec)	 tb="$tb elf32-i386.lo elf-ifunc.lo elf-nacl.lo elf-vxworks.lo elf32.lo $elf" ;;
    i386_elf32_vxworks_vec)	 tb="$tb elf32-i386.lo elf-ifunc.lo elf-nacl.lo elf-vxworks.lo elf32.lo $elf" ;;
    i386_mach_o_vec)		 tb="$tb mach-o-i386.lo" ;;
    i386_msdos_vec)		 tb="$tb i386msdos.lo" ;;
    i386_nlm32_vec)		 tb="$tb nlm32-i386.lo nlm32.lo nlm.lo" ;;
    i386_pe_vec)		 tb="$tb pe-i386.lo peigen.lo cofflink.lo" ;;
    i386_pei_vec)		 tb="$tb pei-i386.lo peigen.lo cofflink.lo" ;;
    i860_coff_vec)		 tb="$tb coff-i860.lo cofflink.lo" ;;
    i860_elf32_vec)		 tb="$tb elf32-i860.lo elf32.lo $elf" ;;
    i860_elf32_le_vec)		 tb="$tb elf32-i860.lo elf32.lo $elf" ;;
    i960_elf32_vec)		 tb="$tb elf32-i960.lo elf32.lo $elf" ;;
    ia64_elf32_be_vec)		 tb="$tb elf32-ia64.lo elfxx-ia64.lo elf32.lo $elf" ;;
    ia64_elf32_hpux_be_vec)	 tb="$tb elf32-ia64.lo elfxx-ia64.lo elf32.lo $elf" ;;
    ia64_elf64_be_vec)		 tb="$tb elf64-ia64.lo elfxx-ia64.lo elf64.lo $elf"; target_size=64 ;;
    ia64_elf64_le_vec)		 tb="$tb elf64-ia64.lo elfxx-ia64.lo elf64.lo $elf"; target_size=64 ;;
    ia64_elf64_hpux_be_vec)	 tb="$tb elf64-ia64.lo elfxx-ia64.lo elf64.lo $elf"; target_size=64 ;;
    ia64_elf64_vms_vec)		 tb="$tb elf64-ia64-vms.lo elf64-ia64.lo elfxx-ia64.lo elf64.lo vms-lib.lo vms-misc.lo $elf"; target_size=64 ;;
    ia64_pei_vec)		 tb="$tb pei-ia64.lo pepigen.lo cofflink.lo"; target_size=64 ;;
    icoff_be_vec)		 tb="$tb coff-i960.lo cofflink.lo" ;;
    icoff_le_vec)		 tb="$tb coff-i960.lo cofflink.lo" ;;
    ieee_vec)			 tb="$tb ieee.lo" ;;
    ip2k_elf32_vec)		 tb="$tb elf32-ip2k.lo elf32.lo $elf" ;;
    iq2000_elf32_vec)		 tb="$tb elf32-iq2000.lo elf32.lo $elf" ;;
    k1om_elf64_vec)		 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    k1om_elf64_fbsd_vec)	 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    l1om_elf64_vec)		 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    l1om_elf64_fbsd_vec)	 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    lm32_elf32_vec)		 tb="$tb elf32-lm32.lo elf32.lo $elf" ;;
    lm32_elf32_fdpic_vec)	 tb="$tb elf32-lm32.lo elf32.lo $elf" ;;
    m32c_elf32_vec)		 tb="$tb elf32-m32c.lo elf32.lo $elf" ;;
    m32r_elf32_vec)		 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m32r_elf32_le_vec)		 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m32r_elf32_linux_vec)	 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m32r_elf32_linux_le_vec)	 tb="$tb elf32-m32r.lo elf32.lo $elf" ;;
    m68hc11_elf32_vec)		 tb="$tb elf32-m68hc11.lo elf32-m68hc1x.lo elf32.lo $elf" ;;
    m68hc12_elf32_vec)		 tb="$tb elf32-m68hc12.lo elf32-m68hc1x.lo elf32.lo $elf" ;;
    m68k_aout_4knbsd_vec)	 tb="$tb m68k4knetbsd.lo aout32.lo" ;;
    m68k_aout_hp300bsd_vec)	 tb="$tb hp300bsd.lo aout32.lo" ;;
    m68k_aout_hp300hpux_vec)	 tb="$tb hp300hpux.lo aout32.lo" ;;
    m68k_aout_linux_vec)	 tb="$tb m68klinux.lo aout32.lo" ;;
    m68k_aout_nbsd_vec)		 tb="$tb m68knetbsd.lo aout32.lo" ;;
    m68k_aout_newsos3_vec)	 tb="$tb newsos3.lo aout32.lo" ;;
    m68k_coff_vec)		 tb="$tb coff-m68k.lo cofflink.lo" ;;
    m68k_coff_apollo_vec)	 tb="$tb coff-apollo.lo" ;;
    m68k_coff_aux_vec)		 tb="$tb coff-aux.lo coff-m68k.lo cofflink.lo" ;;
    m68k_coff_sysv_vec)		 tb="$tb coff-svm68k.lo cofflink.lo" ;;
    m68k_coff_un_vec)		 tb="$tb coff-u68k.lo coff-m68k.lo cofflink.lo" ;;
    m68k_elf32_vec)		 tb="$tb elf32-m68k.lo elf32.lo $elf" ;;
    m68k_versados_vec)		 tb="$tb versados.lo" ;;
    m88k_aout_mach3_vec)	 tb="$tb m88kmach3.lo aout32.lo" ;;
    m88k_aout_obsd_vec)		 tb="$tb m88kopenbsd.lo aout32.lo" ;;
    m88k_coff_bcs_vec)		 tb="$tb coff-m88k.lo" ;;
    m88k_elf32_vec)		 tb="$tb elf32-m88k.lo elf32.lo $elf" ;;
    mach_o_be_vec)		 tb="$tb mach-o.lo" ;;
    mach_o_le_vec)		 tb="$tb mach-o.lo" ;;
    mach_o_fat_vec)		 tb="$tb mach-o.lo" ;;
    mcore_elf32_be_vec)		 tb="$tb elf32-mcore.lo elf32.lo $elf" ;;
    mcore_elf32_le_vec)		 tb="$tb elf32-mcore.lo elf32.lo $elf" ;;
    mcore_pe_be_vec)		 tb="$tb pe-mcore.lo peigen.lo cofflink.lo" ;;
    mcore_pe_le_vec)		 tb="$tb pe-mcore.lo peigen.lo cofflink.lo" ;;
    mcore_pei_be_vec)		 tb="$tb pei-mcore.lo peigen.lo cofflink.lo" ;;
    mcore_pei_le_vec)		 tb="$tb pei-mcore.lo peigen.lo cofflink.lo" ;;
    mep_elf32_vec)		 tb="$tb elf32-mep.lo elf32.lo $elf" ;;
    mep_elf32_le_vec)		 tb="$tb elf32-mep.lo elf32.lo $elf" ;;
    metag_elf32_vec)		 tb="$tb elf32-metag.lo elf32.lo $elf" ;;
    microblaze_elf32_vec)	 tb="$tb elf32-microblaze.lo elf32.lo $elf" ;;
    microblaze_elf32_le_vec)	 tb="$tb elf32-microblaze.lo elf32.lo $elf" ;;
    mips_ecoff_be_vec)		 tb="$tb coff-mips.lo ecoff.lo ecofflink.lo" ;;
    mips_ecoff_le_vec)		 tb="$tb coff-mips.lo ecoff.lo ecofflink.lo" ;;
    mips_ecoff_bele_vec)	 tb="$tb coff-mips.lo ecoff.lo ecofflink.lo" ;;
    mips_elf32_be_vec)		 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf32_le_vec)		 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf32_n_be_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf32_n_le_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf32_ntrad_be_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf32_ntrad_le_vec)	 tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf32_ntradfbsd_be_vec) tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf32_ntradfbsd_le_vec) tb="$tb elfn32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf32_trad_be_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf32_trad_le_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf32_tradfbsd_be_vec)  tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf32_tradfbsd_le_vec)  tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf32_vxworks_be_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf32_vxworks_le_vec)	 tb="$tb elf32-mips.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo" ;;
    mips_elf64_be_vec)		 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf64_le_vec)		 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf64_trad_be_vec)	 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf64_trad_le_vec)	 tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf64_tradfbsd_be_vec)  tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_elf64_tradfbsd_le_vec)  tb="$tb elf64-mips.lo elf64.lo elfxx-mips.lo elf-vxworks.lo elf32.lo $elf ecofflink.lo"; target_size=64 ;;
    mips_pe_le_vec)		 tb="$tb pe-mips.lo peigen.lo cofflink.lo" ;;
    mips_pei_le_vec)		 tb="$tb pei-mips.lo peigen.lo cofflink.lo" ;;
    mmix_elf64_vec)		 tb="$tb elf64-mmix.lo elf64.lo $elf" target_size=64 ;;
    mmix_mmo_vec)			 tb="$tb mmo.lo" target_size=64 ;;
    mn10200_elf32_vec)		 tb="$tb elf-m10200.lo elf32.lo $elf" ;;
    mn10300_elf32_vec)		 tb="$tb elf-m10300.lo elf32.lo $elf" ;;
    moxie_elf32_be_vec)		 tb="$tb elf32-moxie.lo elf32.lo $elf" ;;
    moxie_elf32_le_vec)		 tb="$tb elf32-moxie.lo elf32.lo $elf" ;;
    msp430_elf32_vec)		 tb="$tb elf32-msp430.lo elf32.lo $elf" ;;
    msp430_elf32_ti_vec)	 tb="$tb elf32-msp430.lo elf32.lo $elf" ;;
    mt_elf32_vec)		 tb="$tb elf32-mt.lo elf32.lo $elf" ;;
    nds32_elf32_be_vec)		 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nds32_elf32_le_vec)		 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nds32_elf32_linux_be_vec)	 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nds32_elf32_linux_le_vec)	 tb="$tb elf32-nds32.lo elf32.lo $elf" ;;
    nios2_elf32_be_vec)		 tb="$tb elf32-nios2.lo elf32.lo $elf" ;;
    nios2_elf32_le_vec)		 tb="$tb elf32-nios2.lo elf32.lo $elf" ;;
    ns32k_aout_pc532mach_vec)	 tb="$tb pc532-mach.lo aout-ns32k.lo" ;;
    ns32k_aout_pc532nbsd_vec)	 tb="$tb ns32knetbsd.lo aout-ns32k.lo" ;;
    or1k_elf32_vec)		 tb="$tb elf32-or1k.lo elf32.lo $elf" ;;
    pdp11_aout_vec)		 tb="$tb pdp11.lo" ;;
    pef_vec)			 tb="$tb pef.lo" ;;
    pef_xlib_vec)		 tb="$tb pef.lo" ;;
    pj_elf32_vec)		 tb="$tb elf32-pj.lo elf32.lo $elf" ;;
    pj_elf32_le_vec)		 tb="$tb elf32-pj.lo elf32.lo $elf" ;;
    plugin_vec)			 tb="$tb plugin.lo" ;;
    powerpc_boot_vec)		 tb="$tb ppcboot.lo" ;;
    powerpc_elf32_vec)		 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf32_le_vec)	 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf32_fbsd_vec)	 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf32_vxworks_vec)	 tb="$tb elf32-ppc.lo elf-vxworks.lo elf32.lo $elf" ;;
    powerpc_elf64_vec)		 tb="$tb elf64-ppc.lo elf64-gen.lo elf64.lo $elf"; target_size=64 ;;
    powerpc_elf64_le_vec)	 tb="$tb elf64-ppc.lo elf64-gen.lo elf64.lo $elf" target_size=64 ;;
    powerpc_elf64_fbsd_vec)	 tb="$tb elf64-ppc.lo elf64-gen.lo elf64.lo $elf" target_size=64 ;;
    powerpc_nlm32_vec)		 tb="$tb nlm32-ppc.lo nlm32.lo nlm.lo" ;;
    powerpc_pe_vec)		 tb="$tb pe-ppc.lo peigen.lo cofflink.lo" ;;
    powerpc_pe_le_vec)		 tb="$tb pe-ppc.lo peigen.lo cofflink.lo" ;;
    powerpc_pei_vec)		 tb="$tb pei-ppc.lo peigen.lo cofflink.lo" ;;
    powerpc_pei_le_vec)		 tb="$tb pei-ppc.lo peigen.lo cofflink.lo" ;;
    powerpc_xcoff_vec)		 tb="$tb coff-rs6000.lo xcofflink.lo" ;;
    rl78_elf32_vec)		 tb="$tb elf32-rl78.lo elf32.lo $elf" ;;
    rs6000_xcoff64_vec)		 tb="$tb coff64-rs6000.lo xcofflink.lo aix5ppc-core.lo"; target_size=64 ;;
    rs6000_xcoff64_aix_vec)	 tb="$tb coff64-rs6000.lo xcofflink.lo aix5ppc-core.lo"; target_size=64 ;;
    rs6000_xcoff_vec)		 tb="$tb coff-rs6000.lo xcofflink.lo" ;;
    rx_elf32_be_vec)		 tb="$tb elf32-rx.lo elf32.lo $elf" ;;
    rx_elf32_be_ns_vec)		 tb="$tb elf32-rx.lo elf32.lo $elf" ;;
    rx_elf32_le_vec)		 tb="$tb elf32-rx.lo elf32.lo $elf" ;;
    s390_elf32_vec)		 tb="$tb elf32-s390.lo elf32.lo $elf" ;;
    s390_elf64_vec)		 tb="$tb elf64-s390.lo elf64.lo $elf"; target_size=64 ;;
    score_elf32_be_vec)		 tb="$tb elf32-score.lo elf32-score7.lo elf32.lo $elf"; want64=true; target_size=64 ;;
    score_elf32_le_vec)		 tb="$tb elf32-score.lo elf32-score7.lo elf32.lo $elf"; want64=true; target_size=64 ;;
    # FIXME: We include cofflink.lo not because it's needed for
    # sh64_elf32[_le]_vec, but because we include sh_elf32[_le]_vec
    # which needs it but does not list it.  Should be fixed in right place.
    sh64_elf32_vec)		 tb="$tb elf32-sh64.lo elf32-sh64-com.lo elf-vxworks.lo elf32.lo $elf cofflink.lo" target_size=64 ;;
    sh64_elf32_le_vec)		 tb="$tb elf32-sh64.lo elf32-sh64-com.lo elf-vxworks.lo elf32.lo $elf cofflink.lo" target_size=64 ;;
    sh64_elf32_linux_vec)	 tb="$tb elf32-sh64.lo elf32-sh64-com.lo elf-vxworks.lo elf32.lo $elf cofflink.lo" target_size=64 ;;
    sh64_elf32_linux_be_vec)	 tb="$tb elf32-sh64.lo elf32-sh64-com.lo elf-vxworks.lo elf32.lo $elf cofflink.lo" target_size=64 ;;
    sh64_elf32_nbsd_vec)	 tb="$tb elf32-sh64.lo elf32-sh64-com.lo elf-vxworks.lo elf32.lo $elf cofflink.lo" ;;
    sh64_elf32_nbsd_le_vec)	 tb="$tb elf32-sh64.lo elf32-sh64-com.lo elf-vxworks.lo elf32.lo $elf cofflink.lo" ;;
    sh64_elf64_vec)		 tb="$tb elf64-sh64.lo elf64.lo $elf" target_size=64 ;;
    sh64_elf64_le_vec)		 tb="$tb elf64-sh64.lo elf64.lo $elf" target_size=64 ;;
    sh64_elf64_linux_vec)	 tb="$tb elf64-sh64.lo elf64.lo $elf" target_size=64 ;;
    sh64_elf64_linux_be_vec)	 tb="$tb elf64-sh64.lo elf64.lo $elf" target_size=64 ;;
    sh64_elf64_nbsd_vec)	 tb="$tb elf64-sh64.lo elf64.lo $elf" target_size=64 ;;
    sh64_elf64_nbsd_le_vec)	 tb="$tb elf64-sh64.lo elf64.lo $elf" target_size=64 ;;
    sh_coff_vec)		 tb="$tb coff-sh.lo cofflink.lo" ;;
    sh_coff_le_vec)		 tb="$tb coff-sh.lo cofflink.lo" ;;
    sh_coff_small_vec)		 tb="$tb coff-sh.lo cofflink.lo" ;;
    sh_coff_small_le_vec)	 tb="$tb coff-sh.lo cofflink.lo" ;;
    sh_elf32_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo" ;;
    sh_elf32_le_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo" ;;
    sh_elf32_fdpic_be_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo cofflink.lo" ;;
    sh_elf32_fdpic_le_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo cofflink.lo" ;;
    sh_elf32_linux_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo cofflink.lo" ;;
    sh_elf32_linux_be_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo cofflink.lo" ;;
    sh_elf32_nbsd_vec)		 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo cofflink.lo" ;;
    sh_elf32_nbsd_le_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo cofflink.lo" ;;
    sh_elf32_symbian_le_vec)	 tb="$tb elf32-sh-symbian.lo elf32-sh64-com.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo" ;;
    sh_elf32_vxworks_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo" ;;
    sh_elf32_vxworks_le_vec)	 tb="$tb elf32-sh.lo elf-vxworks.lo elf32.lo $elf coff-sh.lo" ;;
    sh_pe_le_vec)		 tb="$tb pe-sh.lo coff-sh.lo peigen.lo cofflink.lo" ;;
    sh_pei_le_vec)		 tb="$tb pei-sh.lo coff-sh.lo peigen.lo cofflink.lo" ;;
    sparc_aout_le_vec)		 tb="$tb aout-sparcle.lo aout32.lo" ;;
    sparc_aout_linux_vec)	 tb="$tb sparclinux.lo aout32.lo" ;;
    sparc_aout_lynx_vec)	 tb="$tb sparclynx.lo lynx-core.lo aout32.lo" ;;
    sparc_aout_nbsd_vec)	 tb="$tb sparcnetbsd.lo aout32.lo" ;;
    sparc_aout_sunos_be_vec)	 tb="$tb sunos.lo aout32.lo" ;;
    sparc_coff_vec)		 tb="$tb coff-sparc.lo" ;;
    sparc_coff_lynx_vec)	 tb="$tb cf-sparclynx.lo lynx-core.lo" ;;
    sparc_elf32_vec)		 tb="$tb elf32-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf32.lo $elf" ;;
    sparc_elf32_sol2_vec)	 tb="$tb elf32-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf32.lo $elf" ;;
    sparc_elf32_vxworks_vec)	 tb="$tb elf32-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf32.lo $elf" ;;
    sparc_elf64_vec)		 tb="$tb elf64-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf64.lo $elf"; target_size=64 ;;
    sparc_elf64_fbsd_vec)	 tb="$tb elf64-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf64.lo $elf"; target_size=64 ;;
    sparc_elf64_sol2_vec)	 tb="$tb elf64-sparc.lo elfxx-sparc.lo elf-vxworks.lo elf64.lo $elf"; target_size=64 ;;
    sparc_nlm32_vec)		 tb="$tb nlm32-sparc.lo nlm32.lo nlm.lo" ;;
    spu_elf32_vec)		 tb="$tb elf32-spu.lo elf32.lo $elf" ;;
    sym_vec)			 tb="$tb xsym.lo" ;;
    tic30_aout_vec)		 tb="$tb aout-tic30.lo" ;;
    tic30_coff_vec)		 tb="$tb coff-tic30.lo" ;;
    tic4x_coff0_vec)		 tb="$tb coff-tic4x.lo" ;;
    tic4x_coff0_beh_vec)	 tb="$tb coff-tic4x.lo" ;;
    tic4x_coff1_vec)		 tb="$tb coff-tic4x.lo" ;;
    tic4x_coff1_beh_vec)	 tb="$tb coff-tic4x.lo" ;;
    tic4x_coff2_vec)		 tb="$tb coff-tic4x.lo" ;;
    tic4x_coff2_beh_vec)	 tb="$tb coff-tic4x.lo" ;;
    tic54x_coff0_vec)		 tb="$tb coff-tic54x.lo" ;;
    tic54x_coff0_beh_vec)	 tb="$tb coff-tic54x.lo" ;;
    tic54x_coff1_vec)		 tb="$tb coff-tic54x.lo" ;;
    tic54x_coff1_beh_vec)	 tb="$tb coff-tic54x.lo" ;;
    tic54x_coff2_vec)		 tb="$tb coff-tic54x.lo" ;;
    tic54x_coff2_beh_vec)	 tb="$tb coff-tic54x.lo" ;;
    tic6x_elf32_be_vec)		 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_le_vec)		 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_c6000_be_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_c6000_le_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_linux_be_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic6x_elf32_linux_le_vec)	 tb="$tb elf32-tic6x.lo elf32.lo $elf" ;;
    tic80_coff_vec)		 tb="$tb coff-tic80.lo cofflink.lo" ;;
    tilegx_elf32_be_vec)	 tb="$tb elf32-tilegx.lo elfxx-tilegx.lo elf32.lo $elf" ; target_size=32 ;;
    tilegx_elf32_le_vec)	 tb="$tb elf32-tilegx.lo elfxx-tilegx.lo elf32.lo $elf" ; target_size=32 ;;
    tilegx_elf64_be_vec)	 tb="$tb elf64-tilegx.lo elfxx-tilegx.lo elf64.lo $elf" ; target_size=64 ;;
    tilegx_elf64_le_vec)	 tb="$tb elf64-tilegx.lo elfxx-tilegx.lo elf64.lo $elf" ; target_size=64 ;;
    tilepro_elf32_vec)		 tb="$tb elf32-tilepro.lo elf32.lo $elf" ;;
    v800_elf32_vec)		 tb="$tb elf32-v850.lo elf32.lo $elf" ;;
    v850_elf32_vec)		 tb="$tb elf32-v850.lo elf32.lo $elf" ;;
    vax_aout_1knbsd_vec)	 tb="$tb vax1knetbsd.lo aout32.lo" ;;
    vax_aout_bsd_vec)		 tb="$tb vaxbsd.lo aout32.lo" ;;
    vax_aout_nbsd_vec)		 tb="$tb vaxnetbsd.lo aout32.lo" ;;
    vax_elf32_vec)		 tb="$tb elf32-vax.lo elf32.lo $elf" ;;
    w65_coff_vec)		 tb="$tb coff-w65.lo reloc16.lo" ;;
    we32k_coff_vec)		 tb="$tb coff-we32k.lo" ;;
    x86_64_coff_vec)		 tb="$tb coff-x86_64.lo cofflink.lo"; target_size=64 ;;
    x86_64_elf32_vec)		 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo elf32.lo $elf"; target_size=64 ;;
    x86_64_elf32_nacl_vec)	 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo elf32.lo $elf"; target_size=64 ;;
    x86_64_elf64_vec)		 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    x86_64_elf64_fbsd_vec)	 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    x86_64_elf64_nacl_vec)	 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    x86_64_elf64_sol2_vec)	 tb="$tb elf64-x86-64.lo elf-ifunc.lo elf-nacl.lo elf64.lo $elf"; target_size=64 ;;
    x86_64_mach_o_vec)		 tb="$tb mach-o-x86-64.lo" ;;
    x86_64_pe_vec)		 tb="$tb pe-x86_64.lo pex64igen.lo cofflink.lo"; target_size=64 ;;
    x86_64_pe_be_vec)		 tb="$tb pe-x86_64.lo pex64igen.lo cofflink.lo"; target_size=64 ;;
    x86_64_pei_vec)		 tb="$tb pei-x86_64.lo pex64igen.lo cofflink.lo"; target_size=64 ;;
    xc16x_elf32_vec)		 tb="$tb elf32-xc16x.lo elf32.lo $elf" ;;
    xgate_elf32_vec)		 tb="$tb elf32-xgate.lo elf32.lo $elf" ;;
    xstormy16_elf32_vec)	 tb="$tb elf32-xstormy16.lo elf32.lo $elf" ;;
    xtensa_elf32_be_vec)	 tb="$tb xtensa-isa.lo xtensa-modules.lo elf32-xtensa.lo elf32.lo $elf" ;;
    xtensa_elf32_le_vec)	 tb="$tb xtensa-isa.lo xtensa-modules.lo elf32-xtensa.lo elf32.lo $elf" ;;
    z80_coff_vec)		 tb="$tb coff-z80.lo reloc16.lo" ;;
    z8k_coff_vec)		 tb="$tb coff-z8k.lo reloc16.lo cofflink.lo" ;;

    # These appear out of order in targets.c
    srec_vec)			 tb="$tb srec.lo" ;;
    symbolsrec_vec)		 tb="$tb srec.lo" ;;
    tekhex_vec)			 tb="$tb tekhex.lo" ;;
    core_cisco_be_vec)		 tb="$tb cisco-core.lo" ;;
    core_cisco_le_vec)		 tb="$tb cisco-core.lo" ;;

    "")				 ;;
    *) AC_MSG_ERROR(*** unknown target vector $vec) ;;
    esac

    if test ${target_size} = 64; then
	target64=true
    fi
    if test x"${vec}" = x"${defvec}"; then
	bfd_default_target_size=${target_size}
    fi
done

# Target architecture .o files.
# A couple of CPUs use shorter file names to avoid problems on DOS
# filesystems.
ta=`echo $selarchs | sed -e s/bfd_/cpu-/g -e s/_arch/.lo/g -e s/mn10200/m10200/ -e s/mn10300/m10300/`

# Weed out duplicate .o files.
f=""
for i in $tb ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
tb="$f"

f=""
for i in $ta ; do
    case " $f " in
    *" $i "*) ;;
    *) f="$f $i" ;;
    esac
done
ta="$f"

bfd_backends="$tb"
bfd_machines="$ta"

if test x${all_targets} = xtrue ; then
  bfd_backends="${bfd_backends}"' $(ALL_BACKENDS)'
  bfd_machines="${bfd_machines}"' $(ALL_MACHINES)'
  selvecs=
  havevecs=-DHAVE_all_vecs
  selarchs=
  test -n "$assocvecs" &&
    assocvecs=`echo $assocvecs | sed -e 's/^/\&/' -e 's/ \(.\)/,\&\1/g'`
else	# all_targets is true
  # Only set these if they will be nonempty, for the clever echo.
  havevecs=
  assocvecs=
  test -n "$selvecs" &&
    havevecs=`echo $selvecs | sed -e 's/^/-DHAVE_/' -e 's/ \(.\)/ -DHAVE_\1/g'`
  test -n "$selvecs" &&
    selvecs=`echo $selvecs | sed -e 's/^/\&/' -e 's/ \(.\)/,\&\1/g'`
  test -n "$selarchs" &&
    selarchs=`echo $selarchs | sed -e 's/^/\&/' -e 's/ \(.\)/,\&\1/g'`
fi	# all_targets is true

case ${host64}-${target64}-${want64} in
  *true*)
    wordsize=64
    bfd64_libs='$(BFD64_LIBS)'
    all_backends='$(BFD64_BACKENDS) $(BFD32_BACKENDS)'
    if test $BFD_HOST_64_BIT_DEFINED = 0; then
      AC_MSG_WARN([You have requested a 64 bit BFD configuration, but])
      AC_MSG_WARN([your compiler may not have a 64 bit integral type])
    fi
    if test -n "$GCC" ; then
       bad_64bit_gcc=no;
       AC_MSG_CHECKING([for gcc version with buggy 64-bit support])
       # Add more tests for gcc versions with non-working 64-bit support here.
       AC_EGREP_CPP([: 2 : 91 : 1 :],[:__GNUC__:__GNUC_MINOR__:__i386__:],
		    bad_64bit_gcc=yes;
		    AC_MSG_RESULT([yes: egcs-1.1.2 on ix86 spotted]),
		    AC_MSG_RESULT(no))
       if test $bad_64bit_gcc = yes ; then
	 AC_MSG_ERROR([A newer version of gcc is needed for the requested 64-bit BFD configuration])
       fi
    fi
    ;;
  false-false-false)
    wordsize=32
    all_backends='$(BFD32_BACKENDS)'
    ;;
esac

AC_SUBST(wordsize)
AC_SUBST(bfd64_libs)
AC_SUBST(all_backends)
AC_SUBST(bfd_backends)
AC_SUBST(bfd_machines)
AC_SUBST(bfd_default_target_size)

if test "$plugins" = "yes"; then
  supports_plugins=1
else
  supports_plugins=0
fi
AC_SUBST(supports_plugins)
AC_SUBST(lt_cv_dlopen_libs)

# Determine the host dependant file_ptr a.k.a. off_t type.  In order
# prefer: off64_t - if ftello64 and fseeko64, off_t - if ftello and
# fseeko, long.  This assumes that sizeof off_t is .ge. sizeof long.
# Hopefully a reasonable assumption since fseeko et.al. should be
# upward compatible.
AC_CHECK_FUNCS(ftello ftello64 fseeko fseeko64 fopen64)
if test x"$ac_cv_func_ftello" = xyes -a x"$ac_cv_func_fseeko" = xyes; then
    AC_CHECK_SIZEOF(off_t)
fi
AC_MSG_CHECKING([file_ptr type])
bfd_file_ptr="long"
bfd_ufile_ptr="unsigned long"
if test x"$ac_cv_func_ftello64" = xyes -a x"$ac_cv_func_fseeko64" = xyes \
	-o x"${ac_cv_sizeof_off_t}" = x8; then
    bfd_file_ptr=BFD_HOST_64_BIT
    bfd_ufile_ptr=BFD_HOST_U_64_BIT
fi
AC_MSG_RESULT($bfd_file_ptr)
AC_SUBST(bfd_file_ptr)
AC_SUBST(bfd_ufile_ptr)


tdefaults=""
test -n "${defvec}" && tdefaults="${tdefaults} -DDEFAULT_VECTOR=${defvec}"
test -n "${selvecs}" && tdefaults="${tdefaults} -DSELECT_VECS='${selvecs}'"
test -n "${assocvecs}" && tdefaults="${tdefaults} -DASSOCIATED_VECS='${assocvecs}'"
test -n "${selarchs}" && tdefaults="${tdefaults} -DSELECT_ARCHITECTURES='${selarchs}'"
AC_SUBST(tdefaults)
AC_SUBST(havevecs)

dnl AC_CHECK_HEADERS(sys/mman.h)
AC_FUNC_MMAP
AC_CHECK_FUNCS(madvise mprotect)
case ${want_mmap}+${ac_cv_func_mmap_fixed_mapped} in
  true+yes )  AC_DEFINE(USE_MMAP, 1, [Use mmap if it's available?]) ;;
esac

rm -f doc/config.status
AC_CONFIG_FILES([Makefile doc/Makefile bfd-in3.h:bfd-in2.h po/Makefile.in:po/Make-in])

dnl We need this duplication, even though we use AM_PO_SUBDIRS, because of
dnl our two separate POTFILES.  Yuck.
AC_CONFIG_COMMANDS([default],
[[
case "$srcdir" in
  .) srcdirpre= ;;
  *) srcdirpre='$(srcdir)/' ;;
esac
POFILES=
GMOFILES=
for lang in dummy $OBSOLETE_ALL_LINGUAS; do
  if test $lang != dummy; then
    POFILES="$POFILES $srcdirpre$lang.po"
    GMOFILES="$GMOFILES $srcdirpre$lang.gmo"
  fi
done
sed -e '/SRC-POTFILES =/r po/SRC-POTFILES' \
    -e '/BLD-POTFILES =/r po/BLD-POTFILES' \
    -e "s,@POFILES@,$POFILES," \
    -e "s,@GMOFILES@,$GMOFILES," \
    po/Makefile.in > po/Makefile]],[[]])

dnl Required by html, pdf, install-pdf and install-html
AC_SUBST(datarootdir)
AC_SUBST(docdir)
AC_SUBST(htmldir)
AC_SUBST(pdfdir)

AC_OUTPUT

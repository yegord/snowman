# - Try to find libbfd
# Once done this will define
#
#  LIBBFD_FOUND - system has libbfd
#  LIBBFD_INCLUDE_DIRS - the libbfd include directory
#  LIBBFD_LIBRARIES - Link these to use libbfd
#  LIBBFD_DEFINITIONS - Compiler switches required for using libbfd
#
# Based on:
#
#  Copyright (c) 2008 Bernhard Walle <bernhard.walle@gmx.de>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (LIBBFD_LIBRARIES AND LIBBFD_INCLUDE_DIRS)
  set (LIBBFD_FIND_QUIETLY TRUE)
endif (LIBBFD_LIBRARIES AND LIBBFD_INCLUDE_DIRS)

find_path (LIBBFD_INCLUDE_DIRS
    NAMES
      bfd.h
      dis-asm.h
    PATHS
      /usr/include
      /usr/include/binutils
      /usr/local/include
      /usr/local/include/binutils
      /opt/local/include
      /opt/local/include/binutils
      /opt/include
      /opt/include/binutils
      C:/msys32/mingw32/include/binutils
      C:/msys32/mingw64/include/binutils
      C:/msys64/mingw32/include/binutils
      C:/msys64/mingw64/include/binutils
      ENV CPATH)

# Ugly, yes ugly...
find_library (LIBBFD_BFD_LIBRARY
    NAMES
      bfd
    PATHS
      /usr/lib
      /usr/lib64
      /usr/local/lib
      /usr/local/lib64
      /opt/local/lib
      /opt/usr/lib64
      C:/msys32/mingw32/lib/binutils
      C:/msys32/mingw64/lib/binutils
      C:/msys64/mingw32/lib/binutils
      C:/msys64/mingw64/lib/binutils
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)
LIST(APPEND LIBBFD_LIBRARIES ${LIBBFD_BFD_LIBRARY})

find_library (LIBBFD_IBERTY_LIBRARY
    NAMES
      iberty
    PATHS
      /usr/lib
      /usr/lib/x86_64
      /usr/lib64
      /usr/local/lib
      /usr/local/lib/x86_64
      /usr/local/lib64
      /opt/local/lib
      /opt/local/lib/x86_64
      /opt/usr/lib64
      C:/msys32/mingw32/lib/binutils
      C:/msys32/mingw64/lib/binutils
      C:/msys64/mingw32/lib/binutils
      C:/msys64/mingw64/lib/binutils
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)
LIST(APPEND LIBBFD_LIBRARIES ${LIBBFD_IBERTY_LIBRARY})

find_library (LIBBFD_OPCODES_LIBRARY
    NAMES
      opcodes
    PATHS
      /usr/lib
      /usr/lib64
      /usr/local/lib
      /usr/local/lib64
      /opt/local/lib
      /opt/usr/lib64
      C:/msys32/mingw32/lib/binutils
      C:/msys32/mingw64/lib/binutils
      C:/msys64/mingw32/lib/binutils
      C:/msys64/mingw64/lib/binutils
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)
LIST(APPEND LIBBFD_LIBRARIES ${LIBBFD_OPCODES_LIBRARY})

include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBBFD_FOUND to TRUE if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBBFD DEFAULT_MSG
    LIBBFD_BFD_LIBRARY
    LIBBFD_IBERTY_LIBRARY
    LIBBFD_OPCODES_LIBRARY
    LIBBFD_INCLUDE_DIRS)


mark_as_advanced(LIBBFD_INCLUDE_DIRS LIBBFD_LIBRARIES)

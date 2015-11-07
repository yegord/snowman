# Try to find the libintl library. Explicit searching is currently
# only required for Win32, though it might be useful for some UNIX
# variants, too. Therefore code for searching common UNIX include
# directories is included, too.
#
# Once done this will define
#
#  LIBINTL_FOUND - system has libintl
#  LIBINTL_LIBRARIES - libraries needed for linking

IF (LIBINTL_FOUND)
   SET(LIBINTL_FIND_QUIETLY TRUE)
ENDIF ()

# for Windows we rely on the environement variables
# %INCLUDE% and %LIB%; FIND_LIBRARY checks %LIB%
# automatically on Windows
IF(WIN32)
  FIND_LIBRARY(LIBINTL_LIBRARIES
    NAMES intl
  )
  IF(LIBINTL_LIBRARIES)
    SET(LIBINTL_FOUND TRUE)
  ELSE(LIBINTL_LIBRARIES)
    SET(LIBINTL_FOUND FALSE)
  ENDIF(LIBINTL_LIBRARIES)
ELSE()
  include(CheckFunctionExists)
  check_function_exists(dgettext LIBINTL_LIBC_HAS_DGETTEXT)
  if (LIBINTL_LIBC_HAS_DGETTEXT)
    find_library(LIBINTL_LIBRARIES NAMES c)
    set(LIBINTL_FOUND TRUE)
  else (LIBINTL_LIBC_HAS_DGETTEXT)
    find_library(LIBINTL_LIBRARIES 
      NAMES intl libintl 
      PATHS /usr/lib /usr/local/lib
    )
    IF(LIBINTL_LIBRARIES)
      SET(LIBINTL_FOUND TRUE)
    ELSE(LIBINTL_LIBRARIES)
      SET(LIBINTL_FOUND FALSE)
    ENDIF(LIBINTL_LIBRARIES)
  ENDIF (LIBINTL_LIBC_HAS_DGETTEXT)
ENDIF()

IF (LIBINTL_FOUND)
  IF (NOT LIBINTL_FIND_QUIETLY)
    MESSAGE(STATUS "Found libintl: ${LIBINTL_LIBRARIES}")
  ENDIF ()
ELSE ()
  IF (LIBINTL_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find libintl")
  ENDIF ()
ENDIF ()

MARK_AS_ADVANCED(LIBINTL_LIBRARIES LIBINTL_LIBC_HAS_DGETTEXT)

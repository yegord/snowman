# In this file we are doing all of our 'configure' checks. Things like checking
# for headers, functions, libraries, types and size of types.
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

# To check for an include file you do this:
CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)

# To check the size of a primitive type:
CHECK_TYPE_SIZE("char" SIZEOF_CHAR)
CHECK_TYPE_SIZE("int" SIZEOF_INT)
CHECK_TYPE_SIZE("long" SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("short" SIZEOF_SHORT)
CHECK_TYPE_SIZE("void *" SIZEOF_VOID_P)

# And you continue doing these types of checks for each item that you want to check. You can even do custom "try-compiles/Try-run" for more elaborate setups.
# Now that you have your variables define you add lines like this to your CMakeLists.txt file:

configure_file ( config.h.cmake config.h )
# Now make sure that you the the build directory on your "Include" path when compiling
include_directories(${PROJECT_BINARY_DIR})

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4267 /wd4244)
endif()

set(SOURCES
    config.h 
    bfd.c
    bfd.h
)

add_library(libbfd ${SOURCES})
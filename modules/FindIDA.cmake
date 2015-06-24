set(IDA_FOUND FALSE)
set(IDA_SDK_FOUND FALSE)

#
# Find IDA.
#

find_path(IDA_PATH
    NAME "idag.exe" "idaq.exe"
    HINTS $ENV{IDA_DIR} $ENV{IDADIR}
    PATHS "C:/Program Files/IDA" "C:/Program Files (x86)/IDA"
    DOC "IDA installation directory.")

if(IDA_PATH)
    set(IDA_FOUND TRUE)
    message(STATUS "Looking for IDA - found at ${IDA_PATH}")
else()
    message(STATUS "Looking for IDA - not found")
endif()

#
# Make up the name of the SDK library subdirectory.
#

# Detect the platform.
set(platform "unknown")
if(WIN32)
    set(platform "win")
endif()
if(UNIX)
    set(platform "linux")
endif()
if(APPLE)
    set(platform "mac")
endif()

# Detect the compiler.
set(compiler "unknown")
if(BORLAND)
    set(compiler "bcc")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(compiler "gcc")
endif()

if(MSVC)
    set(compiler "vc")
endif()

set(IDA_64_BIT_EA_T OFF CACHE BOOL "Use 64-bit ea_t. Set this to build 64-bit code capable IDA plugins.")

if(IDA_64_BIT_EA_T)
    set(suffix "64")
else()
    set(suffix "32")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT NC_M32)
    set(library_dir "lib/x64_${platform}_${compiler}_${suffix}")
else()
    set(library_dir "lib/x86_${platform}_${compiler}_${suffix}")
endif()

#
# Find IDA SDK.
#
find_path(IDA_SDK_PATH
    NAME ${library_dir}
    HINTS $ENV{IDA_SDK_DIR}
    PATHS "${IDA_PATH}/sdk"
    DOC "IDA SDK directory.")

if(IDA_SDK_PATH)
    set(IDA_SDK_FOUND TRUE)
    set(IDA_INCLUDE_DIR ${IDA_SDK_PATH}/include)
    set(IDA_LIBRARY_DIR ${IDA_SDK_PATH}/${library_dir})

    if(MSVC)
        file(GLOB IDA_LIBRARIES "${IDA_LIBRARY_DIR}/*.lib")
    else()
        file(GLOB IDA_LIBRARIES "${IDA_LIBRARY_DIR}/*.a")
    endif()

    set(IDA_DEFINITIONS -D__IDP__)
    if(WIN32)
        set(IDA_DEFINITIONS ${IDA_DEFINITIONS} -D__NT__)
    endif()
    if(UNIX)
        set(IDA_DEFINITIONS ${IDA_DEFINITIONS} -D__LINUX__)
    endif()
    if(APPLE)
        set(IDA_DEFINITIONS ${IDA_DEFINITIONS} -D__MAC__)
    endif()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT NC_M32)
        set(IDA_DEFINITIONS ${IDA_DEFINITIONS} -D__X64__)
    endif()
    if(IDA_64_BIT_EA_T)
        set(IDA_DEFINITIONS ${IDA_DEFINITIONS} -D__EA64__)
    endif()

    if(WIN32)
        if(IDA_64_BIT_EA_T)
            set(IDA_PLUGIN_EXT ".p64")
        else()
            set(IDA_PLUGIN_EXT ".plw")
        endif()
    elseif(APPLE)
        if(IDA_64_BIT_EA_T)
            set(IDA_PLUGIN_EXT ".pmc64")
            set(IDA_SHARED_LIB_NAME ida64)
        else()
            set(IDA_PLUGIN_EXT ".pmc")
            set(IDA_SHARED_LIB_NAME ida)
        endif()
        if (IDA_PATH)
            file(GLOB_RECURSE IDA_SHARED_LIBRARY  "${IDA_PATH}/*/lib${IDA_SHARED_LIB_NAME}.dylib")
        else()
            file(GLOB_RECURSE IDA_SHARED_LIBRARY  "/Applications/IDA*/lib${IDA_SHARED_LIB_NAME}.dylib")
        endif()
        set(IDA_LIBRARIES ${IDA_LIBRARIES} ${IDA_SHARED_LIBRARY})
    else()
        if(IDA_64_BIT_EA_T)
            set(IDA_PLUGIN_EXT ".plx64")
        else()
            set(IDA_PLUGIN_EXT ".plx")
        endif()
    endif()

    message(STATUS "Looking for IDA SDK - found at ${IDA_SDK_PATH}")
else()
    message(STATUS "Looking for IDA SDK - not found")
endif()

unset(platform)
unset(compiler)
unset(suffix)
unset(library_dir)

# vim:set et sts=4 sw=4 nospell:

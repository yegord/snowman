cmake_minimum_required(VERSION 2.8)
project(Decompiler C CXX)

enable_testing()

# Show what's going on when running make.
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Generate compilation database for YouCompleteMe.
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

# Link against qtmain.lib automatically.
cmake_policy(SET CMP0020 NEW)

#Don't use the RPATH on OS X.
set(CMAKE_MACOSX_RPATH 0)

#
# Include additional scripts.
#
set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}  ${CMAKE_SOURCE_DIR}/../modules)

#
# Specify namespace with which Qt has been configured.
#
set(QT_NAMESPACE "" CACHE STRING "Qt Namespace")

set(NC_M32 OFF CACHE STRING "Make a 32-bit build")

#
# Profiling.
#
set(ENABLE_PROFILING FALSE CACHE BOOL "Enable profiling")

#
# Optimize and generate debug info by default.
#
if(NOT CMAKE_BUILD_TYPE)
    get_property(helpstring CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "${helpstring}" FORCE)
    unset(helpstring)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")

    #
    # Options for Clang.
    #

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    add_definitions(-Wall -Wextra)

    if(${NC_M32})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    endif()

elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

    #
    # Options for GCC.
    #

    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

    add_definitions(-Wall -Wextra)
    add_definitions(-fshow-column)

    # Boost often violates this :-(
    add_definitions(-fno-strict-aliasing)

    # GCC warns about optimizing expressions like x <= x + 1 also when these expressions
    # arise only after inlining, so there is sometimes no decent way to kill the warning.
    add_definitions(-Wno-strict-overflow)

    # GCC produces false positives with boost::optional
    add_definitions(-Wno-maybe-uninitialized)

    if(${ENABLE_PROFILING})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    endif()

    if(${NC_M32})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    endif()

elseif(${MSVC})

    #
    # Options for MSVC.
    #

    # Target Windows XP.
    add_definitions(-DWINVER=0x0501)
endif()

# Add source dir to includes.
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/3rd-party)

# Find Boost.
find_package(Boost 1.46.0 REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Choose which Qt version to look for.
if(NOT DEFINED NC_QT5)
    find_package(Qt5Core)

    set(helpstring "Build using Qt5 rather than Qt4.")
    if(Qt5Core_DIR)
        set(NC_QT5 TRUE CACHE BOOL ${helpstring})
    else()
        set(NC_QT5 FALSE CACHE BOOL ${helpstring})
    endif()
    unset(helpstring)
endif()

if(${NC_QT5})
    # Find Qt5.
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Widgets REQUIRED)

    set(CMAKE_AUTOMOC ON)

    macro(qt4_wrap_cpp)
    endmacro()

    set(QT_LIBRARIES Qt5::Core Qt5::Widgets)
else()
    # Find Qt4.
    find_package(Qt4 REQUIRED QtCore QtGui)
    include(${QT_USE_FILE})

    # This is used to produce standalone bundles on Windows.
    include(DeployQt4)
endif()

if(QT_NAMESPACE)
    add_definitions(-DQT_NAMESPACE=${QT_NAMESPACE})
endif()

#Find libbfd
find_package(Bfd)

if(LIBBFD_FOUND)
  add_definitions(-DHAVE_LIBBFD=1)
endif()

# Find IDA SDK.
find_package(IDA)

# Option for building IDA plug-in.
set(IDA_PLUGIN_ENABLED ${IDA_SDK_FOUND} CACHE BOOL "Build IDA plug-in.")

# Position-independent code is necessary for the IDA plug-in.
if(${IDA_PLUGIN_ENABLED})
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${CMAKE_C_COMPILE_OPTIONS_PIC}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_COMPILE_OPTIONS_PIC}")
endif()

# CMake rocks!
if(${IDA_PLUGIN_ENABLED})
    set(IDA_PLUGIN_DISABLED NO)
else()
    set(IDA_PLUGIN_DISABLED YES)
endif()

# Option for making multithreaded builds.
set(NC_USE_THREADS ${IDA_PLUGIN_DISABLED} CACHE BOOL "Enable threads.")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/nc/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/nc/config.h")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(nc)
add_subdirectory(nocode)
add_subdirectory(snowman)
if(${IDA_PLUGIN_ENABLED})
    add_subdirectory(ida-plugin)
endif()

# Prevent 3rd-party modules from installing anything.
macro(install)
endmacro(install)

add_subdirectory(3rd-party)

# vim:set et sts=4 sw=4 nospell:

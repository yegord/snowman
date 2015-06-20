set(SOURCES
    decode.c
    decode.h
    extern.h
    input.c
    input.h
    itab.c
    itab.h
    syn.c
    syn.h
    syn-att.c
    syn-intel.c
    types.h
    udis86.c
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_definitions(-Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_definitions(-Wno-unused-function -Wno-unused-variable)
endif()

add_library(udis86 ${SOURCES}) 

# vim:set et sts=4 sw=4 nospell:

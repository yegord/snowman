set(X64DBG_SDK_FOUND FALSE)

# 
# Find x64dbg
# 

find_path(X64DBG_SDK_PATH
    NAME "bridgemain.h"
    HINTS $ENV{X64DBG_SDK_DIR}
    PATHS "C:/x64dbg/pluginsdk"
    DOC "Extracted x64dbg snapshot.")

if(X64DBG_SDK_PATH)
    set(X64DBG_SDK_FOUND TRUE)
    message(STATUS "Looking for x64dbg SDK - found")
else()
    message(STATUS "Looking for x64dbg SDK - not found")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    file(GLOB X64DBG_LIBRARIES "${X64DBG_SDK_PATH}/x64bridge.lib")
else()
    file(GLOB X64DBG_LIBRARIES "${X64DBG_SDK_PATH}/x32bridge.lib")
endif()

# vim:set et sts=4 sw=4 nospell:

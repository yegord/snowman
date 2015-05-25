set(MOC_HEADERS
    SnowmanView.h
)

set(SOURCES
    SnowmanView.cpp
    ${MOC_HEADERS}
)

include_directories(${X64DBG_SDK_PATH})

qt4_wrap_cpp(SOURCES ${MOC_HEADERS})

add_library(x64dbg-module SHARED ${SOURCES})
target_link_libraries(x64dbg-module nc-gui nc-core ${Boost_LIBRARIES} ${QT_LIBRARIES} ${X64DBG_LIBRARIES})
set_target_properties(x64dbg-module PROPERTIES OUTPUT_NAME "snowman" PREFIX "" SUFFIX ".dll")
install(TARGETS x64dbg-module RUNTIME DESTINATION bin LIBRARY DESTINATION lib)

# vim:set et sts=4 sw=4 nospell:

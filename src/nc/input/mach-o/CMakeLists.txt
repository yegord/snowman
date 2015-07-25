set(SOURCES
    MachOParser.cpp
    MachOParser.h
    mach-o.h
)

add_library(nc-input-mach-o ${SOURCES})
target_link_libraries(nc-input-mach-o nc-core nc-common ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

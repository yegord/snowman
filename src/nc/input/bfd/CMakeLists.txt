set(SOURCES
    BfdParser.cpp
    BfdParser.h
)

add_library(nc-input-bfd ${SOURCES})
target_link_libraries(nc-input-bfd nc-core nc-common ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

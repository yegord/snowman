include_directories(${LIBBFD_INCLUDE_DIRS})

set(SOURCES
    BfdParser.cpp
    BfdParser.h
    ${LIBBFD_INCLUDE_DIRS}/bfd.h
)

add_library(nc-input-bfd ${SOURCES})
target_link_libraries(nc-input-bfd nc-core nc-common ${Boost_LIBRARIES} ${QT_LIBRARIES} ${LIBBFD_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

set(SOURCES
    main.cpp
)

add_executable(nocode ${SOURCES})
target_link_libraries(nocode nc-core ${Boost_LIBRARIES} ${QT_LIBRARIES})

if (NOT ${IDA_PLUGIN_ENABLED})
    install(TARGETS nocode RUNTIME DESTINATION bin)
    if(WIN32 AND NOT ${NC_QT5})
        install_qt4_executable("bin/nocode.exe")
    endif()
endif()

# vim:set et sts=4 sw=4 nospell:

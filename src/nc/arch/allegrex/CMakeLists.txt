set(SOURCES
    AllegrexArchitecture.cpp
    AllegrexArchitecture.h
    AllegrexDisassembler.cpp
    AllegrexDisassembler.h
    AllegrexInstruction.h
    AllegrexInstructionAnalyzer.cpp
    AllegrexInstructionAnalyzer.h
    AllegrexRegisters.cpp
    AllegrexRegisters.h
    AllegrexCallingConventions.cpp
    AllegrexCallingConventions.h
    Allegrex.cpp
    Allegrex.h
)

add_library(nc-arch-allegrex ${SOURCES})
target_link_libraries(nc-arch-allegrex nc-core nc-common ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

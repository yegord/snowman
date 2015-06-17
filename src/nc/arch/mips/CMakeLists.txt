set(SOURCES
    MipsArchitecture.cpp
    MipsArchitecture.h
    MipsDisassembler.cpp
    MipsDisassembler.h
    MipsInstruction.h
    MipsInstructionAnalyzer.cpp
    MipsInstructionAnalyzer.h
    MipsRegisters.cpp
    MipsRegisters.h
    CallingConventions.cpp
    CallingConventions.h
)

add_library(nc-arch-mips ${SOURCES})
target_link_libraries(nc-arch-mips nc-core nc-common capstone-static ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

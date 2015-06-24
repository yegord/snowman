set(SOURCES
    ArmArchitecture.cpp
    ArmArchitecture.h
    ArmDisassembler.cpp
    ArmDisassembler.h
    ArmInstruction.h
    ArmInstructionAnalyzer.cpp
    ArmInstructionAnalyzer.h
    ArmRegisters.cpp
    ArmRegisters.h
    CallingConventions.cpp
    CallingConventions.h
)

add_library(nc-arch-arm ${SOURCES})
target_link_libraries(nc-arch-arm nc-core nc-common capstone-static ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

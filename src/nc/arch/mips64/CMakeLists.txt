set(SOURCES
    Mips64Architecture.cpp
    Mips64Architecture.h
    Mips64Disassembler.cpp
    Mips64Disassembler.h
    Mips64Instruction.h
    Mips64InstructionAnalyzer.cpp
    Mips64InstructionAnalyzer.h
    Mips64Registers.cpp
    Mips64Registers.h
    CallingConventions.cpp
    CallingConventions.h
)

add_library(nc-arch-mips64 ${SOURCES})
target_link_libraries(nc-arch-mips64 nc-core nc-common capstone-static ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

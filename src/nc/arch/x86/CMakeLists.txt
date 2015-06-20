set(SOURCES
    CallingConventions.cpp
    CallingConventions.h
    X86Architecture.cpp
    X86Architecture.h
    X86Disassembler.cpp
    X86Disassembler.h
    X86Instruction.cpp
    X86Instruction.h
    X86InstructionAnalyzer.cpp
    X86InstructionAnalyzer.h
    X86MasterAnalyzer.cpp
    X86MasterAnalyzer.h
    X86Registers.cpp
    X86Registers.h
    udis86.h
)

add_library(nc-arch-x86 ${SOURCES})
target_link_libraries(nc-arch-x86 nc-core nc-common udis86 ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:

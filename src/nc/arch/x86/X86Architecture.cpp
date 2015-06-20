/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "X86Architecture.h"

#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include "CallingConventions.h"
#include "X86Disassembler.h"
#include "X86Instruction.h"
#include "X86InstructionAnalyzer.h"
#include "X86MasterAnalyzer.h"
#include "X86Registers.h"

namespace nc {
namespace arch {
namespace x86 {

X86Architecture::X86Architecture(Mode mode) {
    static X86MasterAnalyzer masterAnalyzer;
    setMasterAnalyzer(&masterAnalyzer);

    setRegisters(X86Registers::instance());

    switch(mode) {
    case REAL_MODE:
        setName("8086");
        setBitness(16);
        mInstructionPointer = X86Registers::ip();
        mStackPointer = X86Registers::sp();
        mBasePointer  = X86Registers::bp();
        addCallingConvention(std::make_unique<Cdecl16CallingConvention>(this));
        break;
    case PROTECTED_MODE:
        setName("i386");
        setBitness(32);
        mInstructionPointer = X86Registers::eip();
        mStackPointer = X86Registers::esp();
        mBasePointer  = X86Registers::ebp();
        addCallingConvention(std::make_unique<Cdecl32CallingConvention>(this));
        addCallingConvention(std::make_unique<Stdcall32CallingConvention>(this));
        break;
    case LONG_MODE:
        setName("x86-64");
        setBitness(64);
        mInstructionPointer = X86Registers::rip();
        mStackPointer = X86Registers::rsp();
        mBasePointer  = X86Registers::rbp();
        addCallingConvention(std::make_unique<AMD64CallingConvention>(this));
        addCallingConvention(std::make_unique<Microsoft64CallingConvention>(this));
        break;
    default:
        unreachable();
    }

    setMaxInstructionSize(X86Instruction::MAX_SIZE);
}

X86Architecture::~X86Architecture() {}

ByteOrder X86Architecture::getByteOrder(core::ir::Domain) const {
    return ByteOrder::LittleEndian;
}

std::unique_ptr<core::arch::Disassembler> X86Architecture::createDisassembler() const {
    return std::make_unique<X86Disassembler>(this);
}

std::unique_ptr<core::irgen::InstructionAnalyzer> X86Architecture::createInstructionAnalyzer() const {
    return std::make_unique<X86InstructionAnalyzer>(this);
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */

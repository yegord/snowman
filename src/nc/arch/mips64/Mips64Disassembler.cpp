/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Mips64Disassembler.h"

#include <nc/common/make_unique.h>

#include "Mips64Architecture.h"
#include "Mips64Instruction.h"

namespace nc {
namespace arch {
namespace mips64 {

Mips64Disassembler::Mips64Disassembler(const Mips64Architecture *architecture):
    core::arch::Disassembler(architecture)
{
    mode_ = CS_MODE_MIPS64;
    if (architecture->byteOrder() == ByteOrder::LittleEndian) {
        mode_ |= CS_MODE_LITTLE_ENDIAN;
    } else if (architecture->byteOrder() == ByteOrder::BigEndian) {
        mode_ |= CS_MODE_BIG_ENDIAN;
    }
    capstone_ = std::make_unique<core::arch::Capstone>(CS_ARCH_MIPS, mode_);
}

Mips64Disassembler::~Mips64Disassembler() {}

std::shared_ptr<core::arch::Instruction> Mips64Disassembler::disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) {
    if (auto instr = capstone_->disassemble(pc, buffer, size, 1)) {
        /* Instructions must be aligned to their size. */
        if ((instr->address & (instr->size - 1)) == 0) {
            return std::make_shared<Mips64Instruction>(mode_, instr->address, instr->size, buffer);
        }
    }
    return nullptr;
}

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */

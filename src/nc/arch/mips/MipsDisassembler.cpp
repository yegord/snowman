/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "MipsDisassembler.h"

#include <nc/common/make_unique.h>

#include "MipsArchitecture.h"
#include "MipsInstruction.h"

namespace nc {
namespace arch {
namespace mips {

MipsDisassembler::MipsDisassembler(const MipsArchitecture *architecture):
    core::arch::Disassembler(architecture)
{
    mode_ = CS_MODE_MIPS32;
    if (architecture->byteOrder() == ByteOrder::LittleEndian) {
        mode_ |= CS_MODE_LITTLE_ENDIAN;
    } else if (architecture->byteOrder() == ByteOrder::BigEndian) {
        mode_ |= CS_MODE_BIG_ENDIAN;
    }
    capstone_ = std::make_unique<core::arch::Capstone>(CS_ARCH_MIPS, mode_);
}

MipsDisassembler::~MipsDisassembler() {}

std::shared_ptr<core::arch::Instruction> MipsDisassembler::disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) {
    if (auto instr = capstone_->disassemble(pc, buffer, size, 1)) {
        /* Instructions must be aligned to their size. */
        if ((instr->address & (instr->size - 1)) == 0) {
            return std::make_shared<MipsInstruction>(mode_, instr->address, instr->size, buffer);
        }
    }
    return nullptr;
}

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */

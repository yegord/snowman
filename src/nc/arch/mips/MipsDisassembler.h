/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include <nc/core/arch/Capstone.h>
#include <nc/core/arch/Disassembler.h>

namespace nc {
namespace arch {
namespace mips {

class MipsArchitecture;

/**
 * Disassembler for ARM architecture.
 *
 * TODO: Support for THUMB instructions.
 */
class MipsDisassembler: public core::arch::Disassembler {
    std::unique_ptr<core::arch::Capstone> capstone_;
    int mode_;

public:
    /**
     * \param architecture Valid pointer to the ARM architecture.
     */
    MipsDisassembler(const MipsArchitecture *architecture);

    std::shared_ptr<core::arch::Instruction> disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) override;
};

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */

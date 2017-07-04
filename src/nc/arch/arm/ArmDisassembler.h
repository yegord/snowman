/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include <nc/core/arch/Capstone.h>
#include <nc/core/arch/Disassembler.h>

namespace nc {
namespace arch {
namespace arm {

class ArmArchitecture;

/**
 * Disassembler for ARM architecture.
 *
 * TODO: Support for THUMB instructions.
 */
class ArmDisassembler: public core::arch::Disassembler {
    std::unique_ptr<core::arch::Capstone> capstone_;
    int mode_;

public:
    ArmDisassembler(const ArmArchitecture *architecture);

    virtual ~ArmDisassembler();

    int mode() {
        return mode_;
    }

    void setMode(int mode) {
        if (mode_ != mode) {
            mode_ = mode;
            capstone_->setMode(mode);
        }
    }

    std::shared_ptr<core::arch::Instruction> disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */

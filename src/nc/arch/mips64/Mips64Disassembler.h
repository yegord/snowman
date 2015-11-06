/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include <nc/core/arch/Capstone.h>
#include <nc/core/arch/Disassembler.h>

namespace nc {
namespace arch {
namespace mips64 {

class Mips64Architecture;

/**
 * Disassembler for MIPS architecture.
 *
 * TODO: Support for MIPS16 instructions.
 */
class Mips64Disassembler: public core::arch::Disassembler {
    std::unique_ptr<core::arch::Capstone> capstone_;
    int mode_;

public:
    /**
     * \param architecture Valid pointer to the MIPS architecture.
     */
    Mips64Disassembler(const Mips64Architecture *architecture);

    virtual ~Mips64Disassembler();

    std::shared_ptr<core::arch::Instruction> disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) override;
};

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */

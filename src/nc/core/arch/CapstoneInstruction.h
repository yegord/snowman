/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include "Instruction.h"

#include <array>
#include <cassert>

#include <capstone/capstone.h>

namespace nc {
namespace core {
namespace arch {

template<cs_arch csArchitecture_, SmallByteSize maxSize_>
class CapstoneInstruction: public Instruction {
    /** Encoding mode of this instruction, as denoted in Capstone. */
    int csMode_;

    /** Binary representation of the instruction. */
    std::array<uint8_t, maxSize_> bytes_;

public:
    /**
     * Constructor.
     *
     * \param[in] csMode Encoding mode of this instruction, as denoted in Capstone.
     * \param[in] addr Instruction address in bytes.
     * \param[in] size Instruction size in bytes.
     * \param[in] bytes Valid pointer to the bytes of the instruction.
     */
    CapstoneInstruction(int csMode, ByteAddr addr, SmallByteSize size, const void *bytes):
        Instruction(addr, size), csMode_(csMode)
    {
        assert(size > 0);
        assert(size <= maxSize_);
        memcpy(bytes_.data(), bytes, size);
    }

    /**
     * Architecture of this instruction, as denoted in Capstone.
     */
    static cs_arch csArchitecture() { return csArchitecture_; }

    /**
     * Encoding mode of this instruction, as denoted in Capstone.
     */
    int csMode() const { return csMode_; }

    /**
     * \return Max size of the instruction.
     */
    static SmallByteSize maxSize() { return maxSize_; }

    /**
     * \return Valid pointer to the buffer containing the binary
     *         representation of the instruction.
     */
    const uint8_t *bytes() const { return &bytes_[0]; }

    void print(QTextStream &out) const override {
        auto instr = Capstone(csArchitecture_, csMode_).disassemble(addr(), &bytes_[0], size(), 1);
        assert(instr != nullptr);

        out << instr->mnemonic << " " << instr->op_str;
    }
};


} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include "nc/core/arch/Instruction.h"

#include <array>
#include <cassert>

#include "Allegrex.h"

namespace nc {
namespace arch {
namespace allegrex {

    class AllegrexInstruction : public core::arch::Instruction {

        /** Binary representation of the instruction. */
        int32_t bytes_;

    public:
        /**
        * Constructor.
        *
        * \param[in] addr Instruction address in bytes.
        * \param[in] size Instruction size in bytes.
        * \param[in] bytes Valid pointer to the bytes of the instruction.
        */
        AllegrexInstruction(ByteAddr addr, SmallByteSize size, const void *bytes) :
            Instruction(addr, size)
        {
            assert(size == 4);
            memcpy(&bytes_, bytes, size);
        }

        /**
        * \return Max size of the instruction.
        */
        static SmallByteSize maxSize() { return 4; }

        /**
        * \return Valid pointer to the buffer containing the binary
        *         representation of the instruction.
        */
        const uint8_t *bytes() const { return (const uint8_t *)&bytes_; }

        void print(QTextStream &out) const override {
            auto insn = allegrex_disassemble_instruction(bytes_, addr(), false);
            out << insn;
        }
    };

}}} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */

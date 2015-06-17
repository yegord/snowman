/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>
#include <functional>
#include <memory>

#include <nc/common/Types.h>

namespace nc {

class CancellationToken;

namespace core {

namespace image {
    class ByteSource;
}

namespace arch {

class Architecture;
class Instruction;

/**
 * Disassembler for a sequence of instructions.
 */
class Disassembler {
    const Architecture *architecture_; ///< Architecture.

public:
    /**
     * Constructor.
     *
     * \param architecture Valid pointer to the architecture.
     */
    Disassembler(const Architecture *architecture):
        architecture_(architecture)
    {
        assert(architecture);
    }

    /**
     * Virtual destructor.
     */
    virtual ~Disassembler() {}

    typedef std::function<void(std::shared_ptr<Instruction>)> InstructionCallback;

    /**
     * Disassembles all instructions in the given range of addresses.
     *
     * \param source Valid pointer to a byte source.
     * \param begin First address in the range.
     * \param end First address past the range.
     * \param callback Function being called for each disassembled instruction.
     * \param canceled Cancellation token.
     */
    virtual void disassemble(const image::ByteSource *source, ByteAddr begin, ByteAddr end, InstructionCallback callback, const CancellationToken &canceled);

    /**
     * Disassembles a single instruction.
     *
     * \param[in] pc Virtual address of the instruction.
     * \param[in] buffer Valid pointer to the buffer containing the instruction.
     * \param[in] size Buffer size.
     *
     * \return Pointer to the instruction disassembled from the buffer if disassembling succeeded, nullptr otherwise.
     */
    virtual std::shared_ptr<Instruction> disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) = 0;

    /**
     * Disassembles a single instruction.
     *
     * \param[in] pc Virtual address of the instruction.
     * \param[in] source Valid pointer to the byte source containing the instruction.
     *
     * \return Pointer to the instruction disassembled from the buffer if disassembling succeeded, nullptr otherwise.
     */
    virtual std::shared_ptr<Instruction> disassembleSingleInstruction(ByteAddr pc, const image::ByteSource *source);
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Disassembler.h"

#include <algorithm> /* std::max() */

#include <nc/core/image/ByteSource.h>

#include <nc/common/CancellationToken.h>

#include "Architecture.h"
#include "Instruction.h"

namespace nc {
namespace core {
namespace arch {

void Disassembler::disassemble(const image::ByteSource *source, ByteAddr begin, ByteAddr end, InstructionCallback callback, const CancellationToken &canceled) {
    assert(source != nullptr);
    assert(begin <= end);

    const SmallByteSize maxInstructionSize = architecture_->maxInstructionSize();
    const ByteSize bufferSize = std::min(ByteSize(std::max(65536, maxInstructionSize)), end - begin);
    const std::unique_ptr<char[]> buffer(new char[bufferSize]);

    auto bufferBegin = begin;
    auto bufferEnd = begin;

    for (ByteAddr pc = begin; pc < end; canceled.poll()) {
        if (pc + maxInstructionSize > bufferEnd && bufferEnd < end) {
            bufferBegin = pc;
            bufferEnd = bufferBegin + source->readBytes(pc, buffer.get(), std::min(bufferSize, end - pc));
        }

        auto instruction = disassembleSingleInstruction(pc, buffer.get() + (pc - bufferBegin), bufferEnd - pc);

        if (instruction) {
            assert(instruction->size() > 0);
            pc = instruction->endAddr();
            callback(std::move(instruction));
        } else {
            ++pc;
        }
    }
}

std::shared_ptr<Instruction> Disassembler::disassembleSingleInstruction(ByteAddr pc, const image::ByteSource *source) {
    const SmallByteSize maxInstructionSize = architecture_->maxInstructionSize();
    const std::unique_ptr<char[]> buffer(new char[maxInstructionSize]);

    return disassembleSingleInstruction(
        pc,
        buffer.get(),
        source->readBytes(pc, buffer.get(), maxInstructionSize));
}

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

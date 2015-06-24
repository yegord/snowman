/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "X86Disassembler.h"

#include <nc/common/CheckedCast.h>

#include "X86Architecture.h"
#include "X86Instruction.h"

namespace nc {
namespace arch {
namespace x86 {

X86Disassembler::X86Disassembler(const X86Architecture *architecture): core::arch::Disassembler(architecture) {
    ud_init(&ud_obj_);
    ud_set_mode(&ud_obj_, architecture->bitness());
}

std::shared_ptr<core::arch::Instruction> X86Disassembler::disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) {
    ud_set_pc(&ud_obj_, pc);
    ud_set_input_buffer(&ud_obj_, const_cast<uint8_t *>(static_cast<const uint8_t *>(buffer)), checked_cast<std::size_t>(size));

    SmallByteSize instructionSize = ud_disassemble(&ud_obj_);
    if (!instructionSize || ud_obj_.mnemonic == UD_Iinvalid) {
        return nullptr;
    }

    if (instructionSize > X86Instruction::MAX_SIZE) {
        /* Too many prefixes. Skip them. */
        return nullptr;
    }

    return std::make_shared<X86Instruction>(ud_obj_.dis_mode, pc, instructionSize, buffer);
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */

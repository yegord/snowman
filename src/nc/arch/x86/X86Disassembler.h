/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Disassembler.h>

#include "udis86.h"

namespace nc {
namespace arch {
namespace x86 {

class X86Architecture;

/**
 * Disassembler for x86 instructions.
 */
class X86Disassembler: public core::arch::Disassembler {
    ud_t ud_obj_;

public:
    /**
     * Constructor.
     *
     * \param architecture Valid pointer to the architecture.
     */
    explicit
    X86Disassembler(const X86Architecture *architecture);

    std::shared_ptr<core::arch::Instruction> disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize size) override;
};

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */

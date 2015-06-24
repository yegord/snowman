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

#include <nc/core/arch/Architecture.h>

namespace nc {
namespace arch {
namespace x86 {

class X86InstructionAnalyzer;
class X86MasterAnalyzer;
class X86Registers;

/**
 * Intel x86 architecture.
 */
class X86Architecture: public core::arch::Architecture {
public:
    /**
     * Processor mode.
     */
    enum Mode {
        REAL_MODE,      ///< 16-bit real mode.
        PROTECTED_MODE, ///< 32-bit protected mode.
        LONG_MODE,      ///< 64-bit long mode.
    };

    /**
     * Constructor.
     *
     * \param mode Processor mode.
     */
    explicit
    X86Architecture(Mode mode);

    /**
     * Destructor.
     */
    virtual ~X86Architecture();

    /**
     * \return Valid pointer to the stack pointer register.
     */
    const core::arch::Register *stackPointer() const { return mStackPointer; }

    /**
     * \return Valid pointer to the stack frame base pointer register.
     */
    const core::arch::Register *basePointer() const { return mBasePointer; }

    /**
     * \return Valid pointer to instruction pointer register.
     */
    const core::arch::Register *instructionPointer() const { return mInstructionPointer; }

    ByteOrder getByteOrder(core::ir::Domain domain) const override;
    std::unique_ptr<core::arch::Disassembler> createDisassembler() const override;
    std::unique_ptr<core::irgen::InstructionAnalyzer> createInstructionAnalyzer() const override;

protected:
    friend class X86Registers;

private:
    std::unique_ptr<X86MasterAnalyzer> mMasterAnalyzer;

    /** Stack pointer register. */
    const core::arch::Register *mStackPointer;

    /** Stack frame base pointer register. */
    const core::arch::Register *mBasePointer;

    /** Instruction pointer register. */
    const core::arch::Register *mInstructionPointer;
};

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */

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

#include <memory>
#include <vector>

#include <QString>

#include <nc/common/ByteOrder.h>
#include <nc/common/Types.h>

#include <nc/core/ir/MemoryDomain.h>

namespace nc {
namespace core {

class MasterAnalyzer;

namespace ir {
    class MemoryLocation;

    namespace calling {
        class Convention;
    }
}

namespace irgen {
    class InstructionAnalyzer;
}

namespace arch {

class Disassembler;
class Register;
class Registers;

/**
 * Immutable class describing an architecture.
 */
class Architecture {
public:
    /**
     * Constructor.
     */
    Architecture();

    /**
     * Virtual destructor.
     */
    virtual ~Architecture();

    /**
     * \return Name of the architecture.
     */
    const QString &name() const { return mName; }

    /**
     * \returns Architecture's bitness (data pointer size).
     */
    SmallBitSize bitness() const { assert(mBitness); return mBitness; }

    /**
     * \return Maximal length of an instruction.
     */
    SmallByteSize maxInstructionSize() const { assert(mMaxInstructionSize); return mMaxInstructionSize; }

    /**
     * \returns Valid pointer to the disassembler for a single instruction.
     */
    virtual std::unique_ptr<Disassembler> createDisassembler() const = 0;

    /**
     * \returns Valid pointer to the instruction analyzer for this architecture.
     */
    virtual std::unique_ptr<irgen::InstructionAnalyzer> createInstructionAnalyzer() const = 0;

    /**
     * \returns Valid pointer to the universal analyzer for this architecture.
     */
    const MasterAnalyzer *masterAnalyzer() const { return mMasterAnalyzer; }

    /**
     * \returns Valid pointer to the register container for this architecture.
     */
    const Registers *registers() const { return mRegisters; }

    /**
     * \param domain Memory domain.
     *
     * \return Byte order for this domain.
     */
    virtual ByteOrder getByteOrder(ir::Domain domain) const = 0;

    /**
     * \param memoryLocation Memory location.
     *
     * \return True, if the memory location belongs to global memory accessible by all functions.
     */
    virtual bool isGlobalMemory(const ir::MemoryLocation &memoryLocation) const;

    /**
     * \return List of available calling conventions.
     */
    const std::vector<const ir::calling::Convention *> &conventions() const {
        return reinterpret_cast<const std::vector<const ir::calling::Convention *> &>(conventions_);
    }

    /**
     * \param name Name of a calling convention.
     *
     * \return Pointer to the calling convention with the given name. Can be nullptr.
     */
    const ir::calling::Convention *getCallingConvention(const QString &name) const;

protected:
    /**
     * Sets the name of the architecture.
     * The name must be sent only once.
     *
     * \param name Non-empty new name of the architecture.
     */
    void setName(QString name);

    /**
     * Sets the architecture's bitness.
     *
     * \param bitness Architecture's bitness.
     */
    void setBitness(SmallBitSize bitness);

    /**
     * \param size Architecture's maximum instruction size.
     */
    void setMaxInstructionSize(SmallBitSize size);

    /**
     * \param masterAnalyzer Valid pointer to the master analyzer for this architecture.
     */
    void setMasterAnalyzer(const MasterAnalyzer *masterAnalyzer);

    /**
     * \param registers Valid pointer to the registers container for this architecture.
     */
    void setRegisters(Registers *registers);

    /**
     * Adds a calling convention.
     * There must be no convention with the same name already added.
     *
     * \param convention Valid pointer to the calling convention.
     */
    void addCallingConvention(std::unique_ptr<ir::calling::Convention> convention);

private:
    /** Name of the architecture. */
    QString mName;

    /** Architecture's bitness. */
    SmallBitSize mBitness;

    /** Maximum length of an instruction on this architecture. */
    SmallBitSize mMaxInstructionSize;

    /** Master analyzer for this architecture. */
    const MasterAnalyzer *mMasterAnalyzer;

    /** Register container for this architecture. */
    Registers *mRegisters;

    /** Calling conventions. */
    std::vector<std::unique_ptr<ir::calling::Convention>> conventions_;
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

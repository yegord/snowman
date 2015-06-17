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

#include "Architecture.h"

#include <nc/common/Foreach.h>

#include <nc/core/ir/MemoryLocation.h>
#include <nc/core/ir/calling/Convention.h>

namespace nc {
namespace core {
namespace arch {

Architecture::Architecture():
    mBitness(0),
    mMaxInstructionSize(0),
    mMasterAnalyzer(nullptr),
    mRegisters(nullptr)
{}

Architecture::~Architecture() {}

void Architecture::setName(QString name) {
    assert(mName.isEmpty() && "Name must be non-empty.");
    assert(!name.isEmpty() && "Name cannot be reset.");

    mName = std::move(name);
}

void Architecture::setBitness(SmallBitSize bitness) {
    assert(bitness > 0 && "Bitness must be a positive integer.");
    assert(mBitness == 0 && "Bitness cannot be reset.");

    mBitness = bitness;
}

void Architecture::setMaxInstructionSize(SmallBitSize size) {
    assert(size > 0 && "Maximal instruction size must be a positive integer.");
    assert(mMaxInstructionSize == 0 && "Maximal instruction size cannot be reset.");

    mMaxInstructionSize = size;
}

void Architecture::setMasterAnalyzer(const MasterAnalyzer *masterAnalyzer) {
    assert(masterAnalyzer != nullptr);
    assert(mMasterAnalyzer == nullptr && "Master analyzer is already set.");

    mMasterAnalyzer = masterAnalyzer;
}

void Architecture::setRegisters(Registers *registers) {
    assert(registers != nullptr);
    assert(mRegisters == nullptr && "Register container is already set.");

    mRegisters = registers;
}

bool Architecture::isGlobalMemory(const ir::MemoryLocation &memoryLocation) const {
    return memoryLocation.domain() == ir::MemoryDomain::MEMORY;
}

void Architecture::addCallingConvention(std::unique_ptr<ir::calling::Convention> convention) {
    assert(convention != nullptr);
    assert(getCallingConvention(convention->name()) == nullptr &&
           "No two calling conventions with the same name allowed.");

    conventions_.push_back(std::move(convention));
}

const ir::calling::Convention *Architecture::getCallingConvention(const QString &name) const {
    foreach (auto convention, conventions()) {
        if (convention->name() == name) {
            return convention;
        }
    }
    return nullptr;
}

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

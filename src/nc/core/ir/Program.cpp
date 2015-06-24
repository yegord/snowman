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

#include "Program.h"

#include <algorithm> /* std::find_if */
#include <cassert>

#include <QTextStream>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h> /* For nc::find. */
#include <nc/common/make_unique.h>

#include <nc/core/arch/Instruction.h>

#include "CFG.h"
#include "Statement.h"

namespace nc {
namespace core {
namespace ir {

Program::Program() {}

Program::~Program() {}

void Program::addRange(BasicBlock *basicBlock) {
    assert(basicBlock != nullptr);
    assert(basicBlock->address() && basicBlock->successorAddress() && "Basic block must be memory-bound.");
    range2basicBlock_[AddrRange(*basicBlock->address(), *basicBlock->successorAddress())] = basicBlock;
}

void Program::removeRange(BasicBlock *basicBlock) {
    assert(basicBlock != nullptr);
    assert(basicBlock->address() && basicBlock->successorAddress() && "Basic block must be memory-bound.");
    range2basicBlock_.erase(AddrRange(*basicBlock->address(), *basicBlock->successorAddress()));
}

BasicBlock *Program::getBasicBlockStartingAt(ByteAddr address) const {
    return nc::find(start2basicBlock_, address);
}

BasicBlock *Program::getBasicBlockCovering(ByteAddr address) const {
    return nc::find(range2basicBlock_, AddrRange(address, address + 1));
}

BasicBlock *Program::createBasicBlock() {
    return takeOwnership(std::make_unique<BasicBlock>());
}

BasicBlock *Program::createBasicBlock(ByteAddr address) {
    if (BasicBlock *result = getBasicBlockStartingAt(address)) {
        return result;
    } else if (BasicBlock *basicBlock = getBasicBlockCovering(address)) {
        removeRange(basicBlock);

        auto iterator = std::find_if(basicBlock->statements().begin(), basicBlock->statements().end(), [address](const Statement *statement) {
            return statement->instruction()->addr() >= address;
        });

        BasicBlock *result = takeOwnership(basicBlock->split(iterator, address));

        addRange(basicBlock);
        addRange(result);

        return result;
    } else {
        BasicBlock *result = takeOwnership(std::make_unique<BasicBlock>(address));
        addRange(result);
        return result;
    }
}

BasicBlock *Program::getBasicBlockForInstruction(const arch::Instruction *instruction) {
    /* If there is a basic block that starts here, just take it. */
    BasicBlock *result = getBasicBlockStartingAt(instruction->addr());

    if (!result) {
        /* Maybe this instruction stands next to an existing basic block? */
        result = getBasicBlockCovering(instruction->addr() - 1);

        /* No? Create a new block. */
        if (!result) {
            result = createBasicBlock(instruction->addr());
        }
    }

    removeRange(result);
    result->setSuccessorAddress(instruction->endAddr());
    addRange(result);

    return result;
}

BasicBlock *Program::takeOwnership(std::unique_ptr<BasicBlock> basicBlock) {
    assert(basicBlock != nullptr);

    BasicBlock *result = basicBlock.get();
    basicBlocks_.push_back(std::move(basicBlock));

    if (result->address()) {
        assert(getBasicBlockStartingAt(*result->address()) == nullptr);
        start2basicBlock_[*result->address()] = result;
    }

    return result;
}

void Program::print(QTextStream &out) const {
    out << "digraph Program" << this << " {" << endl;
    out << CFG(basicBlocks());
    out << "}" << endl;
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

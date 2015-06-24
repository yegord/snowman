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

#include "CFG.h"

#include <QTextStream>

#ifndef NDEBUG
#include <boost/unordered_set.hpp>
#endif

#include <nc/common/Foreach.h>

#include "BasicBlock.h"
#include "Jump.h"
#include "Statements.h"

namespace nc {
namespace core {
namespace ir {

CFG::CFG(const BasicBlocks &basicBlocks):
    basicBlocks_(basicBlocks)
{
    foreach (const BasicBlock *basicBlock, basicBlocks) {
        if (const Jump *jump = basicBlock->getJump()) {
            addConnections(basicBlock, jump->thenTarget());
            addConnections(basicBlock, jump->elseTarget());
        }
    }

#ifndef NDEBUG
    boost::unordered_set<const BasicBlock *> set(basicBlocks.begin(), basicBlocks.end());
    foreach (auto &pair, predecessors_) {
        assert(nc::contains(set, pair.first));
        foreach (const BasicBlock *predecessor, pair.second) {
            assert(nc::contains(set, predecessor));
        }
    }
    foreach (auto &pair, successors_) {
        assert(nc::contains(set, pair.first));
        foreach (const BasicBlock *successor, pair.second) {
            assert(nc::contains(set, successor));
        }
    }
#endif
}

void CFG::addConnections(const BasicBlock *predecessor, const JumpTarget &jumpTarget) {
    assert(predecessor);

    if (jumpTarget.basicBlock()) {
        addConnection(predecessor, jumpTarget.basicBlock());
    }
    if (jumpTarget.table()) {
        foreach (const JumpTableEntry &entry, *jumpTarget.table()) {
            if (entry.basicBlock()) {
                addConnection(predecessor, entry.basicBlock());
            }
        }
    }
}

void CFG::addConnection(const BasicBlock *predecessor, const BasicBlock *successor) {
    assert(predecessor != nullptr);
    assert(successor != nullptr);

    successors_[predecessor].push_back(successor);
    predecessors_[successor].push_back(predecessor);
}

void CFG::print(QTextStream &out) const {
    foreach (const BasicBlock *basicBlock, basicBlocks()) {
        out << *basicBlock;
    }

    foreach (auto &pair, successors_) {
        foreach (const BasicBlock *successor, pair.second) {
            out << "basicBlock" << pair.first << " -> basicBlock" << successor << ';' << endl;
        }
    }
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

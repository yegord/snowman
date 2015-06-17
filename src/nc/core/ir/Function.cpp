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

#include "Function.h"

#include <QTextStream>

#include <nc/common/Foreach.h>

#include "BasicBlock.h"
#include "CFG.h"
#include "Statements.h"
#include "Term.h"

namespace nc {
namespace core {
namespace ir {

Function::Function(): entry_(nullptr) {}

Function::~Function() {}

void Function::addBasicBlock(std::unique_ptr<BasicBlock> basicBlock) {
    basicBlock->setFunction(this);
    basicBlocks_.push_back(std::move(basicBlock));
}

bool Function::isEmpty() const {
    foreach (auto basicBlock, basicBlocks()) {
        if (!basicBlock->statements().empty()) {
            return false;
        }
    }
    return true;
}

void Function::print(QTextStream &out) const {
    out << "subgraph cluster" << this << " {" << endl;
    out << CFG(basicBlocks());
    out << '}' << endl;
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

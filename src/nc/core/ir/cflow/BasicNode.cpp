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

#include "BasicNode.h"

#include <QTextStream>

#include <nc/common/Escaping.h>
#include <nc/common/Foreach.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Jump.h>

namespace nc {
namespace core {
namespace ir {
namespace cflow {

void BasicNode::print(QTextStream &out) const {
    assert(basicBlock());

    QString label;
    QTextStream ls(&label);

    if (basicBlock()->address()) {
        ls << QString("address %1").arg(*basicBlock()->address(), 0, 16) << '\n';
    }
    ls << QString("basic block %1").arg(reinterpret_cast<uintptr_t>(basicBlock()), 0, 16) << '\n';

    foreach (const Statement *statement, basicBlock()->statements()) {
        ls << *statement;
    }

    out << "node" << this << " [shape=box,label=\"" << escapeDotString(label) << "\"]" << '\n';
}

bool BasicNode::isCondition() const {
    if (const Jump *jump = basicBlock()->getJump()) {
        return jump->isConditional() && jump->thenTarget().basicBlock() && jump->elseTarget().basicBlock();
    } else {
        return false;
    }
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

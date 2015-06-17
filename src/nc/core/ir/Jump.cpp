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

#include "Jump.h"

#include <QTextStream>

#include <nc/common/make_unique.h>

#include "Term.h"

namespace nc {
namespace core {
namespace ir {

Jump::Jump(std::unique_ptr<Term> condition, JumpTarget thenTarget, JumpTarget elseTarget):
    Statement(JUMP), condition_(std::move(condition)),
    thenTarget_(std::move(thenTarget)), elseTarget_(std::move(elseTarget))
{
    assert(condition_ != nullptr && "Jump condition must be not nullptr.");
    assert(thenTarget_ && "Then target must be valid.");
    assert(elseTarget_ && "Else target must be valid.");

    condition_->setStatement(this);

    if (thenTarget_.address()) {
        thenTarget_.address()->setStatement(this);
    }
    if (elseTarget_.address()) {
        elseTarget_.address()->setStatement(this);
    }
}

Jump::Jump(JumpTarget thenTarget):
    Statement(JUMP), thenTarget_(std::move(thenTarget))
{
    assert(thenTarget_ && "Jump target must be valid.");

    if (thenTarget_.address()) {
        thenTarget_.address()->setStatement(this);
    }
}

std::unique_ptr<Statement> Jump::doClone() const {
    if (isConditional()) {
        return std::make_unique<Jump>(condition()->clone(), thenTarget(), elseTarget());
    } else {
        return std::make_unique<Jump>(thenTarget());
    }
}

void Jump::print(QTextStream &out) const {
    if (isConditional()) {
        out << "if " << *condition() << " goto " << thenTarget() << " else goto " << elseTarget() << endl;
    } else {
        out << "goto " << thenTarget() << endl;
    }
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

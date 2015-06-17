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

#include "Term.h"

#include "Statements.h"

namespace nc { namespace core { namespace ir {

void Term::setStatement(const Statement *statement) {
    assert(statement_ == nullptr);
    assert(statement != nullptr);

    statement_ = statement;

    callOnChildren([statement](Term *term) { term->setStatement(statement); });
}

Term::AccessType Term::accessType() const {
    assert(statement() && "Each term must belong to a statement.");

    if (auto assignment = statement()->asAssignment()) {
        if (assignment->left() == this) {
            return WRITE;
        } else {
            return READ;
        }
    } else if (auto touch = statement()->asTouch()) {
        return touch->accessType();
    } else {
        return READ;
    }
}

const Term *Term::source() const {
    assert(statement() && "Each term must belong to a statement.");

    if (auto assignment = statement()->as<Assignment>()) {
        if (assignment->left() == this) {
            return assignment->right();
        }
    }

    return nullptr;
}

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */

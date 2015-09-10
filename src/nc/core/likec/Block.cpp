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

#include "Block.h"

#include <nc/common/Foreach.h>

#include "PrintContext.h"

namespace nc {
namespace core {
namespace likec {

void Block::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    foreach (const auto &declaration, declarations_) {
        fun(declaration.get());
    }
    foreach (const auto &statement, statements_) {
        fun(statement.get());
    }
}

void Block::doPrint(PrintContext &context) const {
    context.out() << "{" << endl;
    context.indentMore();

    foreach (const auto &declaration, declarations_) {
        context.outIndent();
        declaration->print(context);
        context.out() << endl;
    }

    if (!declarations_.empty() && !statements_.empty()) {
        context.out() << endl;
    }

    foreach (const auto &statement, statements_) {
        bool isCaseLabel =
            statement->statementKind() == Statement::CASE_LABEL ||
            statement->statementKind() == Statement::DEFAULT_LABEL;

        if (isCaseLabel) {
            context.indentLess();
        }

        context.outIndent();
        statement->print(context);
        context.out() << endl;

        if (isCaseLabel) {
            context.indentMore();
        }
    }

    context.indentLess();
    context.outIndent() << "}";
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

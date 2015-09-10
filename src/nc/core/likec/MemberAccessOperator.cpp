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

#include "MemberAccessOperator.h"

#include <nc/common/Unreachable.h>

#include "PrintContext.h"

#include "BinaryOperator.h"
#include "Typecast.h"
#include "UnaryOperator.h"

namespace nc {
namespace core {
namespace likec {

void MemberAccessOperator::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(compound_.get());
}

int MemberAccessOperator::precedence() const {
    switch (accessKind()) {
        case ARROW:
        case DOT:
            return 2;
    }
    unreachable();
}

void MemberAccessOperator::doPrint(PrintContext &context) const {
    bool braces = compound()->is<UnaryOperator>() ||
                  compound()->is<BinaryOperator>() ||
                  compound()->is<Typecast>();

    if (braces) {
        context.out() << "(";
    }
    compound_->print(context);
    if (braces) {
        context.out() << ")";
    }

    switch (accessKind()) {
        case ARROW:
            context.out() << "->";
            break;
        case DOT:
            context.out() << '.';
            break;
        default:
            unreachable();
            break;
    }
    
    context.out() << member_->identifier();
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

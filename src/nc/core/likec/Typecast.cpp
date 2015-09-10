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

#include "Typecast.h"

#include <nc/common/make_unique.h>

#include "PrintContext.h"
#include "Types.h"
#include "Tree.h"

namespace nc {
namespace core {
namespace likec {

void Typecast::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(operand_.get());
}

void Typecast::doPrint(PrintContext &context) const {
    int precedence = this->precedence();
    int operandPrecedence = operand()->precedence();

    int absPrecedence = abs(precedence);
    int absOperandPrecedence = abs(operandPrecedence);

    bool operandInBraces = absOperandPrecedence > absPrecedence;

    context.out() << '(' << *type() << ')';

    if (operandInBraces) {
        context.out() << '(';
    }
    operand()->print(context);
    if (operandInBraces) {
        context.out() << ')';
    }
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

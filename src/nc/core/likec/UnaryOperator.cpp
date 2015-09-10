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

#include "UnaryOperator.h"

#include <nc/common/Unreachable.h>

#include "PrintContext.h"

namespace nc {
namespace core {
namespace likec {

void UnaryOperator::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(operand_.get());
}

int UnaryOperator::precedence() const {
    switch (operatorKind()) {
        case DEREFERENCE:
        case REFERENCE:
        case BITWISE_NOT:
        case LOGICAL_NOT:
        case NEGATION:
        case PREFIX_INCREMENT:
        case PREFIX_DECREMENT:
            return -3;
        default:
            unreachable();
            return 0;
    }
}

void UnaryOperator::doPrint(PrintContext &context) const {
    switch (operatorKind()) {
        case DEREFERENCE:
            context.out() << '*';
            break;
        case REFERENCE:
            context.out() << '&';
            break;
        case BITWISE_NOT:
            context.out() << '~';
            break;
        case LOGICAL_NOT:
            context.out() << '!';
            break;
        case NEGATION:
            context.out() << '-';
            break;
        case PREFIX_INCREMENT:
            context.out() << "++";
            break;
        case PREFIX_DECREMENT:
            context.out() << "--";
            break;
        default:
            unreachable();
            break;
    }

    int precedence = this->precedence();
    int operandPrecedence = operand()->precedence();

    int absPrecedence = abs(precedence);
    int absOperandPrecedence = abs(operandPrecedence);

    bool operandInBraces = absOperandPrecedence > absPrecedence;

    /* Avoid too many minuses in a row. */
    if (operatorKind() == NEGATION || operatorKind() == PREFIX_DECREMENT) {
        if (auto unary = operand()->as<UnaryOperator>()) {
            if (unary->operatorKind() == NEGATION || unary->operatorKind() == PREFIX_DECREMENT) {
                operandInBraces = true;
            }
        }
    }

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

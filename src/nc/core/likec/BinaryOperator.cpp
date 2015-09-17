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

#include "BinaryOperator.h"

#include <cstdlib>

#include <nc/common/Unreachable.h>
#include <cstdlib>

#include "PrintContext.h"

namespace nc {
namespace core {
namespace likec {

void BinaryOperator::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(left_.get());
    fun(right_.get());
}

int BinaryOperator::precedence() const {
    switch (operatorKind()) {
        case ARRAY_SUBSCRIPT:
            return 2;
        case MUL:
        case DIV:
        case REM:
            return 5;
        case ADD:
        case SUB:
            return 6;
        case SHL:
        case SHR:
            return 7;
        case LT:
        case LEQ:
        case GT:
        case GEQ:
            return 8;
        case EQ:
        case NEQ:
            return 9;
        case BITWISE_AND:
            return 10;
        case BITWISE_XOR:
            return 11;
        case BITWISE_OR:
            return 12;
        case LOGICAL_AND:
            return 13;
        case LOGICAL_OR:
            return 14;
        case ASSIGN:
            return -16;
        case COMMA:
            return 17;
        default:
            unreachable();
            return 0;
    }
}

void BinaryOperator::doPrint(PrintContext &context) const {
    int precedence = this->precedence();
    int absPrecedence = std::abs(precedence);

    int absLeftPrecedence = std::abs(left()->precedence());
    bool leftInBraces =
        (absLeftPrecedence > absPrecedence) ||
        ((absLeftPrecedence == absPrecedence) && (precedence < 0));

    if (leftInBraces) {
        context.out() << '(';
    }
    left()->print(context);
    if (leftInBraces) {
        context.out() << ')';
    }

    if (operatorKind() == ARRAY_SUBSCRIPT) {
        context.out() << '[';
        right()->print(context);
        context.out() << ']';
        return;
    }

    switch (operatorKind()) {
        case ASSIGN:
            context.out() << " = ";
            break;
        case ADD:
            context.out() << " + ";
            break;
        case SUB:
            context.out() << " - ";
            break;
        case MUL:
            context.out() << " * ";
            break;
        case DIV:
            context.out() << " / ";
            break;
        case REM:
            context.out() << " % ";
            break;
        case BITWISE_AND:
            context.out() << " & ";
            break;
        case LOGICAL_AND:
            context.out() << " && ";
            break;
        case BITWISE_OR:
            context.out() << " | ";
            break;
        case LOGICAL_OR:
            context.out() << " || ";
            break;
        case BITWISE_XOR:
            context.out() << " ^ ";
            break;
        case SHL:
            context.out() << " << ";
            break;
        case SHR:
            context.out() << " >> ";
            break;
        case EQ:
            context.out() << " == ";
            break;
        case NEQ:
            context.out() << " != ";
            break;
        case LT:
            context.out() << " < ";
            break;
        case LEQ:
            context.out() << " <= ";
            break;
        case GT:
            context.out() << " > ";
            break;
        case GEQ:
            context.out() << " >= ";
            break;
        case COMMA:
            context.out() << ", ";
            break;
        default:
            unreachable();
    }

    int absRightPrecedence = std::abs(right()->precedence());
    bool rightInBraces =
        (absRightPrecedence > absPrecedence) ||
        ((absRightPrecedence == absPrecedence) && (precedence > 0));

    if (rightInBraces) {
        context.out() << '(';
    }
    right()->print(context);
    if (rightInBraces) {
        context.out() << ')';
    }
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

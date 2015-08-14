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
#include <nc/common/make_unique.h>

#include "PrintContext.h"
#include "BinaryOperator.h"
#include "Tree.h"
#include "Typecast.h"

namespace nc {
namespace core {
namespace likec {

void UnaryOperator::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(operand());
}

const Type *UnaryOperator::getType() const {
    const Type *operandType = operand()->getType();

    switch (operatorKind()) {
        case REFERENCE: {
            return tree().makePointerType(operandType);
        }
        case DEREFERENCE: {
            if (const PointerType *pointerType = operandType->as<PointerType>()) {
                return pointerType->pointeeType();
            }
            return tree().makeErroneousType();
        }
        case BITWISE_NOT: {
            if (operandType->isInteger()) {
                return tree().integerPromotion(operandType);
            }
            return tree().makeErroneousType();
        }
        case LOGICAL_NOT: {
            if (operandType->isScalar()) {
                return tree().makeIntegerType(tree().intSize(), false);
            }
            return tree().makeErroneousType();
        }
        case NEGATION: {
            if (operandType->isInteger() || operandType->isFloat()) {
                return operandType;
            }
            return tree().makeErroneousType();
        }
        case PREFIX_INCREMENT:
        case PREFIX_DECREMENT: {
            if (operandType->isScalar()) {
                return operandType;
            }
            return tree().makeErroneousType();
        }
        default: {
            unreachable();
        }
    }
}

Expression *UnaryOperator::rewrite() {
    rewriteChild(operand_);

    if (operatorKind() == BITWISE_NOT && operand()->getType()->size() == 1) {
        setOperatorKind(LOGICAL_NOT);
    }

    switch (operatorKind()) {
        case DEREFERENCE: {
            if (UnaryOperator *unary = operand()->as<UnaryOperator>()) {
                if (unary->operatorKind() == REFERENCE) {
                    return unary->releaseOperand().release();
                }
            }
            if (auto binary = operand()->as<BinaryOperator>()) {
                if (binary->operatorKind() == BinaryOperator::ADD) {
                    if (binary->left()->getType()->isPointer()) {
                        return std::make_unique<BinaryOperator>(tree(), BinaryOperator::ARRAY_SUBSCRIPT,
                                                                binary->releaseLeft(), binary->releaseRight()).release();
                    } else if (binary->right()->getType()->isPointer()) {
                        return std::make_unique<BinaryOperator>(tree(), BinaryOperator::ARRAY_SUBSCRIPT,
                                                                binary->releaseRight(), binary->releaseLeft()).release();
                    }
                }
            }
            return this;
        }
        case LOGICAL_NOT: {
            while (Typecast *typecast = operand()->as<Typecast>()) {
                if (typecast->type()->isScalar() && typecast->operand()->getType()->isScalar()) {
                    setOperand(typecast->releaseOperand());
                } else {
                    break;
                }
            }
            if (BinaryOperator *binary = operand()->as<BinaryOperator>()) {
                switch (binary->operatorKind()) {
                    case BinaryOperator::EQ:
                        binary->setOperatorKind(BinaryOperator::NEQ);
                        return releaseOperand().release();
                    case BinaryOperator::NEQ:
                        binary->setOperatorKind(BinaryOperator::EQ);
                        return releaseOperand().release();
                    case BinaryOperator::LT:
                        binary->setOperatorKind(BinaryOperator::GEQ);
                        return releaseOperand().release();
                    case BinaryOperator::LEQ:
                        binary->setOperatorKind(BinaryOperator::GT);
                        return releaseOperand().release();
                    case BinaryOperator::GT:
                        binary->setOperatorKind(BinaryOperator::LEQ);
                        return releaseOperand().release();
                    case BinaryOperator::GEQ:
                        binary->setOperatorKind(BinaryOperator::LT);
                        return releaseOperand().release();
                    default:
                        break;
                }
            }
            if (UnaryOperator *unary = operand()->as<UnaryOperator>()) {
                if (unary->operatorKind() == LOGICAL_NOT) {
                    if (unary->operand()->getType()->size() == 1) {
                        return unary->releaseOperand().release();
                    }
                }
            }
            return this;
        }
        default:
            return this;
    }
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

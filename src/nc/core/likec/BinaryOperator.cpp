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
#include <nc/common/make_unique.h>

#include "PrintContext.h"
#include "Tree.h"
#include "Types.h"
#include "Typecast.h"
#include "IntegerConstant.h"
#include "MemberAccessOperator.h"
#include "StructType.h"
#include "UnaryOperator.h"
#include "VariableIdentifier.h"

namespace nc {
namespace core {
namespace likec {

void BinaryOperator::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(left());
    fun(right());
}

const Type *BinaryOperator::getType() const {
    return getType(left()->getType(), right()->getType());
}

const Type *BinaryOperator::getType(const Type *leftType, const Type *rightType) const {
    switch (operatorKind()) {
        case ASSIGN:
            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && rightType->isPointer() && (leftType->isVoidPointer() || rightType->isVoidPointer())) ||
                (leftType->isPointer() && right()->isZero()))
            {
                return left()->getType();
            } else {
                return tree().makeErroneousType();
            }
            break;
        case ADD:
            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && rightType->isInteger()) {
                return leftType;
            } else if (leftType->isInteger() && rightType->isPointer() && !rightType->isVoidPointer()) {
                return rightType;
            } else {
                return tree().makeErroneousType();
            }
            break;
        case SUB:
            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && leftType == rightType) {
                /* ptrdiff_t is a signed integer of implementation-dependent size. */
                return tree().makeIntegerType(tree().ptrdiffSize(), false);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && rightType->isInteger()) {
                return leftType;
            } else {
                return tree().makeErroneousType();
            }
            break;
        case MUL: /* FALLTHROUGH */
        case DIV:
            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            } else {
                return tree().makeErroneousType();
            }
            break;
        case REM:
            if (leftType->isInteger() && rightType->isInteger()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            } else {
                return tree().makeErroneousType();
            }
            break;
        case BITWISE_AND: /* FALLTHROUGH */
        case BITWISE_OR: /* FALLTHROUGH */
        case BITWISE_XOR:
            if (leftType->isInteger() && rightType->isInteger()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            } else {
                return tree().makeErroneousType();
            }
            break;
        case LOGICAL_AND:
        case LOGICAL_OR:
            if (leftType->isScalar() && rightType->isScalar()) {
                return tree().makeIntegerType(tree().intSize(), false);
            } else {
                return tree().makeErroneousType();
            }
            break;
        case SHL: /* FALLTHROUGH */
        case SHR: {
            if (leftType->isInteger() && rightType->isInteger()) {
                return tree().integerPromotion(leftType);
            } else {
                return tree().makeErroneousType();
            }
            break;
        }
        case EQ:
        case NEQ: {
            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && (rightType->isVoidPointer() || right()->isZero())) ||
                (rightType->isPointer() && (leftType->isVoidPointer() || left()->isZero())))
            {
                return tree().makeIntegerType(tree().intSize(), false);
            } else {
                return tree().makeErroneousType();
            }
            break;
        }
        case LT:
        case LEQ:
        case GT:
        case GEQ: {
            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && rightType->isVoidPointer()) ||
                (rightType->isPointer() && leftType->isVoidPointer()))
            {
                return tree().makeIntegerType(tree().intSize(), false);
            } else {
                return tree().makeErroneousType();
            }
            break;
        }
        case COMMA: {
            return rightType;
        }
        default: {
            unreachable();
            return nullptr;
        }
    }
}

int BinaryOperator::precedence() const {
    switch (operatorKind()) {
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

Expression *BinaryOperator::rewrite() {
    rewriteChild(left_);
    rewriteChild(right_);

    /* Remove typecasts of operands if this won't change anything. */
    switch (operatorKind()) {
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case REM: {
#define REWRITE(left, right, setLeft)                                                               \
            if (Typecast *typecast = left()->as<Typecast>()) {                                      \
                if (typecast->type()->size() >= typecast->operand()->getType()->size()) {           \
                    if (getType() == getType(typecast->operand()->getType(), right()->getType())) { \
                        setLeft(typecast->releaseOperand());                                        \
                    }                                                                               \
                }                                                                                   \
            }
            REWRITE(left, right, setLeft)
            REWRITE(right, left, setRight)
#undef REWRITE
            break;
        }
        default:
            break;
    }

    /* Rewrite computing of member address. */
    switch (operatorKind()) {
        case ADD:
#define REWRITE(left, right, releaseLeft)                                                                                   \
            if (Typecast *typecast = left()->as<Typecast>()) {                                                              \
                if (typecast->type()->isInteger() && typecast->type()->size() == typecast->operand()->getType()->size()) {  \
                    if (const PointerType *pointerType = typecast->operand()->getType()->as<PointerType>()) {               \
                        if (const StructType *structType = pointerType->pointeeType()->as<StructType>()) {                  \
                            if (IntegerConstant *constant = right()->as<IntegerConstant>()) {                               \
                                if (const MemberDeclaration *member = structType->getMember(                                \
                                        constant->value().value() * CHAR_BIT)) {                                            \
                                    return new UnaryOperator(tree(), UnaryOperator::REFERENCE,                              \
                                        std::make_unique<MemberAccessOperator>(tree(), MemberAccessOperator::ARROW,         \
                                                                 typecast->releaseOperand(), member));                      \
                                }                                                                                           \
                            }                                                                                               \
                        }                                                                                                   \
                    }                                                                                                       \
                }                                                                                                           \
            }
            REWRITE(left, right, releaseLeft)
            REWRITE(right, left, releaseRight)
#undef REWRITE
            break;
        default:
            break;
    }

    /*
     * Handling pointer arithmetics:
     *
     * rdi2 = (int32_t*)((int64_t)rdi2 + 4); -> rdi2 = (int32_t*)(int64_t)(rdi2 + 1);
     */
    switch (operatorKind()) {
        case ADD:
#define REWRITE(left, right)                                                                                                \
            if (Typecast *typecast = left()->as<Typecast>()) {                                                              \
                if (typecast->type()->isInteger() && typecast->type()->size() == typecast->operand()->getType()->size()) {  \
                    if (const PointerType *pointerType = typecast->operand()->getType()->as<PointerType>()) {               \
                        if (IntegerConstant *constant = right()->as<IntegerConstant>()) {                                   \
                            if (pointerType->pointeeType()->size() != 0) {                                                  \
                                if ((constant->value().signedValue() * CHAR_BIT) % pointerType->pointeeType()->size() == 0) {\
                                    return new Typecast(tree(),                                                             \
                                        typecast->type(),                                                                   \
                                        std::make_unique<BinaryOperator>(tree(), operatorKind(),                            \
                                            typecast->releaseOperand(),                                                     \
                                            std::make_unique<IntegerConstant>(tree(),                                       \
                                                SizedValue(constant->value().size(), constant->value().signedValue() *      \
                                                     CHAR_BIT / pointerType->pointeeType()->size()),                        \
                                                constant->type())));                                                        \
                                }                                                                                           \
                            }                                                                                               \
                        }                                                                                                   \
                    }                                                                                                       \
                }                                                                                                           \
            }
            REWRITE(left, right)
            REWRITE(right, left)
            break;
        case SUB:
            REWRITE(left, right)
#undef REWRITE
            break;
        default:
            break;
    }

    /*
     * Handling increments and decrements.
     *
     * x = x + 1; -> ++x;
     * x = x - 1; -> --x;
     */
    switch (operatorKind()) {
        case ASSIGN:
            if (VariableIdentifier *leftIdent = left()->as<VariableIdentifier>()) {
                if (BinaryOperator *binary = right()->as<BinaryOperator>()) {
                    if (binary->operatorKind() == ADD) {
#define REWRITE(left, right)                                                                                                \
                        if (VariableIdentifier *rightIdent = binary->left()->as<VariableIdentifier>()) {                    \
                            if (leftIdent->declaration() == rightIdent->declaration()) {                                    \
                                if (IntegerConstant *constant = binary->right()->as<IntegerConstant>()) {                   \
                                    if (constant->value().value() == 1) {                                                   \
                                        return new UnaryOperator(tree(), UnaryOperator::PREFIX_INCREMENT, releaseLeft());   \
                                    } else if (constant->value().signedValue() == -1) {                                     \
                                        return new UnaryOperator(tree(), UnaryOperator::PREFIX_DECREMENT, releaseLeft());   \
                                    }                                                                                       \
                                }                                                                                           \
                            }                                                                                               \
                        }
                        REWRITE(left, right)
                        REWRITE(right, left)
#undef REWRITE
                    } else if (binary->operatorKind() == SUB) {
                        if (VariableIdentifier *rightIdent = binary->left()->as<VariableIdentifier>()) {
                            if (leftIdent->declaration() == rightIdent->declaration()) {
                                if (IntegerConstant *constant = binary->right()->as<IntegerConstant>()) {
                                    if (constant->value().value() == 1) {
                                        return new UnaryOperator(tree(), UnaryOperator::PREFIX_DECREMENT, releaseLeft());
                                    } else if (constant->value().signedValue() == -1) {
                                        return new UnaryOperator(tree(), UnaryOperator::PREFIX_INCREMENT, releaseLeft());
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
    }

    return this;
}

void BinaryOperator::doPrint(PrintContext &context) const {
    int precedence = this->precedence();
    int leftPrecedence = left()->precedence();
    int rightPrecedence = right()->precedence();

    int absPrecedence = std::abs(precedence);
    int absLeftPrecedence = std::abs(leftPrecedence);
    int absRightPrecedence = std::abs(rightPrecedence);

    bool leftInBraces =
        (absLeftPrecedence > absPrecedence) ||
        ((absLeftPrecedence == absPrecedence) && (precedence < 0));

    bool rightInBraces =
        (absRightPrecedence > absPrecedence) ||
        ((absRightPrecedence == absPrecedence) && (precedence > 0));
    
    if (leftInBraces) {
        context.out() << '(';
    }
    left()->print(context);
    if (leftInBraces) {
        context.out() << ')';
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

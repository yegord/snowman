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

namespace {

std::unique_ptr<Expression> divide(Expression *dividend, SignedConstantValue divisor) {
    assert(divisor != 0);

    if (auto constant = dividend->as<IntegerConstant>()) {
        if (constant->value().signedValue() % divisor == 0) {
            return std::make_unique<IntegerConstant>(
                dividend->tree(), SizedValue(constant->value().size(), constant->value().signedValue() / divisor),
                constant->type());
        }
    } else if (auto binary = dividend->as<BinaryOperator>()) {
        if (binary->operatorKind() == BinaryOperator::MUL) {
            if (auto result = divide(binary->left(), divisor)) {
                return std::make_unique<BinaryOperator>(dividend->tree(), BinaryOperator::MUL, std::move(result),
                                                        binary->releaseRight());
            } else if (auto result = divide(binary->right(), divisor)) {
                return std::make_unique<BinaryOperator>(dividend->tree(), BinaryOperator::MUL, binary->releaseLeft(),
                                                        std::move(result));
            }
        }
    }
    return nullptr;
}

/*
 * rdi2 = (int32_t*)((int64_t)rdi2 + 4); -> rdi2 = (int32_t*)(int64_t)(rdi2 + 1);
 *
 * left: (int64_t)rdi2
 * right: 4
 * result: rdi2 + 1
 */
std::unique_ptr<Expression> rewritePointerArithmetic(int operatorKind, Expression *left, Expression *right) {
    assert(operatorKind == BinaryOperator::ADD || operatorKind == BinaryOperator::SUB);

    if (auto typecast = left->as<Typecast>()) {
        if (typecast->type()->isInteger() && typecast->type()->size() == typecast->operand()->getType()->size()) {
            if (auto pointerType = typecast->operand()->getType()->as<PointerType>()) {
                if (pointerType->pointeeType()->size() != 0 && pointerType->pointeeType()->size() % CHAR_BIT == 0) {
                    if (auto quotient = divide(right, pointerType->pointeeType()->size() / CHAR_BIT)) {
                        return std::make_unique<BinaryOperator>(left->tree(), operatorKind, typecast->releaseOperand(),
                                                                std::move(quotient));
                    }
                }
            }
        }
    }

    return nullptr;
}

bool isConstant(const Expression *expression, ConstantValue value) {
    if (auto constant = expression->as<IntegerConstant>()) {
        return constant->value().value() == value;
    } else if (auto typecast = expression->as<Typecast>()) {
        return isConstant(typecast->operand(), value);
    }
    return false;
}

bool isZero(const Expression *expression) {
    return isConstant(expression, 0);
}

bool isOne(const Expression *expression) {
    return isConstant(expression, 1);
}

} // anonymous namespace

void BinaryOperator::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(left());
    fun(right());
}

const Type *BinaryOperator::getType() const {
    return getType(operatorKind(), left(), right());
}

const Type *BinaryOperator::getType(int operatorKind, const Expression *left, const Expression *right) const {
    switch (operatorKind) {
        case ASSIGN: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && rightType->isPointer() && (leftType->isVoidPointer() || rightType->isVoidPointer())) ||
                (leftType->isPointer() && isZero(right)))
            {
                return leftType;
            }
            return tree().makeErroneousType();
        }
        case ADD: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && rightType->isInteger()) {
                return leftType;
            } else if (leftType->isInteger() && rightType->isPointer() && !rightType->isVoidPointer()) {
                return rightType;
            }
            return tree().makeErroneousType();
        }
        case SUB: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && leftType == rightType) {
                /* ptrdiff_t is a signed integer of implementation-dependent size. */
                return tree().makeIntegerType(tree().ptrdiffSize(), false);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && rightType->isInteger()) {
                return leftType;
            }
            return tree().makeErroneousType();
        }
        case MUL: /* FALLTHROUGH */
        case DIV: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            }
            return tree().makeErroneousType();
        }
        case REM: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType->isInteger() && rightType->isInteger()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            }
            return tree().makeErroneousType();
        }
        case BITWISE_AND: /* FALLTHROUGH */
        case BITWISE_OR: /* FALLTHROUGH */
        case BITWISE_XOR: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType->isInteger() && rightType->isInteger()) {
                return tree().usualArithmeticConversion(leftType, rightType);
            }
            return tree().makeErroneousType();
        }
        case LOGICAL_AND:
        case LOGICAL_OR: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType->isScalar() && rightType->isScalar()) {
                return tree().makeIntegerType(tree().intSize(), false);
            }
            return tree().makeErroneousType();
        }
        case SHL: /* FALLTHROUGH */
        case SHR: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType->isInteger() && rightType->isInteger()) {
                return tree().integerPromotion(leftType);
            }
            return tree().makeErroneousType();
        }
        case EQ:
        case NEQ: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && (rightType->isVoidPointer() || isZero(right))) ||
                (rightType->isPointer() && (leftType->isVoidPointer() || isZero(left))))
            {
                return tree().makeIntegerType(tree().intSize(), false);
            }
            return tree().makeErroneousType();
        }
        case LT:
        case LEQ:
        case GT:
        case GEQ: {
            auto leftType = left->getType();
            auto rightType = right->getType();

            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && rightType->isVoidPointer()) ||
                (rightType->isPointer() && leftType->isVoidPointer()))
            {
                return tree().makeIntegerType(tree().intSize(), false);
            }
            return tree().makeErroneousType();
        }
        case COMMA: {
            return right->getType();
        }
        case ARRAY_SUBSCRIPT: {
            if (auto pointerType = getType(ADD, left, right)->as<PointerType>()) {
                return pointerType->pointeeType();
            }
            return tree().makeErroneousType();
        }
        default: {
            unreachable();
        }
    }
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
#define REWRITE(left, right, setLeft)                                                           \
            if (Typecast *typecast = left()->as<Typecast>()) {                                  \
                if (typecast->type()->size() >= typecast->operand()->getType()->size()) {       \
                    if (getType() == getType(operatorKind(), typecast->operand(), right())) {   \
                        setLeft(typecast->releaseOperand());                                    \
                    }                                                                           \
                }                                                                               \
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
     * Handling pointer arithmetic:
     *
     * rdi2 = (int32_t*)((int64_t)rdi2 + 4); -> rdi2 = (int32_t*)(int64_t)(rdi2 + 1);
     */
    switch (operatorKind()) {
        case ADD:
            if (auto result = rewritePointerArithmetic(operatorKind(), left(), right())) {
                return result.release();
            }
            if (auto result = rewritePointerArithmetic(operatorKind(), right(), left())) {
                return result.release();
            }
            break;
        case SUB:
            if (auto result = rewritePointerArithmetic(operatorKind(), left(), right())) {
                return result.release();
            }
            break;
        default:
            break;
    }

    /*
     * Handle mathematical identities.
     */
    switch (operatorKind()) {
        case ADD: {
            if (isZero(left())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseRight()).release();
            }
            if (isZero(right())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseLeft()).release();
            }
            break;
        }
        case SUB: {
            if (isZero(right())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseLeft()).release();
            }
            if (isZero(left())) {
                auto type = getType();
                return std::make_unique<UnaryOperator>(
                           tree(), UnaryOperator::NEGATION,
                           std::make_unique<Typecast>(tree(), type, releaseRight())).release();
            }
            break;
        }
        case MUL: {
            if (isOne(left())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseRight()).release();
            }
            if (isOne(right())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseLeft()).release();
            }
            break;
        }
        case SHL:
        case SHR: {
            if (isZero(right())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseLeft()).release();
            }
            break;
        }
        case BITWISE_OR:
        case BITWISE_XOR:
        case LOGICAL_OR: {
            if (isZero(left())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseRight()).release();
            }
            if (isZero(right())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseLeft()).release();
            }
            break;
        }
        case LOGICAL_AND: {
            if (isOne(right())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseLeft()).release();
            }
            if (isOne(left())) {
                auto type = getType();
                return std::make_unique<Typecast>(tree(), type, releaseRight()).release();
            }
            break;
        }
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

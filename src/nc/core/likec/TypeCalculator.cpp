#include "TypeCalculator.h"

#include <nc/common/Unreachable.h>

#include "BinaryOperator.h"
#include "CallOperator.h"
#include "Expression.h"
#include "FunctionDeclaration.h"
#include "FunctionIdentifier.h"
#include "FunctionPointerType.h"
#include "IntegerConstant.h"
#include "LabelIdentifier.h"
#include "MemberAccessOperator.h"
#include "String.h"
#include "Tree.h"
#include "Typecast.h"
#include "UnaryOperator.h"
#include "UndeclaredIdentifier.h"
#include "Utils.h"
#include "VariableIdentifier.h"

namespace nc {
namespace core {
namespace likec {

const Type *TypeCalculator::getType(const Expression *node) {
    switch (node->expressionKind()) {
        case Expression::BINARY_OPERATOR:
            return getType(node->as<BinaryOperator>());
        case Expression::CALL_OPERATOR:
            return getType(node->as<CallOperator>());
        case Expression::FUNCTION_IDENTIFIER:
            return getType(node->as<FunctionIdentifier>());
        case Expression::INTEGER_CONSTANT:
            return getType(node->as<IntegerConstant>());
        case Expression::LABEL_IDENTIFIER:
            return getType(node->as<LabelIdentifier>());
        case Expression::MEMBER_ACCESS_OPERATOR:
            return getType(node->as<MemberAccessOperator>());
        case Expression::STRING:
            return getType(node->as<String>());
        case Expression::TYPECAST:
            return getType(node->as<Typecast>());
        case Expression::UNARY_OPERATOR:
            return getType(node->as<UnaryOperator>());
        case Expression::VARIABLE_IDENTIFIER:
            return getType(node->as<VariableIdentifier>());
        case Expression::UNDECLARED_IDENTIFIER:
            return getType(node->as<UndeclaredIdentifier>());
    }
    unreachable();
}

const Type *TypeCalculator::getType(const BinaryOperator *node) {
    return getBinaryOperatorType(node->operatorKind(), node->left(), node->right());
}

const Type *TypeCalculator::getType(const CallOperator *node) {
    if (auto functionPointerType = getType(node->callee())->as<FunctionPointerType>()) {
        return functionPointerType->returnType();
    } else {
        return tree_.makeErroneousType();
    }
}

const Type *TypeCalculator::getType(const FunctionIdentifier *node) {
    return node->declaration()->type();
}

const Type *TypeCalculator::getType(const IntegerConstant *node) {
    return node->type();
}

const Type *TypeCalculator::getType(const LabelIdentifier *) {
    return tree_.makePointerType(tree_.pointerSize(), tree_.makeVoidType());
}

const Type *TypeCalculator::getType(const MemberAccessOperator *node) {
    return node->member()->type();
}

const Type *TypeCalculator::getType(const String *) {
    return tree_.makePointerType(tree_.pointerSize(), tree_.makeIntegerType(CHAR_BIT, false));
}

const Type *TypeCalculator::getType(const Typecast *node) {
    return node->type();
}

const Type *TypeCalculator::getType(const UnaryOperator *node) {
    auto operandType = getType(node->operand());

    switch (node->operatorKind()) {
        case UnaryOperator::REFERENCE: {
            return tree_.makePointerType(operandType);
        }
        case UnaryOperator::DEREFERENCE: {
            if (const PointerType *pointerType = operandType->as<PointerType>()) {
                return pointerType->pointeeType();
            }
            return tree_.makeErroneousType();
        }
        case UnaryOperator::BITWISE_NOT: {
            if (operandType->isInteger()) {
                return tree_.integerPromotion(operandType);
            }
            return tree_.makeErroneousType();
        }
        case UnaryOperator::LOGICAL_NOT: {
            if (operandType->isScalar()) {
                return tree_.makeIntegerType(tree_.intSize(), false);
            }
            return tree_.makeErroneousType();
        }
        case UnaryOperator::NEGATION: {
            if (operandType->isInteger() || operandType->isFloat()) {
                return operandType;
            }
            return tree_.makeErroneousType();
        }
        case UnaryOperator::PREFIX_INCREMENT:
        case UnaryOperator::PREFIX_DECREMENT: {
            if (operandType->isScalar()) {
                return operandType;
            }
            return tree_.makeErroneousType();
        }
    }
    unreachable();
}

const Type *TypeCalculator::getType(const VariableIdentifier *node) {
    return node->declaration()->type();
}

const Type *TypeCalculator::getType(const UndeclaredIdentifier *node) {
    return node->type();
}

const Type *TypeCalculator::getBinaryOperatorType(int operatorKind, const Expression *left, const Expression *right) {
    switch (operatorKind) {
        case BinaryOperator::ASSIGN: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && rightType->isPointer() && (leftType->isVoidPointer() || rightType->isVoidPointer())) ||
                (leftType->isPointer() && isZero(right)))
            {
                return leftType;
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::ADD: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree_.usualArithmeticConversion(leftType, rightType);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && rightType->isInteger()) {
                return leftType;
            } else if (leftType->isInteger() && rightType->isPointer() && !rightType->isVoidPointer()) {
                return rightType;
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::SUB: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree_.usualArithmeticConversion(leftType, rightType);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && leftType == rightType) {
                /* ptrdiff_t is a signed integer of implementation-dependent size. */
                return tree_.makeIntegerType(tree_.ptrdiffSize(), false);
            } else if (leftType->isPointer() && !leftType->isVoidPointer() && rightType->isInteger()) {
                return leftType;
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::MUL: /* FALLTHROUGH */
        case BinaryOperator::DIV: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType->isArithmetic() && rightType->isArithmetic()) {
                return tree_.usualArithmeticConversion(leftType, rightType);
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::REM: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType->isInteger() && rightType->isInteger()) {
                return tree_.usualArithmeticConversion(leftType, rightType);
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::BITWISE_AND: /* FALLTHROUGH */
        case BinaryOperator::BITWISE_OR: /* FALLTHROUGH */
        case BinaryOperator::BITWISE_XOR: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType->isInteger() && rightType->isInteger()) {
                return tree_.usualArithmeticConversion(leftType, rightType);
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::LOGICAL_AND:
        case BinaryOperator::LOGICAL_OR: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType->isScalar() && rightType->isScalar()) {
                return tree_.makeIntegerType(1, false);
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::SHL: /* FALLTHROUGH */
        case BinaryOperator::SHR: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType->isInteger() && rightType->isInteger()) {
                return tree_.integerPromotion(leftType);
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::EQ: /* FALLTHROUGH */
        case BinaryOperator::NEQ: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && (rightType->isVoidPointer() || isZero(right))) ||
                (rightType->isPointer() && (leftType->isVoidPointer() || isZero(left))))
            {
                return tree_.makeIntegerType(1, false);
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::LT: /* FALLTHROUGH */
        case BinaryOperator::LEQ: /* FALLTHROUGH */
        case BinaryOperator::GT: /* FALLTHROUGH */
        case BinaryOperator::GEQ: {
            auto leftType = getType(left);
            auto rightType = getType(right);

            if (leftType == rightType ||
                (leftType->isArithmetic() && rightType->isArithmetic()) ||
                (leftType->isPointer() && rightType->isVoidPointer()) ||
                (rightType->isPointer() && leftType->isVoidPointer()))
            {
                return tree_.makeIntegerType(1, false);
            }
            return tree_.makeErroneousType();
        }
        case BinaryOperator::COMMA: {
            return getType(right);
        }
        case BinaryOperator::ARRAY_SUBSCRIPT: {
            if (auto pointerType = getBinaryOperatorType(BinaryOperator::ADD, left, right)->as<PointerType>()) {
                return pointerType->pointeeType();
            }
            return tree_.makeErroneousType();
        }
    }
    unreachable();
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

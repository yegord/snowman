/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Simplify.h"

#include <nc/common/make_unique.h>

#include "BinaryOperator.h"
#include "IntegerConstant.h"
#include "Typecast.h"
#include "UnaryOperator.h"

namespace nc {
namespace core {
namespace likec {

namespace {

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

} // anonymous namespace

std::unique_ptr<Expression> simplify(std::unique_ptr<Expression> expression) {
    assert(expression);
    return std::unique_ptr<Expression>(expression.release()->rewrite());
}

std::unique_ptr<Expression> simplifyBooleanExpression(std::unique_ptr<Expression> expression) {
    assert(expression);
    expression = simplify(std::move(expression));
    assert(expression);

    if (auto binary = expression->as<BinaryOperator>()) {
        if (binary->operatorKind() == BinaryOperator::NEQ) {
            if (isZero(binary->right())) {
                return simplifyBooleanExpression(binary->releaseLeft());
            }
            if (isZero(binary->left())) {
                return simplifyBooleanExpression(binary->releaseRight());
            }
        } else if (binary->operatorKind() == BinaryOperator::EQ) {
            if (isZero(binary->right())) {
                return simplifyBooleanExpression(std::make_unique<UnaryOperator>(
                    expression->tree(), UnaryOperator::LOGICAL_NOT, binary->releaseLeft()));
            }
            if (isZero(binary->left())) {
                return simplifyBooleanExpression(std::make_unique<UnaryOperator>(
                    expression->tree(), UnaryOperator::LOGICAL_NOT, binary->releaseRight()));
            }
        }
    }

    return std::move(expression);
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

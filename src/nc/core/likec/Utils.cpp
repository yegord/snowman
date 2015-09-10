#include "Utils.h"

#include <cassert>

#include <nc/common/make_unique.h>

#include <nc/core/likec/BinaryOperator.h>
#include <nc/core/likec/Expression.h>
#include <nc/core/likec/IntegerConstant.h>
#include <nc/core/likec/Typecast.h>

namespace nc {
namespace core {
namespace likec {

std::unique_ptr<Expression> divide(Expression *dividend, SignedConstantValue divisor) {
    assert(divisor != 0);

    if (auto constant = dividend->as<IntegerConstant>()) {
        if (constant->value().signedValue() % divisor == 0) {
            return std::make_unique<IntegerConstant>(
                SizedValue(constant->value().size(), constant->value().signedValue() / divisor),
                constant->type());
        }
    } else if (auto binary = dividend->as<BinaryOperator>()) {
        if (binary->operatorKind() == BinaryOperator::MUL) {
            if (auto result = divide(binary->left().get(), divisor)) {
                return std::make_unique<BinaryOperator>(BinaryOperator::MUL, std::move(result),
                                                        std::move(binary->right()));
            } else if (auto result = divide(binary->right().get(), divisor)) {
                return std::make_unique<BinaryOperator>(BinaryOperator::MUL, std::move(binary->left()),
                                                        std::move(result));
            }
        }
    }
    return nullptr;
}

namespace {

bool isConstant(const Expression *expression, ConstantValue value) {
    if (auto constant = expression->as<IntegerConstant>()) {
        return constant->value().value() == value;
    } else if (auto typecast = expression->as<Typecast>()) {
        return isConstant(typecast->operand(), value);
    }
    return false;
}

} // anonymous namespace

bool isZero(const Expression *expression) {
    return isConstant(expression, 0);
}

bool isOne(const Expression *expression) {
    return isConstant(expression, 1);
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory>

#include "Expression.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Base class for binary operators.
 */
class BinaryOperator: public Expression {
    NC_BASE_CLASS(BinaryOperator, operatorKind)

    std::unique_ptr<Expression> left_; ///< Left operand.
    std::unique_ptr<Expression> right_; ///< Right operand.
    
    public:

    /**
     * Operator id.
     */
    enum {
        ASSIGN,
        ADD,
        SUB,
        MUL,
        DIV,
        REM,
        BITWISE_AND,
        LOGICAL_AND,
        BITWISE_OR,
        LOGICAL_OR,
        BITWISE_XOR,
        SHL,
        SHR,
        EQ,
        NEQ,
        LT,
        LEQ,
        GT,
        GEQ,
        COMMA,
        ARRAY_SUBSCRIPT
    };

    /**
     * Class constructor.
     *
     * \param[in] operatorKind Operator's kind.
     * \param[in] left Left operand.
     * \param[in] right Right operand.
     */
    BinaryOperator(int operatorKind, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right):
        Expression(BINARY_OPERATOR), operatorKind_(operatorKind), left_(std::move(left)), right_(std::move(right))
    {}

    /**
     * Sets operator's kind.
     *
     * \param operatorKind New kind.
     */
    void setOperatorKind(int operatorKind) { operatorKind_ = operatorKind; }

    /**
     * \return Left operand.
     */
    std::unique_ptr<Expression> &left() { return left_; }

    /**
     * \return Left operand.
     */
    const Expression *left() const { return left_.get(); }

    /**
     * \return Right operand.
     */
    std::unique_ptr<Expression> &right() { return right_; }

    /**
     * \return Right operand.
     */
    const Expression *right() const { return right_.get(); }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Expression, nc::core::likec::BinaryOperator, nc::core::likec::Expression::BINARY_OPERATOR)

/* vim:set et sts=4 sw=4: */

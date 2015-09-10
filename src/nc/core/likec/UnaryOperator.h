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
 * Base class for unary operators.
 */
class UnaryOperator: public Expression {
    NC_BASE_CLASS(UnaryOperator, operatorKind)
    std::unique_ptr<Expression> operand_; ///< Operand.

public:
    /**
     * Operator id.
     */
    enum Kind {
        DEREFERENCE,            ///< *a
        REFERENCE,              ///< &a
        BITWISE_NOT,            ///< ~a
        LOGICAL_NOT,            ///< !a
        NEGATION,               ///< -a
        PREFIX_INCREMENT,       ///< ++a
        PREFIX_DECREMENT,       ///< --a
        USER_OPERATOR = 1000    ///< Base value for user-defined operators.
    };

    /**
     * Class constructor.
     *
     * \param[in] operatorKind Operator's kind.
     * \param[in] operand Operand.
     */
    UnaryOperator(int operatorKind, std::unique_ptr<Expression> operand):
        Expression(UNARY_OPERATOR), operatorKind_(operatorKind), operand_(std::move(operand)) {}

    /**
     * Sets operator's kind.
     *
     * \param operatorKind New kind.
     */
    void setOperatorKind(int operatorKind) { operatorKind_ = operatorKind; }

    /**
     * \return Operand.
     */
    std::unique_ptr<Expression> &operand() { return operand_; }

    /**
     * \return Operand.
     */
    const Expression *operand() const { return operand_.get(); }

    int precedence() const override;

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Expression, nc::core::likec::UnaryOperator, nc::core::likec::Expression::UNARY_OPERATOR)

/* vim:set et sts=4 sw=4: */

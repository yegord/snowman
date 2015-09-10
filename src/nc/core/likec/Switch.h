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

#include "Statement.h"

#include <memory>

namespace nc {
namespace core {
namespace likec {

class Expression;

/**
 * Switch.
 */
class Switch: public Statement {
    std::unique_ptr<Expression> expression_; ///< Switch expression.
    std::unique_ptr<Statement> body_; ///< Switch body.

public:
    /**
     * Class constructor.
     *
     * \param[in] expression Valid pointer to the switch expression.
     * \param[in] body Valid pointer to the switch body.
     */
    Switch(std::unique_ptr<Expression> expression, std::unique_ptr<Statement> body):
        Statement(SWITCH), expression_(std::move(expression)), body_(std::move(body))
    {}

    /**
     * \return Switch expression.
     */
    std::unique_ptr<Expression> &expression() { return expression_; }

    /**
     * \return Switch expression.
     */
    const Expression *expression() const { return expression_.get(); }

    /**
     * \return Switch body.
     */
    std::unique_ptr<Statement> &body() { return body_; }

    /**
     * \return Switch body.
     */
    const Statement *body() const { return body_.get(); }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Statement, nc::core::likec::Switch, nc::core::likec::Statement::SWITCH)

/* vim:set et sts=4 sw=4: */

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
#include "Statement.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Conditional statement (if-then-?else?).
 */
class If: public Statement {
    std::unique_ptr<Expression> condition_; ///< Condition.
    std::unique_ptr<Statement> thenStatement_; ///< Then-statement.
    std::unique_ptr<Statement> elseStatement_; ///< Else-statement.

public:
    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] condition Condition.
     * \param[in] thenStatement Statement of "then" branch.
     * \param[in] elseStatement Statement of "else" branch.
     */
    If(Tree &tree,
       std::unique_ptr<Expression> condition,
       std::unique_ptr<Statement> thenStatement,
       std::unique_ptr<Statement> elseStatement = nullptr)
    :  Statement(tree, IF),
       condition_(std::move(condition)),
       thenStatement_(std::move(thenStatement)),
       elseStatement_(std::move(elseStatement))
    {}

    /**
     * \return Condition.
     */
    Expression *condition() { return condition_.get(); }

    /**
     * \return Condition.
     */
    const Expression *condition() const { return condition_.get(); }

    /**
     * \return Then-statement.
     */
    Statement *thenStatement() { return thenStatement_.get(); }

    /**
     * \return Then-statement.
     */
    const Statement *thenStatement() const { return thenStatement_.get(); }

    /**
     * \return Else-statement.
     */
    Statement *elseStatement() { return elseStatement_.get(); }

    /**
     * \return Else-statement.
     */
    const Statement *elseStatement() const { return elseStatement_.get(); }

    If *rewrite() override;

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Statement, nc::core::likec::If, nc::core::likec::Statement::IF)

/* vim:set et sts=4 sw=4: */

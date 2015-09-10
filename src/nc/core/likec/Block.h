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

#include <cassert>
#include <memory> /* unique_ptr */
#include <vector>

#include "Statement.h"
#include "VariableDeclaration.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Block statement. Contains variable declarations and statements.
 */
class Block: public Statement {
    std::vector<std::unique_ptr<Declaration> > declarations_; ///< Declarations.
    std::vector<std::unique_ptr<Statement> > statements_; ///< Statements.

public:
    /**
     * Constructor.
     */
    Block(): Statement(BLOCK) {}

    /**
     * \return Declarations.
     */
    std::vector<std::unique_ptr<Declaration>> &declarations() { return declarations_; }

    /**
     * \return Declarations.
     */
    const std::vector<Declaration *> &declarations() const {
        return reinterpret_cast<const std::vector<Declaration *> &>(declarations_);
    }

    /**
     * Adds a definition to the block.
     *
     * \param declaration Valid pointer to a declaration.
     */
    void addDeclaration(std::unique_ptr<Declaration> declaration) {
        assert(declaration != nullptr);
        declarations_.push_back(std::move(declaration));
    }

    /**
     * \return Declarations.
     */
    std::vector<std::unique_ptr<Statement>> &statements() { return statements_; }

    /**
     * \return Declarations.
     */
    const std::vector<Statement *> &statements() const {
        return reinterpret_cast<const std::vector<Statement *> &>(statements_);
    }

    /**
     * Adds a statement to the block.
     *
     * \param statement Valid pointer to a statement.
     */
    void addStatement(std::unique_ptr<Statement> statement) {
        assert(statement != nullptr);
        statements_.push_back(std::move(statement));
    }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Statement, nc::core::likec::Block, nc::core::likec::Statement::BLOCK)

/* vim:set et sts=4 sw=4: */

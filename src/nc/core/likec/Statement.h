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

#include <nc/common/Unused.h>

#include "TreeNode.h"

namespace nc {
namespace core {

namespace ir {
    class Statement;
}

namespace likec {

/**
 * Base class for statement node.
 */
class Statement: public TreeNode {
    NC_BASE_CLASS(Statement, statementKind)

    const ir::Statement *statement_; ///< IR statement from which this statement was created.

public:
    enum {
        BLOCK,                          ///< Block.
        BREAK,                          ///< Break.
        CONTINUE,                       ///< Continue.
        DO_WHILE,                       ///< Do-While loop.
        EXPRESSION_STATEMENT,           ///< Expression statement.
        GOTO,                           ///< Goto.
        IF,                             ///< Conditional statement.
        LABEL_STATEMENT,                ///< Label.
        RETURN,                         ///< Return.
        WHILE,                          ///< While loop.
        INLINE_ASSEMBLY,                ///< Inline assembly.
        SWITCH,                         ///< Switch.
        CASE_LABEL,                     ///< Case label.
        DEFAULT_LABEL,                  ///< Default case label.
    };

    /**
     * Class constructor.
     *
     * \param[in] statementKind Statement kind.
     */
    explicit Statement(int statementKind):
        TreeNode(STATEMENT), statementKind_(statementKind), statement_(nullptr)
    {}

    /**
     * \return Pointer to the IR statement from which this statement was created. Can be nullptr.
     */
    const ir::Statement *statement() const { return statement_; }

    /**
     * \param[in] statement Valid pointer to a statement.
     */
    void setStatement(const ir::Statement *statement) {
        assert(statement != nullptr);
        assert(statement_ == nullptr); /* Must be used for initialization only. */

        statement_ = statement;
    }
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::TreeNode, nc::core::likec::Statement, nc::core::likec::TreeNode::STATEMENT)

/* vim:set et sts=4 sw=4: */

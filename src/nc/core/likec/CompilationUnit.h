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

#include <vector>
#include <memory>

#include "Declaration.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Compilation unit.
 */
class CompilationUnit: public TreeNode {
    std::vector<std::unique_ptr<Declaration>> declarations_; ///< Declarations.

public:
    /**
     * Constructor.
     */
    CompilationUnit(): TreeNode(COMPILATION_UNIT) {}

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
     * Adds a declaration to the unit.
     *
     * \param declaration Valid pointer to a declaration. 
     */
    void addDeclaration(std::unique_ptr<Declaration> declaration) {
        assert(declaration);
        declarations_.push_back(std::move(declaration));
    }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::TreeNode, nc::core::likec::CompilationUnit, nc::core::likec::TreeNode::COMPILATION_UNIT)

/* vim:set et sts=4 sw=4: */

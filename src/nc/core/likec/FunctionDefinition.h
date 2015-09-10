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
#include <memory> /* unique_ptr */

#include "Block.h"
#include "FunctionDeclaration.h"
#include "LabelDeclaration.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Function definition = function declaration + function body (block).
 */
class FunctionDefinition: public FunctionDeclaration {
    std::unique_ptr<Block> block_; ///< Block of the function.
    std::vector<std::unique_ptr<LabelDeclaration>> labels_; ///< Label declarations.

public:
    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] identifier Function name.
     * \param[in] returnType Function return type.
     * \param[in] variadic Whether function has variable number of arguments.
     */
    FunctionDefinition(Tree &tree, QString identifier, const Type *returnType, bool variadic = false):
        FunctionDeclaration(tree, FUNCTION_DEFINITION, std::move(identifier), returnType, variadic),
        block_(new Block())
    {}

    /**
     * \return Block of the function.
     */
    std::unique_ptr<Block> &block() { return block_; }

    /**
     * \return Block of the function.
     */
    const std::unique_ptr<const Block> &block() const {
        return reinterpret_cast<const std::unique_ptr<const Block> &>(block_);
    }

    /**
     * \return Labels of the function.
     */
    std::vector<std::unique_ptr<LabelDeclaration>> &labels() { return labels_; };

    /**
     * Adds invisible label declaration to the function.
     *
     * \param[in] label Valid pointer to the label declaration.
     */
    void addLabel(std::unique_ptr<LabelDeclaration> label) { labels_.push_back(std::move(label)); }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Declaration, nc::core::likec::FunctionDefinition, nc::core::likec::Declaration::FUNCTION_DEFINITION)

/* vim:set et sts=4 sw=4: */

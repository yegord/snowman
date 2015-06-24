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

#include <string>

#include "TreeNode.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Declaration of an identifier (variable, function, type, whatever).
 */
class Declaration: public TreeNode {
    NC_BASE_CLASS(Declaration, declarationKind)

    const QString identifier_;

public:

    /**
     * Declaration kind.
     */
    enum {
        FUNCTION_DECLARATION,           ///< Function declaration.
        FUNCTION_DEFINITION,            ///< Function definition.
        LABEL_DECLARATION,              ///< Label declaration.
        MEMBER_DECLARATION,             ///< Declaration of a struct or union member.
        STRUCT_TYPE_DECLARATION,        ///< Declaration of structural type.
        VARIABLE_DECLARATION,           ///< Variable declaration.
        USER_DECLARATION = 1000         ///< Base for user-defined declarations.
    };

    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] declarationKind Declaration kind.
     * \param[in] identifier Name of declared entity.
     */
    Declaration(Tree &tree, int declarationKind, QString identifier):
        TreeNode(tree, DECLARATION), declarationKind_(declarationKind), identifier_(std::move(identifier))
    {}

    /**
     * \return Name of declared entity.
     */
    const QString &identifier() const { return identifier_; }

    Declaration *rewrite() override { return this; }
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::TreeNode, nc::core::likec::Declaration, nc::core::likec::TreeNode::DECLARATION)

/* vim:set et sts=4 sw=4: */

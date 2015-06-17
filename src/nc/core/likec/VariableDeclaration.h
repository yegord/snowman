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
#include <memory>

#include "Commentable.h"
#include "Declaration.h"
#include "Expression.h"
#include "VariableIdentifier.h"

namespace nc {
namespace core {
namespace likec {

class Type;

/**
 * Variable declaration.
 */
class VariableDeclaration: public Declaration, public Commentable {
    const Type *type_; ///< Type of this variable.
    std::unique_ptr<VariableIdentifier> variableIdentifier_; ///< Variable identifier node. Needed for refactoring.
    std::unique_ptr<Expression> initialValue_; ///< Initial value of this variable.

public:
    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] identifier Name of this variable.
     * \param[in] type Valid pointer to the type of this variable.
     * \param[in] initialValue Pointer to the expression representing the initial value. Can be nullptr.
     */
    VariableDeclaration(Tree &tree, QString identifier, const Type *type,
                        std::unique_ptr<Expression> initialValue = nullptr)
        : Declaration(tree, VARIABLE_DECLARATION, std::move(identifier)), type_(type),
          variableIdentifier_(new VariableIdentifier(tree, this)), initialValue_(std::move(initialValue)) {
        assert(type != nullptr);
    }

    /**
     * \return Valid pointer to the type of this variable.
     */
    const Type *type() const { return type_; }

    /**
     * \return Valid pointer to the variable identifier used in the declaration.
     */
    const VariableIdentifier *variableIdentifier() const { return variableIdentifier_.get(); }

    /**
     * \return Pointer to the expression representing the initial value. Can be nullptr.
     */
    const Expression *initialValue() const { return initialValue_.get(); }

    VariableDeclaration *rewrite() override;

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Declaration, nc::core::likec::VariableDeclaration, nc::core::likec::Declaration::VARIABLE_DECLARATION)

/* vim:set et sts=4 sw=4: */

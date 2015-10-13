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

#include "ArgumentDeclaration.h"
#include "Commentable.h"
#include "Declaration.h"
#include "FunctionIdentifier.h"
#include "FunctionPointerType.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Function declaration.
 */
class FunctionDeclaration: public Declaration, public Commentable {
    std::unique_ptr<FunctionPointerType> type_; ///< Type of pointer to this function.
    std::vector<std::unique_ptr<ArgumentDeclaration> > arguments_; ///< Function arguments.
    std::unique_ptr<FunctionIdentifier> functionIdentifier_;

public:
    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] identifier Function name.
     * \param[in] returnType Function return type.
     * \param[in] variadic Whether function has variable number of arguments.
     */
    FunctionDeclaration(Tree &tree, QString identifier, const Type *returnType, bool variadic = false);

protected:
    /**
     * Class constructor having declaration kind argument used by FunctionDefinition.
     *
     * \param[in] tree Owning tree.
     * \param[in] declarationKind Declaration kind.
     * \param[in] identifier Function name.
     * \param[in] returnType Function return type.
     * \param[in] variadic Whether function has variable number of arguments.
     */
    FunctionDeclaration(Tree &tree, int declarationKind, QString identifier, const Type *returnType, bool variadic = false);

public:
    /**
     * \return Type of pointer to this function.
     */
    const FunctionPointerType *type() const { return type_.get(); }

    /**
     * \return Valid pointer to the function identifier used in the declaration.
     */
    const FunctionIdentifier *functionIdentifier() const { return functionIdentifier_.get(); }

    /**
     * Function arguments.
     */
    const std::vector<std::unique_ptr<ArgumentDeclaration>> &arguments() const { return arguments_; }

    /**
     * Adds argument to the function.
     *
     * \param argument Valid pointer to the argument declaration.
     */
    void addArgument(std::unique_ptr<ArgumentDeclaration> argument);

    /**
     * \param declaration Valid pointer to the first declaration of the function.
     */
    void setFirstDeclaration(FunctionDeclaration *declaration) { functionIdentifier_->setDeclaration(declaration); }

    /**
     * \return Valid pointer to the first declaration of the function.
     */
    FunctionDeclaration *getFirstDeclaration() const { return functionIdentifier_->declaration(); }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Declaration, nc::core::likec::FunctionDeclaration, nc::core::likec::Declaration::FUNCTION_DECLARATION)

/* vim:set et sts=4 sw=4: */

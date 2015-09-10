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

#include "Expression.h"

namespace nc {
namespace core {
namespace likec {

class VariableDeclaration;

/**
 * Variable identifier.
 */
class VariableIdentifier: public Expression {
    VariableDeclaration *declaration_; ///< Variable declaration.

public:
    /**
     * Class constructor.
     *
     * \param[in] declaration Variable declaration.
     */
    explicit VariableIdentifier(VariableDeclaration *declaration):
        Expression(VARIABLE_IDENTIFIER), declaration_(declaration)
    {}

    /**
     * \return Variable declaration.
     */
    VariableDeclaration *declaration() { return declaration_; }

    /**
     * \return Variable declaration.
     */
    const VariableDeclaration *declaration() const { return declaration_; }

protected:
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Expression, nc::core::likec::VariableIdentifier, nc::core::likec::Expression::VARIABLE_IDENTIFIER)

/* vim:set et sts=4 sw=4: */

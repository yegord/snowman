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

#include <memory> /* std::unique_ptr */

#include "Expression.h"

namespace nc {
namespace core {
namespace likec {

class Type;

/**
 * Typecast.
 */
class Typecast: public Expression {
    const Type *type_; ///< Type to cast to.
    std::unique_ptr<Expression> operand_; ///< Operand.

public:
    /**
     * Class constructor.
     *
     * \param[in] type Type to cast to.
     * \param[in] operand Expression to be casted.
     */
    Typecast(const Type *type, std::unique_ptr<Expression> operand):
        Expression(TYPECAST), type_(type), operand_(std::move(operand))
    {}

    /**
     * \return Type to cast to.
     */
    const Type *type() const { return type_; }

    /**
     * \return Operand.
     */
    std::unique_ptr<Expression> &operand() { return operand_; }

    /**
     * \return Operand.
     */
    const Expression *operand() const { return operand_.get(); }

    int precedence() const override { return -3; }

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Expression, nc::core::likec::Typecast, nc::core::likec::Expression::TYPECAST)

/* vim:set et sts=4 sw=4: */

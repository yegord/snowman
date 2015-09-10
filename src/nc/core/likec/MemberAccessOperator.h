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

#include <memory> /* unique_ptr */

#include "Expression.h"
#include "MemberDeclaration.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Access to a struct or union member.
 */
class MemberAccessOperator: public Expression {
public:
    /**
     * Operator id.
     */
    enum AccessKind {
        ARROW,  ///< a->b
        DOT,    ///< a.b
    };

private:
    AccessKind accessKind_; ///< Operator id.
    std::unique_ptr<Expression> compound_; ///< Accessed struct or union.
    const MemberDeclaration *member_; ///< Accessed member.

public:
    /**
     * Class constructor.
     *
     * \param[in] accessKind Access kind.
     * \param[in] compound Accessed struct or union.
     * \param[in] member Accessed member.
     */
    MemberAccessOperator(AccessKind accessKind, std::unique_ptr<Expression> compound, const MemberDeclaration *member):
        Expression(MEMBER_ACCESS_OPERATOR), accessKind_(accessKind), compound_(std::move(compound)), member_(member)
    {}

    /**
     * \return Operator id.
     */
    AccessKind accessKind() const { return accessKind_; }

    /**
     * Sets operator id.
     */
    void setAccessKind(AccessKind accessKind) { accessKind_ = accessKind; }

    /**
     * \return Accessed struct or union.
     */
    std::unique_ptr<Expression> &compound() { return compound_; }

    /**
     * \return Accessed struct or union.
     */
    const Expression *compound() const { return compound_.get(); }

    /**
     * \return Accessed member.
     */
    const MemberDeclaration *member() const { return member_; }

    int precedence() const override;

protected:
    void doCallOnChildren(const std::function<void(TreeNode *)> &fun) override;
    void doPrint(PrintContext &context) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Expression, nc::core::likec::MemberAccessOperator, nc::core::likec::Expression::MEMBER_ACCESS_OPERATOR)

/* vim:set et sts=4 sw=4: */

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
    class Term;
}

namespace likec {

class Type;

/**
 * Base class for different kinds of expressions.
 *
 * Expression is a statement.
 */
class Expression: public TreeNode {
    NC_BASE_CLASS(Expression, expressionKind)

    const ir::Term *term_; ///< Term this expression was created from.

public:
    enum {
        BINARY_OPERATOR,        ///< Binary operator.
        CALL_OPERATOR,          ///< Function call.
        FUNCTION_IDENTIFIER,    ///< Function identifier.
        INTEGER_CONSTANT,       ///< Integer constant.
        LABEL_IDENTIFIER,       ///< Identifier of a label.
        MEMBER_ACCESS_OPERATOR, ///< Member access operator: a.b or a-&gt;b.
        STRING,                 ///< C string.
        TYPECAST,               ///< (t)a.
        UNARY_OPERATOR,         ///< Unary operator.
        VARIABLE_IDENTIFIER,    ///< Identifier of a variable.
        USER_EXPRESSION = 1000  ///< Base for user-defined expressions.
    };

    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] expressionKind Kind of expression.
     */
    Expression(Tree &tree, int expressionKind):
        TreeNode(tree, EXPRESSION), expressionKind_(expressionKind), term_(nullptr)
    {}

    /**
     * \return Term this expression was created from.
     */
    const ir::Term *term() const {
        return term_;
    }

    /**
     * \param[in] term Term this expression was created from.
     */
    void setTerm(const ir::Term *term) {
        assert(term != nullptr);
        assert(term_ == nullptr); /* Must be used for initialization only. */
        
        term_ = term;
    }

    /**
     * \return Valid pointer to the type of this expression.
     */
    virtual const Type *getType() const = 0;

    /**
     * \return True, if the expression is a constant integer zero.
     */
    virtual bool isZero() const { return false; }

    Expression *rewrite() override { return this; }

    /**
     * \return Precedence of operator:
     *  zero for non-operators,
     *  positive values for left-to-right associative operators,
     *  negative values for right-to-left associative operators.
     *
     * \see http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
     */
    virtual int precedence() const { return 0; }
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::TreeNode, nc::core::likec::Expression, nc::core::likec::TreeNode::EXPRESSION)

/* vim:set et sts=4 sw=4: */

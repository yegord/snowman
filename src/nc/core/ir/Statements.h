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

#include <memory>
#include <vector>

#include <QString>

#include "Statement.h"
#include "Term.h"

namespace nc {
namespace core {
namespace ir {

class InlineAssembly: public Statement {
public:
    InlineAssembly(): Statement(INLINE_ASSEMBLY) {}

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

/**
 * Assignment statement.
 */
class Assignment: public Statement {
    std::unique_ptr<Term> left_; ///< Term being the left-hand side.
    std::unique_ptr<Term> right_; ///< Term being the right-hand side.

public:
    /**
     * \param[in] left  Valid pointer to the term being the left-hand side.
     * \param[in] right Valid pointer to the term being the right-hand side.
     *
     * Arguments have to have the same size. Use UnaryOperator for size conversions.
     */
    Assignment(std::unique_ptr<Term> left, std::unique_ptr<Term> right);

    /**
     * \return Valid pointer to the term being the left-hand side.
     */
    Term *left() { return left_.get(); }

    /**
     * \return Valid pointer to the term being the left-hand side.
     */
    const Term *left() const { return left_.get(); }

    /**
     * \return Valid pointer to the term being the right-hand side.
     */
    Term *right() { return right_.get(); }

    /**
     * \return Valid pointer to the term being the right-hand side.
     */
    const Term *right() const { return right_.get(); }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

/**
 * A statement reading, writing, or killing a term.
 */
class Touch: public Statement {
    std::unique_ptr<Term> term_; ///< Term being accessed.
    Term::AccessType accessType_;

public:
    /**
     * \param[in] term Valid pointer to the term being accessed.
     * \param[in] accessType Type of term's use.
     */
    Touch(std::unique_ptr<Term> term, Term::AccessType accessType);

    /**
     * \return Valid pointer to the term being accessed.
     */
    Term *term() const { return term_.get(); }

    /**
     * \return Type of the term's use.
     */
    Term::AccessType accessType() const { return accessType_; }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

/**
 * Function call statement.
 */
class Call: public Statement {
    std::unique_ptr<Term> target_; ///< Call target (function address).

public:
    /**
     * \param[in] target    Call target (function address).
     */
    explicit
    Call(std::unique_ptr<Term> target);

    /**
     * \return Call target (function address).
     */
    Term *target() { return target_.get(); }

    /**
     * \return Call target (function address).
     */
    const Term *target() const { return target_.get(); }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

/**
 * Return from a program.
 */
class Halt: public Statement {
public:
    Halt(): Statement(HALT) {}

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

/**
 * DataflowAnalyzer, when executing the statement, calls
 * the function stored in the statement.
 */
class Callback: public Statement {
    std::function<void()> function_; ///< Callback function.

public:
    /**
     * \param function Callback function.
     */
    explicit
    Callback(std::function<void()> function):
        Statement(CALLBACK), function_(std::move(function))
    {
        assert(function_);
    }

    /**
     * \return Callback function.
     */
    const std::function<void()> &function() const { return function_; }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

/**
 * DataflowAnalyzer remembers definitions reaching this statement.
 */
class RememberReachingDefinitions: public Statement {
public:
    RememberReachingDefinitions(): Statement(REMEMBER_REACHING_DEFINITIONS) {}

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

/*
 * Statement implementation follows.
 */

const Assignment *Statement::asAssignment() const {
    return as<Assignment>();
}

const Touch *Statement::asTouch() const {
    return as<Touch>();
}

const Call *Statement::asCall() const {
    return as<Call>();
}

const Callback *Statement::asCallback() const {
    return as<Callback>();
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

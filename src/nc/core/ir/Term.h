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

#include <boost/noncopyable.hpp>

#include <nc/common/Printable.h>
#include <nc/common/Subclass.h>
#include <nc/common/Types.h>

#include "MemoryLocation.h"

namespace nc {
namespace core {
namespace ir {

class Constant;
class Intrinsic;
class BinaryOperator;
class Choice;
class Dereference;
class MemoryLocationAccess;
class UnaryOperator;

class Statement;

/**
 * Base class for different kinds of expressions of intermediate representation.
 */
class Term: public Printable, boost::noncopyable {
    NC_BASE_CLASS(Term, kind)

public:
    /**
     * Term kind.
     */
    enum {
        INT_CONST, ///< Integer constant.
        INTRINSIC, ///< Some special value.
        MEMORY_LOCATION_ACCESS, ///< Access to an abstract memory location.
        DEREFERENCE, ///< Dereference.
        UNARY_OPERATOR, ///< Unary operator.
        BINARY_OPERATOR, ///< Binary operator.
        CHOICE, ///< Choice between two terms.
        USER = 1000 ///< Base for user-defined terms.
    };

    /**
     * How term is used.
     */
    enum AccessType {
        READ,      ///< Term is read.
        WRITE,     ///< Term is written.
        KILL       ///< Term is killed.
    };

private:
    const Statement *statement_; ///< Statement that this term belongs to.
    SmallBitSize size_; ///< Size of this term's value in bits.

public:
    /**
     * Class constructor.
     *
     * \param[in] kind Kind of this term.
     * \param[in] size Size of this term's value in bits.
     */
    Term(int kind, SmallBitSize size):
        kind_(kind), statement_(nullptr), size_(size)
    {
        assert(size != 0);
    }

    /**
     * \returns Size of this term's value in bits.
     */
    SmallBitSize size() const { return size_; }

    /**
     * \return Pointer to the statement this term belongs to. Can be nullptr.
     */
    const Statement *statement() const { return statement_; }

    /**
     * Sets the statement this term and its children belong to.
     *
     * \param[in] statement Valid pointer to the statement.
     *
     * \note Must be called only once for each term.
     */
    void setStatement(const Statement *statement);

    /**
     * \return Term's access type.
     */
    AccessType accessType() const;

    /**
     * \return True if term is used for reading, false otherwise.
     */
    bool isRead() const { return accessType() == READ; }

    /**
     * \return True if term is used for writing, false otherwise.
     */
    bool isWrite() const { return accessType() == WRITE; }

    /**
     * \return True if term is used for killing, false otherwise.
     */
    bool isKill() const { return accessType() == KILL; }

    /**
     * \return If the term stands in the left hand side of an assignment,
     *         returns the right hand size of this assignment. Otherwise,
     *         nullptr is returned.
     */
    const Term *source() const;

    /**
     * \return Clone of the term.
     */
    std::unique_ptr<Term> clone() const { return doClone(); }

    /**
     * Calls a given function on all the children of this term.
     *
     * \param fun Valid function.
     */
    void callOnChildren(const std::function<void(const Term *)> &fun) const {
        assert(fun);
        const_cast<Term *>(this)->doCallOnChildren(fun);
    }

    /**
     * Calls a given function on all the children of this term.
     *
     * \param fun Valid function.
     */
    void callOnChildren(const std::function<void(Term *)> &fun) {
        assert(fun);
        doCallOnChildren(fun);
    }

    /* The following functions are defined in Terms.h. */

    inline const Constant *asConstant() const;
    inline const Intrinsic *asIntrinsic() const;
    inline const MemoryLocationAccess *asMemoryLocationAccess() const;
    inline const Dereference *asDereference() const;
    inline const UnaryOperator *asUnaryOperator() const;
    inline const BinaryOperator *asBinaryOperator() const;
    inline const Choice *asChoice() const;

protected:
    /**
     * \return Valid pointer to the clone of this term.
     */
    virtual std::unique_ptr<Term> doClone() const = 0;

    /**
     * Calls a given function on all the children of this term.
     *
     * \param fun Valid function.
     */
    virtual void doCallOnChildren(const std::function<void(Term *)> &fun) = 0;
};

}}} // namespace nc::core::ir

/**
 * Defines a compile-time mapping from term class to term kind.
 * Makes it possible to use the given class as an argument to <tt>Term::as</tt>
 * and <tt>Term::is</tt> template functions.
 *
 * Must be used at global namespace.
 *
 * \param CLASS Term class.
 * \param KIND  Term kind.
 */
#define NC_REGISTER_TERM_CLASS(CLASS, KIND)                                     \
    NC_SUBCLASS(nc::core::ir::Term, CLASS, KIND)

NC_REGISTER_TERM_CLASS(nc::core::ir::Constant,             nc::core::ir::Term::INT_CONST)
NC_REGISTER_TERM_CLASS(nc::core::ir::Intrinsic,            nc::core::ir::Term::INTRINSIC)
NC_REGISTER_TERM_CLASS(nc::core::ir::MemoryLocationAccess, nc::core::ir::Term::MEMORY_LOCATION_ACCESS)
NC_REGISTER_TERM_CLASS(nc::core::ir::Dereference,          nc::core::ir::Term::DEREFERENCE)
NC_REGISTER_TERM_CLASS(nc::core::ir::UnaryOperator,        nc::core::ir::Term::UNARY_OPERATOR)
NC_REGISTER_TERM_CLASS(nc::core::ir::BinaryOperator,       nc::core::ir::Term::BINARY_OPERATOR)
NC_REGISTER_TERM_CLASS(nc::core::ir::Choice,               nc::core::ir::Term::CHOICE)

/* vim:set et sts=4 sw=4: */

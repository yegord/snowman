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

#include <nc/common/SizedValue.h>

#include "Term.h"

namespace nc {
namespace core {
namespace ir {

/**
 * Constant integer.
 */
class Constant: public Term {
    ConstantValue value_; ///< Value of the constant.

public:
    /**
     * Class constructor.
     *
     * \param[in] value Value of the constant.
     */
    Constant(const SizedValue &value): Term(INT_CONST, value.size()), value_(value.value()) {}

    /**
     * \return Value of the constant.
     */
    SizedValue value() const { return SizedValue(size(), value_, SizedValue::exact); }

    /**
     * Sets the value of the constant.
     *
     * \param[in] value New value. It is truncated to the lower size() bits.
     */
    void setValue(ConstantValue value) { value_ = bitTruncate(value, size()); }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Term> doClone() const override;
    void doCallOnChildren(const std::function<void(Term *)> &fun) override;
};

/**
 * Term representing a special value.
 */
class Intrinsic: public Term {
    int intrinsicKind_;     ///< Kind of this intrinsic.

public:
    /**
     * Intrinsic kinds.
     */
    enum {
        UNKNOWN,            ///< Unknown intrinsic.
        UNDEFINED,          ///< Undefined value, generally not a stack offset.
        ZERO_STACK_OFFSET,  ///< Undefined value, zero stack offset.
        RETURN_ADDRESS,     ///< Return address (e.g. saved by a call instruction).
    };

    /**
     * Constructor.
     *
     * \param[in] intrinsicKind Type of this intrinsic.
     * \param[in] size          Size of this term's value in bits.
     */
    Intrinsic(int intrinsicKind, SmallBitSize size):
        Term(INTRINSIC, size), intrinsicKind_(intrinsicKind)
    {}

    /**
     * \return Kind of this intrinsic.
     */
    int intrinsicKind() const { return intrinsicKind_; }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Term> doClone() const override;
    void doCallOnChildren(const std::function<void(Term *)> &fun) override;
};

/**
 * Reference to some abstract memory location (e.g. register).
 */
class MemoryLocationAccess: public Term {
    MemoryLocation memoryLocation_; ///< Accessed memory location.

public:
    /**
     * Class constructor.
     *
     * \param[in] memoryLocation Referenced memory location.
     */
    MemoryLocationAccess(const MemoryLocation &memoryLocation);

    /**
     * \return Referenced memory location.
     */
    const MemoryLocation &memoryLocation() const { return memoryLocation_; }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Term> doClone() const override;
    void doCallOnChildren(const std::function<void(Term *)> &fun) override;
};

/**
 * Dereference of a memory address.
 */
class Dereference: public Term {
    Domain domain_; ///< Domain the address belongs to.
    std::unique_ptr<Term> address_; ///< Address to be dereferenced.

public:
    /**
     * Class constructor.
     *
     * \param[in] address              Memory address to be dereferenced.
     * \param[in] domain               Memory domain that the address belongs to.
     * \param[in] size                 Size of the value at address in bits.
     */
    Dereference(std::unique_ptr<Term> address, Domain domain, SmallBitSize size);

    /**
     * \return Domain the address belongs to.
     */
    Domain domain() const { return domain_; }

    /**
     * \return Address to be dereferenced.
     */
    Term *address() { return address_.get(); }

    /**
     * \return Address to be dereferenced.
     */
    const Term *address() const { return address_.get(); }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Term> doClone() const override;
    void doCallOnChildren(const std::function<void(Term *)> &fun) override;
};

/**
 * Unary operator.
 */
class UnaryOperator: public Term {
public:
    /**
     * Unary operator kinds.
     */
    enum OperatorKind {
        NOT, ///< Bitwise NOT.
        NEGATION, ///< Integer negation.
        SIGN_EXTEND, ///< Sign extend.
        ZERO_EXTEND, ///< Zero extend.
        TRUNCATE, ///< Truncate.
    };

private:
    int operatorKind_; ///< Operator kind.
    std::unique_ptr<Term> operand_; ///< Operand.

public:
    /**
     * Class constructor.
     *
     * \param[in] operatorKind  Kind of the unary operator.
     * \param[in] operand       The operand of the unary operator.
     * \param[in] size          Size of this term's value in bits.
     *
     * \note If operator's kind is NOT or NEGATION, operator's size must be equal to operand's size.
     * \note If operator's kind is *_EXTEND, operator's size must be strictly greater than operand's size.
     * \note If operator's kind is TRUNCATE, operator's size must be strictly smaller than operand's size.
     */
    UnaryOperator(int operatorKind, std::unique_ptr<Term> operand, SmallBitSize size);

    /**
     * \return Kind of the operator.
     */
    int operatorKind() const { return operatorKind_; }

    /**
     * \return Valid pointer to the operand of this operator.
     */
    Term *operand() { return operand_.get(); }

    /**
     * \return Valid pointer to the operand of this operator.
     */
    const Term *operand() const { return operand_.get(); }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Term> doClone() const override;
    void doCallOnChildren(const std::function<void(Term *)> &fun) override;
};

/**
 * Binary operator.
 */
class BinaryOperator: public Term {
public:
    /**
     * Binary operator kinds.
     */
    enum OperatorKind {
        AND, ///< Bitwise AND.
        OR,  ///< Bitwise OR.
        XOR, ///< Bitwise XOR.
        SHL, ///< Bit shift left.
        SHR, ///< Bit shift right.
        SAR, ///< Arithmetic bit shift right.
        ADD, ///< Integer addition.
        SUB, ///< Integer subtraction.
        MUL, ///< Integer multiplication.
        SIGNED_DIV, ///< Signed integer division.
        SIGNED_REM, ///< Signed integer remainder.
        UNSIGNED_DIV, ///< Unsigned integer division.
        UNSIGNED_REM, ///< Unsigned integer remainder.
        EQUAL, ///< Equality.
        SIGNED_LESS, ///< Integer signed less.
        SIGNED_LESS_OR_EQUAL, ///< Integer signed less or equal.
        UNSIGNED_LESS, ///< Integer unsigned less.
        UNSIGNED_LESS_OR_EQUAL, ///< Integer unsigned less or equal.
    };

private:
    int operatorKind_; ///< Operator kind.
    std::unique_ptr<Term> left_; ///< Left operand.
    std::unique_ptr<Term> right_; ///< Right operand.

public:
    /**
     * Class constructor.
     *
     * \param[in] operatorKind  Kind of the binary operator.
     * \param[in] left          Left operand.
     * \param[in] right         Right operand.
     * \param[in] size          Size of this term's value in bits.
     *
     * \note If operator's kind is AND, OR, XOR, ADD, SUB, MUL, *_DIV, *_REM,
     *       then operator's size and operands' sizes must be equal.
     * \note If operator's kind is SHL, SHR, SAR,
     *       then operator's size must be equal to left operand's size.
     * \note If operator's kind is EQUAL, *_LESS, *_LESS_OR_EQUAL,
     *       then operand's sizes must be equal, and operator's size must be 1.
     */
    BinaryOperator(int operatorKind, std::unique_ptr<Term> left, std::unique_ptr<Term> right, SmallBitSize size);

    /**
     * \return Kind of the operator.
     */
    int operatorKind() const { return operatorKind_; }

    /**
     * \return Valid pointer to the left operand of this operator.
     */
    Term *left() { return left_.get(); }

    /**
     * \return Valid pointer to the left operand of this operator.
     */
    const Term *left() const { return left_.get(); }

    /**
     * \return Valid pointer to the right operand of this operator.
     */
    Term *right() { return right_.get(); }

    /**
     * \return Valid pointer to the right operand of this operator.
     */
    const Term *right() const { return right_.get(); }

    void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Term> doClone() const override;
    void doCallOnChildren(const std::function<void(Term *)> &fun) override;
};

/*
 * Term implementation follows.
 */

const Constant *Term::asConstant() const { return as<Constant>(); }
const Intrinsic *Term::asIntrinsic() const { return as<Intrinsic>(); }
const MemoryLocationAccess *Term::asMemoryLocationAccess() const { return as<MemoryLocationAccess>(); }
const Dereference *Term::asDereference() const { return as<Dereference>(); }
const UnaryOperator *Term::asUnaryOperator() const { return as<UnaryOperator>(); }
const BinaryOperator *Term::asBinaryOperator() const { return as<BinaryOperator>(); }

}}} // namespace nc::core::ir

/* vim:set et sts=4 sw=4: */

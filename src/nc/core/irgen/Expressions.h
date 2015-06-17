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

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */

#include <algorithm> /* std::max */
#include <type_traits>

#include <boost/mpl/int.hpp>

#include <nc/common/Unused.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Register.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/Statements.h>

#include "InvalidInstructionException.h"

namespace nc { namespace core { namespace irgen { namespace expressions {

// -------------------------------------------------------------------------- //
// Statements
// -------------------------------------------------------------------------- //
/**
 * Base class for all instruction analyzer statements.
 */
template<class Derived>
class StatementBase {
public:
    Derived &derived() { return static_cast<Derived &>(*this); }
    const Derived &derived() const { return static_cast<const Derived &>(*this); }
};

template<class T>
struct IsStatement: public std::is_base_of<StatementBase<T>, T> {};

/**
 * Base class for unary statements.
 */
template<class E, class Derived>
class UnaryStatementBase: public StatementBase<Derived> {
public:
    explicit
    UnaryStatementBase(E expression):
        mExpression(std::move(expression))
    {}

    E &expression() { return mExpression; }
    const E &expression() const { return mExpression; }

private:
    E mExpression;
};

/**
 * Base class for binary statements.
 */
template<class L, class R, class Derived>
class BinaryStatementBase: public StatementBase<Derived> {
public:
    BinaryStatementBase(L left, R right):
        mLeft(std::move(left)),
        mRight(std::move(right))
    {}

    L &left() { return mLeft; }
    const L &left() const { return mLeft; }

    R &right() { return mRight; }
    const R &right() const { return mRight; }

private:
    L mLeft;
    R mRight;
};

/**
 * Class for halt statements.
 */
class HaltStatement: public StatementBase<HaltStatement> {};

/**
 * Class for kill statements.
 */
template<class E>
class KillStatement: public UnaryStatementBase<E, KillStatement<E>> {
    typedef UnaryStatementBase<E, KillStatement<E>> base_type;
public:
    explicit
    KillStatement(E expression): base_type(std::move(expression)) {}
};

/**
 * Class for jump statements.
 */
template<class C, class A>
class JumpStatement: public StatementBase<JumpStatement<C, A>> {
    typedef StatementBase<JumpStatement<C, A>> base_type;

public:
    JumpStatement(C condition, A address, ir::BasicBlock *thenBasicBlock, ir::BasicBlock *elseBasicBlock):
        mCondition(std::move(condition)), mAddress(std::move(address)), mThenBasicBlock(thenBasicBlock), mElseBasicBlock(elseBasicBlock)
    {}

    C &condition() { return mCondition; }
    const C &condition() const { return mCondition; }

    A &address() { return mAddress; }
    const A &address() const { return mAddress; }

    ir::BasicBlock *thenBasicBlock() const { return mThenBasicBlock; }
    ir::BasicBlock *elseBasicBlock() const { return mElseBasicBlock; }

private:
    C mCondition;
    A mAddress;
    ir::BasicBlock *mThenBasicBlock;
    ir::BasicBlock *mElseBasicBlock;
};

/**
 * Class for call statements.
 */
template<class E>
class CallStatement: public UnaryStatementBase<E, CallStatement<E>> {
    typedef UnaryStatementBase<E, CallStatement<E>> base_type;
public:
    explicit
    CallStatement(E expression): base_type(std::move(expression)) {}
};

/**
 * Class for assignment statements.
 */
template<class L, class R>
class AssignmentStatement: public BinaryStatementBase<L, R, AssignmentStatement<L, R>> {
    typedef BinaryStatementBase<L, R, AssignmentStatement<L, R>> base_type;
public:
    AssignmentStatement(L left, R right): base_type(std::move(left), std::move(right)) {}
};

/**
 * Class for sequential statements.
 */
template<class L, class R>
class SequenceStatement: public BinaryStatementBase<L, R, SequenceStatement<L, R>> {
    typedef BinaryStatementBase<L, R, SequenceStatement<L, R>> base_type;
public:
    SequenceStatement(L left, R right): base_type(std::move(left), std::move(right)) {}
};


// -------------------------------------------------------------------------- //
// Expressions
// -------------------------------------------------------------------------- //
/**
 * Base class for all instruction analyzer expressions.
 */
template<class Derived>
class ExpressionBase {
public:
    typedef Derived derived_type;

    explicit
    ExpressionBase(SmallBitSize size = 0): mSize(size) {}

    SmallBitSize size() const { return mSize; }

    void setSize(SmallBitSize size) { assert(!mSize); mSize = size; }

    Derived &derived() {
        return static_cast<Derived &>(*this);
    }

    const Derived &derived() const {
        return static_cast<const Derived &>(*this);
    }

private:
    SmallBitSize mSize;
};

template<class T>
struct IsExpression: public std::is_base_of<ExpressionBase<T>, T> {};

/**
 * Base class for unary expressions.
 */
template<class E, class Derived>
class UnaryExpressionBase: public ExpressionBase<Derived> {
    typedef ExpressionBase<Derived> base_type;
public:
    explicit
    UnaryExpressionBase(E operand, SmallBitSize size = 0):
        base_type(size),
        mOperand(std::move(operand))
    {}

    E &operand() { return mOperand; }
    const E &operand() const { return mOperand; }

private:
    E mOperand;
};

/**
 * Base class for binary expressions.
 */
template<class L, class R, class Derived>
class BinaryExpressionBase: public ExpressionBase<Derived> {
    typedef ExpressionBase<Derived> base_type;
public:
    BinaryExpressionBase(L left, R right, SmallBitSize size = 0):
        base_type(size),
        mLeft(std::move(left)),
        mRight(std::move(right))
    {}

    L &left() { return mLeft; }
    const L &left() const { return mLeft; }

    R &right() { return mRight; }
    const R &right() const { return mRight; }

private:
    L mLeft;
    R mRight;
};


/**
 * Class for instrinsic expressions.
 */
class IntrinsicExpression: public ExpressionBase<IntrinsicExpression> {
    typedef ExpressionBase<IntrinsicExpression> base_type;
};

/**
 * Class for undefined expressions.
 */
class UndefinedExpression: public ExpressionBase<UndefinedExpression> {
    typedef ExpressionBase<UndefinedExpression> base_type;
};

/**
 * Class for constant expressions.
 */
class ConstantExpression: public ExpressionBase<ConstantExpression> {
    typedef ExpressionBase<ConstantExpression> base_type;
public:
    explicit
    ConstantExpression(ConstantValue value, SmallBitSize size = 0): base_type(size), mValue(value) {}

    ConstantValue value() const { return mValue; }

private:
    ConstantValue mValue;
};

/**
 * Class for instruction operand expressions.
 */
class TermExpression: public ExpressionBase<TermExpression> {
    typedef ExpressionBase<TermExpression> base_type;
public:
    explicit
    TermExpression(std::unique_ptr<ir::Term> term): base_type(term->size()), mTerm(std::move(term)) {}

    TermExpression(TermExpression &&that):
        base_type(std::move(that)),
        mTerm(std::move(that.mTerm))
    {}

    TermExpression(const TermExpression &that):
        base_type(that),
        mTerm(that.mTerm->clone())
    {}

    TermExpression &operator=(TermExpression &&that) {
        base_type::operator=(std::move(that));
        mTerm = std::move(that.mTerm);
        return *this;
    }

    TermExpression &operator=(const TermExpression &that) {
        base_type::operator=(that);
        mTerm = that.mTerm->clone();
        return *this;
    }

    std::unique_ptr<ir::Term> &term() { return mTerm; }

    const std::unique_ptr<ir::Term> &term() const { return mTerm; }

private:
    std::unique_ptr<ir::Term> mTerm;
};

/**
 * Class for memory location expressions.
 */
class MemoryLocationExpression: public ExpressionBase<MemoryLocationExpression> {
    typedef ExpressionBase<MemoryLocationExpression> base_type;
public:
    MemoryLocationExpression(const ir::MemoryLocation &memoryLocation):
        base_type(memoryLocation.size()),
        mMemoryLocation(memoryLocation)
    {}

    MemoryLocationExpression(const MemoryLocationExpression &that):
        base_type(that),
        mMemoryLocation(that.memoryLocation())
    {}

    const ir::MemoryLocation &memoryLocation() const { return mMemoryLocation; }
private:
    ir::MemoryLocation mMemoryLocation;
};

/**
 * Class for unary expressions.
 */
template<int operatorKind, class E>
class UnaryExpression: public UnaryExpressionBase<E, UnaryExpression<operatorKind, E> > {
    typedef UnaryExpressionBase<E, UnaryExpression<operatorKind, E> > base_type;
public:
    UnaryExpression(E expression, SmallBitSize size = 0): base_type(std::move(expression), size) {}
};

/**
 * Class for dereference expressions.
 */
template<class E>
class DereferenceExpression: public UnaryExpressionBase<E, DereferenceExpression<E> > {
    typedef UnaryExpressionBase<E, DereferenceExpression<E> > base_type;
public:
    DereferenceExpression(E expression, SmallBitSize size = 0): base_type(std::move(expression), size) {}
};

/**
 * Class for binary expressions.
 */
template<int operatorKind, class L, class R>
class BinaryExpression: public BinaryExpressionBase<L, R, BinaryExpression<operatorKind, L, R> > {
    typedef BinaryExpressionBase<L, R, BinaryExpression<operatorKind, L, R> > base_type;
public:
    BinaryExpression(L left, R right, SmallBitSize size = 0): base_type(std::move(left), std::move(right), size) {}
};

/**
 * Class for choice expressions.
 */
template<class L, class R>
class ChoiceExpression: public BinaryExpressionBase<L, R, ChoiceExpression<L, R> > {
    typedef BinaryExpressionBase<L, R, ChoiceExpression<L, R> > base_type;
public:
    ChoiceExpression(L left, R right): base_type(std::move(left), std::move(right)) {}
};

/**
 * Class for sign expressions.
 *
 * These expressions have no direct counterpart in IR. Instead, they are used
 * to mark arithmetic operations as signed or unsigned.
 *
 * \see nc::core::expressions::ExpressionSignedness
 */
template<int signedness, class E>
class SignExpression: public UnaryExpressionBase<E, SignExpression<signedness, E> > {
    typedef UnaryExpressionBase<E, SignExpression<signedness, E> > base_type;
public:
    explicit SignExpression(E expression): base_type(std::move(expression)) {}
};

/**
 * Class for no expression.
 */
class NullExpression: public ExpressionBase<NullExpression> {};

// -------------------------------------------------------------------------- //
// Expression signedness
// -------------------------------------------------------------------------- //
class ExpressionSignedness {
public:
    enum {
        SIGNED,
        UNSIGNED,
        UNKNOWN
    };
};

/**
 * Metafunction that returns expression's signedness.
 */
template<class Expression>
struct expression_signedness: boost::mpl::int_<ExpressionSignedness::UNKNOWN> {};

template<class Derived>
struct expression_signedness<ExpressionBase<Derived>>: expression_signedness<Derived> {};

template<int signedness, class E>
struct expression_signedness<SignExpression<signedness, E>>: boost::mpl::int_<signedness> {};

/**
 * Metafuction that returns binary expression operatorKind based on signedness.
 */
template<int unsignedOperatorKind, int signedness>
struct binary_operator_kind;

template<int unsignedOperatorKind>
struct binary_operator_kind<unsignedOperatorKind, ExpressionSignedness::UNSIGNED>: boost::mpl::int_<unsignedOperatorKind> {};

template<>
struct binary_operator_kind<ir::BinaryOperator::UNSIGNED_DIV, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_DIV> {};

template<>
struct binary_operator_kind<ir::BinaryOperator::UNSIGNED_REM, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_REM> {};

template<>
struct binary_operator_kind<ir::BinaryOperator::UNSIGNED_LESS, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_LESS> {};

template<>
struct binary_operator_kind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SIGNED_LESS_OR_EQUAL> {};

template<>
struct binary_operator_kind<ir::BinaryOperator::SHR, ExpressionSignedness::SIGNED>: boost::mpl::int_<ir::BinaryOperator::SAR> {};

/**
 * Metafunction that returns binary expression operatorKind based on signedness of the given expression.
 */
template<int unsignedOperatorKind, class E>
struct binary_expression_operator_kind: public binary_operator_kind<unsignedOperatorKind, expression_signedness<E>::value> {};


// -------------------------------------------------------------------------- //
// Shortcuts
// -------------------------------------------------------------------------- //
inline
IntrinsicExpression
intrinsic() {
    return IntrinsicExpression();
}

inline
UndefinedExpression
undefined() {
    return UndefinedExpression();
}

inline
ConstantExpression
constant(ConstantValue value, SmallBitSize size = 0) {
    return ConstantExpression(value, size);
}

inline
TermExpression
term(std::unique_ptr<ir::Term> term) {
    return TermExpression(std::move(term));
}

inline
MemoryLocationExpression
regizter(const arch::Register *reg) {
    return MemoryLocationExpression(reg->memoryLocation());
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    SignExpression<ExpressionSignedness::SIGNED, E>
>::type
signed_(E expression) {
    return SignExpression<ExpressionSignedness::SIGNED, E>(std::move(expression));
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    SignExpression<ExpressionSignedness::UNSIGNED, E>
>::type
unsigned_(E expression) {
    return SignExpression<ExpressionSignedness::UNSIGNED, E>(std::move(expression));
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
        DereferenceExpression<E>
>::type
operator*(E expression) {
    return DereferenceExpression<E>(std::move(expression));
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    DereferenceExpression<E>
>::type
dereference(E expression, SmallBitSize size) {
    DereferenceExpression<E> result(std::move(expression));
    result.setSize(size);
    return result;
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    UnaryExpression<ir::UnaryOperator::NOT, E>
>::type
operator~(E expression) {
    return UnaryExpression<ir::UnaryOperator::NOT, E>(std::move(expression));
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    UnaryExpression<ir::UnaryOperator::NEGATION,  E>
>::type
operator-(E expression) {
    return UnaryExpression<ir::UnaryOperator::NEGATION, E>(std::move(expression));
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    UnaryExpression<ir::UnaryOperator::SIGN_EXTEND, E>
>::type
sign_extend(E expression, BitSize size = 0) {
    return UnaryExpression<ir::UnaryOperator::SIGN_EXTEND, E>(std::move(expression), size);
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    UnaryExpression<ir::UnaryOperator::ZERO_EXTEND, E>
>::type
zero_extend(E expression, BitSize size = 0) {
    return UnaryExpression<ir::UnaryOperator::ZERO_EXTEND, E>(std::move(expression), size);
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    UnaryExpression<ir::UnaryOperator::TRUNCATE, E>
>::type
truncate(E expression, SmallBitSize size = 0) {
    return UnaryExpression<ir::UnaryOperator::TRUNCATE, E>(std::move(expression), size);
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::ADD, L, R>
>::type
operator+(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::ADD, L, R>(std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::SUB, L, R>
>::type
operator-(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::SUB, L, R>(std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::MUL, L, R>
>::type
operator*(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::MUL, L, R>(std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_DIV, L>::value, L, R>
>::type
operator/(L left, R right) {
    return BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_DIV, L>::value, L, R>(
        std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_REM, L>::value, L, R>
>::type
operator%(L left, R right) {
    return BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_REM, L>::value, L, R>(
        std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::AND, L, R>
>::type
operator&(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::AND, L, R>(std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::OR, L, R>
>::type
operator|(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::OR, L, R>(std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::XOR, L, R>
>::type
operator^(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::XOR, L, R>(std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::SHL, L, R>
>::type
operator<<(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::SHL, L, R>(std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::SHR, L>::value, L, R>
>::type
operator>>(L left, R right) {
    return BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::SHR, L>::value, L, R>(
        std::move(left), std::move(right));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<ir::BinaryOperator::EQUAL, L, R>
>::type
operator==(L left, R right) {
    return BinaryExpression<ir::BinaryOperator::EQUAL, L, R>(std::move(left), std::move(right), 1);
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS, L>::value, L, R>
>::type
operator<(L left, R right) {
    return BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS, L>::value, L, R>(
        std::move(left), std::move(right), 1);
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, L>::value, L, R>
>::type
operator<=(L left, R right) {
    return BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, L>::value, L, R>(
        std::move(left), std::move(right), 1);
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS, L>::value, R, L>
>::type
operator>(L left, R right) {
    return BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS, L>::value, R, L>(
        std::move(right), std::move(left), 1);
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, L>::value, R, L>
>::type
operator>=(L left, R right) {
    return BinaryExpression<binary_expression_operator_kind<ir::BinaryOperator::UNSIGNED_LESS_OR_EQUAL, L>::value, R, L>(
        std::move(right), std::move(left), 1);
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    ChoiceExpression<L, R>
>::type
choice(L first, R second) {
    return ChoiceExpression<L, R>(std::move(first), std::move(second));
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    AssignmentStatement<L, R>
>::type
operator^=(L left, R right) {
    return AssignmentStatement<L, R>(std::move(left), std::move(right));
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    KillStatement<E>
>::type
kill(E expression) {
    return KillStatement<E>(std::move(expression));
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    JumpStatement<NullExpression, E>
>::type
jump(E thenAddress) {
    return JumpStatement<NullExpression, E>(NullExpression(), std::move(thenAddress), nullptr, nullptr);
}

inline
JumpStatement<NullExpression, NullExpression>
jump(ir::BasicBlock *targetBasicBlock) {
    return JumpStatement<NullExpression, NullExpression>(NullExpression(), NullExpression(), targetBasicBlock, nullptr);
}

template<class L, class R>
inline
typename std::enable_if<
    IsExpression<L>::value && IsExpression<R>::value,
    JumpStatement<L, R>
>::type
jump(L condition, R thenAddress, ir::BasicBlock *elseTarget) {
    return JumpStatement<L, R>(std::move(condition), std::move(thenAddress), nullptr, elseTarget);
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    JumpStatement<E, NullExpression>
>::type
jump(E condition, ir::BasicBlock *thenBasicBlock, ir::BasicBlock *elseTarget) {
    return JumpStatement<E, NullExpression>(std::move(condition), NullExpression(), thenBasicBlock, elseTarget);
}

template<class E>
inline
typename std::enable_if<
    IsExpression<E>::value,
    CallStatement<E>
>::type
call(E target) {
    return CallStatement<E>(std::move(target));
}

inline
HaltStatement
halt() {
    return HaltStatement();
}

template<class L, class R>
inline
typename std::enable_if<
    IsStatement<L>::value && IsStatement<R>::value,
    SequenceStatement<L, R>
>::type
operator,(L first, R second) {
    return SequenceStatement<L, R>(std::move(first), std::move(second));
}


// -------------------------------------------------------------------------- //
// ExpressionFactory
// -------------------------------------------------------------------------- //
/**
 * Factory class that converts instruction analyzer expressions into terms and
 * statements.
 */
template<class Derived>
class ExpressionFactory {
    Q_DECLARE_TR_FUNCTIONS(ExpressionFactory)
public:
    explicit
    ExpressionFactory(const arch::Architecture *architecture):
        mArchitecture(architecture)
    {}

    const arch::Architecture *architecture() const {
        return mArchitecture;
    }

    /**
     * \param expression               Expression to create term from.
     * \returns                        Newly created term for the given expression,
     *                                 with expression size determined automatically.
     */
    template<class E>
    std::unique_ptr<ir::Term> createTerm(ExpressionBase<E> &expression) const {
        auto result(derived().doCreateTerm(expression.derived()));

        if (result && result->size() != expression.size()) {
            throw InvalidInstructionException(tr("Term %1 created from expression of size %2 has completely different size %3")
                .arg(result->toString()).arg(expression.size()).arg(result->size()));
        }

        return result;
    }

    /**
     * \param statement                Statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class E>
    std::unique_ptr<ir::Statement> createStatement(StatementBase<E> &statement) const {
        return derived().doCreateStatement(statement.derived());
    }

    /**
     * \param expression               Expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class E>
    void computeSize(ExpressionBase<E> &expression, SmallBitSize suggestedSize) const {
        derived().doComputeSize(expression.derived(), suggestedSize);
    }

protected:
    /* Overload-based dispatch functions follow.
     *
     * They can be overridden and extended in derived class thanks to CRTP
     * (http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern). */

    /**
     * \param expression               Intrinsic expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(IntrinsicExpression &expression) const {
        NC_UNUSED(expression);

        if (!expression.size()) {
            throw InvalidInstructionException(tr("Size of the intrinsic expression is unknown"));
        }

        return std::make_unique<ir::Intrinsic>(ir::Intrinsic::UNKNOWN, expression.size());
    }

    /**
     * \param expression               Undefined expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(UndefinedExpression &expression) const {
        NC_UNUSED(expression);

        if (!expression.size()) {
            throw InvalidInstructionException(tr("Size of the undefined expression is unknown"));
        }

        return std::make_unique<ir::Intrinsic>(ir::Intrinsic::UNDEFINED, expression.size());
    }

    /**
     * \param expression               Constant expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(ConstantExpression &expression) const {
        if (!expression.size()) {
            throw InvalidInstructionException(tr("Size of the constant expression is unknown"));
        }

        return std::make_unique<ir::Constant>(SizedValue(expression.size(), expression.value()));
    }

    /**
     * \param expression               Instruction operand expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(TermExpression &expression) const {
        return std::move(expression.term());
    }

    /**
     * \param expression               Memory location expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(MemoryLocationExpression &expression) const {
        return std::make_unique<ir::MemoryLocationAccess>(expression.memoryLocation());
    }

    /**
     * \param expression               Unary expression.
     * \returns                        Newly created term for the given expression.
     */
    template<int operatorKind, class E>
    std::unique_ptr<ir::Term> doCreateTerm(UnaryExpression<operatorKind, E> &expression) const {
        if (!expression.size()) {
            throw InvalidInstructionException(tr("Size of the unary expression is unknown"));
        }

        return std::make_unique<ir::UnaryOperator>(operatorKind, createTerm(expression.operand()), expression.size());
    }

    /**
     * \param expression               Dereference expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<class E>
    std::unique_ptr<ir::Term> doCreateTerm(DereferenceExpression<E> &expression) const {
        if (!expression.size()) {
            throw InvalidInstructionException(tr("Size of the dereference expression is unknown"));
        }

        return std::make_unique<ir::Dereference>(createTerm(expression.operand()), ir::MemoryDomain::MEMORY, expression.size());
    }

    /**
     * \param expression               Dereference expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<int signedness, class E>
    std::unique_ptr<ir::Term> doCreateTerm(SignExpression<signedness, E> &expression) const {
        return createTerm(expression.operand());
    }

    /**
     * \param expression               Binary expression that change size to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<int operatorKind, class L, class R>
    std::unique_ptr<ir::Term> doCreateTerm(BinaryExpression<operatorKind, L, R> &expression) const {
        return std::make_unique<ir::BinaryOperator>(operatorKind, createTerm(expression.left()), createTerm(expression.right()), expression.size());
    }

    /**
     * \param expression               Choice expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    template<class L, class R>
    std::unique_ptr<ir::Term> doCreateTerm(ChoiceExpression<L, R> &expression) const {
        return std::make_unique<ir::Choice>(createTerm(expression.left()), createTerm(expression.right()));
    }

    /**
     * \param expression               Null expression to create term from.
     * \returns                        Newly created term for the given expression.
     */
    std::unique_ptr<ir::Term> doCreateTerm(const NullExpression &expression) const {
        NC_UNUSED(expression);

        return std::unique_ptr<ir::Term>();
    }

    /**
     * \param statement                Halt statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    std::unique_ptr<ir::Statement> doCreateStatement(HaltStatement &statement) const {
        NC_UNUSED(statement);

        return std::make_unique<ir::Halt>();
    }

    /**
     * \param statement                Kill statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class E>
    std::unique_ptr<ir::Statement> doCreateStatement(KillStatement<E> &statement) const {
        computeSize(statement.expression(), 0);

        return std::make_unique<ir::Touch>(createTerm(statement.expression()), ir::Term::KILL);
    }

    /**
     * \param statement                Jump statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class C, class A>
    std::unique_ptr<ir::Statement> doCreateStatement(JumpStatement<C, A> &statement) const {
        computeSize(statement.condition(), 1);
        computeSize(statement.address(), mArchitecture->bitness());

        auto condition = createTerm(statement.condition());
        auto address = createTerm(statement.address());

        ir::JumpTarget thenTarget;
        if (address) {
            thenTarget.setAddress(std::move(address));
        } else {
            thenTarget.setBasicBlock(statement.thenBasicBlock());
        }

        ir::JumpTarget elseTarget;
        if (statement.elseBasicBlock()) {
            elseTarget.setBasicBlock(statement.elseBasicBlock());
        }

        if (condition) {
            return std::make_unique<ir::Jump>(std::move(condition), std::move(thenTarget), std::move(elseTarget));
        } else {
            return std::make_unique<ir::Jump>(std::move(thenTarget));
        }
    }

    /**
     * \param statement                Call statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class E>
    std::unique_ptr<ir::Statement> doCreateStatement(CallStatement<E> &statement) const {
        computeSize(statement.expression(), mArchitecture->bitness());

        return std::make_unique<ir::Call>(createTerm(statement.expression()));
    }

    /**
     * \param statement                Assignment statement to create IR statement from.
     * \returns                        Newly created ir statement for the given statement.
     */
    template<class L, class R>
    std::unique_ptr<ir::Statement> doCreateStatement(AssignmentStatement<L, R> &statement) const {
        computeSize(statement.left(), 0);
        computeSize(statement.right(), 0);

        if (!statement.left().size() && statement.right().size()) {
            computeSize(statement.left(), statement.right().size());
        } else if (statement.left().size() && !statement.right().size()) {
            computeSize(statement.right(), statement.left().size());
        }

        if (statement.left().size() != statement.right().size()) {
            throw core::irgen::InvalidInstructionException(tr("Cannot assign expressions of different sizes: %1 and %2")
                .arg(statement.left().size()).arg(statement.right().size()));
        }

        return std::make_unique<ir::Assignment>(createTerm(statement.left()), createTerm(statement.right()));
    }

    /**
     * A 'catch-all' implementation of the size calculation. This is the function
     * that will be called if no suitable overload is found.
     *
     * \param expression               Expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class E>
    void doComputeSize(ExpressionBase<E> &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(expression);

        if (!expression.size() && suggestedSize) {
            expression.setSize(suggestedSize);
        }
    }

    /**
     * \param expression               Operand expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    void doComputeSize(TermExpression &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(suggestedSize);

        if (!expression.size()) {
            expression.setSize(expression.term()->size());
        }
    }

    /**
     * \param expression               Memory location expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    void doComputeSize(MemoryLocationExpression &expression, SmallBitSize suggestedSize) const {
        NC_UNUSED(suggestedSize);

        /* Initialized in the constructor. */
        assert(expression.size());
        assert(expression.size() == expression.memoryLocation().size());
    }

    /**
     * \param expression               Unary expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<int operatorKind, class E>
    void doComputeSize(UnaryExpression<operatorKind, E> &expression, SmallBitSize suggestedSize) const {
        if (expression.size()) {
            computeSize(expression.operand(), expression.size());
        } else {
            computeSize(expression.operand(), suggestedSize);
            switch (operatorKind) {
                case ir::UnaryOperator::SIGN_EXTEND:
                case ir::UnaryOperator::ZERO_EXTEND:
                case ir::UnaryOperator::TRUNCATE:
                    if (suggestedSize) {
                        expression.setSize(suggestedSize);
                    }
                    break;
                default:
                    if (expression.operand().size()) {
                        expression.setSize(expression.operand().size());
                    }
                    break;
            }
        }
    }

    /**
     * \param expression               Dereference.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class E>
    void doComputeSize(DereferenceExpression<E> &expression, SmallBitSize suggestedSize) const {
        computeSize(expression.operand(), suggestedSize);

        if (!expression.size() && suggestedSize) {
            expression.setSize(suggestedSize);
        }
    }

    /**
     * \param expression               Sign expression.
     * \param suggestedSize            Suggested size of the expression.
     */
    template<int signedness, class E>
    void doComputeSize(SignExpression<signedness, E> &expression, SmallBitSize suggestedSize) const {
        if (expression.size()) {
            computeSize(expression.operand(), expression.size());
        } else {
            computeSize(expression.operand(), suggestedSize);
            expression.setSize(expression.operand().size());
        }
    }

    /**
     * \param expression               Binary expression (Binary operation or Choice).
     * \param suggestedSize            Suggested size of the expression.
     */
    template<class L, class R, class D>
    void doComputeSize(BinaryExpressionBase<L, R, D> &expression, SmallBitSize suggestedSize) const {
        computeSize(expression.left(), 0);
        computeSize(expression.right(), 0);

        if (!expression.left().size() || !expression.right().size()) {
            /* If at least one of the operands knows its size by itself, take it as suggested. */
            if (expression.left().size() || expression.right().size()) {
                suggestedSize = std::max(expression.left().size(), expression.right().size());
            }

            /* If there are still no ideas, use my own size. */
            if (!suggestedSize) {
                suggestedSize = expression.size();
            }

            if (suggestedSize) {
                if (!expression.left().size()) {
                    computeSize(expression.left(), suggestedSize);
                }
                if (!expression.right().size()) {
                    computeSize(expression.right(), suggestedSize);
                }
            }
        }

        if (!expression.size()) {
            expression.setSize(std::max(expression.left().size(), expression.right().size()));
        }
    }

private:
    const Derived &derived() const {
        return static_cast<const Derived &>(*this);
    }

    const arch::Architecture *mArchitecture;
};


// -------------------------------------------------------------------------- //
// ExpressionFactoryCallback
// -------------------------------------------------------------------------- //
/**
 * Convenience class that handles sequential statements by adding them to
 * the given basic block.
 */
template<class ExpressionFactory>
class ExpressionFactoryCallback {
public:
    /**
     * Constructor.
     *
     * \param factory       Expression factory to use to create separate statements.
     * \param basicBlock    Basic block to add statements to.
     * \param instruction   Instruction to set to the created statements.
     */
    ExpressionFactoryCallback(ExpressionFactory &factory, ir::BasicBlock *basicBlock, const arch::Instruction *instruction):
        mFactory(factory),
        mBasicBlock(basicBlock),
        mInstruction(instruction)
    {
        assert(basicBlock != nullptr);
    }

    /**
     * \return Basic block to which statements are added.
     */
    ir::BasicBlock *basicBlock() const { return mBasicBlock; }

    /**
     * \param statement                 Call statement to create IR statement from.
     *                                  Created statement will be added to the basic block.
     */
    template<class E>
    void operator[](StatementBase<E> &&statement) const {
        doCallback(statement.derived());
    }

    void operator()(std::unique_ptr<ir::Statement> statement) const {
        statement->setInstruction(mInstruction);
        mBasicBlock->pushBack(std::move(statement));
    }

protected:
    /* Overload-based dispatch functions follow. */

    template<class E>
    void doCallback(StatementBase<E> &statement) const {
        this->operator()(mFactory.createStatement(statement));
    }

    template<class L, class R>
    void doCallback(SequenceStatement<L, R> &statement) const {
        this->operator[](std::move(statement.left()));
        this->operator[](std::move(statement.right()));
    }

private:
    ExpressionFactory &mFactory;
    ir::BasicBlock *mBasicBlock;
    const arch::Instruction *mInstruction;
};


}}}} // namespace nc::core::irgen::expressions

#define NC_DEFINE_REGISTER_EXPRESSION(registers, lowercase) \
const auto lowercase = core::irgen::expressions::regizter(registers::lowercase());

/* vim:set et sts=4 sw=4: */

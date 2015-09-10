/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace likec {

class Expression;

/**
 * \param dividend A valid pointer to an expression.
 * \param divisor An integer.
 *
 * \return Valid pointer to the expression divided by the integer,
 *         in case the expression is a multiple of the divisor,
 *         nullptr otherwise.
 */
std::unique_ptr<Expression> divide(Expression *dividend, SignedConstantValue divisor);

/**
 * \param expression Valid pointer to an expression.
 *
 * \return True iff the expression is constant zero.
 */
bool isZero(const Expression *expression);

/**
 * \param expression Valid pointer to an expression.
 *
 * \return True iff the expression is constant one.
 */
bool isOne(const Expression *expression);

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

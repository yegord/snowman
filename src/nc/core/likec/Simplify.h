/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

namespace nc {
namespace core {
namespace likec {

class Expression;

/**
 * \param expression Valid pointer to an expression node.
 *
 * \return Valid pointer to the simplified expression that has the same value
 *         as the original one.
 */
std::unique_ptr<Expression> simplify(std::unique_ptr<Expression> expression);

/**
 * \param expression Valid pointer to an expression node.
 *
 * \return Valid pointer to the simplified expression that has the same
 *         boolean value as the original one.
 */
std::unique_ptr<Expression> simplifyBooleanExpression(std::unique_ptr<Expression> expression);

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

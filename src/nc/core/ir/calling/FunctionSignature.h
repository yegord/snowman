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

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace calling {

/**
 * Signature of a function: arguments, return value, name.
 */
class FunctionSignature {
    std::vector<std::shared_ptr<const Term>> arguments_; ///< Terms representing the arguments.
    bool variadic_; ///< True if the function is variadic.
    std::shared_ptr<const Term> returnValue_; ///< Term representing the return value.

public:
    /**
     * Constructs an empty signature.
     */
    FunctionSignature(): variadic_(false) {}

    /**
     * \return List of terms representing function's arguments.
     */
    std::vector<std::shared_ptr<const Term>> &arguments() { return arguments_; }

    /**
     * \return List of terms representing function's arguments.
     */
    const std::vector<std::shared_ptr<const Term>> &arguments() const { return arguments_; }

    /**
     * \return True if the function is variadic, false otherwise.
     */
    bool variadic() const { return variadic_; }

    /**
     * Sets whether the function is variadic.
     *
     * \param value Flag whether the function is variadic.
     */
    void setVariadic(bool value = true) { variadic_ = value; }

    /**
     * \return Pointer to the term containing the return value. Can be nullptr.
     */
    const std::shared_ptr<const Term> &returnValue() const { return returnValue_; }

    /**
     * Sets the pointer to the term representing the return value.
     *
     * \param term Valid pointer to the term.
     */
    void setReturnValue(std::shared_ptr<Term> term) { returnValue_ = std::move(term); }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

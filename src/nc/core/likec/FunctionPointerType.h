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

#include <vector>

#include "Type.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Function type.
 */
class FunctionPointerType: public Type {
    const Type *returnType_; ///< Function return type.
    std::vector<const Type *> argumentTypes_; ///< Types of function arguments.
    bool variadic_; ///< True, if function has variable number of arguments.

public:
    /**
     * Class constructor.
     *
     * \param[in] size Type size (in case of function pointer, usually nobody cares).
     * \param[in] returnType Function return type.
     * \param[in] variadic Whether function has variable number of arguments.
     */
    FunctionPointerType(BitSize size, const Type *returnType = nullptr, bool variadic = false):
        Type(size, FUNCTION_POINTER), returnType_(returnType), variadic_(variadic)
    {}

    /**
     * \return Function return type.
     */
    const Type *returnType() const { return returnType_; }

    /**
     * Sets function return type.
     *
     * \param[in] type Type.
     */
    void setReturnType(const Type *type) { returnType_ = type; }

    /**
     * \return Types of function arguments.
     */
    const std::vector<const Type *> &argumentTypes() const { return argumentTypes_; }

    /**
     * Adds argument to the function.
     *
     * \param[in] argumentType Type of added argument.
     */
    void addArgumentType(const Type *argumentType) { argumentTypes_.push_back(argumentType); }

    /**
     * \return True, if function has variable number of arguments.
     */
    bool variadic() const { return variadic_; }

    /**
     * Sets whether function has variable number of arguments.
     *
     * \param[in] variadic Whether function has variable number of arguments.
     */
    void setVariadic(bool variadic) { variadic_ = variadic; }

    virtual void print(QTextStream &out) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Type, nc::core::likec::FunctionPointerType, nc::core::likec::Type::FUNCTION_POINTER)

/* vim:set et sts=4 sw=4: */

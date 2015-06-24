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

#include "Type.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Erroneous type.
 */
class ErroneousType: public Type {
    public:

    /**
     * Class constructor.
     */
    ErroneousType(): Type(-1, ERRONEOUS) {}

    virtual void print(QTextStream &out) const override;
};

/**
 * Void type.
 */
class VoidType: public Type {
    public:

    /**
     * Class constructor.
     */
    VoidType(): Type(0, VOID) {}

    virtual bool isVoid() const override { return true; }

    virtual void print(QTextStream &out) const override;
};

/**
 * Integer type (signed or unsigned).
 */
class IntegerType: public Type {
    bool isUnsigned_; ///< Whether this type is unsigned.

    public:

    /**
     * Class constructor.
     *
     * \param[in] size Type size.
     * \param[in] isUnsigned Whether this type is unsigned.
     */
    IntegerType(BitSize size, bool isUnsigned): Type(size, INTEGER), isUnsigned_(isUnsigned) {}

    /**
     * \return True, if the type is signed.
     */
    bool isSigned() const { return !isUnsigned_; }

    /**
     * \return True, if the type is unsigned.
     */
    bool isUnsigned() const { return isUnsigned_; }

    virtual bool isInteger() const override { return true; }
    virtual bool isArithmetic() const override { return true; }
    virtual bool isScalar() const override { return true; }

    virtual void print(QTextStream &out) const override;
};

/**
 * Floating point type.
 */
class FloatType: public Type {
    public:

    /**
     * Class constructor.
     *
     * \param[in] size Type size.
     */
    FloatType(BitSize size): Type(size, FLOAT) {}

    virtual bool isInteger() const override { return true; }
    virtual bool isArithmetic() const override { return true; }

    virtual void print(QTextStream &out) const override;
};

/**
 * Pointer type.
 */
class PointerType: public Type {
    NC_BASE_CLASS(Type, pointerKind)

    const Type *pointeeType_; ///< Type this one points to.

    public:

    /**
     * Pointer kind.
     */
    enum {
        PLAIN_PTR,      ///< Usual pointer.
        ARRAY_PTR,      ///< Array type.
        USER_PTR = 1000 ///< Base for user-defined pointer types.
    };

    /**
     * Class constructor.
     *
     * \param[in] size Type size.
     * \param[in] pointeeType Pointee type.
     */
    PointerType(BitSize size, const Type *pointeeType):
        Type(size, POINTER), pointerKind_(PLAIN_PTR), pointeeType_(pointeeType)
    {
        assert(pointeeType);
    }

    protected:

    /**
     * Class constructor.
     *
     * \param[in] size Type size.
     * \param[in] pointeeType Pointee type.
     * \param[in] pointerKind Pointer kind.
     */
    PointerType(BitSize size, const Type *pointeeType, int pointerKind):
        Type(size, POINTER), pointerKind_(pointerKind), pointeeType_(pointeeType)
    {
        assert(pointeeType);
    }

    public:

    /**
     * \return Type this one points to.
     */
    const Type *pointeeType() const { return pointeeType_; }

    virtual bool isPointer() const override { return true; }
    virtual bool isVoidPointer() const override { return pointeeType()->isVoid(); }
    virtual bool isStructurePointer() const override { return pointeeType()->isStructure(); }
    virtual bool isScalar() const override { return true; }

    virtual void print(QTextStream &out) const override;
};

/**
 * Array type = pointer type + number of elements.
 */
class ArrayType: public PointerType {
    std::size_t length_; ///< Number of elements in the array.
    
    public:

    /**
     * Class constructor.
     *
     * \param[in] size Type size.
     * \param[in] elementType Type of elements.
     * \param[in] length Length of the array.
     */
    ArrayType(BitSize size, const Type *elementType, std::size_t length):
        PointerType(size, elementType, ARRAY_PTR), length_(length) {}

    /**
     * \return Type of elements.
     */
    const Type *elementType() const { return pointeeType(); }

    /**
     * \return Number of elements in the array.
     */
    std::size_t length() const { return length_; }

    /**
     * Sets the number of elements in the array.
     *
     * \param length Array size.
     */
    void setLength(std::size_t length) { length_ = length; }

    virtual BitSize sizeOf() const override { return elementType()->size() * length(); }

    virtual void print(QTextStream &out) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Type, nc::core::likec::ErroneousType, nc::core::likec::Type::ERRONEOUS)
NC_SUBCLASS(nc::core::likec::Type, nc::core::likec::VoidType,      nc::core::likec::Type::VOID)
NC_SUBCLASS(nc::core::likec::Type, nc::core::likec::IntegerType,   nc::core::likec::Type::INTEGER)
NC_SUBCLASS(nc::core::likec::Type, nc::core::likec::FloatType,     nc::core::likec::Type::FLOAT)
NC_SUBCLASS(nc::core::likec::Type, nc::core::likec::PointerType,   nc::core::likec::Type::POINTER)

NC_SUBCLASS(nc::core::likec::PointerType, nc::core::likec::PointerType, nc::core::likec::PointerType::PLAIN_PTR)
NC_SUBCLASS(nc::core::likec::PointerType, nc::core::likec::ArrayType,   nc::core::likec::PointerType::ARRAY_PTR)

/* vim:set et sts=4 sw=4: */

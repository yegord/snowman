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

#include <nc/common/Printable.h>
#include <nc/common/Subclass.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace likec {

/**
 * Base class for LikeC types.
 *
 * Type objects are usually created and owned by Tree (notable exception is FunctionDeclaration which creates and owns FunctionPointerType).
 */
class Type: public Printable {
    NC_BASE_CLASS(Type, kind)

    BitSize size_; ///< Type size in bits.

    public:

    /**
     * Type kind.
     */
    enum {
        ERRONEOUS,          ///< Erroneous type.
        FLOAT,              ///< Floating point type.
        FUNCTION_POINTER,   ///< Function pointer type.
        INTEGER,            ///< Integer type.
        POINTER,            ///< Pointer type.
        STRUCT_TYPE,        ///< Structural type.
        VOID,               ///< Void type.
        USER_TYPE = 1000    ///< Base for user-defined types.
    };

    /**
     * Class constructor.
     *
     * \param[in] size Type size in bits.
     * \param[in] kind Type kind.
     */
    Type(BitSize size, int kind): kind_(kind), size_(size) {}

    /**
     * Virtual destructor.
     */
    virtual ~Type() {}

    /**
     * \return Type size in bits.
     */
    BitSize size() const { return size_; }

    /**
     * \return sizeof(variable of this type) in bits.
     */
    virtual BitSize sizeOf() const { return size(); }

    /**
     * Sets type size.
     *
     * \param[in] size Type size in bits.
     */
    void setSize(SmallBitSize size) { size_ = size; }

    /**
     * \return True, if the type is void.
     */
    virtual bool isVoid() const { return false; }

    /**
     * \return True, if the type is integer.
     */
    virtual bool isInteger() const { return false; }

    /**
     * \return True, if the type is float.
     */
    virtual bool isFloat() const { return false; }

    /**
     * \return True, if the type is integer of float.
     */
    virtual bool isArithmetic() const { return isInteger() || isFloat(); }

    /**
     * \return True, if the type is pointer.
     */
    virtual bool isPointer() const { return false; }

    /**
     * \return True, if the type is integer or pointer.
     */
    virtual bool isScalar() const { return isInteger() || isPointer(); }

    /**
     * \return True if the type is structure.
     */
    virtual bool isStructure() const { return false; }

    /**
     * \return True, if the type if void pointer.
     */
    virtual bool isVoidPointer() const { return false; }

    /**
     * \return True if the type is a pointer to a struct.
     */
    virtual bool isStructurePointer() const { return false; }
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

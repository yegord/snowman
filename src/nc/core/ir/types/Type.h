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

#ifdef NC_STRUCT_RECOVERY
#include <map>
#endif

#include <nc/common/DisjointSet.h>
#include <nc/common/Printable.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace ir {
namespace types {

class Type;

/**
 * Information about a type of a term.
 */
class Type: public DisjointSet<Type>, public PrintableBase<Type> {
    SmallBitSize size_; ///< Type size in bits.

    bool isInteger_; ///< Type is integer.
    bool isFloat_; ///< Type is float.
    bool isPointer_; ///< Type is pointer.

    Type *pointee_; ///< Type this one is a pointer to.

    bool isSigned_; ///< Type is signed.
    bool isUnsigned_; ///< Type is unsigned.

    ConstantValue factor_; ///< GCD of all increments or decrements of variables of this type.

#ifdef NC_STRUCT_RECOVERY
    std::map<ByteSize, Type *> offsets_; ///< Type traits of byte offsets to values of this type.
#endif

    bool changed_; ///< Type properties have changed since last call to changed().

    public:

    /**
     * Class constructor.
     */
    Type():
        size_(0),
        isInteger_(false), isFloat_(false), isPointer_(false), pointee_(0),
        isSigned_(false), isUnsigned_(false), factor_(0), changed_(false)
    { 
#ifdef NC_STRUCT_RECOVERY
        addOffset(0, this); 
#endif
    }

    /**
     * \return Type size in bits.
     */
    SmallBitSize size() const { return size_; }

    /**
     * Sets type size.
     *
     * \param[in] size New type size in bits.
     */
    void updateSize(SmallBitSize size);

    /**
     * \return True, if type is integer.
     */
    bool isInteger() const { return isInteger_; }

    /**
     * Marks this type as integer.
     */
    void makeInteger();

    /**
     * \return True, if type if floating-point.
     */
    bool isFloat() const { return isFloat_; }

    /**
     * Marks this type as float.
     */
    void makeFloat();

    /**
     * \return True, if type is pointer.
     */
    bool isPointer() const { return isPointer_; }

    /**
     * Marks this type as pointer.
     *
     * \param[in] pointee A type this one is a pointer to.
     */
    void makePointer(Type *pointee = 0);

    /**
     * \return A type this one is a pointer to.
     */
    Type *pointee() { return pointee_ ? pointee_->findSet() : 0; }

    /**
     * \return Type this type is pointer to.
     */
    const Type *pointee() const { return pointee_ ? pointee_->findSet() : 0; }

    /**
     * \return True, if type is signed.
     */
    bool isSigned() const { return isSigned_; }

    /**
     * Marks this type as signed.
     */
    void makeSigned();

    /**
     * \return True, if type is unsigned.
     */
    bool isUnsigned() const { return isUnsigned_; }

    /**
     * Marks this type as unsigned.
     */
    void makeUnsigned();

    /**
     * \return GCD of all increments or decrements of variables of this type.
     */
    ConstantValue factor() const { return factor_; }

    /**
     * Recomputes "factor" of this type, given another 
     *
     * \param[in] increment Increment of a variable of this type.
     */
    void updateFactor(ConstantValue increment);

#ifdef NC_STRUCT_RECOVERY
    /**
     * \return Type traits of offsets to values of this type.
     */
    const std::map<ByteSize, Type *> &offsets() const { return offsets_; };

    /**
     * Adds information about type traits of value which is an offset to a value of this type.
     *
     * \param[in] offset Offset in bytes.
     * \param[in] typeTraits Type traits of this offset.
     */
    void addOffset(ByteSize offset, Type *typeTraits);
#endif

    /**
     * \return True, if type properties have changed since last call to this function.
     */
    bool changed();

    /**
     * Merges this and that types together.
     *
     * \param[in] that Type traits to merge with.
     */
    void unionSet(Type *that);

    /**
     * Copies all information from that type to this type.
     *
     * \param[in] that Type traits to copy information from.
     */
    void join(Type *that);

    /**
     * Prints some information about this type into a stream.
     *
     * \param out Output stream.
     */
    void print(QTextStream &out) const;
};

} // namespace types
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

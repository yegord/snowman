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

#include <climits>
#include <map>
#include <memory>
#include <vector>

#include <boost/noncopyable.hpp>

#include <nc/common/PrintCallback.h>

#include "CompilationUnit.h"
#include "Types.h"

namespace nc {
namespace core {
namespace likec {

/**
 * Abstract syntax tree of high-level program in a C-like language.
 */
class Tree: boost::noncopyable {
    std::unique_ptr<CompilationUnit> root_; ///< Tree root node.

    SmallBitSize intSize_; ///< Size of int in bits for target platform.
    SmallBitSize pointerSize_; ///< Size of void * in bits for target platform.
    SmallBitSize ptrdiffSize_; ///< Size of ptrdiff_t in bits for target platform.

    const VoidType voidType_; ///< Void type.
    std::vector<std::unique_ptr<IntegerType> > integerTypes_; ///< Integer types.
    std::vector<std::unique_ptr<FloatType> > floatTypes_; ///< Float types.
    std::multimap<const Type *, std::unique_ptr<PointerType> > pointerTypes_; ///< Pointers to other types.
    std::multimap<const Type *, std::unique_ptr<ArrayType> > arrayTypes_; ///< Arrays of other types.
    const ErroneousType erroneousType_; ///< Erroneous type.

public:
    /**
     * Class constructor.
     */
    Tree(): intSize_(sizeof(int) * CHAR_BIT), pointerSize_(sizeof(void *) * CHAR_BIT), ptrdiffSize_(sizeof(ptrdiff_t) * CHAR_BIT) {}

    /**
     * \return Size of int in bits for target platform.
     */
    SmallBitSize intSize() const { return intSize_; }

    /**
     * Sets size of int in bits for target platform.
     *
     * \param[in] intSize Size.
     */
    void setIntSize(SmallBitSize intSize) { intSize_ = intSize; }

    /**
     * \return Size of void * in bits for target platform.
     */
    SmallBitSize pointerSize() const { return pointerSize_; }

    /**
     * Sets size of void * in bits for target platform.
     *
     * \param[in] pointerSize Size.
     */
    void setPointerSize(SmallBitSize pointerSize) { pointerSize_ = pointerSize; }

    /**
     * \return Size of ptrdiff_t in bits for target platform.
     */
    SmallBitSize ptrdiffSize() const { return ptrdiffSize_; }

    /**
     * Sets size of ptrdiff_t in bits for target platform.
     *
     * \param[in] ptrdiffSize Size of ptrdiff_t in bits.
     */
    void setPtrdiffSize(SmallBitSize ptrdiffSize) { ptrdiffSize_ = ptrdiffSize; }

    /**
     * \return Tree root node.
     */
    CompilationUnit *root() { return root_.get(); }

    /**
     * \return Tree root node.
     */
    const CompilationUnit *root() const { return root_.get(); }

    /**
     * Sets tree root node.
     *
     * \param node Tree node.
     */
    void setRoot(std::unique_ptr<CompilationUnit> node) { root_ = std::move(node); }

    /**
     * Rewrites the whole tree.
     *
     * \see TreeNode::rewrite()
     */
    void rewriteRoot();

    /**
     * Prints the whole tree into a stream.
     *
     * \param[in] out Output stream.
     * \param[in] callback Print callback.
     */
    void print(QTextStream &out, PrintCallback<const TreeNode *> *callback = 0) const;

    /**
     * \return Void type.
     */
    const VoidType *makeVoidType();

    /**
     * \return Integer type of given size and signedness.
     *
     * \param[in] size Size.
     * \param[in] isUnsigned Type must unsigned.
     */
    const IntegerType *makeIntegerType(SmallBitSize size, bool isUnsigned);

    /**
     * \return Float type of given size.
     *
     * \param[in] size Size.
     */
    const FloatType *makeFloatType(SmallBitSize size);

    /**
     * \return Pointer type of given size pointing to given type.
     *
     * \param[in] size Size.
     * \param[in] pointee Pointee type.
     */
    const PointerType *makePointerType(SmallBitSize size, const Type *pointee);

    /**
     * \return Pointer type to given type. Usual pointer size is used.
     *
     * \param[in] pointee Pointee type.
     */
    const PointerType *makePointerType(const Type *pointee) {
        return makePointerType(pointerSize(), pointee);
    }

    /**
     * \return Array type with required properties.
     *
     * \param[in] size Type size.
     * \param[in] elementType Element type.
     * \param[in] length Array length.
     */
    const ArrayType *makeArrayType(SmallBitSize size, const Type *elementType, std::size_t length);

    /**
     * \return Array type with size equal to usual pointer size.
     *
     * \param[in] elementType Element type.
     * \param[in] length Array length.
     */
    const ArrayType *makeArrayType(const Type *elementType, std::size_t length) {
        return makeArrayType(pointerSize(), elementType, length);
    }

    /**
     * \return Erroneous type.
     */
    const ErroneousType *makeErroneousType();

    /**
     * \param[in] integerType Integer type.
     *
     * \return Type which is "integer promotion" of given type.
     */
    const IntegerType *integerPromotion(const IntegerType *integerType);

    /**
     * \param[in] type Type.
     *
     * \return Type which is "integer promotion" of given type.
     */
    const Type *integerPromotion(const Type *type);

    /**
     * \param[in] leftType One type.
     * \param[in] rightType Another type.
     *
     * \return Type which is the result of "usual arithmetic conversion" of both types.
     */
    const Type *usualArithmeticConversion(const Type *leftType, const Type *rightType);
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

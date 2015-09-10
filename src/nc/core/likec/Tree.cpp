/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "Tree.h"

#include <nc/common/Foreach.h>

#include "PrintContext.h"
#include "Simplifier.h"
#include "Types.h"

namespace nc {
namespace core {
namespace likec {

void Tree::rewriteRoot() {
    if (root_) {
        root_ = Simplifier(*this).simplify(std::move(root_));
    }
}

void Tree::print(QTextStream &out, PrintCallback<const TreeNode *> *callback) const {
    PrintContext context(out, callback);

    root()->print(context);
}

const VoidType *Tree::makeVoidType() {
    return &voidType_;
}

const IntegerType *Tree::makeIntegerType(SmallBitSize size, bool isUnsigned) {
    foreach (const auto &type, integerTypes_) {
        if (type->size() == size && type->isUnsigned() == isUnsigned) {
            return type.get();
        }
    }

    IntegerType *type = new IntegerType(size, isUnsigned);
    integerTypes_.push_back(std::unique_ptr<IntegerType>(type));

    return type;
}

const FloatType *Tree::makeFloatType(SmallBitSize size) {
    foreach (const auto &type, floatTypes_) {
        if (type->size() == size) {
            return type.get();
        }
    }

    FloatType *type = new FloatType(size);
    floatTypes_.push_back(std::unique_ptr<FloatType>(type));

    return type;
}

const PointerType *Tree::makePointerType(SmallBitSize size, const Type *pointee) {
    auto range = pointerTypes_.equal_range(pointee);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second->size() == size) {
            return i->second.get();
        }
    }

    PointerType *type = new PointerType(size, pointee);
    pointerTypes_.insert(std::make_pair(pointee, std::unique_ptr<PointerType>(type)));
    return type;
}

const ArrayType *Tree::makeArrayType(SmallBitSize size, const Type *elementType, std::size_t length) {
    auto range = arrayTypes_.equal_range(elementType);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second->length() == length && i->second->size() == size) {
            return i->second.get();
        }
    }

    ArrayType *type = new ArrayType(size, elementType, length);
    arrayTypes_.insert(std::make_pair(elementType, std::unique_ptr<ArrayType>(type)));
    return type;
}

const ErroneousType *Tree::makeErroneousType() {
    return &erroneousType_;
}

const IntegerType *Tree::integerPromotion(const IntegerType *integerType) {
    if (integerType->size() < intSize()) {
        return makeIntegerType(intSize(), integerType->isUnsigned());
    } else {
        return integerType;
    }
}

const Type *Tree::integerPromotion(const Type *type) {
    if (const IntegerType *integerType = type->as<IntegerType>()) {
        return integerPromotion(integerType);
    } else {
        return type;
    }
}

const Type *Tree::usualArithmeticConversion(const Type *leftType, const Type *rightType) {
    leftType = integerPromotion(leftType);
    rightType = integerPromotion(rightType);

    if (leftType->isFloat() && rightType->isFloat()) {
        return makeFloatType(std::max(leftType->size(), rightType->size()));
    } else if (leftType->isFloat() && rightType->isInteger()) {
        return leftType;
    } else if (leftType->isInteger() && rightType->isFloat()) {
        return rightType;
    } else if (leftType->isInteger() && rightType->isInteger()) {
        if (leftType->size() < rightType->size()) {
            return rightType;
        } else if (leftType->size() > rightType->size()) {
            return leftType;
        } else {
            const IntegerType *leftIntegerType = leftType->as<IntegerType>();
            const IntegerType *rightIntegerType = rightType->as<IntegerType>();

            return makeIntegerType(leftType->size(), leftIntegerType->isUnsigned() || rightIntegerType->isUnsigned());
        }
    }

    return makeErroneousType();
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

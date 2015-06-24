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

#include "Type.h"

#include <cassert>

#include <QTextStream>

#include <nc/common/Foreach.h>

namespace nc {
namespace core {
namespace ir {
namespace types {

void Type::updateSize(SmallBitSize size) {
    if (size && (!size_ || size < size_)) {
        size_ = size;
        changed_ = true;
    }
}

void Type::makeInteger() {
    if (!isInteger_) {
        isInteger_ = true;
        changed_ = true;
    }
}

void Type::makeFloat() {
    if (!isFloat_) {
        isFloat_ = true;
        changed_ = true;
    }
}

void Type::makePointer(Type *pointee) {
    if (!isPointer_) {
        isPointer_ = true;
        changed_ = true;
    }

    if (pointee) {
        if (!pointee_) {
            pointee_ = pointee;
            changed_ = true;
        } else {
            pointee_->unionSet(pointee);
        }
    }
}

void Type::makeSigned() {
    if (!isSigned_) {
        isSigned_ = true;
        changed_ = true;
    }
}

void Type::makeUnsigned() {
    if (!isUnsigned_) {
        isUnsigned_ = true;
        changed_ = true;
    }
}

namespace {

template<class T>
T gcd(T a, T b) {
    if (b == 0) {
        return a;
    } else {
        return gcd(b, a % b);
    }
}

} // anonymous namespace

void Type::updateFactor(ConstantValue increment) {
    ConstantValue oldFactor = factor_;

    factor_ = gcd(increment, factor_);

    if (oldFactor != factor_) {
        changed_ = true;
    }
}

#ifdef NC_STRUCT_RECOVERY
void Type::addOffset(ByteSize offset, Type *typeTraits) {
    Type *&existingTraits = offsets_[offset];
    if (existingTraits) {
        existingTraits->unionSet(typeTraits);
    } else {
        existingTraits = typeTraits;
    }
}
#endif

bool Type::changed() {
    if (changed_) {
        changed_ = false;
        return true;
    } else {
        return false;
    }
}

void Type::unionSet(Type *that) {
    Type *thisSet = this->findSet();
    Type *thatSet = that->findSet();

    DisjointSet<Type>::unionSet(that);

    if (findSet() == thisSet) {
        thisSet->join(thatSet);
    } else {
        thatSet->join(thisSet);
    }
}

void Type::join(Type *that) {
    if (this == that) {
        return;
    }
    updateSize(that->size());
    if (that->isInteger()) {
        makeInteger();
    }
    if (that->isFloat()) {
        makeFloat();
    }
    if (that->isPointer()) {
        makePointer(that->pointee());
    }
    if (that->isSigned()) {
        makeSigned();
    }
    if (that->isUnsigned()) {
        makeUnsigned();
    }

#ifdef NC_STRUCT_RECOVERY
    foreach (auto offsetType, that->offsets_) {
        addOffset(offsetType.first, offsetType.second);
    }
#endif

    updateFactor(that->factor());
}

void Type::print(QTextStream &out) const {
    out << "<type " << this;
    if (size()) {
        out << " size=" << size();
    }
    if (isInteger()) {
        out << " int";
    }
    if (isFloat()) {
        out << " flt";
    }
    if (isPointer()) {
        out << " ptr" << pointee();
    }
    if (isSigned()) {
        out << " sgn";
    }
    if (isUnsigned()) {
        out << " unsgn";
    }
    if (factor()) {
        out << " factor=" << factor();
    }
#ifdef NC_STRUCT_RECOVERY
    if (offsets_.size() > 1) {
        out << " offsets == " << offsets_.size();
    }
#endif
    out << ">";
}

} // namespace types
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

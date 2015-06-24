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

#include "Types.h"

#include <QTextStream>

#include "StructTypeDeclaration.h"

namespace nc {
namespace core {
namespace likec {

void ErroneousType::print(QTextStream &out) const {
    out << "<erroneous type>";
}

void VoidType::print(QTextStream &out) const {
    out << "void";
}

void IntegerType::print(QTextStream &out) const {
    if (size() == 8) {
        if (isUnsigned()) {
            out << "unsigned";
        } else {
            out << "signed";
        }
        out << " char";
    } else {
        if (isUnsigned()) {
            out << 'u';
        }
        out << "int" << size() << "_t";
    }
}

void FloatType::print(QTextStream &out) const {
    out << "float" << size();
}

void PointerType::print(QTextStream &out) const {
    out << *pointeeType_ << '*';
}

void ArrayType::print(QTextStream &out) const {
    out << *elementType() << '[' << length() << ']';
}

void StructType::print(QTextStream &out) const {
    out << "struct " << typeDeclaration_->identifier();
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

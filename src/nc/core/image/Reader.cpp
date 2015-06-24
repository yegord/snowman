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

#include "Reader.h"

#include <QString>

namespace nc {
namespace core {
namespace image {

ByteSize Reader::readBytes(ByteAddr addr, void *buf, ByteSize size) const {
    return externalByteSource_->readBytes(addr, buf, size);
}

QString Reader::readAsciizString(ByteAddr addr, ByteSize maxSize) const {
    assert(maxSize >= 0);

    if (maxSize == 0) {
        return QString();
    }

    std::unique_ptr<char[]> buf(new char[maxSize + 1]);

    ByteSize size = readBytes(addr, buf.get(), maxSize);
    assert(size <= maxSize);

    if (size == 0) {
        return QString();
    } else {
        buf.get()[size] = '\0';
        return QString::fromLatin1(buf.get());
    }
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

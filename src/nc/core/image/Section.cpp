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

#include "Section.h"

namespace nc {
namespace core {
namespace image {

Section::Section(const QString &name, ByteAddr addr, ByteSize size):
    name_(name), addr_(addr), size_(size),
    isAllocated_(false),
    isReadable_(false), isWritable_(false), isExecutable_(false),
    isCode_(false), isData_(false), isBss_(false)
{}

ByteSize Section::readBytes(ByteAddr addr, void *buf, ByteSize size) const {
    auto offset = addr - addr_;

    if (offset < 0 || offset >= size_) {
        return 0;
    }

    size = std::min(size, size_ - offset);

    if (externalByteSource()) {
        return externalByteSource()->readBytes(addr, buf, size);
    } else {
        auto copiedSize = std::min(size, content_.size() - offset);
        if (copiedSize > 0) {
            memcpy(buf, content_.constData() + offset, copiedSize);
        }

        auto zeroedSize = std::min(size, offset + size - content_.size());
        if (zeroedSize > 0) {
            memset(static_cast<char *>(buf) + size - zeroedSize, 0, zeroedSize);
        }

        return size;
    }
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

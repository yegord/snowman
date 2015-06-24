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

#include <algorithm>
#include <cassert>
#include <cstring> /* memset */
#include <memory>

#include <boost/optional.hpp>

#include <nc/common/ByteOrder.h>

#include "ByteSource.h"

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace nc {
namespace core {
namespace image {

class Reader: public ByteSource {
    const ByteSource *externalByteSource_; ///< External byte source.

public:
    /**
     * Constructor.
     *
     * \param externalByteSource Valid pointer to the byte source to take bytes from.
     */
    explicit
    Reader(const ByteSource *externalByteSource):
        externalByteSource_(externalByteSource)
    {
        assert(externalByteSource_ != nullptr);
    }

    /**
     * Reads a sequence of bytes from the external byte source.
     *
     * \param[in] addr  Linear address of the first byte to read.
     * \param[out] buf  Valid pointer to the buffer to read into.
     * \param[in] size  Number of bytes to read.
     *
     * \return Number of bytes actually read and copied into the buffer.
     */
    ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;

    /**
     * Reads an integer value.
     *
     * \param[in] addr      Address to read from.
     * \param[in] size      Size of the integer value.
     * \param[in] byteOrder Byte order used for storing the integer value.
     *
     * \tparam T Result type.
     *
     * \return The integer value on success, boost::none on failure.
     *         If sizeof(T) < size, the lower bytes are returned.
     *         If sizeof(T) > size, the value is zero-extended.
     */
    template<class T>
    boost::optional<T> readInt(ByteAddr addr, ByteSize size, ByteOrder byteOrder) const {
        assert(size >= 0);
        assert(byteOrder != ByteOrder::Unknown);

        std::unique_ptr<char[]> buf(new char[std::max<std::size_t>(size, sizeof(T))]);

        if (readBytes(addr, buf.get(), size) != size) {
            return boost::none;
        }

        ByteOrder::convert(buf.get(), size, byteOrder, ByteOrder::LittleEndian);

        if (static_cast<std::size_t>(size) < sizeof(T)) {
            memset(buf.get() + size, 0, sizeof(T) - size);
        }

        ByteOrder::convert(buf.get(), sizeof(T), ByteOrder::LittleEndian, ByteOrder::Current);

        return *reinterpret_cast<T *>(buf.get());
    }

    /**
     * Reads an ASCIIZ string.
     *
     * \param[in] addr      Linear address of the first byte to read.
     * \param[in] maxSize   Max number of bytes to read.
     *
     * \return ASCIIZ string without zero char terminator on success, nullptr string on failure.
     */
    QString readAsciizString(ByteAddr addr, ByteSize maxSize) const;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

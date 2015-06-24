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

#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace image {

/**
 * Source of bytes of an image or its section.
 */
class ByteSource {
public:
    /**
     * Virtual destructor.
     */
    virtual ~ByteSource() {}

    /**
     * Reads a sequence of bytes.
     *
     * \param[in] addr  Linear address of the first byte to read.
     * \param[out] buf  Valid pointer to the buffer to read into.
     * \param[in] size  Number of bytes to read.
     *
     * \return Number of bytes actually read and copied into the buffer.
     */
    virtual ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const = 0;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

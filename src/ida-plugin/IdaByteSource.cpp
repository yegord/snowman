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

#include "IdaByteSource.h"

#include <nc/common/CheckedCast.h>

#include "IdaWorkaroundStart.h"
#include <bytes.hpp>
#include "IdaWorkaroundEnd.h"

namespace nc { namespace ida {

ByteSize IdaByteSource::readBytes(ByteAddr addr, void *buf, ByteSize size) const {
#ifdef NC_USE_THREADS
#error You are trying to shoot your leg. IDA API is not thread-safe.
#endif

    if (get_many_bytes(checked_cast<ea_t>(addr), buf, checked_cast<ssize_t>(size))) {
        return size;
    }

    char *charBuf = static_cast<char *>(buf);
    ByteSize i;
    for (i = 0; i < size; i++) {
        ea_t idaAddr = checked_cast<ea_t>(addr + i);

        char value = get_byte(idaAddr);
        if (value == 0) {
            flags_t flags = getFlags(idaAddr);
            if (!hasValue(flags)) {
                break;
            }
        }

        *charBuf++ = value;
    }
    return i;
}

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */

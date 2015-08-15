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

#include "Value.h"

#include <nc/common/BitTwiddling.h>
#include <nc/common/Unused.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

#ifndef NDEBUG
namespace {

bool testBitTwiddling() {
    {
        int value = 0xf;
        assert(signExtend(value, 4) == -1);
    }

    {
        int value = 0x4;
        assert(signExtend(value, 5) == 4);
    }

    {
        int value = 0x3;
        assert(bitAbs(value, 2) == 1);
    }

    {
        int value = 0x3;
        assert(bitAbs(value, 10) == 3);
    }

    {
        unsigned value = -1;
        assert(bitTruncate(value, 5) == 0x1f);
    }

    {
        unsigned value = -1;
        assert(bitTruncate(value, sizeof(value) * CHAR_BIT) == value);
    }

    return true;
}

bool bitTwiddlingWorks = (NC_UNUSED(bitTwiddlingWorks), testBitTwiddling());

} // anonymous namespace
#endif

Value::Value(SmallBitSize size):
    abstractValue_(size, 0, 0),
    isStackOffset_(false), isNotStackOffset_(false),
    isProduct_(false), isNotProduct_(false),
    isReturnAddress_(false), isNotReturnAddress_(false)
{}

void Value::makeStackOffset(SignedConstantValue offset) {
    if (isStackOffset_) {
        /*
         * If we get different values of stack pointer from different
         * branches, most likely we screwed up at detecting a stdcall
         * function or a function that never returns.
         *
         * The heuristic is to choose the bigger stack pointer value,
         * i.e. the value corresponding to the branch with less pushes.
         *
         * Note: this assumes a stack growing down.
         */
        if (stackOffset_ < offset) {
            stackOffset_ = offset;
        }
    } else {
        isStackOffset_ = true;
        stackOffset_ = offset;
    }
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

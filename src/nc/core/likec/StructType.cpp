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

#include "StructType.h"

#include <nc/common/Foreach.h>

namespace nc {
namespace core {
namespace likec {

void StructType::addMember(std::unique_ptr<MemberDeclaration> memberDeclaration) {
    setSize(size() + memberDeclaration->type()->sizeOf());
    members_.push_back(std::move(memberDeclaration));
}

const MemberDeclaration *StructType::getMember(BitSize offset) const {
    if (offset >= size()) {
        return 0;
    }

    /* If this starts lagging someday, organize binary search here. */
    SmallBitSize currentOffset = 0;
    foreach (const auto &member, members_) {
        if (currentOffset == offset) {
            return member.get();
        } else if (currentOffset > offset) {
            break;
        } else {
            currentOffset += member->type()->sizeOf();
        }
    }

    return 0;
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

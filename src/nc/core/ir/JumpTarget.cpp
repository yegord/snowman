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

#include "JumpTarget.h"

#include <cassert>

#include <QTextStream>

#include "Term.h"

namespace nc {
namespace core {
namespace ir {

JumpTarget::JumpTarget(): basicBlock_(nullptr) {}

JumpTarget::JumpTarget(std::unique_ptr<Term> address):
    address_(std::move(address)), basicBlock_(nullptr)
{
    assert(address_ != nullptr);
}

JumpTarget::JumpTarget(BasicBlock *basicBlock):
    basicBlock_(basicBlock)
{
    assert(basicBlock != nullptr);
}

JumpTarget::JumpTarget(const JumpTarget &other):
    address_(other.address_ ? other.address_->clone() : nullptr),
    basicBlock_(other.basicBlock_),
    table_(other.table_ ? new JumpTable(*other.table_) : nullptr)
{}

JumpTarget::JumpTarget(JumpTarget &&other):
    address_(std::move(other.address_)), basicBlock_(other.basicBlock_), table_(std::move(other.table_))
{}

JumpTarget::~JumpTarget() {}

void JumpTarget::setAddress(std::unique_ptr<Term> address) {
    assert(address != nullptr);
    address_ = std::move(address);
}

void JumpTarget::print(QTextStream &out) const {
    if (address()) {
        out << "address " << *address();
    } else if (basicBlock()) {
        out << "basic block " << basicBlock();
    }
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

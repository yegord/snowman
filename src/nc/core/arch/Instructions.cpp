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

#include "Instructions.h"

#include <QTextStream>

#include <nc/common/Foreach.h>

namespace nc {
namespace core {
namespace arch {

const std::shared_ptr<const Instruction> &Instructions::getCovering(ByteAddr addr) const {
    auto i = address2instruction_.lower_bound(addr);

    if (i != address2instruction_.end() &&
        i->second->addr() <= addr && addr < i->second->endAddr())
    {
        return i->second;
    } else {
        static const std::shared_ptr<const Instruction> null;
        return null;
    }
}

bool Instructions::add(std::shared_ptr<const Instruction> instruction) {
    assert(instruction != nullptr);

    auto &existing = address2instruction_[instruction->addr()];
    if (!existing) {
        existing = instruction;
        return true;
    } else {
        return false;
    }
}

bool Instructions::remove(const Instruction *instruction) {
    if (get(instruction->addr()).get() == instruction) {
        return address2instruction_.erase(instruction->addr());
    } else {
        return false;
    }
}

void Instructions::print(QTextStream &out, PrintCallback<const Instruction *> *callback) const {
    if (all().empty()) {
        return;
    }

    ByteAddr successorAddress = all().front()->addr();

    foreach (const auto &instr, all()) {
        if (instr->addr() != successorAddress) {
            out << endl;
        }
        successorAddress = instr->endAddr();

        if (callback) {
            callback->onStartPrinting(instr.get());
        }

        int integerBase = out.integerBase();
        hex(out) << instr->addr() << ":\t";
        out.setIntegerBase(integerBase);

        out << *instr;

        if (callback) {
            callback->onEndPrinting(instr.get());
        }

        out << endl;
    }
}

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

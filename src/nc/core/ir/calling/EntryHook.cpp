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

#include "EntryHook.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "Convention.h"
#include "FunctionSignature.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

EntryHook::EntryHook(const Convention *convention, const FunctionSignature *signature) {
    assert(convention != nullptr);

    auto &statements = patch_.statements();

    if (convention->stackPointer()) {
        statements.push_back(std::make_unique<Assignment>(
            std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
            std::make_unique<Intrinsic>(Intrinsic::ZERO_STACK_OFFSET, convention->stackPointer().size<SmallBitSize>())
        ));
    }

    foreach (auto statement, convention->entryStatements()) {
        statements.push_back(statement->clone());
    }

    auto createArgument = [&](const Term *term) {
        auto clone = term->clone();
        argumentTerms_[term] = clone.get();

        statements.push_back(std::make_unique<Assignment>(
            std::move(clone),
            std::make_unique<Intrinsic>(Intrinsic::UNDEFINED, term->size())
        ));
    };

    if (signature) {
        foreach (const auto &term, signature->arguments()) {
            createArgument(term.get());
        }
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */

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

#include "CallHook.h"

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/MemoryDomain.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "CallSignature.h"
#include "Convention.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

CallHook::CallHook(const Convention *convention, const CallSignature *signature,
    const boost::optional<ByteSize> &stackArgumentsSize):
    stackPointer_(nullptr), snapshotStatement_(nullptr)
{
    assert(convention != nullptr);

    auto &statements = patch_.statements();

    if (convention->stackPointer()) {
        auto stackPointer = std::make_unique<MemoryLocationAccess>(convention->stackPointer());
        stackPointer_ = stackPointer.get();

        statements.push_back(std::make_unique<Touch>(
            std::move(stackPointer),
            Term::READ
        ));
    }

    auto addArgumentRead = [&](std::unique_ptr<Term> term) {
        statements.push_back(std::make_unique<Touch>(
            std::move(term),
            Term::READ
        ));
    };

    auto addReturnValueWrite = [&](std::unique_ptr<Term> term) {
        auto size = term->size();
        statements.push_back(std::make_unique<Assignment>(
            std::move(term),
            std::make_unique<Intrinsic>(Intrinsic::UNDEFINED, size)
        ));
    };

    if (signature) {
        foreach (const auto &term, signature->arguments()) {
            auto clone = term->clone();
            argumentTerms_[term.get()] = clone.get();
            addArgumentRead(std::move(clone));
        }

        if (signature->returnValue()) {
            auto clone = signature->returnValue()->clone();
            returnValueTerms_[signature->returnValue().get()] = clone.get();
            addReturnValueWrite(std::move(clone));
        }
    } else {
        auto snapshotStatement = std::make_unique<RememberReachingDefinitions>();
        snapshotStatement_ = snapshotStatement.get();
        statements.push_back(std::move(snapshotStatement));

        foreach (const auto &memoryLocation, convention->returnValueLocations()) {
            auto term = std::make_unique<MemoryLocationAccess>(memoryLocation);
            speculativeReturnValueTerms_.push_back(std::make_pair(memoryLocation, term.get()));
            addReturnValueWrite(std::move(term));
        }
    }

    if (convention->calleeCleanup() && stackArgumentsSize) {
        assert(convention->stackPointer());

        statements.push_back(std::make_unique<Assignment>(
            std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
            std::make_unique<BinaryOperator>(
                BinaryOperator::ADD,
                std::make_unique<MemoryLocationAccess>(convention->stackPointer()),
                std::make_unique<Constant>(SizedValue(convention->stackPointer().size(), *stackArgumentsSize)),
                convention->stackPointer().size<SmallBitSize>())));
    }
}

CallHook::~CallHook() {}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */

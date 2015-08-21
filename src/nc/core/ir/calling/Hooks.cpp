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

#include "Hooks.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Utils.h>
#include <nc/core/ir/dflow/Value.h>

#include "Conventions.h"
#include "CallHook.h"
#include "Convention.h"
#include "EntryHook.h"
#include "Signatures.h"
#include "ReturnHook.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

Hooks::Hooks(const Conventions &conventions, const Signatures &signatures):
    conventions_(conventions), signatures_(signatures)
{}

Hooks::~Hooks() {}

const Convention *Hooks::getConvention(const CalleeId &calleeId) const {
    if (!calleeId) {
        return nullptr;
    }
    if (auto result = conventions_.getConvention(calleeId)) {
        return result;
    } else {
        conventionDetector_(calleeId);
        return conventions_.getConvention(calleeId);
    }
}

const EntryHook *Hooks::getEntryHook(const Function *function) const {
    assert(function != nullptr);

    return nc::find(lastEntryHooks_, function);
}

const CallHook *Hooks::getCallHook(const Call *call) const {
    assert(call != nullptr);

    return nc::find(lastCallHooks_, call);
}

const ReturnHook *Hooks::getReturnHook(const Jump *jump) const {
    assert(jump != nullptr);

    return nc::find(lastReturnHooks_, jump);
}

void Hooks::instrument(Function *function, const dflow::Dataflow *dataflow) {
    assert(function != nullptr);
    assert(dataflow != nullptr);

    deinstrument(function);

    if (function->entry()) {
        function2callback_[function] = function->entry()->pushFront(std::make_unique<Callback>([=](){
            instrumentEntry(function);
        }));
    }

    foreach (auto basicBlock, function->basicBlocks()) {
        foreach (auto statement, basicBlock->statements()) {
            if (auto call = statement->as<Call>()) {
                call2callback_[call] = basicBlock->insertAfter(call, std::make_unique<Callback>([=](){
                    instrumentCall(call, *dataflow);
                }));
            } else if (auto jump = statement->as<Jump>()) {
                jump2callback_[jump] = basicBlock->insertBefore(jump, std::make_unique<Callback>([=](){
                    if (dflow::isReturn(jump, *dataflow)) {
                        instrumentReturn(jump);
                    } else {
                        deinstrumentReturn(jump);
                    }
                }));
            }
        }
    }
}

void Hooks::deinstrument(Function *function) {
    assert(function != nullptr);

    if (auto callback = nc::find(function2callback_, function)) {
        deinstrumentEntry(function);
        callback->basicBlock()->erase(callback);
        function2callback_.erase(function);
    }

    foreach (auto basicBlock, function->basicBlocks()) {
        foreach (auto statement, basicBlock->statements()) {
            if (auto call = statement->as<Call>()) {
                if (auto callback = nc::find(call2callback_, call)) {
                    deinstrumentCall(call);
                    callback->basicBlock()->erase(callback);
                    call2callback_.erase(call);
                }
            } else if (auto jump = statement->as<Jump>()) {
                if (auto callback = nc::find(jump2callback_, jump)) {
                    deinstrumentReturn(jump);
                    callback->basicBlock()->erase(callback);
                    jump2callback_.erase(jump);
                }
            }
        }
    }
}

void Hooks::instrumentEntry(Function *function) {
    auto convention = getConvention(getCalleeId(function));
    auto signature = signatures_.getSignature(function).get();
    auto &entryHook = entryHooks_[std::make_tuple(function, convention, signature)];

    if (!entryHook) {
        entryHook = std::make_unique<EntryHook>(convention, signature);
    }

    auto &lastEntryHook = lastEntryHooks_[function];

    if (entryHook.get() != lastEntryHook) {
        if (lastEntryHook) {
            lastEntryHook->patch().remove();
        }
        if (entryHook) {
            auto callback = nc::find(function2callback_, function);
            entryHook->patch().insertAfter(callback);
        }
        lastEntryHook = entryHook.get();
    }
}

void Hooks::deinstrumentEntry(Function *function) {
    auto &lastEntryHook = lastEntryHooks_[function];

    if (lastEntryHook) {
        lastEntryHook->patch().remove();
        lastEntryHook = nullptr;
    }
}

void Hooks::instrumentCall(Call *call, const dflow::Dataflow &dataflow) {
    auto calleeId = getCalleeId(call, dataflow);
    auto convention = getConvention(calleeId);
    auto signature = signatures_.getSignature(call).get();
    auto stackArgumentsSize = conventions_.getStackArgumentsSize(calleeId);
    auto &callHook = callHooks_[std::make_tuple(call, convention, signature, stackArgumentsSize)];

    if (!callHook) {
        callHook = std::make_unique<CallHook>(convention, signature, stackArgumentsSize);
    }

    auto &lastCallHook = lastCallHooks_[call];

    if (callHook.get() != lastCallHook) {
        if (lastCallHook) {
            lastCallHook->patch().remove();
        }
        if (callHook) {
            auto callback = nc::find(call2callback_, call);
            callHook->patch().insertAfter(callback);
        }
        lastCallHook = callHook.get();
    }
}

void Hooks::deinstrumentCall(Call *call) {
    auto &lastCallHook = lastCallHooks_[call];

    if (lastCallHook) {
        lastCallHook->patch().remove();
        lastCallHook = nullptr;
    }
}

void Hooks::instrumentReturn(Jump *jump) {
    auto function = jump->basicBlock()->function();
    auto convention = getConvention(getCalleeId(function));
    auto signature = signatures_.getSignature(function).get();
    auto &returnHook = returnHooks_[std::make_tuple(jump, convention, signature)];

    if (!returnHook) {
        returnHook = std::make_unique<ReturnHook>(convention, signature);
    }

    auto &lastReturnHook = lastReturnHooks_[jump];

    if (returnHook.get() != lastReturnHook) {
        if (lastReturnHook) {
            lastReturnHook->patch().remove();
        }
        if (returnHook) {
            auto callback = nc::find(jump2callback_, jump);
            returnHook->patch().insertAfter(callback);
        }
        lastReturnHook = returnHook.get();
    }
}

void Hooks::deinstrumentReturn(Jump *jump) {
    auto &lastReturnHook = lastReturnHooks_[jump];

    if (lastReturnHook) {
        lastReturnHook->patch().remove();
        lastReturnHook = nullptr;
    }
}

CalleeId getCalleeId(const Function *function) {
    assert(function != nullptr);

    if (function->entry() && function->entry()->address()) {
        return CalleeId(EntryAddress(*function->entry()->address()));
    } else {
        return CalleeId(function);
    }
}

CalleeId getCalleeId(const Call *call, const dflow::Dataflow &dataflow) {
    assert(call != nullptr);

    auto targetValue = dataflow.getValue(call->target());
    if (targetValue->abstractValue().isConcrete()) {
        return EntryAddress(targetValue->abstractValue().asConcrete().value());
    } else if (call->instruction()) {
        return CallAddress(call->instruction()->addr());
    } else {
        return CalleeId();
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

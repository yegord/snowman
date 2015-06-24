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

#include <vector>

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>
#include <nc/common/Types.h>

#include <nc/core/ir/MemoryLocation.h>

#include "Patch.h"

namespace nc {
namespace core {
namespace ir {

class Statement;
class Term;

namespace calling {

class Convention;
class CallSignature;

/**
 * Hooks installed at a call site.
 */
class CallHook {
    /** Term for tracking stack pointer. */
    const Term *stackPointer_;

    /** Statement for snapshotting reaching definitions. */
    const Statement *snapshotStatement_;

    /** Mapping from argument terms to their clones. */
    boost::unordered_map<const Term *, const Term *> argumentTerms_;

    /** Mapping from return value terms to their clones. */
    boost::unordered_map<const Term *, const Term *> returnValueTerms_;

    /** Mapping from memory locations that can be used for returning values to terms. */
    std::vector<std::pair<MemoryLocation, const Term *>> speculativeReturnValueTerms_;

    Patch patch_;

public:
    /**
     * Class constructor.
     *
     * \param[in] convention Valid pointer to the calling convention.
     * \param[in] signature Pointer to the call's signature. Can be nullptr.
     * \param[in] stackArgumentsSize Size of arguments passed on the stack.
     */
    CallHook(const Convention *convention, const CallSignature *signature, const boost::optional<ByteSize> &stackArgumentsSize);

    /**
     * Destructor.
     */
    ~CallHook();

    /**
     * \return Patch to be inserted at the call site.
     */
    Patch &patch() { return patch_; }

    /**
     * \return Pointer to the statement used for snapshotting reaching definitions.
     *         Will be nullptr if signature was given to the constructor.
     */
    const Statement *snapshotStatement() const { return snapshotStatement_; }

    /**
     * \param term Valid pointer to a term representing the argument
     *             in the signature.
     *
     * \return Pointer to the term representing this argument in the hook.
     *         Will be nullptr, if signature does not include such an argument.
     */
    const Term *getArgumentTerm(const Term *term) const {
        assert(term != nullptr);
        return nc::find(argumentTerms_, term);
    }

    /**
     * \param term Valid pointer to a term representing the return value
     *             in the signature.
     *
     * \return Pointer to the term representing the return value in the hook.
     *         Will be nullptr, if the signature does not include such an argument.
     */
    const Term *getReturnValueTerm(const Term *term) const {
        assert(term != nullptr);
        return nc::find(returnValueTerms_, term);
    }

    /**
     * \return Pointer to the stack pointer term. Can be nullptr if the corresponding
     *         calling convention defines no stack pointer.
     */
    const Term *stackPointer() const { return stackPointer_; }

    /**
     * \return Mapping from argument terms from the signature to their clones.
     */
    const boost::unordered_map<const Term *, const Term *> &argumentTerms() const { return argumentTerms_; }

    /**
     * \return Mapping from return value terms from the signature to their clones.
     */
    const boost::unordered_map<const Term *, const Term *> &returnValueTerms() const { return returnValueTerms_; }

    /**
     * \return Mapping of the memory locations that can contain return values
     *         to terms representing writes to these locations.
     */
    const std::vector<std::pair<MemoryLocation, const Term *>> &speculativeReturnValueTerms() const { return speculativeReturnValueTerms_; }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */

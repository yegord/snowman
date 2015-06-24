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

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>
#include <nc/core/ir/MemoryLocation.h>

#include "Patch.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace calling {

class Convention;
class FunctionSignature;

/**
 * Hook installed at a return site.
 */
class ReturnHook {
    /** Mapping of terms where return values may be kept to their clones. */
    boost::unordered_map<const Term *, const Term *> returnValueTerms_;

    /** Mapping from memory locations that can be used for returning values to terms. */
    std::vector<std::pair<MemoryLocation, const Term *>> speculativeReturnValueTerms_;

    Patch patch_;

public:
    /**
     * Class constructor.
     *
     * \param[in] convention Valid pointer to the calling convention.
     * \param[in] signature Pointer to the function's signature. Can be nullptr.
     */
    ReturnHook(const Convention *convention, const FunctionSignature *signature);

    /**
     * Destructor.
     */
    ~ReturnHook();

    /**
     * \return Patch to be inserted at the return site.
     */
    Patch &patch() { return patch_; }

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
     * \return Mapping from return value terms to their clones.
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

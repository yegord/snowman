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

#include <cassert>
#include <vector>

#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace vars {

/**
 * Class containing information about a reconstructed variable.
 */
class Variable {
public:
    /**
     * Scope of a variable.
     */
    enum Scope {
        GLOBAL, ///< Global variable (declared in a compilation unit).
        LOCAL ///< Local variable (declared in a function).
    };

    /**
     * A pair of term and its location.
     */
    struct TermAndLocation {
        const Term *term;
        MemoryLocation location;

        TermAndLocation(const Term *term, const MemoryLocation &location): term(term), location(location) {}
    };

private:
    /** Scope of the variable. */
    Scope scope_;

    /** Terms belonging to the variable, together with their memory locations. */
    std::vector<TermAndLocation> termsAndLocations_;

    /** Memory location of the variable. */
    MemoryLocation memoryLocation_;

public:
    /**
     * Constructor.
     *
     * \param[in] scope Scope of the variable.
     * \param[in] termsAndLocations List of terms and their memory
     *                              locations belonging to the variable.
     * \param[in] memoryLocation Valid memory location of the variable.
     */
    Variable(Scope scope, std::vector<TermAndLocation> termsAndLocations, const MemoryLocation &memoryLocation):
        scope_(scope), termsAndLocations_(std::move(termsAndLocations)), memoryLocation_(memoryLocation)
    {
        assert(memoryLocation);
    }

    /**
     * Constructor.
     *
     * \param[in] scope Scope of the variable.
     * \param[in] termsAndLocations Non-empty list of terms and their memory
     *                              locations belonging to the variable.
     *
     * This constructor initializes the memory location with the union
     * of the memory locations of constituent terms.
     */
    Variable(Scope scope, std::vector<TermAndLocation> termsAndLocations);

    /**
     * \return Scope of the variable.
     */
    Scope scope() const { return scope_; }

    /**
     * \return True if the variable is global, false otherwise.
     */
    bool isGlobal() const { return scope_ == GLOBAL; }

    /**
     * \return True if the variable is local, false otherwise.
     */
    bool isLocal() const { return scope_ == LOCAL; }

    /**
     * \return List of terms belonging to this variable, together with their memory locations.
     */
    const std::vector<TermAndLocation> &termsAndLocations() const { return termsAndLocations_; }

    /**
     * \return Memory location of the variable.
     */
    const MemoryLocation &memoryLocation() const { return memoryLocation_; }
};

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

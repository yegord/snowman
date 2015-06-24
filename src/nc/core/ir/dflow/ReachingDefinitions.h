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

#include <algorithm>
#include <cassert>
#include <vector>

#include <nc/common/Foreach.h>
#include <nc/common/Printable.h>

#include <nc/core/ir/MemoryLocation.h>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

/**
 * Reaching definitions.
 */
class ReachingDefinitions: public PrintableBase<ReachingDefinitions> {
public:
    /*
     * Memory location and the list of terms defining this memory location.
     */
    class Chunk {
        MemoryLocation location_; ///< Memory location.
        std::vector<const Term *> definitions_; ///< Terms defining this memory location.

        public:

        /*
         * Constructor.
         *
         * \param location      Valid memory location.
         * \param definitions   List of terms defining this memory location.
         */
        Chunk(const MemoryLocation &location, std::vector<const Term *> definitions):
            location_(location), definitions_(std::move(definitions))
        {
            assert(location);
        }

        /**
         * \return Memory location.
         */
        const MemoryLocation &location() const { return location_; }

        /**
         * \return List of terms defining the memory location.
         */
        std::vector<const Term *> &definitions() { return definitions_; }

        /**
         * \return List of terms defining the memory location.
         */
        const std::vector<const Term *> &definitions() const { return definitions_; }

        /**
         * \param that Another object of the same type.
         *
         * \return True if *this and that have the same memory location and list of terms,
         *         false otherwise.
         */
        bool operator==(const Chunk &that) const {
            return location_ == that.location_ && definitions_ == that.definitions_;
        }
    };

private:
    /**
     * Pairs of memory locations and terms defining them.
     * The pairs are sorted by memory location.
     * Terms are sorted using default comparator.
     */
    std::vector<Chunk> chunks_;

public:
    /**
     * \return Pairs of memory locations and vectors of terms defining them.
     *         The pairs are sorted by memory location.
     *         Terms are sorted using default comparator.
     */
    const std::vector<Chunk> &chunks() const { return chunks_; }

    /**
     * \return True if the list of pairs (chunks) is empty, false otherwise.
     */
    bool empty() const { return chunks_.empty(); }

    /**
     * Clears the reaching definitions.
     */
    void clear() { chunks_.clear(); }

    /**
     * Adds a definition of memory location, removing all previous definitions of overlapping memory locations.
     *
     * \param[in] memoryLocation Memory location.
     * \param[in] term Term which is the definition.
     */
    void addDefinition(const MemoryLocation &memoryLocation, const Term *term);

    /**
     * Kills definitions of given memory location.
     *
     * \param[in] memoryLocation Memory location.
     */
    void killDefinitions(const MemoryLocation &memoryLocation);

    /**
     * Computes a subset of reaching definitions defining (parts of)
     * given memory location.
     *
     * \param[in]  memoryLocation   Memory location.
     * \param[out] result           Resulting reaching definitions.
     */
    void project(const MemoryLocation &memoryLocation, ReachingDefinitions &result) const;

    /**
     * \param[in]  memoryLocation   Memory location.
     *
     * \return A subset of reaching definitions defining (parts of)
     * given memory location.
     *
     * \note Prefer using project() as it does less memory allocations.
     */
    ReachingDefinitions projected(const MemoryLocation &memoryLocation) const {
        ReachingDefinitions result;
        project(memoryLocation, result);
        return result;
    }

    /**
     * \return All defined memory locations in the domain.
     *
     * \param[in] domain Domain.
     */
    std::vector<MemoryLocation> getDefinedMemoryLocationsWithin(Domain domain) const;

    /**
     * Adds given reaching definitions to the list of known reaching definitions.
     *
     * \param[in] those Reaching definitions.
     */
    void merge(const ReachingDefinitions &those);

    /**
     * \return True, if these and given reaching definitions are the same.
     *
     * \param[in] those Reaching definitions.
     */
    bool operator==(const ReachingDefinitions &those) const { return chunks_ == those.chunks_; }

    /**
     * \return True, if these and given reaching definitions are different.
     *
     * \param[in] those Reaching definitions.
     */
    bool operator!=(const ReachingDefinitions &those) const { return !(*this == those); }

    /**
     * Removes all reaching definitions for which given predicate returns true.
     *
     * \param pred Predicate functor accepting two arguments: a memory location
     *             and a valid pointer to a term covering this location.
     * \tparam T Predicate functor type.
     */
    template<class T>
    void filterOut(const T &pred) {
        selfTest();
        foreach (auto &chunk, chunks_) {
            chunk.definitions().erase(
                std::remove_if(chunk.definitions().begin(), chunk.definitions().end(),
                    [&](const Term *term) -> bool { return pred(chunk.location(), term); }),
                chunk.definitions().end());
        }
        chunks_.erase(
            std::remove_if(chunks_.begin(), chunks_.end(),
                [](const Chunk &chunk) -> bool { return chunk.definitions().empty(); }),
            chunks_.end());
        selfTest();
    }

    void print(QTextStream &out) const;

private:
    /**
     * Checks if the data structure is in a valid state.
     * Fails with an assertion if not.
     */
    void selfTest() const {
#ifndef NDEBUG
        for (std::size_t i = 1; i < chunks_.size(); ++i) {
            assert(chunks_[i-1].location() < chunks_[i].location());
        }
#endif
    }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

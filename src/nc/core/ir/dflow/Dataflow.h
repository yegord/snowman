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

#include <memory>

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include <nc/core/ir/MemoryLocation.h>
#include <nc/core/ir/Term.h>

#include "ReachingDefinitions.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

class Value;

/**
 * This class contains results of dataflow and constant propagation and folding analysis.
 */
class Dataflow {
    /** Mapping from a term to a description of its value. */
    boost::unordered_map<const Term *, std::unique_ptr<Value>> term2value_;

    /** Mapping from a term to its memory location. */
    boost::unordered_map<const Term *, MemoryLocation> term2location_;

    /** Mapping from a term to the reaching definitions. */
    boost::unordered_map<const Term *, ReachingDefinitions> term2definitions_;

    /** Mapping from a statement to the reaching definitions. */
    boost::unordered_map<const Statement *, ReachingDefinitions> statement2definitions_;

public:
    /**
     * Constructor.
     */
    Dataflow();

    /**
     * Destructor.
     */
    ~Dataflow();

    /**
     * \param[in] term Valid pointer to a term.
     *
     * \return Pointer to the value description for this term. Can be nullptr.
     *         If the term is being assigned to, the value description
     *         for the right hand side of the assignment is returned.
     */
    Value *getValue(const Term *term);

    /**
     * \param[in] term Valid pointer to a term.
     *
     * \return Valid pointer to the value description for this term.
     *         If the term is being assigned to, the value description
     *         for the right hand side of the assignment is returned.
     */
    const Value *getValue(const Term *term) const;

    /**
     * \return Mapping from a term to the description of its value.
     */
    boost::unordered_map<const Term *, std::unique_ptr<Value>> &term2value() { return term2value_; }

    /**
     * \return Mapping from a term to the description of its value.
     */
    const boost::unordered_map<const Term *, std::unique_ptr<Value>> &term2value() const { return term2value_; }

    /**
     * \param[in] term Valid pointer to a term.
     *
     * \return Memory location occupied by the term. If no memory location is associated
     *         with this term, an invalid MemoryLocation object is returned.
     */
    const ir::MemoryLocation &getMemoryLocation(const Term *term) const {
        assert(term != nullptr);
        return nc::find(term2location_, term);
    }

    /**
     * Associates a memory location with given term.
     *
     * \param[in] term Valid pointer to a term.
     * \param[in] memoryLocation Memory location.
     *
     * \return Const reference to the memory location stored in the object.
     */
    const MemoryLocation &setMemoryLocation(const Term *term, const MemoryLocation &memoryLocation) {
        assert(term != nullptr);
        return (term2location_[term] = memoryLocation);
    }

    /**
     * \return Mapping from a term to its memory location.
     */
    boost::unordered_map<const Term *, MemoryLocation> &term2location() { return term2location_; };

    /**
     * \return Mapping from a term to its memory location.
     */
    const boost::unordered_map<const Term *, MemoryLocation> &term2location() const { return term2location_; };

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's definitions. If not set before, it is empty.
     */
    ReachingDefinitions &getDefinitions(const Term *term) {
        assert(term != nullptr);
        assert(term->isRead());
        return term2definitions_[term];
    }

    /**
     * \param[in] term Valid pointer to a read term.
     *
     * \return List of term's definitions. If not set before, it is empty.
     */
    const ReachingDefinitions &getDefinitions(const Term *term) const {
        assert(term != nullptr);
        assert(term->isRead());
        return nc::find(term2definitions_, term);
    }

    /**
     * \return Mapping from a term to its reaching definitions.
     */
    boost::unordered_map<const Term *, ReachingDefinitions> &term2definitions() { return term2definitions_; }

    /**
     * \return Mapping from a term to its reaching definitions.
     */
    const boost::unordered_map<const Term *, ReachingDefinitions> &term2definitions() const { return term2definitions_; }

    /**
     * \param[in] statement Valid pointer to a read statement.
     *
     * \return Definitions reaching the given statement.
     */
    ReachingDefinitions &getDefinitions(const Statement *statement) {
        assert(statement != nullptr);
        return statement2definitions_[statement];
    }

    /**
     * \param[in] statement Valid pointer to a read statement.
     *
     * \return Definitions reaching the given statement.
     */
    const ReachingDefinitions &getDefinitions(const Statement *statement) const {
        assert(statement != nullptr);
        return nc::find(statement2definitions_, statement);
    }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

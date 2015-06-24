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

#include <memory> /* unique_ptr */

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include "Variable.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace vars {

/**
 * Information about reconstructed variables.
 */
class Variables {
    /** All variables. */
    std::vector<std::unique_ptr<Variable>> variables_;

    /** Mapping of terms to variables. */
    boost::unordered_map<const Term *, Variable *> term2variable_;

public:
    /**
     * \return List of all reconstructed variables.
     */
    const std::vector<const Variable *> &list() const {
        return reinterpret_cast<const std::vector<const Variable *> &>(variables_);
    }

    /**
     * Adds information about reconstructed variable.
     *
     * \param variable Valid pointer to the information about the variable.
     */
    void addVariable(std::unique_ptr<Variable> variable);

    /**
     * \param term Valid pointer to a term.
     *
     * \return Pointer to the variable corresponding to the term. Can be nullptr.
     */
    const Variable *getVariable(const Term *term) const {
        assert(term != nullptr);
        return nc::find(term2variable_, term);
    }
};

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

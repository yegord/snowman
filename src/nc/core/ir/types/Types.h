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

#include <boost/unordered_map.hpp>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace types {

class Type;

/**
 * Information about types of terms.
 */
class Types {
    mutable boost::unordered_map<const Term *, std::unique_ptr<Type> > types_; ///< Mapping of terms to their type traits.

    public:

    /**
     * Constructor.
     */
    Types();

    /**
     * Destructor.
     */
    ~Types();

    /**
     * \param[in] term Term.
     *
     * \return Valid pointer to Type traits for this term.
     */
    Type *getType(const Term *term);

    /**
     * \param[in] term Term.
     *
     * \return Valid pointer to type traits for this term.
     */
    const Type *getType(const Term *term) const;

    /**
     * \return Mapping of terms to their type traits.
     */
    boost::unordered_map<const Term *, std::unique_ptr<Type> > &map() { return types_; };
};

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */

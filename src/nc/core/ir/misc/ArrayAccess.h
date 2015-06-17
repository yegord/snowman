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

namespace nc {
namespace core {
namespace ir {

class Term;

namespace misc {

/**
 * Describes an indexing expression like "[base + stride * index]".
 */
class ArrayAccess {
    ConstantValue base_;
    ConstantValue stride_;
    const Term *index_;

public:
    /**
     * Constructs an invalid indexing expression.
     */
    ArrayAccess(): index_(nullptr) {}

    /**
     * Constructs a valid indexing expression.
     *
     * \param base      Base offset.
     * \param stride    Stride value.
     * \param index     Valid pointer to the index term.
     */
    ArrayAccess(ConstantValue base, ConstantValue stride, const Term *index):
        base_(base), stride_(stride), index_(index)
    {
        assert(index_ != nullptr);
    }

    /**
     * \return Base, i.e. the constant offset in the expression.
     */
    ConstantValue base() const { assert(*this); return base_; }

    /**
     * \return Stride, i.e. the constant multiplier in the expression.
     */
    ConstantValue stride() const { assert(*this); return stride_; }

    /**
     * \return Valid pointer to the term serving as an index, i.e. the non-constant
     *         multiplier in the expression.
     */
    const Term *index() const { assert(*this); return index_; }

    /**
     * \return A non-nullptr pointer if and only if the descriptor is valid.
     */
    operator const void*() const { return index_; }
};

}}}} // namespace nc::core::ir::misc

/* vim:set et sts=4 sw=4: */

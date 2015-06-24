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

class BasicBlock;
class Term;

namespace misc {

/**
 * Describes a jump of the form "if (index <= maxValue) then goto ifPassed else goto ifFailed".
 */
class BoundsCheck {
    const Term *index_;
    ConstantValue maxValue_;
    const ir::BasicBlock *ifFailed_;

public:
    /**
     * Constructs an invalid index check.
     */
    BoundsCheck(): index_(nullptr) {}

    /**
     * Constructs a valid index check.
     *
     * \param index     Valid pointer to the index term.
     * \param maxValue  Maximal value the index can have.
     * \param ifFailed  Pointer to the basic block getting control if the check fails. Can be nullptr.
     */
    BoundsCheck(const Term *index, ConstantValue maxValue, const BasicBlock *ifFailed):
        index_(index), maxValue_(maxValue), ifFailed_(ifFailed)
    {
        assert(index_ != nullptr);
    }

    /**
     * \return Valid pointer to the term serving as the index, i.e. the value
     *         being compared against a constant max value.
     */
    const Term *index() const { assert(*this); return index_; }

    /**
     * \return Maximal value the index can have.
     */
    ConstantValue maxValue() const { assert(*this); return maxValue_; }

    /**
     * \return Pointer to the basic block getting control if the check fails. Can be nullptr.
     */
    const BasicBlock *ifFailed() const { assert(*this); return ifFailed_; }

    /**
     * \return A non-nullptr pointer if and only if the object describes a valid index check.
     */
    operator const void*() const { return index_; }
};

}}}} // namespace nc::core::ir::misc

/* vim:set et sts=4 sw=4: */

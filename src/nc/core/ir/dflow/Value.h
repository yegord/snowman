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

#include <nc/common/Types.h>

#include "AbstractValue.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

/**
 * Dataflow information about a term.
 */
class Value {
    AbstractValue abstractValue_; ///< Abstract value of the term, in the host byte order.

    bool isStackOffset_; ///< Value is a stack pointer with a known offset from the frame base.
    bool isNotStackOffset_; ///< Value is not a stack pointer with a known offset from the frame base.

    SignedConstantValue stackOffset_; ///< Offset to stack frame base (in bytes), if the value is a stack pointer.

    bool isProduct_; ///< Value was computed via multiplication.
    bool isNotProduct_; ///< Value was computed not via multiplication.

    bool isReturnAddress_; ///< Value is a return address.
    bool isNotReturnAddress_; ///< Value is not a return address.

public:
    /**
     * Class constructor.
     *
     * \param size Size of the term.
     */
    Value(SmallBitSize size);

    /**
     * \return Abstract value of the term.
     */
    const AbstractValue &abstractValue() const { return abstractValue_; }

    /**
     * Sets the abstract value of the term.
     *
     * \param value New abstract value.
     *
     * The value is resized to the size given to constructor of this class.
     */
    void setAbstractValue(AbstractValue value) { abstractValue_ = value.resize(abstractValue_.size()); }

    /**
     * \return True if the value is stack pointer with a known offset
     *         from the frame base, false otherwise.
     */
    bool isStackOffset() const { return isStackOffset_ && !isNotStackOffset_; }

    /**
     * \return True if the value is not a stack pointer with a known offset
     *         from the frame base, false otherwise.
     */
    bool isNotStackOffset() const { return isNotStackOffset_; }

    /**
     * Marks the value as a stack pointer with a given offset.
     *
     * \param[in] offset Offset to stack frame base.
     */
    void makeStackOffset(SignedConstantValue offset);

    /**
     * Marks the value as not a stack pointer.
     */
    void makeNotStackOffset() { isNotStackOffset_ = true; }

    /**
     * \return Offset to stack frame base (in bytes), if the value is a stack pointer.
     */
    SignedConstantValue stackOffset() const { assert(isStackOffset()); return stackOffset_; }

    /**
     * \return True, if the value has been computed via multiplication.
     */
    bool isProduct() const { return isProduct_ && !isNotProduct_; }

    /**
     * \return True, if the value has not been computed via multiplication.
     */
    bool isNotProduct() const { return isNotProduct_; }

    /**
     * Marks the value as being computed via multiplication.
     */
    void makeProduct() { isProduct_ = true; }

    /**
     * Marks the value as being computed not via multiplication.
     */
    void makeNotProduct() { isNotProduct_ = true; }

    /**
     * \return True if the value is a return address.
     */
    bool isReturnAddress() const { return isReturnAddress_ && !isNotReturnAddress_; }

    /**
     * \return True if the value is a not return address.
     */
    bool isNotReturnAddress() const { return isNotReturnAddress_; }

    /**
     * Marks the value as being a return address.
     */
    void makeReturnAddress() { isReturnAddress_ = true; }

    /**
     * Marks the value as being not a return address.
     */
    void makeNotReturnAddress() { isNotReturnAddress_ = true; }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

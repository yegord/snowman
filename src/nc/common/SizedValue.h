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

#include <nc/common/BitTwiddling.h>
#include <nc/common/Types.h>
#include <nc/common/Unused.h>

namespace nc {

/**
 * Integer value with a size.
 */
class SizedValue {
    /** Size of the value. */
    SmallBitSize size_;

    /** The value. All its bits from size_ and higher are zero. */
    ConstantValue value_;

public:
    /**
     * Constructs a value of zero size.
     */
    SizedValue(): size_(0), value_(0) {}

    /**
     * Constructor.
     *
     * \param[in] size Size of the value.
     * \param[in] value Bits of the value.
     *
     * The value is truncated to the lowest <em>size</em> bits.
     */
    SizedValue(SmallBitSize size, ConstantValue value):
        size_(size),
        value_(bitTruncate(value, size))
    {
        assert(size >= 0);
    }

private:
    /**
     * Helper enum to construct values without truncation.
     */
    enum Exact {};

public:
    /**
     * Helper member to construct values without truncation.
     */
    static const Exact exact = static_cast<Exact>(0);

    /**
     * Constructor.
     *
     * \param[in] size  Size of the value.
     * \param[in] value Bits of the value, with <em>size</em> and higher cleared.
     * \param[in] exact Exact construction flag.
     */
    SizedValue(SmallBitSize size, ConstantValue value, Exact exact):
        size_(size),
        value_(value)
    {
        NC_UNUSED(exact);
        assert(size >= 0);
        assert(bitTruncate(value, size) == value);
    }

    /**
     * \return Size of the value.
     */
    SmallBitSize size() const { return size_; }

    /**
     * \return Value in the unsigned sense (bits as they are, padded with zeroes).
     */
    ConstantValue value() const { return value_; }

    /**
     * \return Value in the signed sense (sign-extended value).
     */
    SignedConstantValue signedValue() const { return signExtend(value_, size_); }

    /**
     * \return Absolute value.
     */
    ConstantValue absoluteValue() const { return bitAbs(value_, size_); }
};

} // namespace nc

/* vim:set et sts=4 sw=4: */

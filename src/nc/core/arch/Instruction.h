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

#include <nc/common/Printable.h>
#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace arch {

/**
 * Base class for instructions.
 */
class Instruction: public Printable {
    /** Instruction's address in bytes. */
    ByteAddr addr_;

    /** Instruction's size in bytes. */
    SmallByteSize size_;

public:
    /**
     * Constructor.
     *
     * \param[in] addr  Instruction's address in bytes.
     * \param[in] size  Instruction's size in bytes.
     */
    Instruction(ByteAddr addr = 0, SmallByteSize size = 0):
        addr_(addr), size_(size)
    {
        assert(size >= 0);
    }

    /**
     * Virtual destructor.
     */
    virtual ~Instruction();

    /**
     * \return Instruction's address in bytes.
     */
    ByteAddr addr() const { return addr_; }

    /**
     * Sets this instruction's address.
     *
     * \param[in] addr Address in bytes.
     */
    void setAddr(ByteAddr addr) { addr_ = addr; }

    /**
     * \return Instruction's size in bytes.
     */
    SmallByteSize size() const { return size_; }

    /**
     * Sets the size of this instruction.
     *
     * \param[in] size Size in bytes.
     */
    void setSize(SmallByteSize size) { assert(size > 0); size_ = size; }

    /**
     * \return Address of the next instruction in memory.
     */
    ByteAddr endAddr() const { return addr_ + size_; }
};

} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

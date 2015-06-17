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

#include <map>
#include <memory> /* std::shared_ptr */

#include <boost/range/adaptor/map.hpp>

#include <nc/common/PrintCallback.h>
#include <nc/common/Range.h> /* nc::find */

#include "Instruction.h"

namespace nc { namespace core { namespace arch {

class Instruction;

/**
 * Class representing a set of instructions.
 */
class Instructions {
    /** Type of mapping of addresses to instruction. */
    typedef std::map<ByteAddr, std::shared_ptr<const Instruction>> AddressInstructionMap;

    /** Mapping of addresses to instructions. */
    AddressInstructionMap address2instruction_;

public:
    /** Type for the sorted range of instructions. */
    typedef boost::select_second_const_range<AddressInstructionMap> InstructionsRange;

    /**
     * \return Range of instructions sorted by their addresses in ascending order.
     */
    InstructionsRange all() const { return address2instruction_ | boost::adaptors::map_values; }

    /**
     * \param[in] addr Address.
     *
     * \return Pointer to the instruction starting at the given address.
     *         Can be nullptr, if there is no such instructions.
     */
    const std::shared_ptr<const Instruction> &get(ByteAddr addr) const { return nc::find(address2instruction_, addr); }

    /**
     * \param[in] addr Address.
     *
     * \return Pointer to the instruction covering the given address.
     *         Can be nullptr, if there are no instructions.
     */
    const std::shared_ptr<const Instruction> &getCovering(ByteAddr addr) const;

    /**
     * Adds instruction if there is no instruction with the given address yet.
     *
     * \param instruction Valid pointer to an instruction.
     *
     * \return True if the instruction was added, false otherwise.
     */
    bool add(std::shared_ptr<const Instruction> instruction);

    /**
     * Deletes given instruction from the set.
     *
     * \param[in] instruction Instruction to be removed.
     *
     * \return True if the instruction was removed, false otherwise.
     */
    bool remove(const Instruction *instruction);

    /**
     * \return Number of instructions in the set.
     */
    std::size_t size() const { return address2instruction_.size(); }

    /**
     * \return True if the set is empty, false is otherwise.
     */
    bool empty() const { return size() == 0; }

    /**
     * Prints all the instructions into a stream.
     *
     * \param out Output stream.
     * \param callback Pointer to the print callback. Can be nullptr.
     */
    void print(QTextStream &out, PrintCallback<const Instruction *> *callback = nullptr) const;
};

}}} // namespace nc::core::arch

/* vim:set et sts=4 sw=4: */

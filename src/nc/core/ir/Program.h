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

#include <map>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <nc/common/Printable.h>
#include <nc/common/Range.h> /* nc::contains */
#include <nc/common/RangeClass.h>

#include "BasicBlock.h"

namespace nc {
namespace core {

namespace arch {
    class Instruction;
}

namespace ir {

/**
 * Intermediate representation of a program.
 */
class Program: public PrintableBase<Program>, boost::noncopyable {
public:
    typedef nc::ilist<BasicBlock> BasicBlocks;

private:
    typedef Range<ByteAddr> AddrRange;

    class ToTheLeft {
    public:
        bool operator()(const AddrRange &a, const AddrRange &b) const {
            return a.end() <= b.start() && a != b;
        }
    };

    BasicBlocks basicBlocks_; ///< Basic blocks.
    std::map<AddrRange, BasicBlock *, ToTheLeft> range2basicBlock_; ///< Mapping of a range of addresses to the basic block covering the range.
    boost::unordered_map<ByteAddr, BasicBlock *> start2basicBlock_; ///< Mapping of an address to the basic block at this address.
    boost::unordered_set<ByteAddr> calledAddresses_; ///< Addresses having calls to them.

public:
    /**
     * Constructor.
     */
    Program();

    /**
     * Destructor.
     */
    ~Program();

    /**
     * \return All basic blocks of the program.
     *
     * \warning Do not insert basic blocks into this container directly.
     *          Use methods of Program class instead.
     */
    BasicBlocks &basicBlocks() { return basicBlocks_; }

    /**
     * \return All basic blocks of the program.
     */
    const BasicBlocks &basicBlocks() const { return basicBlocks_; }

    /**
     * \param address       Address.
     *
     * \return Pointer to the basic block starting at given address. Can be nullptr.
     */
    BasicBlock *getBasicBlockStartingAt(ByteAddr address) const;

    /**
     * \param address       Address.
     *
     * \return Pointer to the basic block covering given address. Can be nullptr.
     */
    BasicBlock *getBasicBlockCovering(ByteAddr address) const;

    /**
     * \return Valid pointer to a newly created non-memory-bound basic block.
     */
    BasicBlock *createBasicBlock();

    /**
     * \param[in] address Start address of the basic block.
     *
     * \return Valid pointer to a memory-bound basic block starting at given address.
     *         When block with given starting already exists, it is returned.
     *         When given address is in the middle of existing basic block, the latter
     *         is split into two basic blocks, and the second one is returned.
     *         When given address is not covered by any existing block, a new
     *         empty memory-bound block is created and returned.
     */
    BasicBlock *createBasicBlock(ByteAddr address);

    /**
     * \param[in] instruction An instruction.
     *
     * \return Valid pointer to a basic block to which the instruction should
     *         be appended.
     */
    BasicBlock *getBasicBlockForInstruction(const arch::Instruction *instruction);

    /**
     * \return Addresses being arguments of calls.
     */
    const boost::unordered_set<ByteAddr> &calledAddresses() const { return calledAddresses_; };

    /**
     * Adds given address to the list of called addresses.
     *
     * \param[in] addr Entry of a function (i.e. an address having calls to himself).
     */
    void addCalledAddress(ByteAddr addr) { calledAddresses_.insert(addr); }

    /**
     * \param[in] addr An address.
     *
     * \return True if the address has a call to it.
     */
    bool isCalledAddress(ByteAddr addr) const { return nc::contains(calledAddresses_, addr); }

    /**
     * Prints the graph into a stream in DOT format.
     *
     * \param out Output stream.
     */
    void print(QTextStream &out) const;

private:
    /**
     * Takes ownership of given basic block.
     *
     * \param basicBlock Valid pointer to a basic block.
     *
     * \return Pointer to the basicBlock that was given.
     */
    BasicBlock *takeOwnership(std::unique_ptr<BasicBlock> basicBlock);

    /**
     * Maps the memory range occupied by given basic block to this basic block.
     * getBasicBlockCovering() will only return a basic block if addRange() was called on it.
     *
     * \param basicBlock Valid pointer to a basic block.
     */
    void addRange(BasicBlock *basicBlock);

    /**
     * Unmaps the memory range occupied by given basic block to this basic block.
     *
     * \param basicBlock Valid pointer to a basic block.
     */
    void removeRange(BasicBlock *basicBlock);
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

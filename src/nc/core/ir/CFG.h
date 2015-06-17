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
#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/common/Printable.h>
#include <nc/common/Range.h>
#include <nc/common/ilist.h>

namespace nc {
namespace core {
namespace ir {

class BasicBlock;
class JumpTarget;

/**
 * Control flow graph.
 *
 * Objects of this class can be constructed from a set of basic blocks
 * and contain information about the successors and predecessors of the
 * basic blocks.
 */
class CFG: public PrintableBase<CFG> {
public:
    typedef nc::ilist<BasicBlock> BasicBlocks;

private:
    /** References to the set of basic blocks passed to the constructor. */
    const BasicBlocks &basicBlocks_;

    /** Mapping from a basic block to the list of its successors. */
    boost::unordered_map<const BasicBlock *, std::vector<const BasicBlock *>> successors_;

    /** Mapping from a basic block to the list of its predecessors. */
    boost::unordered_map<const BasicBlock *, std::vector<const BasicBlock *>> predecessors_;

public:
    /**
     * Constructs control flow graph from a set of basic blocks.
     *
     * \param[in] basicBlocks Basic blocks.
     *
     * Note that the set of basic blocks is not copied.
     * Instead, only a reference to it is stored.
     */
    CFG(const BasicBlocks &basicBlocks);

    /**
     * \return The set of basic blocks that was passed to the constructor.
     */
    const BasicBlocks &basicBlocks() const { return basicBlocks_; }

    /**
     * \param[in] basicBlock Valid pointer to a basic block.
     *
     * \return List of successors of the basic block.
     */
    const std::vector<const BasicBlock *> &getSuccessors(const BasicBlock *basicBlock) const {
        assert(basicBlock != nullptr);
        return nc::find(successors_, basicBlock);
    }

    /**
     * \param[in] basicBlock Valid pointer to a basic block.
     *
     * \return List of predecessors of the basic block.
     */
    const std::vector<const BasicBlock *> &getPredecessors(const BasicBlock *basicBlock) const {
        assert(basicBlock != nullptr);
        return nc::find(predecessors_, basicBlock);
    }

    /**
     * Prints the CFG in DOT format into a stream.
     *
     * \param[in] out Output stream.
     */
    void print(QTextStream &out) const;

private:
    /**
     * Adds an edge from a predecessor to a successor.
     *
     * \param[in] predecessor Valid pointer to a predecessor basic block.
     * \param[in] successor   Valid pointer to a successor basic block.
     */
    void addConnection(const BasicBlock *predecessor, const BasicBlock *successor);

    /**
     * Adds edges from a predecessor to all jump targets.
     *
     * \param[in] predecessor   Valid pointer to a basic block.
     * \param[in] jumpTarget    Jump target of the predecessor's terminating jump.
     */
    void addConnections(const BasicBlock *predecessor, const JumpTarget &jumpTarget);
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

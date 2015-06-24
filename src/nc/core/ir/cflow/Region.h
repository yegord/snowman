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
#include <memory>

#include "Node.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

class Switch;

/**
 * Region is a set of nodes with a single entry and possibly multiple exits.
 */
class Region: public Node {
    NC_BASE_CLASS(Region, regionKind)

public:
    /**
     * Region kind.
     */
    enum RegionKind {
        UNKNOWN, ///< Unknown region.
        BLOCK, ///< Sequence of consecutive nodes.
        COMPOUND_CONDITION, ///< Short-circuit computation of && or ||.
        IF_THEN, ///< If-then.
        IF_THEN_ELSE, ///< If-then-else.
        LOOP, ///< General loop.
        WHILE, ///< Loop with precondition.
        DO_WHILE, ///< Loop with postcondition.
        SWITCH ///< Switch.
    };

private:
    Node *entry_; ///< Region's entry node.
    std::vector<Node *> nodes_; ///< Nodes of the region.
    const BasicBlock *exitBasicBlock_; ///< Exit basic block.
    Node *loopCondition_; ///< Node with the loop condition.

public:
    /**
     * \param regionKind    Region kind.
     */
    Region(RegionKind regionKind):
        Node(REGION), regionKind_(regionKind), entry_(nullptr), exitBasicBlock_(nullptr), loopCondition_(nullptr)
    {}

    /**
     * Sets region kind.
     *
     * \param[in] regionKind Region kind.
     */
    void setRegionKind(RegionKind regionKind) { regionKind_ = regionKind; }

    /**
     * \return Pointer to the region's entry. Can be nullptr.
     */
    Node *entry() const { return entry_; }

    /**
     * Sets region entry.
     *
     * \param[in] entry Valid pointer to the new region entry.
     */
    void setEntry(Node *entry) {
        assert(entry != nullptr);
        entry_ = entry;
    }

    /**
     * \return Region nodes.
     */
    std::vector<Node *> &nodes() { return nodes_; }

    /**
     * \return Region nodes.
     */
    const std::vector<Node *> &nodes() const { return nodes_; }

    /**
     * Adds subregion to the region.
     * All nodes of the subregion are removed from the region.
     * All edges from the nodes of the subregion become edges from the subregion.
     * All edges to the entry node of the subregion become edges to the subregion.
     * All other edges as well as duplicate edges are deleted.
     *
     * \param[in] subregion Valid pointer to the subregion.
     */
    bool addSubregion(std::unique_ptr<Region> subregion);

    /**
     * \return Pointer to the exit basic block. Can be nullptr.
     */
    const BasicBlock *exitBasicBlock() const { return exitBasicBlock_; }

    /**
     * Sets the exit basic block.
     * Exit basic block gets control when the region finishes execution.
     *
     * \param basicBlock Pointer to the new exit basic block. Can be nullptr.
     */
    void setExitBasicBlock(const BasicBlock *basicBlock) { exitBasicBlock_ = basicBlock; }

    /**
     * \return Pointer to the node being the condition of the loop. Can be nullptr.
     */
    Node *loopCondition() const { return loopCondition_; }

    /**
     * Sets the node being the condition of a loop.
     *
     * \param condition Pointer to the loop condition node.
     */
    void setLoopCondition(Node *condition) { loopCondition_ = condition; }

    const BasicBlock *getEntryBasicBlock() const override;
    bool isCondition() const override;
    void print(QTextStream &out) const override;
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::ir::cflow::Region, nc::core::ir::cflow::Switch, nc::core::ir::cflow::Region::SWITCH)

/* vim:set et sts=4 sw=4: */

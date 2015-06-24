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

#include <nc/common/Printable.h>
#include <nc/common/Subclass.h>
#include <nc/common/Types.h>

#include <vector>

namespace nc {
namespace core {
namespace ir {

class BasicBlock;

namespace cflow {

class BasicNode;
class Edge;
class Graph;
class Region;

/**
 * Graph node.
 */
class Node: public Printable {
    NC_BASE_CLASS(Node, nodeKind)
    friend class Edge;

public:
    /**
     * Node kind.
     */
    enum NodeKind {
        BASIC, ///< Basic block node.
        REGION ///< Region.
    };

private:
    Region *parent_; ///< Parent region.
    std::vector<Edge *> inEdges_; ///< Incoming edges.
    std::vector<Edge *> outEdges_; ///< Outgoing edges.

public:
    /**
     * \param kind Kind of the node.
     */
    Node(NodeKind kind): nodeKind_(kind), parent_(nullptr) {}

    /**
     * Virtual destructor.
     */
    virtual ~Node();

    /**
     * \return Pointer to the parent region. Can be nullptr.
     */
    Region *parent() const { return parent_; }

    /**
     * Sets parent region.
     *
     * \param[in] parent Parent region.
     */
    void setParent(Region *parent) { parent_ = parent; }

    /**
     * \return Incoming edges.
     */
    const std::vector<Edge *> &inEdges() const { return inEdges_; }

    /**
     * \return Outgoing edges.
     */
    const std::vector<Edge *> &outEdges() const { return outEdges_; }

    /**
     * \return Valid pointer to the entry basic block of the node.
     */
    virtual const BasicBlock *getEntryBasicBlock() const = 0;

    /**
     * \return Valid pointer to the predecessor node if there is only one
     *         incoming edges, nullptr otherwise.
     */
    Node *uniquePredecessor() const;

    /**
     * \return Valid pointer to the successor node if there is only one
     *         outgoing edges, nullptr otherwise.
     */
    Node *uniqueSuccessor() const;

    /**
     * \return True if the node has two different successors.
     */
    bool isFork() const;

    /**
     * \param[in] notThis Valid pointer to a node.
     *
     * \return Pointer to any successor different from the notThis;
     *         nullptr if there is no such successor.
     */
    Node *getOtherSuccessor(const Node *notThis) const;

    /**
     * \return True if this is a basic block node with a conditional jump whose
     *         destinations are known basic blocks or a compound condition,
     *         false otherwise.
     */
    virtual bool isCondition() const = 0;
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::ir::cflow::Node, nc::core::ir::cflow::BasicNode, nc::core::ir::cflow::Node::BASIC)
NC_SUBCLASS(nc::core::ir::cflow::Node, nc::core::ir::cflow::Region,    nc::core::ir::cflow::Node::REGION)

/* vim:set et sts=4 sw=4: */

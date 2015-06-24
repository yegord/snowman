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

#include <vector>

#include <boost/unordered_map.hpp>

namespace nc {
namespace core {
namespace ir {
namespace cflow {

class Dfs;
class Node;

/**
 * This class finds all nodes on the paths from a given node to itself
 * ending with a back edge to the given node. All the discovered nodes
 * are expected to belong to the loop with the given node being its entry.
 */
class LoopExplorer {
    /** Node color. */
    enum NodeColor {
        WHITE,
        GRAY,
        BLACK
    };

    /** Entry node of a potential loop. */
    Node *entry_;

    /** Mapping from a node to its color. */
    boost::unordered_map<Node *, NodeColor> node2color_;

    /* Nodes on cyclic paths from the entry to the entry. */
    std::vector<Node *> loopNodes_;

public:
    /**
     * Constructor.
     *
     * \param entry Valid pointer to the node being a potential loop entry.
     * \param dfs   DFS results for entry->parent().
     */
    LoopExplorer(Node *entry, const Dfs &dfs);

    /**
     * \return Nodes on cyclic paths from the entry to the entry.
     */
    std::vector<Node *> &loopNodes() { return loopNodes_; }

    /**
     * \return Nodes on cyclic paths from the entry to the entry.
     */
    const std::vector<Node *> &loopNodes() const { return loopNodes_; }

private:
    /**
     * Visits given node and, if the node is not entry, recursively
     * visits all its WHITE predecessors. All the visited nodes are
     * painted GRAY.
     *
     * \param node Valid pointer to a WHITE node.
     */
    void backwardVisit(Node *node);

    /**
     * Visits given node and recursively visits all its GRAY successors.
     * All the visited nodes are painted BLACK.
     *
     * \param node Valid pointer to a GRAY node.
     */
    void forwardVisit(Node *node);
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

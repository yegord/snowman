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
#include <boost/unordered_set.hpp>

#include <nc/common/Range.h>

namespace nc {
namespace core {
namespace ir {
namespace cflow {

class Edge;
class Node;
class Region;

/**
 * This class performs a depth-first search in a given region, sorts its
 * nodes topologically, detects back edges.
 */
class Dfs {
public:
    /** Node color. */
    enum NodeColor {
        WHITE,
        GRAY,
        BLACK
    };

    /** Edge type. */
    enum EdgeType {
        UNKNOWN,
        FORWARD,
        BACK,
        CROSS
    };

private:
    /** List of region nodes in the order of discovery. */
    std::vector<Node *> preordering_;

    /** List of region nodes in the order of leaving. */
    std::vector<Node *> postordering_;

    /** Mapping from a node to its color. */
    boost::unordered_map<const Node *, NodeColor> node2color_;

    /** Mapping from an edge to its type. */
    boost::unordered_map<const Edge *, EdgeType> edge2type_;

public:

    /**
     * Performs DFS in the given region.
     *
     * \param region Valid pointer to the region.
     */
    Dfs(const Region *region);

    /**
     * \return List of region nodes in the order of discovery.
     */
    std::vector<Node *> &preordering() { return preordering_; }

    /**
     * \return List of region nodes in the order of discovery.
     */
    const std::vector<Node *> &preordering() const { return preordering_; }

    /**
     * \return List of region nodes in the order of leaving.
     */
    std::vector<Node *> &postordering() { return postordering_; }

    /**
     * \return List of region nodes in the order of leaving.
     */
    const std::vector<Node *> &postordering() const { return postordering_; }

    /**
     * \param edge Valid pointer to an edge.
     *
     * \return Edge's type.
     */
    EdgeType getEdgeType(const Edge *edge) const { return nc::find(edge2type_, edge, UNKNOWN); }

private:

    /**
     * Visits given node and all its unvisited successors.
     *
     * \param node Valid pointer to a not yet visited node.
     */
    void visit(Node *node);
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

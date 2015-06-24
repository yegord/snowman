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

#include <memory>

namespace nc {
namespace core {
namespace ir {

namespace dflow {
    class Dataflow;
}

namespace cflow {

class Dfs;
class Graph;
class Node;
class Region;

/**
 * Class performing structural analysis on a graph.
 */
class StructureAnalyzer {
    /** Graph to analyze. */
    Graph &graph_;

    /** Dataflow information. */
    const dflow::Dataflow &dataflow_;

public:
    /**
     * Class constructor.
     *
     * \param graph Graph to analyze.
     * \param dataflow Dataflow information.
     */
    StructureAnalyzer(Graph &graph, const dflow::Dataflow &dataflow):
        graph_(graph), dataflow_(dataflow)
    {}

    /**
     * Performs structural analysis on the graph.
     */
    void analyze();

private:
    /**
     * Runs structural analysis in the region.
     *
     * \param[in] region Valid pointer to a region.
     */
    void analyze(Region *region);

    /**
     * Tries to reduce block region ending in the node.
     *
     * \param[in] entry Valid pointer to the entry node.
     *
     * \return True if the region was reduced.
     */
    bool reduceBlock(Node *entry);

    /**
     * Tries to reduce if-then or if-then-else region with given entry.
     *
     * \param[in] entry Valid pointer to the entry node.
     *
     * \return True if the region was reduced.
     */
    bool reduceConditional(Node *entry);

    /**
     * Tries to reduce if-then-else region with given entry.
     * Successfully reduces regions even if then and else branches do not merge.
     *
     * \param[in] entry Valid pointer to the entry node.
     *
     * \return True if the region was reduced.
     */
    bool reduceHopelessConditional(Node *entry);

    /**
     * Tries to reduce compound condition region with given entry.
     *
     * \param[in] entry Valid pointer to the entry node.
     *
     * \return True if the region was reduced.
     */
    bool reduceCompoundCondition(Node *entry);

    /**
     * Tries to reduce cyclic region with the given entry.
     *
     * \param[in] entry Valid pointer to the entry node.
     * \param[in] dfs   Depth-first results for node->parent().
     *                  (Needed for recognizing back edges.)
     *
     * \return True if the region was reduced.
     */
    bool reduceCyclic(Node *entry, const Dfs &dfs);

    /**
     * Tries to reduce a switch using a jump table.
     *
     * \param[in] entry Valid pointer to the switch's entry node.
     *
     * \return True if the region was reduced.
     */
    bool reduceSwitch(Node *entry);

    /**
     * Inserts a subregion into a region.
     * Can fail if the structure of the parent region gets broken because
     * of this insertion. For example, if the entry node of the region
     * is in the inserted subregion, but is not subregion's entry node.
     *
     * All nodes of the subregion are removed from the region.
     * All edges from the nodes of the subregion become edges from the subregion.
     * All edges to the entry node of the subregion become edges to the subregion.
     * All other edges as well as duplicate edges are deleted.
     *
     * \param region Valid pointer to the region.
     * \param subregion Valid pointer to the subregion.
     *
     * \return Pointer to the subregion on success, nullptr on failure.
     */
    Region *insertSubregion(Region *region, std::unique_ptr<Region> subregion);
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

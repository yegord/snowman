/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "LoopExplorer.h"

#include <nc/common/Foreach.h>

#include "Dfs.h"
#include "Edge.h"
#include "Node.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

LoopExplorer::LoopExplorer(Node *entry, const Dfs &dfs):
    entry_(entry)
{
    assert(entry != nullptr);

    /*
     * Find all nodes that can be reached from the back-edge
     * predecessors by reversed edges and paint them gray.
     */
    foreach (Edge *edge, entry_->inEdges()) {
        if (dfs.getEdgeType(edge) == Dfs::BACK) {
            if (find(node2color_, edge->tail()) == WHITE) {
                backwardVisit(edge->tail());
            }
        }
    }

    /*
     * Find all gray nodes that can be visited from the suspected
     * loop entry and paint them black. All the black nodes belong
     * to the loop.
     */
    if (find(node2color_, entry_) == GRAY) {
        forwardVisit(entry_);
    }
}

void LoopExplorer::backwardVisit(Node *node) {
    assert(node != nullptr);
    assert(find(node2color_, node) == WHITE);

    node2color_[node] = GRAY;

    if (node == entry_) {
        return;
    }

    foreach (Edge *edge, node->inEdges()) {
        if (find(node2color_, edge->tail()) == WHITE) {
            backwardVisit(edge->tail());
        }
    }
}

void LoopExplorer::forwardVisit(Node *node) {
    assert(node != nullptr);
    assert(find(node2color_, node) == GRAY);

    node2color_[node] = BLACK;
    loopNodes_.push_back(node);

    foreach (Edge *edge, node->outEdges()) {
        if (find(node2color_, edge->head()) == GRAY) {
            forwardVisit(edge->head());
        }
    }
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

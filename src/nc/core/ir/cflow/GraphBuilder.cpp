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

#include "GraphBuilder.h"

#include <cassert>

#include <boost/unordered_map.hpp>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/CFG.h>
#include <nc/core/ir/Function.h>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include "BasicNode.h"
#include "Graph.h"
#include "Region.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

void GraphBuilder::operator()(Graph &graph, const ir::Function *function) const {
    /* Create the root bulk region. */
    graph.setRoot(graph.addNode(std::make_unique<Region>(Region::UNKNOWN)));

    /*
     * Create nodes.
     */
    boost::unordered_map<const ir::BasicBlock *, Node *> basicBlock2node;

    foreach (const ir::BasicBlock *basicBlock, function->basicBlocks()) {
        auto node = graph.addNode(std::make_unique<BasicNode>(basicBlock));
        graph.root()->nodes().push_back(node);
        node->setParent(graph.root());
        basicBlock2node[basicBlock] = node;
    }
    graph.root()->setEntry(basicBlock2node[function->entry()]);

    /*
     * Create edges.
     */
    CFG cfg(function->basicBlocks());

    foreach (const ir::BasicBlock *tailBasicBlock, function->basicBlocks()) {
        Node *tail = nc::find(basicBlock2node, tailBasicBlock);
        assert(tail);

        foreach (const ir::BasicBlock *headBasicBlock, cfg.getSuccessors(tailBasicBlock)) {
            Node *head = nc::find(basicBlock2node, headBasicBlock);
            assert(head);

            graph.createEdge(tail, head);
        }
    }
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

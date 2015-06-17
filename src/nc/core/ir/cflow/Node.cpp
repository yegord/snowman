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

#include "Node.h"

#include <nc/common/Foreach.h>

#include "Edge.h"
#include "Graph.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

Node::~Node() {}

Node *Node::uniquePredecessor() const {
    if (inEdges().size() == 1) {
        return inEdges()[0]->tail();
    } else {
        return nullptr;
    }
}

Node *Node::uniqueSuccessor() const {
    if (outEdges().size() == 1) {
        return outEdges()[0]->head();
    } else {
        return nullptr;
    }
}

bool Node::isFork() const {
    return outEdges().size() == 2 && outEdges()[0]->head() != outEdges()[1]->head();
}

Node *Node::getOtherSuccessor(const Node *notThis) const {
    foreach (const Edge *edge, outEdges()) {
        if (edge->head() != notThis) {
            return edge->head();
        }
    }
    return nullptr;
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

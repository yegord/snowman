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

#include "Graph.h"

#include <QTextStream>

#include <nc/common/make_unique.h>

#include "Edge.h"
#include "Region.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

Graph::Graph(): root_(nullptr) {}

Graph::~Graph() {}

Edge *Graph::createEdge(Node *tail, Node *head) {
    auto edge = std::make_unique<Edge>(tail, head);
    auto result = edge.get();

    edges_.push_back(std::move(edge));

    return result;
}

void Graph::print(QTextStream &out) const {
    out << *root();
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

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

#include "Dfs.h"

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/Unreachable.h>

#include "Edge.h"
#include "Region.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

Dfs::Dfs(const cflow::Region *region) {
    assert(region != nullptr);

    preordering_.reserve(region->nodes().size());
    postordering_.reserve(region->nodes().size());

    visit(region->entry());

    foreach (cflow::Node *node, region->nodes()) {
        if (find(node2color_, node) == WHITE) {
            visit(node);
        }
    }
}

void Dfs::visit(cflow::Node *node) {
    assert(node != nullptr);
    assert(find(node2color_, node) == WHITE);

    node2color_[node] = GRAY;
    preordering_.push_back(node);

    foreach (cflow::Edge *edge, node->outEdges()) {
        switch (find(node2color_, edge->head())) {
        case WHITE:
            edge2type_[edge] = FORWARD;
            visit(edge->head());
            break;
        case GRAY:
            edge2type_[edge] = BACK;
            break;
        case BLACK:
            edge2type_[edge] = CROSS;
            break;
        default:
            unreachable();
        }
    }

    node2color_[node] = BLACK;
    postordering_.push_back(node);
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

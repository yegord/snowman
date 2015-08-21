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

#include "Region.h"

#include <QTextStream>

#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>

#include "Edge.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

const BasicBlock *Region::getEntryBasicBlock() const {
    return entry()->getEntryBasicBlock();
}

bool Region::isCondition() const {
    return regionKind_ == COMPOUND_CONDITION;
}

void Region::print(QTextStream &out) const {
    out << "node" << this << " [shape=box,label=\"";

    switch (regionKind_) {
        case UNKNOWN:
            out << "UNKNOWN";
            break;
        case BLOCK:
            out << "BLOCK";
            break;
        case COMPOUND_CONDITION:
            out << "COMPOUND_CONDITION";
            break;
        case IF_THEN:
            out << "IF_THEN";
            break;
        case IF_THEN_ELSE:
            out << "IF_THEN_ELSE";
            break;
        case LOOP:
            out << "LOOP";
            break;
        case DO_WHILE:
            out << "DO_WHILE";
            break;
        case WHILE:
            out << "WHILE";
            break;
        case SWITCH:
            out << "SWITCH";
            break;
        default:
            unreachable();
    }

    out << "\"]" << endl;

    out << "subgraph cluster" << this << " {" << endl;
    foreach (const Node *node, nodes()) {
        out << *node;

        foreach (const Edge *edge, node->outEdges()) {
            out << "node" << edge->tail() << " -> node" << edge->head() << endl;
        }
    }
    out << '}' << endl;

    out << "node" << this << " -> node" << (entry() ? entry() : nodes()[0])
        << " [color=\"blue\" lhead=\"cluster" << this << "\"]" << endl;
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

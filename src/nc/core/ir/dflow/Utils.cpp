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

#include "Utils.h"

#include <boost/unordered_set.hpp>

#include <nc/common/Range.h>
#include <nc/common/Unreachable.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Terms.h>

#include "Dataflow.h"
#include "Value.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

const Term *getFirstCopy(const Term *term, const Dataflow &dataflow) {
    assert(term != nullptr);

    /* Terms that were already seen. */
    boost::unordered_set<const Term *> visited;

    do {
        visited.insert(term);

        if (term->isWrite()) {
            if (auto source = term->source()) {
                term = source;
            } else {
                break;
            }
        } else if (term->isRead()) {
            auto &definitions = dataflow.getDefinitions(term);

            if (definitions.chunks().size() == 1 &&
                definitions.chunks().front().location() == dataflow.getMemoryLocation(term) &&
                definitions.chunks().front().definitions().size() == 1)
            {
                term = definitions.chunks().front().definitions().front();
            } else {
                break;
            }
        } else {
            unreachable();
        }
    } while (!nc::contains(visited, term));

    return term;
}

bool isReturn(const Jump *jump, const Dataflow &dataflow) {
    assert(jump != nullptr);

    return isReturnAddress(jump->thenTarget(), dataflow) || isReturnAddress(jump->elseTarget(), dataflow);
}

bool isReturnAddress(const JumpTarget &target, const Dataflow &dataflow) {
    return target.address() && isReturnAddress(target.address(), dataflow);
}

bool isReturnAddress(const Term *term, const Dataflow &dataflow) {
    assert(term != nullptr);

    return dataflow.getValue(term)->isReturnAddress();
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

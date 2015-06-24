/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Patch.h"

#include <nc/common/Foreach.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Statement.h>

namespace nc {
namespace core {
namespace ir {
namespace calling {

Patch::~Patch() {}

void Patch::insertAfter(Statement *after) {
    assert(after != nullptr);

    while (!statements_.empty()) {
        insertedStatements_.push_back(after->basicBlock()->insertAfter(after, statements_.pop_back()));
    }
}

void Patch::remove() {
    foreach (auto statement, insertedStatements_) {
        statements_.push_front(statement->basicBlock()->erase(statement));
    }
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

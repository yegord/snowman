/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <nc/common/ilist.h>

namespace nc {
namespace core {
namespace ir {

class Statement;

namespace calling {

/**
 * A sequence of statements that can be inserted into a basic block
 * after a given statement and removed later.
 */
class Patch {
    nc::ilist<Statement> statements_;
    std::vector<Statement *> insertedStatements_;

public:
    ~Patch();

    /**
     * \return Statements of the patch.
     */
    nc::ilist<Statement> &statements() { return statements_; }

    /**
     * Inserts all statements of the patch in the basic block
     * of the given statement after the given statement.
     *
     * \param after Valid pointer to the statement after which to insert.
     */
    void insertAfter(Statement *after);

    /**
     * Removes all previously inserted statements from the basic
     * block where they were inserted.
     */
    void remove();
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Uses.h"

#include <nc/common/Foreach.h>

#include "Dataflow.h"

namespace nc {
namespace core {
namespace ir {
namespace dflow {

Uses::Uses(const Dataflow &dataflow) {
    foreach (auto &termAndDefinitions, dataflow.term2definitions()) {
        foreach (const auto &chunk, termAndDefinitions.second.chunks()) {
            foreach (const Term *definition, chunk.definitions()) {
                term2uses_[definition].push_back(Use(chunk.location(), termAndDefinitions.first));
            }
        }
    }
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

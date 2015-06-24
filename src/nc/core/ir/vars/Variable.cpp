/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Variable.h"

#include <numeric> /* std::accumulate */

namespace nc {
namespace core {
namespace ir {
namespace vars {

Variable::Variable(Scope scope, std::vector<TermAndLocation> termsAndLocations):
    scope_(scope), termsAndLocations_(std::move(termsAndLocations))
{
    assert(!termsAndLocations_.empty());

    memoryLocation_ = std::accumulate(termsAndLocations_.begin(), termsAndLocations_.end(), MemoryLocation(),
        [](const MemoryLocation &a, const TermAndLocation &b) {
            return MemoryLocation::merge(a, b.location);
    });
}

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

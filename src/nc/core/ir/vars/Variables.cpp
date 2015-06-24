/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Variables.h"

#include <nc/common/Foreach.h>

namespace nc {
namespace core {
namespace ir {
namespace vars {

void Variables::addVariable(std::unique_ptr<Variable> variable) {
    assert(variable != nullptr);

    foreach (const auto &termAndLocation, variable->termsAndLocations()) {
        assert(nc::find(term2variable_, termAndLocation.term) == nullptr);
        term2variable_[termAndLocation.term] = variable.get();
    }

    variables_.push_back(std::move(variable));
}

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

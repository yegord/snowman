/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "UndeclaredIdentifier.h"

#include "PrintContext.h"

namespace nc {
namespace core {
namespace likec {

void UndeclaredIdentifier::doPrint(PrintContext &context) const {
    context.out() << name_;
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

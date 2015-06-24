/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Symbol.h"

#include <QObject>

#include <nc/common/Unreachable.h>

namespace nc {
namespace core {
namespace image {

QString SymbolType::getName(Type type) {
    switch (type) {
        case NOTYPE:
            return tr("None");
        case FUNCTION:
            return tr("Function");
        case OBJECT:
            return tr("Object");
        case SECTION:
            return tr("Section");
    }
    unreachable();
}

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

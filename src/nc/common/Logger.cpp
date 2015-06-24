/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Logger.h"

#include <QObject>

namespace nc {

QString LogLevel::getName(Level level) {
    switch (level) {
        case DEBUG:
            return tr("Debug");
        case INFO:
            return tr("Info");
        case WARNING:
            return tr("Warning");
        case ERROR:
            return tr("Error");
    }
    return QString();
}

} // namespace nc

/* vim:set et sts=4 sw=4: */

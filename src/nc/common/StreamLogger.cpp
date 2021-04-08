/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "StreamLogger.h"

#include <QObject>

namespace nc {

void StreamLogger::log(LogLevel level, const QString &text) {
    stream_ << tr("[%1] %2").arg(level.getName()).arg(text) << '\n';
}

} // namespace nc

/* vim:set et sts=4 sw=4: */

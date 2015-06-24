/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "SignalLogger.h"

namespace nc {

void SignalLogger::log(LogLevel level, const QString &text) {
    Q_EMIT onMessage(tr("[%1] %2").arg(level.getName()).arg(text));
}

} // namespace nc

/* vim:set et sts=4 sw=4: */

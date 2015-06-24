/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QCoreApplication>
#include <QTextStream>

#include "Logger.h"

namespace nc {

/**
 * Logger printing messages to a stream.
 */
class StreamLogger: public nc::Logger {
    Q_DECLARE_TR_FUNCTIONS(StreamLogger)

    QTextStream &stream_;

public:
    /**
     * Constructor.
     *
     * \param stream Reference to the stream to print messages to.
     */
    StreamLogger(QTextStream &stream): stream_(stream) {}

    void log(LogLevel level, const QString &text) override;
};

} // namespace nc

/* vim:set et sts=4 sw=4: */

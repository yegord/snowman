/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <QCoreApplication>
#include <QString>

#include "LogLevel.h"

namespace nc {

/**
 * Log level.
 */
class LogLevel {
    Q_DECLARE_TR_FUNCTIONS(LogLevel)
public:
    /**
     * Log level values.
     */
    enum Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        LOWEST = DEBUG,
        HIGHEST = ERROR
    };

    /**
     * Constructor.
     *
     * \param level Log level value.
     */
    LogLevel(Level level): level_(level) {}

    /**
     * \return Log level value.
     */
    operator Level() const { return level_; }

    /**
     * \param level Log level value.
     *
     * \return Name of the given level.
     */
    static QString getName(Level level);

    /**
     * \return Name of the log level.
     */
    QString getName() const { return getName(level_); }

private:
    /** The value of the log level. */
    Level level_;
};

/**
 * The base class for a logger.
 *
 * Logger does the actual logging of messages.
 */
class Logger {
public:
    /**
     * Virtual destructor.
     */
    virtual ~Logger() {}

    /**
     * Logs a message with a given level.
     *
     * \param[in] level Log level of the message.
     * \param[in] text  Text of the message.
     */
    virtual void log(LogLevel level, const QString &text) = 0;
};

} // namespace nc

/* vim:set et sts=4 sw=4: */

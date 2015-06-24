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

#include <cassert>
#include <memory>

#include "Logger.h"

namespace nc {

/**
 * A copy-constructable class carrying a shared pointer to a logger.
 */
class LogToken {
    /** Logger. */
    std::shared_ptr<Logger> logger_;

public:
    /**
     * Default constructor.
     *
     * Creates a token that does not actually log anything.
     */
    LogToken() {}

    /**
     * Constructor creating a token logging messages via a given logger.
     *
     * \param logger Valid pointer to a logger.
     */
    LogToken(std::shared_ptr<Logger> logger):
        logger_(std::move(logger))
    {
        assert(logger_);
    }

    /**
     * Logs a message with a given level.
     *
     * \param[in] level Log level of the message.
     * \param[in] text  Text of the message.
     */
    void log(LogLevel level, const QString &text) const {
        if (logger_) {
            logger_->log(level, text);
        }
    }

    /**
     * Logs a message with the debug level.
     *
     * \param[in] text Text of the message.
     */
    void debug(const QString &text) const { log(LogLevel::DEBUG, text); }

    /**
     * Logs a message with the info level.
     *
     * \param[in] text Text of the message.
     */
    void info(const QString &text) const { log(LogLevel::INFO, text); }

    /**
     * Logs a message with the warning level.
     *
     * \param[in] text Text of the message.
     */
    void warning(const QString &text) const { log(LogLevel::WARNING, text); }

    /**
     * Logs a message with the error level.
     *
     * \param[in] text Text of the message.
     */
    void error(const QString &text) const { log(LogLevel::ERROR, text); }
};

} // namespace nc

/* vim:set et sts=4 sw=4: */

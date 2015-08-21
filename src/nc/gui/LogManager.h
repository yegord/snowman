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

#include <QObject>

namespace nc { namespace gui {

/**
 * Class for handling Qt debug messages.
 *
 * It converts the messages to Qt signals, so that LogView can subscribe to them.
 */
class LogManager: public QObject {
    Q_OBJECT

    /**
     * Private constructor.
     */
    LogManager() {}

public:
    /**
     * \return Valid pointer to a global unique instance of LogManager.
     */
    static LogManager *instance();

    /**
     * Qt debug message handler.
     * Propagates the message via the message() signal.
     *
     * \param type Message type.
     * \param msg Message text.
     */
    void log(QtMsgType type, const QString &msg);

    /**
     * Propagates given log message via the message() signal.
     *
     * \param text Message text.
     */
    void log(const QString &text);

Q_SIGNAL
    /**
     * A signal emitted when there is a new message to be shown.
     *
     * \param text Message text.
     */
    void message(const QString &text);
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

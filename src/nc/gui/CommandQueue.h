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

#include <QObject>

#include <memory>
#include <deque>

namespace nc {
namespace gui {

class Command;

/**
 * Command for executing a sequence of commands.
 */
class CommandQueue: public QObject {
    Q_OBJECT

    /** Command queue. */
    std::deque<std::unique_ptr<Command>> queue_;

    /** First element of the queue. */
    std::shared_ptr<Command> front_;

    public:

    /**
     * Constructor.
     *
     * \param parent Pointer to the parent widget. Can be nullptr.
     */
    CommandQueue(QObject *parent = nullptr);

    /**
     * Destructor.
     *
     * Cancels currently executing command.
     */
    ~CommandQueue();

    /**
     * Schedules execution of a given command.
     *
     * \param command Valid pointer to a command.
     */
    void push(std::unique_ptr<Command> command);

    /**
     * \return Pointer to the first element of the queue. Can be nullptr if the queue is empty.
     */
    Command *front() const { return front_.get(); }

    /**
     * \return True if the queue is empty, false otherwise.
     */
    bool empty() const { return front() == nullptr; }

    public Q_SLOTS:

    /**
     * Cancels currently executed command.
     */
    void cancel();

    /**
     * Cancels currently executed command and clears the queue.
     */
    void clear();

    Q_SIGNALS:

    /**
     * This signal is emitted just before the next command starts being executed.
     */
    void nextCommand();

    /**
     * This signal is emitted when the queue queue becomes empty.
     */
    void idle();

    private:

    /**
     * Executes the next instruction in the queue.
     */
    void executeNext();

    private Q_SLOTS:

    /**
     * Slot called when an activity is finished.
     */
    void commandFinished();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */


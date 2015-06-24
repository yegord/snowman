/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "CommandQueue.h"

#include <cassert>

#include "Command.h"

namespace nc {
namespace gui {

CommandQueue::CommandQueue(QObject *parent):
    QObject(parent)
{}

CommandQueue::~CommandQueue() {
    cancel();
}

void CommandQueue::push(std::unique_ptr<Command> command) {
    assert(command);

    queue_.push_back(std::move(command));
    executeNext();
}

void CommandQueue::cancel() {
    if (front()) {
        front()->cancel();
    }
}

void CommandQueue::clear() {
    if (!empty()) {
        cancel();
        front_.reset();
        queue_.clear();
        Q_EMIT idle();
    }
}

void CommandQueue::executeNext() {
    if (front()) {
        /* Some command is already being executed. */
        return;
    } else if (queue_.empty()) {
        /* No commands left. */
        Q_EMIT idle();
    } else {
        /* Pop the front command. */
        front_ = std::move(queue_.front());
        queue_.pop_front();

        /* Notify everybody. */
        Q_EMIT nextCommand();

        /* Make sure the command is not deleted while we are executing it. */
        auto command = front_;

        /* Execute it. */
        connect(command.get(), SIGNAL(finished()), this, SLOT(commandFinished()), Qt::QueuedConnection);
        command->execute();
    }
}

void CommandQueue::commandFinished() {
    assert(front_ != nullptr);
    front_.reset();
    executeNext();
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

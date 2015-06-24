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

#include "Command.h"

#ifdef NC_USE_THREADS
#include <QThreadPool>
#endif

#include "Activity.h"

namespace nc {
namespace gui {

Command::Command():
#ifdef NC_USE_THREADS
    threadPool_(QThreadPool::globalInstance()),
#endif
    activityCount_(0),
    isBackground_(false)
{}

Command::~Command() {
    cancel();
}

void Command::execute() {
    assert(!executing());

    cancellationToken_ = CancellationToken();

    ++activityCount_;
    work();
    activityFinished();
}

void Command::delegate(std::unique_ptr<Activity> activity) {
    assert(activity);

    connect(activity.get(), SIGNAL(finished()), this, SLOT(activityFinished()), Qt::QueuedConnection);

    ++activityCount_;

#ifdef NC_USE_THREADS
    activity->setAutoDelete(true);
    threadPool()->start(activity.release());
#else
    activity->run();
#endif
}

void Command::activityFinished() {
    assert(activityCount_ > 0);

    if (--activityCount_ == 0) {
        Q_EMIT finished();
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

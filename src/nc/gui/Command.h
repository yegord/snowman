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

#include <cassert>
#include <memory>

#include <nc/common/CancellationToken.h>

#ifdef NC_USE_THREADS
QT_BEGIN_NAMESPACE
class QThreadPool;
QT_END_NAMESPACE
#endif

namespace nc {
namespace gui {

class Activity;

/**
 * Base class for commands, as in Command Pattern.
 */
class Command: public QObject {
    Q_OBJECT

#ifdef NC_USE_THREADS
    /** Thread pool used for background activities. */
    QThreadPool *threadPool_;
#endif

    /** Number of running background activities. */
    std::size_t activityCount_;

    /** Cancellation token for this command. */
    CancellationToken cancellationToken_;

    /** The command does not prevent the user from doing something else. */
    bool isBackground_;

    public:

    /**
     * Constructor.
     */
    Command();

    /**
     * Destructor.
     *
     * Cancels execution of the command (this is important for commands creating background activities).
     */
    ~Command();

#ifdef NC_USE_THREADS
    /**
     * \return Thread pool used for background activities.
     *
     * By default, the global QThreadPool instance is used.
     */
    QThreadPool *threadPool() const { return threadPool_; }

    /**
     * Sets the thread pool for background activities.
     *
     * \param threadPool Valid pointer to the thread pool.
     */
    void setThreadPool(QThreadPool *threadPool) { assert(threadPool); threadPool_ = threadPool; }
#endif

    /**
     * Executes the command.
     */
    void execute();

    /**
     * \return True if the command is being executed, false otherwise.
     */
    bool executing() const { return activityCount_ > 0; }

    /**
     * Cancels execution of this command.
     */
    void cancel() { cancellationToken_.cancel(); }

    /**
     * \return True if the command was canceled, false otherwise.
     */
    bool canceled() const { return cancellationToken_.cancellationRequested(); }

    /**
     * \return True if the command does not prevent the user from doing something else,
     *         false (default) otherwise.
     */
    bool isBackground() const { return isBackground_; }

    Q_SIGNALS:

    /**
     * Signal emitted when the execution of this command and all its activities is finished.
     */
    void finished();

    protected:

    /**
     * This function does the real work.
     * Override it in subclasses.
     */
    virtual void work() = 0;

    /**
     * Adds a background activity.
     * The activity is executed by the thread pool.
     *
     * \param activity Valid pointer to an activity.
     */
    void delegate(std::unique_ptr<Activity> activity);

    /**
     * \return Cancellation token for this command.
     */
    const CancellationToken &cancellationToken() const { return cancellationToken_; }

    /**
     * Sets whether the command prevents the user from doing something else.
     *
     * \param value True if it doesn't, false if it does.
     */
    void setBackground(bool value) { isBackground_ = value; }

    private Q_SLOTS:

    /**
     * This slot is called when an activity is finished.
     */
    void activityFinished();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

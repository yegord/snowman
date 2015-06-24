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

#include <boost/unordered_map.hpp>

#include <nc/common/Branding.h>
#include <nc/common/Types.h>

#include "IdaPlugin.h"

QT_BEGIN_NAMESPACE
class QApplication;
class QWidget;
QT_END_NAMESPACE

namespace nc { namespace gui {
    class MainWindow;
    class Project;
}}

namespace nc { namespace ida {

/**
 * Decompiler IDA plugin.
 */
class DecompilerPlugin: public QObject, public IdaPlugin {
    Q_OBJECT

    Branding branding_; 

public:
    DecompilerPlugin();

    virtual ~DecompilerPlugin();

    /**
     * Decompiles the function under cursor.
     */
    void decompileFunction();

    /**
     * Decompiles the whole program.
     */
    void decompileProgram();

private:
    std::unique_ptr<QApplication> mApplication;

    /** Mapping from a function's entry address to corresponding main window. */
    boost::unordered_map<ByteAddr, gui::MainWindow *> function2window_;

    /** Mapping from a window to the function's entry address. */
    boost::unordered_map<gui::MainWindow *, ByteAddr> window2function_;

    /** Mapping from a main window to the tab QWidget of IDA. */
    boost::unordered_map<gui::MainWindow *, QWidget *> window2widget_;

    /** Main window decompiliing the whole program. */
    gui::MainWindow *programWindow_;

    /**
     * Creates a main window inside an IDA tab widget.
     *
     * \return Valid pointer to the created main window, owned by IDA.
     */
    gui::MainWindow *createWindow();

    /**
     * Shows/activates the main window.
     *
     * \return window Valid pointer to the main window.
     */
    void activateWindow(gui::MainWindow *window);

    private Q_SLOTS:

    /**
     * Slot handling destruction of a main window (owned by IDA).
     *
     * \param object Valid pointer to the main window object that was destroyed.
     */
    void windowDestroyed(QObject *object);

    private:

    /**
     * Creates a project for decompiling (a part of) the program opened in IDA.
     * \return Valid pointer to the project.
     */
    std::unique_ptr<gui::Project> createIdaProject() const;
};

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */

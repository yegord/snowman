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

#include <nc/config.h>

#include <QObject>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

namespace nc {

namespace gui {
    class MainWindow;
}

namespace ida {

/**
 * This class adds possibility of jumping from instructions and C++ views to IDA views.
 */
class NavigationHelper: public QObject {
    Q_OBJECT

    /** Main window being cared about. */
    gui::MainWindow *mainWindow_;

    /** Action to jump from instructions view to IDA. */
    QAction *jumpFromInstructionsViewAction_;

    /** Action to jump from C++ view to IDA. */
    QAction *jumpFromCxxViewAction_;

    public:

    /**
     * Constructor.
     *
     * \param mainWindow Main window to add navigation to.
     */
    NavigationHelper(gui::MainWindow *mainWindow);

    public Q_SLOTS:

    /**
     * Populates context menu of instructions view with the action
     * of jumping to IDA.
     *
     * \param menu Valid pointer to the context menu.
     */
    void populateInstructionsContextMenu(QMenu *menu);

    /**
     * Populates context menu of C++ view with the action
     * of jumping to IDA.
     *
     * \param menu Valid pointer to the context menu.
     */
    void populateCxxContextMenu(QMenu *menu);

    /**
     * This slot is called when the user asks to jump from instructions view to IDA.
     */
    void jumpFromInstructionsView();

    /**
     * This slot is called when the user asks to jump from C++ view to IDA.
     */
    void jumpFromCxxView();
};

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */

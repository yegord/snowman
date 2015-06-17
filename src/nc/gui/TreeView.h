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

#include <QDockWidget>

#include <memory>

QT_BEGIN_NAMESPACE
class QMenu;
class QTreeView;
QT_END_NAMESPACE

namespace nc { namespace gui {

/**
 * Dock widget for showing trees or lists.
 */
class TreeView: public QDockWidget {
    Q_OBJECT

    QTreeView *treeView_;
    QAction *copyAction_;
    QAction *openSearchAction_;
    QAction *findNextAction_;
    QAction *findPreviousAction_;
    QAction *selectFontAction_;

public:
    /**
     * Class constructor.
     *
     * \param[in] title     Title of the widget.
     * \param[in] parent    Parent widget.
     */
    TreeView(const QString &title, QWidget *parent = nullptr);

    /**
     * \return Valid pointer to the tree widget.
     */
    QTreeView *treeView() const { return treeView_; }

public Q_SLOTS:
    /**
     * Copies currently selected text in the clipboard.
     */
    void copy();

public:
    /**
     * \return The font used for showing the text document.
     */
    const QFont &documentFont() const;

public Q_SLOTS:
    /**
     * Makes the default font of text edit delta points larger.
     *
     * \param delta Points size difference.
     */
    void zoomIn(int delta = 1);

    /**
     * Makes the default font of text edit delta points smaller.
     *
     * \param delta Points size difference.
     */
    void zoomOut(int delta = 1);

    /**
     * Sets the font used for showing the text document.
     */
    void setDocumentFont(const QFont &font);

    /**
     * Lets the user select the font used for showing the document.
     */
    void selectFont();

Q_SIGNALS:
    /**
     * This signal is emitted when a context menu is being created.
     * Intercept this signal to populate the menu with some actions.
     *
     * \param menu Valid pointer to the context menu being created.
     */
    void contextMenuCreated(QMenu *menu);

private Q_SLOTS:
    /**
     * Shows a context menu for the child tree widget.
     *
     * \param pos Position at which the menu is requested.
     */
    void showContextMenu(const QPoint &pos);

    /**
     * Populates the context menu being created.
     *
     * \param menu Valid pointer to the menu being created.
     */
    void populateContextMenu(QMenu *menu);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

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
#include <QEvent>

#include <vector>

#include <nc/common/RangeClass.h>

QT_BEGIN_NAMESPACE
class QMenu;
class QPlainTextEdit;
class QTextDocument;
QT_END_NAMESPACE

namespace nc { namespace gui {

/**
 * Dock widget for showing text.
 */
class TextView: public QDockWidget {
    Q_OBJECT

    QPlainTextEdit *textEdit_;
    QAction *saveAsAction_;
    QAction *openSearchAction_;
    QAction *findNextAction_;
    QAction *findPreviousAction_;
    QAction *openGotoLineAction_;
    QAction *selectFontAction_;
    std::vector<Range<int>> highlighting_;

public:
    /**
     * Class constructor.
     *
     * \param[in] title     Title of the widget.
     * \param[in] parent    Parent widget.
     */
    explicit TextView(const QString &title, QWidget *parent = 0);

    /**
     * \return Valid pointer to the text widget.
     */
    QPlainTextEdit *textEdit() const { return textEdit_; }

    /**
     * Sets the document shown in the text edit widget.
     *
     * \param document Pointer to the document. Can be nullptr.
     *
     * Use this method instead of QPlainTextEdit::setDocument() in order to
     * keep the font configuration intact.
     */
    void setDocument(QTextDocument *document);

public Q_SLOTS:
    /**
     * Highlights text ranges.
     *
     * \param[in] ranges        Vector of ranges to be highlighted.
     * \param[in] ensureVisible Ensure that changes in highlighting are visible.
     */
    void highlight(std::vector<Range<int>> ranges, bool ensureVisible = true);

    /**
     * Moves cursor to given position.
     *
     * \param[in] position      Position.
     * \param[in] ensureVisible Ensure that the cursor is visible.
     */
    void moveCursor(int position, bool ensureVisible = true);

    /**
     * Lets the user choose a file name and saves the text into this file.
     */
    void saveAs();

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

public:
    /**
     * \return The font used for showing the text document.
     */
    const QFont &documentFont() const;

public Q_SLOTS:
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

    /**
     * This signal is emitted when the widget has a message to be displayed in status bar.
     *
     * \param message Message to be displayed.
     */
    void status(const QString &message = QString());

private Q_SLOTS:
    /**
     * Updates extra selections in the textView() to show highlighted ranges.
     */
    void updateExtraSelections();

    /**
     * Generates a status signal reporting current position.
     */
    void updatePositionStatus();

    /**
     * Shows a context menu for the child QTextEdit widget.
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

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

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QStringListModel;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace nc { namespace gui {

/**
 * Widget providing the functionality of moving to the line with a given number in a QPlainTextEdit.
 */
class GotoLineWidget: public QWidget {
    Q_OBJECT

    public:

    /**
     * Constructor.
     *
     * \param[in] textEdit  Valid pointer to the associated QPlainTextEdit instance.
     * \param[in] parent    Pointer to the parent widget. Can be nullptr.
     */
    explicit GotoLineWidget(QPlainTextEdit *textEdit, QWidget *parent = nullptr);

    /**
     * \return Valid pointer to the associated QPlainTextEdit instance.
     */
    QPlainTextEdit *textEdit() const { return textEdit_; }

    public Q_SLOTS:

    /**
     * Shows the widget and sets input focus into it.
     */
    void activate();

    /**
     * Hides the widget.
     */
    void deactivate();

    private Q_SLOTS:

    /**
     * Moves the cursor in the associated QPlainTextEdit to the line with the entered number.
     */
    void go();

    /**
     * Indicates the status of goto operation.
     *
     * \param success Whether goto was successful.
     */
    void indicateStatus(bool success = true);

    /**
     * Remembers currently entered line number for completion.
     */
    void rememberCompletion();

    private:

    /** Associated QPlainTextEdit instance. */
    QPlainTextEdit *textEdit_;

    /** Input for entering a line number. */
    QLineEdit *lineEdit_;

    /** Completion model for the line number. */
    QStringListModel *completionModel_;

    /** Palette of the line edit in a normal state. */
    QPalette normalPalette_;

    /** Palette of the line edit to let user know that the string was not found. */
    QPalette notFoundPalette_;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

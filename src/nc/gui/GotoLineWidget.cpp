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

#include "GotoLineWidget.h"

#include <QCompleter>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStringListModel>
#include <QTextBlock>
#include <QPlainTextEdit>

namespace nc { namespace gui {

GotoLineWidget::GotoLineWidget(QPlainTextEdit *textEdit, QWidget *parent):
    QWidget(parent), textEdit_(textEdit)
{
    QLabel *goLabel = new QLabel(tr("Go to line:"), this);

    completionModel_ = new QStringListModel(this);

    QIntValidator *validator = new QIntValidator(this);
    validator->setBottom(1);

    lineEdit_ = new QLineEdit(this);
    lineEdit_->setCompleter(new QCompleter(completionModel_, this));
    lineEdit_->setValidator(validator);

    connect(lineEdit_, SIGNAL(textChanged(const QString &)), this, SLOT(indicateStatus()));
    connect(lineEdit_, SIGNAL(returnPressed()), this, SLOT(go()));

    QPushButton *goButton = new QPushButton(tr("&Go"), this);
    connect(goButton, SIGNAL(clicked()), this, SLOT(go()));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 4, 4);
    layout->addWidget(goLabel);
    layout->addWidget(lineEdit_);
    layout->addWidget(goButton);

    normalPalette_   = lineEdit_->palette();
    notFoundPalette_ = lineEdit_->palette();
    notFoundPalette_.setColor(QPalette::Base, QColor(255, 192, 192));
}

void GotoLineWidget::activate() {
    show();
    lineEdit_->selectAll();
    lineEdit_->setFocus();
}

void GotoLineWidget::deactivate() {
    if (!isVisible()) {
        return;
    }

    textEdit()->setFocus();
    hide();
}

void GotoLineWidget::go() {
    QTextBlock block;

    if (textEdit()->document()) {
        block = textEdit()->document()->findBlockByNumber(lineEdit_->text().toInt() - 1);
    }

    if (block.isValid()) {
        QTextCursor cursor(block);
        textEdit()->setTextCursor(cursor);
        textEdit()->ensureCursorVisible();

        indicateStatus(true);

        deactivate();
    } else {
        indicateStatus(false);
    }

    rememberCompletion();
}

void GotoLineWidget::indicateStatus(bool success) {
    lineEdit_->setPalette(success ? normalPalette_ : notFoundPalette_);
}

void GotoLineWidget::rememberCompletion() {
    QStringList list = completionModel_->stringList();
    if (!list.contains(lineEdit_->text(), lineEdit_->completer()->caseSensitivity())) {
        list.append(lineEdit_->text());
        completionModel_->setStringList(list);
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

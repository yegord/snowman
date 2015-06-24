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

#include "TextEditSearcher.h"

#include <cassert>

#include <QPlainTextEdit>
#include <QScrollBar>

namespace nc { namespace gui {

TextEditSearcher::TextEditSearcher(QPlainTextEdit *textEdit):
    textEdit_(textEdit), hvalue_(-1), vvalue_(-1)
{
    assert(textEdit != nullptr);
}

void TextEditSearcher::startTrackingViewport() {
    connect(textEdit_, SIGNAL(cursorPositionChanged()), this, SLOT(rememberViewport()));
}

void TextEditSearcher::stopTrackingViewport() {
    disconnect(textEdit_, SIGNAL(cursorPositionChanged()), this, SLOT(rememberViewport()));
}

void TextEditSearcher::rememberViewport() {
    cursor_ = textEdit_->textCursor();
    hvalue_ = textEdit_->horizontalScrollBar()->value();
    vvalue_ = textEdit_->verticalScrollBar()->value();
}

void TextEditSearcher::restoreViewport() {
    if (hvalue_ == -1) {
        return;
    }

    textEdit_->setTextCursor(cursor_);
    textEdit_->horizontalScrollBar()->setValue(hvalue_);
    textEdit_->verticalScrollBar()->setValue(vvalue_);
}

Searcher::FindFlags TextEditSearcher::supportedFlags() const {
    return FindBackward | FindCaseSensitive | FindWholeWords;
}

bool TextEditSearcher::find(const QString &expression, FindFlags flags) {
    if (expression.isEmpty()) {
        return true;
    }

    auto options = QTextDocument::FindFlags();

    if (flags & FindBackward) {
        options |= QTextDocument::FindBackward;
    }
    if (flags & FindCaseSensitive) {
        options |= QTextDocument::FindCaseSensitively;
    }
    if (flags & FindWholeWords) {
        options |= QTextDocument::FindWholeWords;
    }

    if (textEdit_->find(expression, options)) {
        return true;
    } else {
        QTextCursor cursor = textEdit_->textCursor();
        cursor.movePosition((flags & FindBackward) ? QTextCursor::End : QTextCursor::Start);
        textEdit_->setTextCursor(cursor);

        return textEdit_->find(expression, options);
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

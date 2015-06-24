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

#include "LogView.h"

#include <QPlainTextEdit>

#include "LogManager.h"

namespace nc { namespace gui {

LogView::LogView(QWidget *parent):
    TextView(tr("Log"), parent)
{
    /* Limit log length. */
    textEdit()->document()->setMaximumBlockCount(10000);

    /* Log Qt messages here. */
    connect(LogManager::instance(), SIGNAL(message(const QString &)), this, SLOT(log(const QString &)), Qt::QueuedConnection);
}

void LogView::log(const QString &text) {
    textEdit()->appendPlainText(text);
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

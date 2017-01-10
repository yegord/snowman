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

#include "CxxDocument.h"

#include <QPlainTextDocumentLayout>
#include <QTextStream>

#include <nc/core/Context.h>

#include <nc/core/ir/Statement.h>
#include <nc/core/ir/Term.h>

#include <nc/core/likec/Expression.h>
#include <nc/core/likec/FunctionDeclaration.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/FunctionIdentifier.h>
#include <nc/core/likec/LabelDeclaration.h>
#include <nc/core/likec/LabelIdentifier.h>
#include <nc/core/likec/LabelStatement.h>
#include <nc/core/likec/Statement.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/VariableDeclaration.h>
#include <nc/core/likec/VariableIdentifier.h>

#include <nc/common/RangeTreeBuilder.h>
#include <nc/core/likec/CxxDocument.h>

namespace nc { namespace gui {

CxxDocument::CxxDocument(QObject *parent, std::shared_ptr<const core::Context> context):
    QTextDocument(parent),
    core::likec::CxxDocument(context.get()),
    context_(std::move(context))
{
    setDocumentLayout(new QPlainTextDocumentLayout(this));

    if (context_ && context_->tree()) {
        setPlainText(core::likec::CxxDocument::getText());
    }

    connect(this, SIGNAL(contentsChange(int, int, int)), this, SLOT(onContentsChange(int, int, int)));
}

void CxxDocument::onContentsChange(int position, int charsRemoved, int charsAdded) {
    if (charsRemoved > 0) {
        rangeTree_.handleRemoval(position, charsRemoved);
    }
    if (charsAdded > 0) {
        rangeTree_.handleInsertion(position, charsAdded);
    }
}

void CxxDocument::rename(const core::likec::Declaration *declaration, const QString &newName) {
    assert(declaration != nullptr);

    foreach (auto use, getUses(declaration)) {
        replaceText(getRange(use), newName);
    }
}

QString CxxDocument::getText(const Range<int> &range) const {
    QTextCursor cursor(const_cast<CxxDocument *>(this));
    cursor.setPosition(range.start());
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, range.length());
    return cursor.selectedText();
}

void CxxDocument::replaceText(const Range<int> &range, const QString &text) {
    QTextCursor cursor(this);
    cursor.beginEditBlock();
    cursor.setPosition(range.end());
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, range.length());
    cursor.removeSelectedText();
    cursor.insertText(text);
    cursor.endEditBlock();
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

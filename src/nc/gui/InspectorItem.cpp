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

#include "InspectorItem.h"

#include <nc/common/CheckedCast.h>

namespace nc {
namespace gui {

void InspectorItem::addComment(const QString &comment) {
    if (text_.isEmpty()) {
        setText(comment);
    } else {
        setText(QString("%1 (%2)").arg(text_).arg(comment));
    }
}

InspectorItem *InspectorItem::addChild(InspectorItem *item) {
    item->parent_ = this;
    item->row_ = checked_cast<int>(children_.size());
    children_.push_back(std::unique_ptr<InspectorItem>(item));
    return item;
}

InspectorItem *InspectorItem::addChild(const QString &text) {
    return addChild(new InspectorItem(text));
}

InspectorItem *InspectorItem::addChild(const QString &text, const core::likec::TreeNode *node) {
    std::unique_ptr<InspectorItem> child(new InspectorItem(text));
    child->setNode(node);
    return addChild(child.release());
}

InspectorItem *InspectorItem::addChild(const QString &text, const core::ir::Term *term) {
    std::unique_ptr<InspectorItem> child(new InspectorItem(text));
    child->setTerm(term);
    return addChild(child.release());
}

InspectorItem *InspectorItem::addChild(const QString &text, const core::ir::Statement *statement) {
    std::unique_ptr<InspectorItem> child(new InspectorItem(text));
    child->setStatement(statement);
    return addChild(child.release());
}

InspectorItem *InspectorItem::addChild(const QString &text, const core::arch::Instruction *instruction) {
    std::unique_ptr<InspectorItem> child(new InspectorItem(text));
    child->setInstruction(instruction);
    return addChild(child.release());
}

InspectorItem *InspectorItem::addChild(const QString &text, const core::likec::Type *type) {
    std::unique_ptr<InspectorItem> child(new InspectorItem(text));
    child->setType(type);
    return addChild(child.release());
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

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

#include "InstructionsView.h"

#include <QTreeView>

#include <nc/common/Foreach.h>

#include "InstructionsModel.h"

namespace nc { namespace gui {

InstructionsView::InstructionsView(QWidget *parent):
    TreeView(tr("Instructions"), parent),
    model_(nullptr)
{
    treeView()->setHeaderHidden(true);
    treeView()->setItemsExpandable(false);
    treeView()->setRootIsDecorated(false);
    treeView()->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView()->setUniformRowHeights(true);
}

void InstructionsView::setModel(InstructionsModel *model) {
    if (model != model_) {
        model_ = model;
        treeView()->setModel(model);

        connect(treeView()->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this, SLOT(updateSelection()));
        updateSelection();
    }
}

void InstructionsView::updateSelection() {
    std::vector<const core::arch::Instruction *> instructions;

    if (model()) {
        foreach (const QModelIndex &index, treeView()->selectionModel()->selectedIndexes()) {
            /* Process every row only once. */
            if (index.column() == 0) {
                if (const core::arch::Instruction *instruction = model()->getInstruction(index)) {
                    instructions.push_back(instruction);
                }
            }
        }
    }

    if (selectedInstructions_ != instructions) {
        selectedInstructions_.swap(instructions);
        Q_EMIT instructionSelectionChanged();
    }
}

void InstructionsView::highlightInstructions(const std::vector<const core::arch::Instruction *> &instructions, bool ensureVisible) {
    if (model()) {
        model()->setHighlightedInstructions(instructions);

        if (ensureVisible && !instructions.empty()) {
            treeView()->scrollTo(model()->getIndex(instructions.back()));
        }
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

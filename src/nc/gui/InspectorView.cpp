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

#include "InspectorView.h"

#include <QKeyEvent>
#include <QTreeView>

#include <nc/common/Foreach.h>

#include "InspectorItem.h"
#include "InspectorModel.h"

namespace nc {
namespace gui {

InspectorView::InspectorView(QWidget *parent):
    QDockWidget(tr("Inspector"), parent),
    model_(nullptr)
{
    treeView_ = new QTreeView(this);
    treeView_->setHeaderHidden(true);
    treeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView_->installEventFilter(this);

    setWidget(treeView_);
}

InspectorView::~InspectorView() {}

bool InspectorView::eventFilter(QObject *watched, QEvent *event) {
    if (watched == treeView_) {
        /* Forbid expanding all children. */
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            return keyEvent->key() == Qt::Key_Asterisk;
        }
    }
    return QDockWidget::eventFilter(watched, event);
}

void InspectorView::setModel(InspectorModel *model) {
    if (model_ != model) {
        treeView_->blockSignals(true);

        treeView_->setModel(model);
        model_ = model;

        treeView_->blockSignals(false);

        connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this, SLOT(updateSelection()));

        updateSelection();
    }
}

void InspectorView::updateSelection() {
    std::vector<const core::likec::TreeNode *> nodes;
    std::vector<const core::arch::Instruction *> instructions;

    if (model()) {
        foreach (const QModelIndex &index, treeView_->selectionModel()->selectedIndexes()) {
            auto item = model()->getItem(index);
            if (item->node()) {
                nodes.push_back(item->node());
            }
            if (item->instruction()) {
                instructions.push_back(item->instruction());
            }
        }
    }

    if (selectedNodes_ != nodes) {
        selectedNodes_.swap(nodes);
        Q_EMIT nodeSelectionChanged();
    }
    if (selectedInstructions_ != instructions) {
        selectedInstructions_.swap(instructions);
        Q_EMIT instructionSelectionChanged();
    }
}

namespace {

template<class T>
InspectorItem *findDescendant(InspectorItem *item, int maxDepth, T match) {
    if (match(item)) {
        return item;
    }
    if (maxDepth > 0) {
        --maxDepth;
        foreach (const auto &child, item->children()) {
            if (InspectorItem *result = findDescendant(child.get(), maxDepth, match)) {
                return result;
            }
        }
    }
    return nullptr;
}

} // anonymous namespace

void InspectorView::highlightNodes(const std::vector<const core::likec::TreeNode *> &nodes) {
    if (!model()) {
        return;
    }

    disconnect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
               this, SLOT(updateSelection()));

    treeView_->selectionModel()->clearSelection();

    QModelIndex index;

    foreach (const core::likec::TreeNode *node, nodes) {
        std::vector<const core::likec::TreeNode *> path;

        while (node != nullptr) {
            path.push_back(node);
            node = model()->getParent(node);
        }

        InspectorItem *item = model()->root();
        reverse_foreach(const core::likec::TreeNode *node, path) {
            model()->expand(item);
            item = findDescendant(item, 2, [node](InspectorItem *x) { return x->node() == node; } );
            if (!item) {
                break;
            }
        }

        if (item) {
            index = model()->getIndex(item);

            for (QModelIndex parent = index; parent.isValid(); parent = model()->parent(parent)) {
                treeView_->expand(parent);
            }
            treeView_->selectionModel()->select(index, QItemSelectionModel::Select);
        }
    }

    treeView_->scrollTo(index);

    connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(updateSelection()));

    updateSelection();
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

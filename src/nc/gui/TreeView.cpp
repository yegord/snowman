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

#include "TreeView.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QFontDialog>
#include <QMenu>
#include <QTreeView>
#include <QVBoxLayout>
#include <QtAlgorithms>
#include <QWheelEvent>

#include <nc/common/make_unique.h>
#include <nc/common/Foreach.h>

#include "SearchWidget.h"
#include "TreeViewSearcher.h"

namespace nc { namespace gui {

TreeView::TreeView(const QString &title, QWidget *parent):
    QDockWidget(title, parent)
{
    treeView_ = new QTreeView(this);

    treeView_->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_->viewport()->installEventFilter(this);

    connect(treeView_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
    connect(this, SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateContextMenu(QMenu *)));

    auto searchWidget = new SearchWidget(std::make_unique<TreeViewSearcher>(treeView_), this);
    searchWidget->hide();

    QWidget *widget = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(QMargins());
    layout->addWidget(treeView_);
    layout->addWidget(searchWidget);

    setWidget(widget);

    QList<QKeySequence> searchShortcuts;
    searchShortcuts.append(QKeySequence::Find);
    searchShortcuts.append(tr("/"));

    copyAction_ = new QAction(tr("Copy"), this);
    copyAction_->setShortcut(QKeySequence::Copy);
    copyAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    treeView()->addAction(copyAction_);

    connect(copyAction_, SIGNAL(triggered()), this, SLOT(copy()));

    openSearchAction_ = new QAction(tr("Find..."), this);
    openSearchAction_->setShortcuts(searchShortcuts);
    openSearchAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(openSearchAction_);
    connect(openSearchAction_, SIGNAL(triggered()), searchWidget, SLOT(activate()));

    findNextAction_ = new QAction(tr("Find Next"), this);
    findNextAction_->setShortcut(QKeySequence::FindNext);
    findNextAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(findNextAction_);
    connect(findNextAction_, SIGNAL(triggered()), searchWidget, SLOT(findNext()));

    findPreviousAction_ = new QAction(tr("Find Previous"), this);
    findPreviousAction_->setShortcut(QKeySequence::FindPrevious);
    findPreviousAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(findPreviousAction_);
    connect(findPreviousAction_, SIGNAL(triggered()), searchWidget, SLOT(findPrevious()));

    QAction *closeEverythingAction = new QAction(this);
    closeEverythingAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    closeEverythingAction->setShortcut(Qt::Key_Escape);
    addAction(closeEverythingAction);

    connect(closeEverythingAction, SIGNAL(triggered()), searchWidget, SLOT(deactivate()));
    connect(closeEverythingAction, SIGNAL(triggered()), treeView_, SLOT(setFocus()));

    selectFontAction_ = new QAction(tr("Select Font..."), this);
    addAction(selectFontAction_);

    connect(selectFontAction_, SIGNAL(triggered()), this, SLOT(selectFont()));
}

void TreeView::showContextMenu(const QPoint &pos) {
    auto menu = std::make_unique<QMenu>();

    Q_EMIT contextMenuCreated(menu.get());

    if (!menu->isEmpty()) {
        menu->exec(treeView()->viewport()->mapToGlobal(pos));
    }
}

void TreeView::populateContextMenu(QMenu *menu) {
    if (!treeView_->selectionModel()) {
        return;
    }
    if (!treeView_->selectionModel()->selectedIndexes().isEmpty()) {
        menu->addSeparator();
        menu->addAction(copyAction_);
    }

    menu->addSeparator();
    menu->addAction(tr("Select All"), treeView(), SLOT(selectAll()), QKeySequence::SelectAll);
    menu->addSeparator();
    menu->addAction(openSearchAction_);
    menu->addAction(findNextAction_);
    menu->addAction(findPreviousAction_);
    menu->addSeparator();
    menu->addAction(selectFontAction_);
    menu->addSeparator();
}

void TreeView::copy() {
    auto indexes = treeView()->selectionModel()->selectedIndexes();

    if (indexes.isEmpty()) {
        return;
    }

    qSort(indexes.begin(), indexes.end(), [](const QModelIndex &a, const QModelIndex &b) -> bool {
        if (a.parent() == b.parent()) {
            return a.row() < b.row() || (a.row() == b.row() && a.column() < b.column());
        } else {
            return a < b;
        }
    });

    QString text;

    QModelIndex last;
    foreach (const auto &index, indexes) {
        if (last.parent() != index.parent() || last.row() != index.row()) {
            if (!text.isEmpty()) {
                text += '\n';
            }
            last = index;
        } else {
            text += '\t';
        }
        text += index.data().toString();
    }

    QApplication::clipboard()->setText(text);
}

void TreeView::zoomIn(int delta) {
    QFont font = documentFont();
    font.setPointSize(std::max(font.pointSize() + delta, 1));
    setDocumentFont(font);
}

void TreeView::zoomOut(int delta) {
    zoomIn(-delta);
}

const QFont &TreeView::documentFont() const {
    return treeView()->font();
}

void TreeView::setDocumentFont(const QFont &font) {
    treeView()->setFont(font);
}

void TreeView::selectFont() {
    setDocumentFont(QFontDialog::getFont(nullptr, documentFont(), this));
}

bool TreeView::eventFilter(QObject *watched, QEvent *event) {
    if (watched == treeView()->viewport()) {
        if (event->type() == QEvent::Wheel) {
            auto wheelEvent = static_cast<QWheelEvent *>(event);

            if (wheelEvent->orientation() == Qt::Vertical && wheelEvent->modifiers() & Qt::ControlModifier) {
                if (wheelEvent->delta() > 0) {
                    zoomIn(1 + wheelEvent->delta() / 360);
                } else {
                    zoomOut(1 - wheelEvent->delta() / 360);
                }
                return true;
            }
        }
    }
    return QDockWidget::eventFilter(watched, event);
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

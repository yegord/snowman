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

#include "TreeViewSearcher.h"

#include <cassert>

#include <QScrollBar>
#include <QTreeView>

#include <nc/common/Foreach.h>

namespace nc { namespace gui {

TreeViewSearcher::TreeViewSearcher(QTreeView *treeView):
    treeView_(treeView), hvalue_(-1), vvalue_(-1)
{
    assert(treeView != nullptr);
}

void TreeViewSearcher::startTrackingViewport() {
    if (treeView_->selectionModel()) {
        connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this, SLOT(rememberViewport()));
    }
}

void TreeViewSearcher::stopTrackingViewport() {
    if (treeView_->selectionModel()) {
        disconnect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(rememberViewport()));
    }
}

void TreeViewSearcher::rememberViewport() {
    if (!treeView_->selectionModel()) {
        hvalue_ = vvalue_ = -1;
        selectedIndexes_.clear();
        currentIndex_ = QModelIndex();
        return;
    }

    selectedIndexes_ = treeView_->selectionModel()->selectedIndexes();
    currentIndex_ = treeView_->selectionModel()->currentIndex();
    hvalue_ = treeView_->horizontalScrollBar()->value();
    vvalue_ = treeView_->verticalScrollBar()->value();
}

void TreeViewSearcher::restoreViewport() {
    if (hvalue_ == -1 || !treeView_->selectionModel()) {
        return;
    }

    treeView_->setCurrentIndex(currentIndex_);

    treeView_->selectionModel()->blockSignals(true);
    treeView_->selectionModel()->clearSelection();
    foreach (const auto &index, selectedIndexes_) {
        treeView_->selectionModel()->select(index, QItemSelectionModel::Select);
    }
    treeView_->selectionModel()->blockSignals(false);

    /* Trigger TreeView update. */
    treeView_->selectionModel()->select(QModelIndex(), QItemSelectionModel::NoUpdate);

    treeView_->horizontalScrollBar()->setValue(hvalue_);
    treeView_->verticalScrollBar()->setValue(vvalue_);
}

Searcher::FindFlags TreeViewSearcher::supportedFlags() const {
    return FindBackward | FindCaseSensitive | FindRegexp;
}

namespace {

bool match(const QModelIndex &index, const QString &expression, Searcher::FindFlags flags) {
    Qt::CaseSensitivity caseSensitivity = flags & Searcher::FindCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    auto data = index.data().toString();

    if (flags & Searcher::FindRegexp) {
        return data.contains(QRegExp(expression, caseSensitivity));
    } else {
        return data.contains(expression, caseSensitivity);
    }
}

QModelIndex findFirst(const QModelIndex &start, const QString &expression, Searcher::FindFlags flags) {
    if (!start.isValid()) {
        return QModelIndex();
    }

    auto model = start.model();
    auto parent = start.parent();

    int rowCount = model->rowCount(parent);
    int columnCount = model->columnCount(parent);

    if (!(flags & Searcher::FindBackward)) {
        /* Process the end of the same row. */
        for (int column = start.column() + 1; column < columnCount; ++column) {
            auto index = model->index(start.row(), column, parent);
            if (match(index, expression, flags)) {
                return index;
            }
        }

        /* Process all rows below. */
        for (int row = start.row() + 1; row < rowCount; ++row) {
            for (int column = 0; column < columnCount; ++column) {
                auto index = model->index(row, column, parent);
                if (match(index, expression, flags)) {
                    return index;
                }
            }
        }

        /* Start from the beginning. */
        for (int row = 0; row < start.row(); ++row) {
            for (int column = 0; column < columnCount; ++column) {
                auto index = model->index(row, column, parent);
                if (match(index, expression, flags)) {
                    return index;
                }
            }
        }

        /* Process the beginning of the same row. */
        for (int column = 0; column <= start.column(); ++column) {
            auto index = model->index(start.row(), column, parent);
            if (match(index, expression, flags)) {
                return index;
            }
        }
    } else {
        /* Process the beginning of the same row. */
        for (int column = start.column() - 1; column >= 0; --column) {
            auto index = model->index(start.row(), column, parent);
            if (match(index, expression, flags)) {
                return index;
            }
        }

        /* Process all rows above. */
        for (int row = start.row() - 1; row >= 0; --row) {
            for (int column = columnCount - 1; column >= 0; --column) {
                auto index = model->index(row, column, parent);
                if (match(index, expression, flags)) {
                    return index;
                }
            }
        }

        /* Start from the end. */
        for (int row = rowCount - 1; row > start.row(); --row) {
            for (int column = columnCount - 1; column >= 0; --column) {
                auto index = model->index(row, column, parent);
                if (match(index, expression, flags)) {
                    return index;
                }
            }
        }

        /* Process the end of the same row. */
        for (int column = columnCount - 1; column >= start.column(); --column) {
            auto index = model->index(start.row(), column, parent);
            if (match(index, expression, flags)) {
                return index;
            }
        }
    }

    return QModelIndex();
}

} // anonymous namespace

bool TreeViewSearcher::find(const QString &expression, FindFlags flags) {
    if (expression.isEmpty()) {
        return true;
    }

    if (treeView_->model() == nullptr) {
        return false;
    }

    QModelIndex result = findFirst(treeView_->currentIndex(), expression, flags);

    if (result.isValid()) {
        treeView_->setCurrentIndex(result);
        treeView_->scrollTo(result);
        return true;
    } else {
        return false;
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

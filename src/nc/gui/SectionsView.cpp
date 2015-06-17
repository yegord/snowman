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

#include "SectionsView.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include "SectionsModel.h"

namespace nc {
namespace gui {

SectionsView::SectionsView(QWidget *parent):
    TreeView(tr("Sections"), parent),
    model_(nullptr)
{
    treeView()->setItemsExpandable(false);
    treeView()->setRootIsDecorated(false);
    treeView()->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView()->setUniformRowHeights(true);
    treeView()->setSortingEnabled(true);

    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSortRole(SectionsModel::SortRole);
    treeView()->setModel(proxyModel_);
}

void SectionsView::setModel(SectionsModel *model) {
    if (model != model_) {
        model_ = model;
        proxyModel_->setSourceModel(model);
    }
}

const core::image::Section *SectionsView::selectedSection() const {
    return model_->getSection(proxyModel_->mapToSource(treeView()->currentIndex()));
}

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */

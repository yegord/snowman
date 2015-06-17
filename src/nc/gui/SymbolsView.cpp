/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "SymbolsView.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include "SymbolsModel.h"

namespace nc {
namespace gui {

SymbolsView::SymbolsView(QWidget *parent):
    TreeView(tr("Symbols"), parent),
    model_(nullptr)
{
    treeView()->setItemsExpandable(false);
    treeView()->setRootIsDecorated(false);
    treeView()->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView()->setUniformRowHeights(true);
    treeView()->setSortingEnabled(true);

    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSortRole(SymbolsModel::SortRole);
    treeView()->setModel(proxyModel_);
}

void SymbolsView::setModel(SymbolsModel *model) {
    if (model != model_) {
        model_ = model;
        proxyModel_->setSourceModel(model);
    }
}

const core::image::Symbol *SymbolsView::selectedSymbol() const {
    return model_->getSymbol(proxyModel_->mapToSource(treeView()->currentIndex()));
}

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */

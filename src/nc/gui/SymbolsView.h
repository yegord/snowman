/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include "TreeView.h"

QT_BEGIN_NAMESPACE
class QSortFilterProxyModel;
QT_END_NAMESPACE

namespace nc {

namespace core {
    namespace image {
        class Symbol;
    }
}

namespace gui {

class SymbolsModel;

class SymbolsView: public TreeView {
    Q_OBJECT

    /** The model being the source of the data. */
    SymbolsModel *model_;

    /** The model being given to QTreeView. */
    QSortFilterProxyModel *proxyModel_;

public:
    /**
     * Constructor.
     *
     * \param parent Pointer to the parent widget. Can be nullptr.
     */
    explicit SymbolsView(QWidget *parent = 0);

    /**
     * \return Pointer to the model being viewed. Can be nullptr.
     */
    SymbolsModel *model() const { return model_; }

    /**
     * Sets the model being viewed.
     *
     * \param model Pointer to the new model. Can be nullptr.
     */
    void setModel(SymbolsModel *model);

    /**
     * \returns Pointer to the currently selected symbok. Can be nullptr.
     */
    const core::image::Symbol *selectedSymbol() const;
};

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */

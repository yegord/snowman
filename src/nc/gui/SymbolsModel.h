/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory> /* std::shared_ptr */

#include <QAbstractItemModel>

namespace nc {

namespace core {
    namespace image {
        class Symbol;
        class Image;
    }
}

namespace gui {

/**
 * Item model for SymbolsView.
 */
class SymbolsModel: public QAbstractItemModel {
    Q_OBJECT

    std::shared_ptr<const core::image::Image> image_;

public:
    enum {
        SortRole = Qt::UserRole
    };

    /**
     * Constructor.
     *
     * \param parent    Pointer to the parent object. Can be nullptr.
     * \param image     Pointer to the image. Can be nullptr.
     */
    SymbolsModel(QObject *parent = nullptr, std::shared_ptr<const core::image::Image> image = nullptr);

    /**
     * \param index Model index.
     *
     * \return Pointer to the section associated with the index. Can be nullptr.
     */
    const core::image::Symbol *getSymbol(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

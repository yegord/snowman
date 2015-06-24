/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory> /* std::shared_ptr */

#include <QAbstractItemModel>

namespace nc {

namespace core {
    namespace image {
        class Section;
        class Image;
    }
}

namespace gui {

/**
 * Item model for SectionsView.
 */
class SectionsModel: public QAbstractItemModel {
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
    SectionsModel(QObject *parent = nullptr, std::shared_ptr<const core::image::Image> image = nullptr);

    /**
     * \param index Model index.
     *
     * \return Pointer to the section associated with the index. Can be nullptr.
     */
    const core::image::Section *getSection(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

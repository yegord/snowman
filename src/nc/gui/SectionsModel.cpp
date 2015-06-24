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

#include "SectionsModel.h"

#include <QStringList>

#include <nc/common/Unreachable.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>

namespace nc { namespace gui {

enum SectionsModelColumns {
    COL_NAME,
    COL_ADDRESS,
    COL_SIZE,
    COL_TYPE,
    COL_PERMISSIONS,
    COL_COUNT
};

SectionsModel::SectionsModel(QObject *parent, std::shared_ptr<const core::image::Image> image):
    QAbstractItemModel(parent), image_(std::move(image))
{}

const core::image::Section *SectionsModel::getSection(const QModelIndex &index) const {
    return static_cast<const core::image::Section *>(index.internalPointer());
}

int SectionsModel::rowCount(const QModelIndex &parent) const {
    if (!image_) {
        return 0;
    }
    if (parent == QModelIndex()) {
        return static_cast<int>(image_->sections().size());
    } else {
        return 0;
    }
}

int SectionsModel::columnCount(const QModelIndex & /*parent*/) const {
    return COL_COUNT;
}

QModelIndex SectionsModel::index(int row, int column, const QModelIndex &parent) const {
    if (!image_) {
        return QModelIndex();
    }
    if (row < rowCount(parent)) {
        return createIndex(row, column, (void *)image_->sections()[row]);
    } else {
        return QModelIndex();
    }
}

QModelIndex SectionsModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

QVariant SectionsModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole || role == SortRole) {
        auto section = getSection(index);
        assert(section);

        switch (index.column()) {
            case COL_NAME: return section->name();
            case COL_ADDRESS: {
                if (role == Qt::DisplayRole) {
                    return QString("%1").arg(section->addr(), 0, 16);
                } else {
                    return static_cast<qlonglong>(section->addr());
                }
            }
            case COL_SIZE: {
                if (role == Qt::DisplayRole) {
                    return QString("%1").arg(section->size(), 0, 16);
                } else {
                    return static_cast<qlonglong>(section->size());
                }
            }
            case COL_TYPE: {
                QStringList result;
                if (section->isCode()) {
                    result << tr("code");
                }
                if (section->isData()) {
                    result << tr("data");
                }
                if (section->isBss()) {
                    result << tr("bss");
                }
                if (!section->isAllocated()) {
                    result << tr("not allocated");
                }
                return result.join(tr(", "));
            }
            case COL_PERMISSIONS: {
                QString result;
                if (section->isReadable()) {
                    result += tr("r");
                }
                if (section->isWritable()) {
                    result += tr("w");
                }
                if (section->isExecutable()) {
                    result += tr("x");
                }
                return result;
            }
            default: unreachable();
        }
    }
    return QVariant();
}

QVariant SectionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
                case COL_NAME: return tr("Name");
                case COL_ADDRESS: return tr("Address");
                case COL_SIZE: return tr("Size");
                case COL_TYPE: return tr("Type");
                case COL_PERMISSIONS: return tr("Permissions");
                default: unreachable();
            }
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

}} // namespace nc::gui
/* vim:set et sts=4 sw=4: */

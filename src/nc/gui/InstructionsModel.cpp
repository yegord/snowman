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

#include "InstructionsModel.h"

#include <algorithm>

#include <QColor>

#include <nc/common/CheckedCast.h>
#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>

#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>

#include "Colors.h"

namespace nc { namespace gui {

enum InstructionsModelColumns {
    IMC_INSTRUCTION,
    IMC_COUNT
};

InstructionsModel::InstructionsModel(QObject *parent, std::shared_ptr<const core::arch::Instructions> instructions):
    QAbstractItemModel(parent),
    instructions_(std::move(instructions))
{
    std::vector<const core::arch::Instruction *> vector;

    if (instructions_) {
        instructionsVector_.reserve(instructions_->size());

        foreach (const auto &instruction, instructions_->all()) {
            instructionsVector_.push_back(instruction.get());
        }
    }
}

void InstructionsModel::setHighlightedInstructions(std::vector<const core::arch::Instruction *> instructions) {
    beginResetModel();

    std::sort(instructions.begin(), instructions.end());
    highlightedInstructions_ = std::move(instructions);

    endResetModel();
}

const core::arch::Instruction *InstructionsModel::getInstruction(const QModelIndex &index) const {
    return static_cast<const core::arch::Instruction *>(index.internalPointer());
}

QModelIndex InstructionsModel::getIndex(const core::arch::Instruction *instruction) const {
    assert(instruction);

    auto i = std::lower_bound(instructionsVector_.begin(), instructionsVector_.end(), instruction,
        [](const core::arch::Instruction *a, const core::arch::Instruction *b) { return a->addr() < b->addr(); });

    if (i != instructionsVector_.end() && *i == instruction) {
        return index(i - instructionsVector_.begin(), 0, QModelIndex());
    } else {
        return QModelIndex();
    }
}

int InstructionsModel::rowCount(const QModelIndex &parent) const {
    if (parent == QModelIndex()) {
        return checked_cast<int>(instructionsVector_.size());
    } else {
        return 0;
    }
}

int InstructionsModel::columnCount(const QModelIndex & /*parent*/) const {
    return IMC_COUNT;
}

QModelIndex InstructionsModel::index(int row, int column, const QModelIndex &parent) const {
    if (row < rowCount(parent)) {
        return createIndex(row, column, const_cast<core::arch::Instruction *>(instructionsVector_[row]));
    } else {
        return QModelIndex();
    }
}

QModelIndex InstructionsModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

QVariant InstructionsModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        auto instruction = getInstruction(index);
        assert(instruction);

        switch (index.column()) {
            case IMC_INSTRUCTION: return tr("%1:\t%2").arg(instruction->addr(), 0, 16).arg(instruction->toString());
            default: unreachable();
        }
    } else if (role == Qt::BackgroundRole) {
        auto instruction = getInstruction(index);
        assert(instruction);

        if (std::binary_search(highlightedInstructions_.begin(), highlightedInstructions_.end(), instruction)) {
            return QColor(highlightColor);
        }
    }
    return QVariant();
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

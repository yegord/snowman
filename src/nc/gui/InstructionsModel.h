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
#include <vector>

#include <QAbstractItemModel>

namespace nc {

namespace core {
    namespace arch {
        class Instruction;
        class Instructions;
    }
}

namespace gui {

/**
 * Item model for InstructionsView.
 */
class InstructionsModel: public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * \param parent  Pointer to the parent object. Can be nullptr.
     * \param instructions Pointer to the set of instructions. Can be nullptr.
     */
    explicit InstructionsModel(QObject *parent = nullptr, std::shared_ptr<const core::arch::Instructions> instructions = nullptr);

    /**
     * Sets the set of instructions that must be highlighted.
     *
     * \param instructions Instructions that must be highlighted.
     */
    void setHighlightedInstructions(std::vector<const core::arch::Instruction *> instructions);

    /**
     * \param index Model index.
     *
     * \return Pointer to the instruction associated with the index. Can be nullptr.
     */
    const core::arch::Instruction *getInstruction(const QModelIndex &index) const;

    /**
     * \param[in] instruction Valid pointer to an instruction.
     *
     * \return Index for the given instruction.
     */
    QModelIndex getIndex(const core::arch::Instruction *instruction) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

private:
    /** Associated set of instructions. */
    std::shared_ptr<const core::arch::Instructions> instructions_;

    /** Set of instructions as a vector (needed for direct access by index). */
    std::vector<const core::arch::Instruction *> instructionsVector_;

    /** Sorted vector of instructions that must be highlighted. */
    std::vector<const core::arch::Instruction *> highlightedInstructions_;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

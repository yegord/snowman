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

#include <QDockWidget>

#include "TreeView.h"

QT_BEGIN_NAMESPACE
class QTreeView;
QT_END_NAMESPACE

namespace nc {

namespace core {
    namespace arch {
        class Instruction;
    }
}

namespace gui {

class InstructionsModel;

/**
 * Dock widget for showing lists of instructions.
 */
class InstructionsView: public TreeView {
    Q_OBJECT

    /** The model being shown. */
    InstructionsModel *model_;

    /** Instructions currently selected in text. */
    std::vector<const core::arch::Instruction *> selectedInstructions_;

public:
    /**
     * Constructor.
     *
     * \param parent Pointer to the parent widget. Can be nullptr.
     */
    explicit InstructionsView(QWidget *parent = 0);

    /**
     * \return Pointer to the assembler document being viewed. Can be nullptr.
     */
    InstructionsModel *model() const { return model_; }

    /**
     * Sets the model being viewed.
     *
     * \param model Pointer to the new model. Can be nullptr.
     */
    void setModel(InstructionsModel *model);

    /**
     * \return Instructions currently selected in text.
     */
    const std::vector<const core::arch::Instruction *> &selectedInstructions() const { return selectedInstructions_; }

public Q_SLOTS:
    /**
     * Highlights given instructions.
     *
     * \param instructions  Instructions to be highlighted.
     * \param ensureVisible Ensure that changes in highlighting are visible.
     */
    void highlightInstructions(const std::vector<const core::arch::Instruction *> &instructions, bool ensureVisible = true);

Q_SIGNALS:
    /**
     * Signal emitted when the set of currently selected instructions is changed.
     */
    void instructionSelectionChanged();

    /**
     * Signal emitted when deletion of selected instructions is requested.
     */
    void deleteSelectedInstructions();

    /**
     * Signal emitted when decompilation of selected instructions is requested.
     */
    void decompileSelectedInstructions();

private Q_SLOTS:
    /**
     * Updates information about current selections.
     */
    void updateSelection();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

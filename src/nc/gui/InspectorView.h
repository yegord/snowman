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

#include <memory> /* std::shared_ptr */

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
class QTreeView;
QT_END_NAMESPACE

namespace nc {

namespace core {
    namespace arch {
        class Instruction;
    }
    namespace likec {
        class TreeNode;
    }
}

namespace gui {

class InspectorModel;

/**
 * Dock widget for showing decompiler's internal data structures in a tree form.
 */
class InspectorView: public QDockWidget {
    Q_OBJECT

    public:

    /**
     * Constructor.
     *
     * \param[in] parent Pointer to the parent widget. Can be nullptr.
     */
    explicit InspectorView(QWidget *parent = nullptr);

    /**
     * Destructor.
     */
    ~InspectorView();

    /**
     * \return Pointer to the tree model being viewed. Can be nullptr.
     */
    InspectorModel *model() const { return model_; }

    /**
     * \return LikeC tree nodes currently selected in the tree.
     */
    const std::vector<const core::likec::TreeNode *> &selectedNodes() const { return selectedNodes_; }

    /**
     * \return Instructions currently selected in the tree.
     */
    const std::vector<const core::arch::Instruction *> &selectedInstructions() const { return selectedInstructions_; }

    public Q_SLOTS:

    /**
     * Sets the model being viewed.
     *
     * \param model Pointer to the new model. Can be nullptr.
     */
    void setModel(InspectorModel *model);

    /**
     * Highlights given LikeC tree nodes.
     *
     * \param nodes         Nodes to be highlighted.
     */
    void highlightNodes(const std::vector<const core::likec::TreeNode *> &nodes);

    private Q_SLOTS:

    /**
     * Updates information about current selections.
     */
    void updateSelection();

    Q_SIGNALS:

    /**
     * Signal emitted when the set of currently selected LikeC tree nodes is changed.
     */
    void nodeSelectionChanged();

    /**
     * Signal emitted when the set of currently selected instructions is changed.
     */
    void instructionSelectionChanged();

    protected:

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    private:

    /** QTreeView instance used for showing the tree. */
    QTreeView *treeView_;

    /** The model being shown. */
    InspectorModel *model_;

    /** LikeC tree nodes currently selected in the tree. */
    std::vector<const core::likec::TreeNode *> selectedNodes_;

    /** Instructions currently selected in the tree. */
    std::vector<const core::arch::Instruction *> selectedInstructions_;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

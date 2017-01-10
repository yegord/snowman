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

#include <boost/optional.hpp>

#include <nc/common/Types.h>

#include "TextView.h"

namespace nc {

namespace core {
    namespace arch {
        class Instruction;
    }

    namespace ir {
        class Statement;
        class Term;
    }

    namespace likec {
        class Declaration;
        class FunctionDefinition;
        class TreeNode;
    }
}

namespace gui {

class CxxDocument;
class CppSyntaxHighlighter;

/**
 * Dock widget for showing C++ code.
 */
class CxxView: public TextView {
    Q_OBJECT

    /** Syntax highlighter for C++ code. */
    CppSyntaxHighlighter *highlighter_;

    QAction *gotoLabelAction_;
    QAction *gotoDeclarationAction_;
    QAction *gotoDefinitionAction_;
    QAction *renameAction_;

    /** Pointer to the C++ document being viewed. */
    CxxDocument *document_;

    /** LikeC tree nodes currently selected in text. */
    std::vector<const core::likec::TreeNode *> selectedNodes_;

    /** IR statements currently selected in text. */
    std::vector<const core::ir::Statement *> selectedStatements_;

    /** IR terms currently selected in text. */
    std::vector<const core::ir::Term *> selectedTerms_;

    /** Instructions currently selected in text. */
    std::vector<const core::arch::Instruction *> selectedInstructions_;

public:
    /**
     * Constructor.
     *
     * \param[in] parent Pointer to the parent widget. Can be nullptr.
     */
    explicit CxxView(QWidget *parent = nullptr);

    /**
     * \return Pointer to the C++ document being viewed. Can be nullptr.
     */
    CxxDocument *document() const { return document_; }

    /**
     * \return LikeC tree nodes currently selected in text.
     */
    const std::vector<const core::likec::TreeNode *> &selectedNodes() const { return selectedNodes_; }

    /**
     * \return IR statements currently selected in text.
     */
    const std::vector<const core::ir::Statement *> &selectedStatements() const { return selectedStatements_; }

    /**
     * \return IR terms currently selected in text.
     */
    const std::vector<const core::ir::Term *> &selectedTerms() const { return selectedTerms_; }

    /**
     * \return Instructions currently selected in text.
     */
    const std::vector<const core::arch::Instruction *> &selectedInstructions() const { return selectedInstructions_; }

    /**
     * \return Pointer to the node under cursor. Can be nullptr.
     */
    const core::likec::TreeNode *getNodeUnderCursor() const;

    /**
     * \return Integer under cursor, if any, or boost::none otherwise.
     */
    boost::optional<ConstantValue> getIntegerUnderCursor() const;

    /**
     * \return Pointer to the declaration of the identifier under cursor. Can be nullptr.
     */
    const core::likec::Declaration *getDeclarationOfIdentifierUnderCursor() const;

    /**
     * \return Pointer to the declaration of the function whose identifier or declaration is under cursor. Can be nullptr.
     */
    const core::likec::FunctionDefinition *getDefinitionOfFunctionUnderCursor() const;

public Q_SLOTS:
    /**
     * Sets the document being viewed.
     *
     * \param document Pointer to the new document. Can be nullptr.
     */
    void setDocument(CxxDocument *document);

    /**
     * Highlights given LikeC tree nodes.
     *
     * \param nodes         Nodes to be highlighted.
     * \param ensureVisible Ensure that changes in highlighting are visible.
     */
    void highlightNodes(const std::vector<const core::likec::TreeNode *> &nodes, bool ensureVisible = true);

    /**
     * Highlights given instructions (i.e. C++ code produced from them).
     *
     * \param instructions  Instructions to be highlighted.
     * \param ensureVisible Ensure that changes in highlighting are visible.
     */
    void highlightInstructions(const std::vector<const core::arch::Instruction *> &instructions, bool ensureVisible = true);

Q_SIGNALS:
    /**
     * Signal emitted when the set of currently selected LikeC tree nodes is changed.
     */
    void nodeSelectionChanged();

    /**
     * Signal emitted when the set of currently selected instructions is changed.
     */
    void instructionSelectionChanged();

    /**
     * Signal emitted when the set of currently selected IR statements is changed.
     */
    void statementSelectionChanged();

    /**
     * Signal emitted when the set of currently selected IR terms is changed.
     */
    void termSelectionChanged();

private Q_SLOTS:
    /**
     * Updates information about current selections.
     */
    void updateSelection();

    /**
     * Highlights all references of the identifier under cursor.
     */
    void highlightReferences();

    /**
     * Goes to the declaration of the identifier under cursor.
     */
    void gotoDeclaration();

    /**
     * Goes to the definition of the function under cursor.
     */
    void gotoDefinition();

    /**
     * Goes to the label under cursor.
     */
    void gotoLabel();

    /**
     * Renames whatever is under cursor into whatever the user says.
     */
    void rename();

    /**
     * Populates the context menu being created.
     *
     * \param menu Valid pointer to the menu being created.
     */
    void populateContextMenu(QMenu *menu);

private:
    /**
     * Generates the tooltip text displaying the declaration of the function or the variable in a given position.
     *
     * \param position Position in the text.
     *
     * \return Generated tooltip text.
     */
    QString getDeclarationTooltip(int position) const;
    
protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

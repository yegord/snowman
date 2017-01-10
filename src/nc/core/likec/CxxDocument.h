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

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>
#include <nc/common/Types.h>

#include <QString>

#include <nc/common/RangeTree.h>

namespace nc {

namespace core {
    class Context;

    namespace arch {
        class Instruction;
    }

    namespace ir {
        class Statement;
        class Term;
    }

namespace likec {

class Declaration;
class FunctionDeclaration;
class FunctionDefinition;
class LabelDeclaration;
class LabelStatement;
class TreeNode;

/**
 * Text document containing C++ listing.
 */
class CxxDocument {
    RangeTree rangeTree_;
    QString text_;
    boost::unordered_map<const TreeNode *, const RangeNode *> node2rangeNode_;
    boost::unordered_map<const arch::Instruction *, std::vector<const RangeNode *>> instruction2rangeNodes_;
    boost::unordered_map<const Declaration *, std::vector<const TreeNode *>> declaration2uses_;
    boost::unordered_map<const LabelDeclaration *, const LabelStatement *> label2statement_;
    boost::unordered_map<const FunctionDeclaration *, const FunctionDefinition *> functionDeclaration2definition_;

public:
    /**
     * Constructor.
     *
     * \param context Pointer to the context. Can be nullptr.
     */
    CxxDocument(const Context *context);

    /**
     * \return Pointer to the deepest tree node at the given position. Can be nullptr.
     */
    const TreeNode *getLeafAt(int position) const;

    /**
     * \return List of valid pointers to the nodes fully contained in the given range.
     */
    std::vector<const TreeNode *> getNodesIn(const Range<int> &range) const;

    /**
     * \param node Valid pointer to a tree node.
     *
     * \return Text range occupied by this node.
     */
    Range<int> getRange(const TreeNode *node) const;

    /**
     * \param instruction Valid pointer to an instruction.
     * \param[out] result List of ranges occupied by the nodes generated from this instruction.
     */
    void getRanges(const arch::Instruction *instruction, std::vector<Range<int>> &result) const;

    /**
     * \param declaration Valid pointer to a declaration tree node.
     *
     * \return All the tree nodes using this declaration.
     */
    const std::vector<const TreeNode *> &getUses(const Declaration *declaration) const {
        assert(declaration != nullptr);
        return nc::find(declaration2uses_, declaration);
    }

    /**
     * \param declaration Valid pointer to a label declaration node.
     *
     * \return Pointer to the matching label statement. Can be nullptr.
     */
    const LabelStatement *getLabelStatement(const LabelDeclaration *declaration) const {
        assert(declaration != nullptr);
        return nc::find(label2statement_, declaration);
    }

    const FunctionDefinition *getFunctionDefinition(const FunctionDeclaration *declaration) const {
        assert(declaration != nullptr);
        return nc::find(functionDeclaration2definition_, declaration);
    }

    /**
     * \return The text of the document.
     */
    QString getText() const { return text_; }

    /**
     * For a node, computes statement, term, and instruction, from which
     * this node has originated.
     *
     * \param[in]  node         Valid pointer to a tree node.
     * \param[out] statement    Original statement.
     * \param[out] term         Original term.
     * \param[out] instruction  Original instruction.
     */
    static void getOrigin(const TreeNode *node, const ir::Statement *&statement,
                          const ir::Term *&term, const arch::Instruction *&instruction);

    /**
     * \param node Valid pointer to a tree node.
     *
     * \return Declaration of whatever identifier this node is. Can be nullptr.
     */
    static const Declaration *getDeclarationOfIdentifier(const TreeNode *node);

protected:
    void computeReverseMappings(const RangeNode *rangeNode);

private:
    void replaceText(const Range<int> &range, const QString &text);
};

}}} // namespace nc::core::likec

/* vim:set et sts=4 sw=4: */

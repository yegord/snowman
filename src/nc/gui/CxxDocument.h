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

#include <QTextDocument>

#include <nc/common/Range.h>
#include <nc/common/Types.h>

#include <nc/common/RangeTree.h>
#include <nc/core/likec/CxxDocument.h>

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
    }
}

namespace gui {

/**
 * Text document containing C++ listing.
 */
class CxxDocument: public QTextDocument, public core::likec::CxxDocument {
    Q_OBJECT

    std::shared_ptr<const core::Context> context_;
    RangeTree rangeTree_;
    boost::unordered_map<const core::likec::TreeNode *, const RangeNode *> node2rangeNode_;
    boost::unordered_map<const core::arch::Instruction *, std::vector<const RangeNode *>> instruction2rangeNodes_;
    boost::unordered_map<const core::likec::Declaration *, std::vector<const core::likec::TreeNode *>> declaration2uses_;
    boost::unordered_map<const core::likec::LabelDeclaration *, const core::likec::LabelStatement *> label2statement_;
    boost::unordered_map<const core::likec::FunctionDeclaration *, const core::likec::FunctionDefinition *> functionDeclaration2definition_;

public:
    /**
     * Constructor.
     *
     * \param parent  Pointer to the parent object. Can be nullptr.
     * \param context Pointer to the context. Can be nullptr.
     */
    explicit CxxDocument(QObject *parent = nullptr, std::shared_ptr<const core::Context> context = nullptr);

    /**
     * Replaces the text of all identifiers referring to the given declaration
     * with the given name.
     *
     * \param declaration Valid pointer to a declaration.
     * \param newName New name.
     */
    void rename(const core::likec::Declaration *declaration, const QString &newName);

    /**
     * \return Text in the given range.
     */
    QString getText(const Range<int> &range) const;

private Q_SLOTS:
    void onContentsChange(int position, int charsRemoved, int charsAdded);

private:
    void computeReverseMappings(const RangeNode *rangeNode);
    void replaceText(const Range<int> &range, const QString &text);
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

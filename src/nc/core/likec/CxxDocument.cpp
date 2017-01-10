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

#include "CxxDocument.h"

#include <QTextStream>

#include <nc/core/Context.h>

#include <nc/core/ir/Statement.h>
#include <nc/core/ir/Term.h>

#include <nc/core/likec/Expression.h>
#include <nc/core/likec/FunctionDeclaration.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/FunctionIdentifier.h>
#include <nc/core/likec/LabelDeclaration.h>
#include <nc/core/likec/LabelIdentifier.h>
#include <nc/core/likec/LabelStatement.h>
#include <nc/core/likec/Statement.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/VariableDeclaration.h>
#include <nc/core/likec/VariableIdentifier.h>

#include <nc/common/RangeTreeBuilder.h>

namespace nc { namespace core { namespace likec {

namespace {

QString printTree(const Tree &tree, RangeTree &rangeTree) {
    class Callback: public PrintCallback<const TreeNode *> {
        RangeTreeBuilder builder_;
        const QString &out_;

    public:
        Callback(RangeTree &tree, const QString &out) : builder_(tree), out_(out) {}

        void onStartPrinting(const TreeNode *node) override {
            builder_.onStart((void *)(node), out_.size());
        }
        void onEndPrinting(const TreeNode *node) override {
            builder_.onEnd((void *)(node), out_.size());
        }
    };

    QString result;
    QTextStream stream(&result);
    Callback callback(rangeTree, result);

    tree.print(stream, &callback);

    return result;
}

inline const TreeNode *getNode(const RangeNode *rangeNode) {
    return (const TreeNode *)rangeNode->data();
}

} // anonymous namespace

CxxDocument::CxxDocument(const Context *context) {
    if (context && context->tree()) {
        text_ = printTree(*context->tree(), rangeTree_);
        if (rangeTree_.root()) {
            computeReverseMappings(rangeTree_.root());
        }
    }
}

void CxxDocument::computeReverseMappings(const RangeNode *rangeNode) {
    assert(rangeNode != nullptr);

    auto node = getNode(rangeNode);

    node2rangeNode_[node] = rangeNode;

    const ir::Statement *statement;
    const ir::Term *term;
    const arch::Instruction *instruction;

    getOrigin(node, statement, term, instruction);

    if (instruction) {
        instruction2rangeNodes_[instruction].push_back(rangeNode);
    }

    if (auto declaration = getDeclarationOfIdentifier(node)) {
        declaration2uses_[declaration].push_back(node);
    }

    if (auto declaration = node->as<Declaration>()) {
        if (auto definition = declaration->as<FunctionDefinition>()) {
            functionDeclaration2definition_[definition->getFirstDeclaration()] = definition;
        }
    }

    if (auto *statement = node->as<Statement>()) {
        if (auto *labelStatement = statement->as<LabelStatement>()) {
            label2statement_[labelStatement->identifier()->declaration()] = labelStatement;
        }
    }

    foreach (const auto &child, rangeNode->children()) {
        computeReverseMappings(&child);
    }
}

const TreeNode *CxxDocument::getLeafAt(int position) const {
    if (auto rangeNode = rangeTree_.getLeafAt(position)) {
        return getNode(rangeNode);
    }
    return nullptr;
}

std::vector<const TreeNode *> CxxDocument::getNodesIn(const Range<int> &range) const {
    auto rangeNodes = rangeTree_.getNodesIn(range);

    std::vector<const TreeNode *> result;
    result.reserve(result.size());

    foreach (auto rangeNode, rangeNodes) {
        result.push_back(getNode(rangeNode));
    }

    return result;
}

Range<int> CxxDocument::getRange(const TreeNode *node) const {
    assert(node != nullptr);
    if (auto rangeNode = nc::find(node2rangeNode_, node)) {
        return rangeTree_.getRange(rangeNode);
    }
    return Range<int>();
}

void CxxDocument::getRanges(const arch::Instruction *instruction, std::vector<Range<int>> &result) const {
    assert(instruction != nullptr);

    const auto &rangeNodes = nc::find(instruction2rangeNodes_, instruction);

    foreach (auto rangeNode, rangeNodes) {
        if (auto range = rangeTree_.getRange(rangeNode)) {
            result.push_back(range);
        }
    }
}

void CxxDocument::getOrigin(const TreeNode *node, const ir::Statement *&statement,
                            const ir::Term *&term, const arch::Instruction *&instruction)
{
    assert(node != nullptr);

    statement = nullptr;
    term = nullptr;
    instruction = nullptr;

    if (auto stmt = node->as<Statement>()) {
        statement = stmt->statement();
    } else if (auto expr = node->as<Expression>()) {
        term = expr->term();
        if (term) {
            statement = term->statement();
        }
    }

    if (statement) {
        instruction = statement->instruction();
    }
}

const Declaration *CxxDocument::getDeclarationOfIdentifier(const TreeNode *node) {
    assert(node != nullptr);
    if (auto expression = node->as<Expression>()) {
        if (auto identifier = expression->as<FunctionIdentifier>()) {
            return identifier->declaration();
        } else if (auto identifier = expression->as<LabelIdentifier>()) {
            return identifier->declaration();
        } else if (auto identifier = expression->as<VariableIdentifier>()) {
            return identifier->declaration();
        }
    }
    return nullptr;
}

}}} // namespace nc::core::likec

/* vim:set et sts=4 sw=4: */

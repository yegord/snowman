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

#include "InspectorModel.h"

#include <nc/common/CheckedCast.h>

#include <nc/core/Context.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/dflow/Value.h>
#include <nc/core/likec/BinaryOperator.h>
#include <nc/core/likec/CallOperator.h>
#include <nc/core/likec/CaseLabel.h>
#include <nc/core/likec/DefaultLabel.h>
#include <nc/core/likec/DoWhile.h>
#include <nc/core/likec/Expression.h>
#include <nc/core/likec/ExpressionStatement.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/FunctionIdentifier.h>
#include <nc/core/likec/Goto.h>
#include <nc/core/likec/If.h>
#include <nc/core/likec/InlineAssembly.h>
#include <nc/core/likec/IntegerConstant.h>
#include <nc/core/likec/LabelIdentifier.h>
#include <nc/core/likec/MemberAccessOperator.h>
#include <nc/core/likec/Return.h>
#include <nc/core/likec/String.h>
#include <nc/core/likec/StructType.h>
#include <nc/core/likec/Switch.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/Typecast.h>
#include <nc/core/likec/UnaryOperator.h>
#include <nc/core/likec/UndeclaredIdentifier.h>
#include <nc/core/likec/VariableIdentifier.h>
#include <nc/core/likec/While.h>

#include "InspectorItem.h"
#include "ParentTracker.h"

namespace nc { namespace gui {

InspectorModel::InspectorModel(QObject *parent, std::shared_ptr<const core::Context> context):
    QAbstractItemModel(parent), context_(std::move(context)), root_(new InspectorItem(""))
{
    if (context_ && context_->tree()) {
        root_->setNode(context_->tree()->root());
    }
}

InspectorModel::~InspectorModel() {}

InspectorItem *InspectorModel::getItem(const QModelIndex &index) const {
    if (index == QModelIndex()) {
        return root();
    } else {
        assert(index.internalPointer() != nullptr);
        return static_cast<InspectorItem *>(index.internalPointer());
    }
}

QModelIndex InspectorModel::getIndex(const InspectorItem *item) const {
    assert(item != nullptr);
    if (item == root()) {
        return QModelIndex();
    } else {
        return createIndex(item->row(), item->column(), (void *)item);
    }
}

namespace detail {

inline QString tr(const char *text) {
    return InspectorModel::tr(text);
}

void expand(InspectorItem *item, const core::arch::Instruction *instruction) {
    item->addComment(instruction->toString().trimmed().replace('\t', ' '));
    item->addChild(tr("addr = %1").arg(instruction->addr()));
    item->addChild(tr("size = %1").arg(instruction->size()));
}

void expand(InspectorItem *item, const core::ir::Statement *statement) {
    if (statement->instruction()) {
        item->addChild("instruction", statement->instruction());
    }

    switch (statement->kind()) {
        case core::ir::Statement::INLINE_ASSEMBLY: {
            item->addComment("Inline Assembly");
            break;
        }
        case core::ir::Statement::ASSIGNMENT: {
            auto *assignment = statement->asAssignment();
            item->addComment("Assignment");
            item->addChild(tr("left"), assignment->left());
            item->addChild(tr("right"), assignment->right());
            break;
        }
        case core::ir::Statement::JUMP: {
            auto *jump = statement->asJump();
            item->addComment("Jump");
            item->addChild(tr("condition"), jump->condition());
            item->addChild(tr("then target address"), jump->thenTarget().address());
            item->addChild(tr("then target basic block = %1").arg((uintptr_t)jump->thenTarget().address()));
            item->addChild(tr("else target address"), jump->elseTarget().address());
            item->addChild(tr("else target basic block = %1").arg((uintptr_t)jump->elseTarget().address()));
            break;
        }
        case core::ir::Statement::CALL: {
            auto *call = statement->asCall();
            item->addComment("Call");
            item->addChild(tr("target"), call->target());
            break;
        }
        case core::ir::Statement::TOUCH: {
            auto *touch = statement->asTouch();
            item->addComment("Touch");
            item->addChild(tr("access type = %1").arg(touch->term()->accessType()));
            item->addChild(tr("term"), touch->term());
            break;
        }
        case core::ir::Statement::CALLBACK: {
            item->addComment("Callback");
            break;
        }
        default: {
            item->addComment(tr("statement kind = %1").arg(statement->kind()));
            break;
        }
    }
}

void expand(InspectorItem *item, const core::ir::Term *term, const core::Context *context) {
    item->addChild(term->toString());
    if (term->statement()) {
        item->addChild(tr("statement"), term->statement());
    }
    item->addChild(tr("size = %1").arg(term->size()));

    if (term->statement() && term->statement()->basicBlock() && term->statement()->basicBlock()->function()) {
        auto &dataflow = *context->dataflows()->at(term->statement()->basicBlock()->function());

        if (const core::ir::dflow::Value *value = dataflow.getValue(term)) {
            InspectorItem *valueItem = item->addChild(tr("value properties"));
            if (value->abstractValue().isConcrete()) {
                valueItem->addChild(tr("constant value = %1").arg(value->abstractValue().asConcrete().value()));
                valueItem->addChild(tr("signed constant value = %1").arg(value->abstractValue().asConcrete().signedValue()));
            } else {
                valueItem->addChild(tr("zero bits = %1").arg(value->abstractValue().zeroBits(), 0, 16));
                valueItem->addChild(tr("one bits = %1").arg(value->abstractValue().oneBits(), 0, 16));
            }
            if (value->isStackOffset()) {
                valueItem->addChild(tr("stack offset = %1").arg(value->stackOffset()));
            } else if (value->isNotStackOffset()) {
                valueItem->addChild(tr("definitely not a stack offset"));
            } else {
                valueItem->addChild(tr("not a stack offset"));
            }
            if (value->isProduct()) {
                valueItem->addChild(tr("is a product"));
            } else if (value->isNotProduct()) {
                valueItem->addChild(tr("is not a product"));
            }
            if (value->isReturnAddress()) {
                valueItem->addChild(tr("is a return address"));
            } else if (value->isNotReturnAddress()) {
                valueItem->addChild(tr("is not a return address"));
            }
        }

        if (auto &memoryLocation = dataflow.getMemoryLocation(term)) {
            item->addChild(tr("computed memory location = %1").arg(memoryLocation.toString()));
        }

        if (term->isRead()) {
            InspectorItem *definitionsItem = item->addChild(tr("definitions"));

            foreach (auto &chunk, dataflow.getDefinitions(term).chunks()) {
                auto chunkItem = definitionsItem->addChild(chunk.location().toString());
                foreach (auto definition, chunk.definitions()) {
                    chunkItem->addChild("", definition);
                }
            }
        }
    } else {
        item->addChild("function = nullptr");
    }

    switch (term->kind()) {
        case core::ir::Term::INT_CONST: {
            auto *constant = term->asConstant();
            item->addComment(tr("Constant"));
            item->addChild(tr("value = %1").arg(constant->value().value()));
            break;
        }
        case core::ir::Term::INTRINSIC: {
            auto *intrinsic = term->asIntrinsic();
            item->addComment(tr("Intrinsic"));
            item->addChild(tr("intrinsic kind = %1").arg(intrinsic->intrinsicKind()));
            break;
        }
        case core::ir::Term::MEMORY_LOCATION_ACCESS: {
            auto *access = term->asMemoryLocationAccess();
            item->addComment(tr("Memory Location Access"));
            item->addChild(tr("memory location = %1").arg(access->memoryLocation().toString()));
            break;
        }
        case core::ir::Term::DEREFERENCE: {
            auto *dereference = term->asDereference();
            item->addComment(tr("Dereference"));
            item->addChild(tr("domain = %1").arg(dereference->domain()));
            item->addChild(tr("address"), dereference->address());
            break;
        }
        case core::ir::Term::UNARY_OPERATOR: {
            auto *unary = term->asUnaryOperator();
            item->addComment(tr("Unary Operator"));
            item->addChild(tr("operator kind = %1").arg(unary->operatorKind()));
            item->addChild(tr("operand"), unary->operand());
            break;
        }
        case core::ir::Term::BINARY_OPERATOR: {
            auto *binary = term->asBinaryOperator();
            item->addComment(tr("Binary Operator"));
            item->addChild(tr("operator kind = %1").arg(binary->operatorKind()));
            item->addChild(tr("left"), binary->left());
            item->addChild(tr("right"), binary->right());
            break;
        }
        default: {
            item->addChild(tr("kind = %1").arg(term->kind()));
            break;
        }
    }
}

void expand(InspectorItem *item, const core::likec::Declaration *declaration) {
    switch (declaration->declarationKind()) {
        case core::likec::Declaration::FUNCTION_DECLARATION: {
            item->addComment(tr("Function Declaration"));
            break;
        }
        case core::likec::Declaration::FUNCTION_DEFINITION: {
            const core::likec::FunctionDefinition *functionDefinition = declaration->as<core::likec::FunctionDefinition>();
            item->addComment(tr("Function Definition"));
            item->addChild(tr("block"), functionDefinition->block());
            break;
        }
        case core::likec::Declaration::LABEL_DECLARATION: {
            item->addComment(tr("Label Declaration"));
            break;
        }
        case core::likec::Declaration::MEMBER_DECLARATION: {
            item->addComment(tr("Member Declaration"));
            break;
        }
        case core::likec::Declaration::STRUCT_TYPE_DECLARATION: {
            item->addComment(tr("Struct Type Declaration"));
            break;
        }
        case core::likec::Declaration::VARIABLE_DECLARATION: {
            const core::likec::VariableDeclaration *variableDeclaration = declaration->as<core::likec::VariableDeclaration>();
            item->addComment(tr("Variable Declaration"));
            item->addChild(tr("type"), variableDeclaration->type());
            break;
        }
        default: {
            item->addComment(tr("declaration kind = %1").arg(declaration->declarationKind()));
            break;
        }
    }
}

void expand(InspectorItem *item, const core::likec::Expression *expression) {
    if (expression->term()) {
        item->addChild(tr("IR term"), expression->term());
    }
    switch (expression->expressionKind()) {
        case core::likec::Expression::BINARY_OPERATOR: {
            auto binary = expression->as<core::likec::BinaryOperator>();
            item->addComment(tr("Binary Operator"));
            item->addChild(tr("kind = %1").arg(binary->operatorKind()));
            item->addChild(tr("left"), binary->left());
            item->addChild(tr("right"), binary->right());
            break;
        }
        case core::likec::Expression::CALL_OPERATOR: {
            auto call = expression->as<core::likec::CallOperator>();
            item->addComment(tr("Call Operator"));
            item->addChild(tr("callee"), call->callee());

            auto arguments = item->addChild(tr("arguments"));
            foreach (const auto &argument, call->arguments()) {
                arguments->addChild("", argument);
            }
            break;
        }
        case core::likec::Expression::FUNCTION_IDENTIFIER: {
            auto identifier = expression->as<core::likec::FunctionIdentifier>();
            item->addComment(tr("Function Identifier"));
            item->addChild(tr("declaration"), identifier->declaration());
            break;
        }
        case core::likec::Expression::INTEGER_CONSTANT: {
            auto constant = expression->as<core::likec::IntegerConstant>();
            item->addComment(tr("Integer Constant"));
            item->addChild(tr("value = %1").arg(constant->value().value()));
            break;
        }
        case core::likec::Expression::LABEL_IDENTIFIER: {
            auto identifier = expression->as<core::likec::LabelIdentifier>();
            item->addComment(tr("Label Identifier"));
            item->addChild(tr("declaration"), identifier->declaration());
            break;
        }
        case core::likec::Expression::MEMBER_ACCESS_OPERATOR: {
            auto *access = expression->as<core::likec::MemberAccessOperator>();
            item->addComment("Member Access Operator");
            item->addChild(tr("access kind = %1").arg(access->accessKind()));
            item->addChild("compound", access->compound());
            item->addChild("member", access->member());
            break;
        }
        case core::likec::Expression::STRING: {
            auto *string = expression->as<core::likec::String>();
            item->addComment(tr("String"));
            item->addChild(tr("characters = %1").arg(string->characters()));
            break;
        }
        case core::likec::Expression::TYPECAST: {
            auto *typecast = expression->as<core::likec::Typecast>();
            item->addComment(tr("Type Cast"));
            item->addChild(tr("operand"), typecast->operand());
            break;
        }
        case core::likec::Expression::UNARY_OPERATOR: {
            auto *unary = expression->as<core::likec::UnaryOperator>();
            item->addComment(tr("Unary Operator"));
            item->addChild(tr("kind = %1").arg(unary->operatorKind()));
            item->addChild(tr("operand"), unary->operand());
            break;
        }
        case core::likec::Expression::VARIABLE_IDENTIFIER: {
            auto identifier = expression->as<core::likec::VariableIdentifier>();
            item->addComment(tr("Variable Identifier"));
            item->addChild(tr("declaration"), identifier->declaration());
            break;
        }
        case core::likec::Expression::UNDECLARED_IDENTIFIER: {
            auto identifier = expression->as<core::likec::UndeclaredIdentifier>();
            item->addComment(tr("Undeclared Identifier"));
            item->addChild(tr("name = %1").arg(identifier->name()));
            item->addChild(tr("type = %1").arg(identifier->type()->toString()));
            break;
        }
        default: {
            item->addComment(tr("expression kind = %1").arg(expression->expressionKind()));
            break;
        }
    }
}

void expand(InspectorItem *item, const core::likec::Statement *statement) {
    if (statement->statement()) {
        item->addChild(tr("IR statement"), statement->statement());
    }

    switch (statement->statementKind()) {
        case core::likec::Statement::BLOCK: {
            const core::likec::Block *block = statement->as<core::likec::Block>();
            item->addComment(tr("Block"));

            InspectorItem *declarations = item->addChild(tr("declarations"));
            foreach (const auto &declaration, block->declarations()) {
                declarations->addChild(declaration->identifier(), declaration);
            }

            InspectorItem *statements = item->addChild(tr("statements"));
            foreach (const auto &statement, block->statements()) {
                statements->addChild("", statement);
            }
            break;
        }
        case core::likec::Statement::BREAK: {
            item->addComment(tr("Break"));
            break;
        }
        case core::likec::Statement::CONTINUE: {
            item->addComment(tr("Continue"));
            break;
        }
        case core::likec::Statement::DO_WHILE: {
            const core::likec::DoWhile *doWhile = statement->as<core::likec::DoWhile>();
            item->addComment(tr("Do-While"));
            item->addChild(tr("body"), doWhile->body());
            item->addChild(tr("condition"), doWhile->condition());
            break;
        }
        case core::likec::Statement::EXPRESSION_STATEMENT: {
            const core::likec::ExpressionStatement *expressionStatement = statement->as<core::likec::ExpressionStatement>();
            item->addComment(tr("Expression Statement"));
            item->addChild(tr("expression"), expressionStatement->expression());
            break;
        }
        case core::likec::Statement::GOTO: {
            const core::likec::Goto *go = statement->as<core::likec::Goto>();
            item->addComment(tr("Goto"));
            item->addChild(tr("destination"), go->destination());
            break;
        }
        case core::likec::Statement::IF: {
            const core::likec::If *conditional = statement->as<core::likec::If>();
            item->addComment(tr("If"));
            item->addChild(tr("condition"), conditional->condition());
            item->addChild(tr("then"), conditional->thenStatement());
            if (conditional->elseStatement()) {
                item->addChild(tr("else"), conditional->elseStatement());
            }
            break;
        }
        case core::likec::Statement::LABEL_STATEMENT: {
            item->addComment(tr("Label Statement"));
            break;
        }
        case core::likec::Statement::RETURN: {
            const core::likec::Return *ret = statement->as<core::likec::Return>();
            item->addComment(tr("Return"));
            if (ret->returnValue()) {
                item->addChild(tr("return value"), ret->returnValue());
            }
            break;
        }
        case core::likec::Statement::WHILE: {
            const core::likec::While *loop = statement->as<core::likec::While>();
            item->addComment(tr("While"));
            item->addChild(tr("condition"), loop->condition());
            item->addChild(tr("body"), loop->body());
            break;
        }
        case core::likec::Statement::INLINE_ASSEMBLY: {
            auto assembly = statement->as<core::likec::InlineAssembly>();
            item->addComment(tr("Inline assembly"));
            item->addChild(tr("code = %1").arg(assembly->code()));
            break;
        }
        case core::likec::Statement::SWITCH: {
            auto witch = statement->as<core::likec::Switch>();
            item->addComment(tr("Switch"));
            item->addChild(tr("expression"), witch->expression());
            item->addChild(tr("body"), witch->body());
            break;
        }
        case core::likec::Statement::CASE_LABEL: {
            auto label = statement->as<core::likec::CaseLabel>();
            item->addComment(tr("Case label"));
            item->addChild(tr("expression"), label->expression());
            break;
        }
        case core::likec::Statement::DEFAULT_LABEL: {
            item->addComment(tr("Default label"));
            break;
        }
        default: {
            item->addComment(tr("statement kind = %1").arg(statement->statementKind()));
            break;
        }
    }
}

void expand(InspectorItem *item, const core::likec::TreeNode *node) {
    switch (node->nodeKind()) {
        case core::likec::TreeNode::COMPILATION_UNIT: {
            const core::likec::CompilationUnit *unit = node->as<core::likec::CompilationUnit>();
            item->addComment(tr("Compilation unit"));
            foreach (const auto &declaration, unit->declarations()) {
                item->addChild(declaration->identifier(), declaration);
            }
            break;
        }
        case core::likec::TreeNode::DECLARATION: {
            const core::likec::Declaration *declaration = node->as<core::likec::Declaration>();
            expand(item, declaration);
            break;
        }
        case core::likec::TreeNode::EXPRESSION: {
            const core::likec::Expression *expression = node->as<core::likec::Expression>();
            expand(item, expression);
            break;
        }
        case core::likec::TreeNode::STATEMENT: {
            const core::likec::Statement *statement = node->as<core::likec::Statement>();
            expand(item, statement);
            break;
        }
        default: {
            item->addComment(tr("node kind = %1").arg(node->nodeKind()));
            break;
        }
    }
}

void expand(InspectorItem *item, const core::likec::Type *type) {
    item->addChild(tr("ptr = 0x%1").arg(reinterpret_cast<intptr_t>(type), 0, 16));
    item->addChild(tr("size = %1").arg(type->size()));
    item->addChild(tr("sizeof = %1").arg(type->sizeOf()));
    switch (type->kind()) {
        case core::likec::Type::ERRONEOUS: {
            item->addComment(tr("Erroneous"));
            break;
        }
        case core::likec::Type::FLOAT: {
            item->addComment(tr("Float"));
            break;
        }
        case core::likec::Type::FUNCTION_POINTER: {
            const core::likec::FunctionPointerType *fp = type->as<core::likec::FunctionPointerType>();
            item->addComment(tr("Function Pointer"));
            item->addChild(tr("return type"), fp->returnType());
            item->addChild(tr("variadic = %1").arg(fp->variadic()));

            int i = 0;
            foreach(const core::likec::Type *arg, fp->argumentTypes()) {
                item->addChild(tr("arg%1").arg(++i), arg);
            }
            break;
        }
        case core::likec::Type::INTEGER: {
            const core::likec::IntegerType *integer = type->as<core::likec::IntegerType>();
            item->addComment(tr("Integer"));
            item->addChild(tr("unsigned = %1").arg(integer->isUnsigned()));
            break;
        }
        case core::likec::Type::POINTER: {
            const core::likec::PointerType *pointer = type->as<core::likec::PointerType>();
            item->addComment(tr("Pointer"));
            item->addChild(tr("pointee"), pointer->pointeeType());
            break;
        }
        case core::likec::Type::STRUCT_TYPE: {
            const core::likec::StructType *structure = type->as<core::likec::StructType>();
            item->addComment(tr("Struct"));
            foreach (const auto &member, structure->members()) {
                item->addChild(member->identifier(), member->type());
            }
            break;
        }
        case core::likec::Type::VOID: {
            item->addComment(tr("Void"));
            break;
        }
        default: {
            item->addComment(tr("kind = %1").arg(type->kind()));
            break;
        }
    }
}

} // namespace detail

void InspectorModel::expand(InspectorItem *item) const {
    assert(item != nullptr);

    if (item->expanded()) {
        return;
    }

    if (item->node()) {
        detail::expand(item, item->node());
    } else if (item->term()) {
        detail::expand(item, item->term(), context_.get());
    } else if (item->statement()) {
        detail::expand(item, item->statement());
    } else if (item->instruction()) {
        detail::expand(item, item->instruction());
    } else if (item->type()) {
        detail::expand(item, item->type());
    }

    item->setExpanded(true);
}

int InspectorModel::rowCount(const QModelIndex &parent) const {
    InspectorItem *item = getItem(parent);
    expand(item);
    return checked_cast<int>(item->children().size());
}

int InspectorModel::columnCount(const QModelIndex & /*parent*/) const {
    return 1;
}

QModelIndex InspectorModel::index(int row, int column, const QModelIndex &parent) const {
    if (row < rowCount(parent)) {
        return createIndex(row, column, getItem(parent)->children()[row].get());
    } else {
        /* This really happens, at least on Qt 4.7.4 on MSVC. */
        return QModelIndex();
    }
}

QModelIndex InspectorModel::parent(const QModelIndex &index) const {
    InspectorItem *item = getItem(index);
    if (item == root()) {
        return QModelIndex();
    } else {
        assert(item->parent() != nullptr);
        return getIndex(item->parent());
    }
}

QVariant InspectorModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        return getItem(index)->text();
    }
    return QVariant();
}

const core::likec::TreeNode *InspectorModel::getParent(const core::likec::TreeNode *node) {
    if (context_ == nullptr || context_->tree() == nullptr) {
        return nullptr;
    }

    if (node2parent_.empty()) {
        ParentTracker tracker(node2parent_);
        tracker(context_->tree()->root());
    }

    return nc::find(node2parent_, node);
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

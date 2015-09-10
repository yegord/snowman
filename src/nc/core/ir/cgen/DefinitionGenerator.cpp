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

#include "DefinitionGenerator.h"

#include <nc/common/Foreach.h>
#include <nc/common/Memoization.h>
#include <nc/common/Range.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/image/Image.h>
#ifdef NC_PREFER_CSTRINGS_TO_CONSTANTS
#include <nc/core/image/Reader.h>
#include <nc/core/image/Section.h>
#endif
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/CFG.h>
#include <nc/core/ir/Dominators.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/CallHook.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/EntryHook.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/calling/ReturnHook.h>
#include <nc/core/ir/cflow/BasicNode.h>
#include <nc/core/ir/cflow/Dfs.h>
#include <nc/core/ir/cflow/Graphs.h>
#include <nc/core/ir/cflow/Switch.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/dflow/Uses.h>
#include <nc/core/ir/dflow/Utils.h>
#include <nc/core/ir/dflow/Value.h>
#include <nc/core/ir/liveness/Livenesses.h>
#include <nc/core/ir/types/Type.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/vars/Variables.h>
#include <nc/core/likec/BinaryOperator.h>
#include <nc/core/likec/Block.h>
#include <nc/core/likec/Break.h>
#include <nc/core/likec/CallOperator.h>
#include <nc/core/likec/CaseLabel.h>
#include <nc/core/likec/Continue.h>
#include <nc/core/likec/DefaultLabel.h>
#include <nc/core/likec/DoWhile.h>
#include <nc/core/likec/ExpressionStatement.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/FunctionIdentifier.h>
#include <nc/core/likec/Goto.h>
#include <nc/core/likec/If.h>
#include <nc/core/likec/InlineAssembly.h>
#include <nc/core/likec/IntegerConstant.h>
#include <nc/core/likec/LabelIdentifier.h>
#include <nc/core/likec/LabelStatement.h>
#include <nc/core/likec/Return.h>
#include <nc/core/likec/String.h>
#include <nc/core/likec/Switch.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/Typecast.h>
#include <nc/core/likec/UnaryOperator.h>
#include <nc/core/likec/VariableIdentifier.h>
#include <nc/core/likec/While.h>

#include "SwitchContext.h"

namespace nc {
namespace core {
namespace ir {
namespace cgen {

DefinitionGenerator::DefinitionGenerator(CodeGenerator &parent, const Function *function, const CancellationToken &canceled):
    DeclarationGenerator(parent, calling::CalleeId(function), parent.signatures().getSignature(function).get()),
    function_(function),
    dataflow_(*parent.dataflows().at(function)),
    graph_(*parent.graphs().at(function)),
    liveness_(*parent.livenesses().at(function)),
    uses_(std::make_unique<dflow::Uses>(dataflow_)),
    dominators_(std::make_unique<Dominators>(CFG(function->basicBlocks()), canceled)),
    definition_(nullptr)
{
    assert(function != nullptr);
}

DefinitionGenerator::~DefinitionGenerator() {}

void DefinitionGenerator::setDefinition(likec::FunctionDefinition *definition) {
    assert(!definition_);
    definition_ = definition;
    setDeclaration(definition);
}

std::unique_ptr<likec::FunctionDefinition> DefinitionGenerator::createDefinition() {
    auto nameAndComment = parent().nameGenerator().getFunctionName(function_);

    auto functionDefinition = std::make_unique<likec::FunctionDefinition>(tree(),
        std::move(nameAndComment.name()), makeReturnType(), signature()->variadic());

    functionDefinition->setComment(std::move(nameAndComment.comment()));

    setDefinition(functionDefinition.get());

    if (auto entryHook = parent().hooks().getEntryHook(function_)) {
        foreach (const auto &argument, signature()->arguments()) {
            auto term = entryHook->getArgumentTerm(argument.get());
            assert(term != nullptr && "Entry hook must have clones of all arguments in the signature.");
            assert(dataflow_.getMemoryLocation(term) && "Argument must have a memory location.");

            auto variable = parent().variables().getVariable(term);
            assert(variable != nullptr && "Each term with a memory location must belong to a variable.");

            if (variable->memoryLocation() == dataflow_.getMemoryLocation(term)) {
                auto &variableDeclaration = variableDeclarations_[variable];
                assert(!variableDeclaration);
                variableDeclaration = makeArgumentDeclaration(term);
            } else {
                auto variableDeclaration = makeArgumentDeclaration(term);
                definition()->block()->addStatement(std::make_unique<likec::ExpressionStatement>(
                    std::make_unique<likec::BinaryOperator>(
                        likec::BinaryOperator::ASSIGN,
                        makeVariableAccess(term),
                        std::make_unique<likec::VariableIdentifier>(variableDeclaration))));
            }
        }
    }

    computeInvisibleStatements();

    SwitchContext switchContext;
    makeStatements(graph_.root(), definition()->block().get(), nullptr, nullptr, nullptr, switchContext);

    return functionDefinition;
}

likec::VariableDeclaration *DefinitionGenerator::makeLocalVariableDeclaration(const vars::Variable *variable) {
    assert(variable != nullptr);
    assert(variable->isLocal());

    likec::VariableDeclaration *&result = variableDeclarations_[variable];
    if (!result) {
        auto nameAndComment = parent().nameGenerator().getLocalVariableName(variable->memoryLocation(), variableDeclarations_.size());

        auto variableDeclaration = std::make_unique<likec::VariableDeclaration>(
            std::move(nameAndComment.name()), parent().makeVariableType(variable));
        variableDeclaration->setComment(std::move(nameAndComment.comment()));

        result = variableDeclaration.get();
        definition()->block()->addDeclaration(std::move(variableDeclaration));
    }
    return result;
}

likec::VariableDeclaration *DefinitionGenerator::makeVariableDeclaration(const vars::Variable *variable) {
    assert(variable != nullptr);

    if (variable->isGlobal()) {
        return parent().makeGlobalVariableDeclaration(variable);
    } else {
        return makeLocalVariableDeclaration(variable);
    }
}

likec::LabelDeclaration *DefinitionGenerator::makeLabel(const BasicBlock *basicBlock) {
    likec::LabelDeclaration *&result = labels_[basicBlock];
    if (!result) {
        auto label = std::make_unique<likec::LabelDeclaration>(
            basicBlock->address() ?
                QString("addr_0x%1_%2").arg(basicBlock->address().get(), 0, 16).arg(labels_.size()) :
                QString("label_%1").arg(labels_.size())
            );
        result = label.get();
        definition()->addLabel(std::move(label));
    }
    return result;
}

void DefinitionGenerator::addLabels(const BasicBlock *basicBlock, likec::Block *block, SwitchContext &switchContext) {
    assert(basicBlock != nullptr);
    assert(block != nullptr);

    /* Add usual label. */
    block->addStatement(
        std::make_unique<likec::LabelStatement>(std::make_unique<likec::LabelIdentifier>(makeLabel(basicBlock))));

    /* Add case labels. */
    if (basicBlock->address()) {
        if (basicBlock == switchContext.defaultBasicBlock()) {
            block->addStatement(std::make_unique<likec::DefaultLabel>());
        } else {
            foreach (ConstantValue value, switchContext.getCaseValues(*basicBlock->address())) {
                block->addStatement(std::make_unique<likec::CaseLabel>(
                    std::make_unique<likec::IntegerConstant>(value, switchContext.valueType())));
            }
        }
        switchContext.eraseCaseValues(*basicBlock->address());
    }
}

void DefinitionGenerator::makeStatements(const cflow::Node *node, likec::Block *block, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB, SwitchContext &switchContext) {
    switch (node->nodeKind()) {
    case cflow::Node::BASIC: {
        auto basicNode = node->as<cflow::BasicNode>();

        addLabels(basicNode->basicBlock(), block, switchContext);

        foreach (const ir::Statement *statement, basicNode->basicBlock()->statements()) {
            if (auto likecStatement = makeStatement(statement, nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(likecStatement));
            }
        }

        break;
    }
    case cflow::Node::REGION: {
        auto region = node->as<cflow::Region>();

        switch (region->regionKind()) {
        case cflow::Region::UNKNOWN: {
            assert(region->nodes().size() > 0);

            /*
             * We tend to process nodes in DFS order because it is likely
             * to minimize the number of generated gotos.
             */
            makeStatements(cflow::Dfs(region).preordering(), block, nextBB, breakBB, continueBB, switchContext);
            break;
        }
        case cflow::Region::BLOCK: {
            assert(region->nodes().size() > 0);

            makeStatements(region->nodes(), block, nextBB, breakBB, continueBB, switchContext);
            break;
        }
        case cflow::Region::COMPOUND_CONDITION: {
            assert(region->nodes().size() == 2);

            makeStatements(region->nodes(), block, nextBB, breakBB, continueBB, switchContext);
            break;
        }
        case cflow::Region::IF_THEN_ELSE: {
            assert(region->nodes().size() == 3);

            std::unique_ptr<likec::Expression> condition(makeExpression(region->nodes()[0], block,
                region->nodes()[1]->getEntryBasicBlock(), region->nodes()[2]->getEntryBasicBlock(), switchContext));

            auto thenBlock = std::make_unique<likec::Block>();
            makeStatements(region->nodes()[1], thenBlock.get(), nextBB, breakBB, continueBB, switchContext);

            auto elseBlock = std::make_unique<likec::Block>();
            makeStatements(region->nodes()[2], elseBlock.get(), nextBB, breakBB, continueBB, switchContext);

            block->addStatement(std::make_unique<likec::If>(std::move(condition), std::move(thenBlock), std::move(elseBlock)));

            break;
        }
        case cflow::Region::IF_THEN: {
            assert(region->nodes().size() == 2);
            assert(region->exitBasicBlock() != nullptr);

            std::unique_ptr<likec::Expression> condition(makeExpression(region->nodes()[0], block,
                region->nodes()[1]->getEntryBasicBlock(), region->exitBasicBlock(), switchContext));

            auto thenBlock = std::make_unique<likec::Block>();
            makeStatements(region->nodes()[1], thenBlock.get(), nextBB, breakBB, continueBB, switchContext);

            block->addStatement(std::make_unique<likec::If>(std::move(condition), std::move(thenBlock)));

            break;
        }
        case cflow::Region::LOOP: {
            assert(region->nodes().size() > 0);

            auto condition = std::make_unique<likec::IntegerConstant>(1, tree().makeIntegerType(tree().intSize(), false));

            cflow::Dfs dfs(region);

            auto body = std::make_unique<likec::Block>();
            const BasicBlock *entryBB = region->entry()->getEntryBasicBlock();

            makeStatements(dfs.preordering(), body.get(), entryBB, nextBB, entryBB, switchContext);

            block->addStatement(std::make_unique<likec::While>(std::move(condition), std::move(body)));

            break;
        }
        case cflow::Region::WHILE: {
            assert(region->nodes().size() > 0);
            assert(region->exitBasicBlock() != nullptr);

            addLabels(region->entry()->getEntryBasicBlock(), block, switchContext);

            cflow::Node *bodyEntry = region->entry()->uniqueSuccessor();

            auto condition = makeExpression(region->entry(), nullptr,
                bodyEntry ? bodyEntry->getEntryBasicBlock() : region->entry()->getEntryBasicBlock(),
                region->exitBasicBlock(), switchContext);

            cflow::Dfs dfs(region);
            auto &nodes = dfs.preordering();

            assert(nodes.front() == region->entry());
            nodes.erase(nodes.begin());

            auto body = std::make_unique<likec::Block>();
            const BasicBlock *conditionBB = region->entry()->getEntryBasicBlock();

            makeStatements(nodes, body.get(), conditionBB, region->exitBasicBlock(), conditionBB, switchContext);

            block->addStatement(std::make_unique<likec::While>(std::move(condition), std::move(body)));

            if (auto jump = makeJump(region->exitBasicBlock(), nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(jump));
            }

            break;
        }
        case cflow::Region::DO_WHILE: {
            assert(region->nodes().size() > 0);
            assert(region->exitBasicBlock() != nullptr);
            assert(region->loopCondition() != nullptr);

            cflow::Dfs dfs(region);
            auto &nodes = dfs.preordering();

            assert(nc::contains(nodes, region->loopCondition()));
            nodes.erase(std::find(nodes.begin(), nodes.end(), region->loopCondition()));

            auto body = std::make_unique<likec::Block>();
            const BasicBlock *conditionBB = region->loopCondition()->getEntryBasicBlock();

            makeStatements(nodes, body.get(), conditionBB, nextBB, conditionBB, switchContext);

            auto condition = makeExpression(region->loopCondition(), body.get(),
                region->entry()->getEntryBasicBlock(),
                region->exitBasicBlock(),
                switchContext);

            block->addStatement(std::make_unique<likec::DoWhile>(std::move(body), std::move(condition)));

            if (auto jump = makeJump(region->exitBasicBlock(), nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(jump));
            }

            break;
        }
        case cflow::Region::SWITCH: {
            auto witch = region->as<cflow::Switch>();

            /*
             * Generates code for the basic block, except the code for its terminator.
             */
            auto makeStatementsButLast = [&](const BasicBlock *basicBlock) {
                addLabels(basicBlock, block, switchContext);

                auto iend = --basicBlock->statements().end();
                for (auto i = basicBlock->statements().begin(); i != iend; ++i) {
                    /* We do not care about breakBB and others: we will not create gotos. */
                    if (auto likecStatement = makeStatement(*i, nullptr, nullptr, nullptr)) {
                        block->addStatement(std::move(likecStatement));
                    }
                }
            };

            /* Generate code for the basic block doing the bounds check. */
            if (witch->boundsCheckNode()) {
                makeStatementsButLast(witch->boundsCheckNode()->basicBlock());
            }

            /* Generate code for the basic block with the table-based jump. */
            makeStatementsButLast(witch->switchNode()->basicBlock());

            /* The jump via the jump table. */
            const Jump *jump = witch->switchNode()->basicBlock()->getJump();
            assert(jump != nullptr);
            assert(jump->isUnconditional());

            /* The jump table. */
            const JumpTable *jumpTable = jump->thenTarget().table();
            assert(jumpTable != nullptr);

            /*
             * Make a new switch context.
             */
            SwitchContext newSwitchContext;

            newSwitchContext.setValueType(tree().makeIntegerType(witch->switchTerm()->size(), true));

            for (std::size_t i = 0, size = witch->jumpTableSize(); i < size; ++i) {
                newSwitchContext.addCaseValue((*jumpTable)[i].address(), i);
            }

            if (witch->defaultBasicBlock()) {
                newSwitchContext.setDefaultBasicBlock(witch->defaultBasicBlock());
            }

            /* Exit basic block of the switch. */
            const BasicBlock *exitBB = witch->exitBasicBlock();
            if (!exitBB) {
                exitBB = nextBB;
            }

            /*
             * Generate the switch expression.
             */
            auto expression = std::make_unique<likec::Typecast>(
                newSwitchContext.valueType(),
                makeExpression(witch->switchTerm()));

            /*
             * Generate the body of the switch.
             */
            cflow::Dfs dfs(region);
            auto &nodes = dfs.preordering();

            nodes.erase(
                std::remove_if(nodes.begin(), nodes.end(),
                    [witch](const cflow::Node *node){ return node == witch->boundsCheckNode() || node == witch->switchNode(); }),
                nodes.end());

            auto body = std::make_unique<likec::Block>();

            makeStatements(nodes, body.get(), exitBB, exitBB, continueBB, newSwitchContext);

            /*
             * Generate case labels that were not generated before.
             */
            foreach (const auto &pair, newSwitchContext.caseValuesMap()) {
                foreach (ConstantValue value, pair.second) {
                    body->addStatement(std::make_unique<likec::CaseLabel>(
                        std::make_unique<likec::IntegerConstant>(value, newSwitchContext.valueType())));
                }
                body->addStatement(std::make_unique<likec::Goto>(
                    std::make_unique<likec::IntegerConstant>(
                        pair.first,
                        tree().makeIntegerType(tree().pointerSize(), true))));
            }

            /* Generate the switch. */
            block->addStatement(std::make_unique<likec::Switch>(std::move(expression), std::move(body)));

            /* Generate a jump to the exit basic block, if it's not the nextBB. */
            if (auto jump = makeJump(exitBB, nextBB, breakBB, continueBB)) {
                block->addStatement(std::move(jump));
            }

            break;
        }
        default:
            unreachable();
        }
        break;
    }
    default:
        unreachable();
    }
}

void DefinitionGenerator::makeStatements(const std::vector<cflow::Node *> &nodes, likec::Block *block, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB, SwitchContext &switchContext) {
    if (nodes.empty()) {
        return;
    }
    for (std::size_t i = 0, last = nodes.size() - 1; i != last; ++i) {
        makeStatements(nodes[i], block, nodes[i + 1]->getEntryBasicBlock(), breakBB, continueBB, switchContext);
    }
    makeStatements(nodes.back(), block, nextBB, breakBB, continueBB, switchContext);
}

std::unique_ptr<likec::Expression> DefinitionGenerator::makeExpression(const cflow::Node *node, likec::Block *block, const BasicBlock *thenBB, const BasicBlock *elseBB, SwitchContext &switchContext) {
    assert(node != nullptr);
    assert(thenBB != nullptr);
    assert(elseBB != nullptr);
    assert(node->isCondition() && "Can only generate expressions from condition nodes.");

    std::unique_ptr<likec::Expression> result;

    if (const cflow::BasicNode *basicNode = node->as<cflow::BasicNode>()) {
        if (block) {
            addLabels(basicNode->basicBlock(), block, switchContext);
        }

        foreach (const ir::Statement *statement, basicNode->basicBlock()->statements()) {
            std::unique_ptr<likec::Expression> expression;

            if (const Jump *jump = statement->asJump()) {
                assert(jump == basicNode->basicBlock()->getJump());

                expression = makeExpression(jump->condition());

                assert((jump->thenTarget().basicBlock() == thenBB && jump->elseTarget().basicBlock() == elseBB) ||
                       (jump->thenTarget().basicBlock() == elseBB && jump->elseTarget().basicBlock() == thenBB));

                if (jump->thenTarget().basicBlock() != thenBB) {
                    expression = std::make_unique<likec::UnaryOperator>(likec::UnaryOperator::LOGICAL_NOT,
                                                                        std::move(expression));
                }
            } else if (auto stmt = makeStatement(statement, nullptr, nullptr, nullptr)) {
                if (block) {
                    block->addStatement(std::move(stmt));
                } else if (likec::ExpressionStatement *expressionStatement = stmt->as<likec::ExpressionStatement>()) {
                    expression = expressionStatement->releaseExpression();
                }
            }

            if (expression) {
                if (!result) {
                    result = std::move(expression);
                } else {
                    result = std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::COMMA,
                        std::move(result), std::move(expression));
                }
            }
        }
    } else if (const cflow::Region *region = node->as<cflow::Region>()) {
        assert(region->regionKind() == cflow::Region::COMPOUND_CONDITION);
        assert(region->nodes().size() == 2);

        /*
         * Distinguishing AND from OR:
         *
         * if (a || b) { then } { else }
         *
         * a -> then || b
         * b -> then || else
         *
         * if (a && b) { then } { else }
         *
         * a -> b || else
         * b -> then || else
         */

        const cflow::Node *n = region->nodes()[0];
        while (const cflow::Region *r = n->as<cflow::Region>()) {
            assert(r->regionKind() == cflow::Region::COMPOUND_CONDITION);
            assert(r->nodes().size() == 2);
            n = r->nodes()[1];
        }

        const cflow::BasicNode *b = n->as<cflow::BasicNode>();
        assert(b != nullptr);

        const Jump *j = b->basicBlock()->getJump();
        assert(j != nullptr);

        if (j->thenTarget().basicBlock() == thenBB || j->elseTarget().basicBlock() == thenBB) {
            auto left  = makeExpression(region->nodes()[0], block, thenBB, region->nodes()[1]->getEntryBasicBlock(), switchContext);
            auto right = makeExpression(region->nodes()[1], nullptr, thenBB, elseBB, switchContext);

            result = std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::LOGICAL_OR,
                std::move(left), std::move(right));
        } else if (j->thenTarget().basicBlock() == elseBB || j->elseTarget().basicBlock() == elseBB) {
            auto left  = makeExpression(region->nodes()[0], block, region->nodes()[1]->getEntryBasicBlock(), elseBB, switchContext);
            auto right = makeExpression(region->nodes()[1], nullptr, thenBB, elseBB, switchContext);

            result = std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::LOGICAL_AND,
                std::move(left), std::move(right));
        } else {
            assert(!"First component of compound condition must contain a jump to thenBB or elseBB.");
        }
    } else {
        assert(!"Node must be a basic block node or a region.");
    }

    assert(result != nullptr && "Something is very wrong.");

    return result;
}

std::unique_ptr<likec::Statement> DefinitionGenerator::makeStatement(const Statement *statement, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB) {
    assert(statement);

    if (nc::contains(invisibleStatements_, statement)) {
        return nullptr;
    }

    auto result = doMakeStatement(statement, nextBB, breakBB, continueBB);

    if (result != nullptr) {
        class StatementSetter {
            const ir::Statement *statement_;

        public:
            StatementSetter(const ir::Statement *statement): statement_(statement) {
                assert(statement != nullptr);
            }

            void operator()(likec::TreeNode *node) {
                if (auto stmt = node->as<likec::Statement>()) {
                    if (stmt->statement() == nullptr) {
                        stmt->setStatement(statement_);
                        stmt->callOnChildren(*this);
                    }
                }
            }
        };

        StatementSetter setter(statement);
        setter(result.get());
    }

    return result;
}

std::unique_ptr<likec::Statement> DefinitionGenerator::doMakeStatement(const Statement *statement, const BasicBlock *nextBB, const BasicBlock *breakBB, const BasicBlock *continueBB) {
    switch (statement->kind()) {
        case Statement::INLINE_ASSEMBLY: {
            return std::make_unique<likec::InlineAssembly>(
                statement->instruction() ? statement->instruction()->toString() : QString());
        }
        case Statement::ASSIGNMENT: {
            const Assignment *assignment = statement->asAssignment();

            if (!liveness_.isLive(assignment->left())) {
                return nullptr;
            }

            if (isSubstitutableWrite(assignment->left())) {
                return nullptr;
            }

            std::unique_ptr<likec::Expression> left(makeExpression(assignment->left()));
            std::unique_ptr<likec::Expression> right(makeExpression(assignment->right()));

            return std::make_unique<likec::ExpressionStatement>(
                std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::ASSIGN,
                    std::move(left),
                    std::make_unique<likec::Typecast>(
                        parent().makeType(parent().types().getType(assignment->left())),
                        std::move(right))));
        }
        case Statement::JUMP: {
            const Jump *jump = statement->asJump();

            if (jump->isConditional()) {
                auto thenJump = makeJump(jump, jump->thenTarget(), nextBB, breakBB, continueBB);
                auto elseJump = makeJump(jump, jump->elseTarget(), nextBB, breakBB, continueBB);
                auto condition = makeExpression(jump->condition());

                if (thenJump == nullptr) {
                    if (elseJump == nullptr) {
                        return nullptr;
                    } else {
                        std::swap(thenJump, elseJump);
                        condition = std::make_unique<likec::UnaryOperator>(likec::UnaryOperator::LOGICAL_NOT, std::move(condition));
                    }
                }
                return std::make_unique<likec::If>(std::move(condition), std::move(thenJump), std::move(elseJump));
            } else {
                return makeJump(jump, jump->thenTarget(), nextBB, breakBB, continueBB);
            }
        }
        case Statement::CALL: {
            const Call *call = statement->asCall();

            std::unique_ptr<likec::Expression> target;

            auto targetValue = dataflow_.getValue(call->target());
            if (targetValue->abstractValue().isConcrete()) {
                if (auto functionDeclaration = parent().makeFunctionDeclaration(targetValue->abstractValue().asConcrete().value())) {
                    target = std::make_unique<likec::FunctionIdentifier>(functionDeclaration);
                    target->setTerm(call->target());
                }
            }

            if (!target) {
                target = makeExpression(call->target());
            }

            auto callOperator = std::make_unique<likec::CallOperator>(std::move(target));

            if (auto callSignature = parent().signatures().getSignature(call)) {
                if (auto callHook = parent().hooks().getCallHook(call)) {
                    foreach (const auto &argument, callSignature->arguments()) {
                        callOperator->addArgument(makeExpression(callHook->getArgumentTerm(argument.get())));
                    }

                    if (callSignature->returnValue()) {
                        const Term *returnValueTerm = callHook->getReturnValueTerm(callSignature->returnValue().get());

                        if (liveness_.isLive(returnValueTerm)) {
                            return std::make_unique<likec::ExpressionStatement>(
                                std::make_unique<likec::BinaryOperator>(
                                    likec::BinaryOperator::ASSIGN,
                                    makeExpression(returnValueTerm),
                                    std::make_unique<likec::Typecast>(
                                        parent().makeType(parent().types().getType(returnValueTerm)),
                                        std::move(callOperator))));
                        }
                    }
                }
            }

            return std::make_unique<likec::ExpressionStatement>(std::move(callOperator));
        }
        case Statement::HALT: {
            return nullptr;
        }
        case Statement::TOUCH: {
            return nullptr;
        }
        case Statement::CALLBACK: {
            return nullptr;
        }
    }

    unreachable();
    return nullptr;
}

std::unique_ptr<likec::Statement> DefinitionGenerator::makeJump(const BasicBlock *target, const BasicBlock *nextBB,
                                                                const BasicBlock *breakBB,
                                                                const BasicBlock *continueBB) {
    if (target == nextBB) {
        return nullptr;
    } else if (target == breakBB) {
        return std::make_unique<likec::Break>();
    } else if (target == continueBB) {
        return std::make_unique<likec::Continue>();
    } else {
        return std::make_unique<likec::Goto>(
                std::make_unique<likec::LabelIdentifier>(makeLabel(target)));
    }
}

std::unique_ptr<likec::Statement> DefinitionGenerator::makeJump(const Jump *jump, const JumpTarget &target,
                                                                const BasicBlock *nextBB, const BasicBlock *breakBB,
                                                                const BasicBlock *continueBB) {
    assert(jump != nullptr);

    if (target.basicBlock()) {
        return makeJump(target.basicBlock(), nextBB, breakBB, continueBB);
    } else if (target.address()) {
        if (dflow::isReturnAddress(target.address(), dataflow_)) {
            if (signature()->returnValue()) {
                if (auto returnHook = parent().hooks().getReturnHook(jump)) {
                    return std::make_unique<likec::Return>(
                        makeExpression(returnHook->getReturnValueTerm(signature()->returnValue().get())));
                }
            }
            return std::make_unique<likec::Return>();
        }
        return std::make_unique<likec::Goto>(makeExpression(target.address()));
    } else {
        return std::make_unique<likec::Goto>(std::make_unique<likec::String>(QLatin1String("???")));
    }
}

std::unique_ptr<likec::Expression> DefinitionGenerator::makeExpression(const Term *term) {
    assert(term != nullptr);

    auto result = doMakeExpression(term);
    assert(result != nullptr);

    class TermSetter {
        const ir::Term *term_;

    public:
        TermSetter(const ir::Term *term): term_(term) {
            assert(term != nullptr);
        }

        void operator()(likec::TreeNode *node) {
            if (auto expression = node->as<likec::Expression>()) {
                if (expression->term() == nullptr) {
                    expression->setTerm(term_);
                    expression->callOnChildren(*this);
                }
            }
        }
    };

    TermSetter setter(term);
    setter(result.get());

    return result;
}

std::unique_ptr<likec::Expression> DefinitionGenerator::doMakeExpression(const Term *term) {
#ifdef NC_PREFER_CONSTANTS_TO_EXPRESSIONS
    if (term->isRead()) {
        const dflow::Value *value = dataflow_.getValue(term);

        if (value->abstractValue().isConcrete()) {
            return makeConstant(term, value->abstractValue().asConcrete());
        }
    }
#endif

    if (parent().variables().getVariable(term)) {
        if (term->isRead() && isSubstitutableRead(term)) {
            return makeExpression(getTheOnlyDefinition(term)->source());
        } else {
            return makeVariableAccess(term);
        }
    }

    switch (term->kind()) {
        case Term::INT_CONST: {
            return makeConstant(term, term->asConstant()->value());
        }
        case Term::INTRINSIC: {
            auto intrinsic = term->as<Intrinsic>();
            QString name;
            switch (intrinsic->intrinsicKind()) {
                case Intrinsic::UNDEFINED:
                    name = QLatin1String("undefined");
                    break;
                case Intrinsic::ZERO_STACK_OFFSET:
                    name = QLatin1String("zero stack offset");
                    break;
                case Intrinsic::RETURN_ADDRESS:
                    name = QLatin1String("return address");
                    break;
                default:
                    name = QLatin1String("intrinsic");
                    break;
            }
            return std::make_unique<likec::CallOperator>(std::make_unique<likec::String>(name));
        }
        case Term::MEMORY_LOCATION_ACCESS: {
            assert(!"The term must belong to a variable.");
            return nullptr;
        }
        case Term::DEREFERENCE: {
            assert(!dataflow_.getMemoryLocation(term) && "The term must belong to a variable.");

            auto dereference = term->asDereference();
            auto type = parent().types().getType(dereference);
            auto addressType = parent().types().getType(dereference->address());

            return std::make_unique<likec::UnaryOperator>(likec::UnaryOperator::DEREFERENCE,
                std::make_unique<likec::Typecast>(
                    tree().makePointerType(addressType->size(), parent().makeType(type)),
                    makeExpression(dereference->address())));
        }
        case Term::UNARY_OPERATOR: {
            return doMakeExpression(term->asUnaryOperator());
        }
        case Term::BINARY_OPERATOR: {
            return doMakeExpression(term->asBinaryOperator());
        }
        case Term::CHOICE: {
            const Choice *choice = term->asChoice();
            if (!dataflow_.getDefinitions(choice->preferredTerm()).empty()) {
                return makeExpression(choice->preferredTerm());
            } else {
                return makeExpression(choice->defaultTerm());
            }
        }
        default: {
            unreachable();
            return nullptr;
        }
    }
}

std::unique_ptr<likec::Expression> DefinitionGenerator::doMakeExpression(const UnaryOperator *unary) {
    std::unique_ptr<likec::Expression> operand(makeExpression(unary->operand()));

    switch (unary->operatorKind()) {
        case UnaryOperator::NOT: {
            const types::Type *operandType = parent().types().getType(unary->operand());
            return std::make_unique<likec::UnaryOperator>(likec::UnaryOperator::BITWISE_NOT,
                std::make_unique<likec::Typecast>(
                    tree().makeIntegerType(operandType->size(), operandType->isUnsigned()), std::move(operand)));
        }
        case UnaryOperator::NEGATION: {
            const types::Type *operandType = parent().types().getType(unary->operand());
            return std::make_unique<likec::UnaryOperator>(likec::UnaryOperator::NEGATION,
                std::make_unique<likec::Typecast>(
                    tree().makeIntegerType(operandType->size(), operandType->isUnsigned()), std::move(operand)));
        }
        case UnaryOperator::SIGN_EXTEND: {
            return std::make_unique<likec::Typecast>(
                tree().makeIntegerType(unary->size(), false),
                std::make_unique<likec::Typecast>(
                    tree().makeIntegerType(unary->operand()->size(), false), std::move(operand)));
        }
        case UnaryOperator::ZERO_EXTEND: {
            return std::make_unique<likec::Typecast>(
                tree().makeIntegerType(unary->size(), true),
                std::make_unique<likec::Typecast>(
                    tree().makeIntegerType(unary->operand()->size(), true), std::move(operand)));
        }
        case UnaryOperator::TRUNCATE: {
            const types::Type *type = parent().types().getType(unary);
            return std::make_unique<likec::Typecast>(parent().makeType(type), std::move(operand));
        }
        default:
            unreachable();
            return nullptr;
    }
}

std::unique_ptr<likec::Expression> DefinitionGenerator::doMakeExpression(const BinaryOperator *binary) {
    const types::Type *leftType = parent().types().getType(binary->left());
    const types::Type *rightType = parent().types().getType(binary->right());

    std::unique_ptr<likec::Expression> left(makeExpression(binary->left()));
    std::unique_ptr<likec::Expression> right(makeExpression(binary->right()));

    switch (binary->operatorKind()) {
        case BinaryOperator::AND:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::BITWISE_AND,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::OR:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::BITWISE_OR,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::XOR:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::BITWISE_XOR,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SHL:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::SHL,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SHR:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::SHR,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SAR:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::SHR,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::ADD:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::ADD,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SUB:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::SUB,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::MUL:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::MUL,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), leftType->isUnsigned()), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), rightType->isUnsigned()), std::move(right)));

        case BinaryOperator::SIGNED_DIV:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::DIV,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::SIGNED_REM:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::REM,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::UNSIGNED_DIV:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::DIV,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::UNSIGNED_REM:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::REM,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::EQUAL:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::EQ,
                std::move(left),
                std::move(right));

        case BinaryOperator::SIGNED_LESS:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::LT,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::SIGNED_LESS_OR_EQUAL:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::LEQ,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), false), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), false), std::move(right)));

        case BinaryOperator::UNSIGNED_LESS:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::LT,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), true), std::move(right)));

        case BinaryOperator::UNSIGNED_LESS_OR_EQUAL:
            return std::make_unique<likec::BinaryOperator>(likec::BinaryOperator::LEQ,
                std::make_unique<likec::Typecast>(tree().makeIntegerType(leftType->size(), true), std::move(left)),
                std::make_unique<likec::Typecast>(tree().makeIntegerType(rightType->size(), true), std::move(right)));

        default:
            unreachable();
            return nullptr;
    }
}

std::unique_ptr<likec::Expression> DefinitionGenerator::makeConstant(const Term *term, const SizedValue &value) {
    const types::Type *type = parent().types().getType(term);

#ifdef NC_PREFER_FUNCTIONS_TO_CONSTANTS
    if (auto section = parent().image().getSectionContainingAddress(value.value())) {
        if (section->isCode()) {
            if (auto functionDeclaration = parent().makeFunctionDeclaration(value.value())) {
                return std::make_unique<likec::FunctionIdentifier>(functionDeclaration);
            }
        }
    }
#endif

#ifdef NC_PREFER_CSTRINGS_TO_CONSTANTS
    if (!type->pointee() || type->pointee()->size() <= 1) {
        auto isAscii = [](const QString &string) -> bool {
            foreach (QChar c, string) {
                if (c >= 0x80 || (c < 0x20 && c != '\r' && c != '\n' && c != '\t')) {
                    return false;
                }
            }
            return true;
        };

        QString string = image::Reader(&parent().image()).readAsciizString(value.value(), 1024);

        if (!string.isEmpty() && isAscii(string)) {
            return std::make_unique<likec::String>(string);
        }
    }
#endif

#ifdef NC_PREFER_GLOBAL_VARIABLES_TO_CONSTANTS
    if (type->pointee() && type->pointee()->size()) {
        return std::make_unique<likec::UnaryOperator>(
            likec::UnaryOperator::REFERENCE,
            std::make_unique<likec::VariableIdentifier>(
                parent().makeGlobalVariableDeclaration(
                    MemoryLocation(MemoryDomain::MEMORY, value.value() * CHAR_BIT, type->pointee()->size()),
                    type)));
    }
#endif

    return std::make_unique<likec::IntegerConstant>(value, tree().makeIntegerType(value.size(), type->isUnsigned()));
}

std::unique_ptr<likec::Expression> DefinitionGenerator::makeVariableAccess(const Term *term) {
    assert(term != nullptr);

    const auto &termLocation = dataflow_.getMemoryLocation(term);
    assert(termLocation);

    auto variable = parent().variables().getVariable(term);
    assert(variable != nullptr);

    auto identifier = std::make_unique<likec::VariableIdentifier>(makeVariableDeclaration(variable));

    if (termLocation == variable->memoryLocation()) {
        return std::move(identifier);
    } else {
        /*
         * Generate pointer arithmetics to get to the right part of the variable.
         *
         * Note: this does not handle the case of non-byte-aligned locations.
         * However, I am not sure whether they can be reliably handled in C at all.
         */
        auto variableAddress = std::make_unique<likec::Typecast>(
            tree().makeIntegerType(tree().pointerSize(), false),
            std::make_unique<likec::UnaryOperator>(
                likec::UnaryOperator::REFERENCE,
                std::move(identifier)));

        std::unique_ptr<likec::Expression> termAddress;
        if (termLocation.addr() == variable->memoryLocation().addr()) {
            termAddress = std::move(variableAddress);
        } else {
            termAddress = std::make_unique<likec::BinaryOperator>(
                likec::BinaryOperator::ADD,
                std::move(variableAddress),
                std::make_unique<likec::IntegerConstant>(
                    (termLocation.addr() - variable->memoryLocation().addr()) / CHAR_BIT,
                    tree().makeIntegerType(tree().pointerSize(), false)));
        }

        return std::make_unique<likec::UnaryOperator>(
            likec::UnaryOperator::DEREFERENCE,
            std::make_unique<likec::Typecast>(
                tree().makePointerType(parent().makeType(parent().types().getType(term))),
                std::move(termAddress)));
    }
}

void DefinitionGenerator::computeInvisibleStatements() {
    if (auto hook = parent().hooks().getEntryHook(function_)) {
        foreach (const auto &termAndClone, hook->argumentTerms()) {
            invisibleStatements_.insert(termAndClone.second->statement());
        }
    }

    foreach (auto basicBlock, function_->basicBlocks()) {
        foreach (auto statement, basicBlock->statements()) {
            if (auto call = statement->asCall()) {
                if (auto hook = parent().hooks().getCallHook(call)) {
                    foreach (const auto &termAndClone, hook->argumentTerms()) {
                        invisibleStatements_.insert(termAndClone.second->statement());
                    }
                    foreach (const auto &termAndClone, hook->returnValueTerms()) {
                        invisibleStatements_.insert(termAndClone.second->statement());
                    }
                }
            } else if (auto jump = statement->asJump()) {
                if (dflow::isReturn(jump, dataflow_)) {
                    if (auto hook = parent().hooks().getReturnHook(jump)) {
                        foreach (const auto &termAndClone, hook->returnValueTerms()) {
                            invisibleStatements_.insert(termAndClone.second->statement());
                        }
                    }
                }
            }
        }
    }
}

bool DefinitionGenerator::isSubstitutableWrite(const Term *write) const {
    assert(write != nullptr);
    assert(write->isWrite());
    assert(liveness_.isLive(write) && "We should not care about dead writes.");

    return nc::memoize(write2isSubstitutable_, write, [&]() -> bool {
        auto source = write->source();
        if (!source) {
            return false;
        }

        if (nc::contains(invisibleStatements_, write->statement())) {
            return false;
        }

        const auto &memoryLocation = dataflow_.getMemoryLocation(write);
        if (!memoryLocation) {
            return false;
        }

        if (!parent().variables().getVariable(write)->isLocal()) {
            return false;
        }

        std::size_t nuses = 0;

        foreach (const auto &use, uses_->getUses(write)) {
            auto destination = use.term();

            if (liveness_.isLive(destination)) {
                if (dataflow_.getMemoryLocation(destination) != memoryLocation) {
                    return false;
                }

                auto theOnlyDefinition = getTheOnlyDefinition(destination);
                if (!theOnlyDefinition) {
                    return false;
                }
                assert(theOnlyDefinition == write);

                if (!isDominating(write, destination)) {
                    return false;
                }

                if (!canBeMoved(source, destination)) {
                    return false;
                }

                ++nuses;
            }
        }

        assert(nuses >= 1 && "Live write must have at least one live read.");

        if (nuses > 1 && (source->kind() == Term::UNARY_OPERATOR || source->kind() == Term::BINARY_OPERATOR)) {
            /* We do not want to substitute complex expressions multiple times. */
            return false;
        }

        return true;
    });
}

bool DefinitionGenerator::canBeMoved(const Term *source, const Term *destination) const {
    assert(source != nullptr);
    assert(source->isRead());
    assert(liveness_.isLive(source) && "We should not care about dead reads.");
    assert(destination != nullptr);
    assert(destination->isRead());
    assert(liveness_.isLive(destination) && "We should not care about dead reads.");

#ifdef NC_PREFER_CONSTANTS_TO_EXPRESSIONS
    if (dataflow_.getValue(source)->abstractValue().isConcrete()) {
        return true;
    }
#endif

    switch (source->kind()) {
        case Term::INT_CONST:
        case Term::INTRINSIC: {
            return true;
        }
        case Term::MEMORY_LOCATION_ACCESS:
        case Term::DEREFERENCE: {
            /* This must be a local variable. */
            auto variable = parent().variables().getVariable(source);
            if (!variable || !variable->isLocal()) {
                return false;
            }

            if (source->statement()->basicBlock() == destination->statement()->basicBlock()) {
                /* There must be no writes to source's variable in between. */

                auto begin = source->statement()->basicBlock()->statements().get_iterator(source->statement());
                auto end = destination->statement()->basicBlock()->statements().get_iterator(destination->statement());

                return std::find_if(begin, end, [&](const Statement *statement) -> bool {
                    if (auto assignment = statement->asAssignment()) {
                        return parent().variables().getVariable(assignment->left()) == variable;
                    } else if (auto touch = statement->asTouch()) {
                        return touch->term()->isWrite() && parent().variables().getVariable(touch->term()) == variable;
                    }
                    return false;
                }) == end;
            } else {
                /* All definitions must dominate all uses. */
                foreach (const auto &def, variable->termsAndLocations()) {
                    if (def.term->isWrite()) {
                        foreach (const auto &use, variable->termsAndLocations()) {
                            if (use.term->isRead()) {
                                if (!isDominating(def.term, use.term)) {
                                    return false;
                                }
                            }
                        }
                    }
                }
            }

            return true;
        }
        case Term::UNARY_OPERATOR: {
            auto unary = source->asUnaryOperator();
            return canBeMoved(unary->operand(), destination);
        }
        case Term::BINARY_OPERATOR: {
            auto binary = source->asBinaryOperator();
            return canBeMoved(binary->left(), destination) && canBeMoved(binary->right(), destination);
        }
        case Term::CHOICE: {
            auto choice = source->asChoice();
            if (!dataflow_.getDefinitions(choice->preferredTerm()).empty()) {
                return canBeMoved(choice->preferredTerm(), destination);
            } else {
                return canBeMoved(choice->defaultTerm(), destination);
            }
        }
        default: {
            unreachable();
        }
    }
}

bool DefinitionGenerator::isSubstitutableRead(const Term *read) const {
    assert(read != nullptr);
    assert(read->isRead());
    assert(liveness_.isLive(read) && "We should not care about dead reads.");

    auto write = getTheOnlyDefinition(read);
    return write && isSubstitutableWrite(write);
}

const Term *DefinitionGenerator::getTheOnlyDefinition(const Term *read) const {
    assert(read != nullptr);
    assert(read->isRead());

    const auto &definitions = dataflow_.getDefinitions(read);

    if (definitions.chunks().size() == 1 &&
        definitions.chunks().front().definitions().size() == 1)
    {
        return definitions.chunks().front().definitions().front();
    }
    return nullptr;
}

bool DefinitionGenerator::isDominating(const Term *write, const Term *read) const {
    assert(write != nullptr);
    assert(write->isWrite());
    assert(read != nullptr);
    assert(read->isRead());

    if (write->statement()->basicBlock() == read->statement()->basicBlock()) {
        if (write->statement()->instruction() && read->statement()->instruction() &&
            write->statement()->instruction() != read->statement()->instruction())
        {
            return write->statement()->instruction()->addr() < read->statement()->instruction()->addr();
        } else {
            const auto &statements = read->statement()->basicBlock()->statements();
            assert(nc::contains(statements, write->statement()));
            assert(nc::contains(statements, read->statement()));
            return std::find(
                std::find(statements.begin(), statements.end(), write->statement()),
                statements.end(),
                read->statement()) != statements.end();
        }
    } else {
        return dominators_->isDominating(write->statement()->basicBlock(), read->statement()->basicBlock());
    }
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

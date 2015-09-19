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

#include "LivenessAnalyzer.h"

#include <cassert>

#include <nc/common/Foreach.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/CallHook.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/ReturnHook.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/cflow/BasicNode.h>
#include <nc/core/ir/cflow/Graph.h>
#include <nc/core/ir/cflow/Switch.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Utils.h>
#include <nc/core/ir/dflow/Value.h>

#include "Liveness.h"

namespace nc {
namespace core {
namespace ir {
namespace liveness {

LivenessAnalyzer::LivenessAnalyzer(Liveness &liveness, const Function *function,
    const dflow::Dataflow &dataflow, const arch::Architecture *architecture,
    const cflow::Graph *regionGraph, const calling::Hooks &hooks,
    const calling::Signatures *signatures, const LogToken &log
):
    liveness_(liveness), function_(function), dataflow_(dataflow),
    architecture_(architecture), regionGraph_(regionGraph), hooks_(hooks),
    signatures_(signatures), log_(log)
{}

void LivenessAnalyzer::analyze() {
    computeInvisibleJumps();

    foreach (const BasicBlock *basicBlock, function_->basicBlocks()) {
        foreach (const Statement *statement, basicBlock->statements()) {
            computeLiveness(statement);
        }
    }
}

void LivenessAnalyzer::computeInvisibleJumps() {
    if (!regionGraph_) {
        return;
    }

    invisibleJumps_.clear();

    foreach (auto node, regionGraph_->nodes()) {
        if (auto region = node->as<cflow::Region>()) {
            if (auto witch = region->as<cflow::Switch>()) {
                if (witch->boundsCheckNode()) {
                    invisibleJumps_.push_back(witch->boundsCheckNode()->basicBlock()->getJump());
                }
                invisibleJumps_.push_back(witch->switchNode()->basicBlock()->getJump());
                makeLive(witch->switchTerm());
            }
        }
    }

    std::sort(invisibleJumps_.begin(), invisibleJumps_.end());
}

void LivenessAnalyzer::computeLiveness(const Statement *statement) {
    switch (statement->kind()) {
        case Statement::INLINE_ASSEMBLY:
            break;
        case Statement::ASSIGNMENT: {
            auto assignment = statement->asAssignment();
            auto memoryLocation = dataflow_.getMemoryLocation(assignment->left());

            if (!memoryLocation || architecture_->isGlobalMemory(memoryLocation)) {
                makeLive(assignment->left());
            }
            break;
        }
        case Statement::JUMP: {
            const Jump *jump = statement->asJump();

            if (!std::binary_search(invisibleJumps_.begin(), invisibleJumps_.end(), jump)) {
                if (jump->condition()) {
                    makeLive(jump->condition());
                }
                if (jump->thenTarget().address() && !dflow::isReturnAddress(jump->thenTarget(), dataflow_)) {
                    makeLive(jump->thenTarget().address());
                }
                if (jump->elseTarget().address() && !dflow::isReturnAddress(jump->elseTarget(), dataflow_)) {
                    makeLive(jump->elseTarget().address());
                }

                if (signatures_ && dflow::isReturn(jump, dataflow_)) {
                    if (auto signature = signatures_->getSignature(function_)) {
                        if (signature->returnValue()) {
                            if (auto returnHook = hooks_.getReturnHook(jump)) {
                                makeLive(returnHook->getReturnValueTerm(signature->returnValue().get()));
                            }
                        }
                    }
                }
            }
            break;
        }
        case Statement::CALL: {
            const Call *call = statement->asCall();

            makeLive(call->target());

            if (signatures_) {
                if (auto signature = signatures_->getSignature(call)) {
                    if (auto callHook = hooks_.getCallHook(call)) {
                        foreach (const auto &argument, signature->arguments()) {
                            makeLive(callHook->getArgumentTerm(argument.get()));
                        }
                    }
                }
            }

            break;
        }
        case Statement::HALT:
            break;
        case Statement::TOUCH:
            break;
        case Statement::CALLBACK:
            break;
        case Statement::REMEMBER_REACHING_DEFINITIONS:
            break;
        default:
            log_.warning(tr("%1: Unknown statement kind: %2.").arg(Q_FUNC_INFO).arg(statement->kind()));
            break;
    }
}

void LivenessAnalyzer::propagateLiveness(const Term *term) {
    assert(term != nullptr);

#ifdef NC_PREFER_CONSTANTS_TO_EXPRESSIONS
    if (term->isRead() && dataflow_.getValue(term)->abstractValue().isConcrete()) {
        return;
    }
#endif

    switch (term->kind()) {
        case Term::INT_CONST:
            break;
        case Term::INTRINSIC:
            break;
        case Term::MEMORY_LOCATION_ACCESS: {
            if (term->isRead()) {
                foreach (auto &chunk, dataflow_.getDefinitions(term).chunks()) {
                    foreach (const Term *definition, chunk.definitions()) {
                        makeLive(definition);
                    }
                }
            } else if (term->isWrite()) {
                if (auto source = term->source()) {
                    makeLive(source);
                }
            }
            break;
        }
        case Term::DEREFERENCE: {
            if (term->isRead()) {
                foreach (auto &chunk, dataflow_.getDefinitions(term).chunks()) {
                    foreach (const Term *definition, chunk.definitions()) {
                        makeLive(definition);
                    }
                }
            } else if (term->isWrite()) {
                if (auto source = term->source()) {
                    makeLive(source);
                }
            }

            if (!dataflow_.getMemoryLocation(term)) {
                makeLive(term->asDereference()->address());
            }
            break;
        }
        case Term::UNARY_OPERATOR: {
            const UnaryOperator *unary = term->asUnaryOperator();
            makeLive(unary->operand());
            break;
        }
        case Term::BINARY_OPERATOR: {
            const BinaryOperator *binary = term->asBinaryOperator();
            makeLive(binary->left());
            makeLive(binary->right());
            break;
        }
        default:
            log_.warning(tr("%1: Unknown term kind: %2.").arg(Q_FUNC_INFO).arg(term->kind()));
            break;
    }
}

void LivenessAnalyzer::makeLive(const Term *term) {
    assert(term != nullptr);
    if (!liveness_.isLive(term)) {
        liveness_.makeLive(term);
        propagateLiveness(term);
    }
}

} // namespace liveness
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

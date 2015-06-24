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

#include "TypeAnalyzer.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Foreach.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/CallHook.h>
#include <nc/core/ir/calling/EntryHook.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/ReturnHook.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/dflow/Utils.h>
#include <nc/core/ir/dflow/Value.h>
#include <nc/core/ir/liveness/Livenesses.h>
#include <nc/core/ir/vars/Variables.h>

#include "Type.h"
#include "Types.h"

namespace nc {
namespace core {
namespace ir {
namespace types {

void TypeAnalyzer::analyze() {
    uniteTypesOfAssignedTerms();
    uniteVariableTypes();
    uniteArgumentTypes();
    markStackPointersAsPointers();

    /*
     * Recompute types until reaching fixpoint.
     */
    bool changed;
    do {
        changed = false;

        foreach (const Function *function, functions_.list()) {
            while (analyze(function)) {
                changed = true;
                canceled_.poll();
            }
            canceled_.poll();
        }
    } while (changed);
}

void TypeAnalyzer::uniteTypesOfAssignedTerms() {
    /*
     * Unite types of arguments of left and right hand sides of assignments.
     */
    foreach (const auto &functionAndLiveness, livenesses_) {
        foreach (const Term *term, functionAndLiveness.second->liveTerms()) {
            if (auto source = term->source()) {
                types_.getType(term)->unionSet(types_.getType(source));
            }
        }
    }
}

namespace {

void uniteTypes(Type *&a, Type *b) {
    if (a == nullptr) {
        a = b;
    } else {
        a->unionSet(b);
    }
};

} // anonymous namespace

void TypeAnalyzer::uniteVariableTypes() {
    foreach (const vars::Variable *variable, variables_.list()) {
        boost::unordered_map<MemoryLocation, Type *> location2type;

        foreach (const auto &termAndLocation, variable->termsAndLocations()) {
            uniteTypes(location2type[termAndLocation.location], types_.getType(termAndLocation.term));
        }
    }
}

void TypeAnalyzer::uniteArgumentTypes() {
    foreach (auto function, functions_.list()) {
        const auto &dataflow = *dataflows_.at(function);

        if (auto entryHook = hooks_.getEntryHook(function)) {
            if (auto signature = signatures_.getSignature(function)) {
                foreach (const auto &term, signature->arguments()) {
                    types_.getType(term.get())->unionSet(types_.getType(entryHook->getArgumentTerm(term.get())));
                }
            }
        }

        foreach (auto basicBlock, function->basicBlocks()) {
            foreach (auto statement, basicBlock->statements()) {
                if (auto call = statement->asCall()) {
                    if (auto callHook = hooks_.getCallHook(call)) {
                        if (auto signature = signatures_.getSignature(call)) {
                            foreach (const auto &term, signature->arguments()) {
                                types_.getType(term.get())->unionSet(types_.getType(callHook->getArgumentTerm(term.get())));
                            }
                            if (auto returnValue = signature->returnValue().get()) {
                                types_.getType(returnValue)->unionSet(types_.getType(callHook->getReturnValueTerm(returnValue)));
                            }
                        }
                    }
                } else if (auto jump = statement->asJump()) {
                    if (dflow::isReturn(jump, dataflow)) {
                        if (auto returnHook = hooks_.getReturnHook(jump)) {
                            if (auto signature = signatures_.getSignature(function)) {
                                if (auto returnValue = signature->returnValue().get()) {
                                    types_.getType(returnValue)->unionSet(types_.getType(returnHook->getReturnValueTerm(returnValue)));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void TypeAnalyzer::markStackPointersAsPointers() {
    foreach (const auto &functionAndLiveness, livenesses_) {
        const auto &dataflow = *dataflows_.at(functionAndLiveness.first);
        foreach (const auto term, functionAndLiveness.second->liveTerms()) {
            if (dataflow.getValue(term)->isStackOffset()) {
                types_.getType(term)->makePointer();
            }
        }
    }
}

bool TypeAnalyzer::analyze(const Function *function) {
    const auto &liveness = *livenesses_.at(function);

    /*
     * Going in both directions makes the process
     * converge much faster on some examples.
     */
    foreach (const Term *term, liveness.liveTerms()) {
        analyze(term);
    }
    reverse_foreach (const Term *term, liveness.liveTerms()) {
        analyze(term);
    }

    bool changed = false;
    foreach (const Term *term, liveness.liveTerms()) {
        if (types_.getType(term)->changed()) {
            changed = true;
        }
    }
    return changed;
}

void TypeAnalyzer::analyze(const Term *term) {
    switch (term->kind()) {
        case Term::INT_CONST: /* FALLTHROUGH */
        case Term::INTRINSIC: /* FALLTHROUGH */
        case Term::MEMORY_LOCATION_ACCESS:
            break;
        case Term::DEREFERENCE: {
            auto dereference = term->asDereference();
            types_.getType(dereference->address())->makePointer(types_.getType(dereference));
            break;
        }
        case Term::UNARY_OPERATOR:
            analyze(term->asUnaryOperator());
            break;
        case Term::BINARY_OPERATOR:
            analyze(term->asBinaryOperator());
            break;
        case Term::CHOICE:
            break;
        default:
            unreachable();
            break;
    }
}

void TypeAnalyzer::analyze(const UnaryOperator *unary) {
    Type *type = types_.getType(unary);
    Type *operandType = types_.getType(unary->operand());

    switch (unary->operatorKind()) {
        case UnaryOperator::NOT: /* FALLTHROUGH */
            operandType->makeInteger();
            type->makeInteger();
            break;
        case UnaryOperator::NEGATION:
            operandType->makeInteger();
            type->makeInteger();
            operandType->makeSigned();
            type->makeSigned();
            break;
        case UnaryOperator::SIGN_EXTEND:
            operandType->makeSigned();
            break;
        case UnaryOperator::ZERO_EXTEND:
            if (operandType->isSigned()) {
                type->makeUnsigned();
            }
            break;
        case UnaryOperator::TRUNCATE:
            break;
        default:
            unreachable();
            break;
    }
}

void TypeAnalyzer::analyze(const BinaryOperator *binary) {
    /* Be careful: these pointers become invalid after the first call to unionSet(). */
    Type *type = types_.getType(binary);
    Type *leftType = types_.getType(binary->left());
    Type *rightType = types_.getType(binary->right());

    const auto &dataflow = *dataflows_.at(binary->statement()->basicBlock()->function());
    const dflow::Value *value = dataflow.getValue(binary->left());
    const dflow::Value *leftValue = dataflow.getValue(binary->left());
    const dflow::Value *rightValue = dataflow.getValue(binary->right());

    switch (binary->operatorKind()) {
        case BinaryOperator::AND: /* FALLTHROUGH */
        case BinaryOperator::OR: /* FALLTHROUGH */
        case BinaryOperator::XOR:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeUnsigned();
            rightType->makeUnsigned();
            type->makeUnsigned();
            break;

        case BinaryOperator::SHL:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            rightType->makeUnsigned();
            if (leftType->isSigned()) {
                type->makeSigned();
            }
            if (leftType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
            }
            if (type->isUnsigned()) {
                leftType->makeUnsigned();
            }

            if (rightValue->abstractValue().isConcrete()) {
                type->updateFactor(leftType->factor() *
                    shiftLeft<ConstantValue>(1, rightValue->abstractValue().asConcrete().value()));
            }
            break;

        case BinaryOperator::SHR:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeUnsigned();
            type->makeUnsigned();
            break;

        case BinaryOperator::SAR:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeSigned();
            type->makeSigned();
            break;

        case BinaryOperator::ADD:
            if (leftType->isInteger() && rightType->isInteger()) {
                type->makeInteger();
            }
            if ((leftType->isInteger() && rightType->isPointer()) ||
                (leftType->isPointer() && rightType->isInteger())) {
                type->makePointer();
            }
            if (type->isInteger()) {
                leftType->makeInteger();
                rightType->makeInteger();
            }
            if (type->isPointer()) {
                if (leftType->isInteger()) {
                    rightType->makePointer();
                }
                if (rightType->isInteger()) {
                    leftType->makePointer();
                }
                if (leftType->isPointer()) {
                    rightType->makeInteger();
                }
                if (rightType->isPointer()) {
                    leftType->makeInteger();
                }
                if (!leftType->isPointer() && !rightType->isPointer()) {
                    if (leftValue->isProduct()) {
                        rightType->makePointer();
                    } else if (rightValue->isProduct()) {
                        leftType->makePointer();
                    } else if (leftValue->abstractValue().isConcrete()) {
                        if (leftValue->abstractValue().asConcrete().value() < 4096) {
                            leftType->makeInteger();
                        } else {
                            leftType->makePointer();
                        }
                    } else if (rightValue->abstractValue().isConcrete()) {
                        if (rightValue->abstractValue().asConcrete().value() < 4096) {
                            rightType->makeInteger();
                        } else {
                            rightType->makePointer();
                        }
                    }
                }
            }

            if (leftType->isUnsigned() || rightType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (leftType->isSigned() && rightType->isSigned()) {
                type->makeSigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
                rightType->makeSigned();
            }
            if (type->isUnsigned()) {
                if (leftType->isSigned()) {
                    rightType->makeUnsigned();
                }
                if (rightType->isSigned()) {
                    rightType->makeUnsigned();
                }
            }

            if (rightValue->abstractValue().isConcrete()) {
                if (type == leftType) {
                    type->updateFactor(rightValue->abstractValue().asConcrete().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        leftType->addOffset(rightValue->abstractValue().asConcrete().signedValue(), type);
                    }
#endif
                }
            }
            if (leftValue->abstractValue().isConcrete()) {
                if (type == rightType) {
                    type->updateFactor(leftValue->abstractValue().asConcrete().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        rightType->addOffset(leftValue->abstractValue().asConcrete().signedValue(), type);
                    }
#endif
                }
            }

            if (leftType->isPointer() && rightValue->isProduct()) {
                type->makePointer(leftType->pointee());
            }
            if (rightType->isPointer() && leftValue->isProduct()) {
                type->makePointer(rightType->pointee());
            }
            break;

        case BinaryOperator::SUB:
            if (leftType->isInteger() && rightType->isInteger()) {
                type->makeInteger();
            }
            if (leftType->isPointer() && rightType->isInteger()) {
                type->makePointer();
            }
            if (type->isPointer()) {
                leftType->makePointer();
                rightType->makeInteger();
            }

            if (leftType->isUnsigned() || rightType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (leftType->isSigned() && rightType->isSigned()) {
                type->makeSigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
                rightType->makeSigned();
            }
            if (type->isUnsigned()) {
                if (leftType->isSigned()) {
                    rightType->makeUnsigned();
                }
                if (rightType->isSigned()) {
                    rightType->makeUnsigned();
                }
            }

            if (rightValue->abstractValue().isConcrete()) {
                if (type == leftType) {
                    type->updateFactor(rightValue->abstractValue().asConcrete().absoluteValue());
                } else {
#ifdef NC_STRUCT_RECOVERY
                    if (!value->isStackOffset()) {
                        leftType->addOffset(-rightValue->abstractValue().asConcrete().signedValue(), type);
                    }
#endif
                }
            }

            if (leftType->isPointer() && rightValue->isProduct()) {
                type->makePointer(leftType->pointee());
            }
            break;

        case BinaryOperator::MUL:
            type->makeInteger();
            leftType->makeInteger();
            rightType->makeInteger();

            if (leftType->isUnsigned() || rightType->isUnsigned()) {
                type->makeUnsigned();
            }
            if (leftType->isSigned() && rightType->isSigned()) {
                type->makeSigned();
            }
            if (type->isSigned()) {
                leftType->makeSigned();
                rightType->makeSigned();
            }
            if (type->isUnsigned()) {
                if (leftType->isSigned()) {
                    rightType->makeUnsigned();
                }
                if (rightType->isSigned()) {
                    rightType->makeUnsigned();
                }
            }

            if (rightValue->abstractValue().isConcrete()) {
                type->updateFactor(leftType->factor() * rightValue->abstractValue().asConcrete().value());
            }
            if (leftValue->abstractValue().isConcrete()) {
                type->updateFactor(rightType->factor() * leftValue->abstractValue().asConcrete().value());
            }
            break;

        case BinaryOperator::SIGNED_DIV:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeSigned();
            rightType->makeSigned();
            type->makeSigned();
            break;

        case BinaryOperator::SIGNED_REM:
            leftType->makeInteger();
            rightType->makeInteger();
            type->makeInteger();

            leftType->makeSigned();
            rightType->makeSigned();
            type->makeSigned();
            break;

        case BinaryOperator::UNSIGNED_DIV:
            type->makeInteger();
            leftType->makeInteger();
            rightType->makeInteger();

            if (leftType->isSigned()) {
                rightType->makeUnsigned();
            }
            if (rightType->isUnsigned()) {
                leftType->makeSigned();
            }
            type->makeUnsigned();
            break;

        case BinaryOperator::UNSIGNED_REM:
            type->makeInteger();
            leftType->makeInteger();
            rightType->makeInteger();

            if (leftType->isSigned()) {
                rightType->makeUnsigned();
            }
            if (rightType->isUnsigned()) {
                leftType->makeSigned();
            }
            type->makeUnsigned();
            break;

        case BinaryOperator::EQUAL:
            leftType->unionSet(rightType);
            break;

        case BinaryOperator::SIGNED_LESS: /* FALLTHROUGH */
        case BinaryOperator::SIGNED_LESS_OR_EQUAL: /* FALLTHROUGH */
            leftType->makeSigned();
            rightType->makeSigned();
            leftType->unionSet(rightType);
            break;

        case BinaryOperator::UNSIGNED_LESS: /* FALLTHROUGH */
        case BinaryOperator::UNSIGNED_LESS_OR_EQUAL: /* FALLTHROUGH */
            if (rightType->isSigned()) {
                leftType->makeUnsigned();
            } else if (leftType->isSigned()) {
                rightType->makeUnsigned();
            } else {
                leftType->makeUnsigned();
                rightType->makeUnsigned();
            }
            leftType->unionSet(rightType);
            break;

        default:
            unreachable();
            break;
    }
}

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */

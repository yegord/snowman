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

#include "PatternRecognition.h"

#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Utils.h>
#include <nc/core/ir/dflow/Value.h>

#include "ArrayAccess.h"
#include "BoundsCheck.h"

namespace nc {
namespace core {
namespace ir {
namespace misc {

namespace {

ArrayAccess recognizeArrayAccess(const Term *base, const Term *multiplication, const dflow::Dataflow &dataflow) {
    const dflow::Value *baseValue = dataflow.getValue(base);

    if (baseValue->abstractValue().isConcrete()) {
        if (const BinaryOperator *binary = multiplication->asBinaryOperator()) {
            if (binary->operatorKind() == BinaryOperator::SHL) {
                const dflow::Value *shiftValue = dataflow.getValue(binary->right());

                if (shiftValue->abstractValue().isConcrete()) {
                    return ArrayAccess(baseValue->abstractValue().asConcrete().value(), 1 << shiftValue->abstractValue().asConcrete().value(), binary->left());
                }
            } else if (binary->operatorKind() == BinaryOperator::MUL) {
                const dflow::Value *leftValue = dataflow.getValue(binary->left());
                if (leftValue->abstractValue().isConcrete()) {
                    return ArrayAccess(baseValue->abstractValue().asConcrete().value(), leftValue->abstractValue().asConcrete().value(), binary->right());
                }

                const dflow::Value *rightValue = dataflow.getValue(binary->right());
                if (rightValue->abstractValue().isConcrete()) {
                    return ArrayAccess(baseValue->abstractValue().asConcrete().value(), rightValue->abstractValue().asConcrete().value(), binary->left());
                }
            }
        }
    }

    return ArrayAccess();
}

} // anonymous namespace

ArrayAccess recognizeArrayAccess(const Term *term, const dflow::Dataflow &dataflow) {
    assert(term != nullptr);

    term = dflow::getFirstCopy(term, dataflow);

    if (const Dereference *dereference = term->asDereference()) {
        const Term *address = dflow::getFirstCopy(dereference->address(), dataflow);

        if (const BinaryOperator *addition = address->asBinaryOperator()) {
            if (addition->operatorKind() == BinaryOperator::ADD) {
                const Term *left = dflow::getFirstCopy(addition->left(), dataflow);
                const Term *right = dflow::getFirstCopy(addition->right(), dataflow);

                if (auto result = recognizeArrayAccess(left, right, dataflow)) {
                    return result;
                }
                if (auto result = recognizeArrayAccess(right, left, dataflow)) {
                    return result;
                }
            }
        }
    }

    return ArrayAccess();
}

BoundsCheck recognizeBoundsCheck(const Jump *jump, const BasicBlock *ifPassed, const dflow::Dataflow &dataflow) {
    bool inverse;
    if (jump->thenTarget().basicBlock() == ifPassed) {
        inverse = false;
    } else if (jump->elseTarget().basicBlock() == ifPassed) {
        inverse = true;
    } else {
        return BoundsCheck();
    }

    auto condition = getFirstCopy(jump->condition(), dataflow);

    for (std::size_t niterations = 0; niterations < 10; ++niterations) {
        if (auto unary = condition->as<UnaryOperator>()) {
            if (unary->operatorKind() == UnaryOperator::NOT && unary->size() == 1) {
                condition = getFirstCopy(unary->operand(), dataflow);
                inverse = !inverse;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    if (auto binary = condition->as<BinaryOperator>()) {
        if (!inverse) {
            switch (binary->operatorKind()) {
                case BinaryOperator::UNSIGNED_LESS_OR_EQUAL: {
                    const dflow::Value *rightValue = dataflow.getValue(binary->right());
                    if (rightValue->abstractValue().isConcrete()) {
                        return BoundsCheck(binary->left(), rightValue->abstractValue().asConcrete().value(), jump->elseTarget().basicBlock());
                    }
                    break;
                }
                case BinaryOperator::UNSIGNED_LESS: {
                    const dflow::Value *rightValue = dataflow.getValue(binary->right());
                    if (rightValue->abstractValue().isConcrete()) {
                        return BoundsCheck(binary->left(), rightValue->abstractValue().asConcrete().value() - 1, jump->elseTarget().basicBlock());
                    }
                    break;
                }
            }
        } else {
            switch (binary->operatorKind()) {
                case BinaryOperator::UNSIGNED_LESS: {
                    const dflow::Value *leftValue = dataflow.getValue(binary->left());
                    if (leftValue->abstractValue().isConcrete()) {
                        return BoundsCheck(binary->right(), leftValue->abstractValue().asConcrete().value(), jump->thenTarget().basicBlock());
                    }
                    break;
                }
                case BinaryOperator::UNSIGNED_LESS_OR_EQUAL: {
                    const dflow::Value *leftValue = dataflow.getValue(binary->left());
                    if (leftValue->abstractValue().isConcrete()) {
                        return BoundsCheck(binary->right(), leftValue->abstractValue().asConcrete().value() - 1, jump->thenTarget().basicBlock());
                    }
                    break;
                }
            }
        }
    }

    return BoundsCheck();
}

}}}} // namespace nc::core::ir::misc

/* vim:set et sts=4 sw=4: */

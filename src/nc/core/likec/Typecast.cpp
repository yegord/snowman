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

#include "Typecast.h"

#include <nc/common/make_unique.h>

#include "BinaryOperator.h"
#include "IntegerConstant.h"
#include "PrintContext.h"
#include "MemberAccessOperator.h"
#include "StructType.h"
#include "UnaryOperator.h"
#include "Types.h"
#include "Tree.h"

namespace nc {
namespace core {
namespace likec {

void Typecast::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(operand());
}

Expression *Typecast::rewrite() {
    rewriteChild(operand_);

    /* Convert cast of pointer to a structure to a cast of pointer to its first field. */
    if (type_->isPointer() && !type_->isStructurePointer()) {
        if (const PointerType *pointerType = operand()->getType()->as<PointerType>()) {
            if (const StructType *structType = pointerType->pointeeType()->as<StructType>()) {
                if (const MemberDeclaration *member = structType->getMember(0)) {
                    setOperand(std::make_unique<UnaryOperator>(tree(), UnaryOperator::REFERENCE,
                        std::make_unique<MemberAccessOperator>(tree(), MemberAccessOperator::ARROW, releaseOperand(), member)));
                }
            }
        }
    }

    /*
     * (int32_t*)(int64_t)expr -> (int32_t*)expr
     */
    if (type_->isScalar()) {
        if (Typecast *typecast = operand()->as<Typecast>()) {
            const Type *operandType = typecast->operand()->getType();

            if (typecast->type()->isScalar() &&
                operandType->isScalar() &&
                type_->size() == typecast->type()->size() &&
                typecast->type()->size() == operandType->size())
            {
                setOperand(typecast->releaseOperand());
            }
        }
    }

    /*
     * (int32_t*)((uintptr_t)expr + const) -> (int32_t)(expr + const / sizeof(int32_t))
     */
    if (auto pointerType = type_->as<PointerType>()) {
        if (pointerType->pointeeType()->size() && pointerType->pointeeType()->size() % CHAR_BIT == 0) {
            ByteSize pointeeSizeInBytes = pointerType->pointeeType()->size() / CHAR_BIT;

            if (auto binary = operand()->as<BinaryOperator>()) {
                if (binary->operatorKind() == BinaryOperator::ADD) {
                    auto rewrite = [&](Expression *binaryLeft, Expression *binaryRight) -> Expression * {
                        if (auto typecast = binaryLeft->as<Typecast>()) {
                            if (typecast->type()->isInteger() &&
                                typecast->type()->size() == tree().pointerSize() &&
                                typecast->operand()->getType()->isPointer())
                            {
                                if (auto constant = binaryRight->as<IntegerConstant>()) {
                                    if (constant->type()->size() <= tree().pointerSize() &&
                                        constant->value().value() % pointeeSizeInBytes == 0)
                                    {
                                        return new BinaryOperator(tree(),
                                            BinaryOperator::ADD,
                                            std::make_unique<Typecast>(tree(),
                                                pointerType,
                                                typecast->releaseOperand()),
                                            std::make_unique<IntegerConstant>(tree(),
                                                constant->value().value() / pointeeSizeInBytes,
                                                constant->type()));
                                    }
                                }
                            }
                        }
                        return nullptr;
                    };

                    if (auto result = rewrite(binary->left(), binary->right())) {
                        return result;
                    } else if (auto result = rewrite(binary->right(), binary->left())) {
                        return result;
                    }
                }
            }
        }
    }

    /* This really must be the last rule. */
    if (type_ == operand()->getType()) {
        return releaseOperand().release();
    }

    return this;
}

void Typecast::doPrint(PrintContext &context) const {
    context.out() << '(' << *type() << ')';
    bool braces = operand()->is<BinaryOperator>();
    if (braces) {
        context.out() << '(';
    }
    operand()->print(context);
    if (braces) {
        context.out() << ')';
    }
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

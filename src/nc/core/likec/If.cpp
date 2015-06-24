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

#include "If.h"

#include <nc/common/make_unique.h>

#include "Block.h"
#include "PrintContext.h"
#include "UnaryOperator.h"

namespace nc {
namespace core {
namespace likec {

void If::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    fun(condition());
    fun(thenStatement());

    if (elseStatement() != nullptr) {
        fun(elseStatement());
    }
}

If *If::rewrite() {
    assert(condition_);
    assert(thenStatement_);

    rewriteChild(thenStatement_);
    if (elseStatement_) {
        rewriteChild(elseStatement_);

        if (auto block = elseStatement_->as<Block>()) {
            if (block->statements().empty()) {
                elseStatement_.reset();
            }
        }

        if (elseStatement_) {
            if (auto block = thenStatement_->as<Block>()) {
                if (block->statements().empty()) {
                    thenStatement_ = std::move(elseStatement_);
                    condition_ = std::make_unique<UnaryOperator>(tree(), UnaryOperator::LOGICAL_NOT, std::move(condition_));
                }
            }
        }
    }
    rewriteChild(condition_);

    return this;
}

void If::doPrint(PrintContext &context) const {
    context.out() << "if (";
    condition_->print(context);
    context.out() << ") ";
    printNestedStatement(thenStatement(), context);
    if (elseStatement_) {
        context.out() << " else ";
        printNestedStatement(elseStatement(), context);
    }
}

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

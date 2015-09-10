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

#include "FunctionDeclaration.h"

#include <nc/common/Foreach.h>

#include "Tree.h"
#include "PrintContext.h"

namespace nc {
namespace core {
namespace likec {

FunctionDeclaration::FunctionDeclaration(Tree &tree, QString identifier, const Type *returnType, bool variadic):
    Declaration(FUNCTION_DECLARATION, std::move(identifier)),
    type_(new FunctionPointerType(tree.pointerSize(), returnType, variadic)),
    functionIdentifier_(new FunctionIdentifier(this))
{
    assert(returnType != nullptr);
}

FunctionDeclaration::FunctionDeclaration(Tree &tree, int declarationKind, QString identifier, const Type *returnType, bool variadic):
    Declaration(declarationKind, std::move(identifier)),
    type_(new FunctionPointerType(tree.pointerSize(), returnType, variadic)),
    functionIdentifier_(new FunctionIdentifier(this))
{
    assert(returnType != nullptr);
}

void FunctionDeclaration::addArgument(std::unique_ptr<ArgumentDeclaration> argument) {
    type_->addArgumentType(argument->type());
    arguments_.push_back(std::move(argument));
}

void FunctionDeclaration::doCallOnChildren(const std::function<void(TreeNode *)> &fun) {
    foreach (const auto &argument, arguments_) {
        fun(argument.get());
    }
}

void FunctionDeclaration::doPrint(PrintContext &context) const {
    printComment(context);
    printSignature(context);
    context.out() << ';';
}

void FunctionDeclaration::printSignature(PrintContext &context) const {
    context.out() << *type()->returnType() << ' ';
    functionIdentifier()->print(context);
    context.out() << '(';

    bool comma = false;
    foreach (const auto &argument, arguments_) {
        if (comma) {
            context.out() << ", ";
        } else {
            comma = true;
        }
        argument->print(context);
    }

    if (type()->variadic()) {
        if (comma) {
            context.out() << ", ";
        }
        context.out() << "...";
    }
    
    context.out() << ')';
}

} /* namespace likec */
} // namespace core
} /* namespace nc */

/* vim:set et sts=4 sw=4: */

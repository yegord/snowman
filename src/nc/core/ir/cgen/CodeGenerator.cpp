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

#include "CodeGenerator.h"

#include <nc/common/CancellationToken.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Reader.h>
#include <nc/core/image/Relocation.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/types/Type.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/vars/Variable.h>
#include <nc/core/likec/FunctionDefinition.h>
#include <nc/core/likec/IntegerConstant.h>
#include <nc/core/likec/StructType.h>
#include <nc/core/likec/StructTypeDeclaration.h>
#include <nc/core/likec/Tree.h>
#include <nc/core/likec/Typecast.h>

#include "DefinitionGenerator.h"
#include "NameGenerator.h"

namespace nc {
namespace core {
namespace ir {
namespace cgen {

void CodeGenerator::makeCompilationUnit() {
    tree().setPointerSize(image().platform().architecture()->bitness());
    tree().setRoot(std::make_unique<likec::CompilationUnit>(tree()));

    foreach (const Function *function, functions().list()) {
        makeFunctionDefinition(function);
        cancellationToken().poll();
    }

    tree().rewriteRoot();
}

const likec::Type *CodeGenerator::makeType(const types::Type *typeTraits) {
    assert(!typeTraits || typeTraits->findSet() == typeTraits);

    if (!typeTraits) {
        return tree().makeVoidType();
    } else if (typeTraits->isPointer()) {
        if (std::find(typeCreationStack_.begin(), typeCreationStack_.end(), typeTraits) != typeCreationStack_.end()) {
            /* Circular dependency. */
            return tree().makePointerType(typeTraits->size(), tree().makeVoidType());
#ifdef NC_STRUCT_RECOVERY
        } else if (const likec::Type *structuralType = makeStructuralType(typeTraits)) {
            return tree().makePointerType(typeTraits->size(), structuralType);
#endif
        } else {
            typeCreationStack_.push_back(typeTraits);
            const likec::Type *pointee = makeType(typeTraits->pointee());
            typeCreationStack_.pop_back();

            return tree().makePointerType(typeTraits->size(), pointee);
        }
    } else if (typeTraits->isFloat()) {
        return tree().makeFloatType(typeTraits->size());
    } else {
        return tree().makeIntegerType(typeTraits->size(), typeTraits->isUnsigned());
    }
}

#ifdef NC_STRUCT_RECOVERY
const likec::StructType *CodeGenerator::makeStructuralType(const types::Type *typeTraits) {
    assert(typeTraits->findSet() == typeTraits);

    if (!typeTraits->isPointer()) {
        return nullptr;
    }

    if (typeTraits->offsets().size() < 2) {
        return nullptr;
    }

    auto i = traits2structType_.find(typeTraits);
    if (i != traits2structType_.end()) {
        return i->second;
    }

    bool isStruct = false;
    foreach (auto offset, typeTraits->offsets()) {
        ByteSize offsetValue = offset.first;
        const types::Type *offsetType = offset.second->findSet();

        if (offsetValue > 0 && offsetType == typeTraits) {
            break;
        }

        if (offsetValue > 0 && offsetType->pointee() && offsetType->pointee()->size()) {
            isStruct = true;
            break;
        }
    }
    if (!isStruct) {
        return nullptr;
    }

    auto typeDeclaration = std::make_unique<likec::StructTypeDeclaration>(tree_, QString("s%1").arg(traits2structType_.size()));

    likec::StructType *type = typeDeclaration->type();
    traits2structType_[typeTraits] = type;

    foreach (auto offset, typeTraits->offsets()) {
        ByteSize offsetValue = offset.first;
        const types::Type *offsetType = offset.second->findSet();

        if (offsetValue > 0 && offsetType == typeTraits) {
            break;
        }

        if (offsetValue >= 0 && offsetType->pointee() && offsetType->pointee()->size()) {
            if (offsetValue > type->size() / CHAR_BIT) {
                typeDeclaration->type()->addMember(std::make_unique<likec::MemberDeclaration>(
                    tree_, QString("pad%1").arg(offsetValue),
                    tree_.makeArrayType(tree_.makeIntegerType(CHAR_BIT, false), offsetValue - type->size() / CHAR_BIT)));
            }
            typeDeclaration->type()->addMember(std::make_unique<likec::MemberDeclaration>(
                tree_, QString("f%1").arg(offsetValue), makeType(offsetType->pointee())));
        }
    }

    tree().root()->addDeclaration(std::move(typeDeclaration));

    return type;
}
#endif

const likec::Type *CodeGenerator::makeVariableType(const vars::Variable *variable) {
    assert(variable != nullptr);

    foreach (auto termAndLocation, variable->termsAndLocations()) {
        if (termAndLocation.location == variable->memoryLocation()) {
            return makeType(types().getType(termAndLocation.term));
        }
    }

    return tree().makeIntegerType(variable->memoryLocation().size(), true);
}

likec::VariableDeclaration *CodeGenerator::makeGlobalVariableDeclaration(const vars::Variable *variable) {
    assert(variable != nullptr);
    assert(variable->isGlobal());

    if (auto result = nc::find(variableDeclarations_, variable)) {
        return result;
    } else {
        auto type = makeVariableType(variable);
        auto initialValue = makeInitialValue(variable->memoryLocation(), type);
        auto nameAndComment = nameGenerator().getGlobalVariableName(variable->memoryLocation());

        auto declaration = std::make_unique<likec::VariableDeclaration>(tree(),
            std::move(nameAndComment.name()),
            type,
            std::move(initialValue));
        declaration->setComment(std::move(nameAndComment.comment()));

        result = declaration.get();
        tree().root()->addDeclaration(std::move(declaration));
        variableDeclarations_[variable] = result;

        return result;
    }
}

std::unique_ptr<likec::Expression> CodeGenerator::makeInitialValue(const MemoryLocation &memoryLocation, const likec::Type *type) {
    assert(memoryLocation);
    assert(type != nullptr);

    if (memoryLocation.domain() == MemoryDomain::MEMORY &&
        memoryLocation.addr() % CHAR_BIT == 0 &&
        memoryLocation.size() % CHAR_BIT == 0 &&
        type->isScalar())
    {
        ByteAddr addr = memoryLocation.addr() / CHAR_BIT;
        ByteSize size = memoryLocation.size() / CHAR_BIT;

        if (auto value = image::Reader(&image()).readInt<ConstantValue>(
                addr, size, image().platform().architecture()->getByteOrder(MemoryDomain::MEMORY))) {
            if (auto integerType = type->as<likec::IntegerType>()) {
                return std::make_unique<likec::IntegerConstant>(tree(), *value, integerType);
            } else {
                return std::make_unique<likec::Typecast>(tree(),
                    type,
                    std::make_unique<likec::IntegerConstant>(tree(), *value, tree().makeIntegerType(type->size(), true)));
            }
        }
    }

    return nullptr;
}

likec::FunctionDeclaration *CodeGenerator::makeFunctionDeclaration(ByteAddr addr) {
    auto signature = signatures().getSignature(addr).get();
    if (!signature) {
        return nullptr;
    }

    if (auto declaration = nc::find(signature2declaration_, signature)) {
        return declaration;
    }

    DeclarationGenerator generator(*this, calling::EntryAddress(addr), signature);
    tree().root()->addDeclaration(generator.createDeclaration());
    return generator.declaration();
}

likec::FunctionDefinition *CodeGenerator::makeFunctionDefinition(const Function *function) {
    DefinitionGenerator generator(*this, function, cancellationToken());
    tree().root()->addDeclaration(generator.createDefinition());
    return generator.definition();
}

void CodeGenerator::setFunctionDeclaration(const calling::FunctionSignature *signature, likec::FunctionDeclaration *declaration) {
    assert(signature != nullptr);
    assert(declaration != nullptr);

    auto &currentDeclaration = signature2declaration_[signature];
    if (currentDeclaration == nullptr) {
        currentDeclaration = declaration;
    } else {
        declaration->setFirstDeclaration(currentDeclaration);
    }
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

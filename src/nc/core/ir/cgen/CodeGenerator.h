/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include <nc/core/ir/MemoryLocation.h>

#include "NameGenerator.h"

namespace nc {

class CancellationToken;

namespace core {

namespace image {
    class Image;
}

namespace likec {
    class FunctionDeclaration;
    class FunctionDefinition;
    class Expression;
    class StructType;
    class Tree;
    class Type;
    class VariableDeclaration;
}

namespace ir {

class Function;
class Functions;
class Term;

namespace calling {
    class CalleeId;
    class FunctionSignature;
    class Hooks;
    class Signatures;
}

namespace cflow {
    class Graphs;
}

namespace dflow {
    class Dataflows;
}

namespace liveness {
    class Livenesses;
}

namespace types {
    class Type;
    class Types;
}

namespace vars {
    class Variable;
    class Variables;
}

namespace cgen {

/**
 * LikeC code generator.
 */
class CodeGenerator: boost::noncopyable {
    likec::Tree &tree_;
    const image::Image &image_;
    const Functions &functions_;
    const calling::Hooks &hooks_;
    const calling::Signatures &signatures_;
    const dflow::Dataflows &dataflows_;
    const vars::Variables &variables_;
    const cflow::Graphs &graphs_;
    const liveness::Livenesses &livenesses_;
    const types::Types &types_;
    const CancellationToken &cancellationToken_;
    const NameGenerator nameGenerator_;

    /** Types being translated to LikeC. */
    std::vector<const ir::types::Type *> typeCreationStack_;

    /** Structural types generated for IR types. */
    boost::unordered_map<const ir::types::Type *, const likec::StructType *> traits2structType_;

    /** Already declared global variables. */
    boost::unordered_map<const vars::Variable *, likec::VariableDeclaration *> variableDeclarations_;

    /** Mapping of functions to their declarations. */
    boost::unordered_map<const calling::FunctionSignature *, likec::FunctionDeclaration *> signature2declaration_;

public:

    /**
     * Constructor.
     *
     * \param[out] tree Abstract syntax tree to generate code in.
     * \param[in] image Executable image being decompiled.
     * \param[in] functions Intermediate representation of functions.
     * \param[in] hooks Hooks manager.
     * \param[in] signatures Signatures of functions.
     * \param[in] dataflows Dataflow information for all functions.
     * \param[in] variables Information about reconstructed variables.
     * \param[in] graphs Reduced control-flow graphs.
     * \param[in] livenesses Liveness information for all functions.
     * \param[in] types Information about types.
     * \param[in] cancellationToken Cancellation token.
     */
    CodeGenerator(likec::Tree &tree, const image::Image &image, const Functions &functions, const calling::Hooks &hooks,
        const calling::Signatures &signatures, const dflow::Dataflows &dataflows, const vars::Variables &variables,
        const cflow::Graphs &graphs, const liveness::Livenesses &livenesses, const types::Types &types,
        const CancellationToken &cancellationToken
    ):
        tree_(tree), image_(image), functions_(functions), hooks_(hooks), signatures_(signatures),
        dataflows_(dataflows), variables_(variables), graphs_(graphs), livenesses_(livenesses),
        types_(types), cancellationToken_(cancellationToken), nameGenerator_(image)
    {}

    /**
     * \return Abstract syntax tree to generate code in.
     */
    likec::Tree &tree() const { return tree_; }

    /**
     * \return Executable image being decompiled.
     */
    const image::Image &image() const { return image_; }

    /**
     * \return Intermediate representation of functions.
     */
    const Functions &functions() const { return functions_; }

    /**
     * \return Hooks of calling conventions.
     */
    const calling::Hooks &hooks() const { return hooks_; }

    /**
     * \return Signatures of functions.
     */
    const calling::Signatures &signatures() const { return signatures_; }

    /**
     * \return Dataflow information for all functions.
     */
    const ir::dflow::Dataflows &dataflows() const { return dataflows_; }

    /**
     * \return Reconstructed variables.
     */
    const vars::Variables &variables() const { return variables_; }

    /**
     * \return Reduced control-flow graphs.
     */
    const cflow::Graphs &graphs() const { return graphs_; }

    /**
     * \return Liveness information for all functions.
     */
    const liveness::Livenesses &livenesses() const { return livenesses_; }

    /**
     * \return Information about types.
     */
    const types::Types &types() const { return types_; }

    /**
     * \return Cancellation token.
     */
    const CancellationToken &cancellationToken() const { return cancellationToken_; }

    const NameGenerator &nameGenerator() const { return nameGenerator_; }

    /**
     * Translates input program into LikeC compilation unit.
     */
    void makeCompilationUnit();

    /**
     * Creates high-level type object from given type traits.
     *
     * \param typeTraits Type traits.
     */
    const likec::Type *makeType(const types::Type *typeTraits);

#ifdef NC_STRUCT_RECOVERY
    /**
     * Creates high-level description of struct type from given type traits of a pointer to such struct.
     *
     * \param typeTraits Type traits.
     */
    const likec::StructType *makeStructuralType(const types::Type *typeTraits);
#endif

    /**
     * \param variable Valid pointer to a variable.
     *
     * \return Valid pointer to the LikeC type of this variable.
     */
    const likec::Type *makeVariableType(const vars::Variable *variable);

    /**
     * \param[in] variable Valid pointer to a global variable.
     *
     * \return Valid pointer to corresponding global variable declaration.
     */
    likec::VariableDeclaration *makeGlobalVariableDeclaration(const vars::Variable *variable);

    /**
     * \param[in] memoryLocation Valid memory location.
     * \param[in] type Valid pointer to its type.
     *
     * \return Pointer to the expression representing the initial value of this location.
     *         Can be nullptr.
     */
    std::unique_ptr<likec::Expression> makeInitialValue(const MemoryLocation &memoryLocation, const likec::Type *type);

    /**
     * Creates a function's declaration, if it was not yet, and adds it to the compilation unit.
     *
     * \param[in] addr Address of a function.
     *
     * \return Pointer to the declaration for a function with this address.
     *         Will be nullptr if no signature is known for the function at this address.
     */
    likec::FunctionDeclaration *makeFunctionDeclaration(ByteAddr addr);

    /**
     * Creates function's definition and adds it to the compilation unit.
     *
     * \param[in] function Function to create definition for.
     *
     * \return Created function definition.
     */
    likec::FunctionDefinition *makeFunctionDefinition(const Function *function);

    /**
     * Registers a declaration of a function.
     *
     * \param[in] signature Valid pointer to the signature of this function.
     * \param[in] declaration Valid pointer to the function's declaration.
     *
     * This function is called from DeclarationGenerator and DefinitionGenerator
     * immediately after they have created the declaration or definition.
     * Thus, when a function, whose body is being generated, is looking for
     * its own declaration, CodeGenerator already knows about it.
     */
    void setFunctionDeclaration(const calling::FunctionSignature *signature, likec::FunctionDeclaration *declaration);
};

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

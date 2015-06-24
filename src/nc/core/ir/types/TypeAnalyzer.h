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

namespace nc {

class CancellationToken;

namespace core {
namespace ir {

class BinaryOperator;
class Function;
class Functions;
class Term;
class UnaryOperator;

namespace calling {
    class Hooks;
    class Signatures;
}

namespace dflow {
    class Dataflows;
}

namespace liveness {
    class Livenesses;
}

namespace vars {
    class Variables;
}

namespace types {

class Types;

/**
 * This class performs interprocedural reconstruction of types.
 */
class TypeAnalyzer {
    Types &types_; ///< Information about terms' types.
    const Functions &functions_; ///< Intermediate representations of functions.
    const dflow::Dataflows &dataflows_; ///< Dataflow information.
    const vars::Variables &variables_; ///< Information about reconstructed variables.
    const liveness::Livenesses &livenesses_; ///< Set of terms producing actual high-level code.
    const calling::Hooks &hooks_; ///< Hooks manager.
    const calling::Signatures &signatures_; ///< Signatures of functions.
    const CancellationToken &canceled_;

public:
    /**
     * Constructor.
     *
     * \param[out] types Information about types of terms.
     * \param[in] functions Intermediate representations of functions.
     * \param[in] dataflows Dataflow information.
     * \param[in] variables Information about reconstructed variables.
     * \param[in] livenesses Liveness information.
     * \param[in] hooks Hooks manager.
     * \param[in] signatures Signatures of functions.
     * \param[in] canceled Cancellation token.
     */
    TypeAnalyzer(Types &types, const Functions &functions, const dflow::Dataflows &dataflows,
        const vars::Variables &variables, const liveness::Livenesses &livenesses,
        const calling::Hooks &hooks, const calling::Signatures &signatures,
        const CancellationToken &canceled
    ):
        types_(types), functions_(functions), dataflows_(dataflows), variables_(variables),
        livenesses_(livenesses), hooks_(hooks), signatures_(signatures), canceled_(canceled)
    {}

    /**
     * Computes type traits for all terms in all functions.
     */
    void analyze();

private:
    /**
     * Unites types of terms assigned to each other.
     */
    void uniteTypesOfAssignedTerms();

    /**
     * Unites types of terms accessing the same part of the same variable.
     */
    void uniteVariableTypes();

    /**
     * Unites types of terms representing matching arguments and return values.
     */
    void uniteArgumentTypes();

    /**
     * Marks types of terms whose value is believed to be a stack pointer
     * as pointer.
     */
    void markStackPointersAsPointers();

    /**
     * Recomputes types of terms in the given function.
     *
     * \param function Valid pointer to a function.
     *
     * \return True if types changed, false otherwise.
     */
    bool analyze(const Function *function);

    /**
     * Recomputes type of the given term.
     *
     * \param term Valid pointer to a term.
     */
    void analyze(const Term *term);

    /**
     * Recomputes type of the given term.
     *
     * \param unary Valid pointer to a unary operator term.
     */
    void analyze(const UnaryOperator *unary);

    /**
     * Recomputes type of the given term.
     *
     * \param binary Valid pointer to a binary operator term.
     */
    void analyze(const BinaryOperator *binary);
};

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */

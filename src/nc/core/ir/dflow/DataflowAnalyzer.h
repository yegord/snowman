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

#include <QCoreApplication>

#include <nc/common/CancellationToken.h>
#include <nc/common/LogToken.h>

#include <cassert>

namespace nc {

namespace core {

namespace arch {
    class Architecture;
}

namespace ir {

class CFG;
class MemoryLocation;
class Statement;
class Term;
class UnaryOperator;
class BinaryOperator;

namespace dflow {

class AbstractValue;
class Dataflow;
class ReachingDefinitions;
class Value;

/**
 * Implements a dataflow analysis based on abstract interpretation loop.
 */
class DataflowAnalyzer {
    Q_DECLARE_TR_FUNCTIONS(DataflowAnalyzer)

    Dataflow &dataflow_; ///< Dataflow information.
    const arch::Architecture *architecture_; ///< Valid pointer to architecture description.
    const CancellationToken &canceled_;
    const LogToken &log_;

public:
    /**
     * Constructor.
     *
     * \param dataflow      An object where to store results of analyses.
     * \param architecture  Valid pointer to architecture description.
     * \param canceled      Cancellation token.
     * \param log           Log token.
     */
    DataflowAnalyzer(Dataflow &dataflow, const arch::Architecture *architecture,
        const CancellationToken &canceled, const LogToken &log):
        dataflow_(dataflow), architecture_(architecture), canceled_(canceled), log_(log)
    {
        assert(architecture != nullptr);
    }

    /**
     * An object where the results of analyses are stored.
     */
    Dataflow &dataflow() const { return dataflow_; }

    /**
     * \return Valid pointer to architecture description.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * Performs joint reaching definitions and constant propagation/folding
     * analysis on the given control flow graph.
     *
     * \param[in] cfg Control flow graph to run dataflow analysis on.
     */
    void analyze(const CFG &cfg);

    /**
     * Executes a statement.
     *
     * \param statement     Valid pointer to a statement.
     * \param definitions   Reaching definitions.
     */
    void execute(const Statement *statement, ReachingDefinitions &definitions);

protected:

    /**
     * Computes the value of the given term.
     *
     * \param term          Valid pointer to a term.
     * \param definitions   Reaching definitions.
     *
     * \return Valid pointer to the computed value, owned by dataflow().
     */
    Value *computeValue(const Term *term, const ReachingDefinitions &definitions);

    /**
     * Computes the memory location of the given term.
     *
     * \param term          Valid pointer to a term.
     * \param definitions   Reaching definitions.
     *
     * \return Reference to the computed memory location, owned by dataflow().
     */
    const MemoryLocation &computeMemoryLocation(const Term *term, const ReachingDefinitions &definitions);

    /**
     * Computes reaching definitions of the given term.
     *
     * \param term              Valid pointer to a term.
     * \param memoryLocation    Memory location of this term.
     * \param definitions       Reaching definitions.
     *
     * \return Reference to the computed reaching definitions, owned by dataflow().
     */
    const ReachingDefinitions &computeReachingDefinitions(const Term *term, const MemoryLocation &memoryLocation,
                                                          const ReachingDefinitions &definitions);

    /**
     * \param memoryLocation Memory location.
     *
     * \return True if reaching definitions for this memory location must be tracked,
     *         false otherwise.
     */
    bool isTracked(const MemoryLocation &memoryLocation) const;

    /**
     * Computes the value of a term by merging the values of its reaching definitions.
     *
     * \param term              Valid pointer to a read term.
     * \param memoryLocation    Memory location of this term.
     * \param definitions       Reaching definitions of this memory location.
     *
     * \return Valid pointer to the computed value, owned by dataflow().
     */
    Value *computeValue(const Term *term, const MemoryLocation &memoryLocation, const ReachingDefinitions &definitions);

    /**
     * Executes a unary operator.
     *
     * \param[in] unary     Valid pointer to a UnaryOperator instance.
     * \param definitions   Reaching definitions.
     */
    Value *computeValue(const UnaryOperator *unary, const ReachingDefinitions &definitions);

    /**
     * Executes a binary operator.
     *
     * \param[in] binary    Valid pointer to a BinaryOperator instance.
     * \param definitions   Reaching definitions.
     */
    Value *computeValue(const BinaryOperator *binary, const ReachingDefinitions &definitions);

    /**
     * Applies a unary operator to an abstract value.
     *
     * \param unary Valid pointer to the unary operand.
     * \param a Operand value.
     *
     * \return Resulting abstract value. Its size is equal to unary->size().
     */
    AbstractValue apply(const UnaryOperator *unary, const AbstractValue &a);

    /**
     * Applies a binary operator to abstract values.
     *
     * \param binary Valid pointer to the binary operand.
     * \param a Left operand's value.
     * \param b Right operand's value.
     *
     * \return Resulting abstract value. Its size is equal to unary->size().
     */
    AbstractValue apply(const BinaryOperator *binary, const AbstractValue &a, const AbstractValue &b);

    /**
     * Remembers in the reaching definitions that the given term
     * now defines the given memory location.
     *
     * \param term              Valid pointer to a term.
     * \param memoryLocation    Memory location.
     * \param definitions       Reaching definitions.
     */
    void handleWrite(const Term *term, const MemoryLocation &memoryLocation, ReachingDefinitions &definitions);

    /**
     * Removes all definitions of the given memory location from the
     * reaching definitions.
     *
     * \param memoryLocation    Memory location.
     * \param definitions       Reaching definitions.
     */
    void handleKill(const MemoryLocation &memoryLocation, ReachingDefinitions &definitions);
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

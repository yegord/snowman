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

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */

namespace nc {
namespace core {

namespace ir {
    class Function;

    namespace calling {
        class CalleeId;
    }
}

class Context;

/**
 * Class capable of performing various kinds of analysis in the right order.
 *
 * If an architecture requires to do some analysis differently,
 * you can derive from this class, reimplement certain virtual functions,
 * and register it by calling Architecture::setMasterAnalyzer().
 * 
 * Methods of this class can be executed concurrently.
 * (Though, only on different context currently.)
 * Therefore, they all are const.
 */
class MasterAnalyzer {
    Q_DECLARE_TR_FUNCTIONS(MasterAnalyzer)

public:
    /**
     * Virtual destructor.
     */
    virtual ~MasterAnalyzer();

    /**
     * Builds an intermediate representation of a program from a set of instructions.
     *
     * \param context Context.
     */
    virtual void createProgram(Context &context) const;

    /**
     * Isolates functions in the program.
     *
     * \param context Context.
     */
    virtual void createFunctions(Context &context) const;

    /**
     * Creates the hooks manager.
     *
     * \param context Context.
     */
    virtual void createHooks(Context &context) const;

    virtual void detectCallingConventions(Context &context) const;

    /**
     * Detects and sets the calling convention of a function.
     *
     * \param context Context.
     * \param calleeId Id of the function.
     */
    virtual void detectCallingConvention(Context &context, const ir::calling::CalleeId &calleeId) const;

    /**
     * Performs dataflow analysis of all functions.
     *
     * \param context Context.
     */
    virtual void dataflowAnalysis(Context &context) const;

    /**
     * Performs dataflow analysis of the given function.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void dataflowAnalysis(Context &context, ir::Function *function) const;

    /**
     * Reconstructs signatures of functions.
     *
     * \param context Context.
     */
    virtual void reconstructSignatures(Context &context) const;

    /**
     * Reconstructs local and global variables.
     *
     * \param context Context.
     */
    virtual void reconstructVariables(Context &context) const;

    /**
     * Performs liveness analysis on all functions.
     *
     * \param context Context.
     */
    virtual void livenessAnalysis(Context &context) const;

    /**
     * Performs liveness analysis on the given function.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void livenessAnalysis(Context &context, const ir::Function *function) const;

    /**
     * Performs structural analysis of all functions.
     *
     * \param context Context.
     */
    virtual void structuralAnalysis(Context &context) const;

    /**
     * Performs structural analysis of a function.
     *
     * \param context Context.
     * \param function Valid pointer to the function.
     */
    virtual void structuralAnalysis(Context &context, const ir::Function *function) const;

    /**
     * Computes information about types.
     *
     * \param context Context.
     */
    virtual void reconstructTypes(Context &context) const;

    /**
     * Generates LikeC tree for the context.
     *
     * \param context Context.
     */
    virtual void generateTree(Context &context) const;

    /**
     * Decompiles the assembler program.
     *
     * \param context Context.
     */
    virtual void decompile(Context &context) const;

protected:
    /**
     * \param context Context.
     * \param function Valid pointer to a function.
     *
     * \return Name of the function that can be shown to the user.
     */
    virtual QString getFunctionName(Context &context, const ir::Function *function) const;
};

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

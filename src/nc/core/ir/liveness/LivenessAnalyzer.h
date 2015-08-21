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

#include <nc/common/LogToken.h>

#include <vector>

namespace nc {
namespace core {

namespace arch {
    class Architecture;
}

namespace ir {

class Function;
class Jump;
class Statement;
class Term;

namespace calling {
    class Hooks;
    class Signatures;
}

namespace cflow {
    class Graph;
}

namespace dflow {
    class Dataflow;
}

namespace liveness {

class Liveness;

/**
 * This class computes the set of terms which compute values actually used
 * in the high-level code to be generated.
 *
 * Note that this differs from the classic liveness analysis. Although a term
 * may be live in the classic definition sense, it may be not used for
 * generating actual code and therefore be dead in our sense. For example,
 * stack pointer updates do not appear in the generated code, as long as
 * offsets from the frame base can be inferred. So, they are not used,
 * although they are live in the classical sense.
 */
class LivenessAnalyzer {
    Q_DECLARE_TR_FUNCTIONS(LivenessAnalyzer)

    Liveness &liveness_;
    const Function *function_;
    const dflow::Dataflow &dataflow_;
    const arch::Architecture *architecture_;
    const cflow::Graph *regionGraph_;
    const calling::Hooks &hooks_;
    const calling::Signatures *signatures_;
    const LogToken &log_;
    std::vector<const Jump *> invisibleJumps_;

public:
    /**
     * Constructor.
     *
     * \param[out] liveness     Liveness information.
     * \param[in]  function     Valid pointer to a function to be analyzed.
     * \param[in]  dataflow     Dataflow information.
     * \param[in]  architecture Valid pointer to the architecture.
     * \param[in]  regionGraph  Pointed to the reduced control-flow graph. Can be NULL.
     * \param[in]  hooks        Hooks manager.
     * \param[in]  log          Log token.
     * \param[in]  signatures   Pointer to the function signatures. Can be NULL.
     */
    LivenessAnalyzer(Liveness &liveness, const Function *function,
        const dflow::Dataflow &dataflow, const arch::Architecture *architecture, 
        const cflow::Graph *regionGraph, const calling::Hooks &hooks,
        const calling::Signatures *signatures, const LogToken &log);

    /**
     * Computes the set of used terms.
     */
    void analyze();

private:
    /**
     * Computes the jumps that will not be visible in the generated code.
     */
    void computeInvisibleJumps();

    /**
     * Computes liveness of statement's terms based on the statement's kind.
     *
     * \param[in] statement Statement.
     */
    void computeLiveness(const Statement *statement);

    /**
     * Marks as used all the terms, used by given term in order to generate code.
     *
     * \param[in] term Used term.
     */
    void propagateLiveness(const Term *term);

    /**
     * If given term is not used, marks it as used and propagates liveness further.
     *
     * \param[in] term Term.
     */
    void makeLive(const Term *term);
};

} // namespace liveness
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

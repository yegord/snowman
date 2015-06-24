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

#include <cassert>

namespace nc {
namespace core {
namespace arch {
    class Architecture;
}
namespace ir {

class Function;

namespace calling {
    class Hooks;
}

namespace dflow {
    class Dataflows;
}

namespace vars {

class Variables;

/**
 * Class performing reconstruction of local and global variables.
 */
class VariableAnalyzer {
    Variables &variables_; ///< Mapping of terms to variables.
    const dflow::Dataflows &dataflows_; ///< Dataflow information for each function.
    const arch::Architecture *architecture_; ///< Architecture.

public:
    /**
     * Constructor.
     *
     * \param[out] variables Information about variables.
     * \param[in] dataflows Dataflow information for each function.
     * \param[in] architecture Valid pointer to the architecture.
     */
    VariableAnalyzer(Variables &variables, const dflow::Dataflows &dataflows, const arch::Architecture *architecture):
        variables_(variables), dataflows_(dataflows), architecture_(architecture)
    {
        assert(architecture != nullptr);
    }

    /**
     * Computes mapping of terms to variables.
     */
    void analyze();
};

} // namespace vars
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

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

#include <memory>

namespace nc {
namespace core {

namespace ir {
    class Program;
    class Term;
}

namespace arch {
    class Instructions;
    class Instruction;
    class Register;
}

namespace irgen {

/**
 * Class used for producing IR code from an instruction.
 */
class InstructionAnalyzer {
public:
    /**
     * Creates intermediate representation of an instruction and adds newly created statements to
     * the intermediate representation of the program.
     *
     * \param[in] instruction   Valid pointer to the instruction to generate intermediate representation for.
     * \param[out] program      Valid pointer to the intermediate representation of a program.
     */
    void createStatements(const arch::Instruction *instruction, ir::Program *program);

    /**
     * \param[in] reg Valid pointer to a register.
     *
     * \return Valid pointer to the intermediate representation of this register as a term.
     */
    static std::unique_ptr<ir::Term> createTerm(const arch::Register *reg);

    void setInstructions(const nc::core::arch::Instructions *);

    const nc::core::arch::Instructions *instructions() const;

protected:
    /**
     * Actually creates intermediate representation of an instruction.
     *
     * \param[in] instruction   Valid pointer to the instruction to generate intermediate representation for.
     * \param[out] program      Valid pointer to the intermediate representation of a program.
     */
    virtual void doCreateStatements(const arch::Instruction *instruction, ir::Program *program) = 0;

private:
    const nc::core::arch::Instructions *instructions_;
};

} // namespace irgen
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

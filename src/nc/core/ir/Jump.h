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

#include "JumpTarget.h"
#include "Statement.h"

namespace nc {
namespace core {
namespace ir {

class Term;

/**
 * Jump to an address and/or basic block.
 */
class Jump: public Statement {
    /** Jump condition. */
    std::unique_ptr<Term> condition_;

    /** Jump destination if condition is non-zero. */
    JumpTarget thenTarget_;

    /** Jump destination if condition is zero. */
    JumpTarget elseTarget_;

public:
    /**
     * Constructor of a conditional jump.
     *
     * \param[in] condition     Valid pointer to the term representing jump condition.
     * \param[in] thenTarget    Jump target if condition is non-zero.
     * \param[in] elseTarget    Jump target if condition is zero.
     */
    Jump(std::unique_ptr<Term> condition, JumpTarget thenTarget, JumpTarget elseTarget);

    /**
     * Constructor of an unconditional jump.
     *
     * \param[in] thenTarget    Jump target.
     */
    Jump(JumpTarget thenTarget);

    /**
     * \return Pointer to the term representing jump condition, nullptr for unconditional jump.
     */
    const Term *condition() const { return condition_.get(); }

    /**
     * \return True if this is a conditional jump, false if this is an unconditional jump.
     */
    bool isConditional() const { return condition() != nullptr; }

    /**
     * \return True if this is a unconditional jump, false if this is an conditional jump.
     */
    bool isUnconditional() const { return !isConditional(); }

    /**
     * \return Jump destination if condition is non-zero.
     */
    JumpTarget &thenTarget() { return thenTarget_; }

    /**
     * \return Jump destination if condition is non-zero.
     */
    const JumpTarget &thenTarget() const { return thenTarget_; }

    /**
     * \return Jump destination if condition is zero.
     */
    JumpTarget &elseTarget() { return elseTarget_; }

    /**
     * \return Jump destination if condition is zero.
     */
    const JumpTarget &elseTarget() const { return elseTarget_; }

    virtual void print(QTextStream &out) const override;

protected:
    std::unique_ptr<Statement> doClone() const override;
};

const Jump *Statement::asJump() const {
    return as<Jump>();
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

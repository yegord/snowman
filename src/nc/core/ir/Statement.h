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
#include <memory>

#include <boost/noncopyable.hpp>

#include <QString>

#include <nc/common/Printable.h>
#include <nc/common/Subclass.h>
#include <nc/common/ilist.h>

namespace nc {
namespace core {

namespace arch {
    class Instruction;
}

namespace ir {

class Assignment;
class BasicBlock;
class Call;
class Callback;
class Halt;
class InlineAssembly;
class Jump;
class Term;
class Touch;

/**
 * Base class for different kinds of statements of intermediate representation.
 */
class Statement: public Printable, public nc::ilist_item, boost::noncopyable {
    NC_BASE_CLASS(Statement, kind)

public:
    /**
     * Statement kind.
     */
    enum {
        INLINE_ASSEMBLY,///< Inline assembly.
        ASSIGNMENT,     ///< Assignment.
        JUMP,           ///< Jump.
        CALL,           ///< Function call.
        HALT,           ///< Return from a program.
        TOUCH,          ///< Reads or writes a term.
        CALLBACK,       ///< Custom operation.
        REMEMBER_REACHING_DEFINITIONS, ///< Remembers reaching definition.
        USER = 1000     ///< Base for user-defined statements.
    };

private:
    BasicBlock *basicBlock_; ///< Basic block to which this statement belongs.
    const arch::Instruction *instruction_; ///< Instruction from which this statement was generated.

public:
    /**
     * Class constructor.
     *
     * \param[in] kind Kind of the statement.
     */
    explicit Statement(int kind): kind_(kind), basicBlock_(nullptr), instruction_(nullptr) {}

    /**
     * \return Pointer to the basic block to which this statement belongs.
     *         Can be nullptr.
     */
    BasicBlock *basicBlock() { return basicBlock_; }

    /**
     * \return Pointer to the basic block to which this statement belongs.
     *         Can be nullptr.
     */
    const BasicBlock *basicBlock() const { return basicBlock_; }

    /**
     * Sets the pointer to the basic block to which this statement belongs.
     *
     * \param[in] basicBlock Pointer to the basic block. Can be nullptr.
     */
    void setBasicBlock(BasicBlock *basicBlock) { basicBlock_ = basicBlock; }

    /**
     * \param[in] instruction Instruction from which this statement was generated.
     */
    void setInstruction(const arch::Instruction *instruction) {
        assert(!instruction_ && "Instruction must be set only once.");
        instruction_ = instruction;
    }

    /**
     * \return Instruction that this statement was generated from. Can be nullptr.
     */
    const arch::Instruction *instruction() const { return instruction_; }

    /**
     * \return True iff this a terminator statement, i.e. a statement
     *         which can be only the last statement of a basic block.
     */
    bool isTerminator() const {
        return is<Jump>() || is<Halt>();
    }

    /**
     * Clones the statement and sets the instruction of the clone
     * to the instruction of this statement.
     *
     * \returns Valid pointer to the clone.
     */
    std::unique_ptr<Statement> clone() const;

    /* The following functions are defined in Statements.h. */

    inline const Assignment *asAssignment() const;
    inline const Jump *asJump() const;
    inline const Call *asCall() const;
    inline const Touch *asTouch() const;
    inline const Callback *asCallback() const;
    
protected:
    /**
     * \return Valid pointer to the clone of the statement.
     */
    virtual std::unique_ptr<Statement> doClone() const = 0;
};

}}} // namespace nc::core::ir

/**
 * Defines a compile-time mapping from statement class to statement kind.
 * Makes it possible to use the given class as an argument to <tt>Statement::as</tt>
 * and <tt>Statement::is</tt> template functions.
 * 
 * Must be used at global namespace.
 * 
 * \param CLASS                        Statement class.
 * \param KIND                         Statement kind.
 */
#define NC_REGISTER_STATEMENT_CLASS(CLASS, KIND)                                \
    NC_SUBCLASS(nc::core::ir::Statement, CLASS, KIND)

NC_REGISTER_STATEMENT_CLASS(nc::core::ir::InlineAssembly, nc::core::ir::Statement::INLINE_ASSEMBLY)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Assignment,     nc::core::ir::Statement::ASSIGNMENT)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Jump,           nc::core::ir::Statement::JUMP)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Call,           nc::core::ir::Statement::CALL)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Halt,           nc::core::ir::Statement::HALT)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Touch,          nc::core::ir::Statement::TOUCH)
NC_REGISTER_STATEMENT_CLASS(nc::core::ir::Callback,       nc::core::ir::Statement::CALLBACK)

/* vim:set et sts=4 sw=4: */

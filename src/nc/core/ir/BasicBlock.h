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

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <nc/common/Printable.h>
#include <nc/common/Types.h>
#include <nc/common/ilist.h>

namespace nc {
namespace core {
namespace ir {

class Function;
class Jump;
class Statement;

/**
 * Basic block.
 */
class BasicBlock: public PrintableBase<BasicBlock>, public nc::ilist_item, boost::noncopyable {
public:
    typedef nc::ilist<Statement> Statements;

private:
    boost::optional<ByteAddr> address_; ///< Address of basic block.
    boost::optional<ByteAddr> successorAddress_; ///< Address of the end of the basic block.
    Statements statements_; ///< Statements.
    Function *function_; ///< Function this basic block belongs to.

public:
    /**
     * Constructor.
     *
     * \param[in] address Address of the basic block.
     */
    BasicBlock(const boost::optional<ByteAddr> &address = boost::none);

    /**
     * Destructor.
     */
    ~BasicBlock();

    /**
     * \return Address of the basic block.
     */
    const boost::optional<ByteAddr> &address() const { return address_; }

    /**
     * Sets the address of this basic block.
     *
     * \param address New address.
     */
    void setAddress(const boost::optional<ByteAddr> &address);

    /**
     * \return Address of the basic block's successor.
     */
    const boost::optional<ByteAddr> &successorAddress() const { return successorAddress_; }

    /**
     * Sets the address of the basic block's successor.
     *
     * \param[in] successorAddress New successor address.
     */
    void setSuccessorAddress(const boost::optional<ByteAddr> &successorAddress);

    /**
     * \return Pointer to the function this basic block belongs to. Can be nullptr.
     */
    Function *function() { return function_; }

    /**
     * \return Pointer to the function this basic block belongs to. Can be nullptr.
     */
    const Function *function() const { return function_; }

    /**
     * Sets the function this basic block belongs to.
     *
     * \param function Pointer to the function. Can be nullptr.
     */
    void setFunction(Function *function) { function_ = function; }

    /**
     * \return Statements of the basic block.
     *
     * \warning Do not insert or erase statements into/from the container
     *          directly. Use the methods of BasicBlock class instead.
     */
    Statements &statements() { return statements_; }

    /**
     * \return Statements of the basic block.
     */
    const Statements &statements() const { return statements_; }

    /**
     * Inserts a statement at the given position.
     *
     * \param position Position where the statement must be inserted.
     * \param statement Valid pointer to the statement being inserted.
     *
     * \return Valid pointer to the inserted statement.
     */
    Statement *insert(Statements::const_iterator position, std::unique_ptr<Statement> statement);

    /**
     * Insertes a statement at the front of the basic block.
     *
     * \param statement Valid pointer to the statement being added.
     *
     * \return Valid pointer to the inserted statement.
     */
    Statement *pushFront(std::unique_ptr<Statement> statement);

    /**
     * Inserts a statement at the end of the basic block.
     *
     * \param statement Valid pointer to the statement being added.
     *
     * \return Valid pointer to the inserted statement.
     */
    Statement *pushBack(std::unique_ptr<Statement> statement);

    /**
     * Inserts a statement after a given one.
     *
     * \param after Valid pointer to the statement after which to insert.
     * \param statement Valid pointer to the statement being inserted.
     *
     * \return Valid pointer to the inserted statement.
     */
    Statement *insertAfter(const Statement *after, std::unique_ptr<Statement> statement);

    /**
     * Inserts a statement before a given one.
     *
     * \param before Valid pointer to the statement before which to insert.
     * \param statement Valid pointer to the statement being inserted.
     *
     * \return Valid pointer to the inserted statement.
     */
    Statement *insertBefore(const Statement *before, std::unique_ptr<Statement> statement);

    /**
     * Erases a statement pointed by the given iterator.
     * Sets the basic block pointer in this statement to nullptr.
     *
     * \param statement Valid pointer to the erased statement.
     *
     * \return The erased statement.
     */
    std::unique_ptr<Statement> erase(Statement *statement);

    /**
     * \return Valid pointer to the last statement in the basic block if this
     *         is a terminator statement, nullptr otherwise.
     */
    const Statement *getTerminator() const;

    /**
     * \return Valid pointer to the last statement in the basic block if this
     *         is a jump, nullptr otherwise.
     */
    Jump *getJump();

    /**
     * \return Valid pointer to the last statement in the basic block if this
     *         is a jump, nullptr otherwise.
     */
    const Jump *getJump() const;

    /**
     * Breaks the basic block into two parts. The first part contains the
     * statements before the given position and remains in the original basic
     * block. The second part includes remaining statements and is moved to a
     * newly created basic block.
     *
     * \param[in] position Iterator identifying the first statement of the second part.
     * \param[in] address Start address to give to the newly created basic block.
     *
     * \return Valid pointer to the newly created basic block containing the
     *         second part of statements.
     */
    std::unique_ptr<BasicBlock> split(Statements::const_iterator position, const boost::optional<ByteAddr> &address);

    /**
     * \return A valid pointer to the basic block which is a copy of this one.
     */
    std::unique_ptr<BasicBlock> clone() const;

    /**
     * Prints the basic block and its incoming edges into a stream in DOT format.
     *
     * \param out Output stream.
     */
    void print(QTextStream &out) const;
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

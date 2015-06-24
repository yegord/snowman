/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "BasicBlock.h"

#include <QTextStream>

#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Term.h>

#include <nc/common/Escaping.h>
#include <nc/common/Foreach.h>

namespace nc {
namespace core {
namespace ir {

BasicBlock::BasicBlock(const boost::optional<ByteAddr> &address):
    address_(address), successorAddress_(address), function_(nullptr)
{}

BasicBlock::~BasicBlock() {}

void BasicBlock::setAddress(const boost::optional<ByteAddr> &address) {
    address_ = address;
    if (!address_) {
        setSuccessorAddress(boost::none);
    }
}

void BasicBlock::setSuccessorAddress(const boost::optional<ByteAddr> &successorAddress) {
    assert((!successorAddress || address()) && "A non-memory-bound basic block cannot have a successor address.");
    successorAddress_ = successorAddress;
}

Statement *BasicBlock::insert(ilist<Statement>::const_iterator position, std::unique_ptr<Statement> statement) {
    assert(statement != nullptr);
    assert(statement->basicBlock() == nullptr);

    auto result = statement.get();
    statements_.insert(position, std::move(statement));
    result->setBasicBlock(this);
    return result;
}

Statement *BasicBlock::pushFront(std::unique_ptr<Statement> statement) {
    assert(statement != nullptr);

    return insert(statements_.begin(), std::move(statement));
}

Statement *BasicBlock::pushBack(std::unique_ptr<Statement> statement) {
    assert(statement != nullptr);

    return insert(statements_.end(), std::move(statement));
}

Statement *BasicBlock::insertAfter(const Statement *after, std::unique_ptr<Statement> statement) {
    assert(after != nullptr);
    assert(statement != nullptr);

    return insert(++statements_.get_iterator(after), std::move(statement));
}

Statement *BasicBlock::insertBefore(const Statement *before, std::unique_ptr<Statement> statement) {
    assert(before != nullptr);
    assert(statement != nullptr);

    return insert(statements_.get_iterator(before), std::move(statement));
}

std::unique_ptr<Statement> BasicBlock::erase(Statement *statement) {
    auto result = statements_.erase(statement);
    assert(result->basicBlock() == this);
    result->setBasicBlock(nullptr);
    return result;
}

const Statement *BasicBlock::getTerminator() const {
    if (statements().empty()) {
        return nullptr;
    }

    auto lastStatement = statements().back();
    if (lastStatement->isTerminator()) {
        return lastStatement;
    }

    return nullptr;
}

Jump *BasicBlock::getJump() {
    if (statements().empty()) {
        return nullptr;
    } else {
        return statements().back()->as<Jump>();
    }
}

const Jump *BasicBlock::getJump() const {
    if (statements().empty()) {
        return nullptr;
    } else {
        return statements().back()->as<Jump>();
    }
}

std::unique_ptr<BasicBlock> BasicBlock::split(ilist<Statement>::const_iterator position, const boost::optional<ByteAddr> &address) {
    /* Create a new basic block. */
    std::unique_ptr<BasicBlock> result(new BasicBlock(address));
    result->setSuccessorAddress(this->successorAddress());
    this->setSuccessorAddress(address);

    /* Move statements to it. */
    result->statements_ = statements_.cut_out(position, statements_.end());
    foreach (auto statement, result->statements_) {
        statement->setBasicBlock(result.get());
    }

    return result;
}

std::unique_ptr<BasicBlock> BasicBlock::clone() const {
    std::unique_ptr<BasicBlock> result(new BasicBlock(address()));
    result->setSuccessorAddress(successorAddress());

    foreach (const Statement *statement, statements()) {
        result->pushBack(statement->clone());
    }

    return result;
}

void BasicBlock::print(QTextStream &out) const {
    out << "basicBlock" << this << " [shape=box,label=\"";

    QString label;
    QTextStream ls(&label);

    ls << "Address: ";
    if (address()) {
        int integerBase = ls.integerBase();
        hex(ls) << "0x" << *address();
        ls.setIntegerBase(integerBase);
    } else {
        ls << "None";
    }
    ls << endl;

    foreach (const Statement *statement, statements()) {
        ls << *statement;
    }

    out << escapeDotString(label) << "\"];" << endl;
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

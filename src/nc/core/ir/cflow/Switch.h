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

#include "Region.h"

namespace nc {
namespace core {
namespace ir {

class BasicBlock;
class Term;

namespace cflow {

class BasicNode;

/**
 * Switch region.
 */
class Switch: public Region {
    /** Node that does table-based jump. */
    BasicNode *switchNode_;

    /** Term being switched upon. */
    const Term *switchTerm_;

    /** Jump table size. */
    std::size_t jumpTableSize_;

    /** Bounds check node. */
    BasicNode *boundsCheckNode_;

    /** Default basic block. */
    const BasicBlock *defaultBasicBlock_;

public:
    /**
     * Constructor.
     *
     * \param switchNode    Valid pointer to the node doing the table-based jump.
     * \param switchTerm    Valid pointer to the term being switched upon.
     * \param jumpTableSize Size of the jump table.
     */
    Switch(BasicNode *switchNode, const Term *switchTerm, std::size_t jumpTableSize):
        Region(Region::SWITCH),
        switchNode_(switchNode),
        switchTerm_(switchTerm),
        jumpTableSize_(jumpTableSize),
        boundsCheckNode_(nullptr),
        defaultBasicBlock_(nullptr)
    {
        assert(switchNode);
        assert(switchTerm);
    }

    /**
     * \return Pointer to the node that does table-based jump. Can be nullptr.
     */
    BasicNode *switchNode() const { return switchNode_; };

    /**
     * \return Valid pointer to the term being switched upon.
     */
    const Term *switchTerm() const { return switchTerm_; }

    /**
     * \return Size of the jump table.
     */
    std::size_t jumpTableSize() const { return jumpTableSize_; }

    /**
     * \return Pointer to the bounds check node. Can be nullptr.
     */
    BasicNode *boundsCheckNode() const { return boundsCheckNode_; }

    /**
     * Sets the pointer to the bounds check node.
     *
     * \param node Pointer to the node. Can be nullptr.
     */
    void setBoundsCheckNode(BasicNode *node) { boundsCheckNode_ = node; }

    /**
     * \return Pointer to the basic block to be marked by default label. Can be nullptr.
     */
    const BasicBlock *defaultBasicBlock() const { return defaultBasicBlock_; }

    /**
     * Sets the basic block to be marked by default label.
     *
     * \param basicBlock Pointer to the basic block. Can be nullptr.
     */
    void setDefaultBasicBlock(const BasicBlock *basicBlock) { defaultBasicBlock_ = basicBlock; }
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

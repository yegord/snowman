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

namespace nc {
namespace core {
namespace ir {
namespace cflow {

class Node;

class Edge {
    public:

    private:

    Node *tail_; ///< Edge tail.
    Node *head_; ///< Edge head.

    public:

    /**
     * Class constructor.
     *
     * \param[in] tail Edge tail.
     * \param[in] head Edge head.
     */
    Edge(Node *tail, Node *head);

    /**
     * \return Pointer to the edge's tail. Can be nullptr.
     */
    Node *tail() const { return tail_; }

    /**
     * Sets edge tail.
     * Removes the edge from the list of outgoing edges of the previous tail and adds to the one of new tail.
     *
     * \param[in] tail Pointer to the edge's tail. Can be nullptr.
     */
    void setTail(Node *tail);

    /**
     * \return Pointer to the edge's head. Can be nullptr.
     */
    Node *head() const { return head_; }

    /**
     * Sets edge head.
     *
     * Removes the edge from the list of incoming edges of the previous head and adds to the one of new head.
     *
     * \param[in] head Pointer to the edge's head. Can be nullptr.
     */
    void setHead(Node *head);
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

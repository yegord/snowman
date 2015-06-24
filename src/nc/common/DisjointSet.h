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

/**
 * Template class implementing disjoint set using weighted union and path compression heuristics.
 *
 * It can be used like this:
 *
 * \code
 * class Node;
 *
 * class Node: public DisjointSet<Node> { ... }
 * \endcode
 *
 * \tparam T Type of disjoint set element.
 */
template<class T>
class DisjointSet {
    mutable DisjointSet<T> *parent_; ///< Disjoint set parent.
    int rank_; ///< Disjoint set rank.

    public:

    /**
     * Class constructor.
     */
    DisjointSet(): parent_(this), rank_(0) {}

    /**
     * Creates a singleton.
     */
    void makeSet() {
        parent_ = this;
        rank_ = 0;
    }

    /**
     * Finds a representative of the set.
     *
     * \return The representative.
     */
    T *findSet() {
        return static_cast<T *>(findSetImpl());
    }

    /**
     * Finds a representative of the set.
     *
     * \return The representative.
     */
    const T *findSet() const {
        return static_cast<const T *>(findSetImpl());
    }

    /**
     * Unions this set with the given one using weighted union algorithm.
     *
     * \param[in] that The set to merge with this one.
     */
    void unionSet(DisjointSet<T> *that) {
        DisjointSet<T> *x = this->findSet();
        DisjointSet<T> *y = that->findSet();

        if (x->rank_ < y->rank_) {
            x->parent_ = y;
        } else if (x != y) {
            y->parent_ = x;
            if (x->rank_ == y->rank_) {
                ++x->rank_;
            }
        }
    }

    private:

    /**
     * Finds a representative of the set using path compression.
     *
     * \return The representative.
     */
    DisjointSet<T> *findSetImpl() const {
        if (parent_ != this) {
            parent_ = parent_->findSetImpl();
        }
        return parent_;
    }
};

} // namespace nc

/* vim:set et sts=4 sw=4: */

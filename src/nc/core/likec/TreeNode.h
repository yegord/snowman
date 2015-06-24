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

#include <algorithm> /* remove_if */
#include <functional>

#include <QString>

#include <nc/common/Foreach.h>
#include <nc/common/Subclass.h>
#include <nc/common/Unused.h>

namespace nc {
namespace core {
namespace likec {

class PrintContext;
class Tree;

/**
 * Base class for tree nodes.
 */
class TreeNode {
    NC_BASE_CLASS(TreeNode, nodeKind)

    Tree &tree_; ///< Owning tree.

public:
    /**
     * Node kind.
     */
    enum {
        COMPILATION_UNIT,               ///< Compilation unit.
        DECLARATION,                    ///< Declaration.
        EXPRESSION,                     ///< Expression.
        STATEMENT,                      ///< Statement.
        USER = 1000                     ///< Base for user-defined nodes.
    };

    /**
     * Class constructor.
     *
     * \param[in] tree Owning tree.
     * \param[in] nodeKind Node kind.
     */
    TreeNode(Tree &tree, int nodeKind): nodeKind_(nodeKind), tree_(tree) {}

    /**
     * Virtual destructor.
     */
    virtual ~TreeNode() {}

    /**
     * \return Owning tree.
     */
    Tree &tree() const { return tree_; }

    /**
     * Calls a given function on all the children of this node.
     *
     * \param fun Valid function.
     */
    void callOnChildren(const std::function<void(const TreeNode *)> &fun) const {
        assert(fun);
        const_cast<TreeNode *>(this)->doCallOnChildren(fun);
    }

    /**
     * Calls a given function on all the children of this node.
     *
     * \param fun Valid function.
     */
    void callOnChildren(const std::function<void(TreeNode *)> &fun) {
        assert(fun);
        doCallOnChildren(fun);
    }

    /**
     * Rewrites subtree starting from this node for the purpose of its optimization.
     *
     * \return Rewritten node.
     */
    virtual TreeNode *rewrite() { return this; }

    /**
     * Prints the tree node calling appropriate print callbacks.
     *
     * \param[in] context Print context.
     */
    void print(PrintContext &context) const;

protected:
    /**
     * Calls a given function on all the children of this node.
     * Default implementation does nothing. Subclasses having
     * children must override this method.
     *
     * \param fun Valid function.
     */
    virtual void doCallOnChildren(const std::function<void(TreeNode *)> &fun);

    /**
     * Prints the tree node without calling any callbacks.
     *
     * \param[in] context Print context.
     */
    virtual void doPrint(PrintContext &context) const = 0;

    /**
     * Rewrites child and puts rewritten subtree in its place.
     *
     * \tparam T Type of smart pointer to a child node.
     *
     * \param child Smart pointer to a child.
     */
    template<class T>
    void rewriteChild(T &child) {
        auto *result = child->rewrite();
        if (result != child.get()) {
            child.reset(result);
            if (child) {
                rewriteChild(child);
            }
        }
    }

    /**
     * Rewrites children and puts rewritten subtrees in its place.
     *
     * \tparam T Type of container of smart pointers to children.
     *
     * \param children Reference to a container of smart pointers to children.
     */
    template<class T>
    void rewriteChildren(T &children) {
        foreach (auto &child, children) {
            rewriteChild(child);
        }
        children.erase(std::remove_if(children.begin(), children.end(), IsNull()), children.end());
    }

private:
    /**
     * Predicate class for checking if a smart pointer is non-zero.
     */
    class IsNull {
        public:

        template<class T>
        bool operator()(const T& pointer) const {
            return !pointer.get();
        }
    };
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

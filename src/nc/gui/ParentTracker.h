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

#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/core/likec/TreeNode.h>

namespace nc {
namespace gui {

/**
 * Visitor of LikeC nodes for computing parent mapping.
 */
class ParentTracker {
    /** Mapping from a node to its parent. */
    boost::unordered_map<const core::likec::TreeNode *, const core::likec::TreeNode *> &map_;

    /** Ancestors of the currently visited node. */
    std::vector<core::likec::TreeNode *> stack_;

public:
    /**
     * Constructor.
     *
     * \param[out] map  Mapping from a node to its parent.
     */
    ParentTracker(boost::unordered_map<const core::likec::TreeNode *, const core::likec::TreeNode *> &map):
        map_(map)
    {
        stack_.push_back(nullptr);
    }

    /**
     * Visits LikeC node, remembers its parent, and descends to its children.
     *
     * \param node Valid pointer to a LikeC node.
     */
    void operator()(core::likec::TreeNode *node) {
        map_[node] = stack_.back();

        stack_.push_back(node);
        node->callOnChildren(*this);
        stack_.pop_back();
    }
};

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */

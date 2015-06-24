/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include "RangeNode.h"

namespace nc {

namespace core { namespace likec {
    class TreeNode;
    class Tree;
}}

namespace gui {

class RangeTree {
    std::unique_ptr<RangeNode> root_;

public:
    RangeTree();
    ~RangeTree();

    const RangeNode *root() const { return root_.get(); }
    void setRoot(std::unique_ptr<RangeNode> root);

    const RangeNode *getLeafAt(int position) const;
    std::vector<const RangeNode *> getNodesIn(const Range<int> &range) const;

    Range<int> getRange(const RangeNode *node) const;

    std::vector<const RangeNode *> handleRemoval(int position, int nchars);
    std::vector<const RangeNode *> handleInsertion(int position, int nchars);
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

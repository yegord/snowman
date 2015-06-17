/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <algorithm>
#include <cassert>
#include <vector>

#include <nc/common/Foreach.h>
#include <nc/common/RangeClass.h>

namespace nc {

namespace core { namespace likec {
    class TreeNode;
}}

namespace gui {

class RangeNode {
    void *data_;
    int offset_;
    int size_;
    std::vector<RangeNode> children_;
    RangeNode *parent_;

public:
    RangeNode(void *data, int offset):
        data_(data), offset_(offset), size_(-1), parent_(nullptr)
    {
        assert(offset >= 0);
    }

    void *data() const { return data_; }

    int offset() const { return offset_; }
    void setOffset(int offset) { assert(offset >= 0); offset_ = offset; }

    int size() const { assert(size_ >= 0); return size_; }
    void setSize(int size) { assert(size >= 0); size_ = size; }

    int endOffset() const { return offset() + size(); }

    Range<int> range() const { return make_range(offset(), endOffset()); }

    std::vector<RangeNode> &children() { return children_; }
    const std::vector<RangeNode> &children() const { return children_; }

    RangeNode *addChild(RangeNode node) {
        assert(children_.empty() || children_.back().endOffset() <= node.offset());
        children_.push_back(std::move(node));
        return &children_.back();
    }

    const RangeNode *parent() const { return parent_; }

    void updateParentPointers() {
        foreach (auto &child, children_) {
            child.parent_ = this;
            child.updateParentPointers();
        }
    }
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

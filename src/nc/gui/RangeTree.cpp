/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "RangeTree.h"

namespace nc {
namespace gui {

RangeTree::RangeTree() {}

RangeTree::~RangeTree() {}

void RangeTree::setRoot(std::unique_ptr<RangeNode> root) {
    assert(!root || root->offset() == 0);
    root_ = std::move(root);
}

namespace {

std::vector<RangeNode>::iterator getFirstChildNotToTheLeftOf(RangeNode &node, int offset) {
    return std::lower_bound(node.children().begin(), node.children().end(), offset,
                            [](const RangeNode &node, int offset) { return node.endOffset() <= offset; });
}

RangeNode *getChildAtOffset(RangeNode &node, int offset) {
    auto i = getFirstChildNotToTheLeftOf(node, offset);

    if (i != node.children().end() && i->range().contains(offset)) {
        return &*i;
    } else {
        return nullptr;
    }
}

} // anonymous namespace

const RangeNode *RangeTree::getLeafAt(int position) const {
    if (!root_ || !root_->range().contains(position)) {
        return nullptr;
    }

    auto node = root_.get();
    while (auto child = getChildAtOffset(*node, position)) {
        node = child;
        position -= node->offset();
    }
    return node;
}

namespace {

void doGetNodesIn(RangeNode &node, const Range<int> &range, std::vector<const RangeNode *> &result) {
    if (range.start() <= 0 && node.size() <= range.end()) {
        result.push_back(&node);
    }

    auto i = getFirstChildNotToTheLeftOf(node, range.start());
    auto iend = node.children().end();

    for (; i != iend && i->offset() < range.end(); ++i) {
        doGetNodesIn(*i, range.shifted(-i->offset()), result);
    }
}

} // anonymous namespace

std::vector<const RangeNode *> RangeTree::getNodesIn(const Range<int> &range) const {
    std::vector<const RangeNode *> result;
    if (root_ && root_->range().overlaps(range)) {
        doGetNodesIn(*root_, range, result);
    }
    return result;
}

Range<int> RangeTree::getRange(const RangeNode *node) const {
    assert(node != nullptr);
    assert(root_ != nullptr);

    if (node->parent() == nullptr && node != root_.get()) {
        root_->updateParentPointers();
        assert(node->parent() != nullptr);
    }

    int offset = 0;
    for (auto current = node; current != root_.get(); current = current->parent()) {
        offset += current->offset();
    }
    return make_range(offset, offset + node->size());
}

namespace {

void doHandleRemoval(RangeNode &node, int offset, int nchars, std::vector<const RangeNode *> &modified) {
    if (offset < 0) {
        nchars += offset;
        offset = 0;
    }

    if (nchars + offset > node.size()) {
        nchars = node.size() - offset;
    }

    if (nchars <= 0) {
        return;
    }

    node.setSize(node.size() - nchars);
    modified.push_back(&node);

    auto i = getFirstChildNotToTheLeftOf(node, offset);

    for (auto iend = node.children().end(); i != iend; ++i) {
        if (i->offset() < offset + nchars) {
            doHandleRemoval(*i, offset - i->offset(), nchars, modified);
            if (offset < i->offset()) {
                i->setOffset(offset);
            }
        } else {
            i->setOffset(i->offset() - nchars);
        }
    }
}

} // anonymous namespace

std::vector<const RangeNode *> RangeTree::handleRemoval(int position, int nchars) {
    std::vector<const RangeNode *> result;
    if (root_ && root_->range().contains(position)) {
        doHandleRemoval(*root_, position, nchars, result);
    }
    return result;
}

namespace {

void doHandleInsertion(RangeNode &node, int offset, int nchars, std::vector<const RangeNode *> &modified) {
    assert(offset <= node.size());

    node.setSize(node.size() + nchars);
    modified.push_back(&node);

    auto i = getFirstChildNotToTheLeftOf(node, offset - 1);

    if (i != node.children().end()) {
        if (i->range().contains(offset) || i->endOffset() == offset) {
            doHandleInsertion(*i, offset - i->offset(), nchars, modified);
            ++i;
        }

        for (auto iend = node.children().end(); i != iend; ++i) {
            i->setOffset(i->offset() + nchars);
        }
    }
}

} // anonymous namespace

std::vector<const RangeNode *> RangeTree::handleInsertion(int position, int nchars) {
    std::vector<const RangeNode *> result;
    if (root_ && root_->range().contains(position)) {
        doHandleInsertion(*root_, position, nchars, result);
    }
    return result;
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

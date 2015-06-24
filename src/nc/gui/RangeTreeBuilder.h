/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>
#include <stack>

#include <nc/common/make_unique.h>

#include "RangeTree.h"

namespace nc {
namespace gui {

class RangeNodeAndPosition {
    RangeNode *node_;
    int position_;

public:
    RangeNodeAndPosition(RangeNode *node, int position):
        node_(node), position_(position)
    {
        assert(position >= 0);
    }

    RangeNode *node() const { return node_; }
    int position() const { return position_; }
};

class RangeTreeBuilder {
    RangeTree &tree_;
    std::stack<RangeNodeAndPosition> stack_;

public:
    RangeTreeBuilder(RangeTree &tree): tree_(tree) {}

    void onStart(void *data, int position) {
        if (stack_.empty()) {
            auto root = std::make_unique<RangeNode>(data, 0);
            stack_.push(RangeNodeAndPosition(root.get(), 0));
            tree_.setRoot(std::move(root));
        } else {
            stack_.push(RangeNodeAndPosition(
                stack_.top().node()->addChild(RangeNode(data, position - stack_.top().position())),
                position));
        }
    }

    void onEnd(void *data, int position) {
        assert(stack_.top().node()->data() == data);

        stack_.top().node()->setSize(position - stack_.top().position());
        stack_.pop();
    }
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

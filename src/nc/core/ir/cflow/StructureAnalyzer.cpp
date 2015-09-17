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

#include "StructureAnalyzer.h"

#include <algorithm>
#include <queue>

#include <boost/unordered_map.hpp>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/dflow/Utils.h>
#include <nc/core/ir/misc/ArrayAccess.h>
#include <nc/core/ir/misc/BoundsCheck.h>
#include <nc/core/ir/misc/PatternRecognition.h>

#include "BasicNode.h"
#include "Dfs.h"
#include "Edge.h"
#include "Graph.h"
#include "LoopExplorer.h"
#include "Switch.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

void StructureAnalyzer::analyze() {
    analyze(graph_.root());
}

void StructureAnalyzer::analyze(Region *region) {
    bool changed;

    do {
        changed = false;

        /*
         * Classify edges, sort nodes topologically.
         */
        Dfs dfs(region);

        /*
         * Try to reduce various kinds of regions.
         */
        foreach (Node *node, dfs.postordering()) {
            if (reduceCompoundCondition(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceCyclic(node, dfs)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceBlock(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceConditional(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceSwitch(node) || reduceHopelessConditional(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }
    } while (changed);
}

bool StructureAnalyzer::reduceBlock(Node *entry) {
    Node *uniquePredecessor = entry->uniquePredecessor();

    /*
     * Blocks are only needed for reducing ifs.
     * If the unique predecessor cannot be an if condition, stop.
     */
    if (!uniquePredecessor || !uniquePredecessor->isFork() || !uniquePredecessor->isCondition()) {
        return false;
    }

    /* Discover all the nodes in the block. */
    std::vector<Node *> traverse;
    Node *node = entry;
    do {
        traverse.push_back(node);
        node = node->uniqueSuccessor();
    } while (node && node->uniquePredecessor());

    /* Finally, create the subregion. */
    if (traverse.size() > 1) {
        auto subregion = std::make_unique<Region>(Region::BLOCK);
        subregion->setEntry(entry);
        subregion->nodes() = std::move(traverse);
        return insertSubregion(entry->parent(), std::move(subregion));
    }

    return false;
}

bool StructureAnalyzer::reduceConditional(Node *entry) {
    if (!entry->isFork() || !entry->isCondition()) {
        return false;
    }

    Node *left = entry->outEdges()[0]->head();
    Node *right = entry->outEdges()[1]->head();

    /*
     * IF_THEN_ELSE
     */
    if (left->inEdges().size() == 1 && right->inEdges().size() == 1 &&
        left->outEdges().size() <= 1 && right->outEdges().size() <= 1 &&
        (left->outEdges().empty() || right->outEdges().empty() ||
         left->outEdges()[0]->head() == right->outEdges()[0]->head()))
    {
        auto subregion = std::make_unique<Region>(Region::IF_THEN_ELSE);
        subregion->setEntry(entry);
        subregion->nodes().push_back(entry);
        subregion->nodes().push_back(left);
        subregion->nodes().push_back(right);
        return insertSubregion(entry->parent(), std::move(subregion));
    }

    /*
     * IF_THEN
     */
#define REDUCE(left, right)                                                         \
    if (left->inEdges().size() == 1 &&                                              \
        (left->outEdges().empty() ||                                                \
         (left->outEdges().size() == 1 && left->outEdges()[0]->head() == right)))   \
    {                                                                               \
        auto subregion = std::make_unique<Region>(Region::IF_THEN);                 \
        subregion->setEntry(entry);                                                 \
        subregion->nodes().push_back(entry);                                        \
        subregion->nodes().push_back(left);                                         \
        subregion->setExitBasicBlock(right->getEntryBasicBlock());                  \
        return insertSubregion(entry->parent(), std::move(subregion));              \
    }
    REDUCE(left, right)
    REDUCE(right, left)
#undef REDUCE

    return false;
}

bool StructureAnalyzer::reduceHopelessConditional(Node *entry) {
    if (!entry->isFork() || !entry->isCondition()) {
        return false;
    }

    Node *left = entry->outEdges()[0]->head();
    Node *right = entry->outEdges()[1]->head();

    auto subregion = std::make_unique<Region>(Region::IF_THEN_ELSE);
    subregion->setEntry(entry);
    subregion->nodes().push_back(entry);
    subregion->nodes().push_back(left);
    subregion->nodes().push_back(right);
    return insertSubregion(entry->parent(), std::move(subregion));
}

bool StructureAnalyzer::reduceCompoundCondition(Node *entry) {
    if (!entry->isFork() || !entry->isCondition()) {
        return false;
    }

    Node *left = entry->outEdges()[0]->head();
    Node *right = entry->outEdges()[1]->head();

#define REDUCE(left, right)                                                                 \
    if (left->inEdges().size() == 1 && left->isFork() && left->isCondition() &&             \
        ((left->outEdges()[0]->head() == right && left->outEdges()[1]->head() != entry) ||  \
         (left->outEdges()[1]->head() == right && left->outEdges()[0]->head() != entry)))   \
    {                                                                                       \
        auto subregion = std::make_unique<Region>(Region::COMPOUND_CONDITION);              \
        subregion->setEntry(entry);                                                         \
        subregion->nodes().push_back(entry);                                                \
        subregion->nodes().push_back(left);                                                 \
        return insertSubregion(entry->parent(), std::move(subregion));                      \
    }
    REDUCE(left, right)
    REDUCE(right, left)
#undef REDUCE

    return false;
}

namespace {

struct LoopDescription {
    Region::RegionKind kind;
    Node *condition;
    Node *bodyEntry;
    Node *exitNode;

    LoopDescription(Region::RegionKind kind, Node *condition, Node *bodyEntry, Node *exitNode):
        kind(kind), condition(condition), bodyEntry(bodyEntry), exitNode(exitNode)
    {}
};

} // anonymous namespace

bool StructureAnalyzer::reduceCyclic(Node *entry, const Dfs &dfs) {
    /*
     * Try to find the nodes constituting the loop.
     */
    LoopExplorer explorer(entry, dfs);

    if (explorer.loopNodes().empty()) {
        return false;
    }

    /*
     * Create the region.
     */
    auto subregion = std::make_unique<Region>(Region::LOOP);
    subregion->setEntry(entry);
    subregion->nodes() = std::move(explorer.loopNodes());

    /*
     * Potential condition nodes, together with the respective loop
     * exit blocks. We need to compute it before doing the structural
     * analysis on the loop subregion, because we need the edges before
     * their redirection by insertSubregion().
     */
    std::vector<LoopDescription> loopDescriptions;

    auto detectLoop = [&](Region::RegionKind kind, Node *condition, Node *bodyEntry, Node *loopExit) {
        if (!nc::contains(subregion->nodes(), loopExit)) {
            assert(nc::contains(subregion->nodes(), bodyEntry));
            loopDescriptions.push_back(LoopDescription(kind, condition, bodyEntry, loopExit));
        }
    };

    /*
     * For a DO_WHILE loop.
     */
    foreach (Edge *edge, entry->inEdges()) {
        if (dfs.getEdgeType(edge) == Dfs::BACK) {
            Node *n = edge->tail();
            if (n->isFork() && n->isCondition()) {
                detectLoop(Region::DO_WHILE, n, n->outEdges()[0]->head(), n->outEdges()[1]->head());
                detectLoop(Region::DO_WHILE, n, n->outEdges()[1]->head(), n->outEdges()[0]->head());
            }
        }
    }

    /*
     * For a WHILE loop.
     */
    if (entry->isFork() && entry->isCondition()) {
        detectLoop(Region::WHILE, entry, entry->outEdges()[0]->head(), entry->outEdges()[1]->head());
        detectLoop(Region::WHILE, entry, entry->outEdges()[1]->head(), entry->outEdges()[0]->head());
    }

    /*
     * Insert the subregion, redirect edges.
     */
    Region *loop = insertSubregion(entry->parent(), std::move(subregion));
    if (!loop) {
        return false;
    }

    /*
     * Remove 'continue' edges. They only make the structural
     * analysis in the loop region harder.
     */
    std::vector<Edge *> continueEdges = entry->inEdges();
    foreach (Edge *edge, continueEdges) {
        edge->setTail(nullptr);
        edge->setHead(nullptr);
    }

    /*
     * Run structural analysis inside the loop region.
     */
    analyze(loop);

    /*
     * Try to find a condition node.
     */
    foreach (const auto &description, loopDescriptions) {
        if (description.condition->parent() == loop) {
            if (description.kind == Region::WHILE) {
                /*
                 * analyze() could put the body entry somewhere
                 * in the middle of some other region, in which
                 * case we will not be able to fall through into
                 * it from the while condition.
                 */
                auto uniqueSuccessor = description.condition->uniqueSuccessor();
                if (!uniqueSuccessor ||
                    description.bodyEntry->getEntryBasicBlock() != uniqueSuccessor->getEntryBasicBlock()) {
                    continue;
                }
            }

            loop->setRegionKind(description.kind);
            loop->setLoopCondition(description.condition);
            loop->setExitBasicBlock(description.exitNode->getEntryBasicBlock());
            break;
        }
    }

    return true;
}

bool StructureAnalyzer::reduceSwitch(Node *entry) {
    /*
     * Do not detect the same switch multiple times.
     */
    if (auto witch = entry->parent()->as<Switch>()) {
        if (witch->switchNode() == entry) {
            return false;
        }
    }

    /*
     * Entry must be a basic block node.
     */
    BasicNode *basicEntry = entry->as<BasicNode>();

    if (!basicEntry) {
        return false;
    }

    /*
     * Entry must end with an unconditional jump using a jump table,
     * and for which we can trace the jump index.
     */
    misc::ArrayAccess arrayAccess;
    std::size_t jumpTableSize;

    if (const Jump *jump = basicEntry->basicBlock()->getJump()) {
        if (jump->isUnconditional() && jump->thenTarget().table()) {
            arrayAccess = misc::recognizeArrayAccess(jump->thenTarget().address(), dataflow_);
            jumpTableSize = jump->thenTarget().table()->size();
        }
    }

    if (!arrayAccess) {
        return false;
    }

    /*
     * Typically, there is a bounds check before the switch:
     *
     * if (x < 10) {
     *     switch (x) {
     *       ...
     *     }
     * }
     */
    BasicNode *boundsCheckNode = nullptr;

    if (Node *node = entry->uniquePredecessor()) {
        if (BasicNode *basicBlockNode = node->as<BasicNode>()) {
            if (const Jump *jump = basicBlockNode->basicBlock()->getJump()) {
                if (auto boundsCheck = misc::recognizeBoundsCheck(jump, entry->getEntryBasicBlock(), dataflow_)) {
                    if (dflow::getFirstCopy(boundsCheck.index(), dataflow_) ==
                        dflow::getFirstCopy(arrayAccess.index(), dataflow_))
                    {
                        boundsCheckNode = basicBlockNode;
                        jumpTableSize = std::min(jumpTableSize, static_cast<std::size_t>(boundsCheck.maxValue() + 1));
                    }
                }
            }
        }
    }

    /*
     * Node getting control if the bounds check fails is either an exit or a default.
     */
    Node *exitOrDefaultBranch = nullptr;

    if (boundsCheckNode) {
        exitOrDefaultBranch = boundsCheckNode->getOtherSuccessor(entry);
    }

    /*
     * Direct successors of entry node and exitOrDefaultBranch are branches of the switch.
     */
    std::vector<Node *> branches;

    foreach (Edge *edge, entry->outEdges()) {
        branches.push_back(edge->head());
    }

    if (exitOrDefaultBranch) {
        branches.push_back(exitOrDefaultBranch);
    }

    std::sort(branches.begin(), branches.end());
    branches.erase(std::unique(branches.begin(), branches.end()), branches.end());

    /*
     * Using BFS, for each branch, compute the nodes reachable solely from this branch.
     */
    boost::unordered_map<Node *, Node *> node2branch;
    std::queue<Node *> queue;

    foreach (Node *node, branches) {
        queue.push(node);
        node2branch[node] = node;
    }

    while (!queue.empty()) {
        foreach (Edge *outEdge, queue.front()->outEdges()) {
            if (!nc::find(node2branch, outEdge->head())) {
                Node *branch = nullptr;

                foreach (Edge *inEdge, outEdge->head()->inEdges()) {
                    if (Node *inBranch = nc::find(node2branch, inEdge->tail())) {
                        if (branch == nullptr) {
                            branch = inBranch;
                        } else if (branch != inBranch) {
                            branch = nullptr;
                            break;
                        }
                    } else {
                        branch = nullptr;
                        break;
                    }
                }

                if (branch != nullptr) {
                    node2branch[outEdge->head()] = branch;
                    queue.push(outEdge->head());
                }
            }
        }

        queue.pop();
    }

    /*
     * Returns how many branches actually join in the given node.
     */
    auto getJoinDegree = [&](const Node *node) -> std::size_t {
        std::vector<Node *> incomingBranches;

        foreach (const Edge *edge, node->inEdges()) {
            incomingBranches.push_back(find(node2branch, edge->tail()));
        }

        std::sort(incomingBranches.begin(), incomingBranches.end());
        incomingBranches.erase(std::unique(incomingBranches.begin(), incomingBranches.end()), incomingBranches.end());

        return incomingBranches.size();
    };

    /*
     * One of the branches can be actually an exit branch, i.e. go outside
     * the switch region.
     */
    Node *exitBranch = nullptr;

    /* More than so many other branches must join in the exit branch. */
    std::size_t exitBranchJoinDegree = 2;

    foreach (Node *branch, branches) {
        auto mergeDegree = getJoinDegree(branch);
        if (mergeDegree > exitBranchJoinDegree) {
            exitBranchJoinDegree = mergeDegree;
            exitBranch = branch;
        }
    }

    /* If bounds check does not lead to an exit, it leads to the default branch. */
    Node *defaultBranch = nullptr;

    if (exitBranch != exitOrDefaultBranch) {
        defaultBranch = exitOrDefaultBranch;
    }

    /*
     * Create the region.
     */
    auto subregion = std::make_unique<Switch>(basicEntry, arrayAccess.index(), jumpTableSize);

    subregion->nodes().push_back(entry);

    if (boundsCheckNode) {
        subregion->nodes().push_back(boundsCheckNode);
        subregion->setBoundsCheckNode(boundsCheckNode);
        subregion->setEntry(boundsCheckNode);
    } else {
        subregion->setEntry(entry);
    }

    if (exitBranch) {
        subregion->setExitBasicBlock(exitBranch->getEntryBasicBlock());
    }

    if (defaultBranch) {
        subregion->setDefaultBasicBlock(defaultBranch->getEntryBasicBlock());
    }

    foreach (const auto &pair, node2branch) {
        assert(pair.second != nullptr);

        if (pair.second != exitBranch) {
            subregion->nodes().push_back(pair.first);
        }
    }

    /*
     * Install this new region, redirect edges.
     */
    return insertSubregion(entry->parent(), std::move(subregion));
}

Region *StructureAnalyzer::insertSubregion(Region *region, std::unique_ptr<Region> subregion) {
    assert(region != nullptr);
    assert(subregion != nullptr);

    if (region->entry() == subregion->entry()) {
        region->setEntry(subregion.get());
    } else if (nc::contains(subregion->nodes(), region->entry())) {
        return nullptr;
    }

    foreach (auto node, subregion->nodes()) {
        node->setParent(subregion.get());
    }

    region->nodes().erase(
        std::remove_if(
            region->nodes().begin(),
            region->nodes().end(),
            [&](Node *n) { return n->parent() == subregion.get(); }),
        region->nodes().end());

    subregion->setParent(region);
    region->nodes().push_back(subregion.get());

    /*
     * Redirect edges properly.
     */

    std::vector<Edge *> edgesToSubregion;
    std::vector<Edge *> edgesFromSubregion;
    std::vector<Edge *> duplicateEdges;

    std::vector<Node *> tails;
    std::vector<Node *> heads;

    foreach (Node *node, subregion->nodes()) {
        foreach (Edge *edge, node->inEdges()) {
            assert(edge->tail()->parent() == region || edge->tail()->parent() == subregion.get());

            if (edge->tail()->parent() == region) {
                if (edge->head() == subregion->entry() && !nc::contains(tails, edge->tail())) {
                    edgesToSubregion.push_back(edge);
                    tails.push_back(edge->tail());
                } else {
                    duplicateEdges.push_back(edge);
                }
            }
        }
        foreach (Edge *edge, node->outEdges()) {
            assert(edge->head()->parent() == region || edge->head()->parent() == subregion.get());

            if (edge->head()->parent() == region) {
                if (!nc::contains(heads, edge->head())) {
                    edgesFromSubregion.push_back(edge);
                    heads.push_back(edge->head());
                } else {
                    duplicateEdges.push_back(edge);
                }
            }
        }
    }

    foreach (Edge *edge, edgesToSubregion) {
        edge->setHead(subregion.get());
    }
    foreach (Edge *edge, edgesFromSubregion) {
        edge->setTail(subregion.get());
    }
    foreach (Edge *edge, duplicateEdges) {
        edge->setTail(nullptr);
        edge->setHead(nullptr);
    }

    return graph_.addNode(std::move(subregion));
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Utils.h"

#include <cassert>
#include <queue>

#include <boost/unordered_map.hpp>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/Unreachable.h>
#include <nc/common/Unused.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/CFG.h>
#include <nc/core/ir/Dominators.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/CallHook.h>
#include <nc/core/ir/calling/EntryHook.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/ReturnHook.h>
#include <nc/core/ir/dflow/Dataflow.h>
#include <nc/core/ir/dflow/Utils.h>

namespace nc {
namespace core {
namespace ir {
namespace cgen {

bool isBefore(const Statement *first, const Statement *second) {
    assert(first != nullptr);
    assert(second != nullptr);
    assert(first->basicBlock() == second->basicBlock());

    if (first == second) {
        return false;
    }

    if (first->instruction() && second->instruction() &&
        first->instruction() != second->instruction())
    {
        return first->instruction()->addr() < second->instruction()->addr();
    } else {
        const auto &statements = first->basicBlock()->statements();
        assert(nc::contains(statements, first));
        assert(nc::contains(statements, second));
        return std::find(
            std::next(std::find(statements.begin(), statements.end(), first)),
            statements.end(),
            second) != statements.end();
    }
}

bool isDominating(const Statement *first, const Statement *second, const Dominators &dominators) {
    assert(first != nullptr);
    assert(second != nullptr);

    if (first->basicBlock() == second->basicBlock()) {
        return isBefore(first, second);
    }

    return dominators.isDominating(first->basicBlock(), second->basicBlock());
}

boost::optional<bool> allOfBasicBlocksBetween(const BasicBlock *first, const BasicBlock *second, const CFG &cfg,
                                              std::function<bool(const BasicBlock *)> pred) {
    enum Color {
        WHITE,
        GRAY,
        BLACK
    };

    std::queue<const BasicBlock *> queue;
    boost::unordered_map<const BasicBlock *, Color> colors;

    queue.push(first);
    colors[first] = GRAY;

    while (!queue.empty()) {
        foreach (auto successor, cfg.getSuccessors(queue.front())) {
            if (nc::find(colors, successor) == WHITE) {
                if (successor != second) {
                    queue.push(successor);
                }
                colors[successor] = GRAY;
            }
        }
        queue.pop();
    }

    if (nc::find(colors, second) == WHITE) {
        return boost::none;
    }

    queue.push(second);
    colors[second] = BLACK;

    while (!queue.empty()) {
        foreach (auto predecessor, cfg.getPredecessors(queue.front())) {
            if (nc::find(colors, predecessor) == GRAY) {
                if (predecessor != first) {
                    if (!pred(predecessor)) {
                        return false;
                    }
                    queue.push(predecessor);
                }
                colors[predecessor] = BLACK;
            }
        }
        queue.pop();
    }

    assert(nc::find(colors, first) == BLACK);

    return true;
}

boost::optional<bool> allOfStatementsBetween(const Statement *first, const Statement *second, const CFG &cfg,
                                             std::function<bool(const Statement *)> pred) {
    assert(first != nullptr);
    assert(second != nullptr);

    if (first->basicBlock() == second->basicBlock()) {
        if (isBefore(first, second)) {
            const auto &statements = first->basicBlock()->statements();
            auto begin = statements.get_iterator(first);
            auto end = statements.get_iterator(second);

            return std::all_of(begin, end, pred);
        }
        return boost::none;
    }

    if (auto result = allOfBasicBlocksBetween(first->basicBlock(), second->basicBlock(), cfg,
                                              [&](const BasicBlock *basicBlock) -> bool {
            return std::all_of(basicBlock->statements().begin(), basicBlock->statements().end(), pred);
        })) {
        return *result &&
            std::all_of(std::next(first->basicBlock()->statements().get_iterator(first)),
                        first->basicBlock()->statements().end(), pred) &&
            std::all_of(second->basicBlock()->statements().begin(),
                        second->basicBlock()->statements().get_iterator(second), pred);
    }

    return boost::none;
}

const Term *getWrittenTerm(const Statement *statement) {
    assert(statement != nullptr);
    if (auto assignment = statement->asAssignment()) {
        return assignment->left();
    } else if (auto touch = statement->asTouch()) {
        if (touch->accessType() == Term::WRITE) {
            return touch->term();
        }
    }
    return nullptr;
}

boost::optional<Domain> getDomain(const Term *term) {
    assert(term != nullptr);
    if (auto access = term->asMemoryLocationAccess()) {
        return access->memoryLocation().domain();
    } else if (auto dereference = term->asDereference()) {
        return dereference->domain();
    }
    return boost::none;
}

const Term *getTheOnlyDefinition(const Term *term, const dflow::Dataflow &dataflow) {
    assert(term != nullptr);
    assert(term->isRead());

    const auto &definitions = dataflow.getDefinitions(term);

    if (definitions.chunks().size() == 1 &&
        definitions.chunks().front().definitions().size() == 1)
    {
        return definitions.chunks().front().definitions().front();
    }
    return nullptr;
}

boost::unordered_set<const Statement *> getHookStatements(const Function *function, const dflow::Dataflow &dataflow, const calling::Hooks &hooks) {
    boost::unordered_set<const Statement *> result;

    if (auto hook = hooks.getEntryHook(function)) {
        foreach (const auto &termAndClone, hook->argumentTerms()) {
            result.insert(termAndClone.second->statement());
        }
    }

    foreach (auto basicBlock, function->basicBlocks()) {
        foreach (auto statement, basicBlock->statements()) {
            if (auto call = statement->asCall()) {
                if (auto hook = hooks.getCallHook(call)) {
                    foreach (const auto &termAndClone, hook->argumentTerms()) {
                        result.insert(termAndClone.second->statement());
                    }
                    foreach (const auto &termAndClone, hook->returnValueTerms()) {
                        result.insert(termAndClone.second->statement());
                    }
                }
            } else if (auto jump = statement->asJump()) {
                if (dflow::isReturn(jump, dataflow)) {
                    if (auto hook = hooks.getReturnHook(jump)) {
                        foreach (const auto &termAndClone, hook->returnValueTerms()) {
                            result.insert(termAndClone.second->statement());
                        }
                    }
                }
            }
        }
    }

    return result;
}

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

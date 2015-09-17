/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <functional>

#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>

#include <nc/core/ir/MemoryDomain.h>

namespace nc {
namespace core {
namespace ir {

class BasicBlock;
class CFG;
class Dominators;
class Function;
class Statement;
class Term;

namespace calling {
    class Hooks;
}

namespace dflow {
    class Dataflow;
}

namespace cgen {

/**
 * \param[in] first Valid pointer to a statement.
 * \param[in] second Valid pointer to a statement in the same basic block.
 *
 * \return True iff the first statement is before the second statement in the basic block.
 */
bool isBefore(const Statement *first, const Statement *second);

/**
 * \param[in] first Valid pointer to a statement in a CFG.
 * \param[in] second Valid pointer to a statement in the same CFG.
 * \param[in] dominators Dominator sets for the CFG.
 *
 * \return True iff the first statement dominates the second statement in the CFG.
 */
bool isDominating(const Statement *first, const Statement *second, const Dominators &dominators);

/**
 * \param[in] first Valid pointer to a basic block a CFG.
 * \param[in] second Valid pointer to a basic block in the same CFG.
 * \param[in] cfg The CFG.
 * \param[in] pred A predicate.
 *
 * \return True if the predicate holds for all basic blocks (excluding
 *         first and second) lying on all simple CFG paths from first to
 *         second, false if it does not, boost::none if there is no path
 *         from the first basic block to the second basic block.
 */
boost::optional<bool> allOfBasicBlocksBetween(const BasicBlock *first, const BasicBlock *second, const CFG &cfg,
                                              std::function<bool(const BasicBlock *)> pred);

/**
 * \param[in] first Valid pointer to a statement in a CFG.
 * \param[in] second Valid pointer to a statement in the same CFG.
 * \param[in] cfg The CFG.
 * \param[in] pred A predicate.
 *
 * \return True iff the predicate holds for all statements (except first
 *         and second) lying on all simple CFG paths from first to second,
 *         false if it does not, boost::none if there is no path from
 *         the first statement to the second statement in the CFG.
 */
boost::optional<bool> allOfStatementsBetween(const Statement *first, const Statement *second, const CFG &cfg,
                                             std::function<bool(const Statement *)> pred);

/**
 * \param[in] statement Valid pointer to a statement.
 *
 * \return Pointer to the term written by the statement. Can be nullptr.
 */
const Term *getWrittenTerm(const Statement *statement);

/**
 * \param[in] term Valid pointer to a term.
 *
 * \return The memory domain accessed by the term,
 *         boost::none if the term cannot have a memory location.
 */
boost::optional<Domain> getDomain(const Term *term);

/**
 * \param[in] term Valid pointer to a read term.
 * \param[in] dataflow Dataflow information for the function to which the term belongs.
 *
 * \return A valid pointer to the write term being the only
 *         term defining what the read term reads. If there is
 *         more than one such term or no such term, returns nullptr.
 */
const Term *getTheOnlyDefinition(const Term *read, const dflow::Dataflow &dataflow);

/**
 * \param[in] function Valid pointer to a function.
 * \param[in] dataflow Dataflow information for the function.
 * \param[in] hooks Call hooks.
 *
 * \return The list of statements belonging to the hooks of calling conventions in the function.
 */
boost::unordered_set<const Statement *> getHookStatements(const Function *function, const dflow::Dataflow &dataflow,
                                                          const calling::Hooks &hooks);

} // namespace cgen
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

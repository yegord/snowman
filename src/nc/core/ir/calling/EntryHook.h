/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include "Patch.h"

namespace nc {
namespace core {
namespace ir {

class Term;

namespace calling {

class Convention;
class FunctionSignature;

/**
 * Hook installed at function's entry.
 */
class EntryHook {
    /** Mapping from argument terms to their clones. */
    boost::unordered_map<const Term *, const Term *> argumentTerms_;

    Patch patch_;

public:
    /**
     * Class constructor.
     *
     * \param[in] convention    Valid pointer to the calling convention.
     * \param[in] signature     Pointer to the function's signature. Can be nullptr.
     */
    EntryHook(const Convention *convention, const FunctionSignature *signature);

    /**
     * \return Patch to be inserted in the beginning of the function's entry.
     */
    Patch &patch() { return patch_; }

    /**
     * \param term Valid pointer to the term representing an argument
     *             in the signature.
     *
     * \return Pointer to the term representing this argument in the instrumentation.
     *         Will be nullptr, if the signature does not include such an argument.
     */
    const Term *getArgumentTerm(const Term *term) const {
        assert(term != nullptr);
        return nc::find(argumentTerms_, term);
    }

    /**
     * \return Mapping from argument terms to their clones.
     */
    const boost::unordered_map<const Term *, const Term *> &argumentTerms() const { return argumentTerms_; }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et ts=4 sw=4: */

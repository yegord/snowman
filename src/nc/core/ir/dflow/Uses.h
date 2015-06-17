/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {

class Term;

namespace dflow {

class Dataflow;

/**
 * Information about which term is being read by which terms.
 */
class Uses {
public:
    /**
     * Information about a use of a definition: a memory location
     * being read and a term reading this memory location.
     */
    class Use {
        MemoryLocation location_;
        const Term *term_;

    public:
        Use(const MemoryLocation &location, const Term *term):
            location_(location), term_(term)
        {}

        const MemoryLocation &location() const { return location_; }
        const Term *term() const { return term_; }
    };

private:
    /** Mapping from a write term to the list of its uses. */
    boost::unordered_map<const Term *, std::vector<Use>> term2uses_;

public:
    /**
     * Constructs use information from dataflow information.
     *
     * \param dataflow Dataflow.
     */
    explicit
    Uses(const Dataflow &dataflow);

    /**
     * \param[in] term Valid pointer to a write term.
     *
     * \return List of term's uses.
     */
    const std::vector<Use> &getUses(const Term *term) const {
        assert(term != nullptr);
        assert(term->isWrite());
        return nc::find(term2uses_, term);
    }
};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

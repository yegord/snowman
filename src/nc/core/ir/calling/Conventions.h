/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include "CalleeId.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

class Convention;

/**
 * Information about assigned calling conventions and their parameters.
 */
class Conventions {
    /** Mapping from a callee id to the associated calling convention. */
    boost::unordered_map<CalleeId, const Convention *> id2convention_;

    /** Mapping from a callee id to the size of its arguments passed on the stack. */
    boost::unordered_map<CalleeId, ByteSize> id2stackArgumentsSize_;

public:

    /**
     * Assigns a calling convention to a callee id.
     *
     * \param calleeId Callee id.
     * \param convention Pointer to a calling convention of the function. Can be nullptr.
     */
    void setConvention(const CalleeId &calleeId, const Convention *convention) {
        id2convention_[calleeId] = convention;
    }

    /**
     * \param calleeId Callee id.
     *
     * \return Pointer to the calling convention used for calls to given address. Can be nullptr.
     */
    const Convention *getConvention(const CalleeId &calleeId) const {
        return nc::find(id2convention_, calleeId);
    }

    /**
     * Sets the stack arguments size for the given callee id.
     *
     * \param calleeId Callee id.
     * \param size Size of the arguments passed on the stack, or boost::none if unknown.
     */
    void setStackArgumentsSize(const CalleeId &calleeId, const boost::optional<ByteSize> &size) {
        if (size) {
            id2stackArgumentsSize_[calleeId] = *size;
        } else {
            id2stackArgumentsSize_.erase(calleeId);
        }
    }

    /**
     * \param calleeId Callee id.
     *
     * \return Size of the arguments passed on the stack, or boost::none if unknown.
     */
    boost::optional<ByteSize> getStackArgumentsSize(const CalleeId &calleeId) const {
        return nc::find_optional(id2stackArgumentsSize_, calleeId);
    }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

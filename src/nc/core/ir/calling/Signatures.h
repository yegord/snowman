/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>

#include <boost/unordered_map.hpp>

#include <nc/common/Range.h>

#include "CallSignature.h"
#include "FunctionSignature.h"

namespace nc {
namespace core {
namespace ir {
namespace calling {

/**
 * Signatures of functions and calls to functions.
 */
class Signatures {
    /** Mapping from an address to the signature of the function located at this address. */
    boost::unordered_map<ByteAddr, std::shared_ptr<FunctionSignature>> addr2signature_;

    /** Mapping from a function to its signature. */
    boost::unordered_map<const Function *, std::shared_ptr<FunctionSignature>> function2signature_;

    /** Mapping from a call to its signature. */
    boost::unordered_map<const Call *, std::shared_ptr<CallSignature>> call2signature_;

public:
    /**
     * \param addr Address of a function.
     *
     * \return Pointer to the signature of the function. Can be nullptr.
     */
    const std::shared_ptr<FunctionSignature> &getSignature(ByteAddr addr) const {
        return nc::find(addr2signature_, addr);
    }

    /**
     * Sets the signature of a function at the given address.
     *
     * \param addr Address of the function.
     * \param signature Pointer to the signature. Can be nullptr.
     */
    void setSignature(ByteAddr addr, std::shared_ptr<FunctionSignature> signature) {
        addr2signature_[addr] = std::move(signature);
    }

    /**
     * \param function Valid pointer to a function.
     *
     * \return Pointer to the signature of the function. Can be nullptr.
     */
    const std::shared_ptr<FunctionSignature> &getSignature(const Function *function) const {
        assert(function != nullptr);
        return nc::find(function2signature_, function);
    }

    /**
     * Sets the signature of a function.
     *
     * \param function Valid pointer to the function.
     * \param signature Pointer to the signature. Can be nullptr.
     */
    void setSignature(const Function *function, std::shared_ptr<FunctionSignature> signature) {
        assert(function != nullptr);
        function2signature_[function] = std::move(signature);
    }

    /**
     * \param call Valid pointer to a call statement.
     *
     * \return Pointer to the signature of the call. Can be nullptr.
     */
    const std::shared_ptr<CallSignature> &getSignature(const Call *call) const {
        assert(call != nullptr);
        return nc::find(call2signature_, call);
    }

    /**
     * Sets the signature of a call.
     *
     * \param call Valid pointer to the call statement.
     * \param signature Pointer to the signature. Can be nullptr.
     */
    void setSignature(const Call *call, std::shared_ptr<CallSignature> signature) {
        assert(call != nullptr);
        call2signature_[call] = std::move(signature);
    }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

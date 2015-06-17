/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace calling {

/**
 * Signature of a function: arguments, return value, name.
 */
class CallSignature {
    std::vector<std::shared_ptr<const Term>> arguments_; ///< Terms representing the arguments.
    std::shared_ptr<const Term> returnValue_; ///< Terms representing the return value.

public:
    /**
     * \return List of terms representing the arguments.
     */
    std::vector<std::shared_ptr<const Term>> &arguments() { return arguments_; }

    /**
     * \return List of terms representing the arguments.
     */
    const std::vector<std::shared_ptr<const Term>> &arguments() const { return arguments_; }

    /**
     * \return Pointer to the term containing the return value. Can be nullptr.
     */
    const std::shared_ptr<const Term> &returnValue() const { return returnValue_; }

    /**
     * Sets the pointer to the term representing the return value.
     *
     * \param term Valid pointer to the term.
     */
    void setReturnValue(std::shared_ptr<const Term> term) { returnValue_ = std::move(term); }
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */


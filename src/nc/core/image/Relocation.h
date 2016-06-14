/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>

#include <nc/common/Types.h>

namespace nc {
namespace core {
namespace image {

class Symbol;

/**
 * Information about a single relocation.
 */
class Relocation {
    ByteAddr address_; ///< Virtual address to be patched.
    const Symbol *symbol_; ///< Symbol with whose address to patch.
    ByteSize size_; ///< Size of the patched value
    ByteSize addend_; ///< Displacement to add to the symbol's address.

public:

    /**
     * Constructor.
     *
     * \param address Virtual address to be patched.
     * \param symbol Valid pointer to the symbol whose address to use.
     * \param addend Displacement to add to the symbol's address.
     */
    Relocation(ByteAddr address, const Symbol *symbol, ByteSize size, ByteSize addend = 0):
        address_(address), symbol_(symbol), size_(size), addend_(addend)
    {
        assert(symbol != nullptr);
    }

    /**
     * \return Virtual address to be patched.
     */
    ByteAddr address() const { return address_; }

    /**
     * \return Valid pointer to the symbol with whose address to patch.
     */
    const Symbol *symbol() const { return symbol_; }

    /**
     * \return Displacement to add to the symbol's address.
     */
    ByteSize addend() const { return addend_; }

    /**
     * \return Size of address to be patched
     */
    ByteSize size() const { return size_; }
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

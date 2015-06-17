/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include <boost/unordered_map.hpp>

#include <QString>

#include "ByteSource.h"
#include "Symbol.h"

namespace nc { namespace core {

namespace arch {
    class Architecture;
}

namespace mangling {
    class Demangler;
}

namespace image {

class Section;
class Relocation;

/**
 * An executable image.
 */
class Image: public ByteSource {
    const arch::Architecture *architecture_; ///< Architecture.
    std::vector<std::unique_ptr<Section>> sections_; ///< The list of sections.
    std::vector<std::unique_ptr<Symbol>> symbols_; ///< The list of symbols.
    boost::unordered_map<ConstantValue, Symbol *> value2symbol_; ///< Mapping from value to the symbol with this value.
    std::vector<std::unique_ptr<Relocation>> relocations_; ///< The list of relocations.
    boost::unordered_map<ByteAddr, Relocation *> address2relocation_; ///< Mapping from an address to the relocation with this address.
    std::unique_ptr<mangling::Demangler> demangler_; ///< Demangler.

public:
    /**
     * Constructor.
     */
    Image();

    /**
     * Destructor.
     */
    ~Image();

    /**
     * \return Pointer to the architecture. Can be nullptr.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * Sets the architecture of this executable image.
     * The architecture must not have been set before.
     *
     * \param architecture Valid pointer to the architecture.
     */
    void setArchitecture(const arch::Architecture *architecture);

    /**
     * Sets the architecture of this executable image.
     * The architecture must not have been set before.
     *
     * \param name Name of the architecture.
     */
    void setArchitecture(const QString &name);

    /**
     * Adds a new section.
     *
     * \param section Valid pointer to the section.
     */
    void addSection(std::unique_ptr<Section> section);

    /**
     * \return List of all sections.
     */
    const std::vector<Section *> &sections() {
        return reinterpret_cast<const std::vector<Section *> &>(sections_);
    }

    /**
     * \return List of all sections.
     */
    const std::vector<const Section *> &sections() const {
        return reinterpret_cast<const std::vector<const Section *> &>(sections_);
    }

    /**
     * \param[in] addr Linear address.
     *
     * \return A valid pointer to allocated section containing given
     *         virtual address or nullptr if there is no such section.
     */
    const Section *getSectionContainingAddress(ByteAddr addr) const;

    /**
     * \param[in] name Section name.
     *
     * \return Valid pointer to a section with the given name,
     *         nullptr if there is no such section.
     */
    const Section *getSectionByName(const QString &name) const;

    /**
     * Reads a sequence of bytes from the section containing
     * the given address and allocated during program execution.
     */
    ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;

    /**
     * Adds a symbol.
     *
     * \param symbol Valid pointer to the symbol.
     *
     * \return Pointer to the added symbol.
     */
    const Symbol *addSymbol(std::unique_ptr<Symbol> symbol);

    /**
     * \return List of all symbols.
     */
    const std::vector<const Symbol *> &symbols() const {
        return reinterpret_cast<const std::vector<const Symbol *> &>(symbols_);
    }

    /**
     * Finds a symbol with a given type and value.
     *
     * \param value Value of the symbol.
     *
     * \return Pointer to a symbol with the given value. Can be nullptr.
     */
    const Symbol *getSymbol(ConstantValue value) const;

    /**
     * Adds an information about relocation.
     *
     * \param relocation Valid pointer to a relocation information.
     *
     * \return Pointer to the added relocation.
     */
    const Relocation *addRelocation(std::unique_ptr<Relocation> relocation);

    /**
     * \param address Virtual address.
     *
     * \return Pointer to a relocation for this address. Can be nullptr.
     */
    const Relocation *getRelocation(ByteAddr address) const;

    /**
     * \return Valid pointer to a demangler.
     */
    const mangling::Demangler *demangler() const { return demangler_.get(); }

    /**
     * Sets the demangler.
     *
     * \param demangler Valid pointer to the new demangler.
     */
    void setDemangler(std::unique_ptr<mangling::Demangler> demangler);

    /**
     * Sets the demangler.
     *
     * \param name Name of the demangler.
     */
    void setDemangler(const QString &name);
};

}}} // namespace nc::core::image

/* vim:set et sts=4 sw=4: */

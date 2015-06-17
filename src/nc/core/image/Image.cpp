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

#include "Image.h"

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/ArchitectureRepository.h>
#include <nc/core/image/Image.h>
#include <nc/core/mangling/DefaultDemangler.h>

#include "Relocation.h"
#include "Section.h"

namespace nc { namespace core { namespace image {

Image::Image():
    architecture_(nullptr),
    demangler_(new mangling::DefaultDemangler())
{}

Image::~Image() {}

void Image::setArchitecture(const arch::Architecture *architecture) {
    assert(architecture != nullptr);
    assert(architecture_ == nullptr && "Can't set the architecture twice.");

    architecture_ = architecture;
}

void Image::setArchitecture(const QString &name) {
    setArchitecture(arch::ArchitectureRepository::instance()->getArchitecture(name));
}

void Image::addSection(std::unique_ptr<Section> section) {
    assert(section != nullptr);
    sections_.push_back(std::move(section));
}

const Section *Image::getSectionContainingAddress(ByteAddr addr) const {
    foreach (auto section, sections()) {
        if (section->isAllocated() && section->containsAddress(addr)) {
            return section;
        }
    }
    return nullptr;
}

const Section *Image::getSectionByName(const QString &name) const {
    foreach (auto section, sections()) {
        if (section->name() == name) {
            return section;
        }
    }
    return nullptr;
}

ByteSize Image::readBytes(ByteAddr addr, void *buf, ByteSize size) const {
    if (const Section *section = getSectionContainingAddress(addr)) {
        return section->readBytes(addr, buf, size);
    } else {
        return 0;
    }
}

const Symbol *Image::addSymbol(std::unique_ptr<Symbol> symbol) {
    auto result = symbol.get();

    symbols_.push_back(std::move(symbol));

    if (result->value()) {
        value2symbol_[*result->value()] = result;
    }

    return result;
}

const Symbol *Image::getSymbol(ConstantValue value) const {
    return nc::find(value2symbol_, value);
}

const Relocation *Image::addRelocation(std::unique_ptr<Relocation> relocation) {
    auto result = relocation.get();

    relocations_.push_back(std::move(relocation));
    address2relocation_[result->address()] = result;

    return result;
}

const Relocation *Image::getRelocation(ByteAddr address) const {
    return nc::find(address2relocation_, address);
}

void Image::setDemangler(std::unique_ptr<mangling::Demangler> demangler) {
    assert(demangler != nullptr);

    demangler_ = std::move(demangler);
}

void Image::setDemangler(const QString &) {
    /* Currently only the bundled demangler is supported. */
}

}}} // namespace nc::core::image

/* vim:set et sts=4 sw=4: */

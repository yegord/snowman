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

#include "ParserRepository.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/input/elf/ElfParser.h>
#include <nc/input/mach-o/MachOParser.h>
#include <nc/input/pe/PeParser.h>
#include <nc/input/le/LeParser.h>

#include "Parser.h"

namespace nc { namespace core { namespace input {

namespace {

ParserRepository *createInstance() {
    static ParserRepository result;
    result.registerParser(std::make_unique<nc::input::elf::ElfParser>());
    result.registerParser(std::make_unique<nc::input::mach_o::MachOParser>());
    result.registerParser(std::make_unique<nc::input::pe::PeParser>());
    result.registerParser(std::make_unique<nc::input::le::LeParser>());
    return &result;
}

} // anonymous namespace

ParserRepository *ParserRepository::instance() {
    static auto repository = createInstance();
    return repository;
}

void ParserRepository::registerParser(std::unique_ptr<Parser> parser) {
    assert(parser != nullptr);
    assert(!getParser(parser->name()) && "Cannot register two parsers with the same name.");

    parsers_.push_back(std::move(parser));
}

const Parser *ParserRepository::getParser(const QString &name) const {
    foreach (auto parser, parsers()) {
        if (parser->name() == name) {
            return parser;
        }
    }
    return nullptr;
}

const std::vector<const Parser *> &ParserRepository::parsers() const {
    return reinterpret_cast<const std::vector<const Parser *> &>(parsers_);
}

}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */

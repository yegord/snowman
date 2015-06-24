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

#include "Disassembly.h"

#include <nc/core/Context.h>
#include <nc/core/Driver.h>

#include <cassert>

namespace nc {
namespace gui {

Disassembly::Disassembly(const std::shared_ptr<core::Context> &context, const core::image::ByteSource *source, ByteAddr begin, ByteAddr end):
    context_(context), source_(source), begin_(begin), end_(end)
{
    assert(context);
    assert(source);
}

Disassembly::~Disassembly() {}

void Disassembly::work() {
    try {
        core::Driver::disassemble(*context_, source_, begin_, end_);
    } catch (const CancellationException &) {
        /* Nothing to do. */
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

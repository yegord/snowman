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

#include "Decompile.h"

#include <cassert>

#include <nc/common/make_unique.h>

#include <nc/core/Context.h>

#include "Decompilation.h"
#include "Project.h"

namespace nc {
namespace gui {

Decompile::Decompile(Project *project, const std::shared_ptr<const core::arch::Instructions> &instructions):
    project_(project),
    instructions_(instructions)
{
    assert(project);
    assert(instructions);

    setBackground(true);
}

void Decompile::work() {
    auto context = std::make_shared<core::Context>();
    context->setImage(project_->image());
    context->setInstructions(instructions_);
    context->setCancellationToken(cancellationToken());
    context->setLogToken(project_->logToken());

    project_->setContext(context);

    delegate(std::make_unique<Decompilation>(context));
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

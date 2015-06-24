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

#include "Project.h"

#include <cassert>

#include <nc/common/make_unique.h>
#include <nc/common/Foreach.h>

#include <nc/core/Context.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>

#include "CommandQueue.h"
#include "Decompile.h"
#include "DecompileAll.h"
#include "DeleteInstructions.h"
#include "Disassemble.h"

namespace nc {
namespace gui {

Project::Project(QObject *parent):
    QObject(parent),
    image_(std::make_shared<core::image::Image>()),
    instructions_(std::make_shared<core::arch::Instructions>()),
    context_(std::make_shared<core::Context>()),
    commandQueue_(new CommandQueue(this))
{
}

Project::~Project() {}

void Project::setName(const QString &name) {
    if (name_ != name) {
        name_ = name;
        Q_EMIT nameChanged();
    }
}

void Project::setImage(const std::shared_ptr<core::image::Image> &image) {
    assert(image);

    if (image_ != image) {
        image_ = image;
        Q_EMIT imageChanged();
    }
}

void Project::setInstructions(const std::shared_ptr<const core::arch::Instructions> &instructions) {
    assert(instructions);

    if (instructions_ != instructions) {
        instructions_ = instructions;
        Q_EMIT instructionsChanged();
    }
}

void Project::updateInstructions() {
    assert(context());
    setInstructions(context()->instructions());
}

void Project::setContext(const std::shared_ptr<const core::Context> &context) {
    assert(context);

    if (context_ != context) {
        context_ = context;

        connect(context_.get(), SIGNAL(instructionsChanged()), this, SLOT(updateInstructions()));
        connect(context_.get(), SIGNAL(treeChanged()), this, SIGNAL(treeChanged()));
    }
}

void Project::deleteInstructions(const std::vector<const core::arch::Instruction *> &instructions) {
    commandQueue()->push(std::make_unique<DeleteInstructions>(this, instructions));
}

void Project::disassemble() {
    foreach (const core::image::Section *section, image()->sections()) {
        if (section->isCode()) {
            disassemble(section, section->addr(), section->endAddr());
        }
    }
}

void Project::disassemble(const core::image::ByteSource *source, ByteAddr begin, ByteAddr end) {
    assert(source);

    commandQueue()->push(std::make_unique<Disassemble>(this, source, begin, end));
}

void Project::decompile() {
    commandQueue()->push(std::make_unique<DecompileAll>(this));
}

void Project::decompile(const std::vector<const core::arch::Instruction *> &instructions) {
    auto subset = std::make_shared<core::arch::Instructions>();

    foreach (const core::arch::Instruction *instruction, instructions) {
        auto instr = this->instructions()->get(instruction->addr());
        if (instr.get() == instruction) {
            subset->add(instr);
        }
    }

    decompile(subset);
}

void Project::decompile(const std::shared_ptr<const core::arch::Instructions> &instructions) {
    assert(instructions);

    commandQueue()->push(std::make_unique<Decompile>(this, instructions));
}

void Project::cancelAll() {
    commandQueue()->clear();
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

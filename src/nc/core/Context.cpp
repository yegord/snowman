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

#include "Context.h"

#include <nc/common/Foreach.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/calling/Conventions.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/calling/Signatures.h>
#include <nc/core/ir/cflow/Graphs.h>
#include <nc/core/ir/dflow/Dataflows.h>
#include <nc/core/ir/liveness/Livenesses.h>
#include <nc/core/ir/types/Types.h>
#include <nc/core/ir/vars/Variables.h>
#include <nc/core/likec/Tree.h>

namespace nc {
namespace core {

Context::Context():
    image_(std::make_shared<image::Image>()),
    instructions_(std::make_shared<arch::Instructions>())
{}

Context::~Context() {}

void Context::setImage(const std::shared_ptr<image::Image> &image) {
    image_ = image;
}

void Context::setInstructions(const std::shared_ptr<const arch::Instructions> &instructions) {
    instructions_ = instructions;
    Q_EMIT instructionsChanged();
}

void Context::setProgram(std::unique_ptr<ir::Program> program) {
    program_ = std::move(program);
}

void Context::setFunctions(std::unique_ptr<ir::Functions> functions) {
    functions_ = std::move(functions);
}

void Context::setConventions(std::unique_ptr<ir::calling::Conventions> conventions) {
    conventions_ = std::move(conventions);
}

void Context::setHooks(std::unique_ptr<ir::calling::Hooks> hooks) {
    hooks_ = std::move(hooks);
}

void Context::setSignatures(std::unique_ptr<ir::calling::Signatures> signatures) {
    signatures_ = std::move(signatures);
}

void Context::setDataflows(std::unique_ptr<ir::dflow::Dataflows> dataflows) {
    dataflows_ = std::move(dataflows);
}

void Context::setVariables(std::unique_ptr<ir::vars::Variables> variables) {
    variables_ = std::move(variables);
}

void Context::setLivenesses(std::unique_ptr<ir::liveness::Livenesses> livenesses) {
    livenesses_ = std::move(livenesses);
}

void Context::setGraphs(std::unique_ptr<ir::cflow::Graphs> graphs) {
    graphs_ = std::move(graphs);
}

void Context::setTypes(std::unique_ptr<ir::types::Types> types) {
    types_ = std::move(types);
}

void Context::setTree(std::unique_ptr<likec::Tree> tree) {
    tree_ = std::move(tree);
    Q_EMIT treeChanged();
}

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */

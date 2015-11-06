/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/ir/calling/Convention.h>

namespace nc {
namespace arch {
namespace mips {

class MipsArchitecture;

class o32CallingConvention: public core::ir::calling::Convention {
public:
    o32CallingConvention(const MipsArchitecture *architecture_);
};

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */

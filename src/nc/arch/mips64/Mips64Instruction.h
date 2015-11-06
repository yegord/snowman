/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/CapstoneInstruction.h>

namespace nc {
namespace arch {
namespace mips64 {

typedef core::arch::CapstoneInstruction<CS_ARCH_MIPS, 4> Mips64Instruction;

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */

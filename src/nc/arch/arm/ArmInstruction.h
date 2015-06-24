/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/CapstoneInstruction.h>

namespace nc {
namespace arch {
namespace arm {

typedef core::arch::CapstoneInstruction<CS_ARCH_ARM, 4> ArmInstruction;

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */

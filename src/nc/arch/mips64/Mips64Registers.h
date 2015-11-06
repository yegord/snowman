/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Registers.h>

namespace nc { namespace arch { namespace mips64 {

/**
 * Container class for MIPS registers.
 */
class Mips64Registers: public core::arch::StaticRegisters<Mips64Registers> {
public:
    Mips64Registers();

#define REGISTER_TABLE <nc/arch/mips64/Mips64RegisterTable.i>
#include <nc/core/arch/Registers.i>
};

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */

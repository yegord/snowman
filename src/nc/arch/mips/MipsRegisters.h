/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Registers.h>

namespace nc { namespace arch { namespace mips {

/**
 * Container class for MIPS registers.
 */
class MipsRegisters: public core::arch::StaticRegisters<MipsRegisters> {
public:
    MipsRegisters();

#define REGISTER_TABLE <nc/arch/mips/MipsRegisterTable.i>
#include <nc/core/arch/Registers.i>
};

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */

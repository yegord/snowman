/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Registers.h>

namespace nc { namespace arch { namespace allegrex {

/**
 * Container class for MIPS registers.
 */
class AllegrexRegisters: public core::arch::StaticRegisters<AllegrexRegisters> {
public:
    AllegrexRegisters();

#define REGISTER_TABLE <nc/arch/allegrex/AllegrexRegisterTable.i>
#include <nc/core/arch/Registers.i>
};

}}} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */

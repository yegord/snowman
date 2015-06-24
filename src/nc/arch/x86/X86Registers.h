/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Registers.h>

namespace nc { namespace arch { namespace x86 {

/**
 * Container class for x86 registers.
 */
class X86Registers: public core::arch::StaticRegisters<X86Registers> {
public:
    X86Registers();

#define REGISTER_TABLE <nc/arch/x86/X86RegisterTable.i>
#include <nc/core/arch/Registers.i>
};

}}} // namespace nc::arch::x86

/* vim:set et sts=4 sw=4: */

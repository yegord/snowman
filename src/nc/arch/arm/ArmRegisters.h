/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Registers.h>

namespace nc { namespace arch { namespace arm {

/**
 * Container class for ARM registers.
 */
class ArmRegisters: public core::arch::StaticRegisters<ArmRegisters> {
public:
    ArmRegisters();

#define REGISTER_TABLE <nc/arch/arm/ArmRegisterTable.i>
#include <nc/core/arch/Registers.i>
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */

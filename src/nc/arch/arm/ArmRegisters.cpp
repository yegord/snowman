/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "ArmRegisters.h"

namespace nc { namespace arch { namespace arm {

ArmRegisters::ArmRegisters() {
#define REGISTER_TABLE <nc/arch/arm/ArmRegisterTable.i>
#include <nc/core/arch/RegistersConstructor.i>
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */

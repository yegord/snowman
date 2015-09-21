/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Mips64Registers.h"

namespace nc { namespace arch { namespace mips64 {

Mips64Registers::Mips64Registers() {
#define REGISTER_TABLE <nc/arch/mips64/Mips64RegisterTable.i>
#include <nc/core/arch/RegistersConstructor.i>
}

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */

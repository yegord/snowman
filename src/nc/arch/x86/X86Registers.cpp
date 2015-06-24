/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "X86Registers.h"

namespace nc { namespace arch { namespace x86 {

X86Registers::X86Registers() {
#define REGISTER_TABLE <nc/arch/x86/X86RegisterTable.i>
#include <nc/core/arch/RegistersConstructor.i>
}

}}} // namespace nc::arch::x86

/* vim:set et sts=4 sw=4: */

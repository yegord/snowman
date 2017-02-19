/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#include "MipsRegisters.h"

namespace nc { namespace arch { namespace mips {

MipsRegisters::MipsRegisters() {
#define REGISTER_TABLE <nc/arch/mips/MipsRegisterTable.i>
#include <nc/core/arch/RegistersConstructor.i>
}

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */

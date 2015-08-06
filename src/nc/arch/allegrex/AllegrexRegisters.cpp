/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "AllegrexRegisters.h"

namespace nc { namespace arch { namespace allegrex {

AllegrexRegisters::AllegrexRegisters() {
#define REGISTER_TABLE <nc/arch/allegrex/AllegrexRegisterTable.i>
#include <nc/core/arch/RegistersConstructor.i>
}

}}} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */

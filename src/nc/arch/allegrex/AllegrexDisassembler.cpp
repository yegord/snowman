/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "AllegrexDisassembler.h"

#include <nc/common/make_unique.h>

#include "AllegrexArchitecture.h"
#include "AllegrexInstruction.h"

#include "Allegrex.h"

namespace nc {
    namespace arch {
        namespace allegrex {

            AllegrexDisassembler::AllegrexDisassembler(const AllegrexArchitecture *architecture) :
                core::arch::Disassembler(architecture)
            {
            }

            AllegrexDisassembler::~AllegrexDisassembler() {}

            std::shared_ptr<core::arch::Instruction> AllegrexDisassembler::disassembleSingleInstruction(ByteAddr pc, const void *buffer, ByteSize /*size*/) {
                if (nc::arch::allegrex::allegrex_decode_instruction(*((uint32_t *)buffer), true)) {
                    return std::make_shared<AllegrexInstruction>(pc, 4, buffer);
                }
                return nullptr;
            }

        }
    }
} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */
